#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
    eval 'use Errno';
    die $@ if $@ and !is_miniperl();
}

my $has_link = Win32::FsType() eq 'NTFS';

$has_link
  or skip_all("link() only works on NTFS");

plan tests => 4;

my $tmpfile1 = tempfile();
my $tmpfile2 = tempfile();

# RT #112272
ok(!link($tmpfile1, $tmpfile2),
   "Cannot link to unknown file");
is(0+$!, &Errno::ENOENT, "check errno is ENOENT");
open my $fh, ">", $tmpfile1
    or skip("Cannot create test link src", 2);
close $fh;
open my $fh, ">", $tmpfile2
    or skip("Cannot create test link target", 2);
close $fh;
ok(!link($tmpfile1, $tmpfile2),
   "Cannot link to existing file");
is(0+$!, &Errno::EEXIST, "check for EEXIST");
