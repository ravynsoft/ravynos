use strict;
use warnings;

use IO::Zlib;

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

# Test this only iff we have an executable /usr/bin/gzip
# AND we have /usr/bin in our PATH
# AND we have a useable /usr/bin directory.
# This limits the testing to UNIX-like
# systems but that should be enough.

my $gzip = "/usr/bin/gzip";

unless (-x $gzip &&
        ":$ENV{PATH}:" =~ m!:/usr/bin:! &&
        -d "/usr/bin" && -x "/usr/bin")
{
    print "1..0 # Skip: no $gzip\n";
    exit 0;
}

my $hasCompressZlib;

BEGIN {
    eval { require Compress::Zlib };
    $hasCompressZlib = $@ ? 0 : 1;
}

print "1..35\n";

ok(1, $hasCompressZlib == IO::Zlib::has_Compress_Zlib());

eval "use IO::Zlib qw(:gzip_external)";

ok(2, $@ =~ /^IO::Zlib::import: ':gzip_external' requires an argument /);

eval "use IO::Zlib";

ok(3, !$@);
ok(4, $hasCompressZlib || IO::Zlib::gzip_used());
ok(5, !defined IO::Zlib::gzip_external());
ok(6, IO::Zlib::gzip_read_open() eq 'gzip -dc %s |');
ok(7, IO::Zlib::gzip_write_open() eq '| gzip > %s');
ok(8, \&IO::Zlib::gzopen == \&IO::Zlib::gzopen_external ||
      ($hasCompressZlib && \&IO::Zlib::gzopen == \&Compress::Zlib::gzopen));

eval "use IO::Zlib qw(:gzip_external 0)";

ok(9, !IO::Zlib::gzip_external());
ok(10, ($hasCompressZlib && \&IO::Zlib::gzopen == \&Compress::Zlib::gzopen) ||
       (!$hasCompressZlib && $@ =~ /^IO::Zlib::import: no Compress::Zlib and no external gzip /));

eval "use IO::Zlib qw(:gzip_external 1)";

ok(11, IO::Zlib::gzip_used());
ok(12, IO::Zlib::gzip_external());
ok(13, \&IO::Zlib::gzopen == \&IO::Zlib::gzopen_external);

eval 'IO::Zlib->new("foo", "xyz")';

ok(14, $@ =~ /^IO::Zlib::gzopen_external: mode 'xyz' is illegal /);

# The following is a copy of the basic.t, shifted up by 14 tests,
# the difference being that now we should be using the external gzip.

my $name="test_external_$$.gz";

my $hello = <<EOM ;
hello world
this is a test
EOM

my $file;

ok(15, $file = IO::Zlib->new($name, "wb"));
ok(16, $file->print($hello));
ok(17, $file->opened());
ok(18, $file->close());
ok(19, !$file->opened());

my $uncomp;

ok(20, $file = IO::Zlib->new());
ok(21, $file->open($name, "rb"));
ok(22, !$file->eof());
ok(23, $file->read($uncomp, 1024) == length($hello));
ok(24, $uncomp eq $hello);
ok(25, $file->eof());
ok(26, $file->opened());
ok(27, $file->close());
ok(28, !$file->opened());

$file = IO::Zlib->new($name, "rb");
ok(29, $file->read($uncomp, 1024, length($uncomp)) == length($hello));
ok(30, $uncomp eq $hello . $hello);
$file->close();

unlink($name);

ok(31, !defined(IO::Zlib->new($name, "rb")));

# Then finally test modifying the open commands.

my $new_read = 'gzip.exe /d /c %s |';

eval "use IO::Zlib ':gzip_read_open' => '$new_read'";

ok(32, IO::Zlib::gzip_read_open() eq $new_read);

eval "use IO::Zlib ':gzip_read_open' => 'bad'";

ok(33, $@ =~ /^IO::Zlib::import: ':gzip_read_open' 'bad' is illegal /);

my $new_write = '| gzip.exe %s';

eval "use IO::Zlib ':gzip_write_open' => '$new_write'";

ok(34, IO::Zlib::gzip_write_open() eq $new_write);

eval "use IO::Zlib ':gzip_write_open' => 'bad'";

ok(35, $@ =~ /^IO::Zlib::import: ':gzip_write_open' 'bad' is illegal /);
