use strict;
use warnings;
use Test::More tests => 10;

BEGIN { use_ok('NEXT') };
my $order = 0;

package A;
our @ISA = qw/B C D/;
use if $] >= 5.009005, 'mro', 'dfs';


sub test { ++$order; ::ok($order==1,"test A"); $_[0]->NEXT::ACTUAL::test;}

package B;
our @ISA = qw/D C/;
use if $] >= 5.009005, 'mro', 'dfs';
sub test { ++$order; ::ok($order==2,"test B"); $_[0]->NEXT::ACTUAL::test;}

package C;
our @ISA = qw/D/;
use if $] >= 5.009005, 'mro', 'dfs';

sub test {
	++$order; ::ok($order==4||$order==6,"test C");
	$_[0]->NEXT::ACTUAL::test;
}

package D;
use if $] >= 5.009005, 'mro', 'dfs';

sub test {
	++$order; ::ok($order==3||$order==5||$order==7||$order==8,"test D");
        $_[0]->NEXT::ACTUAL::test;
}

package main;
use if $] >= 5.009005, 'mro', 'dfs';

my $foo = {};

bless($foo,"A");

eval{ $foo->test }
	? fail("Didn't die on missing ancestor")
	: pass("Correctly dies after full traversal");
