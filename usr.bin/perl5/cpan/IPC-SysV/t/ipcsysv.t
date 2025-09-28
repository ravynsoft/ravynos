################################################################################
#
#  Version 2.x, Copyright (C) 2007-2013, Marcus Holland-Moritz <mhx@cpan.org>.
#  Version 1.x, Copyright (C) 1999, Graham Barr <gbarr@pobox.com>.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use strict;
use warnings;

our %Config;
BEGIN {
  require Test::More; Test::More->import;
  require Config; Config->import;

  if ($ENV{'PERL_CORE'} && $Config{'extensions'} !~ m[\bIPC/SysV\b]) {
    plan(skip_all => 'IPC::SysV was not built');
  }
}

if ($Config{'d_sem'} ne 'define') {
  plan(skip_all => '$Config{d_sem} undefined');
}
elsif ($Config{'d_msg'} ne 'define') {
  plan(skip_all => '$Config{d_msg} undefined');
}

plan(tests => 39);

# These constants are common to all tests.
# Later the sem* tests will import more for themselves.

use IPC::SysV qw(IPC_PRIVATE IPC_NOWAIT IPC_STAT IPC_RMID S_IRWXU);

{
  my $did_diag = 0;

  sub do_sys_diag
  {
    return if $did_diag++;

    if ($^O eq 'cygwin') {
      diag(<<EOM);

It may be that the cygserver service isn't running.

EOM

      diag(<<EOM) unless exists $ENV{CYGWIN} && $ENV{CYGWIN} eq 'server';
You also may have to set the CYGWIN environment variable
to 'server' before running the test suite:

  export CYGWIN=server

EOM
    }
    else {
      diag(<<EOM);

It may be that your kernel does not have SysV IPC configured.

EOM

      diag(<<EOM) if $^O eq 'freebsd';
You must have following options in your kernel:

options         SYSVSHM
options         SYSVSEM
options         SYSVMSG

See config(8).

EOM
    }
  }
}

{
  my $SIGSYS_caught = 0;

  sub skip_or_die
  {
    my($what, $why) = @_;
    if ($SIGSYS_caught) {
      do_sys_diag();
      return "$what failed: SIGSYS caught";
    }
    my $info = "$what failed: $why";
    if ($why == &IPC::SysV::ENOSPC || $why == &IPC::SysV::ENOSYS ||
        $why == &IPC::SysV::ENOMEM || $why == &IPC::SysV::EACCES) {
      do_sys_diag() if $why == &IPC::SysV::ENOSYS;
      return $info;
    }
    die $info;
  }

  sub catchsig
  {
    my $code = shift;
    if (exists $SIG{SYS}) {
      local $SIG{SYS} = sub { $SIGSYS_caught++ };
      return $code->();
    }
    return $code->();
  }
}

# FreeBSD and cygwin are known to throw this if there's no SysV IPC
# in the kernel or the cygserver isn't running properly.
if (exists $SIG{SYS}) {  # No SIGSYS with older perls...
  $SIG{SYS} = sub {
    do_sys_diag();
    diag('Bail out! SIGSYS caught');
    exit(1);
  };
}

my $msg;

my $perm = S_IRWXU;
my $test_name;
my $N = do { my $foo = eval { pack "L!", 0 }; $@ ? '' : '!' };

SKIP: {
  skip('lacking d_msgget d_msgctl d_msgsnd d_msgrcv', 6) unless
      $Config{'d_msgget'} eq 'define' &&
      $Config{'d_msgctl'} eq 'define' &&
      $Config{'d_msgsnd'} eq 'define' &&
      $Config{'d_msgrcv'} eq 'define';

  $msg = catchsig(sub { msgget(IPC_PRIVATE, $perm) });

  # Very first time called after machine is booted value may be 0 
  unless (defined $msg && $msg >= 0) {
    skip(skip_or_die('msgget', $!), 6);
  }

  pass('msgget IPC_PRIVATE S_IRWXU');

  #Putting a message on the queue
  my $msgtype = 1;
  my $msgtext = "hello";

  my $test2bad;
  my $test5bad;
  my $test6bad;

  $test_name = 'queue a message';

  if (msgsnd($msg, pack("L$N a*", $msgtype, $msgtext), IPC_NOWAIT)) {
    pass($test_name);
  }
  else {
    fail($test_name);
    $test2bad = 1;
    diag(<<EOM);
The failure of the subtest #2 may indicate that the message queue
resource limits either of the system or of the testing account
have been reached.  Error message "Operating would block" is
usually indicative of this situation.  The error message was now:
"$!"

You can check the message queues with the 'ipcs' command and
you can remove unneeded queues with the 'ipcrm -q id' command.
You may also consider configuring your system or account
to have more message queue resources.

Because of the subtest #2 failing also the substests #5 and #6 will
very probably also fail.
EOM
  }

  my $data = '';
  ok(msgctl($msg, IPC_STAT, $data), 'msgctl IPC_STAT call');

  cmp_ok(length($data), '>', 0, 'msgctl IPC_STAT data');

  $test_name = 'message get call';

  my $msgbuf = '';
  if (msgrcv($msg, $msgbuf, 256, 0, IPC_NOWAIT)) {
    pass($test_name);
  }
  else {
    fail($test_name);
    $test5bad = 1;
  }
  if ($test5bad && $test2bad) {
    diag(<<EOM);
This failure was to be expected because the subtest #2 failed.
EOM
  }

  $test_name = 'message get data';

  my($rmsgtype, $rmsgtext);
  ($rmsgtype, $rmsgtext) = unpack("L$N a*", $msgbuf);

  if ($rmsgtype == $msgtype && $rmsgtext eq $msgtext) {
    pass($test_name);
  }
  else {
    fail($test_name);
    $test6bad = 1;
  }

  if ($test6bad && $test2bad) {
    print <<EOM;
This failure was to be expected because the subtest #2 failed.
EOM
  }
}

