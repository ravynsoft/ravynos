#!./perl
BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';    # for fresh_perl_is() etc
    require './loc_tools.pl'; # to find locales
}

use strict;
use warnings;

########
# These tests are here instead of lib/locale.t because
# some bugs depend on the internal state of the locale
# settings and pragma/locale messes up that state pretty badly.
# We need "fresh runs".
BEGIN {
    eval { require POSIX; POSIX->import("locale_h") };
    if ($@) {
	skip_all("could not load the POSIX module"); # running minitest?
    }
}
use Config;

if ($^O eq "aix" && ($Config{osvers} =~ /^(\d+)/)[0] < 7) {
    # https://www.ibm.com/support/pages/apar/IV22174
    skip_all("old AIX setlocale is broken in some cases");
}

use I18N::Langinfo qw(langinfo RADIXCHAR);
my $have_strtod = $Config{d_strtod} eq 'define';
my $have_localeconv = defined $Config{d_locconv} && $Config{d_locconv} eq 'define';
my @locales = find_locales( [ 'LC_ALL', 'LC_CTYPE', 'LC_NUMERIC' ]);
skip_all("no locales available") unless @locales;
note("locales available: @locales");

my $debug = 0;
my $switches = "";
if (defined $ARGV[0] && $ARGV[0] ne "") {
    if ($ARGV[0] ne 'debug') {
        print STDERR "Usage: $0 [ debug ]\n";
        exit 1
    }
    $debug = 1;
}
$switches = "switches => [ '-DLv' ]" if $debug;

# reset the locale environment
delete local @ENV{'LANGUAGE', 'LANG', (grep /^LC_[A-Z]+$/, keys %ENV)};

# If user wants this to happen, they set the environment variable AND use
# 'debug'
delete local $ENV{'PERL_DEBUG_LOCALE_INIT'} unless $debug;

{
    fresh_perl_is(<<"EOF",
            use locale;
            use POSIX;
            POSIX::setlocale(POSIX::LC_CTYPE(),"C");
            print "h" =~ /[g\\w]/i || 0;
            print "\\n";
EOF
        1, { stderr => 'devnull' }, "/il matching of [bracketed] doesn't skip POSIX class if fails individ char");
}

{
    fresh_perl_is(<<"EOF",
            use locale;
            use POSIX;
            POSIX::setlocale(POSIX::LC_CTYPE(),"C");
            print "0" =~ /[\\d[:punct:]]/l || 0;
            print "\\n";
EOF
        1, { stderr => 'devnull' }, "/l matching of [bracketed] doesn't skip non-first POSIX class");

}

my $non_C_locale;
foreach my $locale (@locales) {
    next if $locale eq "C" || $locale eq 'POSIX' || $locale eq "C.UTF-8";
    $non_C_locale = $locale;
    last;
}

