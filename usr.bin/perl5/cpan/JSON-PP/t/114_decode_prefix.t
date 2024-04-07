use strict;
use warnings;
use Test::More tests => 8;

BEGIN {
    $ENV{ PERL_JSON_BACKEND } = 0;
}

use JSON::PP;

my $json = JSON::PP->new;

my $complete_text = qq/{"foo":"bar"}/;
my $garbaged_text  = qq/{"foo":"bar"}\n/;
my $garbaged_text2 = qq/{"foo":"bar"}\n\n/;
my $garbaged_text3 = qq/{"foo":"bar"}\n----/;

is( ( $json->decode_prefix( $complete_text )  ) [1], 13 );
is( ( $json->decode_prefix( $garbaged_text )  ) [1], 13 );
is( ( $json->decode_prefix( $garbaged_text2 ) ) [1], 13 );
is( ( $json->decode_prefix( $garbaged_text3 ) ) [1], 13 );

eval { $json->decode( "\n" ) }; ok( $@ =~ /malformed JSON/ );
eval { $json->allow_nonref(0)->decode('null') }; ok $@ =~ /allow_nonref/;

eval { $json->decode_prefix( "\n" ) }; ok( $@ =~ /malformed JSON/ );
eval { $json->allow_nonref(0)->decode_prefix('null') }; ok $@ =~ /allow_nonref/;

