use strict;
use warnings;

require IO::Zlib; # uncomp2.t is like uncomp1.t but without 'use'

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

print "1..10\n";

my $hello = <<EOM ;
hello world
this is a test
EOM

my $name = "test_uncomp2_$$";

if (open(FH, ">$name"))
{
    binmode FH;
    print FH $hello;
    close FH;
}
else
{
    die "$name: $!";
}

my $file;
my $uncomp;

ok(1, $file = IO::Zlib->new());
ok(2, $file->open($name, "rb"));
ok(3, !$file->eof());
ok(4, $file->read($uncomp, 1024) == length($hello));
ok(5, $file->eof());
ok(6, $file->opened());
ok(7, $file->close());
ok(8, !$file->opened());

unlink($name);

ok(9, $hello eq $uncomp);

ok(10, !defined(IO::Zlib->new($name, "rb")));

