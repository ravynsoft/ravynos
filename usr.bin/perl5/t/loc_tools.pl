# Common tools for test files to find the locales which exist on the
# system.  Caller should have verified that this isn't miniperl before calling
# the functions.

# Note that it's okay that some languages have their native names
# capitalized here even though that's not "right".  They are lowercased
# anyway later during the scanning process (and besides, some clueless
# vendor might have them capitalized erroneously anyway).

# Functions whose names begin with underscore are internal helper functions
# for this file, and are not to be used by outside callers.

use Config;
use strict;
use warnings;
use feature 'state';

my %known_bad_locales = (   # XXX eventually will need version info if and
                            # when these get fixed.
    solaris => [ 'vi_VN.UTF-8', ],  # Use of U+A8 segfaults: GH #20578
);

eval { require POSIX; import POSIX 'locale_h'; };
my $has_locale_h = ! $@;

my @known_categories = ( qw(LC_ALL LC_COLLATE LC_CTYPE LC_MESSAGES LC_MONETARY
                            LC_NUMERIC LC_TIME LC_ADDRESS LC_IDENTIFICATION
                            LC_MEASUREMENT LC_PAPER LC_TELEPHONE LC_SYNTAX
                            LC_TOD LC_NAME));
my @platform_categories;

sub category_excluded($) {
    my $cat_name = shift =~ s/^LC_//r;

    # Recognize Configure option to exclude a category
    return $Config{ccflags} =~ /\bD?NO_LOCALE_$cat_name\b/;
}

# LC_ALL can be -1 on some platforms.  And, in fact the implementors could
# legally use any integer to represent any category.  But it makes the most
# sense for them to have used small integers.  Below, we create new locale
# numbers for ones missing from this machine.  We make them very negative,
# hopefully more negative than anything likely to be a valid category on the
# platform, but also below is a check to be sure that our guess is valid.
my $max_bad_category_number = -1000000;

# Initialize this hash so that it looks like e.g.,
#   6 => 'CTYPE',
# where 6 is the value of &POSIX::LC_CTYPE
my %category_name;
my %category_number;
if ($has_locale_h) {
    my $number_for_missing_category = $max_bad_category_number;
    foreach my $name (@known_categories) {
        my $number = eval "&POSIX::$name";
        if ($@) {
            # Use a negative number (smaller than any legitimate category
            # number) if the platform doesn't support this category, so we
            # have an entry for all the ones that might be specified in calls
            # to us.
            $number = $number_for_missing_category--;
        }
        elsif (   $number !~ / ^ -? \d+ $ /x
               || $number <=  $max_bad_category_number)
        {
            # We think this should be an int.  And it has to be larger than
            # any of our synthetic numbers.
            die "Unexpected locale category number '$number' for $name"
        }
        else {
            push @platform_categories, $name;
        }

        $name =~ s/LC_//;
        $category_name{$number} = "$name";
        $category_number{$name} = $number;
    }
}

