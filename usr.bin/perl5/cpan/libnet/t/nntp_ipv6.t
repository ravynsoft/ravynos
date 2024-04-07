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
use Net::NNTP;

my $debug = 0; # Net::NNTP->new( Debug => .. )

my $inet6class = Net::NNTP->can_inet6;
plan skip_all => "no IPv6 support found in Net::NNTP" if ! $inet6class;

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
my $host = $srv->sockhost;
my $port = $srv->sockport;
note("server on $host port $port");

plan tests => 1;

defined( my $pid = fork()) or die "fork failed: $!";
exit(nntp_server()) if ! $pid;

my $cl = Net::NNTP->new(Host => $host, Port => $port,, Debug => $debug);
note("created Net::NNTP object");
if (!$cl) {
  fail("IPv6 NNTP connect failed");
} else {
  $cl->quit;
  pass("IPv6 success");
}
wait;

sub nntp_server {
  my $ssl = shift;
  my $cl = $srv->accept or die "accept failed: $!";
  print $cl "200 nntp.example.com\r\n";
  while (<$cl>) {
    my ($cmd,$arg) = m{^(\S+)(?: +(.*))?\r\n} or die $_;
    $cmd = uc($cmd);
    if ($cmd eq 'QUIT' ) {
      print $cl "205 bye\r\n";
      last;
    } elsif ( $cmd eq 'MODE' ) {
      print $cl "201 Posting denied\r\n";
    } else {
      diag("received unknown command: $cmd");
      print "500 unknown cmd\r\n";
    }
  }
  note("NNTP dialog done");
  return 0;
}
