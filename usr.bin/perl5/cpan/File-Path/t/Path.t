#! /usr/bin/env perl
# Path.t -- tests for module File::Path

use strict;

use Test::More tests => 167;
use Config;
use Fcntl ':mode';
use lib './t';
use FilePathTest qw(
    _run_for_warning
    _run_for_verbose
    _cannot_delete_safe_mode
    _verbose_expected
    create_3_level_subdirs
    cleanup_3_level_subdirs
);
use Errno qw(:POSIX);
use Carp;

BEGIN {
    use_ok('Cwd');
    use_ok('File::Path', qw(rmtree mkpath make_path remove_tree));
    use_ok('File::Spec::Functions');
}

my $Is_VMS = $^O eq 'VMS';

my $fchmod_supported = 0;
if (open my $fh, curdir()) {
    my ($perm) = (stat($fh))[2];
    $perm &= 07777;
    eval { $fchmod_supported = chmod( $perm, $fh); };
}

# first check for stupid permissions second for full, so we clean up
# behind ourselves
for my $perm (0111,0777) {
    my $path = catdir(curdir(), "mhx", "bar");
    mkpath($path);
    chmod $perm, "mhx", $path;

    my $oct = sprintf('0%o', $perm);

    ok(-d "mhx", "mkdir parent dir $oct");
    ok(-d $path, "mkdir child dir $oct");

    rmtree("mhx");

    ok(! -e "mhx", "mhx does not exist $oct");
}

# find a place to work
my ($error, $list, $file, $message);
my $tmp_base = catdir(
    curdir(),
    sprintf( 'test-%x-%x-%x', time, $$, rand(99999) ),
);

# invent some names
my @dir = (
    catdir($tmp_base, qw(a b)),
    catdir($tmp_base, qw(a c)),
    catdir($tmp_base, qw(z b)),
    catdir($tmp_base, qw(z c)),
);

# create them
my @created = mkpath([@dir]);

is(scalar(@created), 7, "created list of directories");

# pray for no race conditions blowing them out from under us
@created = mkpath([$tmp_base]);
is(scalar(@created), 0, "skipped making existing directory")
    or diag("unexpectedly recreated @created");

# create a file
my $file_name = catfile( $tmp_base, 'a', 'delete.me' );
my $file_count = 0;
if (open OUT, "> $file_name") {
    print OUT "this file may be deleted\n";
    close OUT;
    ++$file_count;
}
else {
    diag( "Failed to create file $file_name: $!" );
}

SKIP: {
    skip "cannot remove a file we failed to create", 1
        unless $file_count == 1;
    my $count = rmtree($file_name);
    is($count, 1, "rmtree'ed a file");
}

@created = mkpath('');
is(scalar(@created), 0, "Can't create a directory named ''");

my $dir;
my $dir2;

sub gisle {
    # background info: @_ = 1; !shift # gives '' not 0
    # Message-Id: <3C820CE6-4400-4E91-AF43-A3D19B356E68@activestate.com>
    # http://www.nntp.perl.org/group/perl.perl5.porters/2008/05/msg136625.html
    mkpath(shift, !shift, 0755);
}

sub count {
    opendir D, shift or return -1;
    my $count = () = readdir D;
    closedir D or return -1;
    return $count;
}

{
    mkdir 'solo', 0755;
    chdir 'solo';
    open my $f, '>', 'foo.dat';
    close $f;
    my $before = count(curdir());
    cmp_ok($before, '>', 0, "baseline $before");

    gisle('1st', 1);
    is(count(curdir()), $before + 1, "first after $before");

    $before = count(curdir());
    gisle('2nd', 1);

    is(count(curdir()), $before + 1, "second after $before");

    chdir updir();
    rmtree 'solo';
}

