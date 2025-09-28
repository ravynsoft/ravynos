use strict;
use warnings;

use IO::Zlib;

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

my $name = "test_basic_$$.gz";

print "1..17\n";

my $hello = <<EOM ;
hello world
this is a test
EOM

my $file;

ok(1, $file = IO::Zlib->new($name, "wb"));
ok(2, $file->print($hello));
ok(3, $file->opened());
ok(4, $file->close());
ok(5, !$file->opened());

my $uncomp;

ok(6, $file = IO::Zlib->new());
ok(7, $file->open($name, "rb"));
ok(8, !$file->eof());
ok(9, $file->read($uncomp, 1024) == length($hello));
ok(10, $uncomp eq $hello);
ok(11, $file->eof());
ok(12, $file->opened());
ok(13, $file->close());
ok(14, !$file->opened());

$file = IO::Zlib->new($name, "rb");
ok(15, $file->read($uncomp, 1024, length($uncomp)) == length($hello));
ok(16, $uncomp eq $hello . $hello);
$file->close();

unlink($name);

ok(17, !defined(IO::Zlib->new($name, "rb")));
