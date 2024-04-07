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
use Net::NNTP;
use Net::Cmd qw(CMD_REJECT);

unless(@{$NetConfig{nntp_hosts}}) {
    print "1..0 # Skip: no nntp_hosts defined in config\n";
    exit;
}

unless($NetConfig{test_hosts}) {
    print "1..0 # Skip: test_hosts not enabled in config\n";
    exit;
}

print "1..4\n";

my $i = 1;

my $nntp = Net::NNTP->new(Debug => 0)
        or (print("not ok 1\n"), exit);

print "ok 1\n";

my @grp;
foreach my $grp (qw(test alt.test control news.announce.newusers)) {
    @grp = $nntp->group($grp);
    last if @grp;
}

if($nntp->status == CMD_REJECT) {
    # Command was rejected, probably because we need authinfo
    map { print "ok ",$_,"\n" } 2,3,4;
    exit;
}

print "not " unless @grp;
print "ok 2\n";


if(@grp && $grp[2] > $grp[1]) {
    $nntp->head($grp[1]) or print "not ";
}
print "ok 3\n";

if(@grp) {
    $nntp->quit or print "not ";
}
print "ok 4\n";

