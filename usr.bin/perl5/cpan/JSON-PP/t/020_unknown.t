use strict;
use warnings;

use Test::More;
BEGIN { plan tests => 10 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $json = JSON::PP->new;

eval q| $json->encode( [ sub {} ] ) |;
ok( $@ =~ /encountered CODE/, $@ );

eval q|  $json->encode( [ \-1 ] ) |;
ok( $@ =~ /cannot encode reference to scalar/, $@ );

eval q|  $json->encode( [ \undef ] ) |;
ok( $@ =~ /cannot encode reference to scalar/, $@ );

eval q|  $json->encode( [ \{} ] ) |;
ok( $@ =~ /cannot encode reference to scalar/, $@ );

$json->allow_unknown;

is( $json->encode( [ sub {} ] ), '[null]' );
is( $json->encode( [ \-1 ] ),    '[null]' );
is( $json->encode( [ \undef ] ), '[null]' );
is( $json->encode( [ \{} ] ),    '[null]' );


$json->allow_unknown(0);

my $fh;
open( $fh, '>hoge.txt' ) or die $!;

eval q| $json->encode( [ $fh ] ) |;
ok( $@ =~ /encountered GLOB|cannot encode reference to scalar/, $@ );

$json->allow_unknown(1);

is( $json->encode( [ $fh ] ),    '[null]' );

close $fh;

unlink('hoge.txt');
