#!perl -w

BEGIN {
    require 'loc_tools.pl';   # Contains locales_enabled() and
                              # find_utf8_ctype_locale()
}

use strict;
use Test::More;
use Config;

use XS::APItest;

my $tab = " " x 4;  # Indent subsidiary tests this much

use Unicode::UCD qw(search_invlist prop_invmap prop_invlist);
my ($charname_list, $charname_map, $format, $default) = prop_invmap("Name Alias");

sub get_charname($) {
    my $cp = shift;

    # If there is a an abbreviation for the code point name, use it
    my $name_index = search_invlist(\@{$charname_list}, $cp);
    if (defined $name_index) {
        my $synonyms = $charname_map->[$name_index];
        if (ref $synonyms) {
            my $pat = qr/: abbreviation/;
            my @abbreviations = grep { $_ =~ $pat } @$synonyms;
            if (@abbreviations) {
                return $abbreviations[0] =~ s/$pat//r;
            }
        }
    }

    # Otherwise, use the full name
    use charnames ();
    return charnames::viacode($cp) // "No name";
}

sub truth($) {  # Converts values so is() works
    return (shift) ? 1 : 0;
}

my $base_locale;
my $utf8_locale;
if(locales_enabled('LC_ALL')) {
    require POSIX;
    $base_locale = POSIX::setlocale( &POSIX::LC_ALL, "C");
    if (defined $base_locale && $base_locale eq 'C') {
        use locale; # make \w work right in non-ASCII lands

        # Some locale implementations don't have the 128-255 characters all
        # mean nothing.  Skip the locale tests in that situation
        for my $u (128 .. 255) {
            if (chr(utf8::unicode_to_native($u)) =~ /[[:print:]]/) {
                undef $base_locale;
                last;
            }
        }

        $utf8_locale = find_utf8_ctype_locale() if $base_locale;
    }
}

sub get_display_locale_or_skip($$) {

    # Helper function intimately tied to its callers.  It knows the loop
    # iterates with a locale of "", meaning don't use locale; $base_locale
    # meaning to use a non-UTF-8 locale; and $utf8_locale.
    #
    # It checks to see if the current test should be skipped or executed,
    # returning an empty list for the former, and for the latter:
    #   ( 'locale display name',
    #     bool of is this a UTF-8 locale )
    #
    # The display name is the empty string if not using locale.  Functions
    # with _LC in their name are skipped unless in locale, and functions
    # without _LC are executed only outside locale.

    my ($locale, $suffix) = @_;

    # The test should be skipped if the input is for a non-existent locale
    return unless defined $locale;

    # Here the input is defined, either a locale name or "".  If the test is
    # for not using locales, we want to do the test for non-LC functions,
    # and skip it for LC ones.
    if ($locale eq "") {
        return ("", 0) if $suffix !~ /LC/;
        return;
    }

    # Here the input is for a real locale.  We don't test the non-LC functions
    # for locales.
    return if $suffix !~ /LC/;

    # Here is for a LC function and a real locale.  The base locale is not
    # UTF-8.
    return (" ($locale locale)", 0) if $locale eq $base_locale;

    # The only other possibility is that we have a UTF-8 locale
    return (" ($locale)", 1);
}

sub try_malforming($$$)
{
    # Determines if the tests for malformed UTF-8 should be done.  When done,
    # the .xs code creates malformations by pretending the length is shorter
    # than it actually is.  Some things can't be malformed, and sometimes this
    # test knows that the current code doesn't look for a malformation under
    # various circumstances.

    my ($u, $function, $using_locale) = @_;
    # $u is unicode code point;

    # Single bytes can't be malformed
    return 0 if $u < ((ord "A" == 65) ? 128 : 160);

    # ASCII doesn't need to ever look beyond the first byte.
    return 0 if $function eq "ASCII";

    # Nor, on EBCDIC systems, does CNTRL
    return 0 if ord "A" != 65 && $function eq "CNTRL";

    # No controls above 255, so the code doesn't look at those
    return 0 if $u > 255 && $function eq "CNTRL";

    # No non-ASCII digits below 256, except if using locales.
    return 0 if $u < 256 && ! $using_locale && $function =~ /X?DIGIT/;

    return 1;
}

my %properties = (
                   # name => Lookup-property name
                   alnum => 'Word',
                   wordchar => 'Word',
                   alphanumeric => 'Alnum',
                   alpha => 'XPosixAlpha',
                   ascii => 'ASCII',
                   blank => 'Blank',
                   cntrl => 'Control',
                   digit => 'Digit',
                   graph => 'Graph',
                   idfirst => '_Perl_IDStart',
                   idcont => '_Perl_IDCont',
                   lower => 'XPosixLower',
                   print => 'Print',
                   psxspc => 'XPosixSpace',
                   punct => 'XPosixPunct',
                   quotemeta => '_Perl_Quotemeta',
                   space => 'XPerlSpace',
                   vertws => 'VertSpace',
                   upper => 'XPosixUpper',
                   xdigit => 'XDigit',
                );

my %seen;
my @warnings;
local $SIG{__WARN__} = sub { push @warnings, @_ };

my %utf8_param_code = (
                        "_safe"                 =>  0,
                        "_safe, malformed"      =>  1,
                      );

# This test is split into this number of files.
my $num_test_files = $ENV{TEST_JOBS} || 1;
$::TEST_CHUNK = 0 if $num_test_files == 1 && ! defined $::TEST_CHUNK;
$num_test_files = 10 if $num_test_files > 10;

