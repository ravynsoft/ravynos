#!/usr/bin/perl -w
use strict;
use Test::More;
use autodie;

use constant SYSINIT => 1;

if ($^O eq 'MSWin32') {
    plan skip_all => "Can't send signals to own process on recent versions of Windows.";
}

if (not CORE::kill(0,$$)) {
    plan skip_all => "Can't send signals to own process on this system.";
}

if (CORE::kill(0, SYSINIT)) {
    plan skip_all => "Can unexpectedly signal process 1. Won't run as root.";
}

$SIG{HUP} = sub { }; # Ignore SIGHUP

plan tests => 6;

eval { my $rv = kill(0, $$); };
is($@, '', "Signalling self is fine");

eval { kill('HUP', $$); };
is($@, '', "Kill with non-zero signal, in void context is ok");

eval { kill(0, SYSINIT) };
isa_ok($@, 'autodie::exception', "kill 0 should die if called in void context");

eval { my $rv = kill(0, SYSINIT) };
is($@, '', "kill 0 should never die if called in scalar context");

eval { my $rv = kill('HUP', $$, SYSINIT) };
isa_ok($@, 'autodie::exception', 'kill exception on single failure.');
is($@->return, 1, "kill fails correctly on a 'true' failure.");
