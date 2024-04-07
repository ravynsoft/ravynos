use strict;
use warnings;
use Test2::Tools::Tiny;

# this test is only relevant under Devel::Cover

require Test::More;

my $destroy = 0;
sub CountDestroy::DESTROY { $destroy++ }

my $obj = bless {}, 'CountDestroy';

Test::More::is($obj, $obj, 'compare object to itself using is');

undef $obj;

is $destroy, 1, 'undef object destroyed after being passed to is';

done_testing;
