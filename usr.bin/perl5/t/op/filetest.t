#!./perl

# There are few filetest operators that are portable enough to test.
# See pod/perlport.pod for details.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../cpan/Perl-OSType/lib');
}

plan(tests => 58 + 27*14);

if ($^O =~ /MSWin32|cygwin|msys/ && !is_miniperl) {
  require Win32; # for IsAdminUser()
}

# Tests presume we are in t/op directory and that file 'TEST' is found
# therein.
is(-d 'op', 1, "-d: directory correctly identified");
is(-f 'TEST', 1, "-f: plain file correctly identified");
isnt(-f 'op', 1, "-f: directory is not a plain file");
isnt(-d 'TEST', 1, "-d: plain file is not a directory");
is(-r 'TEST', 1, "-r: file readable by effective uid/gid not found");

# Make a read only file. This happens to be empty, so we also use it later.
my $ro_empty_file = tempfile();

{
    open my $fh, '>', $ro_empty_file or die "open $fh: $!";
    close $fh or die "close $fh: $!";
}

chmod 0555, $ro_empty_file or die "chmod 0555, '$ro_empty_file' failed: $!";

SKIP: {
    my $restore_root;
    skip "Need Win32::IsAdminUser() on $^O", 1
      if $^O =~ /MSWin32|cygwin|msys/ && is_miniperl();
    my $Is_WinAdminUser = ($^O =~ /MSWin32|cygwin|msys/ and Win32::IsAdminUser()) ? 1 : 0;
    # TODO: skip("On an ACL filesystem like $^O we cannot rely on -w via uid/gid");
    # We have no filesystem check for ACL in core
    if ($Is_WinAdminUser) {
        skip("As Windows Administrator we cannot rely on -w via uid/gid");
    }
    elsif ($> == 0) {
	# root can read and write anything, so switch uid (may not be
	# implemented)
	eval '$> = 1';

	skip("Can't drop root privs to test read-only files") if $> == 0;
	note("Dropped root privs to test read-only files. \$> == $>");
	++$restore_root;
    }

    isnt(-w $ro_empty_file, 1, "-w: file writable by effective uid/gid");

    if ($restore_root) {
	# If the previous assignment to $> worked, so should this:
	$> = 0;
	note("Restored root privs after testing read-only files. \$> == $>");
    }
}

# these would fail for the euid 1
# (unless we have unpacked the source code as uid 1...)
is(-r 'op', 1, "-r: directory readable by effective uid/gid");
is(-w 'op', 1, "-w: directory writable by effective uid/gid");
is(-x 'op', 1, "-x: executable by effective uid/gid"); # Hohum.  Are directories -x everywhere?

is( "@{[grep -r, qw(foo io noo op zoo)]}", "io op",
    "-r: found directories readable by effective uid/gid" );

# Test stackability of filetest operators

is(defined( -f -d 'TEST' ), 1, "-f and -d stackable: plain file found");
isnt(-f -d _, 1, "-f and -d stackable: no plain file found");
isnt(defined( -e 'zoo' ), 1, "-e: file does not exist");
isnt(defined( -e -d 'zoo' ), 1, "-e and -d: neither file nor directory exists");
isnt(defined( -f -e 'zoo' ), 1, "-f and -e: not a plain file and does not exist");
is(-f -e 'TEST', 1, "-f and -e: plain file and exists");
is(-e -f 'TEST', 1, "-e and -f: exists and is plain file");
is(defined(-d -e 'TEST'), 1, "-d and -e: file at least exists");
is(defined(-e -d 'TEST'), 1, "-e and -d: file at least exists");
isnt( -f -d 'op', 1, "-f and -d: directory found but is not a plain file");
is(-x -d -x 'op', 1, "-x, -d and -x again: directory exists and is executable");
my ($size) = (stat 'TEST')[7];
cmp_ok($size, '>', 1, 'TEST is longer than 1 byte');
is( (-s -f 'TEST'), $size, "-s returns real size" );
is(-f -s 'TEST', 1, "-f and -s: plain file with non-zero size");

# now with an empty file
is(-f $ro_empty_file, 1, "-f: plain file found");
is(-s $ro_empty_file, 0, "-s: file has 0 bytes");
is(-f -s $ro_empty_file, 0, "-f and -s: plain file with 0 bytes");
is(-s -f $ro_empty_file, 0, "-s and -f: file with 0 bytes is plain file");

