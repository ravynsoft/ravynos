#!perl -w

# This is a base file to be used by various .t's in its directory
# It tests various malformed UTF-8 sequences and some code points that are
# "problematic", and verifies that the correct warnings/flags etc are
# generated when using them.  For the code points, it also takes the UTF-8 and
# perturbs it to be malformed in various ways, and tests that this gets
# appropriately detected.

use strict;
use Test::More;

BEGIN {
    require './t/utf8_setup.pl';
    use_ok('XS::APItest');
};

$|=1;

use Data::Dumper;

my @well_formed = (
            "\xE1",
            "The quick brown fox jumped over the lazy dog",
            "Ces systèmes de codage sont souvent incompatibles entre eux.  Ainsi, deux systèmes peuvent utiliser le même nombre pour deux caractères différents ou utiliser différents nombres pour le même caractère.",
            "Kelimelerin m\xC3\xAAme caract\xC3\xA8re ve yaz\xC3\xB1abc",
);

my @malformed = (
            "Kelimelerin m\xC3\xAAme caract\xC3\xA8re ve yaz\xC4\xB1abc",
            "Kelimelerin m\xC3\xAAme caract\xC3\xA8re ve yaz\xC4\xB1\xC3\xA8abc",
            "Kelimelerin m\xC3\xAAme caract\xC3re ve yazi\xC3\xA8abc",
            "Kelimelerin m\xC3\xAAme caract\xA8 ve yazi\xC3\xA8abc",
            "Kelimelerin m\xC3\xAAme caract\xC3\xA8\xC3re ve yazi\xC3\xA8abc",
);

for my $test (@well_formed) {
    my $utf8 = $test;
    utf8::upgrade($utf8);
    my $utf8_length;
    my $byte_length = length $test;

    {
        use bytes;
        $utf8_length = length $utf8;
    }

    my $ret_ref = test_utf8_to_bytes($utf8, $utf8_length);

    is ($ret_ref->[0], $test, "Successfully downgraded "
                            . display_bytes($utf8));
    is ($ret_ref->[1], $byte_length, "... And returned correct length("
                                     . $byte_length . ")");
}

for my $test (@malformed) {
    my $utf8 = $test;
    my $utf8_length = length $test;

    my $ret_ref = test_utf8_to_bytes($utf8, $utf8_length);

    ok (! defined $ret_ref->[0], "Returned undef for malformed "
                                . display_bytes($utf8));
    is ($ret_ref->[1], -1, "... And returned length -1");
    is ($ret_ref->[2], $utf8, "... And left the input unchanged");
}

done_testing();
