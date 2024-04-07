use strict;
use warnings;
use Test::More tests => 7;

BEGIN { use_ok('NEXT') };
my $order = 0;

package A;
our @ISA = qw/B C D/;
use if $] >= 5.009005, 'mro', 'dfs';

sub test { ::ok(++$order==1,"test A"); $_[0]->NEXT::UNSEEN::test; 1}

package B;

our @ISA = qw/D C/;
use if $] >= 5.009005, 'mro', 'dfs';
sub test { ::ok(++$order==2,"test B"); $_[0]->NEXT::UNSEEN::test; 1}

package C;
our @ISA = qw/D/;
use if $] >= 5.009005, 'mro', 'dfs';

sub test { ::ok(++$order==4,"test C"); $_[0]->NEXT::UNSEEN::test; 1}

package D;
use if $] >= 5.009005, 'mro', 'dfs';

sub test { ::ok(++$order==3,"test D"); $_[0]->NEXT::UNSEEN::test; 1}

package main;
use if $] >= 5.009005, 'mro', 'dfs';

my $foo = {};

bless($foo,"A");

eval{ $foo->test }
	? pass("Correctly survives after C")
	: fail("Shouldn't die on missing ancestor");

package Diamond::Base;
use if $] >= 5.009005, 'mro', 'dfs';
my $seen;
sub test {
	$seen++ ? ::fail("Can't visit inherited test twice")
		: ::pass("First diamond is okay");
	shift->NEXT::UNSEEN::test;
}

package Diamond::Left;  our @ISA = qw[Diamond::Base];
use if $] >= 5.009005, 'mro', 'dfs';
package Diamond::Right; our @ISA = qw[Diamond::Base];
use if $] >= 5.009005, 'mro', 'dfs';
package Diamond::Top;   our @ISA = qw[Diamond::Left Diamond::Right];
use if $] >= 5.009005, 'mro', 'dfs';


package main;

Diamond::Top->test;
