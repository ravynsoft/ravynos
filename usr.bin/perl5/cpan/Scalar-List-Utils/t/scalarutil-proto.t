#!./perl

use strict;
use warnings;

use Scalar::Util ();
use Test::More  (grep { /set_prototype/ } @Scalar::Util::EXPORT_FAIL)
    ? (skip_all => 'set_prototype requires XS version')
    : (tests => 14);

Scalar::Util->import('set_prototype');

sub f { }
is( prototype('f'), undef, 'no prototype');

my $r = set_prototype(\&f,'$');
is( prototype('f'), '$', 'set prototype');
is( $r, \&f, 'return value');

set_prototype(\&f,undef);
is( prototype('f'), undef, 'remove prototype');

set_prototype(\&f,'');
is( prototype('f'), '', 'empty prototype');

sub g (@) { }
is( prototype('g'), '@', '@ prototype');

set_prototype(\&g,undef);
is( prototype('g'), undef, 'remove prototype');

sub stub;
is( prototype('stub'), undef, 'non existing sub');

set_prototype(\&stub,'$$$');
is( prototype('stub'), '$$$', 'change non existing sub');

sub f_decl ($$$$);
is( prototype('f_decl'), '$$$$', 'forward declaration');

set_prototype(\&f_decl,'\%');
is( prototype('f_decl'), '\%', 'change forward declaration');

eval { &set_prototype( 'f', '' ); };
print "not " unless 
ok($@ =~ /^set_prototype: not a reference/, 'not a reference');

eval { &set_prototype( \'f', '' ); };
ok($@ =~ /^set_prototype: not a subroutine reference/, 'not a sub reference');

# RT 72080

{
  package TiedCV;
  sub TIESCALAR {
    my $class = shift;
    return bless {@_}, $class;
  }
  sub FETCH {
    return \&my_subr;
  }
  sub my_subr {
  }
}

my $cv;
tie $cv, 'TiedCV';

&Scalar::Util::set_prototype($cv, '$$');
is( prototype($cv), '$$', 'set_prototype() on tied CV ref' );
