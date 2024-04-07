#!perl -w
use warnings;
use strict;

use Test::More;
use XS::APItest;

sub expected($$$$) {
    my ($copied,        # What the copy should look like
        $length,        # but truncated to this,
        $poison,        # and filled with this so as to catch overruns
        $actual_dest_length)   # to this total number of bytes
    = @_;

    return substr($copied, 0, $length) . ($poison x ($actual_dest_length - $length));
}

my $b = "\\";
my $poison = '?';
my $failure_return = 0x7FFF_FFFF;   # I32 max
my $ret;

# ib = innocent bystander; a character that isn't a delimiter nor an escape
my $ib = 'y';

foreach my $d ("x", "\0") {     # Try both printable and NUL delimiters
    my $source = $ib;
    my $source_len = 1;
    my $should_be = $source;

    $ret = test_delimcpy($source, $source_len, $d, $source_len, $source_len, $poison);
    is($ret->[0], expected($source, $source_len, $poison, $source_len),
       "Works when there is no delimiter at all");
    is($ret->[1], $source_len, "Destination length is correct");
    is($ret->[2], 1, "Source advance is correct");

    $source .= $d;
    $ret = test_delimcpy($source, $source_len, $d, $source_len, $source_len, $poison);
    is($ret->[0], expected($source, $source_len, $poison, $source_len),
       "Works when delimiter is just beyond the examined portion");
    is($ret->[1], $source_len, "Destination length is correct");
    is($ret->[2], 1, "Source advance is correct");

    $should_be = $ib . $b;
    $source = $should_be . $d;
    $source_len = 2;
    $ret = test_delimcpy($source, $source_len, $d, $source_len, $source_len, $poison);
    is($ret->[0], expected($source, $source_len, $poison, $source_len),
       "Works when delimiter is just beyond the examined portion, which"
     . " ends in a backslash");
    is($ret->[1], $source_len, "Destination length is correct");
    is($ret->[2], 2, "Source advance is correct");

    # Delimiter in first byte
    my $actual_dest_len = 5;
    $ret = test_delimcpy($d, 1, $d, $actual_dest_len, $actual_dest_len, $poison);
    is($ret->[0], "\0" . $poison x ($actual_dest_len - 1),
       "Copied correctly when delimiter is first character");
    is($ret->[1], 0, "0 bytes copied");
    is($ret->[2], 0, "0 bytes advanced");

    # Now to more extensive tests.  The source includes escaped delimiters
    # (which should have one backslash stripped), and finally a delimiter with
    # an even number of backslashes, so is not escaped
    my $base_source = $b . $d . $b x 3 . $d . $b x 5 . $d . $b x 2 . $d;
    $should_be =           $d . $b x 2 . $d . $b x 4 . $d . $b x 2;
    # byte counts:          |    ||       |    ||||     |    ||   = 11 bytes
    my $dest_len = 11;

    # The return from this function should be how many bytes did it advance
    # the source pointer.  This won't include the unescaped delimiter
    my $source_advance = length($base_source) - 1;

    # Add some trailing non-special charss
    $source = $base_source . ($ib x 6);
    $source_len = length $source;
    $actual_dest_len = $source_advance + 10;

    my $with_NUL = $should_be . "\0";
    my $trunc_dest_len = length $with_NUL;

    $ret = test_delimcpy($source, $source_len,
                         $d, $actual_dest_len, $trunc_dest_len+1, $poison);
    is($ret->[0], expected($with_NUL, $trunc_dest_len, $poison,
                                                            $actual_dest_len),
      "Stops at first unescaped delimiter; stripping off the escapes;"
    . " destination has more than enough space for a safety NUL");
    is($ret->[1], $dest_len, "Destination length is correct");
    is($ret->[2], $source_advance, "Source advance is correct");

    $ret = test_delimcpy($source, $source_len, $d,
                         $actual_dest_len, $trunc_dest_len, $poison);
    is($ret->[0], expected($with_NUL, $trunc_dest_len, $poison,
                                                            $actual_dest_len),
       "As above, but with just enough space for a safety NUL");
    is($ret->[1], $dest_len, "Destination length is correct");
    is($ret->[2], $source_advance, "Source advance is correct");

    $trunc_dest_len--;
    $ret = test_delimcpy($source, $source_len,
                         $d, $actual_dest_len, $trunc_dest_len,
                         $poison);
    is($ret->[0], expected($should_be, $trunc_dest_len, $poison,
                                                               $actual_dest_len),
      "As above, but not enough room for the safety NUL");
    is($ret->[1], $dest_len, "Destination length is correct");
    is($ret->[2], $source_advance, "Source advance is correct");

    $trunc_dest_len--;
    $ret = test_delimcpy($source, $source_len,
                         $d, $actual_dest_len, $trunc_dest_len,
                         $poison);
    is($ret->[0], expected($should_be, $trunc_dest_len, $poison,
                                                            $actual_dest_len),
       "As above, but not enough room for the final backslash");
    ok($ret->[1] > $trunc_dest_len,
       "Error return is correctly > buffer length");
    is($ret->[2], $source_advance, "Source advance is correct");

    # Keep trying shorter and shorter permissible dest lengths
    do {
        $trunc_dest_len--;
        $ret = test_delimcpy($source, $source_len,
                             $d, $actual_dest_len, $trunc_dest_len,
                             $poison);
        is($ret->[0], expected($should_be, $trunc_dest_len, $poison,
                                                            $actual_dest_len),
           "Preceding test but room only for $trunc_dest_len bytes");
        ok($ret->[1] > $trunc_dest_len,
           "Error return is correctly > buffer length");
        is($ret->[2], $source_advance, "Source advance is correct");
    } while ($trunc_dest_len > 0);
}

