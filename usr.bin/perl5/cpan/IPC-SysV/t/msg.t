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
  if ($ENV{'PERL_CORE'}) {
    chdir 't' if -d 't';
    @INC = '../lib' if -d '../lib' && -d '../ext';
  }

  require Test::More; Test::More->import;
  require Config; Config->import;

  if ($ENV{'PERL_CORE'} && $Config{'extensions'} !~ m[\bIPC/SysV\b]) {
    plan(skip_all => 'IPC::SysV was not built');
  }
}

if ($Config{'d_sem'} ne 'define') {
  plan(skip_all => '$Config{d_sem} undefined');
} elsif ($Config{'d_msg'} ne 'define') {
  plan(skip_all => '$Config{d_msg} undefined');
}

use IPC::SysV qw(IPC_PRIVATE IPC_RMID IPC_NOWAIT IPC_STAT S_IRWXU S_IRWXG S_IRWXO);

use IPC::Msg;
#Creating a message queue

my $msq = sub {
  my $code = shift;
  if (exists $SIG{SYS}) {
    local $SIG{SYS} = sub { plan(skip_all => "SIGSYS caught") };
    return $code->();
  }
  return $code->();
}->(sub { IPC::Msg->new(IPC_PRIVATE, S_IRWXU | S_IRWXG | S_IRWXO) });

unless (defined $msq) {
  my $info = "IPC::Msg->new failed: $!";
  if ($! == &IPC::SysV::ENOSPC || $! == &IPC::SysV::ENOSYS ||
      $! == &IPC::SysV::ENOMEM || $! == &IPC::SysV::EACCES) {
    plan(skip_all => $info);
  }
  else {
    die $info;
  }
}

plan(tests => 9);

pass('create message queue');

#Putting a message on the queue
my $test_name = 'enqueue message';

my $msgtype = 1;
my $msg = "hello";
if ($msq->snd($msgtype,$msg,IPC_NOWAIT)) {
  pass($test_name);
}
else {
  print "# snd: $!\n";
  fail($test_name);
}

#Check if there are messages on the queue
my $ds = $msq->stat;
ok($ds, 'stat');

if ($ds) {
  is($ds->qnum, 1, 'qnum');
}
else {
  fail('qnum');
}

#Retrieving a message from the queue
my $rmsg;
my $rmsgtype = 0; # Give me any type
$rmsgtype = $msq->rcv($rmsg,256,$rmsgtype,IPC_NOWAIT);
is($rmsgtype, $msgtype, 'rmsgtype');
is($rmsg, $msg, 'rmsg');

$ds = $msq->stat;
ok($ds, 'stat');

if ($ds) {
  is($ds->qnum, 0, 'qnum');
}
else {
  fail('qnum');
}

END {
  ok($msq->remove, 'remove message') if defined $msq;
}
