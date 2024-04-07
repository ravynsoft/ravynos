#!/usr/bin/perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 4);

use mro;

{
    package Proxy;
    our @ISA = qw//;
    sub next_proxy { goto &next::method }
    sub maybe_proxy { goto &maybe::next::method }
    sub can_proxy { goto &next::can }

    package TBase;
    our @ISA = qw//;
    sub foo { 42 }
    sub bar { 24 }
    # baz doesn't exist intentionally
    sub quux { 242 }

    package TTop;
    our @ISA = qw/TBase/;
    sub foo { shift->Proxy::next_proxy() }
    sub bar { shift->Proxy::maybe_proxy() }
    sub baz { shift->Proxy::maybe_proxy() }
    sub quux { shift->Proxy::can_proxy()->() }
}

is(TTop->foo, 42, 'proxy next::method via goto');
is(TTop->bar, 24, 'proxy maybe::next::method via goto');
ok(!TTop->baz, 'proxy maybe::next::method via goto with no method');
is(TTop->quux, 242, 'proxy next::can via goto');