# stacked -l
eval { -l -e "TEST" };
like $@, qr/^The stat preceding -l _ wasn't an lstat at /,
  'stacked -l non-lstat error with warnings off';
{
 local $^W = 1;
 eval { -l -e "TEST" };
 like $@, qr/^The stat preceding -l _ wasn't an lstat at /,
  'stacked -l non-lstat error with warnings on';
}
# Make sure -l is using the previous stat buffer, and not using the previ-
# ous op’s return value as a file name.
# t/TEST can be a symlink under -Dmksymlinks, so use our temporary file.
SKIP: {
 use Perl::OSType 'os_type';
 if (os_type ne 'Unix') { skip "Not Unix", 3 }
 if ( $^O =~ /android/ ) {
     # Even the most basic toolbox in android provides ln,
     # but not which.
     $ln = "ln";
 }
 else {
     chomp(my $ln = `which ln`);
     if ( ! -e $ln ) { skip "No ln"   , 3 }
 }
 lstat $ro_empty_file;
 `ln -s $ro_empty_file 1`;
 isnt(-l -e _, 1, 'stacked -l uses previous stat, not previous retval');
 unlink 1;

 # Since we already have our skip block set up, we might as well put this
 # test here, too:
 # -l always treats a non-bareword argument as a file name
 system 'ln', '-s', $ro_empty_file, \*foo;
 local $^W = 1;
 my @warnings;
 local $SIG{__WARN__} = sub { push @warnings, @_ };
 is(-l \*foo, 1, '-l \*foo is a file name');
 ok($warnings[0] =~ /-l on filehandle foo/, 'warning for -l $handle');
 unlink \*foo;
}
# More -l $handle warning tests
{
 local $^W = 1;
 my @warnings;
 local $SIG{__WARN__} = sub { push @warnings, @_ };
 () = -l \*{"\x{3c6}oo"};
 like($warnings[0], qr/-l on filehandle \x{3c6}oo/,
  '-l $handle warning is utf8-clean');
 () = -l *foo;
 like($warnings[1], qr/-l on filehandle foo/,
  '-l $handle warning occurs for globs, not just globrefs');
 tell foo; # vivify the IO slot
 () = -l *foo{IO};
    # (element [3] because tell also warns)
 like($warnings[3], qr/-l on filehandle at/,
  '-l $handle warning occurs for iorefs as well');
} 

# test that _ is a bareword after filetest operators

-f 'TEST';
is(-f _, 1, "_ is bareword after filetest operator");
sub _ { "this is not a file name" }
is(-f _, 1, "_ is bareword after filetest operator");

my $over;
{
    package OverFtest;

    use overload 
	fallback => 1,
        -X => sub { 
            $over = [qq($_[0]), $_[1]];
            "-$_[1]"; 
        };
}
{
    package OverString;

    # No fallback. -X should fall back to string overload even without
    # it.
    use overload q/""/ => sub { $over = 1; "TEST" };
}
{
    package OverBoth;

    use overload
        q/""/   => sub { "TEST" },
        -X      => sub { "-$_[1]" };
}
{
    package OverNeither;

    # Need fallback. Previous versions of perl required 'fallback' to do
    # -X operations on an object with no "" overload.
    use overload 
        '+' => sub { 1 },
        fallback => 1;
}

my $ft = bless [], "OverFtest";
my $ftstr = qq($ft);
my $str = bless [], "OverString";
my $both = bless [], "OverBoth";
my $neither = bless [], "OverNeither";
my $nstr = qq($neither);

open my $gv, "<", "TEST";
bless $gv, "OverString";
open my $io, "<", "TEST";
$io = *{$io}{IO};
bless $io, "OverString";

my $fcntl_not_available;
eval { require Fcntl } or $fcntl_not_available = 1;

for my $op (split //, "rwxoRWXOezsfdlpSbctugkTMBAC") {
    $over = [];
    my $rv = eval "-$op \$ft";
    isnt( $rv, undef,               "overloaded -$op succeeds" )
        or diag( $@ );
    is( $over->[0], $ftstr,         "correct object for overloaded -$op" );
    is( $over->[1], $op,            "correct op for overloaded -$op" );
    is( $rv,        "-$op",         "correct return value for overloaded -$op");

    my ($exp, $is) = (1, "is");

    $over = 0;
    $rv = eval "-$op \$str";
    is($@, "",                      "-$op succeeds with string overloading");
    is( $rv, eval "-$op 'TEST'",    "correct -$op on string overload" );
    is( $over,      $exp,           "string overload $is called for -$op" );

    ($exp, $is) = $op eq "l" ? (1, "is") : (0, "not");

    $over = 0;
    eval "-$op \$gv";
    is( $over,      $exp,   "string overload $is called for -$op on GLOB" );

    # IO refs always get string overload called. This might be a bug.
    $op eq "t" || $op eq "T" || $op eq "B"
        and ($exp, $is) = (1, "is");

    $over = 0;
    eval "-$op \$io";
    is( $over,      $exp,   "string overload $is called for -$op on IO");

    $rv = eval "-$op \$both";
    is( $rv,        "-$op",         "correct -$op on string/-X overload" );

    $rv = eval "-$op \$neither";
    is($@, "",                      "-$op succeeds with random overloading");
    is( $rv, eval "-$op \$nstr",    "correct -$op with random overloading" );

    is( eval "-r -$op \$ft", "-r",      "stacked overloaded -$op" );
    is( eval "-$op -r \$ft", "-$op",    "overloaded stacked -$op" );
}

