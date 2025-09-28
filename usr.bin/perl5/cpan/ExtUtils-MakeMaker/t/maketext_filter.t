#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use Test::More tests => 6;

use ExtUtils::MakeMaker;
use ExtUtils::MM_VMS;

sub test_filter {
    my($text, $vms_text) = @_;

    local $Test::Builder::Level = $Test::Builder::Level + 1;
    is( ExtUtils::MM_Any->maketext_filter($text), $text,     'default filter' );
    is( ExtUtils::MM_VMS->maketext_filter($text), $vms_text, 'VMS filter' );
}


# VMS filter puts a space after the target
test_filter(<<'END', <<'VMS');
foo: bar
    thing: splat
END
foo : bar
    thing: splat
VMS


# And it does it for all targets
test_filter(<<'END', <<'VMS');
foo: bar
    thing: splat

up: down
    yes
END
foo : bar
    thing: splat

up : down
    yes
VMS


# And it doesn't mess with macros
test_filter(<<'END', <<'VMS');
CLASS=Foo: Bar

target: stuff
    $(PROGRAM) And::Stuff
END
CLASS=Foo: Bar

target : stuff
    $(PROGRAM) And::Stuff
VMS
