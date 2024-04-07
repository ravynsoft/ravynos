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
use Net::SMTP;

unless(@{$NetConfig{smtp_hosts}}) {
    print "1..0 # Skip: no smtp_hosts defined in config\n";
    exit 0;
}

unless($NetConfig{test_hosts}) {
    print "1..0 # Skip: test_hosts not enabled in config\n";
    exit 0;
}

print "1..3\n";

my $i = 1;

my $smtp = Net::SMTP->new(Debug => 0)
        or (print("not ok 1\n"), exit);

print "ok 1\n";

$smtp->domain or print "not ";
print "ok 2\n";

$smtp->quit or print "not ";
print "ok 3\n";