my $property_count = -1;
foreach my $name (sort keys %properties, 'octal') {

    # We test every nth property in this run so that this test is split into
    # smaller chunks to minimize test suite elapsed time when run in parallel.
    $property_count++;
    next if $property_count % $num_test_files != $::TEST_CHUNK;

    my @invlist;
    if ($name eq 'octal') {
        # Hand-roll an inversion list with 0-7 in it and nothing else.
        push @invlist, ord "0", ord "8";
    }
    else {
        my $property = $properties{$name};
        @invlist = prop_invlist($property, '_perl_core_internal_ok');
        if (! @invlist) {

            # An empty return could mean an unknown property, or merely that
            # it is empty.  Call in scalar context to differentiate
            if (! prop_invlist($property, '_perl_core_internal_ok')) {
                fail("No inversion list found for $property");
                next;
            }
        }
    }

    # Include all the Latin1 code points, plus 0x100.
    my @code_points = (0 .. 256);

    # Then include the next few boundaries above those from this property
    my $above_latins = 0;
    foreach my $range_start (@invlist) {
        next if $range_start < 257;
        push @code_points, $range_start - 1, $range_start;
        $above_latins++;
        last if $above_latins > 5;
    }

    # This makes sure we are using the Perl definition of idfirst and idcont,
    # and not the Unicode.  There are a few differences.
    push @code_points, ord "\N{ESTIMATED SYMBOL}" if $name =~ /^id(first|cont)/;
    if ($name eq "idcont") {    # And some that are continuation but not start
        push @code_points, ord("\N{GREEK ANO TELEIA}"),
                           ord("\N{COMBINING GRAVE ACCENT}");
    }

    # And finally one non-Unicode code point.
    push @code_points, 0x110000;    # Above Unicode, no prop should match
    no warnings 'non_unicode';

    for my $n (@code_points) {
        my $u = utf8::native_to_unicode($n);
        my $function = uc($name);

        is (@warnings, 0, "Got no unexpected warnings in previous iteration")
           or diag("@warnings");
        undef @warnings;

        my $matches = search_invlist(\@invlist, $n);
        if (! defined $matches) {
            $matches = 0;
        }
        else {
            $matches = truth(! ($matches % 2));
        }

        my $ret;
        my $char_name = get_charname($n);
        my $display_name = sprintf "\\x{%02X, %s}", $n, $char_name;
        my $display_call = "is${function}( $display_name )";

        foreach my $suffix ("", "_A", "_L1", "_LC", "_uni", "_uvchr",
                            "_LC_uvchr", "_utf8", "_LC_utf8")
        {

            # Not all possible macros have been defined
            if ($name eq 'vertws') {

                # vertws is always all of Unicode
                next if $suffix !~ / ^ _ ( uni | uvchr | utf8 ) $ /x;
            }
            elsif ($name eq 'alnum') {

                # ALNUM_A, ALNUM_L1, and ALNUM_uvchr are not defined as these
                # suffixes were added later, after WORDCHAR was created to be
                # a clearer synonym for ALNUM
                next if    $suffix eq '_A'
                        || $suffix eq '_L1'
                        || $suffix eq '_uvchr';
            }
            elsif ($name eq 'octal') {
                next if $suffix ne ""  && $suffix ne '_A' && $suffix ne '_L1';
            }
            elsif ($name eq 'quotemeta') {
                # There is only one macro for this, and is defined only for
                # Latin1 range
                next if $suffix ne ""
            }

            foreach my $locale ("", $base_locale, $utf8_locale) {

                my ($display_locale, $locale_is_utf8)
                                = get_display_locale_or_skip($locale, $suffix);
                next unless defined $display_locale;

                use if $locale, "locale";
                POSIX::setlocale( &POSIX::LC_ALL, $locale) if $locale;

                if ($suffix !~ /utf8/) {    # _utf8 has to handled specially
                    my $display_call
                       = "is${function}$suffix( $display_name )$display_locale";
                    $ret = truth eval "test_is${function}$suffix($n)";
                    if (is ($@, "", "$display_call didn't give error")) {
                        my $truth = $matches;
                        if ($truth) {

                            # The single byte functions are false for
                            # above-Latin1
                            if ($n >= 256) {
                                $truth = 0
                                        if $suffix=~ / ^ ( _A | _L [1C] )? $ /x;
                            }
                            elsif (   $u >= 128
                                   && $name ne 'quotemeta')
                            {

                                # The no-suffix and _A functions are false
                                # for non-ASCII.  So are  _LC  functions on a
                                # non-UTF-8 locale
                                $truth = 0 if    $suffix eq "_A"
                                              || $suffix eq ""
                                              || (     $suffix =~ /LC/
                                                  && ! $locale_is_utf8);
                            }
                        }

                        is ($ret, $truth, "${tab}And correctly returns $truth");
                    }
                }
                else {  # _utf8 suffix
                    my $char = chr($n);
                    utf8::upgrade($char);
                    $char = quotemeta $char if $char eq '\\' || $char eq "'";
                    my $truth;
                    if (   $suffix =~ /LC/
                        && ! $locale_is_utf8
                        && $n < 256
                        && $u >= 128)
                    {   # The C-locale _LC function returns FALSE for Latin1
                        # above ASCII
                        $truth = 0;
                    }
                    else {
                        $truth = $matches;
                    }

                    foreach my $utf8_param("_safe",
                                           "_safe, malformed",
                                          )
                    {
                        my $utf8_param_code = $utf8_param_code{$utf8_param};
                        my $expect_error = $utf8_param_code > 0;
                        next if      $expect_error
                                && ! try_malforming($u, $function,
                                                    $suffix =~ /LC/);

                        my $display_call = "is${function}$suffix( $display_name"
                                         . ", $utf8_param )$display_locale";
                        $ret = truth eval "test_is${function}$suffix('$char',"
                                        . " $utf8_param_code)";
                        if ($expect_error) {
                            isnt ($@, "",
                                    "expected and got error in $display_call");
                            like($@, qr/Malformed UTF-8 character/,
                                "${tab}And got expected message");
                            if (is (@warnings, 1,
                                           "${tab}Got a single warning besides"))
                            {
                                like($warnings[0],
                                     qr/Malformed UTF-8 character.*short/,
                                     "${tab}Got expected warning");
                            }
                            else {
                                diag("@warnings");
                            }
                            undef @warnings;
                        }
                        elsif (is ($@, "", "$display_call didn't give error")) {
                            is ($ret, $truth,
                                "${tab}And correctly returned $truth");
                            if ($utf8_param_code < 0) {
                                my $warnings_ok;
                                my $unique_function = "is" . $function . $suffix;
                                if (! $seen{$unique_function}++) {
                                    $warnings_ok = is(@warnings, 1,
                                        "${tab}This is first call to"
                                      . " $unique_function; Got a single"
                                      . " warning");
                                    if ($warnings_ok) {
                                        $warnings_ok = like($warnings[0],
                qr/starting in Perl .* will require an additional parameter/,
                                            "${tab}The warning was the expected"
                                          . " deprecation one");
                                    }
                                }
                                else {
                                    $warnings_ok = is(@warnings, 0,
                                        "${tab}This subsequent call to"
                                      . " $unique_function did not warn");
                                }
                                $warnings_ok or diag("@warnings");
                                undef @warnings;
                            }
                        }
                    }
                }
            }
        }
    }
}

