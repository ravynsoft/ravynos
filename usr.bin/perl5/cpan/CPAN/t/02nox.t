#!./perl

if (! eval { require Test::More; 1 }) {
  printf "1..1\nok 1 # Test::More not available: skipping %s\n", __FILE__;
  exit;
}
require Test::More;
Test::More->import(tests => 8);

# use this first to $CPAN::term can be undefined
use_ok( 'CPAN' );
$CPAN::Suppress_readline = $CPAN::Suppress_readline; # silence
$CPAN::META = $CPAN::META; # silence
$CPAN::term = $CPAN::term; # silence
undef $CPAN::term;

# this kicks off all the magic
use_ok( 'CPAN::Nox' );

# this will be set if $CPAN::term is undefined
is( $CPAN::Suppress_readline, 1, 'should set suppress readline flag' );

# all of these modules have XS components, should be marked unavailable
my $mod;
for $mod (qw( Digest::MD5 LWP Compress::Zlib )) {
  is( $CPAN::META->has_inst($mod), 0, "$mod should be marked unavailable" );
}

# and these will be set to those in CPAN
is( scalar @CPAN::Nox::EXPORT, scalar @CPAN::EXPORT, 'should export just what CPAN does' );
is( \&CPAN::Nox::AUTOLOAD, \&CPAN::AUTOLOAD, 'AUTOLOAD should be aliased' );

# Local Variables:
# mode: cperl
# cperl-indent-level: 2
# End:
