#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bIO\b/ && $^O ne 'VMS') {
	print "1..0\n";
	exit 0;
    }
}

use strict;
use FileHandle;
autoflush STDOUT 1;
use Test::More;
my $TB = Test::More->builder;

my $mystdout = new_from_fd FileHandle 1,"w";
$| = 1;
autoflush $mystdout;

print $mystdout "ok ".fileno($mystdout),
    " - ", "create new handle from file descriptor", "\n";
$TB->current_test($TB->current_test + 1);

my $fh = (new FileHandle "./TEST", O_RDONLY
       or new FileHandle "TEST", O_RDONLY);
ok(defined($fh), "create new handle O_RDONLY");

my $buffer = <$fh>;
is($buffer, "#!./perl\n", "Got expected first line via handle");

ungetc $fh ord 'A';
my $buf;
CORE::read($fh, $buf,1);
is($buf, 'A', "Got expected ordinal value via ungetc in handle's input stream");
close $fh;

$fh = new FileHandle;
ok(($fh->open("< TEST") && <$fh> eq $buffer),
    "FileHandle open() method created handle, which got expected first line");

$fh->seek(0,0);
ok((<$fh> eq $buffer), "Averted possible mixed CRLF/LF in t/TEST");

$fh->seek(0,2);
my $line = <$fh>;
ok(! (defined($line) || !$fh->eof), "FileHandle seek() and eof() methods");

ok(($fh->open("TEST","r") && !$fh->tell && $fh->close),
    "FileHandle open(), tell() and close() methods");

autoflush STDOUT 0;
ok(! $|, "handle not auto-flushing current output channel");

autoflush STDOUT 1;
ok($|, "handle auto-flushing current output channel");

{
    my ($rd,$wr) = FileHandle::pipe;
    my $non_forking = (
        $^O eq 'VMS' || $^O eq 'os2' || $^O eq 'amigaos' ||
        $^O eq 'MSWin32' || $Config{d_fork} ne 'define'
    );
    my $content = "Writing to one end of a pipe, reading from the other\n";
    if ($non_forking) {
        $wr->autoflush;
        $wr->print($content);
        is($rd->getline, $content,
            "Read content from pipe on non-forking platform");
    }
    else {
        my $child;
        if ($child = fork) {
            # parent
            $wr->close;
            is($rd->getline, $content,
                "Read content from pipe on forking platform");
        }
        elsif (defined $child) {
            # child
            $rd->close;
            $wr->print($content);
            exit(0);
        }
        else {
            die "fork failed: $!";
        }
    }
}

ok(!FileHandle->new('', 'r'), "Can't open empty filename");

done_testing();