if ($non_C_locale) {
    note("using non-C locale '$non_C_locale'");
    setlocale(LC_NUMERIC, $non_C_locale);
    isnt(setlocale(LC_NUMERIC), "C", "retrieving current non-C LC_NUMERIC doesn't give 'C'");
    setlocale(LC_ALL, $non_C_locale);
    isnt(setlocale(LC_ALL), "C", "retrieving current non-C LC_ALL doesn't give 'C'");

    my @test_numeric_locales = @locales;

    # Skip this locale on these cygwin versions as the returned radix character
    # length is wrong
    if (   $^O eq 'cygwin'
        && version->new(($Config{'osvers'} =~ /^(\d+(?:\.\d+)+)/)[0]) le v2.4.1)
    {
        @test_numeric_locales = grep { $_ !~ m/ps_AF/i } @test_numeric_locales;
    }

    # Similarly the arabic locales on solaris don't work right on the
    # multi-byte radix character, generating malformed UTF-8.
    if ($^O eq 'solaris') {
        @test_numeric_locales = grep { $_ !~ m/ ^ ( ar_ | pa_ ) /x }
                                                        @test_numeric_locales;
    }

    fresh_perl_is("for (qw(@test_numeric_locales)) {\n" . <<'EOF',
        use POSIX qw(locale_h);
        use locale;
        setlocale(LC_NUMERIC, "$_") or next;
        my $s = sprintf "%g %g", 3.1, 3.1;
        next if $s eq '3.1 3.1' || $s =~ /^(3.+1) \1$/;
        no warnings "utf8";
        print "$_ $s\n";
    }
EOF
        "", { eval $switches }, "no locales where LC_NUMERIC breaks");

    SKIP: {
        skip("Windows stores locale defaults in the registry", 1 )
                                                                if $^O eq 'MSWin32';
        fresh_perl_is("for (qw(@locales)) {\n" . <<'EOF',
            use POSIX qw(locale_h);
            use locale;
            my $in = 4.2;
            my $s = sprintf "%g", $in; # avoid any constant folding bugs
            next if $s eq "4.2";
            no warnings "utf8";
            print "$_ $s\n";
        }
EOF
        "", { eval $switches }, "LC_NUMERIC without environment nor setlocale() has no effect in any locale");
    }

    # try to find out a locale where LC_NUMERIC makes a difference
    my $original_locale = setlocale(LC_NUMERIC);

    my ($base, $different, $comma, $difference, $utf8_radix);
    my $radix_encoded_as_utf8;
    for ("C", @locales) { # prefer C for the base if available
        use locale;
        setlocale(LC_NUMERIC, $_) or next;
        my $in = 4.2; # avoid any constant folding bugs
        if ((my $s = sprintf("%g", $in)) eq "4.2")  {
            $base ||= $_;
        } else {
            $different ||= $_;
            $difference ||= $s;
            my $radix = langinfo(RADIXCHAR);

            # For utf8 locales with a non-ascii radix, it should be encoded as
            # UTF-8 with the internal flag so set.
            if (! defined $utf8_radix
                && $radix =~ /[[:^ascii:]]/u  # /u because /l can raise warnings
                && is_locale_utf8($_))
            {
                $utf8_radix = $_;
                $radix_encoded_as_utf8 = utf8::is_utf8($radix);
            }
            else {
                $comma ||= $_ if $radix eq ',';
            }
        }

        last if $base && $different && $comma && $utf8_radix;
    }
    setlocale(LC_NUMERIC, $original_locale);

    SKIP: {
        skip("no UTF-8 locale available where LC_NUMERIC radix isn't ASCII", 1 )
            unless $utf8_radix;
        is($radix_encoded_as_utf8, 1, "UTF-8 locale '$utf8_radix' with non-ASCII"
                                    . " radix is marked UTF-8");
    }

    SKIP: {
        skip("no locale available where LC_NUMERIC radix isn't '.'", 30) unless $different;
        note("using the '$different' locale for LC_NUMERIC tests");
        {
            local $ENV{LC_NUMERIC} = $different;

            fresh_perl_is(<<'EOF', "4.2", { eval $switches },
    format STDOUT =
@.#
4.179
.
    write;
EOF
                "format() does not look at LC_NUMERIC without 'use locale'");

            {
                fresh_perl_is(<<'EOF', "$difference\n", { eval $switches },
                use POSIX;
                use locale;
                format STDOUT =
@.#
4.179
.
    write;
EOF
                "format() looks at LC_NUMERIC with 'use locale'");
            }

      SKIP: {
                unless ($have_localeconv) {
                    skip("no localeconv()", 1);
                }
                else {
                    fresh_perl_is(<<'EOF', ",,", { eval $switches },
    use POSIX;
    no warnings "utf8";
    print localeconv()->{decimal_point};
    use locale;
    print localeconv()->{decimal_point};
EOF
                "localeconv() looks at LC_NUMERIC with and without 'use locale'");
                }
            }

            {
                my $categories = ":collate :characters :collate :ctype :monetary :time";
                fresh_perl_is(<<"EOF", "4.2", { eval $switches },
    use locale qw($categories);
    format STDOUT =
@.#
4.179
.
    write;
EOF
                "format() does not look at LC_NUMERIC with 'use locale qw($categories)'");
            }

            {
                fresh_perl_is(<<'EOF', $difference, { eval $switches },
    use locale;
    format STDOUT =
@.#
4.179
.
    write;
EOF
                "format() looks at LC_NUMERIC with 'use locale'");
            }

            for my $category (qw(collate characters collate ctype monetary time)) {
                for my $negation ("!", "not_") {
                    fresh_perl_is(<<"EOF", $difference, { eval $switches },
    use locale ":$negation$category";
format STDOUT =
@.#
4.179
.
    write;
EOF
                    "format() looks at LC_NUMERIC with 'use locale \":"
                    . "$negation$category\"'");
                }
            }

            {
                fresh_perl_is(<<'EOF', $difference, { eval $switches },
    use locale ":numeric";
format STDOUT =
@.#
4.179
.
    write;
EOF
                "format() looks at LC_NUMERIC with 'use locale \":numeric\"'");
            }

            {
                fresh_perl_is(<<'EOF', "4.2", { eval $switches },
format STDOUT =
@.#
4.179
.
    { use locale; write; }
EOF
                "too late to look at the locale at write() time");
            }

            {
                fresh_perl_is(<<'EOF', $difference, { eval $switches },
    use locale;
    format STDOUT =
@.#
4.179
.
    { no locale; write; }
EOF
                "too late to ignore the locale at write() time");
            }
        }

        {
            # do not let "use 5.000" affect the locale!
            # this test is to prevent regression of [rt.perl.org #105784]
            fresh_perl_is(<<"EOF",
                use locale;
                use POSIX;
                my \$i = 0.123;
                POSIX::setlocale(POSIX::LC_NUMERIC(),"$different");
                \$a = sprintf("%.2f", \$i);
                require version;
                \$b = sprintf("%.2f", \$i);
                no warnings "utf8";
                print ".\$a \$b" unless \$a eq \$b
EOF
                "", { eval $switches }, "version does not clobber version");

            fresh_perl_is(<<"EOF",
                use locale;
                use POSIX;
                my \$i = 0.123;
                POSIX::setlocale(POSIX::LC_NUMERIC(),"$different");
                \$a = sprintf("%.2f", \$i);
                eval "use v5.0.0";
                \$b = sprintf("%.2f", \$i);
                no warnings "utf8";
                print "\$a \$b" unless \$a eq \$b
EOF
                "", { eval $switches }, "version does not clobber version (via eval)");
        }

        {
            local $ENV{LC_NUMERIC} = $different;
            fresh_perl_is(<<'EOF', "$difference "x4, { eval $switches },
                use locale;
                use POSIX qw(locale_h);
                my $in = 4.2;
                printf("%g %g %s %s ", $in, 4.2, sprintf("%g", $in), sprintf("%g", 4.2));
EOF
            "sprintf() and printf() look at LC_NUMERIC regardless of constant folding");
        }

        {
            local $ENV{LC_NUMERIC} = $different;
            fresh_perl_is(<<'EOF', "$difference "x4, { eval $switches },
                use locale;
                use POSIX qw(locale_h);
                my $in = 4.2;
                printf("%g %g %s %s ", $in, 4.2, sprintf("%g", $in), sprintf("%g", 4.2));
EOF
            "Uses the above test to verify that on Windows the system default locale has lower priority than LC_NUMERIC");
        }


        # within this block, STDERR is closed. This is because fresh_perl_is()
        # forks a shell, and some shells (like bash) can complain noisily when
        # LC_ALL or similar is set to an invalid value

        {
            open my $saved_stderr, ">&STDERR" or die "Can't dup STDERR: $!";
            close STDERR;

            {
                local $ENV{LC_ALL} = "invalid";
                local $ENV{LC_NUMERIC} = "invalid";
                local $ENV{LANG} = $different;
                local $ENV{PERL_BADLANG} = 0;

                if (! fresh_perl_is(<<"EOF", "$difference", { eval $switches  },
                    if (\$ENV{LC_ALL} ne "invalid") {
                        # Make the test pass if the sh didn't accept the ENV set
                        no warnings "utf8";
                        print "$difference\n";
                        exit 0;
                    }
                    use locale;
                    use POSIX qw(locale_h);
                    my \$in = 4.2;
                    printf("%g", \$in);
EOF
                "LANG is used if LC_ALL, LC_NUMERIC are invalid"))
            {
                note "To see details change this .t, do not close STDERR";
            }
            }

            SKIP: {
                if ($^O eq 'MSWin32') {
                    skip("Win32 uses system default locale in preference to \"C\"",
                            1);
                }
                else {
                    local $ENV{LC_ALL} = "invalid";
                    local $ENV{LC_NUMERIC} = "invalid";
                    local $ENV{LANG} = "invalid";
                    local $ENV{PERL_BADLANG} = 0;

                    if (! fresh_perl_is(<<"EOF", 4.2, { eval $switches  },
                        if (\$ENV{LC_ALL} ne "invalid") {
                            no warnings "utf8";
                            print "$difference\n";
                            exit 0;
                        }
                        use locale;
                        use POSIX qw(locale_h);
                        my \$in = 4.2;
                        printf("%g", \$in);
EOF
                    'C locale is used if LC_ALL, LC_NUMERIC, LANG are invalid'))
                    {
                        note "To see details change this .t, do not close STDERR";
                    }
                }
            }

        open STDERR, ">&", $saved_stderr or die "Can't dup \$saved_stderr: $!";
        }

        {
            local $ENV{LC_NUMERIC} = $different;
            fresh_perl_is(<<"EOF",
                use POSIX qw(locale_h);

                BEGIN { setlocale(LC_NUMERIC, \"$different\"); };
                setlocale(LC_ALL, "C");
                use 5.008;
                print setlocale(LC_NUMERIC);
EOF
            "C", { stderr => 'devnull' },
            "No compile error on v-strings when setting the locale to non-dot radix at compile time when default environment has non-dot radix");
        }

        unless ($comma) {
            skip("no locale available where LC_NUMERIC is a comma", 3);
        }
        else {

            fresh_perl_is(<<"EOF",
                my \$i = 1.5;
                {
                    use locale;
                    use POSIX;
                    POSIX::setlocale(POSIX::LC_NUMERIC(),"$comma");
                    print \$i, "\n";
                }
                print \$i, "\n";
EOF
                "1,5\n1.5", { stderr => 'devnull' }, "Radix print properly in locale scope, and without");

            fresh_perl_is(<<"EOF",
                my \$i = 1.5;   # Should be exactly representable as a base 2
                                # fraction, so can use 'eq' below
                use locale;
                use POSIX;
                POSIX::setlocale(POSIX::LC_NUMERIC(),"$comma");
                print \$i, "\n";
                \$i += 1;
                print \$i, "\n";
EOF
                "1,5\n2,5", { stderr => 'devnull' }, "Can do math when radix is a comma"); # [perl 115800]

            SKIP: {
                skip "Perl not compiled with 'useithreads'", 1 if ! $Config{'useithreads'};

                local $ENV{LC_ALL} = undef;
                local $ENV{LC_NUMERIC} = $comma;
                fresh_perl_is(<<"EOF",
                    use threads;

                    my \$x = eval "1.25";
                    print "\$x", "\n";  # number is ok before thread
                    my \$str_x = "\$x";

                    my \$thr = threads->create(sub {});
                    \$thr->join();

                    print "\$x\n";  # number stringifies the same after thread

                    my \$y = eval "1.25";
                    print "\$y\n";  # number is ok after threads
                    print "\$y" eq "\$str_x" || 0;    # new number stringifies the same as old number
EOF
                "1.25\n1.25\n1.25\n1", { eval $switches }, "Thread join doesn't disrupt calling thread"
                ); # [GH 20155]
            }

          SKIP: {
            unless ($have_strtod) {
                skip("no strtod()", 1);
            }
            else {
                fresh_perl_is(<<"EOF",
                    use POSIX;
                    POSIX::setlocale(POSIX::LC_NUMERIC(),"$comma");
                    my \$one_point_5 = POSIX::strtod("1,5");
                    \$one_point_5 =~ s/0+\$//;  # Remove any trailing zeros
                    print \$one_point_5, "\n";
EOF
                "1.5", { stderr => 'devnull' }, "POSIX::strtod() uses underlying locale");
            }
          }
        }
    }

SKIP: {
        # Note: the setlocale Configure probe could be enhanced to give us the
        # syntax to use, but khw doesn't think it's worth it at this time, as
        # the current outliers seem to be skipped by the test just below
        # anyway.  If the POSIX 2008 locale functions are being used, the
        # syntax becomes mostly irrelevant, so do the test anyway if they are.
        # It's a lot of trouble to figure out in a perl script.
        if ($Config{d_setlocale_accepts_any_locale_name})
        {
            skip("Can't distinguish between valid and invalid locale names on this system", 2);
        }

        my @valid_categories = valid_locale_categories();

        my $valid_string = "";
        my $invalid_string = "";

        # Deliberately don't include all categories, so as to test this situation
        for my $i (0 .. @valid_categories - 2) {
            my $category = $valid_categories[$i];
            if ($category ne "LC_ALL") {
                $invalid_string .= ";" if $invalid_string ne "";
                $invalid_string .= "$category=foo_BAR";

                next unless $non_C_locale;
                $valid_string .= ";" if $valid_string ne "";
                $valid_string .= "$category=$non_C_locale";
            }
        }

        fresh_perl_is(<<"EOF",
                use locale;
                use POSIX;
                POSIX::setlocale(LC_ALL, "$invalid_string");
EOF
            "", { eval $switches },
            "In setting complicated invalid LC_ALL, final individ category doesn't need a \';'");

        skip("no non-C locale available", 1 ) unless $non_C_locale;
        fresh_perl_is(<<"EOF",
                use locale;
                use POSIX;
                POSIX::setlocale(LC_ALL, "$valid_string");
EOF
            "", { eval $switches },
            "In setting complicated valid LC_ALL, final individ category doesn't need a \';'");
    }

}

SKIP:
{
    use locale;
    # look for an english locale (so a < B, hopefully)
    my ($en) = grep { /^en_/ } find_locales( [ 'LC_COLLATE' ]);
    defined $en
        or skip "didn't find a suitable locale", 1;
    POSIX::setlocale(LC_COLLATE, $en);
    unless ("a" lt "B") {
        skip "didn't find a suitable locale", 1;
    }
    fresh_perl_is(<<'EOF', "ok\n", { args => [ $en ] }, "check for failed assertion");
use locale ':collate';
use POSIX qw(setlocale LC_COLLATE);
if (setlocale(LC_COLLATE, shift)) {
     my $x = "a";
     my $y = "B";
     print $x lt $y ? "ok\n" : "not ok\n";
     $x = "c"; # should empty the collxfrm magic but not remove it
     # which the free code asserts on
}
else {
     print "ok\n";
}
EOF
}

SKIP: {   # GH #20085
    my @utf8_locales = find_utf8_ctype_locales();
    skip "didn't find a UTF-8 locale", 1 unless @utf8_locales;

    local $ENV{LC_CTYPE} = $utf8_locales[0];
    local $ENV{LC_ALL} = undef;
    fresh_perl_is(<<~'EOF', "ok\n", {}, "check that setlocale overrides startup");
        use POSIX;

        my $a_acute = "\N{LATIN SMALL LETTER A WITH ACUTE}";
        my $egrave  = "\N{LATIN SMALL LETTER E WITH GRAVE}";
        my $combo = "$a_acute.$egrave";

        setlocale(&POSIX::LC_ALL, "C");
        use locale;

        # In a UTF-8 locale, \b matches Latin1 before string, mid, and end
        if ($combo eq ($combo =~ s/\b/!/gr)) {
            print "ok\n";
        }
        else {
            print "not ok\n";
        }
    EOF
}

SKIP: {   # GH #20054
    skip "Even illegal locale names are accepted", 1
                    if $Config{d_setlocale_accepts_any_locale_name}
                    && $Config{d_setlocale_accepts_any_locale_name} eq 'define';
	
    my @lc_all_locales = find_locales('LC_ALL');
    my $locale = $lc_all_locales[0];
    skip "LC_ALL not enabled on this platform", 1 unless $locale;

    local $ENV{LC_ALL} = "This is not a legal locale name";
    local $ENV{LANG} = "Nor this neither";

    my $fallback = ($^O eq "MSWin32")
                    ? "system default"
                    : "standard";
    fresh_perl_like("", qr/Falling back to the $fallback locale/,
                    {}, "check that illegal startup environment falls back");
}

done_testing();
