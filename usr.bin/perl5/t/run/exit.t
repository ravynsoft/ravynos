#!./perl
#
# Tests for perl exit codes, playing with $?, etc...


BEGIN {
    chdir 't' if -d 't';
    @INC = qw(. ../lib);
}

# Run some code, return its wait status.
sub run {
    my($code) = shift;
    $code = "\"" . $code . "\"" if $^O eq 'VMS'; #VMS needs quotes for this.
    return system($^X, "-e", $code);
}

BEGIN {
    $numtests = ($^O eq 'VMS') ? 16 : 17;
}


my $vms_exit_mode = 0;

if ($^O eq 'VMS') {
    if (eval 'require VMS::Feature') {
        $vms_exit_mode = !(VMS::Feature::current("posix_exit"));
    } else {
        my $env_unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $env_posix_ex = $ENV{'PERL_VMS_POSIX_EXIT'} || '';
        my $unix_rpt = $env_unix_rpt =~ /^[ET1]/i; 
        my $posix_ex = $env_posix_ex =~ /^[ET1]/i;
        if (($unix_rpt || $posix_ex) ) {
            $vms_exit_mode = 0;
        } else {
            $vms_exit_mode = 1;
        }
    }
    $numtests = 29 unless $vms_exit_mode;
}

require "./test.pl";
plan(tests => $numtests);

my $native_success = 0;
   $native_success = 1 if $^O eq 'VMS';

my $exit, $exit_arg;

$exit = run('exit');
is( $exit >> 8, 0,              'Normal exit' );
is( $exit, $?,                  'Normal exit $?' );
is( ${^CHILD_ERROR_NATIVE}, $native_success,  'Normal exit ${^CHILD_ERROR_NATIVE}' );

if (!$vms_exit_mode) {
  my $posix_ok = eval { require POSIX; };
  my $wait_macros_ok = defined &POSIX::WIFEXITED;
  eval { POSIX::WIFEXITED(${^CHILD_ERROR_NATIVE}) };
  $wait_macros_ok = 0 if $@;
  $exit = run('exit 42');
  is( $exit >> 8, 42,             'Non-zero exit' );
  is( $exit, $?,                  'Non-zero exit $?' );
  isnt( !${^CHILD_ERROR_NATIVE}, 0, 'Non-zero exit ${^CHILD_ERROR_NATIVE}' );
  SKIP: {
    skip("No POSIX", 3) unless $posix_ok;
    skip("No POSIX wait macros", 3) unless $wait_macros_ok;
    ok(POSIX::WIFEXITED(${^CHILD_ERROR_NATIVE}), "WIFEXITED");
    ok(!POSIX::WIFSIGNALED(${^CHILD_ERROR_NATIVE}), "WIFSIGNALED");
    is(POSIX::WEXITSTATUS(${^CHILD_ERROR_NATIVE}), 42, "WEXITSTATUS");
  }

  SKIP: {
    skip("Skip signals and core dump tests on Win32 and VMS", 7) 
        if ($^O eq 'MSWin32' || $^O eq 'VMS');

    #TODO VMS will backtrace on this test and exits with code of 0
    #instead of 15.

    $exit = run('kill 15, $$; sleep(1);');

    is( $exit & 127, 15,            'Term by signal' );
    ok( !($exit & 128),             'No core dump' );
    is( $? & 127, 15,               'Term by signal $?' );
    isnt( ${^CHILD_ERROR_NATIVE},  0, 'Term by signal ${^CHILD_ERROR_NATIVE}' );
    SKIP: {
      skip("No POSIX", 3) unless $posix_ok;
      skip("No POSIX wait macros", 3) unless $wait_macros_ok;
      ok(!POSIX::WIFEXITED(${^CHILD_ERROR_NATIVE}), "WIFEXITED");
      ok(POSIX::WIFSIGNALED(${^CHILD_ERROR_NATIVE}), "WIFSIGNALED");
      is(POSIX::WTERMSIG(${^CHILD_ERROR_NATIVE}), 15, "WTERMSIG");
    }
  }

}

if ($^O eq 'VMS') {

# On VMS, successful returns from system() are reported 0,  VMS errors that
# can not be translated to UNIX are reported as EVMSERR, which has a value
# of 65535. Codes from 2 through 7 are assumed to be from non-compliant
# VMS systems and passed through.  Programs written to use _POSIX_EXIT()
# codes like GNV will pass the numbers 2 through 255 encoded in the
# C facility by multiplying the number by 8 and adding %x35A000 to it.
# Perl will decode that number from children back to it's internal status.
#
# For native VMS status codes, success codes are odd numbered, error codes
# are even numbered.  The 3 LSBs of the code indicate if the success is
# an informational message or the severity of the failure.
#
# Because the failure codes for the tests of the CLI facility status codes can
# not be translated to UNIX error codes, they will be reported as EVMSERR,
# even though Perl will exit with them having the VMS status codes.
#
# Note that this is testing the perl exit() routine, and not the VMS
# DCL EXIT statement.
#
# The value %x1000000 has been added to the exit code to prevent the
# status message from being sent to the STDOUT and STDERR stream.
#
# Double quotes are needed to pass these commands through DCL to PERL

  $exit = run("exit 268632065"); # %CLI-S-NORMAL
  is( $exit >> 8, 0,             'PERL success exit' );
  is( ${^CHILD_ERROR_NATIVE} & 7, 1, 'VMS success exit' );

  $exit = run("exit 268632067");  # %CLI-I-NORMAL
  is( $exit >> 8, 0,             'PERL informational exit' );
  is( ${^CHILD_ERROR_NATIVE} & 7, 3, 'VMS informational exit' );

  $exit = run("exit 268632064");  # %CLI-W-NORMAL
  is( $exit >> 8, 1,             'Perl warning exit' );
  is( ${^CHILD_ERROR_NATIVE} & 7, 0, 'VMS warning exit' );

  $exit = run("exit 268632066");  # %CLI-E-NORMAL
  is( $exit >> 8, 2,             'Perl error exit' );
  is( ${^CHILD_ERROR_NATIVE} & 7, 2, 'VMS error exit' );

  $exit = run("exit 268632068");  # %CLI-F-NORMAL
  is( $exit >> 8, 4,             'Perl fatal error exit' );
  is( ${^CHILD_ERROR_NATIVE} & 7, 4, 'VMS fatal exit' );

  $exit = run("exit 02015320012"); # POSIX exit code 1
  is( $exit >> 8, 1,	                 'Posix exit code 1' );

  $exit = run("exit 02015323771"); # POSIX exit code 255
  is( $exit >> 8 , 255,	                 'Posix exit code 255' );
}

$exit_arg = 42;
$exit = run("END { \$? = $exit_arg }");

# On VMS, in the child process the actual exit status will be SS$_ABORT,
# or 44, which is what you get from any non-zero value of $? except for
# 65535 that has been dePOSIXified by STATUS_UNIX_SET.  If $? is set to
# 65535 internally when there is a VMS status code that is valid, and
# when Perl exits, it will set that status code.
#
# In this test on VMS, the child process exit with a SS$_ABORT, which
# the parent stores in ${^CHILD_ERROR_NATIVE}.  The SS$_ABORT code is
# then translated to the UNIX code EINTR which has the value of 4 on VMS.
#
# This is complex because Perl translates internally generated UNIX
# status codes to SS$_ABORT on exit, but passes through unmodified UNIX
# status codes that exit() is called with by scripts.

$exit_arg = (44 & 7) if $vms_exit_mode;

is( $exit >> 8, $exit_arg,             'Changing $? in END block' );
