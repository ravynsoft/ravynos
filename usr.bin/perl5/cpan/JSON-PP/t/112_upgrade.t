use strict;
use warnings;
use Test::More;

BEGIN { plan tests => 3 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $json = JSON::PP->new->allow_nonref->utf8;
my $str  = '\\u00b6';

my $value = $json->decode( '"\\u00b6"' );

#use Devel::Peek;
#Dump( $value );

is( $value, chr 0xb6 );

ok( utf8::is_utf8( $value ) );

eval { $json->decode( '"' . chr(0xb6) . '"' ) };
ok( $@ =~ /malformed UTF-8 character in JSON string/ );

