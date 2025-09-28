#!perl -w

BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }

    use Config;
    if ($Config{byteorder} ne "1234") {
	print "1..0 # Skip: Test only works for 32 bit little-ending machines\n";
	exit 0;
    }
}

use strict;
use Storable qw(retrieve);
use Test::More;

my $file = "xx-$$.pst";
my @dumps = (
    # some sample dumps of the hash { one => 1 }
    "perl-store\x041234\4\4\4\x94y\22\b\3\1\0\0\0vxz\22\b\1\1\0\0\x001Xk\3\0\0\0oneX", # 0.1
    "perl-store\0\x041234\4\4\4\x94y\22\b\3\1\0\0\0vxz\22\b\b\x81Xk\3\0\0\0oneX",      # 0.4@7
);

plan(tests => 3 * @dumps);

my $testno;
for my $dump (@dumps) {
    $testno++;

    open(FH, '>', $file) || die "Can't create $file: $!";
    binmode(FH);
    print FH $dump;
    close(FH) || die "Can't write $file: $!";

    my $data = eval { retrieve($file) };
    is($@, '', "No errors for $file");
    is(ref $data, 'HASH', "Got HASH for $file");
    is($data->{one}, 1, "Got data for $file");

    unlink($file);
}
