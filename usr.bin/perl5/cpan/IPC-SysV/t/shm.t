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

if ($Config{'d_shm'} ne 'define') {
  plan(skip_all => '$Config{d_shm} undefined');
}

use IPC::SysV qw( IPC_PRIVATE S_IRWXU );
use IPC::SharedMem;

my $shm = sub {
  my $code = shift;
  if (exists $SIG{SYS}) {
    local $SIG{SYS} = sub { plan(skip_all => "SIGSYS caught") };
    return $code->();
  }
  return $code->();
}->(sub { IPC::SharedMem->new(IPC_PRIVATE, 8, S_IRWXU) });

unless (defined $shm) {
  my $info = "IPC::SharedMem->new failed: $!";
  if ($! == &IPC::SysV::ENOSPC || $! == &IPC::SysV::ENOSYS ||
      $! == &IPC::SysV::ENOMEM || $! == &IPC::SysV::EACCES) {
    plan(skip_all => $info);
  }
  else {
    die $info;
  }
}

plan(tests => 23);

pass('acquired shared mem');

my $st = $shm->stat;

ok($st, 'stat it');
is($st->nattch, 0, 'st->nattch');
is($st->cpid, $$, 'cpid');
ok($st->segsz >= 8, 'segsz');

ok($shm->write(pack("N", 4711), 0, 4), 'write(offs=0)');
ok($shm->write(pack("N", 210577), 4, 4), 'write(offs=4)');

is($shm->read(0, 4), pack("N", 4711), 'read(offs=0)');
is($shm->read(4, 4), pack("N", 210577), 'read(offs=4)');

ok($shm->attach, 'attach');

$st = $shm->stat;

ok($st, 'stat it');
is($st->nattch, 1, 'st->nattch');
is($st->cpid, $$, 'lpid');

is($shm->read(0, 4), pack("N", 4711), 'read(offs=0)');
is($shm->read(4, 4), pack("N", 210577), 'read(offs=4)');

ok($shm->write("Shared", 1, 6), 'write(offs=1)');

ok(!$shm->is_removed, '!is_removed');
ok($shm->remove, 'remove');
ok($shm->is_removed, 'is_removed');

is($shm->read(1, 6), 'Shared', 'read(offs=1)');
ok($shm->write("Memory", 0, 6), 'write(offs=0)');
is(unpack("P6", $shm->addr), 'Memory', 'read using unpack');

ok($shm->detach, 'detach');

