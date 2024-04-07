#!./perl -w

#
# Verify that C<die> return the return code
#	-- Robin Barker 
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

$| = 1;

my @tests = (
	[   0,   0],
	[   0,   1],
	[   0, 127],
	[   0, 128],
	[   0, 255],
	[   0, 256],
	[   0, 512],
	[   1,   0],
	[   1,   1],
	[   1, 256],
	[ 128,   0],
	[ 128,   1],
	[ 128, 256],
	[ 255,   0],
	[ 255,   1],
	[ 255, 256],
	# see if implicit close preserves $?
	[  0,  512, '{ local *F; open F, q[TEST]; close F; $!=0 } die;'],
);

plan(tests => scalar @tests);

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
}

# Dump any error messages from the dying processes off to a temp file.
my $tempfile = tempfile();
open STDERR, '>', $tempfile or die "Can't open temp error file $tempfile:  $!";

foreach my $test (@tests) {
    my($bang, $query, $code) = @$test;
    $code ||= 'die;';
    if ($^O eq 'MSWin32' || $^O eq 'VMS') {
        system(qq{$^X -e "\$! = $bang; \$? = $query; $code"});
    }
    else {
        system(qq{$^X -e '\$! = $bang; \$? = $query; $code'});
    }
    my $exit = $?;

    # The legacy VMS exit code 44 (SS$_ABORT) is returned if a program dies.
    # We only get the severity bits, which boils down to 4.  See L<perlvms/$?>.
    $bang = 4 if $vms_exit_mode;

    is($exit, (($bang || ($query >> 8) || 255) << 8),
       sprintf "exit = 0x%04x bang = 0x%04x query = 0x%04x", $exit, $bang, $query);
}

close STDERR;
