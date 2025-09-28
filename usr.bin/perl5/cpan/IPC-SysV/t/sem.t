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
}
elsif ($Config{'d_msg'} ne 'define') {
  plan(skip_all => '$Config{d_msg} undefined');
}

use IPC::SysV qw(
	SETALL
	IPC_PRIVATE
	IPC_CREAT
	IPC_RMID
	IPC_NOWAIT
	IPC_STAT
	S_IRWXU
	S_IRWXG
	S_IRWXO
);
use IPC::Semaphore;

# FreeBSD's default limit seems to be 9
my $nsem = 5;
my $sem = sub {
  my $code = shift;
  if (exists $SIG{SYS}) {
    local $SIG{SYS} = sub { plan(skip_all => "SIGSYS caught") };
    return $code->();
  }
  return $code->();
}->(sub { IPC::Semaphore->new(IPC_PRIVATE, $nsem, S_IRWXU | S_IRWXG | S_IRWXO | IPC_CREAT) });

unless (defined $sem) {
  my $info = "IPC::Semaphore->new failed: $!";
  if ($! == &IPC::SysV::ENOSPC || $! == &IPC::SysV::ENOSYS ||
      $! == &IPC::SysV::ENOMEM || $! == &IPC::SysV::EACCES) {
    plan(skip_all => $info);
  }
  else {
    die $info;
  }
}

plan(tests => 11);

pass('acquired a semaphore');

ok(my $st = $sem->stat,'stat it');

ok($sem->setall((0) x $nsem), 'set all');

my @sem = $sem->getall;
cmp_ok(join("", @sem), 'eq', "00000", 'get all');

$sem[2] = 1;
ok($sem->setall(@sem), 'set after change');

@sem = $sem->getall;
cmp_ok(join("", @sem), 'eq', "00100", 'get again');

my $ncnt = $sem->getncnt(0);
ok(!$sem->getncnt(0), 'procs waiting now');
ok(defined($ncnt), 'prev procs waiting');

ok($sem->op(2, -1, IPC_NOWAIT), 'op nowait');

ok(!$sem->getncnt(0), 'no procs waiting');

END {
  ok($sem->remove, 'remove semaphore') if defined $sem;
}
