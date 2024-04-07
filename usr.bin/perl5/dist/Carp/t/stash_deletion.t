#!/usr/bin/env perl
use strict;
use warnings;
use Test::More tests => 15;

use Carp;

{
    my $sub = eval <<'EVAL';
package Die;
sub {
#line 1 foo
    die "blah";
}
EVAL
    ok(!$@);
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
    {
        no strict 'refs';
        delete ${'::'}{'Die::'};
    }
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
}

{
    my $sub = eval <<'EVAL';
package Confess;
sub {
#line 1 foo
    Carp::confess("blah");
}
EVAL
    ok(!$@);
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
    {
        no strict 'refs';
        delete ${'::'}{'Confess::'};
    }
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
}

{
    my $sub = eval <<'EVAL';
package CroakHelper;
sub x {
    Carp::croak("blah");
}
package Croak;
sub {
#line 1 foo
    CroakHelper::x();
}
EVAL
    ok(!$@);
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
    {
        no strict 'refs';
        delete ${'::'}{'Croak::'};
    }
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
    {
        no strict 'refs';
        delete ${'::'}{'CroakHelper::'};
    }
    eval { $sub->() };
    like($@, qr/^blah at foo line 1/);
}

{
    # the amount of information available and how it is displayed varies quite
    # a bit depending on the version of perl (specifically, what caller returns
    # in that version), so there is a bit of fiddling around required to handle
    # that
    my $unknown_pat = qr/__ANON__::/;
    $unknown_pat = qr/$unknown_pat|\(unknown\)/
        if $] < 5.014;

    my $sub = eval <<'EVAL';
package SubHelper;
sub x {
    Carp::confess("blah");
}
package Sub;
sub {
#line 1 foo
    SubHelper::x();
}
EVAL
    ok(!$@);
    eval { $sub->() };
    unlike($@, qr/$unknown_pat/);
    {
        no strict 'refs';
        delete ${'::'}{'Sub::'};
    }
    eval { $sub->() };
    like($@, qr/$unknown_pat|Sub::/);
    unlike($@, qr/$unknown_pat.*$unknown_pat/s);
    {
        no strict 'refs';
        delete ${'::'}{'SubHelper::'};
    }
    eval { $sub->() };
    like($@, qr/(?:$unknown_pat|SubHelper::).*(?:$unknown_pat|Sub::)/s);
}
