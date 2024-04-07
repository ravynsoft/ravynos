use warnings;
use strict;

use Test::More tests => 76;

use XS::APItest qw(pad_scalar);

is pad_scalar(1, "foo"), "NOT_IN_PAD";
is pad_scalar(2, "foo"), "NOT_IN_PAD";
is pad_scalar(3, "foo"), "NOT_IN_PAD";
is pad_scalar(4, "foo"), "NOT_IN_PAD";
is pad_scalar(1, "bar"), "NOT_IN_PAD";
is pad_scalar(2, "bar"), "NOT_IN_PAD";
is pad_scalar(3, "bar"), "NOT_IN_PAD";

our $foo = "wibble";
my $bar = "wobble";
is pad_scalar(1, "foo"), "NOT_MY";
is pad_scalar(2, "foo"), "NOT_MY";
is pad_scalar(3, "foo"), "NOT_MY";
is pad_scalar(4, "foo"), "NOT_MY";
is pad_scalar(1, "bar"), "wobble";
is pad_scalar(2, "bar"), "wobble";
is pad_scalar(3, "bar"), "wobble";

sub aa($);
sub aa($) {
    my $xyz;
    ok \pad_scalar(1, "xyz") == \$xyz;
    ok \pad_scalar(2, "xyz") == \$xyz;
    ok \pad_scalar(3, "xyz") == \$xyz;
    aa(0) if $_[0];
    ok \pad_scalar(1, "xyz") == \$xyz;
    ok \pad_scalar(2, "xyz") == \$xyz;
    ok \pad_scalar(3, "xyz") == \$xyz;
    is pad_scalar(1, "bar"), "wobble";
    is pad_scalar(2, "bar"), "wobble";
    is pad_scalar(3, "bar"), "wobble";
}
aa(1);

sub bb() {
    my $counter = 0;
    my $foo = \$counter;
    return sub {
	ok pad_scalar(1, "foo") == \pad_scalar(1, "counter");
	ok pad_scalar(2, "foo") == \pad_scalar(1, "counter");
	ok pad_scalar(3, "foo") == \pad_scalar(1, "counter");
	ok pad_scalar(4, "foo") == \pad_scalar(1, "counter");
	if(pad_scalar(1, "counter") % 3 == 0) {
	    return pad_scalar(1, "counter")++;
	} elsif(pad_scalar(1, "counter") % 3 == 0) {
	    return pad_scalar(2, "counter")++;
	} else {
	    return pad_scalar(3, "counter")++;
	}
    };
}
my $a = bb();
my $b = bb();
is $a->(), 0;
is $a->(), 1;
is $a->(), 2;
is $a->(), 3;
is $b->(), 0;
is $b->(), 1;
is $a->(), 4;
is $b->(), 2;

is pad_scalar(1, "foo"), "NOT_MY";
is pad_scalar(2, "foo"), "NOT_MY";
is pad_scalar(3, "foo"), "NOT_MY";
is pad_scalar(4, "foo"), "NOT_MY";

1;
