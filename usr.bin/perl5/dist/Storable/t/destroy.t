# [perl #118139] crash in global destruction when accessing the freed cxt.
use Test::More tests => 1;
use Storable;
BEGIN {
  store {}, "foo";
}
package foo;
sub new { return bless {} }
DESTROY {
  open FH, '<', "foo" or die $!;
  eval { Storable::pretrieve(*FH); };
  close FH or die $!;
  unlink "foo";
}

package main;
# print "# $^X\n";
$x = foo->new();

ok(1);
