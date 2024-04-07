use strict;
use warnings;
use Test::More tests => 6;

BEGIN { use_ok('NEXT') };
my $order = 0;

package A;

our @ISA = qw/B C D/;
use if $] >= 5.009005, 'mro', 'dfs';


sub test { ::ok(++$order==1,"test A"); $_[0]->NEXT::UNSEEN::ACTUAL::test;}

package B;
our @ISA = qw/D C/;
use if $] >= 5.009005, 'mro', 'dfs';
sub test { ::ok(++$order==2,"test B"); $_[0]->NEXT::ACTUAL::UNSEEN::test;}

package C;
our @ISA = qw/D/;
use if $] >= 5.009005, 'mro', 'dfs';

sub test { ::ok(++$order==4,"test C"); $_[0]->NEXT::UNSEEN::ACTUAL::test;}

package D;
use if $] >= 5.009005, 'mro', 'dfs';

sub test { ::ok(++$order==3,"test D"); $_[0]->NEXT::ACTUAL::UNSEEN::test;}

package main;

my $foo = {};

bless($foo,"A");

eval{ $foo->test }
	? fail("Didn't die on missing ancestor") 
	: pass("Correctly dies after C");
