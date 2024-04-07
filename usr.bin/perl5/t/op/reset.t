#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use strict;

plan tests => 40;

package aiieee;

sub zlopp {
    (shift =~ m?zlopp?) ? 1 : 0;
}

sub reset_zlopp {
    reset;
}

package CLINK;

sub ZZIP {
    shift =~ m?ZZIP? ? 1 : 0;
}

sub reset_ZZIP {
    reset;
}

package main;

is(aiieee::zlopp(""), 0, "mismatch doesn't match");
is(aiieee::zlopp("zlopp"), 1, "match matches first time");
is(aiieee::zlopp(""), 0, "mismatch doesn't match");
is(aiieee::zlopp("zlopp"), 0, "match doesn't match second time");
aiieee::reset_zlopp();
is(aiieee::zlopp("zlopp"), 1, "match matches after reset");
is(aiieee::zlopp(""), 0, "mismatch doesn't match");

aiieee::reset_zlopp();

is(aiieee::zlopp(""), 0, "mismatch doesn't match");
is(aiieee::zlopp("zlopp"), 1, "match matches first time");
is(CLINK::ZZIP(""), 0, "mismatch doesn't match");
is(CLINK::ZZIP("ZZIP"), 1, "match matches first time");
is(CLINK::ZZIP(""), 0, "mismatch doesn't match");
is(CLINK::ZZIP("ZZIP"), 0, "match doesn't match second time");
is(aiieee::zlopp(""), 0, "mismatch doesn't match");
is(aiieee::zlopp("zlopp"), 0, "match doesn't match second time");

aiieee::reset_zlopp();
is(aiieee::zlopp("zlopp"), 1, "match matches after reset");
is(aiieee::zlopp(""), 0, "mismatch doesn't match");

is(CLINK::ZZIP(""), 0, "mismatch doesn't match");
is(CLINK::ZZIP("ZZIP"), 0, "match doesn't match third time");

CLINK::reset_ZZIP();
is(CLINK::ZZIP("ZZIP"), 1, "match matches after reset");
is(CLINK::ZZIP(""), 0, "mismatch doesn't match");

sub match_foo{
    "foo" =~ m?foo?;
}
match_foo();
reset "";
ok !match_foo(), 'reset "" leaves patterns alone [perl #97958]';

$scratch::a = "foo";
$scratch::a2 = "bar";
$scratch::b   = "baz";
package scratch { reset "a" }
is join("-", $scratch::a//'u', $scratch::a2//'u', $scratch::b//'u'),
   "u-u-baz",
   'reset "char"';

$scratch::a = "foo";
$scratch::a2 = "bar";
$scratch::b   = "baz";
$scratch::c    = "sea";
package scratch { reset "bc" }
is join("-", $scratch::a//'u', $scratch::a2//'u', $scratch::b//'u',
             $scratch::c//'u'),
   "foo-bar-u-u",
   'reset "chars"';

$scratch::a = "foo";
$scratch::a2 = "bar";
$scratch::b   = "baz";
$scratch::c    = "sea";
package scratch { reset "a-b" }
is join("-", $scratch::a//'u', $scratch::a2//'u', $scratch::b//'u',
             $scratch::c//'u'),
   "u-u-u-sea",
   'reset "range"';

{ no strict; ${"scratch::\0foo"} = "bar" }
$scratch::a = "foo";
package scratch { reset "\0a" }
is join("-", $scratch::a//'u', do { no strict; ${"scratch::\0foo"} }//'u'),
   "u-u",
   'reset "\0char"';

$scratch::cow = __PACKAGE__;
$scratch::qr = ${qr//};
$scratch::v  = v6;
$scratch::glob = *is;
*scratch::ro = \1;
package scratch { reset 'cqgvr' }
is join ("-", map $_//'u', $scratch::cow, $scratch::qr, $scratch::v,
                           $scratch::glob,$scratch::ro), 'u-u-u-u-1',
   'cow, qr, vstring, glob, ro test';

@scratch::an_array = 1..3;
%scratch::a_hash   = 1..4;
package scratch { reset 'a' }
is @scratch::an_array, 0, 'resetting an array';
is %scratch::a_hash,   0, 'resetting a hash';

@scratch::an_array = 1..3;
%scratch::an_array = 1..4;
*scratch::an_array = \1;
package scratch { reset 'a' }
is @scratch::an_array, 0, 'resetting array in the same gv as a ro scalar';
is @scratch::an_array, 0, 'resetting a hash in the same gv as a ro scalar';
is $scratch::an_array, 1, 'reset skips ro scalars in the same gv as av/hv';

for our $z (*_) {
    {
        local *_;
        reset "z";
        $z = 3;
        () = *_{SCALAR};
	no warnings;
        () = "$_";   # used to crash
    }
    is ref\$z, "GLOB", 'reset leaves real-globs-as-scalars as GLOBs';
    is $z, "*main::_", 'And the glob still has the right value';
}

package _128106 {
    # Crash on non-globs in the stash.
    sub u;    # stub without proto
    sub v($); # proto stub
    sub w{};  # as of 5.22, $::{w} == \&w
    $::{x} = undef;
    reset 'u-x';
    ::ok (1, "no crash on non-globs in the stash");
}

# This used to crash under threaded builds, because pmops were remembering
# their stashes by name, rather than by pointer.
fresh_perl_is( # it crashes more reliably with a smaller script
  'package bar;
   sub foo {
     m??;
     BEGIN { *baz:: = *bar::; *bar:: = *foo:: }
     # The name "bar" no langer refers to the same package
   }
   undef &foo; # so freeing the op does not remove it from the stash\'s list
   $_ = "";
   push @_, ($_) x 10000;  # and its memory is scribbled over
   reset;  # so reset on the original package tries to reset an invalid op
   print "ok\n";',
  "ok\n", {},
  "no crash if package is effectively renamed before op is freed");

sub _117941 { package _117941; reset }
delete $::{"_117941::"};
_117941();
pass("no crash when current package is freed");

undef $/;
my $prog = <DATA>;

SKIP:
{
    eval {require threads; 1} or
	skip "No threads", 4;
    foreach my $eight ('/', '?') {
	foreach my $nine ('/', '?') {
	    my $copy = $prog;
	    $copy =~ s/8/$eight/gm;
	    $copy =~ s/9/$nine/gm;
	    fresh_perl_is($copy, "pass", {},
			  "first pattern $eight$eight, second $nine$nine");
	}
    }
}

__DATA__
#!perl
use warnings;
use strict;

# Note that there are no digits in this program, other than the placeholders
sub a {
m8one8;
}
sub b {
m9two9;
}

use threads;
use threads::shared;

sub wipe {
    eval 'no warnings; sub b {}; 1' or die $@;
}

sub lock_then_wipe {
    my $l_r = shift;
    lock $$l_r;
    cond_wait($$l_r) until $$l_r eq "B";
    wipe;
    $$l_r = "C";
    cond_signal $$l_r;
}

my $lock : shared = "A";
my $r = \$lock;

my $t;
{
    lock $$r;
    $t = threads->new(\&lock_then_wipe, $r);
    wipe;
    $lock = "B";
    cond_signal $lock;
}

{
    lock $lock;
    cond_wait($lock) until $lock eq "C";
    reset;
}

$t->join;
print "pass\n";