my $sem;

SKIP: {
  skip('lacking d_semget d_semctl', 11) unless
      $Config{'d_semget'} eq 'define' &&
      $Config{'d_semctl'} eq 'define';

  use IPC::SysV qw(IPC_CREAT GETALL SETALL);

  # FreeBSD's default limit seems to be 9
  my $nsem = 5;

  $sem = catchsig(sub { semget(IPC_PRIVATE, $nsem, $perm | IPC_CREAT) });

  # Very first time called after machine is booted value may be 0 
  unless (defined $sem && $sem >= 0) {
    skip(skip_or_die('semget', $!), 11);
  }

  pass('sem acquire');

  my $data = '';
  ok(semctl($sem, 0, IPC_STAT, $data), 'sem data call');

  cmp_ok(length($data), '>', 0, 'sem data len');

  ok(semctl($sem, 0, SETALL, pack("s$N*", (0) x $nsem)), 'set all sems');

  $data = "";
  ok(semctl($sem, 0, GETALL, $data), 'get all sems');

  is(length($data), length(pack("s$N*", (0) x $nsem)), 'right length');

  my @data = unpack("s$N*", $data);

  my $adata = "0" x $nsem;

  is(scalar(@data), $nsem, 'right amount');
  cmp_ok(join("", @data), 'eq', $adata, 'right data');

  my $poke = 2;

  $data[$poke] = 1;
  ok(semctl($sem, 0, SETALL, pack("s$N*", @data)), 'poke it');
  
  $data = "";
  ok(semctl($sem, 0, GETALL, $data), 'and get it back');

  @data = unpack("s$N*", $data);
  my $bdata = "0" x $poke . "1" . "0" x ($nsem - $poke - 1);

  cmp_ok(join("", @data), 'eq', $bdata, 'changed');
}

SKIP: {
  skip('lacking d_shm', 11) unless
      $Config{'d_shm'} eq 'define';

  use IPC::SysV qw(shmat shmdt memread memwrite ftok);

  my $shm = catchsig(sub { shmget(IPC_PRIVATE, 4, S_IRWXU) });

  # Very first time called after machine is booted value may be 0 
  unless (defined $shm && $shm >= 0) {
    skip(skip_or_die('shmget', $!), 11);
  }

  pass("shm acquire");

  ok(shmwrite($shm, pack("N", 0xdeadbeef), 0, 4), 'shmwrite(0xdeadbeef)');

  my $addr = shmat($shm, undef, 0);
  ok(defined $addr, 'shmat');

  is(unpack("N", unpack("P4", $addr)), 0xdeadbeef, 'read shm by addr');

  ok(defined shmctl($shm, IPC_RMID, 0), 'shmctl(IPC_RMID)');

  my $var = '';
  ok(memread($addr, $var, 0, 4), 'memread($var)');

  is(unpack("N", $var), 0xdeadbeef, 'read shm by memread');

  ok(memwrite($addr, pack("N", 0xbadc0de5), 0, 4), 'memwrite(0xbadc0de5)');

  is(unpack("N", unpack("P4", $addr)), 0xbadc0de5, 'read modified shm by addr');

  is(shmat(-1, undef, 0), undef, 'shmat illegal id fails');

  ok(defined shmdt($addr), 'shmdt');
}

SKIP: {
  skip('lacking d_shm', 11) unless
      $Config{'d_shm'} eq 'define';

  use IPC::SysV qw(ftok);

  my $key1i = ftok($0);
  my $key1e = ftok($0, 1);

  ok(defined $key1i, 'ftok implicit project id');
  ok(defined $key1e, 'ftok explicit project id');
  is($key1i, $key1e, 'keys match');

  my $keyAsym = ftok($0, 'A');
  my $keyAnum = ftok($0, ord('A'));

  ok(defined $keyAsym, 'ftok symbolic project id');
  ok(defined $keyAnum, 'ftok numeric project id');
  is($keyAsym, $keyAnum, 'keys match');

  my $two = '2';
  my $key1 = ftok($0, 2);
  my $key2 = ftok($0, ord('2'));
  my $key3 = ftok($0, $two);
  my $key4 = ftok($0, int($two));

  is($key1, $key4, 'keys match');
  isnt($key1, $key2, 'keys do not match');
  is($key2, $key3, 'keys match');

  eval { my $foo = ftok($0, 'AA') };
  ok(index($@, 'invalid project id') >= 0, 'ftok error');

  eval { my $foo = ftok($0, 3.14159) };
  ok(index($@, 'invalid project id') >= 0, 'ftok error');
}

END {
  msgctl($msg, IPC_RMID, 0)    if defined $msg;
  semctl($sem, 0, IPC_RMID, 0) if defined $sem;
}
