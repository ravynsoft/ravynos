#!perl

use 5.008001;

use strict;
use warnings;

use Test::More;

BEGIN {
    if (!eval { require Socket }) {
        plan skip_all => "no Socket";
    }
    elsif (ord('A') == 193 && !eval { require Convert::EBCDIC }) {
        plan skip_all => "EBCDIC but no Convert::EBCDIC";
    }
}

use Config;
use File::Temp 'tempfile';
use Net::POP3;

my $debug = 0; # Net::POP3->new( Debug => .. )

my $inet6class = Net::POP3->can_inet6;
plan skip_all => "no IPv6 support found in Net::POP3" if ! $inet6class;

plan skip_all => "fork not supported on this platform"
  unless $Config::Config{d_fork} || $Config::Config{d_pseudofork} ||
    (($^O eq 'MSWin32' || $^O eq 'NetWare') and
     $Config::Config{useithreads} and
     $Config::Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/);

my $srv = $inet6class->new(
  LocalAddr => '::1',
  Listen => 10
);
plan skip_all => "cannot create listener on ::1: $!" if ! $srv;
my $saddr = "[".$srv->sockhost."]".':'.$srv->sockport;
note("server on $saddr");

plan tests => 1;

defined( my $pid = fork()) or die "fork failed: $!";
exit(pop3_server()) if ! $pid;

my $cl = Net::POP3->new($saddr, Debug => $debug);
note("created Net::POP3 object");
if (!$cl) {
  fail("IPv6 POP3 connect failed");
} else {
  $cl->quit;
  pass("IPv6 success");
}
wait;

sub pop3_server {
  my $cl = $srv->accept or die "accept failed: $!";
  print $cl "+OK localhost ready\r\n";
  while (<$cl>) {
    my ($cmd,$arg) = m{^(\S+)(?: +(.*))?\r\n} or die $_;
    $cmd = uc($cmd);
    if ($cmd eq 'QUIT' ) {
      print $cl "+OK bye\r\n";
      last;
    } elsif ( $cmd eq 'CAPA' ) {
      print $cl "+OK\r\n".
        ".\r\n";
    } else {
      diag("received unknown command: $cmd");
      print "-ERR unknown cmd\r\n";
    }
  }

  note("POP3 dialog done");
  return 0;
}
