#!perl

use Test::More tests => 18;
use Attribute::Handlers;

sub Args : ATTR(CODE) {
    my ($package, $symbol, $referent, $attr, $data, $phase, $filename, $linenum) = @_;
    is( $package,	'main',		'package' );
    is( $symbol,	\*foo,		'symbol' );
    is( $referent,	\&foo,		'referent' );
    is( $attr,		'Args',		'attr' );
    is( ref $data,	'ARRAY',	'data' );
    is( $data->[0],	'bar',		'data' );
    is( $phase,		'CHECK',	'phase' );
    is( $filename,	__FILE__,	'filename' );
    is( $linenum,	19,		'linenum' );
}

sub foo :Args(bar) {}

my $ref;
sub myref { $ref = shift; }
my $b;
#line 42
eval "my \$bar :SArgs(grumpf); \$b = \\\$bar";
is( $b, $ref, 'referent' );

sub SArgs : ATTR(SCALAR) {
    my ($package, $symbol, $referent, $attr, $data, $phase, $filename, $linenum) = @_;
    is( $package,	'main',		'package' );
    is( $symbol,	'LEXICAL',	'symbol' );
    myref($referent);
    is( $attr,		'SArgs',	'attr' );
    is( ref $data,	'ARRAY',	'data' );
    is( $data->[0],	'grumpf',	'data' );
    is( $phase,		'CHECK',	'phase' );
    TODO: {
      local $TODO = "Doesn't work correctly" if $] < 5.008;
      is( $filename,	__FILE__,	'filename' );
      is( $linenum,	42,		'linenum' );
    }
}
