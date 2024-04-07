#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
}

use strict;
use Fcntl ":seek";
use Config;
use Errno;
use Cwd "getcwd";

Win32::FsType() eq 'NTFS'
    or skip_all("need NTFS");

my (undef, $maj, $min) = Win32::GetOSVersion();

my $vista_or_later = $maj >= 6;

my $tmpfile1 = tempfile();
my $tmpfile2 = tempfile();

# test some of the win32 specific stat code, since we
# don't depend on the CRT for some of it

ok(link($0, $tmpfile1), "make a link to test nlink");

my @st = stat $0;
open my $fh, "<", $0 or die;
my @fst = stat $fh;

ok(seek($fh, 0, SEEK_END), "seek to end");
my $size = tell($fh);
close $fh;

# the ucrt stat() is inconsistent here, using an A=0 drive letter for stat()
# and the fd for fstat(), I assume that's something backward compatible.
#
# I don't see anything we could reasonable populate it with either.
$st[6] = $fst[6] = 0;

is("@st", "@fst", "check named stat vs handle stat");

ok($st[0], "we set dev by default now");
ok($st[1], "and ino");

# unlikely, but someone else might have linked to win32/stat.t
cmp_ok($st[3], '>', 1, "should be more than one link");

# we now populate all stat fields ourselves, so check what we can
is($st[7], $size, "we fetch size correctly");

cmp_ok($st[9], '<=', time(), "modification time before or on now");
ok(-f $0, "yes, we are a file");
ok(-d "win32", "and win32 is a directory");
pipe(my ($p1, $p2));
ok(-p $p1, "a pipe is a pipe");
close $p1; close $p2;
ok(-r $0, "we are readable");
ok(!-x $0, "but not executable");
ok(-e $0, "we exist");

ok(open(my $nul, ">", "nul"), "open nul");
ok(-c $nul, "nul is a character device");
close $nul;

my $nlink = $st[3];

# check we get nlinks etc for a directory
@st = stat("win32");
ok($st[0], "got dev for a directory");
ok($st[1], "got ino for a directory");
ok($st[3], "got nlink for a directory");

# symbolic links
unlink($tmpfile1); # no more hard link

if (open my $fh, ">", "$tmpfile1.bat") {
    ok(-x "$tmpfile1.bat", 'batch file is "executable"');
    SKIP: {
        skip "executable bit for handles needs vista or later", 1
            unless $vista_or_later;
        ok(-x $fh, 'batch file handle is "executable"');
    }
    close $fh;
    unlink "$tmpfile1.bat";
}

# mklink is available from Vista onwards
# this may only work in an admin shell
# MKLINK [[/D] | [/H] | [/J]] Link Target
if (system("mklink $tmpfile1 win32\\stat.t") == 0) {
    ok(-l $tmpfile1, "lstat sees a symlink");

    # check stat on file vs symlink
    @st = stat $0;
    my @lst = stat $tmpfile1;

    $st[6] = $lst[6] = 0;

    is("@st", "@lst", "check stat on file vs link");

    # our hard link no longer exists, check that is reflected in nlink
    is($st[3], $nlink-1, "check nlink updated");

    is((lstat($tmpfile1))[7], length(readlink($tmpfile1)),
       "check size matches length of link");

    unlink($tmpfile1);
}

# similarly for a directory
if (system("mklink /d $tmpfile1 win32") == 0) {
    ok(-l $tmpfile1, "lstat sees a symlink on the directory symlink");

    # check stat on directory vs symlink
    @st = stat "win32";
    my @lst = stat $tmpfile1;

    $st[6] = $lst[6] = 0;

    is("@st", "@lst", "check stat on dir vs link");

    # for now at least, we need to rmdir symlinks to directories
    rmdir( $tmpfile1 );
}

# check a junction looks like a symlink

if (system("mklink /j $tmpfile1 win32") == 0) {
    ok(-l $tmpfile1, "lstat sees a symlink on the directory junction");

    my @st = lstat($tmpfile1);
    is($st[7], length(readlink($tmpfile1)),
       "check returned length matches POSIX");

    rmdir( $tmpfile1 );
}

