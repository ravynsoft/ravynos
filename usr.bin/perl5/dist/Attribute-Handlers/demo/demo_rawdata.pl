package UNIVERSAL;
use Attribute::Handlers;

sub Cooked : ATTR(SCALAR) { print pop, "\n" }
sub PostRaw : ATTR(SCALAR,RAWDATA) { print pop, "\n" }
sub PreRaw : ATTR(SCALAR,RAWDATA) { print pop, "\n" }

package main;

my $x : Cooked(1..5);
my $y : PreRaw(1..5);
my $z : PostRaw(1..5);
