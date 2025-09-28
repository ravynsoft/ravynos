use warnings;
use strict;

use Test::More tests => 3;

use XS::APItest qw(establish_cleanup);

my @events;

# unwinding on local return from sub

sub aa {
    push @events, "aa0";
    establish_cleanup sub { push @events, "bb0" };
    push @events, "aa1";
    "aa2";
}

sub cc {
    push @events, "cc0";
    push @events, [ "cc1", aa() ];
    push @events, "cc2";
    "cc3";
}

@events = ();
push @events, "dd0";
push @events, [ "dd1", cc() ];
is_deeply \@events, [
    "dd0",
    "cc0",
    "aa0",
    "aa1",
    "bb0",
    [ "cc1", "aa2" ],
    "cc2",
    [ "dd1", "cc3" ],
];

# unwinding on local return from format

sub ff { push @events, "ff0" }

format EE =
@<<
((push @events, "ee0"), (establish_cleanup \&ff), (push @events, "ee1"), "ee2")
.

sub gg {
    push @events, "gg0";
    write(EE);
    push @events, "gg1";
    "gg2";
}

@events = ();
open EE, ">", \(my $ee);
push @events, "hh0";
push @events, [ "hh1", gg() ];
close EE;
is_deeply \@events, [
    "hh0",
    "gg0",
    "ee0",
    "ee1",
    "ff0",
    "gg1",
    [ "hh1", "gg2" ],
];

# unwinding on die

sub pp {
    my $value = eval {
	push @events, "pp0";
	establish_cleanup sub { push @events, "qq0" };
	push @events, "pp1";
	die "pp2\n";
	push @events, "pp3";
	"pp4";
    };
    [ "pp5", $value, $@ ];
}

@events = ();
push @events, "rr0";
push @events, [  "rr1", pp() ];
is_deeply \@events, [
	"rr0",
	"pp0",
	"pp1",
	"qq0",
	[ "rr1", [ "pp5", undef, "pp2\n" ] ],
];

1;