{
    mkdir 'solo', 0755;
    chdir 'solo';
    open my $f, '>', 'foo.dat';
    close $f;
    my $before = count(curdir());

    cmp_ok($before, '>', 0, "ARGV $before");
    {
        local @ARGV = (1);
        mkpath('3rd', !shift, 0755);
    }

    is(count(curdir()), $before + 1, "third after $before");

    $before = count(curdir());
    {
        local @ARGV = (1);
        mkpath('4th', !shift, 0755);
    }

    is(count(curdir()), $before + 1, "fourth after $before");

    chdir updir();
    rmtree 'solo';
}

SKIP: {
    # tests for rmtree() of ancestor directory
    my $nr_tests = 6;
    my $cwd = getcwd() or skip "failed to getcwd: $!", $nr_tests;
    my $dir  = catdir($cwd, 'remove');
    my $dir2 = catdir($cwd, 'remove', 'this', 'dir');

    skip "failed to mkpath '$dir2': $!", $nr_tests
        unless mkpath($dir2, {verbose => 0});
    skip "failed to chdir dir '$dir2': $!", $nr_tests
        unless chdir($dir2);

    rmtree($dir, {error => \$error});
    my $nr_err = @$error;

    is($nr_err, 1, "ancestor error");

    if ($nr_err) {
        my ($file, $message) = each %{$error->[0]};

        is($file, $dir, "ancestor named");
        my $ortho_dir = $^O eq 'MSWin32' ? File::Path::_slash_lc($dir2) : $dir2;
        $^O eq 'MSWin32' and $message
            =~ s/\A(cannot remove path when cwd is )(.*)\Z/$1 . File::Path::_slash_lc($2)/e;

        is($message, "cannot remove path when cwd is $ortho_dir", "ancestor reason");

        ok(-d $dir2, "child not removed");

        ok(-d $dir, "ancestor not removed");
    }
    else {
        fail( "ancestor 1");
        fail( "ancestor 2");
        fail( "ancestor 3");
        fail( "ancestor 4");
    }
    chdir $cwd;
    rmtree($dir);

    ok(!(-d $dir), "ancestor now removed");
};

my $count = rmtree({error => \$error});

is( $count, 0, 'rmtree of nothing, count of zero' );

is( scalar(@$error), 0, 'no diagnostic captured' );

@created = mkpath($tmp_base, 0);

is(scalar(@created), 0, "skipped making existing directories (old style 1)")
    or diag("unexpectedly recreated @created");

$dir = catdir($tmp_base,'C');
# mkpath returns unix syntax filespecs on VMS
$dir = VMS::Filespec::unixify($dir) if $Is_VMS;
@created = make_path($tmp_base, $dir);

is(scalar(@created), 1, "created directory (new style 1)");

is($created[0], $dir, "created directory (new style 1) cross-check");

@created = mkpath($tmp_base, 0, 0700);

is(scalar(@created), 0, "skipped making existing directories (old style 2)")
    or diag("unexpectedly recreated @created");

$dir2 = catdir($tmp_base,'D');
# mkpath returns unix syntax filespecs on VMS
$dir2 = VMS::Filespec::unixify($dir2) if $Is_VMS;
@created = make_path($tmp_base, $dir, $dir2);

is(scalar(@created), 1, "created directory (new style 2)");

is($created[0], $dir2, "created directory (new style 2) cross-check");

$count = rmtree($dir, 0);

is($count, 1, "removed directory unsafe mode");

my $expected_count = _cannot_delete_safe_mode($dir2) ? 0 : 1;

$count = rmtree($dir2, 0, 1);

is($count, $expected_count, "removed directory safe mode");

# mkdir foo ./E/../Y
# Y should exist
# existence of E is neither here nor there
$dir = catdir($tmp_base, 'E', updir(), 'Y');
@created =mkpath($dir);

cmp_ok(scalar(@created), '>=', 1, "made one or more dirs because of ..");

cmp_ok(scalar(@created), '<=', 2, "made less than two dirs because of ..");

ok( -d catdir($tmp_base, 'Y'), "directory after parent" );

@created = make_path(catdir(curdir(), $tmp_base));

is(scalar(@created), 0, "nothing created")
    or diag(@created);

