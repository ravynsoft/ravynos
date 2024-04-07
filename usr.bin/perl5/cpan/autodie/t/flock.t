#!/usr/bin/perl -w
use strict;
use Test::More;
use Fcntl qw(:flock);
use POSIX qw(EWOULDBLOCK EAGAIN);
use Config;

require Fatal;

my $EWOULDBLOCK = eval { EWOULDBLOCK() }
                  || $Fatal::_EWOULDBLOCK{$^O}
                  || plan skip_all => "EWOULDBLOCK not defined on this system";

my $try_EAGAIN = ($^O eq 'linux' and $Config{archname} =~ /hppa|parisc/) ? 1 : 0;
my $EAGAIN = eval { EAGAIN() };

my ($self_fh, $self_fh2);

eval {
    use autodie;
    open($self_fh,  '<', $0);
    open($self_fh2, '<', $0);
    open(SELF,      '<', $0);
};

if ($@) {
    plan skip_all => "Cannot lock this test on this system.";
}

my $flock_return = eval { flock($self_fh, LOCK_EX | LOCK_NB); };

if (not $flock_return) {
    plan skip_all => "flock on my own test not supported on this system.";
}

my $flock_return2 = flock($self_fh2, LOCK_EX | LOCK_NB);

if ($flock_return2) {
    plan skip_all => "this test requires locking a file twice with ".
                     "different filehandles to fail";
}

$flock_return = flock($self_fh, LOCK_UN);

if (not $flock_return) {
    plan skip_all => "Odd, I can't unlock a file with flock on this system.";
}

# If we're here, then we can lock and unlock our own file.

plan 'no_plan';

ok( flock($self_fh, LOCK_EX | LOCK_NB), "Test file locked");

my $return;

eval {
    use autodie qw(flock);
    $return = flock($self_fh2, LOCK_EX | LOCK_NB);
};

if (!$try_EAGAIN) {
    is($!+0, $EWOULDBLOCK, "Double-flocking should be EWOULDBLOCK");
} else {
    ok($!+0 == $EWOULDBLOCK || $!+0 == $EAGAIN, "Double-flocking should be EWOULDBLOCK or EAGAIN");
}
ok(!$return, "flocking a file twice should fail");
is($@, "", "Non-blocking flock should not fail on EWOULDBLOCK");

__END__

# These are old tests which I'd love to resurrect, but they need
# a reliable way of getting flock to throw exceptions but with
# minimal blocking.  They may turn into author tests.

eval {
    use autodie;
    flock($self_fh2, LOCK_EX | LOCK_NB);
};

ok($@, "Locking a file twice throws an exception with vanilla autodie");
isa_ok($@, "autodie::exception", "Exception is from autodie::exception");

like($@,   qr/LOCK_EX/, "error message contains LOCK_EX switch");
like($@,   qr/LOCK_NB/, "error message contains LOCK_NB switch");
unlike($@, qr/GLOB/   , "error doesn't include ugly GLOB mention");

eval {
    use autodie;
    flock(SELF, LOCK_EX | LOCK_NB);
};

ok($@, "Locking a package filehanlde twice throws exception with vanilla autodie");
isa_ok($@, "autodie::exception", "Exception is from autodie::exception");

like($@,   qr/LOCK_EX/, "error message contains LOCK_EX switch");
like($@,   qr/LOCK_NB/, "error message contains LOCK_NB switch");
like($@,   qr/SELF/   , "error mentions actual filehandle name.");
