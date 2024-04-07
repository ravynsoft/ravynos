use Test::More tests => 1;
 
package dumb_thing;

use strict; use warnings;
use Tie::Array;
use Carp;
use base 'Tie::StdArray';

sub TIEARRAY {
    my $class = shift;
    my $this  = bless [], $class;
    my $that  = shift;

    @$this = @$that;

    $this;
}

package main;

use strict; use warnings;
use Storable qw(freeze thaw);

my $x = [1,2,3,4];

broken($x); # ties $x
broken( thaw( freeze($x) ) ); # since 5.16 fails with "Cannot tie unreifiable array"

sub broken {
    my $w = shift;
    tie @$_, dumb_thing => $_ for $w;
}

# fails since 5.16
ok 1, 'Does not fail with "Cannot tie unreifiable array" RT#84705';
