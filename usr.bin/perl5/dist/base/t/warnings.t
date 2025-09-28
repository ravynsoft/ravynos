#!/usr/bin/perl -w

use strict;

use Test::More tests => 1;

my $warnings;
BEGIN {
    $SIG{__WARN__} = sub { $warnings = join '', @_ };
}

{
    package Foo;
    use fields qw(thing);
}

{
    package Bar;
    use fields qw(stuff);
    use base qw(Foo);
}

::like $warnings,
       '/^Bar is inheriting from Foo but already has its own fields!/',
       'Inheriting from a base with protected fields warns';