my %to_properties = (
                FOLD  => 'Case_Folding',
                LOWER => 'Lowercase_Mapping',
                TITLE => 'Titlecase_Mapping',
                UPPER => 'Uppercase_Mapping',
            );

$property_count = -1;
foreach my $name (sort keys %to_properties) {

    $property_count++;
    next if $property_count % $num_test_files != $::TEST_CHUNK;

    my $property = $to_properties{$name};
    my ($list_ref, $map_ref, $format, $missing)
                                      = prop_invmap($property, );
    if (! $list_ref || ! $map_ref) {
        fail("No inversion map found for $property");
        next;
    }
    if ($format !~ / ^ a l? $ /x) {
        fail("Unexpected inversion map format ('$format') found for $property");
        next;
    }

    # Include all the Latin1 code points, plus 0x100.
    my @code_points = (0 .. 256);

    # Then include the next few multi-char folds above those from this
    # property, and include the next few single folds as well
    my $above_latins = 0;
    my $multi_char = 0;
    for my $i (0 .. @{$list_ref} - 1) {
        my $range_start = $list_ref->[$i];
        next if $range_start < 257;
        if (ref $map_ref->[$i] && $multi_char < 5)  {
            push @code_points, $range_start - 1
                                        if $code_points[-1] != $range_start - 1;
            push @code_points, $range_start;
            $multi_char++;
        }
        elsif ($above_latins < 5) {
            push @code_points, $range_start - 1
                                        if $code_points[-1] != $range_start - 1;
            push @code_points, $range_start;
            $above_latins++;
        }
        last if $above_latins >= 5 && $multi_char >= 5;
    }

    # And finally one non-Unicode code point.
    push @code_points, 0x110000;    # Above Unicode, no prop should match
    no warnings 'non_unicode';

    # $n is native; $u unicode.
    for my $n (@code_points) {
        my $u = utf8::native_to_unicode($n);
        my $function = $name;

        my $index = search_invlist(\@{$list_ref}, $n);

        my $ret;
        my $char_name = get_charname($n);
        my $display_name = sprintf "\\N{U+%02X, %s}", $n, $char_name;

        foreach my $suffix ("", "_L1", "_LC") {

            # This is the only macro defined for L1
            next if $suffix eq "_L1" && $function ne "LOWER";

          SKIP:
            foreach my $locale ("", $base_locale, $utf8_locale) {

                # titlecase is not defined in locales.
                next if $name eq 'TITLE' && $suffix eq "_LC";

                my ($display_locale, $locale_is_utf8)
                                = get_display_locale_or_skip($locale, $suffix);
                next unless defined $display_locale;

                skip("to${name}_LC does not work for LATIN SMALL LETTER SHARP S"
                  . "$display_locale", 1)
                            if  $u == 0xDF && $name =~ / FOLD | UPPER /x
                             && $suffix eq "_LC" && $locale_is_utf8;

                use if $locale, "locale";
                POSIX::setlocale( &POSIX::LC_ALL, $locale) if $locale;

                my $display_call = "to${function}$suffix("
                                 . " $display_name )$display_locale";
                $ret = eval "test_to${function}$suffix($n)";
                if (is ($@, "", "$display_call didn't give error")) {
                    my $should_be;
                    if ($n > 255) {
                        $should_be = $n;
                    }
                    elsif (     $u > 127
                            && (   $suffix eq ""
                                || ($suffix eq "_LC" && ! $locale_is_utf8)))
                    {
                        $should_be = $n;
                    }
                    elsif ($map_ref->[$index] != $missing) {
                        $should_be = $map_ref->[$index] + $n - $list_ref->[$index]
                    }
                    else {
                        $should_be = $n;
                    }

                    is ($ret, $should_be,
                        sprintf("${tab}And correctly returned 0x%02X",
                                                              $should_be));
                }
            }
        }

        # The _uni, uvchr, and _utf8 functions return both the ordinal of the
        # first code point of the result, and the result in utf8.  The .xs
        # tests return these in an array, in [0] and [1] respectively, with
        # [2] the length of the utf8 in bytes.
        my $utf8_should_be = "";
        my $first_ord_should_be;
        if (ref $map_ref->[$index]) {   # A multi-char result
            for my $n (0 .. @{$map_ref->[$index]} - 1) {
                $utf8_should_be .= chr $map_ref->[$index][$n];
            }

            $first_ord_should_be = $map_ref->[$index][0];
        }
        else {  # A single-char result
            $first_ord_should_be = ($map_ref->[$index] != $missing)
                                    ? $map_ref->[$index] + $n
                                                         - $list_ref->[$index]
                                    : $n;
            $utf8_should_be = chr $first_ord_should_be;
        }
        utf8::upgrade($utf8_should_be);

        # Test _uni, uvchr
        foreach my $suffix ('_uni', '_uvchr') {
            my $s;
            my $len;
            my $display_call = "to${function}$suffix( $display_name )";
            $ret = eval "test_to${function}$suffix($n)";
            if (is ($@, "", "$display_call didn't give error")) {
                is ($ret->[0], $first_ord_should_be,
                    sprintf("${tab}And correctly returned 0x%02X",
                                                    $first_ord_should_be));
                is ($ret->[1], $utf8_should_be, "${tab}Got correct utf8");
                use bytes;
                is ($ret->[2], length $utf8_should_be,
                    "${tab}Got correct number of bytes for utf8 length");
            }
        }

        # Test _utf8
        my $char = chr($n);
        utf8::upgrade($char);
        $char = quotemeta $char if $char eq '\\' || $char eq "'";
        foreach my $utf8_param("_safe",
                                "_safe, malformed",
                                )
        {
            use Config;
            next if    $utf8_param eq 'deprecated mathoms'
                    && $Config{'ccflags'} =~ /-DNO_MATHOMS/;

            my $utf8_param_code = $utf8_param_code{$utf8_param};
            my $expect_error = $utf8_param_code > 0;

            # Skip if can't malform (because is a UTF-8 invariant)
            next if $expect_error && $u < ((ord "A" == 65) ? 128 : 160);

            my $display_call = "to${function}_utf8($display_name, $utf8_param )";
            $ret = eval   "test_to${function}_utf8('$char', $utf8_param_code)";
            if ($expect_error) {
                isnt ($@, "", "expected and got error in $display_call");
                like($@, qr/Malformed UTF-8 character/,
                     "${tab}And got expected message");
                undef @warnings;
            }
            elsif (is ($@, "", "$display_call didn't give error")) {
                is ($ret->[0], $first_ord_should_be,
                    sprintf("${tab}And correctly returned 0x%02X",
                                                    $first_ord_should_be));
                is ($ret->[1], $utf8_should_be, "${tab}Got correct utf8");
                use bytes;
                is ($ret->[2], length $utf8_should_be,
                    "${tab}Got correct number of bytes for utf8 length");
                if ($utf8_param_code < 0) {
                    my $warnings_ok;
                    if (! $seen{"${function}_utf8$utf8_param"}++) {
                        $warnings_ok = is(@warnings, 1,
                                                   "${tab}Got a single warning");
                        if ($warnings_ok) {
                            my $expected;
                            if ($utf8_param_code == -2) {
                                my $lc_func = lc $function;
                                $expected
                = qr/starting in Perl .* to_utf8_$lc_func\(\) will be removed/;
                            }
                            else {
                                $expected
                = qr/starting in Perl .* will require an additional parameter/;
                            }
                            $warnings_ok = like($warnings[0], $expected,
                                      "${tab}Got expected deprecation warning");
                        }
                    }
                    else {
                        $warnings_ok = is(@warnings, 0,
                                  "${tab}Deprecation warned only the one time");
                    }
                    $warnings_ok or diag("@warnings");
                    undef @warnings;
                }
            }
        }
    }
}

# This is primarily to make sure that no non-Unicode warnings get generated
is(scalar @warnings, 0, "No unexpected warnings were generated in the tests")
  or diag @warnings;

done_testing;
