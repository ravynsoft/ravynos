use strict;
use Test;
use Config qw(%Config);
use Cwd qw(cwd);
use Encode qw();
use Win32;

BEGIN {
    unless (defined &Win32::BuildNumber && Win32::BuildNumber() >= 820 or $] >= 5.008009) {
	print "1..0 # Skip: Needs ActivePerl 820 or Perl 5.8.9 or later\n";
	exit 0;
    }
    if ((((Win32::FsType())[1] & 4) == 0) || (Win32::FsType() =~ /^FAT/)) {
	print "1..0 # Skip: Filesystem doesn't support Unicode\n";
	exit 0;
    }
    unless ((Win32::GetOSVersion())[1] > 4) {
	print "1..0 # Skip: Unicode support requires Windows 2000 or later\n";
	exit 0;
    }
    Win32::CreateFile("8dot3test_canary_Unicode $$");
    my $canary = Win32::GetShortPathName("8dot3test_canary_Unicode $$");
    unlink("8dot3test_canary_Unicode $$");
    if ( length $canary > 12 ) {
        print "1..0 # Skip: The system and/or current volume is not configured to support short names.\n";
        exit 0;        
    }
}

my $home = Win32::GetCwd();
my $cwd  = cwd(); # may be a Cygwin path
my $dir  = "Foo \x{394}\x{419} Bar \x{5E7}\x{645} Baz";
my $file = "$dir\\xyzzy \x{394}\x{419} plugh \x{5E7}\x{645}";

sub cleanup {
    chdir($home);
    my $ansi = Win32::GetANSIPathName($file);
    unlink($ansi) if -f $ansi;
    $ansi = Win32::GetANSIPathName($dir);
    rmdir($ansi) if -d $ansi;
}

cleanup();
END { cleanup() }

plan test => 12;

# Create Unicode directory
Win32::CreateDirectory($dir);
ok(-d Win32::GetANSIPathName($dir));

# Create Unicode file
Win32::CreateFile($file);
ok(-f Win32::GetANSIPathName($file));

# readdir() returns ANSI form of Unicode filename
ok(opendir(my $dh, Win32::GetANSIPathName($dir)));
while ($_ = readdir($dh)) {
    next if /^\./;
    # On Cygwin 1.7 readdir() returns the utf8 representation of the
    # filename but doesn't turn on the SvUTF8 bit
    Encode::_utf8_on($_) if $^O eq "cygwin" && $Config{osvers} !~ /^1.5/;
    ok($file, Win32::GetLongPathName("$dir\\$_"));
}
closedir($dh);

# Win32::GetLongPathName() of the absolute path restores the Unicode dir name
my $full = Win32::GetFullPathName($dir);
my $long = Win32::GetLongPathName($full);

ok($long, Win32::GetLongPathName($home)."\\$dir");

# We can Win32::SetCwd() into the Unicode directory
ok(Win32::SetCwd($dir));

my $w32dir = Win32::GetCwd();
# cwd() also returns a usable ANSI directory name
my $subdir = cwd();

# change back to home directory to make sure relative paths
# in @INC continue to work
ok(chdir($home));
ok(Win32::GetCwd(), $home);

ok(Win32::GetLongPathName($w32dir), $long);

# cwd() on Cygwin returns a mapped path that we need to translate
# back to a Windows path. Invoking `cygpath` on $subdir doesn't work.
if ($^O eq "cygwin") {
    $subdir = Cygwin::posix_to_win_path($subdir, 1);
}
$subdir =~ s,/,\\,g;
# Cygwin64 no longer returns an ANSI name
skip($^O eq "cygwin", Win32::GetLongPathName($subdir), $long);

# We can chdir() into the Unicode directory if we use the ANSI name
ok(chdir(Win32::GetANSIPathName($dir)));
ok(Win32::GetLongPathName(Win32::GetCwd()), $long);
