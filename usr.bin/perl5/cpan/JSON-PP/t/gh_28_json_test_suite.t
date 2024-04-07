# the following test cases are taken from JSONTestSuite
# by Nicolas Seriot (https://github.com/nst/JSONTestSuite)

use strict;
use warnings;
use Test::More;

BEGIN { plan tests => 20 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $DECODER = JSON::PP->new->utf8->allow_nonref;

# n_multidigit_number_then_00
decode_should_fail(qq!123\x00!);

# number_-01
decode_should_fail(qq![-01]!);

# number_neg_int_starting_with_zero
decode_should_fail(qq![-012]!);

# n_object_trailing_comment
decode_should_fail(qq!{"a":"b"}/**/!);

# n_object_trailing_comment_slash_open
decode_should_fail(qq!{"a":"b"}//!);

# n_structure_null-byte-outside-sting
decode_should_fail(qq![\x00]!);

# n_structure_object_with_comment
decode_should_fail(qq!{"a":/*comment*/"b"}!);

# n_structure_whitespace_formfeed
decode_should_fail(qq![\0x0c]!);

# y_string_utf16BE_no_BOM
decode_should_pass(qq!\x00[\x00"\x00\xE9\x00"\x00]!);

# y_string_utf16LE_no_BOM
decode_should_pass(qq![\x00"\x00\xE9\x00"\x00]\x00!);

sub decode_should_pass {
    my $json = shift;
    my $result = eval { $DECODER->decode($json); };
    ok !$@, $@ || '';
    ok defined $result;
}

sub decode_should_fail {
    my $json = shift;
    my $result = eval { $DECODER->decode($json); };
    ok $@, $@ || '';
    ok !defined $result;
}
