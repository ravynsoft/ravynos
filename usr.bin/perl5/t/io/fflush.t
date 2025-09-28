#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# Script to test auto flush on fork/exec/system/qx.  The idea is to
# print "Pe" to a file from a parent process and "rl" to the same file
# from a child process.  If buffers are flushed appropriately, the
# file should contain "Perl".  We'll see...
use Config;
use warnings;
use strict;

# This attempts to mirror the #ifdef forest found in perl.h so that we
# know when to run these tests.  If that forest ever changes, change
# it here too or expect test gratuitous test failures.
my $useperlio = defined $Config{useperlio} ? $Config{useperlio} eq 'define' ? 1 : 0 : 0;
my $fflushNULL = defined $Config{fflushNULL} ? $Config{fflushNULL} eq 'define' ? 1 : 0 : 0;
my $fflushall = defined $Config{fflushall} ? $Config{fflushall} eq 'define' ? 1 : 0 : 0;
my $d_fork = defined $Config{d_fork} ? $Config{d_fork} eq 'define' ? 1 : 0 : 0;

skip_all('fflush(NULL) or equivalent not available')
    unless $useperlio || $fflushNULL || $fflushall;

plan(tests => 7);

my $runperl = $^X =~ m/\s/ ? qq{"$^X"} : $^X;
$runperl .= qq{ "-I../lib"};

sub file_eq {
    my $f   = shift;
    my $val = shift;

    open IN, $f or die "open $f: $!";
    chomp(my $line = <IN>);
    close IN;

    print "# got $line\n";
    print "# expected $val\n";
    return $line eq $val;
}

# This script will be used as the command to execute from
# child processes
my $ffprog = tempfile();
open PROG, "> $ffprog" or die "open $ffprog: $!";
print PROG <<'EOF';
my $f = shift;
my $str = shift;
open OUT, ">> $f" or die "open $f: $!";
print OUT $str;
close OUT;
EOF
    ;
close PROG or die "close $ffprog: $!";;

$| = 0; # we want buffered output

# Test flush on fork/exec
if (!$d_fork) {
    print "ok 1 # skipped: no fork\n";
} else {
    my $f = tempfile();
    open OUT, "> $f" or die "open $f: $!";
    print OUT "Pe";
    my $pid = fork;
    if ($pid) {
	# Parent
	wait;
	close OUT or die "close $f: $!";
    } elsif (defined $pid) {
	# Kid
	print OUT "r";
	my $command = qq{$runperl "$ffprog" "$f" "l"};
	print "# $command\n";
	exec $command or die $!;
	exit;
    } else {
	# Bang
	die "fork: $!";
    }

    print file_eq($f, "Perl") ? "ok 1\n" : "not ok 1\n";
}

# Test flush on system/qx/pipe open
my %subs = (
            "system" => sub {
                my $c = shift;
                system $c;
            },
            "qx"     => sub {
                my $c = shift;
                qx{$c};
            },
            "popen"  => sub {
                my $c = shift;
                open PIPE, "$c|" or die "$c: $!";
                close PIPE;
            },
            );
my $t = 2;
for (qw(system qx popen)) {
    my $code    = $subs{$_};
    my $f       = tempfile();
    my $command = qq{$runperl $ffprog "$f" "rl"};
    open OUT, "> $f" or die "open $f: $!";
    print OUT "Pe";
    close OUT or die "close $f: $!";;
    print "# $command\n";
    $code->($command);
    print file_eq($f, "Perl") ? "ok $t\n" : "not ok $t\n";
    ++$t;
}

my $cmd = _create_runperl(
			  switches => ['-l'],
			  prog =>
			  sprintf('print qq[ok $_] for (%d..%d)', $t, $t+2));
print "# cmd = '$cmd'\n";
open my $CMD, "$cmd |" or die "Can't open pipe to '$cmd': $!";
while (<$CMD>) {
    system("$runperl -e 0");
    print;
}
close $CMD;
$t += 3;
curr_test($t);