$dir  = catdir($tmp_base, 'a');
$dir2 = catdir($tmp_base, 'z');

rmtree( $dir, $dir2,
    {
        error     => \$error,
        result    => \$list,
        keep_root => 1,
    }
);


is(scalar(@$error), 0, "no errors unlinking a and z");

is(scalar(@$list),  4, "list contains 4 elements")
    or diag("@$list");

ok(-d $dir,  "dir a still exists");

ok(-d $dir2, "dir z still exists");

$dir = catdir($tmp_base,'F');
# mkpath returns unix syntax filespecs on VMS
$dir = VMS::Filespec::unixify($dir) if $Is_VMS;

@created = mkpath($dir, undef, 0770);

is(scalar(@created), 1, "created directory (old style 2 verbose undef)");

is($created[0], $dir, "created directory (old style 2 verbose undef) cross-check");

is(rmtree($dir, undef, 0), 1, "removed directory 2 verbose undef");

@created = mkpath($dir, undef);

is(scalar(@created), 1, "created directory (old style 2a verbose undef)");

is($created[0], $dir, "created directory (old style 2a verbose undef) cross-check");

is(rmtree($dir, undef), 1, "removed directory 2a verbose undef");

@created = mkpath($dir, 0, undef);

is(scalar(@created), 1, "created directory (old style 3 mode undef)");

is($created[0], $dir, "created directory (old style 3 mode undef) cross-check");

is(rmtree($dir, 0, undef), 1, "removed directory 3 verbose undef");

SKIP: {
    skip "fchmod of directories not supported on this platform", 3 unless $fchmod_supported;
    $dir = catdir($tmp_base,'G');
    $dir = VMS::Filespec::unixify($dir) if $Is_VMS;

    @created = mkpath($dir, undef, 0400);

    is(scalar(@created), 1, "created read-only dir");

    is($created[0], $dir, "created read-only directory cross-check");

    is(rmtree($dir), 1, "removed read-only dir");
}

# borderline new-style heuristics
if (chdir $tmp_base) {
    pass("chdir to temp dir");
}
else {
    fail("chdir to temp dir: $!");
}

$dir   = catdir('a', 'd1');
$dir2  = catdir('a', 'd2');

@created = make_path( $dir, 0, $dir2 );

is(scalar @created, 3, 'new-style 3 dirs created');

$count = remove_tree( $dir, 0, $dir2, );

is($count, 3, 'new-style 3 dirs removed');

@created = make_path( $dir, $dir2, 1 );

is(scalar @created, 3, 'new-style 3 dirs created (redux)');

$count = remove_tree( $dir, $dir2, 1 );

is($count, 3, 'new-style 3 dirs removed (redux)');

@created = make_path( $dir, $dir2 );

is(scalar @created, 2, 'new-style 2 dirs created');

$count = remove_tree( $dir, $dir2 );

is($count, 2, 'new-style 2 dirs removed');

$dir = catdir("a\nb", 'd1');
$dir2 = catdir("a\nb", 'd2');

SKIP: {
  # Better to search for *nix derivatives?
  # Not sure what else doesn't support newline in paths
  skip "$^O doesn't allow newline in paths", 2
    if $^O =~ m/^(MSWin32|VMS)$/;

  @created = make_path( $dir, $dir2 );

  is(scalar @created, 3, 'new-style 3 dirs created in parent with newline');

  $count = remove_tree( $dir, $dir2 );

  is($count, 2, 'new-style 2 dirs removed in parent with newline');
}

if (chdir updir()) {
    pass("chdir parent");
}
else {
    fail("chdir parent: $!");
}

