#!./perl
#
# Test inheriting file descriptors across exec (close-on-exec).
#
# perlvar describes $^F aka $SYSTEM_FD_MAX as follows:
#
#  The maximum system file descriptor, ordinarily 2.  System file
#  descriptors are passed to exec()ed processes, while higher file
#  descriptors are not.  Also, during an open(), system file descriptors
#  are preserved even if the open() fails.  (Ordinary file descriptors
#  are closed before the open() is attempted.)  The close-on-exec
#  status of a file descriptor will be decided according to the value of
#  C<$^F> when the corresponding file, pipe, or socket was opened, not
#  the time of the exec().
#
# This documented close-on-exec behaviour is typically implemented in
# various places (e.g. pp_sys.c) with code something like:
#
#  #if defined(HAS_FCNTL) && defined(F_SETFD)
#      fcntl(fd, F_SETFD, fd > PL_maxsysfd);  /* ensure close-on-exec */
#  #endif
#
# This behaviour, therefore, is only currently implemented for platforms
# where:
#
#  a) HAS_FCNTL and F_SETFD are both defined
#  b) Integer fds are native OS handles
#
# ... which is typically just the Unix-like platforms.
#
# Notice that though integer fds are supported by the C runtime library
# on Windows, they are not native OS handles, and so are not inherited
# across an exec (though native Windows file handles are).

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    skip_all_without_config('d_fcntl');
}

use strict;

$|=1;

# When in doubt, skip.
skip_all($^O)
    if $^O eq 'VMS' or $^O eq 'MSWin32' or $^O eq 'amigaos';

sub make_tmp_file {
    my ($fname, $fcontents) = @_;
    local *FHTMP;
    open   FHTMP, ">$fname"  or die "open  '$fname': $!";
    print  FHTMP $fcontents  or die "print '$fname': $!";
    close  FHTMP             or die "close '$fname': $!";
}

my $Perl = which_perl();
my $quote = "'";

my $tmperr             = tempfile();
my $tmpfile1           = tempfile();
my $tmpfile2           = tempfile();
my $tmpfile1_contents  = "tmpfile1 line 1\ntmpfile1 line 2\n";
my $tmpfile2_contents  = "tmpfile2 line 1\ntmpfile2 line 2\n";
make_tmp_file($tmpfile1, $tmpfile1_contents);
make_tmp_file($tmpfile2, $tmpfile2_contents);

# $Child_prog is the program run by the child that inherits the fd.
# Note: avoid using ' or " in $Child_prog since it is run with -e
my $Child_prog = <<'CHILD_PROG';
my $fd = shift;
print qq{childfd=$fd\n};
open INHERIT, qq{<&=$fd} or die qq{open $fd: $!};
my $line = <INHERIT>;
close INHERIT or die qq{close $fd: $!};
print $line
CHILD_PROG
$Child_prog =~ tr/\n//d;

plan(tests => 22);

sub test_not_inherited {
    my $expected_fd = shift;
    ok( -f $tmpfile2, "tmpfile '$tmpfile2' exists" );
    my $cmd = qq{$Perl -e $quote$Child_prog$quote $expected_fd};
    # Expect 'Bad file descriptor' or similar to be written to STDERR.
    local *SAVERR; open SAVERR, ">&STDERR";  # save original STDERR
    open STDERR, ">$tmperr" or die "open '$tmperr': $!";
    my $out = `$cmd`;
    my $rc  = $? >> 8;
    open STDERR, ">&SAVERR" or die "error: restore STDERR: $!";
    close SAVERR or die "error: close SAVERR: $!";
    # XXX: it seems one cannot rely on a non-zero return code,
    # at least not on Tru64.
    # cmp_ok( $rc, '!=', 0,
    #     "child return code=$rc (non-zero means cannot inherit fd=$expected_fd)" );
    cmp_ok( $out =~ tr/\n//, '==', 1,
        "child stdout: has 1 newline (rc=$rc, should be non-zero)" );
    is( $out, "childfd=$expected_fd\n", 'child stdout: fd' );
}

sub test_inherited {
    my $expected_fd = shift;
    ok( -f $tmpfile1, "tmpfile '$tmpfile1' exists" );
    my $cmd = qq{$Perl -e $quote$Child_prog$quote $expected_fd};
    my $out = `$cmd`;
    my $rc  = $? >> 8;
    cmp_ok( $rc, '==', 0,
        "child return code=$rc (zero means inherited fd=$expected_fd ok)" );
    my @lines = split(/^/, $out);
    cmp_ok( $out =~ tr/\n//, '==', 2, 'child stdout: has 2 newlines' );
    cmp_ok( scalar(@lines),  '==', 2, 'child stdout: split into 2 lines' );
    is( $lines[0], "childfd=$expected_fd\n", 'child stdout: fd' );
    is( $lines[1], "tmpfile1 line 1\n",      'child stdout: line 1' );
}

$^F == 2 or print STDERR "# warning: \$^F is $^F (not 2)\n";

# Should not be able to inherit > $^F in the default case.
open FHPARENT2, "<$tmpfile2" or die "open '$tmpfile2': $!";
my $parentfd2 = fileno FHPARENT2;
defined $parentfd2 or die "fileno: $!";
cmp_ok( $parentfd2, '>', $^F, "parent open fd=$parentfd2 (\$^F=$^F)" );
test_not_inherited($parentfd2);
close FHPARENT2 or die "close '$tmpfile2': $!";

# Should be able to inherit $^F after setting to $parentfd2
# Need to set $^F before open because close-on-exec set at time of open.
$^F = $parentfd2;
open FHPARENT1, "<$tmpfile1" or die "open '$tmpfile1': $!";
my $parentfd1 = fileno FHPARENT1;
defined $parentfd1 or die "fileno: $!";
cmp_ok( $parentfd1, '<=', $^F, "parent open fd=$parentfd1 (\$^F=$^F)" );
test_inherited($parentfd1);
close FHPARENT1 or die "close '$tmpfile1': $!";

# ... and test that you cannot inherit fd = $^F+n.
open FHPARENT1, "<$tmpfile1" or die "open '$tmpfile1': $!";
open FHPARENT2, "<$tmpfile2" or die "open '$tmpfile2': $!";
$parentfd2 = fileno FHPARENT2;
defined $parentfd2 or die "fileno: $!";
cmp_ok( $parentfd2, '>', $^F, "parent open fd=$parentfd2 (\$^F=$^F)" );
test_not_inherited($parentfd2);
close FHPARENT2 or die "close '$tmpfile2': $!";
close FHPARENT1 or die "close '$tmpfile1': $!";

# ... and now you can inherit after incrementing.
$^F = $parentfd2;
open FHPARENT2, "<$tmpfile2" or die "open '$tmpfile2': $!";
open FHPARENT1, "<$tmpfile1" or die "open '$tmpfile1': $!";
$parentfd1 = fileno FHPARENT1;
defined $parentfd1 or die "fileno: $!";
cmp_ok( $parentfd1, '<=', $^F, "parent open fd=$parentfd1 (\$^F=$^F)" );
test_inherited($parentfd1);
close FHPARENT1 or die "close '$tmpfile1': $!";
close FHPARENT2 or die "close '$tmpfile2': $!";
