use strict;
use warnings;
use Test::More tests => 9;

BEGIN { require_ok('Module::CoreList::Utils'); }

ok( defined $Module::CoreList::Utils::utilities{5}{a2p}, '5 had a2p' );
is( Module::CoreList::Utils->first_release('a2p'), 5, 'a2p first released in 5' );
is( Module::CoreList::Utils::first_release('a2p'), 5, 'a2p first released in 5' );
is( Module::CoreList::Utils->first_release('corelist'), 5.008009, 'corelist with v5.8.9');
is( Module::CoreList::Utils->first_release_by_date('corelist'), 5.009002, 'corelist with v5.9.2');
is( Module::CoreList::Utils::first_release_by_date('corelist'), 5.009002, 'corelist with v5.9.2');
{
  my @expected = sort qw(a2p c2ph cppstdin find2perl h2xs pstruct s2p);
  {
    my @foo = Module::CoreList::Utils->utilities(5.001);
    is_deeply( \@foo, \@expected, '5.001 utils all present and correct' );
  }
  {
    my @foo = Module::CoreList::Utils::utilities(5.001);
    is_deeply( \@foo, \@expected, '5.001 utils all present and correct' );
  }
}
