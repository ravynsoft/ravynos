#!./perl -w

# Tests for the command-line switches:
# -0, -c, -l, -s, -m, -M, -V, -v, -h, -i, -E and all unknown
# Some switches have their own tests, see MANIFEST.

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
}

BEGIN { require "./test.pl";  require "./loc_tools.pl"; }

use Config;

# due to a bug in VMS's piping which makes it impossible for runperl()
# to emulate echo -n (ie. stdin always winds up with a newline), these 
# tests almost totally fail.
$TODO = "runperl() unable to emulate echo -n due to pipe bug" if $^O eq 'VMS';

my $r;
my @tmpfiles = ();
END { unlink_all @tmpfiles }

# Tests for -0

$r = runperl(
    switches	=> [ '-0', ],
    stdin	=> 'foo\0bar\0baz\0',
    prog	=> 'print qq(<$_>) while <>',
);
is( $r, "<foo\0><bar\0><baz\0>", "-0" );

$r = runperl(
    switches	=> [ '-l', '-0', '-p' ],
    stdin	=> 'foo\0bar\0baz\0',
    prog	=> '1',
);
is( $r, "foo\nbar\nbaz\n", "-0 after a -l" );

$r = runperl(
    switches	=> [ '-0', '-l', '-p' ],
    stdin	=> 'foo\0bar\0baz\0',
    prog	=> '1',
);
is( $r, "foo\0bar\0baz\0", "-0 before a -l" );

$r = runperl(
    switches	=> [ sprintf("-0%o", ord 'x') ],
    stdin	=> 'fooxbarxbazx',
    prog	=> 'print qq(<$_>) while <>',
);
is( $r, "<foox><barx><bazx>", "-0 with octal number" );

$r = runperl(
    switches	=> [ '-00', '-p' ],
    stdin	=> 'abc\ndef\n\nghi\njkl\nmno\n\npq\n',
    prog	=> 's/\n/-/g;$_.=q(/)',
);
is( $r, 'abc-def--/ghi-jkl-mno--/pq-/', '-00 (paragraph mode)' );

$r = runperl(
    switches	=> [ '-0777', '-p' ],
    stdin	=> 'abc\ndef\n\nghi\njkl\nmno\n\npq\n',
    prog	=> 's/\n/-/g;$_.=q(/)',
);
is( $r, 'abc-def--ghi-jkl-mno--pq-/', '-0777 (slurp mode)' );

$r = runperl(
    switches	=> [ '-066' ],
    prog	=> 'BEGIN { print qq{($/)} } print qq{[$/]}',
);
is( $r, "(\066)[\066]", '$/ set at compile-time' );

# Tests for -g

$r = runperl(
    switches => [ '-g' ],
    prog => 'BEGIN { printf q<(%d)>, defined($/) } printf q<[%d]>, defined($/)',
);
is( $r, "(0)[0]", '-g undefines $/ at compile-time' );

# Tests for -c

my $filename = tempfile();
SKIP: {
    local $TODO = '';   # this one works on VMS

    open my $f, ">$filename" or skip( "Can't write temp file $filename: $!" );
    print $f <<'SWTEST';
BEGIN { print "block 1\n"; }
CHECK { print "block 2\n"; }
INIT  { print "block 3\n"; }
	print "block 4\n";
END   { print "block 5\n"; }
SWTEST
    close $f or die "Could not close: $!";
    $r = runperl(
	switches	=> [ '-c' ],
	progfile	=> $filename,
	stderr		=> 1,
    );
    # Because of the stderr redirection, we can't tell reliably the order
    # in which the output is given
    ok(
	$r =~ /$filename syntax OK/
	&& $r =~ /\bblock 1\b/
	&& $r =~ /\bblock 2\b/
	&& $r !~ /\bblock 3\b/
	&& $r !~ /\bblock 4\b/
	&& $r !~ /\bblock 5\b/,
	'-c'
    );
}

