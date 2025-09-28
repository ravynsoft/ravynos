#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

use strict;
use warnings;

eval "require 'meta_notation.pm'";
if ($@) {
    fail("Could not find 'meta_notation.pm'");
}
else {

    is(_meta_notation("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C"),
                      "^@^A^B^C^D^E^F^G^H^I^J^K^L");
    is(_meta_notation("\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"),
                      "^M^N^O^P^Q^R^S^T^U^V^W^X^Y");
    is(_meta_notation("\x1A\x1B\x1C\x1D\x1E\x1F\c?"),
                      "^Z^[^\\^]^^^_^?");
    is(_meta_notation("09%AZaz\x{103}"), "09%AZaz\x{103}");

    if ($::IS_ASCII || $::IS_ASCII) {
        is(_meta_notation("\x7f\x80\x81\x82\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1"),
                        '^?M-^@M-^AM-^BM-^ZM-^[M-^\\M-^]M-^^M-^_M- M-!');
        is(_meta_notation("\x{c1}\x{e2}"), 'M-AM-b');
        is(_meta_notation("\x{df}"), 'M-_');
    }
    else {  # EBCDIC platform
        # In the first iteration we are looking for a non-ASCII control; in
        # the second, a regular non-ASCII character.  SPACE marks the end of
        # most controls.  We test each to see that they are properly converted
        # to \x{...}
        foreach my $start (0x20, ord " ") {
            for (my $i = $start; $i < 256; $i++) {
                my $char = chr $i;
                next if $char =~ /[[:ascii:]]/;
                is(_meta_notation($char), sprintf("\\x{%X}", $i));
                last;
            }
        }
    }
}

done_testing();
