#!perl

BEGIN {
    chdir 't' if -d 't';
    # We need '../../lib' as well as '../lib' because parts of Config are
    # delay-loaded, after we've chdir()'ed into $testdir.
    @INC = ('../lib', '../../lib');
    # XXX this could be further munged to enable some parts on other
    # platforms
    require './test.pl';
}

BEGIN {
    unless ($^O =~ /^MSWin/) {
        skip_all 'windows specific test';
    }
}

use File::Path;
use File::Copy;
use Config;
use Cwd;
use strict;

$| = 1;

my $cwd = cwd();

my $testdir = "t e s t";
my $exename = "showav";
my $plxname = "showargv";
rmtree($testdir);
mkdir($testdir);
die "Could not create '$testdir':$!" unless -d $testdir;

open(my $F, ">$testdir/$exename.c")
    or die "Can't create $testdir/$exename.c: $!";
print $F <<'EOT';
#include <stdio.h>
int
main(int ac, char **av)
{
    int i;
    for (i = 0; i < ac; i++)
	printf("[%s]", av[i]);
    printf("\n");
    return 0;
}
EOT

open($F, ">$testdir/$plxname.bat")
    or die "Can't create $testdir/$plxname.bat: $!";
print $F <<'EOT';
@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
EOT

print $F <<EOT;
"$^X" -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
"$^X" -x -S %0 %*
EOT
print $F <<'EOT';
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!perl
#line 15
print "[$_]" for ($0, @ARGV);
print "\n";
__END__
:endofperl
EOT

close $F;

# build the executable
chdir($testdir);
END {
    chdir($cwd) && rmtree("$cwd/$testdir") if -d "$cwd/$testdir";
    unlink "cmd.exe";
}
if (open(my $EIN, "$cwd/win32/${exename}_exe.uu")) {
    note "Unpacking $exename.exe";
    my $e;
    {
	local $/;
	$e = unpack "u", <$EIN>;
	close $EIN;
    }
    open my $EOUT, ">$exename.exe" or die "Can't write $exename.exe: $!";
    binmode $EOUT;
    print $EOUT $e;
    close $EOUT;
}
else {
    my $minus_o = '';
    if ($Config{cc} =~ /\bgcc/i)
     {
      $minus_o = "-o $exename.exe";
     }
    note "Compiling $exename.c";
    note "$Config{cc} $Config{ccflags} $exename.c";
    if (system("$Config{cc} $Config{ccflags} $minus_o $exename.c >log 2>&1") != 0 ||
        !-f "$exename.exe") {
	note "Could not compile $exename.c, status $?";
        note "Where is your C compiler?";
        if (open(LOG,'<log'))
        {
            while(<LOG>) {
                note $_;
            }
        }
        else {
            warn "Cannot open log (in $testdir):$!";
        }
        skip_all "can't build test executable";
    }
}
copy("$plxname.bat","$plxname.cmd");
chdir($cwd);
unless (-x "$testdir/$exename.exe") {
    note "Could not build $exename.exe";
    skip_all "can't build test executable";
}

# test we only look for cmd.exe in the standard place
delete $ENV{PERLSHELL};
copy("$testdir/$exename.exe", "$testdir/cmd.exe") or die $!;
copy("$testdir/$exename.exe", "cmd.exe") or die $!;
$ENV{PATH} = qq("$testdir";$ENV{PATH});

open my $T, "$^X -I../lib -w win32/system_tests |"
    or die "Can't spawn win32/system_tests: $!";
my $expect;
my $comment = "";
while (<$T>) {
    chomp;
    if (s/^1\.\.//) {
	plan $_;
    }
    elsif (/^#+\s(.*)$/) {
	$comment = $1;
    }
    elsif (/^</) {
	$expect = $_;
	$expect =~ tr/<>/[]/;
	$expect =~ s/\Q$plxname\E]/$plxname.bat]/;
    }
    else {
	if ($expect ne $_) {
	    note $comment if $comment;
	    note "want: $expect";
	    note "got : $_";
	}
	ok($expect eq $_, $comment // '');
    }
}
close $T;
