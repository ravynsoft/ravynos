#!/usr/bin/perl -w

use strict;
use File::Spec;
use lib File::Spec->catfile('t', 'lib');
use Test::More;
local $|=1;

my @platforms = qw(Cygwin Epoc Mac OS2 Unix VMS Win32);
my $tests_per_platform = 10;

my $vms_unix_rpt = 0;
my $vms_efs = 0;
my $vms_unix_mode = 0;
my $vms_real_root = 0;

if ($^O eq 'VMS') {
    $vms_unix_mode = 0;
    if (eval 'require VMS::Feature') {
        $vms_unix_rpt = VMS::Feature::current("filename_unix_report");
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_unix_rpt = $unix_rpt =~ /^[ET1]/i; 
        $vms_efs = $efs_charset =~ /^[ET1]/i; 
    }

    # Traditional VMS mode only if VMS is not in UNIX compatible mode.
    $vms_unix_mode = ($vms_efs && $vms_unix_rpt);

    # If we are in UNIX mode, we may or may not have a real root.
    if ($vms_unix_mode) {
        my $rootdir = File::Spec->rootdir;
        $vms_real_root = 1 if ($rootdir eq '/');
    }

}


plan tests => 1 + @platforms * $tests_per_platform;

my %volumes = (
	       Mac => 'Macintosh HD',
	       OS2 => 'A:',
	       Win32 => 'A:',
	       VMS => 'v',
	      );
my %other_vols = (
		  Mac => 'Mounted Volume',
		  OS2 => 'B:',
		  Win32 => 'B:',
		  VMS => 'w',
	      );

ok 1, "Loaded";

foreach my $platform (@platforms) {
  my $module = "File::Spec::$platform";
  
 SKIP:
  {
    eval "require $module; 1";

    skip "Can't load $module", $tests_per_platform
      if $@;
    
    my $v = $volumes{$platform} || '';
    my $other_v = $other_vols{$platform} || '';
    
    # Fake out the environment on MacOS and Win32
    no strict 'refs';
    my $save_w = $^W;
    $^W = 0;
    local *{"File::Spec::Mac::rootdir"} = sub { "Macintosh HD:" };
    local *{"File::Spec::Win32::_cwd"}  = sub { "C:\\foo" };
    $^W = $save_w;
    use strict 'refs';


    my ($file, $base, $result);

    $base = $module->catpath($v, $module->catdir('', 'foo'), '');
    $base = $module->catdir($module->rootdir, 'foo');

    is $module->file_name_is_absolute($base), 1, "$base is absolute on $platform";

    # splitdir('') -> ()
    my @result = $module->splitdir('');
    is @result, 0, "$platform->splitdir('') -> ()";

    # canonpath() -> undef
    $result = $module->canonpath();
    is $result, undef, "$platform->canonpath() -> undef";

    # canonpath(undef) -> undef
    $result = $module->canonpath(undef);
    is $result, undef, "$platform->canonpath(undef) -> undef";

    # abs2rel('A:/foo/bar', 'A:/foo')    ->  'bar'
    $file = $module->catpath($v, $module->catdir($module->rootdir, 'foo', 'bar'), 'file');
    $base = $module->catpath($v, $module->catdir($module->rootdir, 'foo'), '');
    $result = $module->catfile('bar', 'file');
 
    if ($vms_unix_mode and $platform eq 'VMS') {
        # test 56 special
        # If VMS is in UNIX mode, so is the result, but having the volume
        # parameter present forces the abs2rel into VMS mode.
        $result = VMS::Filespec::vmsify($result);
        $result =~ s/\.$//;

        # If we have a real root, then we are dealing with absolute directories
        $result =~ s/\[\./\[/ if $vms_real_root;
    }

    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";
    

    # abs2rel('A:/foo/bar', 'B:/foo')    ->  'A:/foo/bar'
    $base = $module->catpath($other_v, $module->catdir($module->rootdir, 'foo'), '');
    $result = volumes_differ($module, $file, $base) ? $file : $module->catfile('bar', 'file');
    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";


    # abs2rel('A:/foo/bar', '/foo')      ->  'A:/foo/bar'
    $base = $module->catpath('', $module->catdir($module->rootdir, 'foo'), '');
    $result = volumes_differ($module, $file, $base) ? $file : $module->catfile('bar', 'file');
    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";


    # abs2rel('/foo/bar/file', 'A:/foo')    ->  '/foo/bar'
    $file = $module->catpath('', $module->catdir($module->rootdir, 'foo', 'bar'), 'file');
    $base = $module->catpath($v, $module->catdir($module->rootdir, 'foo'), '');
    $result = volumes_differ($module, $file, $base) ? $module->rel2abs($file) : $module->catfile('bar', 'file');

    if ($vms_unix_mode and $platform eq 'VMS') {
        # test 59 special
        # If VMS is in UNIX mode, so is the result, but having the volume
        # parameter present forces the abs2rel into VMS mode.
        $result = VMS::Filespec::vmsify($result);
    }

    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";
    

    # abs2rel('/foo/bar', 'B:/foo')    ->  '/foo/bar'
    $base = $module->catpath($other_v, $module->catdir($module->rootdir, 'foo'), '');
    $result = volumes_differ($module, $file, $base) ? $module->rel2abs($file) : $module->catfile('bar', 'file');

    if ($vms_unix_mode and $platform eq 'VMS') {
        # test 60 special
        # If VMS is in UNIX mode, so is the result, but having the volume
        # parameter present forces the abs2rel into VMS mode.
        $result = VMS::Filespec::vmsify($result);
    }

    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";
    

    # abs2rel('/foo/bar', '/foo')      ->  'bar'
    $base = $module->catpath('', $module->catdir($module->rootdir, 'foo'), '');
    $result = $module->catfile('bar', 'file');

    is $module->abs2rel($file, $base), $result, "$platform->abs2rel($file, $base)";
  }
}

sub volumes_differ {
  my ($module, $one, $two) = @_;
  my ($one_v) = $module->splitpath( $module->rel2abs($one) );
  my ($two_v) = $module->splitpath( $module->rel2abs($two) );
  return $one_v ne $two_v;
}
