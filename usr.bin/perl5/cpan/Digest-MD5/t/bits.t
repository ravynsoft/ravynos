#!perl -w

use strict;
use warnings;

use Test qw(plan ok);
plan tests => 2;

use Digest::MD5;

my $md5 = Digest::MD5->new;

if ($Digest::base::VERSION) {
    $md5->add_bits("01111111");
    ok($md5->hexdigest, "83acb6e67e50e31db6ed341dd2de1595");
    eval {
	$md5->add_bits("0111");
    };
    ok($@ =~ /must be multiple of 8/);
}
else {
    print "# No Digest::base\n";
    eval {
	$md5->add_bits("foo");
    };
    ok($@ =~ /^Can\'t locate Digest\/base\.pm in \@INC/);
    ok(1);  # dummy
}

