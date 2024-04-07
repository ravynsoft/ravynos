#!./perl -w

use strict;

use Cwd;

chdir 't';
@INC = '../../../lib' if $ENV{PERL_CORE};

use Config;
use File::Spec;
use File::Path;
use Errno qw(EACCES);

use lib File::Spec->catdir('t', 'lib');
use Test::More;

my $IsVMS = $^O eq 'VMS';

my $vms_unix_rpt = 0;
my $vms_efs = 0;
my $vms_mode = 0;

if ($IsVMS) {
    require VMS::Filespec;
    use Carp;
    use Carp::Heavy;
    $vms_mode = 1;
    if (eval 'require VMS::Feature') {
        $vms_unix_rpt = VMS::Feature::current("filename_unix_report");
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_unix_rpt = $unix_rpt =~ /^[ET1]/i; 
        $vms_efs = $efs_charset =~ /^[ET1]/i; 
    }
    $vms_mode = 0 if ($vms_unix_rpt);
}

my $tests = 31;
# _perl_abs_path() currently only works when the directory separator
# is '/', so don't test it when it won't work.
my $EXTRA_ABSPATH_TESTS = ($Config{prefix} =~ m/\//) && $^O ne 'cygwin';
$tests += 4 if $EXTRA_ABSPATH_TESTS;
plan tests => $tests;

SKIP: {
  skip "no need to check for blib/ in the core", 1 if $ENV{PERL_CORE};
  like $INC{'Cwd.pm'}, qr{blib}i, "Cwd should be loaded from blib/ during testing";
}


# check imports
can_ok('main', qw(cwd getcwd fastcwd fastgetcwd));
ok( !defined(&chdir),           'chdir() not exported by default' );
ok( !defined(&abs_path),        '  nor abs_path()' );
ok( !defined(&fast_abs_path),   '  nor fast_abs_path()');

{
  my @fields = qw(PATH IFS CDPATH ENV BASH_ENV);
  my $before = grep exists $ENV{$_}, @fields;
  cwd();
  my $after = grep exists $ENV{$_}, @fields;
  is($before, $after, "cwd() shouldn't create spurious entries in %ENV");
}

# XXX force Cwd to bootstrap its XSUBs since we have set @INC = "../lib"
# XXX and subsequent chdir()s can make them impossible to find
eval { fastcwd };

# Must find an external pwd (or equivalent) command.

my $pwd = $^O eq 'MSWin32' ? "cmd" : "pwd";
my $pwd_cmd =
    ($^O eq "NetWare") ?
        "cd" :
        (grep { -x && -f } map { "$_/$pwd$Config{exe_ext}" }
	                   split m/$Config{path_sep}/, $ENV{PATH})[0];

$pwd_cmd = 'SHOW DEFAULT' if $IsVMS;
if ($^O eq 'MSWin32') {
    $pwd_cmd =~ s,/,\\,g;
    $pwd_cmd = "$pwd_cmd /c cd";
}
$pwd_cmd =~ s=\\=/=g if ($^O eq 'dos');

SKIP: {
    skip "No native pwd command found to test against", 4 unless $pwd_cmd;

    print "# native pwd = '$pwd_cmd'\n";

    local @ENV{qw(PATH IFS CDPATH ENV BASH_ENV)};
    my ($pwd_cmd_untainted) = $pwd_cmd =~ /^(.+)$/; # Untaint.
    chomp(my $start = `$pwd_cmd_untainted`);

    # Win32's cd returns native C:\ style
    $start =~ s,\\,/,g if ($^O eq 'MSWin32' || $^O eq "NetWare");
    if ($IsVMS) {
        # DCL SHOW DEFAULT has leading spaces
        $start =~ s/^\s+//;

        # When in UNIX report mode, need to convert to compare it.
        if ($vms_unix_rpt) {
            $start = VMS::Filespec::unixpath($start);
            # Remove trailing slash.
            $start =~ s#/$##;
        }
    }
    SKIP: {
        skip("'$pwd_cmd' failed, nothing to test against", 4) if $?;
        skip("/afs seen, paths unlikely to match", 4) if $start =~ m|/afs/|;

	# Darwin's getcwd(3) (which Cwd.xs:bsd_realpath() uses which
	# Cwd.pm:getcwd uses) has some magic related to the PWD
	# environment variable: if PWD is set to a directory that
	# looks about right (guess: has the same (dev,ino) as the '.'?),
	# the PWD is returned.  However, if that path contains
	# symlinks, the path will not be equal to the one returned by
	# /bin/pwd (which probably uses the usual walking upwards in
	# the path -trick).  This situation is easy to reproduce since
	# /tmp is a symlink to /private/tmp.  Therefore we invalidate
	# the PWD to force getcwd(3) to (re)compute the cwd in full.
	# Admittedly fixing this in the Cwd module would be better
	# long-term solution but deleting $ENV{PWD} should not be
	# done light-heartedly. --jhi
	delete $ENV{PWD} if $^O eq 'darwin';

	my $cwd        = cwd;
	my $getcwd     = getcwd;
	my $fastcwd    = fastcwd;
	my $fastgetcwd = fastgetcwd;

	is($cwd,        $start, 'cwd()');
	is($getcwd,     $start, 'getcwd()');
	is($fastcwd,    $start, 'fastcwd()');
	is($fastgetcwd, $start, 'fastgetcwd()');
    }
}

my @test_dirs = qw{_ptrslt_ _path_ _to_ _a_ _dir_};
my $Test_Dir     = File::Spec->catdir(@test_dirs);

mkpath([$Test_Dir], 0, 0777);
Cwd::chdir $Test_Dir;

foreach my $func (qw(cwd getcwd fastcwd fastgetcwd)) {
  my $result = eval "$func()";
  is $@, '', "No exception for ${func}() in string eval";
  dir_ends_with( $result, $Test_Dir, "$func()" );
}

{
  # Some versions of File::Path (e.g. that shipped with perl 5.8.5)
  # call getcwd() with an argument (perhaps by calling it as a
  # method?), so make sure that doesn't die.
  is getcwd(), getcwd('foo'), "Call getcwd() with an argument";
}

# Cwd::chdir should also update $ENV{PWD}
dir_ends_with( $ENV{PWD}, $Test_Dir, 'Cwd::chdir() updates $ENV{PWD}' );
my $updir = File::Spec->updir;

for (1..@test_dirs) {
  Cwd::chdir $updir;
  print "#$ENV{PWD}\n";
}

rmtree($test_dirs[0], 0, 0);

{
  my $check = ($vms_mode ? qr|\b((?i)t)\]$| :
			   qr|\bt$| );
  
  like($ENV{PWD}, $check, "We're in a 't' directory");
}

