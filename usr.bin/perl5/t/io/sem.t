#!perl

BEGIN {
  chdir 't' if -d 't';

  require "./test.pl";
  set_up_inc( '../lib' ) if -d '../lib' && -d '../ext';
  require Config; import Config;

  if ($ENV{'PERL_CORE'} && $Config{'extensions'} !~ m[\bIPC/SysV\b]) {
    skip_all('-- IPC::SysV was not built');
  }
  skip_all_if_miniperl();
  if ($Config{'d_sem'} ne 'define') {
    skip_all('-- $Config{d_sem} undefined');
  }
}

use strict;
use warnings;
our $TODO;

use sigtrap qw/die normal-signals error-signals/;
use IPC::SysV qw/ IPC_PRIVATE S_IRUSR S_IWUSR IPC_RMID SETVAL GETVAL SETALL GETALL IPC_CREAT IPC_STAT /;

my $id;
my $nsem = 10;
my $ignored = 0;
END { semctl $id, 0, IPC_RMID, 0 if defined $id }

{
    local $SIG{SYS} = sub { skip_all("SIGSYS caught") } if exists $SIG{SYS};
    $id = semget IPC_PRIVATE, $nsem, S_IRUSR | S_IWUSR | IPC_CREAT;
}

if (not defined $id) {
    my $info = "semget failed: $!";
    if ($! == &IPC::SysV::ENOSPC || $! == &IPC::SysV::ENOSYS ||
	$! == &IPC::SysV::ENOMEM || $! == &IPC::SysV::EACCES) {
        skip_all($info);
    }
    else {
        die $info;
    }
}
else {
    plan(tests => 22);
    pass('acquired semaphore');
}

my @warnings;
$SIG{__WARN__} = sub { push @warnings, "@_"; print STDERR @_; };
{ # [perl #120635] 64 bit big-endian semctl SETVAL bug
    ok(semctl($id, $ignored, SETALL, pack("s!*",(0)x$nsem)),
       "Initialize all $nsem semaphores to zero");

    my $sem2set = 3;
    my $semval = 192;
    ok(semctl($id, $sem2set, SETVAL, $semval),
       "Set semaphore $sem2set to $semval");

    my $semvals;
    ok(semctl($id, $ignored, GETALL, $semvals),
       'Get current semaphore values');

    my @semvals = unpack("s!*", $semvals);
    is(scalar(@semvals), $nsem, 
       "Make sure we get back statuses for all $nsem semaphores");

    is($semvals[$sem2set], $semval, 
       "Checking value of semaphore $sem2set");

    is(semctl($id, $sem2set, GETVAL, $ignored), $semval,
       "Check value via GETVAL");

    # check utf-8 flag handling
    # first that we reset it on a fetch
    utf8::upgrade($semvals);
    ok(semctl($id, $ignored, GETALL, $semvals),
       "fetch into an already UTF-8 buffer");
    @semvals = unpack("s!*", $semvals);
    is($semvals[$sem2set], $semval,
       "Checking value of semaphore $sem2set after fetch into originally UTF-8 buffer");

    # second that we treat it as bytes on input
    @semvals = ( 0 ) x $nsem;
    $semvals[$sem2set] = $semval + 1;
    $semvals = pack "s!*", @semvals;
    utf8::upgrade($semvals);
    # eval{} since it would crash due to the UTF-8 form being longer
    ok(eval { semctl($id, $ignored, SETALL, $semvals) },
       "set all semaphores from an upgraded string");
    # undef here to test it doesn't warn
    is(semctl($id, $sem2set, GETVAL, undef), $semval+1,
       "test value set from UTF-8");

    # third, that we throw on a code point above 0xFF
    substr($semvals, 0, 1) = chr(0x101);
    ok(!eval { semctl($id, $ignored, SETALL, $semvals); 1 },
       "throws on code points above 0xff");
    like($@, qr/Wide character/, "with the expected error");

    {
        # semop tests
        ok(semctl($id, $sem2set, SETVAL, 0),
           "reset our working entry");
        # sanity check without UTF-8
        my $op = pack "s!*", $sem2set, $semval, 0;
        ok(semop($id, $op), "add to entry $sem2set");
        is(semctl($id, $sem2set, GETVAL, 0), $semval,
           "check it added to the entry");
        utf8::upgrade($op);
        # unlike semctl this doesn't throw on a bad size, so we don't need an
        # eval with the buggy code
        ok(semop($id, $op), "add more to entry $sem2set (UTF-8)");
        is(semctl($id, $sem2set, GETVAL, 0), $semval*2,
           "check it added to the entry");

        substr($op, 0, 1) = chr(0x101);
        ok(!eval { semop($id, $op); 1 },
           "test semop throws if the op string isn't 'bytes'");
        like($@, qr/Wide character/, "with the expected error");
    }
}

{
    my $stat;
    # shouldn't warn
    semctl($id, $ignored, IPC_STAT, $stat);
    ok(defined $stat, "it statted");
}

is(scalar @warnings, 0, "no warnings");