SKIP: {
    # test bug http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=487319
    skip "Don't need Force_Writeable semantics on $^O", 6
        if grep {$^O eq $_} qw(amigaos dos epoc MSWin32 MacOS os2);
    skip "Symlinks not available", 6 unless $Config{d_symlink};
    $dir  = 'bug487319';
    $dir2 = 'bug487319-symlink';
    @created = make_path($dir, {mask => 0700});

    is( scalar @created, 1, 'bug 487319 setup' );
    symlink($dir, $dir2);

    ok(-e $dir2, "debian bug 487319 setup symlink") or diag($dir2);

    chmod 0500, $dir;
    my $mask_initial = (stat $dir)[2];
    remove_tree($dir2);

    my $mask = (stat $dir)[2];

    is( $mask, $mask_initial, 'mask of symlink target dir unchanged (debian bug 487319)');

    # now try a file
    #my $file = catfile($dir, 'file');
    my $file  = 'bug487319-file';
    my $file2 = 'bug487319-file-symlink';
    open my $out, '>', $file;
    close $out;

    ok(-e $file, 'file exists');

    chmod 0500, $file;
    $mask_initial = (stat $file)[2];

    symlink($file, $file2);

    ok(-e $file2, 'file2 exists');
    remove_tree($file2);

    $mask = (stat $file)[2];

    is( $mask, $mask_initial, 'mask of symlink target file unchanged (debian bug 487319)');

    remove_tree($dir);
    remove_tree($file);
}

# see what happens if a file exists where we want a directory
SKIP: {
    my $entry = catfile($tmp_base, "file");
    skip "VMS can have a file and a directory with the same name.", 4
        if $Is_VMS;
    skip "Cannot create $entry", 4 unless open OUT, "> $entry";
    print OUT "test file, safe to delete\n", scalar(localtime), "\n";
    close OUT;
    ok(-e $entry, "file exists in place of directory");

    mkpath( $entry, {error => \$error} );
    is( scalar(@$error), 1, "caught error condition" );
    ($file, $message) = each %{$error->[0]};
    is( $entry, $file, "and the message is: $message");

    eval {@created = mkpath($entry, 0, 0700)};
    $error = $@;
    chomp $error; # just to remove silly # in TAP output
    cmp_ok( $error, 'ne', "", "no directory created (old-style) err=$error" )
        or diag(@created);
}

{
    $dir = catdir($tmp_base, 'ZZ');
    @created = mkpath($dir);
    is(scalar(@created), 1, "create a ZZ directory");

    local @ARGV = ($dir);
    rmtree( [grep -e $_, @ARGV], 0, 0 );
    ok(!-e $dir, "blow it away via \@ARGV");
}

SKIP : {
    my $skip_count = 18;
    # this test will fail on Windows, as per:
    #   http://perldoc.perl.org/perlport.html#chmod

    skip "Windows chmod test skipped", $skip_count
        if $^O eq 'MSWin32';
    skip "fchmod() on directories is not supported on this platform", $skip_count
        unless $fchmod_supported;
    my $mode;
    my $octal_mode;
    my @inputs = (
      0777, 0700, 0470, 0407,
      0433, 0400, 0430, 0403,
      0111, 0100, 0110, 0101,
      0731, 0713, 0317, 0371,
      0173, 0137);
    my $input;
    my $octal_input;

    foreach (@inputs) {
        $input = $_;
        $dir = catdir($tmp_base, sprintf("chmod_test%04o", $input));
        # We can skip from here because 0 is last in the list.
        skip "Mode of 0 means assume user defaults on VMS", 1
          if ($input == 0 && $Is_VMS);
        @created = mkpath($dir, {chmod => $input});
        $mode = (stat($dir))[2];
        $octal_mode = S_IMODE($mode);
        $octal_input = sprintf "%04o", S_IMODE($input);
        SKIP: {
	    skip "permissions are not fully supported by the filesystem", 1
                if (($^O eq 'MSWin32' || $^O eq 'cygwin') && ((Win32::FsType())[1] & 8) == 0);
            is($octal_mode,$input, "create a new directory with chmod $input ($octal_input)");
	    }
        rmtree( $dir );
    }
}

my $dir_base = catdir($tmp_base,'output');
my $dir_a    = catdir($dir_base, 'A');
my $dir_b    = catdir($dir_base, 'B');

