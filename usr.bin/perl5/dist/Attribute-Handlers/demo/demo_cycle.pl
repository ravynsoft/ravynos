package Selfish;

sub TIESCALAR {
	use Data::Dumper 'Dumper';
	print Dumper [ \@_ ];
	bless [ @_[1..$#_] ], $_[0];
}

sub FETCH {
	use Data::Dumper 'Dumper';
	Dumper [ @{$_[0]} ];
}

package main;

use Attribute::Handlers autotieref => { Selfish => Selfish };

my $next : Selfish("me");
print "$next\n";

my $last : Selfish("you","them","who?");
print "$last\n";

my $other : Selfish(["you","them","who?"]);
print "$other\n";
