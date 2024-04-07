#!./perl -w

# What does this test?
# Test that changes to perl header files don't cause external
# references by simplying #including them.  This breaks library probe
# code on CPAN, and can break cflags.SH.
#
# Why do we test this?
# See https://github.com/Perl/perl5/issues/12824
#
# It's broken - how do I fix it?
# You added an initializer or static function to a header file that
# references some symbol you didn't define, you need to remove it.

BEGIN {
  require "./test.pl";
  unshift @INC, ".." if -f "../TestInit.pm";
}

use TestInit qw(T A); # T is chdir to the top level, A makes paths absolute
use strict;
use warnings;
use Config;
use File::Path 'rmtree';
use Cwd;
use IPC::Cmd qw(can_run);

if ($Config{'usecrosscompile'} && !can_run($Config{'cc'})) {
    skip_all("compiler not available (cross-compiling)");
} else {
    plan(tests => 1);
}

my $VERBOSE = grep {$_ eq '-v'} @ARGV;

ok(try_compile_and_link(<<'CODE'));
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

int main(int argc, char **argv) {
  return 0;
}
CODE


# from Time::HiRes's Makefile.PL with minor modifications
sub try_compile_and_link {
    my ($c, %args) = @_;

    my $ld_exeext = ($^O eq 'cygwin' || $^O eq 'MSWin32' ||
                 $^O eq 'os2' && $Config{ldflags} =~ /-Zexe\b/) ? '.exe' :
                (($^O eq 'vos') ? $Config{exe_ext} : '');

    my ($ok) = 0;
    my $tempdir = tempfile();
    my $cwd = getcwd();
    mkdir $tempdir;
    chdir $tempdir;
    my ($tmp) = "temp";

    my $obj_ext = $Config{obj_ext} || ".o";

    if (open(my $tmpc, ">$tmp.c")) {
	print $tmpc $c;
	unless (close($tmpc)) {
	    chdir($cwd);
	    rmtree($tempdir);
	    warn "Failing closing code file: $!\n" if $VERBOSE;
	    return 0;
	}

	my $COREincdir =
	    File::Spec->catdir(File::Spec->updir, File::Spec->updir);

	my $ccflags = $Config{'ccflags'} . ' ' . "-I$COREincdir"
	 . ' -DPERL_NO_INLINE_FUNCTIONS';

	if ($^O eq "MSWin32") {
	    $ccflags .= " -I../../win32 -I../../win32/include";
	}

	my $libs = '';

	# Include libs to be sure of linking against bufferoverflowU.lib for
	# the SDK2003 compiler on Windows. See win32/Makefile for more details.
	if ($^O eq "MSWin32" && $Config{cc} =~ /\bcl\b/i) {
	    $libs = " /link $Config{'libs'}";
	}

	my $null = File::Spec->devnull;

	my $errornull = $VERBOSE ? '' : ">$null 2>$null";

	# Darwin g++ 4.2.1 is fussy and demands a space.
	# FreeBSD g++ 4.2.1 does not.
	# We do not know the reaction of either to the presence of brown M&Ms.
	my $out_opt = "-o ";
	if ($^O eq "MSWin32" && $Config{cc} =~ /\bcl\b/i) {
	    $out_opt = "/Fe";
	}

	my $tmp_exe = "$tmp$ld_exeext";

        my $cccmd = "$Config{'cc'} $out_opt$tmp_exe $ccflags $tmp.c $libs $errornull";

	if ($^O eq 'VMS') {
            $cccmd = "$Config{'cc'} /include=($COREincdir) $tmp.c";
        }

       if ($^O eq 'VMS') {
	    open( my $cmdfile, ">$tmp.com" );
	    print $cmdfile "\$ SET MESSAGE/NOFACILITY/NOSEVERITY/NOIDENT/NOTEXT\n";
	    print $cmdfile "\$ $cccmd\n";
	    print $cmdfile "\$ IF \$SEVERITY .NE. 1 THEN EXIT 44\n"; # escalate
	    close $cmdfile;
	    system("\@ $tmp.com");
	    $ok = $?==0;
	    chdir($cwd);
	    rmtree($tempdir);
        }
        else
        {
	    printf "cccmd = $cccmd\n" if $VERBOSE;
	    my $res = system($cccmd);
	    $ok = defined($res) && $res == 0 && -s $tmp_exe && -x _;

	    chdir($cwd);
	    rmtree($tempdir);
        }
    }

    return $ok;
}