is(_run_for_verbose(sub {@created = mkpath($dir_a, 1)}),
    _verbose_expected('mkpath', $dir_base, 0, 1)
    . _verbose_expected('mkpath', $dir_a, 0),
    'mkpath verbose (old style 1)'
);

is(_run_for_verbose(sub {@created = mkpath([$dir_b], 1)}),
    _verbose_expected('mkpath', $dir_b, 0),
    'mkpath verbose (old style 2)'
);

my $verbose_expected;

# Must determine expectations while directories still exist.
$verbose_expected = _verbose_expected('rmtree', $dir_a, 1)
                  . _verbose_expected('rmtree', $dir_b, 1);

is(_run_for_verbose(sub {$count = rmtree([$dir_a, $dir_b], 1, 1)}),
    $verbose_expected,
    'rmtree verbose (old style)'
);

# In case we didn't delete them in safe mode.
rmtree($dir_a) if -d $dir_a;
rmtree($dir_b) if -d $dir_b;

is(_run_for_verbose(sub {@created = mkpath( $dir_a,
                                            {verbose => 1, mask => 0750})}),
    _verbose_expected('mkpath', $dir_a, 0),
    'mkpath verbose (new style 1)'
);

is(_run_for_verbose(sub {@created = mkpath($dir_b, 1, 0771)}),
    _verbose_expected('mkpath', $dir_b, 0),
    'mkpath verbose (new style 2)'
);

$verbose_expected = _verbose_expected('rmtree', $dir_a, 1)
                  . _verbose_expected('rmtree', $dir_b, 1);

is(_run_for_verbose(sub {$count = rmtree([$dir_a, $dir_b], 1, 1)}),
    $verbose_expected,
    'again: rmtree verbose (old style)'
);

rmtree($dir_a) if -d $dir_a;
rmtree($dir_b) if -d $dir_b;

is(_run_for_verbose(sub {@created = make_path( $dir_a, $dir_b,
                                               {verbose => 1, mode => 0711});}),
      _verbose_expected('make_path', $dir_a, 1)
    . _verbose_expected('make_path', $dir_b, 1),
    'make_path verbose with final hashref'
);

$verbose_expected = _verbose_expected('remove_tree', $dir_a, 0)
                  . _verbose_expected('remove_tree', $dir_b, 0);

is(_run_for_verbose(sub {@created = remove_tree( $dir_a, $dir_b,
                                                 {verbose => 1});}),
    $verbose_expected,
    'remove_tree verbose with final hashref'
);

rmtree($dir_a) if -d $dir_a;
rmtree($dir_b) if -d $dir_b;

# Have to re-create these 2 directories so that next block is not skipped.
@created = make_path(
    $dir_a,
    $dir_b,
    { mode => 0711 }
);
is(@created, 2, "2 directories created");

SKIP: {
    $file = catfile($dir_b, "file");
    skip "Cannot create $file", 2 unless open OUT, "> $file";
    print OUT "test file, safe to delete\n", scalar(localtime), "\n";
    close OUT;

    $verbose_expected = _verbose_expected('rmtree', $dir_a, 1)
                      . _verbose_expected('unlink', $file, 0)
                      . _verbose_expected('rmtree', $dir_b, 1);

    ok(-e $file, "file created in directory");

    is(_run_for_verbose(sub {$count = rmtree( $dir_a, $dir_b,
                                              {verbose => 1, safe => 1})}),
        $verbose_expected,
        'rmtree safe verbose (new style)'
    );
    rmtree($dir_a) if -d $dir_a;
    rmtree($dir_b) if -d $dir_b;
}