{
  # Make sure abs_path() doesn't trample $ENV{PWD}
  my $start_pwd = $ENV{PWD};
  mkpath([$Test_Dir], 0, 0777);
  Cwd::abs_path($Test_Dir);
  is $ENV{PWD}, $start_pwd, "abs_path() does not trample \$ENV{PWD}";
  rmtree($test_dirs[0], 0, 0);
}

SKIP: {
    skip "no symlinks on this platform", 2+$EXTRA_ABSPATH_TESTS unless $Config{d_symlink} && $^O !~ m!^(qnx|nto)!;

    # on Win32 GetCurrentDirectory() includes the symlink if
    # you chdir() to a path including the symlink.
    skip "Win32 symlinks are unusual", 2+$EXTRA_ABSPATH_TESTS if $^O eq "MSWin32";

    my $file = "linktest";
    mkpath([$Test_Dir], 0, 0777);
    symlink $Test_Dir, $file;

    my $abs_path      =  Cwd::abs_path($file);
    my $fast_abs_path =  Cwd::fast_abs_path($file);
    my $pas           =  Cwd::_perl_abs_path($file);
    my $want          =  quotemeta(
                           File::Spec->rel2abs( $Test_Dir )
                         );
    if ($^O eq 'VMS') {
       # Not easy to predict the physical volume name
       $want = $ENV{PERL_CORE} ? $Test_Dir : File::Spec->catdir('t', $Test_Dir);

       # So just use the relative volume name
       $want =~ s/^\[//;

       $want = quotemeta($want);
    }

    like($abs_path,      qr|$want$|i, "Cwd::abs_path produced $abs_path");
    like($fast_abs_path, qr|$want$|i, "Cwd::fast_abs_path produced $fast_abs_path");
    if ($EXTRA_ABSPATH_TESTS) {
        # _perl_abs_path() can fail if some ancestor directory isn't readable
        if (defined $pas) {
            like($pas,           qr|$want$|i, "Cwd::_perl_abs_path produced $pas");
        }
        else {
            is($!+0, EACCES, "check we got the expected error on failure");
        }
    }

    rmtree($test_dirs[0], 0, 0);
    1 while unlink $file;
}

# Make sure we can run abs_path() on files, not just directories
my $path = 'cwd.t';
path_ends_with(Cwd::abs_path($path), 'cwd.t', 'abs_path() can be invoked on a file');
path_ends_with(Cwd::fast_abs_path($path), 'cwd.t', 'fast_abs_path() can be invoked on a file');
path_ends_with(Cwd::_perl_abs_path($path), 'cwd.t', '_perl_abs_path() can be invoked on a file')
  if $EXTRA_ABSPATH_TESTS;

$path = File::Spec->catfile(File::Spec->updir, 't', $path);
path_ends_with(Cwd::abs_path($path), 'cwd.t', 'abs_path() can be invoked on a file');
path_ends_with(Cwd::fast_abs_path($path), 'cwd.t', 'fast_abs_path() can be invoked on a file');
path_ends_with(Cwd::_perl_abs_path($path), 'cwd.t', '_perl_abs_path() can be invoked on a file')
  if $EXTRA_ABSPATH_TESTS;


  
SKIP: {
  my $file;
  {
    my $root = Cwd::abs_path(File::Spec->rootdir);	# Add drive letter?
    local *FH;
    opendir FH, $root or skip("Can't opendir($root): $!", 2+$EXTRA_ABSPATH_TESTS);
    ($file) = grep {-f $_ and not -l $_} map File::Spec->catfile($root, $_), readdir FH;
    closedir FH;
  }
  skip "No plain file in root directory to test with", 2+$EXTRA_ABSPATH_TESTS unless $file;
  
  $file = VMS::Filespec::rmsexpand($file) if $^O eq 'VMS';
  is Cwd::abs_path($file), $file, 'abs_path() works on files in the root directory';
  is Cwd::fast_abs_path($file), $file, 'fast_abs_path() works on files in the root directory';
  is Cwd::_perl_abs_path($file), $file, '_perl_abs_path() works on files in the root directory'
    if $EXTRA_ABSPATH_TESTS;
}

SKIP: {
  my $dir = "${$}a\nx";
  mkdir $dir or skip "OS does not support dir names containing LF", 1;
  chdir $dir or skip "OS cannot chdir into LF", 1;
  eval { Cwd::fast_abs_path() };
  is $@, "", 'fast_abs_path does not die in dir whose name contains LF';
  chdir File::Spec->updir;
  rmdir $dir;
}


#############################################
# These routines give us sort of a poor-man's cross-platform
# directory or path comparison capability.

sub bracketed_form_dir {
  return join '', map "[$_]", 
    grep length, File::Spec->splitdir(File::Spec->canonpath( shift() ));
}

sub dir_ends_with {
  my ($dir, $expect) = (shift, shift);
  my $bracketed_expect = quotemeta bracketed_form_dir($expect);
  like( bracketed_form_dir($dir), qr|$bracketed_expect$|i, (@_ ? shift : ()) );
}

sub bracketed_form_path {
  return join '', map "[$_]", 
    grep length, File::Spec->splitpath(File::Spec->canonpath( shift() ));
}

sub path_ends_with {
  my ($dir, $expect) = (shift, shift);
  my $bracketed_expect = quotemeta bracketed_form_path($expect);
  like( bracketed_form_path($dir), qr|$bracketed_expect$|i, (@_ ? shift : ()) );
}
