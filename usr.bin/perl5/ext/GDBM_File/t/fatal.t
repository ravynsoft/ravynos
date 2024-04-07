#!./perl -w
#
# Exercise the error handling callback mechanism in gdbm.
#
# Try to trigger an error by surreptitiously closing the file handle which
# gdbm has opened.  Note that this won't trigger an error in newer
# releases of the gdbm library, which uses mmap() rather than write() etc:
# so skip in that case.

use strict;

use Test::More;
use Config;
use File::Temp 'tempdir';
use File::Spec;

BEGIN {
    plan(skip_all => "GDBM_File was not built")
	unless $Config{extensions} =~ /\bGDBM_File\b/;

    # https://rt.perl.org/Public/Bug/Display.html?id=117967
    plan(skip_all => "GDBM_File is flaky in $^O")
        if $^O =~ /darwin/;

    plan(tests => 8);
    use_ok('GDBM_File');
}

open my $fh, '<', $^X or die "Can't open $^X: $!";
my $fileno = fileno $fh;
isnt($fileno, undef, "Can find next available file descriptor");
close $fh or die $!;

is((open $fh, "<&=$fileno"), undef,
   "Check that we cannot open fileno $fileno. \$! is $!");

umask(0);
my $wd = tempdir(CLEANUP => 1);
my %h;
isa_ok(tie(%h, 'GDBM_File', File::Spec->catfile($wd, 'fatal_dbmx'),
           GDBM_WRCREAT, 0640), 'GDBM_File');

isnt((open $fh, "<&=$fileno"), undef, "dup fileno $fileno")
    or diag("\$! = $!");
isnt(close $fh, undef,
     "close fileno $fileno, out from underneath the GDBM_File");

# store some data to a closed file handle

my $res = eval {
    $h{Perl} = 'Rules';
    untie %h;
    99;
};

SKIP: {
    skip "Can't trigger failure", 2 if (defined $res and $res == 99);

    is $res, undef, "eval should return undef";

    # Observed "File write error" and "lseek error" from two different
    # systems.  So there might be more variants. Important part was that
    # we trapped the error # via croak.
    like($@, qr/ at .*\bfatal\.t line \d+\.\n\z/,
         'expected error message from GDBM_File');
}

