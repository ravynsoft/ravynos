use strict;
use warnings;

use IO::Zlib;

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

my $name = "test_tied_$$.gz";

print "1..11\n";

my $hello = <<EOM ;
hello world
this is a test
EOM

ok(1, tie *OUT, "IO::Zlib", $name, "wb");
ok(2, printf OUT "%s - %d\n", "hello", 123);
ok(3, print OUT $hello);
ok(4, untie *OUT);

my $uncomp;

ok(5, tie *IN, "IO::Zlib", $name, "rb");
ok(6, !eof IN);
ok(7, <IN> eq "hello - 123\n");
ok(8, read(IN, $uncomp, 1024) == length($hello));
ok(9, eof IN);
ok(10, untie *IN);

unlink($name);

ok(11, $hello eq $uncomp);