# Repeat a few of the tests with a backslash delimiter, which means there
# is no possibiliby of an escape.  And this escape-less form can be used to
# also do a general test on 'delimcpy_no_escape'
foreach my $d ("x", "\0", '\\') {
    for my $func (qw(delimcpy delimcpy_no_escape)) {
        next if $func eq 'delimcpy' && $d ne '\\';
        my $test_func = "test_$func";

        my $source = $ib;
        my $source_len = 1;
        my $should_be = $source;

        $ret = eval "$test_func(\$source, \$source_len, \$d, \$source_len, \$source_len, \$poison)";
        is($ret->[0], expected($source, $source_len, $poison, $source_len),
           "$func works when there is no delimiter at all");
        is($ret->[1], $source_len, "Destination length is correct");
        is($ret->[2], 1, "Source advance is correct");

        $source .= $d;
        $ret = eval "$test_func(\$source, \$source_len, \$d, \$source_len, \$source_len, \$poison)";
        is($ret->[0], expected($source, $source_len, $poison, $source_len),
        "Works when delimiter is just beyond the examined portion");
        is($ret->[1], $source_len, "Destination length is correct");
        is($ret->[2], 1, "Source advance is correct");

        # Delimiter in first byte
        my $actual_dest_len = 5;
        $ret = eval "$test_func(\$d, 1, \$d, \$actual_dest_len, \$actual_dest_len, \$poison)";
        is($ret->[0], "\0" . $poison x ($actual_dest_len - 1),
        "Copied correctly when delimiter is first character");
        is($ret->[1], 0, "0 bytes copied");
        is($ret->[2], 0, "0 bytes advanced");

        $source = $ib x 6;
        my $len_sans_delim = length $source;
        my $with_NULL = $source . "\0";
        $source .= $d . ($ib x 7);
        $source_len = length $source;
        $ret = eval "$test_func(\$source, \$source_len, \$d, \$source_len, \$source_len, \$poison)";
        is($ret->[0], expected($with_NULL, $len_sans_delim + 1, $poison, $source_len),
           "$func works when delim is in middle of source, plenty of room");
        is($ret->[1], $len_sans_delim, "Destination length is correct");
        is($ret->[2], $len_sans_delim, "Source advance is correct");

        $ret = eval "$test_func(\$source, \$source_len, \$d, \$source_len, \$len_sans_delim, \$poison)";
        is($ret->[0], expected($source, $len_sans_delim, $poison, $source_len),
           "$func works when delim is in middle of source; no room for safety NUL");
        is($ret->[1], $len_sans_delim, "Destination length is correct");
        is($ret->[2], $len_sans_delim, "Source advance is correct");

        $ret = eval "$test_func(\$source, \$source_len, \$d, \$source_len, \$len_sans_delim - 1, \$poison)";
        is($ret->[0], expected($source, $len_sans_delim - 1, $poison, $source_len),
           "$func works when not enough space for copy");
        is($ret->[1], $failure_return, "Destination length is correct");
        is($ret->[2], $len_sans_delim, "Source advance is correct");
    }
}

done_testing();