SKIP: {
    skip 'locales not available', 1 unless locales_enabled('LC_ALL');

    my $tempdir = tempfile;
    mkdir $tempdir, 0700 or die "Can't mkdir '$tempdir': $!";

    local $ENV{'LC_ALL'} = 'C'; # Keep the test simple: expect English
    local $ENV{LANGUAGE} = 'C';
    setlocale(LC_ALL, "C");

    # Win32 won't let us open the directory, so we never get to die with
    # EISDIR, which happens after open.
    require Errno;
    import Errno qw(EACCES EISDIR);
    my $error  = do {
        local $! = $^O eq 'MSWin32' ? &EACCES : &EISDIR; "$!"
    };
    like(
        runperl( switches => [ '-c' ], args  => [ $tempdir ], stderr => 1),
        qr/Can't open perl script.*$tempdir.*\Q$error/s,
        "RT \#61362: Cannot syntax-check a directory"
    );
    rmdir $tempdir or die "Can't rmdir '$tempdir': $!";
}

# Tests for -l

$r = runperl(
    switches	=> [ sprintf("-l%o", ord 'x') ],
    prog	=> 'print for qw/foo bar/'
);
is( $r, 'fooxbarx', '-l with octal number' );

# Tests for -s

$r = runperl(
    switches	=> [ '-s' ],
    prog	=> 'for (qw/abc def ghi/) {print defined $$_ ? $$_ : q(-)}',
    args	=> [ '--', '-abc=2', '-def', ],
);
is( $r, '21-', '-s switch parsing' );

$filename = tempfile();
SKIP: {
    open my $f, ">$filename" or skip( "Can't write temp file $filename: $!" );
    print $f <<'SWTEST';
#!perl -s
BEGIN { print $x,$y; exit }
SWTEST
    close $f or die "Could not close: $!";
    $r = runperl(
	progfile    => $filename,
	args	    => [ '-x=foo -y' ],
    );
    is( $r, 'foo1', '-s on the shebang line' );
}

# Bug ID 20011106.084 (#7876)
$filename = tempfile();
SKIP: {
    open my $f, ">$filename" or skip( "Can't write temp file $filename: $!" );
    print $f <<'SWTEST';
#!perl -sn
BEGIN { print $x; exit }
SWTEST
    close $f or die "Could not close: $!";
    $r = runperl(
	progfile    => $filename,
	args	    => [ '-x=foo' ],
    );
    is( $r, 'foo', '-sn on the shebang line' );
}

# Tests for -m and -M

