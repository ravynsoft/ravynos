#!/usr/bin/perl -w

use strict;
use Test::More tests => 3;

my $Has_PH = $] < 5.009;
my $Field = $Has_PH ? "pseudo-hash field" : "class field";

{
    package Parent;
    use fields qw(this that);
    sub new { fields::new(shift) }
}

{
    package Child;
    use base qw(Parent);
}

my Child $obj = Child->new;

eval q(return; my Child $obj3 = $obj; $obj3->{notthere} = "");
like $@, 
    qr/^No such .*field "notthere" in variable \$obj3 of type Child/,
    "Compile failure of undeclared fields (helem)";

# Slices
# We should get compile time failures field name typos
SKIP: {
    skip("Pseudo-hashes do not support compile-time slice checks", 2)
        if $Has_PH;

    eval q(return; my Child $obj3 = $obj; my $k; @$obj3{$k,'notthere'} = ());
    like $@, 
        qr/^No such .*field "notthere" in variable \$obj3 of type Child/,
        "Compile failure of undeclared fields (hslice)";

    eval q(return; my Child $obj3 = $obj; my $k; @{$obj3}{$k,'notthere'} = ());
    like 
        $@, qr/^No such .*field "notthere" in variable \$obj3 of type Child/,
        "Compile failure of undeclared fields (hslice (block form))";
}