{
    my $base = catdir( $tmp_base, 'output2');
    my $dir  = catdir( $base, 'A');
    my $dir2 = catdir( $base, 'B');

    {
        my $warn = _run_for_warning( sub {
            my @created = make_path(
                $dir,
                $dir2,
                { mode => 0711, foo => 1, bar => 1 }
            );
        } );
        like($warn,
            qr/Unrecognized option\(s\) passed to mkpath\(\) or make_path\(\):.*?bar.*?foo/,
            'make_path with final hashref warned due to unrecognized options'
        );
    }

    {
        my $warn = _run_for_warning( sub {
            my @created = remove_tree(
                $dir,
                $dir2,
                { foo => 1, bar => 1 }
            );
        } );
        like($warn,
            qr/Unrecognized option\(s\) passed to remove_tree\(\):.*?bar.*?foo/,
            'remove_tree with final hashref failed due to unrecognized options'
        );
    }
}

SKIP: {
    my $nr_tests = 6;
    my $cwd = getcwd() or skip "failed to getcwd: $!", $nr_tests;
    rmtree($tmp_base, {result => \$list} );
    is(ref($list), 'ARRAY', "received a final list of results");
    ok( !(-d $tmp_base), "test base directory gone" );

    my $p = getcwd();
    my $x = "x$$";
    my $xx = $x . "x";

    # setup
    ok(mkpath($xx), "make $xx");
    ok(chdir($xx), "... and chdir $xx");
    END {
#         ok(chdir($p), "... now chdir $p");
#         ok(rmtree($xx), "... and finally rmtree $xx");
       chdir($p);
       rmtree($xx);
    }

    # create and delete directory
    my $px = catdir($p, $x);
    ok(mkpath($px), 'create and delete directory 2.07');
    ok(rmtree($px), '.. rmtree fails in File-Path-2.07');
    chdir updir();
}

my $windows_dir = 'C:\Path\To\Dir';
my $expect = 'c:/path/to/dir';
is(
    File::Path::_slash_lc($windows_dir),
    $expect,
    "Windows path unixified as expected"
);

{
    my ($x, $message, $object, $expect, $rv, $arg, $error);
    my ($k, $v, $second_error, $third_error);
    local $! = ENOENT;
    $x = $!;

    $message = 'message in a bottle';
    $object = '/path/to/glory';
    $expect = "$message for $object: $x";
    $rv = _run_for_warning( sub {
        File::Path::_error(
            {},
            $message,
            $object
        );
    } );
    like($rv, qr/^$expect/,
        "no \$arg->{error}: defined 2nd and 3rd args: got expected error message");

    $object = undef;
    $expect = "$message: $x";
    $rv = _run_for_warning( sub {
        File::Path::_error(
            {},
            $message,
            $object
        );
    } );
    like($rv, qr/^$expect/,
        "no \$arg->{error}: defined 2nd arg; undefined 3rd arg: got expected error message");

    $message = 'message in a bottle';
    $object = undef;
    $expect = "$message: $x";
    $arg = { error => \$error };
    File::Path::_error(
        $arg,
        $message,
        $object
    );
    is(ref($error->[0]), 'HASH',
        "first element of array inside \$error is hashref");
    ($k, $v) = %{$error->[0]};
    is($k, '', 'key of hash is empty string, since 3rd arg was undef');
    is($v, $expect, "value of hash is 2nd arg: $message");

    $message = '';
    $object = '/path/to/glory';
    $expect = "$message: $x";
    $arg = { error => \$second_error };
    File::Path::_error(
        $arg,
        $message,
        $object
    );
    is(ref($second_error->[0]), 'HASH',
        "first element of array inside \$second_error is hashref");
    ($k, $v) = %{$second_error->[0]};
    is($k, $object, "key of hash is '$object', since 3rd arg was defined");
    is($v, $expect, "value of hash is 2nd arg: $message");

    $message = '';
    $object = undef;
    $expect = "$message: $x";
    $arg = { error => \$third_error };
    File::Path::_error(
        $arg,
        $message,
        $object
    );
    is(ref($third_error->[0]), 'HASH',
        "first element of array inside \$third_error is hashref");
    ($k, $v) = %{$third_error->[0]};
    is($k, '', "key of hash is empty string, since 3rd arg was undef");
    is($v, $expect, "value of hash is 2nd arg: $message");
}