my $package = tempfile();
$filename = "$package.pm";
SKIP: {
    open my $f, ">$filename" or skip( "Can't write temp file $filename: $!",4 );
    print $f <<"SWTESTPM";
package $package;
sub import { print map "<\$_>", \@_ }
1;
SWTESTPM
    close $f or die "Could not close: $!";
    $r = runperl(
	switches    => [ "-I.", "-M$package" ],
	prog	    => '1',
    );
    is( $r, "<$package>", '-M' );
    $r = runperl(
	switches    => [ "-I.", "-M$package=foo" ],
	prog	    => '1',
    );
    is( $r, "<$package><foo>", '-M with import parameter' );
    $r = runperl(
	switches    => [ "-m$package" ],
	prog	    => '1',
    );

    {
        local $TODO = '';  # this one works on VMS
        is( $r, '', '-m' );
    }
    $r = runperl(
	switches    => [ "-I.", "-m$package=foo,bar" ],
	prog	    => '1',
    );
    is( $r, "<$package><foo><bar>", '-m with import parameters' );
    push @tmpfiles, $filename;

  {
    local $TODO = '';  # these work on VMS

    is( runperl( switches => [ '-MTie::Hash' ], stderr => 1, prog => 1 ),
	  '', "-MFoo::Bar allowed" );

    like( runperl( switches => [ "-M:$package" ], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/Invalid module name [\w:]+ with -M option\b/,
          "-M:Foo not allowed" );

    like( runperl( switches => [ '-mA:B:C' ], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/Invalid module name [\w:]+ with -m option\b/,
          "-mFoo:Bar not allowed" );

    like( runperl( switches => [ '-m-A:B:C' ], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/Invalid module name [\w:]+ with -m option\b/,
          "-m-Foo:Bar not allowed" );

    like( runperl( switches => [ '-m-' ], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/Module name required with -m option\b/,
  	  "-m- not allowed" );

    like( runperl( switches => [ '-M-=' ], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/Module name required with -M option\b/,
  	  "-M- not allowed" );
  }  # disable TODO on VMS
}
is runperl(stderr => 1, prog => '#!perl -m'),
   qq 'Too late for "-m" option at -e line 1.\n', '#!perl -m';
is runperl(stderr => 1, prog => '#!perl -M'),
   qq 'Too late for "-M" option at -e line 1.\n', '#!perl -M';

# Tests for -V

{
    local $TODO = '';   # these ones should work on VMS

    # basic perl -V should generate significant output.
    # we don't test actual format too much since it could change
    like( runperl( switches => ['-V'] ), qr/(\n.*){20}/,
          '-V generates 20+ lines' );

    like( runperl( switches => ['-V'] ),
	  qr/\ASummary of my perl5 .*configuration:/,
          '-V looks okay' );

    # lookup a known config var
    chomp( $r=runperl( switches => ['-V:osname'] ) );
    is( $r, "osname='$^O';", 'perl -V:osname');

    # lookup a nonexistent var
    chomp( $r=runperl( switches => ['-V:this_var_makes_switches_test_fail'] ) );
    is( $r, "this_var_makes_switches_test_fail='UNKNOWN';",
        'perl -V:unknown var');

    # regexp lookup
    # platforms that don't like this quoting can either skip this test
    # or fix test.pl _quote_args
    $r = runperl( switches => ['"-V:i\D+size"'] );
    # should be unlike( $r, qr/^$|not found|UNKNOWN/ );
    like( $r, qr/^(?!.*(not found|UNKNOWN))./, 'perl -V:re got a result' );

    # make sure each line we got matches the re
    ok( !( grep !/^i\D+size=/, split /^/, $r ), '-V:re correct' );
}

# Tests for -v

{
    local $TODO = '';   # these ones should work on VMS
    # There may be build configs where this test will fail; DG/UX was one,
    # but we no longer support it. Maybe we should remove these special cases?
  SKIP:
    {
        skip "Win32 miniperl produces a default archname in -v", 1
	  if $^O eq 'MSWin32' && is_miniperl;
        my $v = sprintf "%vd", $^V;
        my $ver = $Config{PERL_VERSION};
        my $rel = $Config{PERL_SUBVERSION};
        like( runperl( switches => ['-v'] ),
	      qr/This is perl 5, version \Q$ver\E, subversion \Q$rel\E \(v\Q$v\E(?:[-*\w]+| \([^)]+\))?\) built for \Q$Config{archname}\E.+Copyright.+Larry Wall.+Artistic License.+GNU General Public License/s,
              '-v looks okay' );
    }
}

# Tests for -h and -?

{
    local $TODO = '';   # these ones should work on VMS

    like( runperl( switches => ['-h'] ),
	  qr/Usage: .+(?i:perl(?:$Config{_exe})?).+switches.+programfile.+arguments/,
          '-h looks okay' );

    like( runperl( switches => ['-?'] ),
	  qr/Usage: .+(?i:perl(?:$Config{_exe})?).+switches.+programfile.+arguments/,
          '-? looks okay' );

}

# Tests for switches which do not exist

foreach my $switch (split //, "ABbGHJjKkLNOoPQqRrYyZz123456789_")
{
    local $TODO = '';   # these ones should work on VMS

    like( runperl( switches => ["-$switch"], stderr => 1,
		   prog => 'die q{oops}' ),
	  qr/\QUnrecognized switch: -$switch  (-h will show valid options)./,
          "-$switch correctly unknown" );

    # [perl #104288]
    like( runperl( stderr => 1, prog => "#!perl -$switch" ),
	  qr/^Unrecognized switch: -$switch  \(-h will show valid (?x:
	     )options\) at -e line 1\./,
          "-$switch unrecognised on #! line" );
}

# Tests for unshebangable switches
for (qw( e f x E S V )) {
    $r = runperl(
	stderr   => 1,
	prog     => "#!perl -$_",
    );
    is $r, "Can't emulate -$_ on #! line at -e line 1.\n","-$_ on #! line";
}

# Tests for -i

SKIP:
{
    local $TODO = '';   # these ones should work on VMS

    sub do_i_unlink { unlink_all("tmpswitches", "tmpswitches.bak") }

    open(FILE, ">tmpswitches") or die "$0: Failed to create 'tmpswitches': $!";
    my $yada = <<__EOF__;
foo yada dada
bada foo bing
king kong foo
__EOF__
    print FILE $yada;
    close FILE;

    END { do_i_unlink() }

    runperl( switches => ['-pi.bak'], prog => 's/foo/bar/', args => ['tmpswitches'] );

    open(FILE, "tmpswitches") or die "$0: Failed to open 'tmpswitches': $!";
    chomp(my @file = <FILE>);
    close FILE;

    open(BAK, "tmpswitches.bak") or die "$0: Failed to open 'tmpswitches.bak': $!";
    chomp(my @bak = <BAK>);
    close BAK;

    is(join(":", @file),
       "bar yada dada:bada bar bing:king kong bar",
       "-i new file");
    is(join(":", @bak),
       "foo yada dada:bada foo bing:king kong foo",
       "-i backup file");

    my $out1 = runperl(
        switches => ['-i.bak -p'],
        prog     => 'exit',
        stderr   => 1,
        stdin    => "1\n",
    );
    is(
        $out1,
        "-i used with no filenames on the command line, reading from STDIN.\n",
        "warning when no files given"
    );
    my $out2 = runperl(
        switches => ['-i.bak -p'],
        prog     => 'exit',
        stderr   => 1,
        stdin    => "1\n",
        args     => ['tmpswitches'],
    );
    is($out2, "", "no warning when files given");

    open my $f, ">", "tmpswitches" or die "$0: failed to create 'tmpswitches': $!";
    print $f "foo\nbar\n";
    close $f;

    # a backup extension is no longer required on any platform
    my $out3 = runperl(
        switches => [ '-i', '-p' ],
        prog => 's/foo/quux/',
        stderr => 1,
        args => [ 'tmpswitches' ],
    );
    is($out3, "", "no warnings/errors without backup extension");
    open $f, "<", "tmpswitches" or die "$0: cannot open 'tmpswitches': $!";
    chomp(my @out4 = <$f>);
    close $f;
    is(join(":", @out4), "quux:bar", "correct output without backup extension");

    eval { require File::Spec; 1 }
      or skip "Cannot load File::Spec - miniperl?", 20;

    my $tmpinplace = tempfile();

    require File::Path;
    END {
        File::Path::rmtree($tmpinplace)
            if $tmpinplace && -d $tmpinplace;
    }

    # test.pl's tempfile() doesn't create the file so we can
    # safely mkdir it
    mkdir $tmpinplace
      or die "Cannot create $tmpinplace: $!";

    my $work = File::Spec->catfile($tmpinplace, "foo");

    # exit or die should leave original content in file
    for my $inplace (qw/-i -i.bak/) {
        for my $prog ("die", "exit 1") {
            open my $fh, ">", $work or die "$0: failed to open '$work': $!";
            print $fh $yada;
            close $fh or die "Failed to close: $!";
            my $out = runperl (
               switches => [ $inplace, '-n' ],
               prog => "print q(foo\n); $prog",
               stderr => 1,
               args => [ $work ],
            );
            open my $in, "<", $work or die "$0: failed to open '$work': $!";
            my $data = do { local $/; <$in> };
            close $in;
            is ($data, $yada, "check original content still in file");
            unlink $work, "$work.bak";
        }
    }

    # test that path parsing is correct
    open $f, ">", $work or die "Cannot create $work: $!";
    print $f "foo\nbar\n";
    close $f;

    my $out4 = runperl
      (
       switches => [ "-i", "-p" ],
       prog => 's/foo/bar/',
       stderr => 1,
       args => [ $work ],
      );
    is ($out4, "", "no errors or warnings");
    open $f, "<", $work or die "Cannot open $work: $!";
    chomp(my @file4 = <$f>);
    close $f;
    is(join(":", @file4), "bar:bar", "check output");

  SKIP:
    {
        # this needs to match how ARGV_USE_ATFUNCTIONS is defined in doio.c
        skip "Not enough *at functions", 3
          unless $Config{d_unlinkat} && $Config{d_renameat} && $Config{d_fchmodat}
              && ($Config{d_dirfd} || $Config{d_dir_dd_fd})
              && $Config{d_linkat}
              && $Config{ccflags} !~ /-DNO_USE_ATFUNCTIONS\b/;
        my ($osvers) = ($Config{osvers} =~ /^(\d+(?:\.\d+)?)/);
        skip "NetBSD 6 libc defines at functions, but they're incomplete", 3
          if $^O eq "netbsd" && $osvers < 7;
        my $code = <<'CODE';
@ARGV = ("tmpinplace/foo");
$^I = "";
while (<>) {
  chdir "..";
  print "xx\n";
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/;
        fresh_perl_is($code, "ok\n", { },
                       "chdir while in-place editing");
        ok(open(my $fh, "<", $work), "open out file");
        is(scalar <$fh>, "xx\n", "file successfully saved after chdir");
        close $fh;
    }

  SKIP:
    {
        skip "Need threads and full perl", 3
          if !$Config{useithreads} || is_miniperl();

        my $code = <<'CODE';
use threads;
use strict;
@ARGV = ("tmpinplace/foo");
$^I = "";
while (<>) {
  threads->create(sub { })->join;
  print "yy\n";
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/;
        fresh_perl_is($code, "ok\n", { stderr => 1 },
                      "threads while in-place editing");
        ok(open(my $fh, "<", $work), "open out file");
        is(scalar <$fh>, "yy\n", "file successfully saved after chdir");
        close $fh;
    }

  SKIP:
    {
        skip "Need fork", 3 if !$Config{d_fork};
        open my $fh, ">", $work
          or die "Cannot open $work: $!";
        # we want only a single line for this test, otherwise
        # it attempts to close the file twice
        print $fh "foo\n";
        close $fh or die "Cannot close $work: $!";
        my $code = <<'CODE';
use strict;
@ARGV = ("tmpinplace/foo");
$^I = "";
while (<>) {
  my $pid = fork;
  if (defined $pid && !$pid) {
     # child
     close ARGVOUT or die "Cannot close in child\n"; # this shouldn't do ARGVOUT magic
     exit 0;
  }
  wait;
  print "yy\n";
  close ARGVOUT or die "Cannot close in parent\n"; # this should
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/;
        fresh_perl_is($code, "ok\n", { stderr => 1 },
                      "fork while in-place editing");
        ok(open($fh, "<", $work), "open out file");
        is(scalar <$fh>, "yy\n", "file successfully saved after fork");
        close $fh;
    }

    {
        # test we handle the rename to the backup failing
        if ($^O eq 'VMS') {
            # make it fail by creating a .bak file with a version than which no higher can be created
            # can't make a directory because foo.bak and foo^.bak.DIR do not conflict.
            open my $fh, '>', "$work.bak;32767" or die "Cannot make mask backup file: $!";
            close $fh or die "Failed to close: $!";
        }
        else {
            # make it fail by creating a directory of the backup name
            mkdir "$work.bak" or die "Cannot make mask backup directory: $!";
        }
        my $code = <<'CODE';
@ARGV = ("tmpinplace/foo");
$^I = ".bak";
while (<>) {
  print;
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/;
        fresh_perl_like($code, qr/Can't rename/, { stderr => 1 }, "fail backup rename");
        if ($^O eq 'VMS') {
            1 while unlink "$work.bak";
        }
        else {
            rmdir "$work.bak" or die "Cannot remove mask backup directory: $!";
        }
    }

    {
        # test with absolute paths, this was failing on FreeBSD 11ish due
        # to a bug in renameat()
        my $abs_work = File::Spec->rel2abs($work);
        fresh_perl_is(<<'CODE', "",
while (<>) {
  print;
}
CODE
                      { stderr => 1, args => [ $abs_work ], switches => [ "-i" ] },
                      "abs paths");
    }

    # we now use temp files for in-place editing, make sure we didn't leave
    # any behind in the above test
    opendir my $d, $tmpinplace or die "Cannot opendir $tmpinplace: $!";
    my @names = grep !/^\.\.?$/ && $_ ne 'foo' && $_ ne 'foo.', readdir $d;
    closedir $d;
    is(scalar(@names), 0, "no extra files")
      or diag "Found @names, expected none";

    # the following tests might leave work files behind

    # this test can leave the work file in the directory, since making
    # the directory non-writable also prevents removing the work file
  SKIP:
    {
        # test we handle the rename of the work to the original failing
        # make it fail by removing write perms from the directory
        # but first check that doesn't prevent writing
        chmod 0500, $tmpinplace;
        my $check = File::Spec->catfile($tmpinplace, "check");
        my $canwrite = open my $fh, ">", $check;
        unlink $check;
        chmod 0700, $tmpinplace or die "Cannot make $tmpinplace writable again: $!";
        skip "Cannot make $tmpinplace read only", 1
          if $canwrite;
        my $code = <<'CODE';
@ARGV = ("tmpinplace/foo");
$^I = "";
while (<>) {
  chmod 0500, "tmpinplace";
  print;
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/g;
        fresh_perl_like($code, qr/failed to rename/, { stderr => 1 }, "fail final rename");
        chmod 0700, $tmpinplace or die "Cannot make $tmpinplace writable again: $!";
    }

  SKIP:
    {
        # this needs to reverse match how ARGV_USE_ATFUNCTIONS is defined in doio.c
        skip "Testing without *at functions", 1
          if $Config{d_unlinkat} && $Config{d_renameat} && $Config{d_fchmodat}
              && ($Config{d_dirfd} || $Config{d_dir_dd_fd})
              && $Config{d_linkat}
              && $Config{ccflags} !~ /-DNO_USE_ATFUNCTIONS\b/;
        my $code = <<'CODE';
@ARGV = ("tmpinplace/foo");
$^I = "";
while (<>) {
  chdir "..";
  print "xx\n";
}
print "ok\n";
CODE
        $code =~ s/tmpinplace/$tmpinplace/;
        fresh_perl_like($code, qr/^Cannot complete in-place edit of \Q$tmpinplace\E\/foo: .* - line 5, <> line \d+\./, { },
                       "chdir while in-place editing (no at-functions)");
    }

    unlink $work;

    opendir $d, $tmpinplace or die "Cannot opendir $tmpinplace: $!";
    @names = grep !/^\.\.?$/ && !/foo$/aai, readdir $d;
    closedir $d;

    # clean up in case the above failed
    unlink map File::Spec->catfile($tmpinplace, $_), @names;

    rmdir $tmpinplace;
    undef $tmpinplace;
}

# Tests for -E

$TODO = '';  # the -E tests work on VMS

$r = runperl(
    switches	=> [ '-E', '"say q(Hello, world!)"']
);
is( $r, "Hello, world!\n", "-E say" );

$r = runperl(
    switches    => [ '-nE', q("} END { say q/affe/") ],
    stdin       => 'zomtek',
);
is( $r, "affe\n", '-E works outside of the block created by -n' );

$r = runperl(
    switches	=> [ '-E', q("*{'bar'} = sub{}; print 'Hello, world!',qq|\n|;")]
);
is( $r, "Hello, world!\n", "-E does not enable strictures" );

# RT #30660

$filename = tempfile();
SKIP: {
    open my $f, ">$filename" or skip( "Can't write temp file $filename: $!" );
    print $f <<'SWTEST';
#!perl -w    -iok
print "$^I\n";
SWTEST
    close $f or die "Could not close: $!";
    $r = runperl(
	progfile    => $filename,
    );
    like( $r, qr/ok/, 'Spaces on the #! line (#30660)' );
}

done_testing();
