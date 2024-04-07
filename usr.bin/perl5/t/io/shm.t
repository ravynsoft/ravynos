#!perl
################################################################################
#
#  $Revision: 6 $
#  $Author: mhx $
#  $Date: 2010/03/07 16:01:42 +0100 $
#
################################################################################
#
#  Version 2.x, Copyright (C) 2007-2010, Marcus Holland-Moritz <mhx@cpan.org>.
#  Version 1.x, Copyright (C) 1999, Graham Barr <gbarr@pobox.com>.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

BEGIN {
  chdir 't' if -d 't';
  require "./test.pl";
  set_up_inc('../lib') if -d '../lib' && -d '../ext';

  require Config; import Config;

  if ($ENV{'PERL_CORE'} && $Config{'extensions'} !~ m[\bIPC/SysV\b]) {
    skip_all('-- IPC::SysV was not built');
  }
  skip_all_if_miniperl();
  if ($Config{'d_shm'} ne 'define') {
    skip_all('-- $Config{d_shm} undefined');
  }
}


use sigtrap qw/die normal-signals error-signals/;
use IPC::SysV qw/ IPC_PRIVATE S_IRWXU IPC_RMID /;

my $key;
END { shmctl $key, IPC_RMID, 0 if defined $key }

{
	local $SIG{SYS} = sub { skip_all("SIGSYS caught") } if exists $SIG{SYS};
	$key = shmget IPC_PRIVATE, 8, S_IRWXU;
}

if (not defined $key) {
  my $info = "IPC::SharedMem->new failed: $!";
  if ($! == &IPC::SysV::ENOSPC || $! == &IPC::SysV::ENOSYS ||
      $! == &IPC::SysV::ENOMEM || $! == &IPC::SysV::EACCES) {
    skip_all($info);
  }
  else {
    die $info;
  }
}
else {
	plan(tests => 21);
	pass('acquired shared mem');
}

ok(shmwrite($key, pack("N", 4711), 0, 4), 'write(offs=0)');
ok(shmwrite($key, pack("N", 210577), 4, 4), 'write(offs=4)');

my $var;
ok(shmread($key, $var, 0, 4), 'read(offs=0) returned ok');
is($var, pack("N", 4711), 'read(offs=0) correct');
ok(shmread($key, $var, 4, 4), 'read(offs=4) returned ok');
is($var, pack("N", 210577), 'read(offs=4) correct');

ok(shmwrite($key, "Shared", 1, 6), 'write(offs=1)');

ok(shmread($key, $var, 1, 6), 'read(offs=1) returned ok');
is($var, 'Shared', 'read(offs=1) correct');
ok(shmwrite($key,"Memory", 0, 6), 'write(offs=0)');

my $number = 1;
my $int = 2;
shmwrite $key, $int, 0, 1;
shmread $key, $number, 0, 1;
is("$number", $int, qq{"\$id" eq "$int"});
cmp_ok($number + 0, '==', $int, "\$id + 0 == $int");

my ($fetch, $store) = (0, 0);
{ package Counted;
  sub TIESCALAR { bless [undef] }
  sub FETCH     { ++$fetch; $_[0][0] }
  sub STORE     { ++$store; $_[0][0] = $_[1] } }
tie $ct, 'Counted';
shmread $key, $ct, 0, 1;
is($fetch, 1, "shmread FETCH once");
is($store, 1, "shmread STORE once");

{
    # check reading into an upgraded buffer is sane
    my $text = "\xC0\F0AB";
    ok(shmwrite($key, $text, 0, 4), "setup text");
    my $rdbuf = "\x{101}";
    ok(shmread($key, $rdbuf, 0, 4), "read it back");
    is($rdbuf, $text, "check we got back the expected");

    # check writing from an upgraded buffer
    utf8::upgrade(my $utext = $text);
    ok(shmwrite($key, $utext, 0, 4), "setup text (upgraded source)");
    $rdbuf = "";
    ok(shmread($key, $rdbuf, 0, 4), "read it back (upgraded source)");
    is($rdbuf, $text, "check we got back the expected (upgraded source)");
}