# test interaction between stat and utime
if (ok(open(my $fh, ">", $tmpfile1), "make a work file")) {
    # make our test file
    close $fh;

    my @st = stat $tmpfile1;
    ok(@st, "stat our work file");

    # switch to the other half of the year, to flip from/to daylight
    # savings time.  It won't always do so, but it's close enough and
    # avoids having to deal with working out exactly when it
    # starts/ends (if it does), along with the hemisphere.
    #
    # By basing this on the current file times and using an offset
    # that's the multiple of an hour we ensure the filesystem
    # resolution supports the time we set.
    my $moffset = 6 * 30 * 24 * 3600;
    my $aoffset = $moffset - 24 * 3600;;
    my $mymt = $st[9] - $moffset;
    my $myat = $st[8] - $aoffset;
    ok(utime($myat, $mymt, $tmpfile1), "set access and mod times");
    my @mst = stat $tmpfile1;
    ok(@mst, "fetch stat after utime");
    is($mst[9], $mymt, "check mod time");
    is($mst[8], $myat, "check access time");

    unlink $tmpfile1;
}

# same for a directory
if (ok(mkdir($tmpfile1), "make a work directory")) {
    my @st = stat $tmpfile1;
    ok(@st, "stat our work directory");

    my $moffset = 6 * 30 * 24 * 3600;
    my $aoffset = $moffset - 24 * 3600;;
    my $mymt = $st[9] - $moffset;
    my $myat = $st[8] - $aoffset;
    ok(utime($myat, $mymt, $tmpfile1), "set access and mod times");
    my @mst = stat $tmpfile1;
    ok(@mst, "fetch stat after utime");
    is($mst[9], $mymt, "check mod time");
    is($mst[8], $myat, "check access time");

    rmdir $tmpfile1;
}

 SKIP:
{ # github 19668
    $Config{ivsize} == 8
        or skip "Need 64-bit int", 1;
    open my $tmp, ">", $tmpfile1
        or skip "Cannot create test file: $!", 1;
    close $tmp;
    fresh_perl_is("utime(500_000_000_000, 500_000_000_000, '$tmpfile1')",
                  "", { stderr => 1 },
                  "check debug output removed");
    unlink $tmpfile1;
}

# Other stat issues possibly fixed by the stat() re-work

# https://github.com/Perl/perl5/issues/9025 - win32 - file test operators don't work for //?/UNC/server/file filenames
# can't really make a reliable regression test for this
# reproduced original problem with a gcc build
# confirmed fixed with a gcc build

# https://github.com/Perl/perl5/issues/8502 - filetest problem with STDIN/OUT on Windows

{
    ok(-r *STDIN, "check stdin is readable");
    ok(-w *STDOUT, "check stdout is writable");

    # CompareObjectHandles() could fix this, but requires Windows 10
    local our $TODO = "dupped *STDIN and *STDOUT not read/write";
    open my $dupin, "<&STDIN" or die;
    open my $dupout, ">&STDOUT" or die;
    ok(-r $dupin, "check duplicated stdin is readable");
    ok(-w $dupout, "check duplicated stdout is writable");
}

# https://github.com/Perl/perl5/issues/6080 - Last mod time from stat() can be wrong on Windows NT/2000/XP
# tested already

# https://github.com/Perl/perl5/issues/4145 - Problem with filetest -x _ on Win2k AS Perl build 626
# tested already

# https://github.com/Perl/perl5/issues/14687 -  Function lstat behavior case differs between Windows and Unix #14687

{
    local our $TODO = "... .... treated as .. by Win32 API";
    ok(!-e ".....", "non-existing many dots shouldn't returned existence");
}

