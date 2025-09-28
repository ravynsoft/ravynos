#!perl

use 5.008001;

use strict;
use warnings;

BEGIN {
    if (!eval { require Socket }) {
        print "1..0 # Skip: no Socket\n"; exit 0;
    }
    if (ord('A') == 193 && !eval { require Convert::EBCDIC }) {
        print "1..0 # Skip: EBCDIC but no Convert::EBCDIC\n"; exit 0;
    }
}

use Net::Config;
use Net::FTP;

unless(defined($NetConfig{ftp_testhost})) {
    print "1..0 # Skip: no ftp_testhost defined in config\n";
    exit 0;
}

unless($NetConfig{test_hosts}) {
    print "1..0 # Skip: test_hosts not enabled in config\n";
    exit 0;
}

my $t = 1;
print "1..7\n";

my $ftp = Net::FTP->new($NetConfig{ftp_testhost})
        or (print("not ok 1\n"), exit);

printf "ok %d\n",$t++;

$ftp->login('anonymous') or die($ftp->message . "\n");
printf "ok %d\n",$t++;

$ftp->pwd  or do {
  print STDERR $ftp->message,"\n";
  print "not ";
};

printf "ok %d\n",$t++;

$ftp->cwd('/pub') or do {
  print STDERR $ftp->message,"\n";
  print "not ";
};

my $data;
if ($data = $ftp->stor('libnet.tst')) {
  my $text = "abc\ndef\nqwe\n";
  printf "ok %d\n",$t++;
  $data->write($text,length $text);
  $data->close;
  $data = $ftp->retr('libnet.tst');
  my $buf;
  $data->read($buf,length $text);
  $data->close;
  print "not " unless $text eq $buf;
  printf "ok %d\n",$t++;
  $ftp->delete('libnet.tst') or print "not ";
  printf "ok %d\n",$t++;
  
}
else {
  print "# ",$ftp->message,"\n";
  printf "ok %d\n",$t++;
  printf "ok %d\n",$t++;
  printf "ok %d\n",$t++;
}

$ftp->quit  or do {
  print STDERR $ftp->message,"\n";
  print "not ";
};

printf "ok %d\n",$t++;
