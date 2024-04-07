#!./perl

use strict;
use warnings;

use Sub::Util qw( prototype set_prototype );
use Test::More tests => 13;

sub f { }
is( prototype('f'), undef, 'no prototype');
is( CORE::prototype('f'), undef, 'no prototype from CORE');

my $r = set_prototype('$', \&f);
is( prototype('f'), '$', 'prototype');
is( CORE::prototype('f'), '$', 'prototype from CORE');
is( $r,   \&f, 'return value');

set_prototype(undef, \&f);
is( prototype('f'), undef, 'remove prototype');

set_prototype('', \&f);
is( prototype('f'), '', 'empty prototype');

sub g (@) { }
is( prototype('g'), '@', '@ prototype');

set_prototype(undef, \&g);
is( prototype('g'), undef, 'remove prototype');

sub stub;
is( prototype('stub'), undef, 'non existing sub');

set_prototype('$$$', \&stub);
is( prototype('stub'), '$$$', 'change non existing sub');

sub f_decl ($$$$);
is( prototype('f_decl'), '$$$$', 'forward declaration');

set_prototype('\%', \&f_decl);
is( prototype('f_decl'), '\%', 'change forward declaration');