sub _my_diag($) {
    my $message = shift;
    if (defined &main::diag) {
        diag($message);
    }
    else {
        local($\, $", $,) = (undef, ' ', '');
        print STDERR $message, "\n";
    }
}

# Larger than any real test
my $my_count = 1_000_000;

sub _my_fail($) {
    my $message = shift;
    if (defined &main::fail) {
        fail($message);
    }
    else {
        local($\, $", $,) = (undef, ' ', '');
        print "not ok " . $my_count++ . $message . "\n";
    }
}

sub valid_locale_categories() {
    # Returns a list of the locale categories (expressed as strings, like
    # "LC_ALL) known to this program that are available on this platform.

    return grep { ! category_excluded($_) } @platform_categories;
}

sub is_category_valid($) {
    my $name = shift;
    $name = 'LC_' . $name =~ s/^LC_//r;
    return grep { $name eq $_ } valid_locale_categories();
}

# It turns out that strings generated under the control of a given locale
# category are often affected as well by LC_CTYPE.  If the two categories
# don't match, one can get mojibake or even core dumps.  (khw thinks it more
# likely that it's the code set, not the locale that's critical here; but
# didn't run experiments to verify this.)  Hence, in the code below, CTYPE and
# the tested categories are all set to the same locale.  If CTYPE isn't
# available on the platform, LC_ALL is instead used.  One might think to just
# use LC_ALL all the time, but on Windows
#    setlocale(LC_ALL, "some_borked_locale")
# can return success, whereas setting LC_CTYPE to it fails.
my $master_category;
$master_category = $category_number{'CTYPE'}
        if is_category_valid('LC_CTYPE') && defined $category_number{'CTYPE'};
$master_category = $category_number{'ALL'}
        if ! defined $master_category
          && is_category_valid('LC_ALL') && defined $category_number{'ALL'};

sub _trylocale ($$$$) { # For use only by other functions in this file!

    # Adds the locale given by the first parameter to the list given by the
    # 3rd iff the platform supports the locale in each of the category numbers
    # given by the 2nd parameter, which is either a single category or a
    # reference to a list of categories.
    #
    # The 4th parameter is true if to accept locales that aren't apparently
    # fully compatible with Perl.

    my $locale = shift;
    my $categories = shift;
    my $list = shift;
    my $allow_incompatible = shift;

    my $normalized_locale = lc ($locale =~ s/\W//gr);
    return if ! $locale || grep { $normalized_locale eq lc ($_ =~ s/\W//gr) } @$list;

    # This is a toy (pig latin) locale that is not fully implemented on some
    # systems
    return if $locale =~ / ^ pig $ /ix;

    # Certain platforms have a crippled locale system in which setlocale
    # returns success for just about any possible locale name, but if anything
    # actually happens as a result of the call, it is that the underlying
    # locale is set to a system default, likely C or C.UTF-8.  We can't test
    # such systems fully, but we shouldn't disable the user from using
    # locales, as it may work out for them (or not).
    return if    defined $Config{d_setlocale_accepts_any_locale_name}
              && $locale !~ / ^ (?: C | POSIX | C\.UTF-?8 ) $/ix;

    if (exists $known_bad_locales{$^O}) {
        my @bad_locales = $known_bad_locales{$^O}->@*;
        return if grep { $locale eq $_ } @bad_locales;
    }

    $categories = [ $categories ] unless ref $categories;

    my $badutf8 = 0;
    my $plays_well = 1;
    my $unsupported = 0;

    use warnings 'locale';

    local $SIG{__WARN__} = sub {
        $badutf8 = 1 if grep { /Malformed UTF-8/ } @_;
        $unsupported = 1 if grep { /Locale .* is unsupported/i } @_;
        $plays_well = 0 if grep {
                    /The following characters .* may not have the same meaning as the Perl program expects(?#
                   )|The Perl program will use the expected meanings/i
            } @_;
    };

    my $first_time = 1;
    foreach my $category ($master_category, $categories->@*) {
        next if ! defined $category || (! $first_time && $category == $master_category);
        $first_time = 0;

        my $save_locale = setlocale($category);
        if (! $save_locale) {
            _my_fail("Verify could save previous locale");
            return;
        }

        # Incompatible locales aren't warned about unless using locales.
        use locale;

        my $result = setlocale($category, $locale);
        return unless defined $result;

        no locale;

        # We definitely don't want the locale set to something that is
        # unsupported
        if (! setlocale($category, $save_locale)) {
            my $error_text = "\$!=$!";
            $error_text .= "; \$^E=$^E" if $^E != $!;
            die "Couldn't restore locale '$save_locale', category $category;"
              . $error_text;
        }
        if ($badutf8) {
            _my_fail("Verify locale name doesn't contain malformed utf8");
            return;
        }

        return if $unsupported;

        # Commas in locale names are bad in Windows, and there is a bug in
        # some versions where setlocale() turns a legal input locale name into
        # an illegal return value, which it can't later parse.
        return if $result =~ /,/;

        return unless $plays_well || $allow_incompatible;
    }

    push @$list, $locale;
}

sub _decode_encodings { # For use only by other functions in this file!
    my @enc;

    foreach (split(/ /, shift)) {
	if (/^(\d+)$/) {
	    push @enc, "ISO8859-$1";
	    push @enc, "iso8859$1";	# HP
	    if ($1 eq '1') {
		 push @enc, "roman8";	# HP
	    }
	    push @enc, $_;
            push @enc, "$_.UTF-8";
            push @enc, "$_.65001"; # Windows UTF-8
            push @enc, "$_.ACP"; # Windows ANSI code page
            push @enc, "$_.OCP"; # Windows OEM code page
            push @enc, "$_.1252"; # Windows
	}
    }
    if ($^O eq 'os390') {
	push @enc, qw(IBM-037 IBM-819 IBM-1047);
    }
    push @enc, "UTF-8";
    push @enc, "65001"; # Windows UTF-8

    return @enc;
}

sub locales_enabled(;$) {
    # If no parameter is specified, the function returns 1 if there is any
    # "safe" locale handling available to the caller; otherwise 0.  Safeness
    # is defined here as the caller operating in the main thread of a program,
    # or if threaded locales are safe on the platform and Configured to be
    # used.  This sub is used for testing purposes, and for those, this
    # definition of safety is sufficient, and necessary to get some tests to
    # run on certain configurations on certain platforms.  But beware that the
    # main thread can change the locale of any subthreads unless
    # ${^SAFE_LOCALES} is non-zero.
    #
    # Use the optional parameter to discover if a particular category or
    # categories are available on the system.  1 is returned if the global
    # criteria described in the previous paragraph are true, AND if all the
    # specified categories are available on the platform and Configured to be
    # used.  Otherwise 0 is returned.  The parameter is either a single POSIX
    # locale category or a reference to a list of them.  Each category must be
    # its name as a string, like 'LC_TIME' (the initial 'LC_' is optional), or
    # the number this platform uses to signify the category (e.g.,
    # 'locales_enabled(&POSIX::LC_CTYPE)'
    #
    # When the function returns 1 and a parameter was specified as a list
    # reference, the reference will be altered on return to point to an
    # equivalent list such that  the categories are numeric instead of strings
    # and sorted to meet the input expectations of _trylocale().
    #
    # It is a fatal error to call this with something that isn't a known
    # category to this file.  If this happens, look first for a typo, and
    # second if you are using a category unknown to Perl.  In the latter case
    # a bug report should be submitted.

    # khw cargo-culted the '?' in the pattern on the next line.
    return 0 if $Config{ccflags} =~ /\bD?NO_LOCALE\b/;

    # If we can't load the POSIX XS module, we can't have locales even if they
    # normally would be available
    return 0 if ! defined &DynaLoader::boot_DynaLoader;

    # Don't test locales where they aren't safe.  On systems with unsafe
    # threads, for the purposes of testing, we consider the main thread safe,
    # and all other threads unsafe.
    if (! ${^SAFE_LOCALES}) {
        return 0 if $^O eq 'os390'; # Threaded locales don't work well here
        require threads;
        return 0 if threads->tid() != 0;
    }

    # If no setlocale, we need the POSIX 2008 alternatives
    if (! $Config{d_setlocale}) {
        return 0 if $Config{ccflags} =~ /\bD?NO_POSIX_2008_LOCALE\b/;
        return 0 unless $Config{d_newlocale};
        return 0 unless $Config{d_uselocale};
        return 0 unless $Config{d_duplocale};
        return 0 unless $Config{d_freelocale};
    }

    # Done with the global possibilities.  Now check if any passed in category
    # is disabled.

    my $categories_ref = $_[0];
    my $return_categories_numbers = 0;
    my @categories_numbers;
    my $has_LC_ALL = 0;
    my $has_LC_COLLATE = 0;

    if (defined $categories_ref) {
        my @local_categories_copy;

        my $reftype = ref $categories_ref;
        if ($reftype eq 'ARRAY') {
            @local_categories_copy = @$categories_ref;
            $return_categories_numbers = 1;
        }
        elsif ($reftype ne "") {
            die "Parameter to locales_enabled() must be an ARRAY;"
              . " instead you used a $reftype";
        }
        else {  # Single category passed in
            @local_categories_copy = $categories_ref;
        }

        for my $category_name_or_number (@local_categories_copy) {
            my $name;
            my $number;
            if ($category_name_or_number =~ / ^ -? \d+ $ /x) {
                $number = $category_name_or_number;
                die "Invalid locale category number '$number'"
                    unless grep { $number == $_ } keys %category_name;
                $name = $category_name{$number};
            }
            else {
                $name = $category_name_or_number;
                $name =~ s/ ^ LC_ //x;
                foreach my $trial (keys %category_name) {
                    if ($category_name{$trial} eq $name) {
                        $number = $trial;
                        last;
                    }
                }
                die "Invalid locale category name '$name'"
                    unless defined $number;
            }

            return 0 if   $number <= $max_bad_category_number
                       || category_excluded($name);


            eval "defined &POSIX::LC_$name";
            return 0 if $@;

            if ($return_categories_numbers) {
                if ($name eq 'CTYPE') {
                    unshift @categories_numbers, $number;   # Always first
                }
                elsif ($name eq 'ALL') {
                    $has_LC_ALL = 1;
                }
                elsif ($name eq 'COLLATE') {
                    $has_LC_COLLATE = 1;
                }
                else {
                    push @categories_numbers, $number;
                }
            }
        }
    }

    if ($return_categories_numbers) {

        # COLLATE comes after all other locales except ALL, which comes last
        if ($has_LC_COLLATE) {
            push @categories_numbers, $category_number{'COLLATE'};
        }
        if ($has_LC_ALL) {
            push @categories_numbers, $category_number{'ALL'};
        }

        @$categories_ref = @categories_numbers;
    }

    return 1;
}


sub find_locales ($;$) {

    # Returns an array of all the locales we found on the system.  If the
    # optional 2nd parameter is non-zero, the list includes all found locales;
    # otherwise it is restricted to those locales that play well with Perl, as
    # far as we can easily determine.
    #
    # The first parameter is either a single locale category or a reference to
    # a list of categories to find valid locales for it (or in the case of
    # multiple) for all of them.  Each category can be a name (like 'LC_ALL'
    # or simply 'ALL') or the C enum value for the category.

    my $input_categories = shift;
    my $allow_incompatible = shift // 0;

    my @categories = (ref $input_categories)
                      ? $input_categories->@*
                      : $input_categories;
    return unless locales_enabled(\@categories);

    # Note, the subroutine call above converts the $categories into a form
    # suitable for _trylocale().

    # Visual C's CRT goes silly on strings of the form "en_US.ISO8859-1"
    # and mingw32 uses said silly CRT
    # This doesn't seem to be an issue any more, at least on Windows XP,
    # so re-enable the tests for Windows XP onwards.
    my $winxp = ($^O eq 'MSWin32' && defined &Win32::GetOSVersion &&
                    join('.', (Win32::GetOSVersion())[1..2]) >= 5.1);
    return if (($^O eq 'MSWin32' && !$winxp)
                && $Config{cc} =~ /^(cl|gcc|g\+\+|ici)/i);

    my @Locale;
    _trylocale("C", \@categories, \@Locale, $allow_incompatible);
    _trylocale("POSIX", \@categories, \@Locale, $allow_incompatible);

    if ($Config{d_has_C_UTF8} && $Config{d_has_C_UTF8} eq 'true') {
        _trylocale("C.UTF-8", \@categories, \@Locale, $allow_incompatible);
    }

    # There's no point in looking at anything more if we know that setlocale
    # will return success on any garbage or non-garbage name.
    return sort @Locale if defined $Config{d_setlocale_accepts_any_locale_name};

    foreach (1..16) {
        _trylocale("ISO8859-$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("iso8859$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("iso8859-$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("iso_8859_$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("isolatin$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("isolatin-$_", \@categories, \@Locale, $allow_incompatible);
        _trylocale("iso_latin_$_", \@categories, \@Locale, $allow_incompatible);
    }

    # Sanitize the environment so that we can run the external 'locale'
    # program without the taint mode getting grumpy.

    # $ENV{PATH} is special in VMS.
    delete local $ENV{PATH} if $^O ne 'VMS' or $Config{d_setenv};

    # Other subversive stuff.
    delete local @ENV{qw(IFS CDPATH ENV BASH_ENV)};

    if (-x "/usr/bin/locale"
        && open(LOCALES, '-|', "/usr/bin/locale -a 2>/dev/null"))
    {
        while (<LOCALES>) {
            # It seems that /usr/bin/locale steadfastly outputs 8 bit data, which
            # ain't great when we're running this testPERL_UNICODE= so that utf8
            # locales will cause all IO hadles to default to (assume) utf8
            next unless utf8::valid($_);
            chomp;
            _trylocale($_, \@categories, \@Locale, $allow_incompatible);
        }
        close(LOCALES);
    } elsif ($^O eq 'VMS'
             && defined($ENV{'SYS$I18N_LOCALE'})
             && -d 'SYS$I18N_LOCALE')
    {
    # The SYS$I18N_LOCALE logical name search list was not present on
    # VAX VMS V5.5-12, but was on AXP && VAX VMS V6.2 as well as later versions.
        opendir(LOCALES, "SYS\$I18N_LOCALE:");
        while ($_ = readdir(LOCALES)) {
            chomp;
            _trylocale($_, \@categories, \@Locale, $allow_incompatible);
        }
        close(LOCALES);
    } elsif (($^O eq 'openbsd' || $^O eq 'bitrig' ) && -e '/usr/share/locale') {

        # OpenBSD doesn't have a locale executable, so reading
        # /usr/share/locale is much easier and faster than the last resort
        # method.

        opendir(LOCALES, '/usr/share/locale');
        while ($_ = readdir(LOCALES)) {
            chomp;
            _trylocale($_, \@categories, \@Locale, $allow_incompatible);
        }
        close(LOCALES);
    } else { # Final fallback.  Try our list of locales hard-coded here

        # This is going to be slow.
        my @Data;

        # Locales whose name differs if the utf8 bit is on are stored in these
        # two files with appropriate encodings.
        my $data_file = ($^H & 0x08 || (${^OPEN} || "") =~ /:utf8/)
                        ? _source_location() . "/lib/locale/utf8"
                        : _source_location() . "/lib/locale/latin1";
        if (-e $data_file) {
            @Data = do $data_file;
        }
        else {
            _my_diag(__FILE__ . ":" . __LINE__ . ": '$data_file' doesn't exist");
        }

        # The rest of the locales are in this file.
        state @my_data = <DATA>; close DATA if fileno DATA;
        push @Data, @my_data;

        foreach my $line (@Data) {
            chomp $line;
            my ($locale_name, $language_codes, $country_codes, $encodings) =
                split /:/, $line;
            _my_diag(__FILE__ . ":" . __LINE__ . ": Unexpected syntax in '$line'")
                                                     unless defined $locale_name;
            my @enc = _decode_encodings($encodings);
            foreach my $loc (split(/ /, $locale_name)) {
                _trylocale($loc, \@categories, \@Locale, $allow_incompatible);
                foreach my $enc (@enc) {
                    _trylocale("$loc.$enc", \@categories, \@Locale,
                                                            $allow_incompatible);
                }
                $loc = lc $loc;
                foreach my $enc (@enc) {
                    _trylocale("$loc.$enc", \@categories, \@Locale,
                                                            $allow_incompatible);
                }
            }
            foreach my $lang (split(/ /, $language_codes)) {
                _trylocale($lang, \@categories, \@Locale, $allow_incompatible);
                foreach my $country (split(/ /, $country_codes)) {
                    my $lc = "${lang}_${country}";
                    _trylocale($lc, \@categories, \@Locale, $allow_incompatible);
                    foreach my $enc (@enc) {
                        _trylocale("$lc.$enc", \@categories, \@Locale,
                                                            $allow_incompatible);
                    }
                    my $lC = "${lang}_\U${country}";
                    _trylocale($lC, \@categories, \@Locale, $allow_incompatible);
                    foreach my $enc (@enc) {
                        _trylocale("$lC.$enc", \@categories, \@Locale,
                                                            $allow_incompatible);
                    }
                }
            }
        }
    }

    @Locale = sort @Locale;

    return @Locale;
}

sub is_locale_utf8 ($) { # Return a boolean as to if core Perl thinks the input
                         # is a UTF-8 locale

    # On z/OS, even locales marked as UTF-8 aren't.
    return 0 if ord "A" != 65;

    return 0 unless locales_enabled('LC_CTYPE');

    my $locale = shift;

    no warnings 'locale'; # We may be trying out a weird locale
    use locale;

    my $save_locale = setlocale(&POSIX::LC_CTYPE());
    if (! $save_locale) {
        _my_fail("Verify could save previous locale");
        return 0;
    }

    if (! setlocale(&POSIX::LC_CTYPE(), $locale)) {
        _my_fail("Verify could setlocale to $locale");
        return 0;
    }

    my $ret = 0;

    # Use an op that gives different results for UTF-8 than any other locale.
    # If a platform has UTF-8 locales, there should be at least one locale on
    # most platforms with UTF-8 in its name, so if there is a bug in the op
    # giving a false negative, we should get a failure for those locales as we
    # go through testing all the locales on the platform.
    if (CORE::fc(chr utf8::unicode_to_native(0xdf)) ne "ss") {
        if ($locale =~ /UTF-?8/i) {
            _my_fail("Verify $locale with UTF-8 in name is a UTF-8 locale");
        }
    }
    else {
        $ret = 1;
    }

    die "Couldn't restore locale '$save_locale'"
                            unless setlocale(&POSIX::LC_CTYPE(), $save_locale);

    return $ret;
}

sub classify_locales_wrt_utf8ness($) {

    # Takes the input list of locales, and returns two lists split apart from
    # it: the UTF-8 ones, and the non-UTF-8 ones.

    my $locales_ref = shift;
    my (@utf8, @non_utf8);

    if (! locales_enabled('LC_CTYPE')) {  # No CTYPE implies all are non-UTF-8
        @non_utf8 = $locales_ref->@*;
        return ( \@utf8, \@non_utf8 );
    }

    foreach my $locale (@$locales_ref) {
        my $which = (is_locale_utf8($locale)) ? \@utf8 : \@non_utf8;
        push $which->@*, $locale;
    }

    return ( \@utf8, \@non_utf8 );
}

sub find_utf8_ctype_locales (;$) {

    # Return the names of the locales that core Perl thinks are UTF-8 LC_CTYPE
    # locales.  Optional parameter is a reference to a list of locales to try;
    # if omitted, this tries all locales it can find on the platform

    return unless locales_enabled('LC_CTYPE');

    my $locales_ref = shift;
    if (! defined $locales_ref) {

        my @locales = find_locales(&POSIX::LC_CTYPE());
        $locales_ref = \@locales;
    }

    my ($utf8_ref, undef) = classify_locales_wrt_utf8ness($locales_ref);
    return unless $utf8_ref;
    return $utf8_ref->@*;
}

sub find_utf8_ctype_locale (;$) { # Return the name of a locale that core Perl
                                  # thinks is a UTF-8 LC_CTYPE non-turkic
                                  # locale.
                                  # Optional parameter is a reference to a
                                  # list of locales to try; if omitted, this
                                  # tries all locales it can find on the
                                  # platform
    my $try_locales_ref = shift;

    my @utf8_locales = find_utf8_ctype_locales($try_locales_ref);
    my @turkic_locales = find_utf8_turkic_locales($try_locales_ref);

    my %seen_turkic;

    # Create undef elements in the hash for turkic locales
    @seen_turkic{@turkic_locales} = ();

    foreach my $locale (@utf8_locales) {
        return $locale unless exists $seen_turkic{$locale};
    }

    return;
}

sub find_utf8_turkic_locales (;$) {

    # Return the name of all the locales that core Perl thinks are UTF-8
    # Turkic LC_CTYPE.  Optional parameter is a reference to a list of locales
    # to try; if omitted, this tries all locales it can find on the platform

    my @return;

    return unless locales_enabled('LC_CTYPE');

    my $save_locale = setlocale(&POSIX::LC_CTYPE());
    foreach my $locale (find_utf8_ctype_locales(shift)) {
        use locale;
        setlocale(&POSIX::LC_CTYPE(), $locale);
        push @return, $locale if uc('i') eq "\x{130}";
    }

    die "Couldn't restore locale '$save_locale'"
                            unless setlocale(&POSIX::LC_CTYPE(), $save_locale);

    return @return;
}

sub find_utf8_turkic_locale (;$) {
    my @turkics = find_utf8_turkic_locales(shift);

    return unless @turkics;
    return $turkics[0]
}


# returns full path to the directory containing the current source
# file, inspired by mauke's Dir::Self
sub _source_location {
    require File::Spec;

    my $caller_filename = (caller)[1];

    my $loc = File::Spec->rel2abs(
        File::Spec->catpath(
            (File::Spec->splitpath($caller_filename))[0, 1], ''
        )
    );

    return ($loc =~ /^(.*)$/)[0]; # untaint
}

1

# Format of data is: locale_name, language_codes, country_codes, encodings
__DATA__
Afrikaans:af:za:1 15
Arabic:ar:dz eg sa:6 arabic8
Brezhoneg Breton:br:fr:1 15
Bulgarski Bulgarian:bg:bg:5
Chinese:zh:cn tw:cn.EUC eucCN eucTW euc.CN euc.TW Big5 GB2312 tw.EUC
Hrvatski Croatian:hr:hr:2
Cymraeg Welsh:cy:cy:1 14 15
Czech:cs:cz:2
Dansk Danish:da:dk:1 15
Nederlands Dutch:nl:be nl:1 15
English American British:en:au ca gb ie nz us uk zw:1 15 cp850
Esperanto:eo:eo:3
Eesti Estonian:et:ee:4 6 13
Suomi Finnish:fi:fi:1 15
Flamish::fl:1 15
Deutsch German:de:at be ch de lu:1 15
Euskaraz Basque:eu:es fr:1 15
Galego Galician:gl:es:1 15
Ellada Greek:el:gr:7 g8
Frysk:fy:nl:1 15
Greenlandic:kl:gl:4 6
Hebrew:iw:il:8 hebrew8
Hungarian:hu:hu:2
Indonesian:id:id:1 15
Gaeilge Irish:ga:IE:1 14 15
Italiano Italian:it:ch it:1 15
Nihongo Japanese:ja:jp:euc eucJP jp.EUC sjis
Korean:ko:kr:
Latine Latin:la:va:1 15
Latvian:lv:lv:4 6 13
Lithuanian:lt:lt:4 6 13
Macedonian:mk:mk:1 15
Maltese:mt:mt:3
Moldovan:mo:mo:2
Norsk Norwegian:no no\@nynorsk nb nn:no:1 15
Occitan:oc:es:1 15
Polski Polish:pl:pl:2
Rumanian:ro:ro:2
Russki Russian:ru:ru su ua:5 koi8 koi8r KOI8-R koi8u cp1251 cp866
Serbski Serbian:sr:yu:5
Slovak:sk:sk:2
Slovene Slovenian:sl:si:2
Sqhip Albanian:sq:sq:1 15
Svenska Swedish:sv:fi se:1 15
Thai:th:th:11 tis620
Turkish:tr:tr:9 turkish8
Yiddish:yi::1 15
