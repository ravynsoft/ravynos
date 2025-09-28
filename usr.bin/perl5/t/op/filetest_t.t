#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

plan 7;

my($dev_tty, $dev_null) = qw(/dev/tty /dev/null);
  ($dev_tty, $dev_null) = qw(con      nul      ) if $^O =~ /^(MSWin32|os2)$/;
  ($dev_tty, $dev_null) = qw(TT:      _NLA0:   ) if $^O eq "VMS";

SKIP: {
    open(my $tty, "<", $dev_tty)
	or skip("Can't open terminal '$dev_tty': $!", 4);
    if ($^O eq 'VMS') {
        # TT might be a mailbox or other non-terminal device
        my $tt_dev = VMS::Filespec::vmspath('TT');
        skip("'$tt_dev' is probably not a terminal", 4) if $tt_dev !~ m/^_(tt|ft|rt)/i;
    }
    ok(-t $tty, "'$dev_tty' is a TTY");
    ok(-t -e $tty, "'$dev_tty' is a TTY (with -t -e)");
    -e 'mehyparchonarcheion'; # clear last stat buffer
    ok(-e -t $tty, "'$dev_tty' is a TTY (with -e -t)");
    -e 'mehyparchonarcheion';
    ok(-e -t -t $tty, "'$dev_tty' is a TTY (with -e -t -t)");
}
SKIP: {
    open(my $null, "<", $dev_null)
	or skip("Can't open null device '$dev_null': $!", 3);
    ok(!-t $null, "'$dev_null' is not a TTY");
    ok(!-t -e $null, "'$dev_null' is not a TTY (with -t -e)");
    ok(!-e -t $null, "'$dev_null' is not a TTY (with -e -t)");
}
