#!perl
BEGIN {
    require Config;
    unless ($Config::Config{'usethreads'}) {
        print "1..0 # Skip -- need threads for this test\n";
        exit 0;
    }
    if (($Config::Config{'extensions'} !~ m!\bPerlIO/via\b!) ){
        print "1..0 # Skip -- Perl configured without PerlIO::via module\n";
        exit 0;
    }
}

use strict;
use warnings;
use threads;

my $tmp = "via$$";

END {
    1 while unlink $tmp;
}

use Test::More tests => 2;

our $push_count = 0;

{
    open my $fh, ">:via(Test1)", $tmp
      or die "Cannot open $tmp: $!";
    $fh->autoflush;

    print $fh "AXAX";

    # previously this would crash
    threads->create(
        sub {
            print $fh "XZXZ";
        })->join;

    print $fh "BXBX";
    close $fh;

    open my $in, "<", $tmp;
    my $line = <$in>;
    close $in;

    is($line, "AYAYYZYZBYBY", "check thread data delivered");

    is($push_count, 1, "PUSHED not called for dup on thread creation");
}

package PerlIO::via::Test1;

sub PUSHED {
    my ($class) = @_;
    ++$main::push_count;
    bless {}, $class;
}

sub WRITE {
    my ($self, $data, $fh) = @_;
    $data =~ tr/X/Y/;
    $fh->autoflush;
    print $fh $data;
    return length $data;
}


