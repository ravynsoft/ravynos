#!perl

use strict;
use warnings;

require "../../t/test.pl";

use XS::APItest;

# clone_with_stack creates a clone of the perl interpreter including
# the stack, then destroys the original interpreter and runs the
# remaining code using the new one.
# This is like doing a pseudo-fork and exiting the parent.

use Config;
if (not $Config{'useithreads'}) {
    skip_all("clone_with_stack requires threads");
}

plan(8);

fresh_perl_is( <<'----', <<'====', undef, "minimal clone_with_stack" );
use XS::APItest;
clone_with_stack();
print "ok\n";
----
ok
====

fresh_perl_is( <<'----', <<'====', undef, "inside a subroutine" );
use XS::APItest;
sub f {
    clone_with_stack();
}
f();
print "ok\n";
----
ok
====

{
    local our $TODO = "clone_with_stack inside a begin block";
    fresh_perl_is( <<'----', <<'====', undef, "inside a BEGIN block" );
use XS::APItest;
BEGIN {
    clone_with_stack();
}
print "ok\n";
----
ok
====

}

{
    fresh_perl_is( <<'----', <<'====', undef, "clone stack" );
use XS::APItest;
sub f {
    clone_with_stack();
    0..4;
}
print 'X-', 'Y-', join(':', f()), "-Z\n";
----
X-Y-0:1:2:3:4-Z
====

}

{
    fresh_perl_is( <<'----', <<'====', undef, "with a lexical sub" );
use XS::APItest;
use experimental lexical_subs=>;
my sub f { print "42\n" }
clone_with_stack();
f();
----
42
====

}

{
    fresh_perl_is( <<'----', <<'====', undef, "with localised stuff" );
use XS::APItest;
$s = "outer";
$a[0] = "anterior";
$h{k} = "hale";
{
    local $s = "inner";
    local $a[0] = 'posterior';
    local $h{k} = "halt";
    clone_with_stack();
}
print "scl: $s\n";
print "ary: $a[0]\n";
print "hsh: $h{k}\n";
----
scl: outer
ary: anterior
hsh: hale
====

}

{
    fresh_perl_is( <<'----', <<'====', undef, "inside a loop inside a fn" );
use XS::APItest;
my $a = 'aa';
sub f {
    my $b = 'bb';
    my @c;
    my $d = 'dd';
    for my $d (0..4) {
	clone_with_stack() if $d == 2;
	push @c, $d;
    }
    return @c, $d;
}
print "X-$a-", join(':', f()), "-Z\n";
----
X-aa-0:1:2:3:4:dd-Z
====

}

{
    fresh_perl_is( <<'----', <<'====', undef, "inside fn inside a loop inside a fn" );
use XS::APItest;
my $a = 'aa';

sub g {
    my $e = 'ee';
    my $f = 'ff';
    clone_with_stack();
}

sub f {
    my $b = 'bb';
    my @c;
    my $d = 'dd';
    for my $d (0..4) {
	g() if $d == 2;
	push @c, $d;
    }
    return @c, $d;
}
print "X-$a-", join(':', f()), "-Z\n";
----
X-aa-0:1:2:3:4:dd-Z
====

}