# -l stack corruption: this bug occurred from 5.8 to 5.14
{
 push my @foo, "bar", -l baz;
 is $foo[0], "bar", '-l bareword does not corrupt the stack';
}

# -l and fatal warnings
stat "test.pl";
eval { use warnings FATAL => io; -l cradd };
isnt(stat _, 1,
     'fatal warnings do not prevent -l HANDLE from setting stat status');

# File test ops should not call get-magic on the topmost SV on the stack if
# it belongs to another op.
{
  my $w;
  sub oon::TIESCALAR{bless[],'oon'}
  sub oon::FETCH{$w++}
  tie my $t, 'oon';
  push my @a, $t, -t;
  is $w, 1, 'file test does not call FETCH on stack item not its own';
}

# -T and -B

my $Perl = which_perl();

SKIP: {
    skip "no -T on filehandles", 8 unless eval { -T STDERR; 1 };

    # Test that -T HANDLE sets the last stat type
    -l "perl.c";   # last stat type is now lstat
    -T STDERR;     # should set it to stat, since -T does a stat
    eval { -l _ }; # should die, because the last stat type is not lstat
    like $@, qr/^The stat preceding -l _ wasn't an lstat at /,
	'-T HANDLE sets the stat type';

    # statgv should be cleared when freed
    fresh_perl_is
	'open my $fh, "test.pl"; -r $fh; undef $fh; open my $fh2, '
	. "q\0$Perl\0; print -B _",
	'',
	{ switches => ['-l'] },
	'PL_statgv should not point to freed-and-reused SV';

    # or coerced into a non-glob
    fresh_perl_is
	'open Fh, "test.pl"; -r($h{i} = *Fh); $h{i} = 3; undef %h;'
	. 'open my $fh2, ' . "q\0" . which_perl() . "\0; print -B _",
	'',
	{ switches => ['-l'] },
	'PL_statgv should not point to coerced-freed-and-reused GV';

    # -T _ should work after stat $ioref
    open my $fh, 'test.pl';
    stat $Perl; # a binary file
    stat *$fh{IO};
    is(-T _, 1, '-T _ works after stat $ioref');

    # and after -r $ioref
    -r *$fh{IO};
    is(-T _, 1, '-T _ works after -r $ioref');

    # -T _ on closed filehandle should still reset stat info
    stat $fh;
    close $fh;
    -T _;
    isnt(stat _, 1, '-T _ on closed filehandle resets stat info');

    lstat "test.pl";
    -T $fh; # closed
    eval { lstat _ };
    like $@, qr/^The stat preceding lstat\(\) wasn't an lstat at /,
	'-T on closed handle resets last stat type';

    # Fatal warnings should not affect the setting of errno.
    $! = 7;
    -T cradd;
    my $errno = $!;
    $! = 7;
    eval { use warnings FATAL => unopened; -T cradd };
    my $errno2 = $!;
    is $errno2, $errno,
	'fatal warnings do not affect errno after -T BADHADNLE';
}

is runperl(prog => '-T _', switches => ['-w'], stderr => 1), "",
  'no uninit warnings from -T with no preceding stat';

SKIP: {
    my $rand_file_name = 'filetest-' . rand =~ y/.//dr;
    if (-e $rand_file_name) { skip "File $rand_file_name exists", 1 }
    stat 'test.pl';
    -T $rand_file_name;
    isnt(stat _, 1, '-T "nonexistent" resets stat success status');
}

# Unsuccessful filetests on filehandles should leave stat buffers in the
# same state whether fatal warnings are on or off.
{
    stat "test.pl";
    # This GV has no IO
    -r *phlon;
    my $failed_stat1 = stat _;

    stat "test.pl";
    eval { use warnings FATAL => unopened; -r *phlon };
    my $failed_stat2 = stat _;

    is $failed_stat2, $failed_stat1,
	'failed -r($gv_without_io) with and w/out fatal warnings';

    stat "test.pl";
    -r cength;  # at compile time autovivifies IO, but with no fp
    $failed_stat1 = stat _;

    stat "test.pl";
    eval { use warnings FATAL => unopened; -r cength };
    $failed_stat2 = stat _;
    
    is $failed_stat2, $failed_stat1,
	'failed -r($gv_with_io_but_no_fp) with and w/out fatal warnings';
} 

{
    # [perl #131895] stat() doesn't fail on filenames containing \0 / NUL
    ok(!-T "TEST\0-", '-T on name with \0');
    ok(!-B "TEST\0-", '-B on name with \0');
    ok(!-f "TEST\0-", '-f on name with \0');
    ok(!-r "TEST\0-", '-r on name with \0');
}

{
    # github #18293
    "" =~ /(.*)/;
    my $x = $1; # call magic on $1, setting the pv to ""
    "test.pl" =~ /(.*)/;
    ok(-f -r $1, "stacked handles on a name with magic");
}