{
    # https://rt.cpan.org/Ticket/Display.html?id=117019
    # remove_tree(): Permit re-use of options hash without issuing a warning

    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| ZoYhvc6RmGnl S2CrQ0lju0o7 lvOqVYWpfhcP | );
    my @created;
    @created = File::Path::make_path($deepest, { mode => 0711 });
    is(scalar(@created), 3, "Created 3 subdirectories");

    my $x = '';
    my $opts = { error => \$x };
    File::Path::remove_tree($deepest, $opts);
    ok(! -d $deepest, "directory '$deepest' removed, as expected");

    my $warn;
    $warn = _run_for_warning( sub { File::Path::remove_tree($next_deepest, $opts); } );
    ok(! $warn, "CPAN 117019: No warning thrown when re-using \$opts");
    ok(! -d $next_deepest, "directory '$next_deepest' removed, as expected");

    $warn = _run_for_warning( sub { File::Path::remove_tree($least_deep, $opts); } );
    ok(! $warn, "CPAN 117019: No warning thrown when re-using \$opts");
    ok(! -d $least_deep, "directory '$least_deep' removed, as expected");
}

{
    # Corner cases with respect to arguments provided to functions
    my $count;

    $count = remove_tree();
    is($count, 0,
        "If not provided with any paths, remove_tree() will return a count of 0 things deleted");

    $count = remove_tree('');
    is($count, 0,
        "If not provided with any paths, remove_tree() will return a count of 0 things deleted");

    my $warn;
    $warn = _run_for_warning( sub { $count = rmtree(); } );
    like($warn, qr/No root path\(s\) specified/s, "Got expected carp");
    is($count, 0,
        "If not provided with any paths, remove_tree() will return a count of 0 things deleted");

    $warn = _run_for_warning( sub {$count = rmtree(undef); } );
    like($warn, qr/No root path\(s\) specified/s, "Got expected carp");
    is($count, 0,
        "If provided only with an undefined value, remove_tree() will return a count of 0 things deleted");

    $warn = _run_for_warning( sub {$count = rmtree(''); } );
    like($warn, qr/No root path\(s\) specified/s, "Got expected carp");
    is($count, 0,
        "If provided with an empty string for a path, remove_tree() will return a count of 0 things deleted");

    $count = make_path();
    is($count, 0,
        "If not provided with any paths, make_path() will return a count of 0 things created");

    $count = mkpath();
    is($count, 0,
        "If not provided with any paths, make_path() will return a count of 0 things created");
}

SKIP: {
    my $skip_count = 3;
    skip "Windows will not set this error condition", $skip_count
        if $^O eq 'MSWin32';

    # mkpath() with hashref:  case of phony user
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| Hhu1KpF4EVAV vUj5k37bih8v Vkdw02POXJxj | );
    my (@created, $error);
    my $user = join('_' => 'foobar', $$);
    @created = mkpath($deepest, { mode => 0711, user => $user, error => \$error });
#    TODO: {
#        local $TODO = "Notwithstanding the phony 'user', mkpath will actually create subdirectories; should it?";
#        is(scalar(@created), 0, "No subdirectories created");
#    }
    is(scalar(@$error), 1, "caught error condition" );
    my ($file, $message) = each %{$error->[0]};
    like($message,
        qr/unable to map $user to a uid, ownership not changed/s,
        "Got expected error message for phony user",
    );

    cleanup_3_level_subdirs($least_deep);
}

{
    # mkpath() with hashref:  case of valid uid
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| b5wj8CJcc7gl XTJe2C3WGLg5 VZ_y2T0XfKu3 | );
    my (@created, $error);
    my $warn;
    local $SIG{__WARN__} = sub { $warn = shift };
    @created = mkpath($deepest, { mode => 0711, uid => $>, error => \$error });
    SKIP: {
        my $skip_count = 1;
        skip "Warning should only appear on Windows", $skip_count
            unless $^O eq 'MSWin32';
        like($warn,
            qr/Option\(s\) implausible on Win32 passed to mkpath\(\) or make_path\(\)/,
            'make_path with final hashref warned due to options implausible on Win32'
        );
    }
    is(scalar(@created), 3, "Provide valid 'uid' argument: 3 subdirectories created");

    cleanup_3_level_subdirs($least_deep);
}

