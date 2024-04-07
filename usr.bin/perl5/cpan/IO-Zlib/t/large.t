use strict;
use warnings;

use IO::Zlib;

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

my $name = "test_large_$$.gz";

print "1..7\n";

my $contents = "";

foreach (1 .. 5000)
{
     $contents .= chr(int(rand(255)));
}

my $file;

ok(1, $file = IO::Zlib->new($name, "wb"));
ok(2, $file->print($contents));
ok(3, $file->close());

my $uncomp;

ok(4, $file = IO::Zlib->new($name, "rb"));
ok(5, $file->read($uncomp, 8192) == length($contents));
ok(6, $file->close());

unlink($name);

ok(7, $contents eq $uncomp);
