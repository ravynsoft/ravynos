#! /usr/local/bin/perl -w

use 5.006;
use base Demo;

my $y : Demo :This($this) = sub : Demo(1,2,3) {};
sub x : Demo(4, 5, 6) :Multi {}
my %z : Demo(hash) :Multi(method,maybe);
# my %a : NDemo(hash);

{
	package Named;

	use base Demo;

	sub Demo :ATTR(SCALAR) { print STDERR "tada\n" }

	my $y : Demo :This($this) = sub : Demo(1,2,3) {};
	sub x : ExplMulti :Demo(4,5,6) {}
	my %z : ExplMulti :Demo(hash);
	my Named $q : Demo;
}

package Other;

my Demo $dother : Demo :This($this) = "okay";
my Named $nother : Demo :This($this) = "okay";

# my $unnamed : Demo;

# sub foo : Demo();