# https://github.com/Perl/perl5/issues/7410 - -e tests not reliable under Win32
{
    # there's to issues here:
    # 1) CreateFile() successfully opens " . . " when opened with backup
    # semantics/directory
    # 2) opendir(" . . ") becomes FindFirstFile(" . . /*") which fails
    #
    # So we end up with success for the first and failure for the second,
    # making them inconsistent, there may be a Vista level fix for this,
    # but if we expect -e " . . " to fail we need a more complex fix.
    local our $TODO = "strange space handling by Windows";
    ok(!-e " ", "filename ' ' shouldn't exist");
    ok(!-e " . . ", "filename ' . . ' shouldn't exist");
    ok(!-e " .. ", "filename ' .. ' shouldn't exist");
    ok(!-e " . ", "filename ' . ' shouldn't exist");

    ok(!!-e " . . " == !!opendir(FOO, " . . "),
       "these should be consistent");
}

# https://github.com/Perl/perl5/issues/12431 - Win32: -e '"' always returns true

{
    ok(!-e '"', qq(filename '"' shouldn't exist));
}

# https://github.com/Perl/perl5/issues/20204
# Win32: stat/unlink fails on UNIX sockets
SKIP:
{
    use IO::Socket;
    unlink $tmpfile1;
    my $listen = IO::Socket::UNIX->new(Local => $tmpfile1, Listen => 0)
        or skip "Cannot create unix socket", 1;
    ok(-S $tmpfile1, "can stat a socket");
    ok(!-l $tmpfile1, "doesn't look like a symlink");
    unlink $tmpfile2;
    if (system("mklink $tmpfile2 $tmpfile1") == 0) {
        ok(-l $tmpfile2, "symlink to socket is a symlink (via lstat)");
        ok(-S $tmpfile2, "symlink to socket is also a socket (via stat)");
        unlink $tmpfile2;
    }
    close $listen;
    unlink $tmpfile1;
}

{
    # if a symlink chain leads to a socket, or loops, or is broken,
    # CreateFileA() fails, so we do our own link following.
    # The link leading to a socket is checked above, here check loops
    # fail, and that we get ELOOP (which isn't what MSVC returns, but
    # try to be better).
    if (system("mklink $tmpfile1 $tmpfile2") == 0
        && system("mklink $tmpfile2 $tmpfile1") == 0) {
        ok(!stat($tmpfile1), "looping symlink chain fails stat");
        is($!+0, &Errno::ELOOP, "check error set");
        ok(lstat($tmpfile1), "looping symlink chain passes lstat");

        unlink $tmpfile2;
        ok(!stat($tmpfile1), "broken symlink");
        is($!+0, &Errno::ENOENT, "check error set");
        ok(lstat($tmpfile1), "broken symlink chain passes lstat");
    }
    unlink $tmpfile1, $tmpfile2;
}

{
    # $tmpfile4 -> $tmpfile1/file1 -> ../$tmpfile2 -> abspath($tmpfile3)
    # $tmpfile3 either doesn't exist, is a file, or is a socket
    my ($tmpfile3, $tmpfile4) = (tempfile(), tempfile());
    ok(mkdir($tmpfile1), "make a directory");
    my $cwd = getcwd();
    if (system(qq(mklink $tmpfile4 $tmpfile1\\file1)) == 0
        && system(qq(mklink $tmpfile1\\file1 ..\\$tmpfile2)) == 0
        && system(qq(mklink $tmpfile2 "$cwd\\$tmpfile3")) == 0) {
        ok(-l $tmpfile4, "yes, $tmpfile4 is a symlink");
        ok(!-e $tmpfile4, "but we can't stat it");

        open my $fh, ">", $tmpfile3 or die $!;
        close $fh;
        ok(-f $tmpfile4, "now $tmpfile4 leads to a file");
        unlink $tmpfile3;

      SKIP:
        {
            my $listen = IO::Socket::UNIX->new(Local => $tmpfile3, Listen => 0)
                or skip "Cannot create unix socket", 1;
            ok(!-f $tmpfile4, "$tmpfile4 no longer leads to a file");
            ok(-S $tmpfile4, "now $tmpfile4 leads to a socket");
            ok(-S "$tmpfile1/file1", "$tmpfile1/file1 should lead to a socket");
            ok(-S $tmpfile2, "$tmpfile2 should lead to a socket");
            unlink $tmpfile3;
        }
    }
    unlink $tmpfile2, $tmpfile4, "$tmpfile1/file1";
    rmdir $tmpfile1;
}
done_testing();
