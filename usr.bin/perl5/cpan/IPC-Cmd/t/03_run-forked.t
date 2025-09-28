#!/usr/bin/perl

BEGIN { chdir 't' if -d 't' };

use strict;
use warnings;
use lib qw[../lib];
use Test::More 'no_plan';
use Data::Dumper;
use File::Temp qw(tempfile);

use_ok("IPC::Cmd", "run_forked");

unless ( IPC::Cmd->can_use_run_forked ) {
  ok(1, "run_forked not available on this platform");
  exit;
}
else {
  ok(1, "run_forked available on this platform");
}

my $true = IPC::Cmd::can_run('true');
my $false = IPC::Cmd::can_run('false');
my $echo = IPC::Cmd::can_run('echo');
my $sleep = IPC::Cmd::can_run('sleep');
my $cat = IPC::Cmd::can_run('cat');

unless ( $true and $false and $echo and $sleep and $cat ) {
  ok(1, 'Either "true" or "false" "echo" or "sleep" or "cat" is missing on this platform');
  exit;
}

my $r;

$r = run_forked($true);
ok($r->{'exit_code'} eq '0', "$true returns 0");
$r = run_forked($false);
ok($r->{'exit_code'} ne '0', "$false returns not 0");

$r = run_forked([$echo, "test"]);
ok($r->{'stdout'} =~ /test/, "arrayref cmd: https://rt.cpan.org/Ticket/Display.html?id=70530");

$r = run_forked("$sleep 5", {'timeout' => 2});
ok($r->{'timeout'}, "[$sleep 5] runs longer than 2 seconds");

SKIP: {
  skip "Exhibits problems on Cygwin", 4 if $^O eq 'cygwin';
  # https://rt.cpan.org/Ticket/Display.html?id=85912
  sub runSub {
       my $blah = "blahblah";
       my $out= $_[0];
       my $err= $_[1];

       my $s = sub {
          print "$blah\n";
          print "$$: Hello $out\n";
          warn "Boo!\n$err\n";
       };

       return run_forked($s);
  }

  my $retval= runSub("sailor", "eek!");
  ok($retval->{"stdout"} =~ /blahblah/, "https://rt.cpan.org/Ticket/Display.html?id=85912 stdout 1");
  ok($retval->{"stdout"} =~ /Hello sailor/, "https://rt.cpan.org/Ticket/Display.html?id=85912 stdout 2");
  ok($retval->{"stderr"} =~ /Boo/, "https://rt.cpan.org/Ticket/Display.html?id=85912 stderr 1");
  ok($retval->{"stderr"} =~ /eek/, "https://rt.cpan.org/Ticket/Display.html?id=85912 stderr 2");
}

$r = run_forked("$echo yes i know this is the way", {'discard_output' => 1});
ok($r->{'stdout'} eq '', "discard_output stdout");
ok($r->{'stderr'} eq '', "discard_output stderr");
ok($r->{'merged'} eq '', "discard_output merged");
ok($r->{'err_msg'} eq '', "discard_output err_msg");

my ($fh, $filename) = tempfile();
my $one_line = "in Montenegro with Katyusha\n";
for (my $i = 0; $i < 10240; $i++) {
  print $fh $one_line;
}
close($fh);


SKIP: {
  skip 'Skip these tests in PERL_CORE', 100 if $ENV{PERL_CORE};
  skip 'These tests heisenfail on HPUX', 100 if $^O eq 'hpux';
  for (my $i = 0; $i < 100; $i++) {
    my $f_ipc_cmd = IPC::Cmd::run_forked("$cat $filename");
    my $f_backticks = `$cat $filename`;
    if ($f_ipc_cmd->{'stdout'} ne $f_backticks) {
      fail ("reading $filename: run_forked output length [" . length($f_ipc_cmd->{'stdout'}) . "], backticks output length [" . length ($f_backticks) . "]");
      #print Data::Dumper::Dumper($f_ipc_cmd);
      die;
    }
    else {
      pass ("$i: reading $filename");
    }
  }
}
unlink($filename);
