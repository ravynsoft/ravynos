#!./perl

use strict;
use warnings;

use List::Util qw(first);
use Test::More;
plan tests => 24;
my $v;

ok(defined &first, 'defined');

$v = first { 8 == ($_ - 1) } 9,4,5,6;
is($v, 9, 'one more than 8');

$v = first { 0 } 1,2,3,4;
is($v, undef, 'none match');

$v = first { 0 };
is($v, undef, 'no args');

$v = first { $_->[1] le "e" and "e" le $_->[2] }
    [qw(a b c)], [qw(d e f)], [qw(g h i)];
is_deeply($v, [qw(d e f)], 'reference args');

# Check that eval{} inside the block works correctly
my $i = 0;
$v = first { eval { die }; ($i == 5, $i = $_)[0] } 0,1,2,3,4,5,5;
is($v, 5, 'use of eval');

$v = eval { first { die if $_ } 0,0,1 };
is($v, undef, 'use of die');

sub foobar {  first { !defined(wantarray) || wantarray } "not ","not ","not " }

($v) = foobar();
is($v, undef, 'wantarray');

# Can we leave the sub with 'return'?
$v = first {return ($_>6)} 2,4,6,12;
is($v, 12, 'return');

# ... even in a loop?
$v = first {while(1) {return ($_>6)} } 2,4,6,12;
is($v, 12, 'return from loop');

# Does it work from another package?
{ package Foo;
  ::is(List::Util::first(sub{$_>4},(1..4,24)), 24, 'other package');
}

# Can we undefine a first sub while it's running?
sub self_immolate {undef &self_immolate; 1}
eval { $v = first \&self_immolate, 1,2; };
like($@, qr/^Can't undef active subroutine/, "undef active sub");

# Redefining an active sub should not fail, but whether the
# redefinition takes effect immediately depends on whether we're
# running the Perl or XS implementation.

sub self_updating {
  no warnings 'redefine';
  *self_updating = sub{1};
  1
}
eval { $v = first \&self_updating, 1,2; };
is($@, '', 'redefine self');

{ my $failed = 0;

    sub rec { my $n = shift;
        if (!defined($n)) {  # No arg means we're being called by first()
            return 1; }
        if ($n<5) { rec($n+1); }
        else { $v = first \&rec, 1,2; }
        $failed = 1 if !defined $n;
    }

    rec(1);
    ok(!$failed, 'from active sub');
}

# Calling a sub from first should leave its refcount unchanged.
SKIP: {
    skip("No Internals::SvREFCNT", 1) if !defined &Internals::SvREFCNT;
    sub huge {$_>1E6}
    my $refcnt = &Internals::SvREFCNT(\&huge);
    $v = first \&huge, 1..6;
    is(&Internals::SvREFCNT(\&huge), $refcnt, "Refcount unchanged");
}

# These tests are only relevant for the real multicall implementation. The
# pseudo-multicall implementation behaves differently.
SKIP: {
    $List::Util::REAL_MULTICALL ||= 0; # Avoid use only once
    skip("Poor man's MULTICALL can't cope", 2)
      if !$List::Util::REAL_MULTICALL;

    # Can we goto a label from the 'first' sub?
    eval {()=first{goto foo} 1,2; foo: 1};
    like($@, qr/^Can't "goto" out of a pseudo block/, "goto label");

    # Can we goto a subroutine?
    eval {()=first{goto sub{}} 1,2;};
    like($@, qr/^Can't goto subroutine from a sort sub/, "goto sub");
}

use constant XSUBC_TRUE  => 1;
use constant XSUBC_FALSE => 0;

is first(\&XSUBC_TRUE,  42, 1, 2, 3), 42,    'XSUB callbacks';
is first(\&XSUBC_FALSE, 42, 1, 2, 3), undef, 'XSUB callbacks';


eval { &first(1) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &first(1,2) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &first(qw(a b)) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &first([],1,2,3) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &first(+{},1,2,3) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');