SKIP: {
    my $skip_count = 3;
    skip "getpwuid() and getgrgid() not implemented on Windows", $skip_count
        if $^O eq 'MSWin32';

    # mkpath() with hashref:  case of valid owner
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| aiJEDKaAEH25 nqhXsBM_7_bv qfRj4cur4Jrs | );
    my (@created, $error);
    my $name = getpwuid($>);
    @created = mkpath($deepest, { mode => 0711, owner => $name, error => \$error });
    is(scalar(@created), 3, "Provide valid 'owner' argument: 3 subdirectories created");

    cleanup_3_level_subdirs($least_deep);
}

SKIP: {
    my $skip_count = 5;
    skip "Windows will not set this error condition", $skip_count
        if $^O eq 'MSWin32';

    # mkpath() with hashref:  case of phony group
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| nOR4lGRMdLvz NnwkEHEVL5li _3f1Kv6q77yA | );
    my (@created, $error);
    my $bad_group = join('_' => 'foobarbaz', $$);
    @created = mkpath($deepest, { mode => 0711, group => $bad_group, error => \$error });
#    TODO: {
#        local $TODO = "Notwithstanding the phony 'group', mkpath will actually create subdirectories; should it?";
#        is(scalar(@created), 0, "No subdirectories created");
#    }
    is(scalar(@$error), 1, "caught error condition" );
    my ($file, $message) = each %{$error->[0]};
    like($message,
        qr/unable to map $bad_group to a gid, group ownership not changed/s,
        "Got expected error message for phony user",
    );

    cleanup_3_level_subdirs($least_deep);
}

{
    # mkpath() with hashref:  case of valid group
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| BEcigvaBNisY rd4lJ1iZRyeS OyQnDPIBxP2K | );
    my (@created, $error);
    my $warn;
    local $SIG{__WARN__} = sub { $warn = shift };
    @created = mkpath($deepest, { mode => 0711, group => $(, error => \$error });
    SKIP: {
        my $skip_count = 1;
        skip "Warning should only appear on Windows", $skip_count
            unless $^O eq 'MSWin32';
        like($warn,
            qr/Option\(s\) implausible on Win32 passed to mkpath\(\) or make_path\(\)/,
            'make_path with final hashref warned due to options implausible on Win32'
        );
    }
    is(scalar(@created), 3, "Provide valid 'group' argument: 3 subdirectories created");

    cleanup_3_level_subdirs($least_deep);
}

SKIP: {
    my $skip_count = 3;
    skip "getpwuid() and getgrgid() not implemented on Windows", $skip_count
        if $^O eq 'MSWin32';

    # mkpath() with hashref:  case of valid group
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| IayhWFDvys8X gTd6gaeuFzmV VVI6UWLJCOEC | );
    my (@created, $error);
    my $group_name = (getgrgid($())[0];
    @created = mkpath($deepest, { mode => 0711, group => $group_name, error => \$error });
    is(scalar(@created), 3, "Provide valid 'group' argument: 3 subdirectories created");

    cleanup_3_level_subdirs($least_deep);
}

SKIP: {
    my $skip_count = 3;
    skip "getpwuid() and getgrgid() not implemented on Windows", $skip_count
        if $^O eq 'MSWin32';

    # mkpath() with hashref:  case of valid owner and group
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| xsmOvlnxOqJc olsGlBSoVUpp tDuRilkD35rd | );
    my (@created, $error);
    my $name = getpwuid($>);
    my $group_name = (getgrgid($())[0];
    @created = mkpath($deepest, { mode => 0711, owner => $name, group => $group_name, error => \$error });
    is(scalar(@created), 3, "Provide valid 'owner' and 'group' 'group' arguments: 3 subdirectories created");

    cleanup_3_level_subdirs($least_deep);
}
