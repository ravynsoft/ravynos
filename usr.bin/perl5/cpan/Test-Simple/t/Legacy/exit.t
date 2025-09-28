#!/usr/bin/perl -w
# HARNESS-NO-STREAM

# Can't use Test.pm, that's a 5.005 thing.
package My::Test;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

require Test::Builder;
my $TB = Test::Builder->create();
$TB->level(0);


package main;

use Cwd;
use File::Spec;

my $Orig_Dir = cwd;

my $Perl = File::Spec->rel2abs($^X);
if( $^O eq 'VMS' ) {
    # VMS can't use its own $^X in a system call until almost 5.8
    $Perl = "MCR $^X" if $] < 5.007003;

    # Quiet noisy 'SYS$ABORT'
    $Perl .= q{ -"I../lib"} if $ENV{PERL_CORE};
    $Perl .= q{ -"Mvmsish=hushed"};
} else {
    $Perl = qq("$Perl"); # protect from shell if spaces
}

eval { require POSIX; &POSIX::WEXITSTATUS(0) };
if( $@ ) {
    *exitstatus = sub { $_[0] >> 8 };
}
else {
    *exitstatus = sub { POSIX::WEXITSTATUS($_[0]) }
}


# Some OS' will alter the exit code to their own native sense...
# sometimes.  Rather than deal with the exception we'll just
# build up the mapping.
print "# Building up a map of exit codes.  May take a while.\n";
my %Exit_Map;

open my $fh, ">", "exit_map_test" or die $!;
print $fh <<'DONE';
if ($^O eq 'VMS') {
    require vmsish;
    import vmsish qw(hushed);
}
my $exit = shift;
print "exit $exit\n";
END { $? = $exit };
DONE

close $fh;
END { 1 while unlink "exit_map_test" }

for my $exit (0..255) {
    # This correctly emulates Test::Builder's behavior.
    my $out = qx[$Perl exit_map_test $exit];
    $TB->like( $out, qr/^exit $exit\n/, "exit map test for $exit" );
    $Exit_Map{$exit} = exitstatus($?);
}
print "# Done.\n";


my %Tests = (
             # File                        Exit Code
             'success.plx'              => 0,
             'one_fail.plx'             => 1,
             'two_fail.plx'             => 2,
             'five_fail.plx'            => 5,
             'extras.plx'               => 2,
             'too_few.plx'              => 255,
             'too_few_fail.plx'         => 2,
             'death.plx'                => 255,
             'last_minute_death.plx'    => 255,
             'pre_plan_death.plx'       => 'not zero',
             'death_in_eval.plx'        => 0,
             'require.plx'              => 0,
             'death_with_handler.plx'   => 255,
             'exit.plx'                 => 1,
             'one_fail_without_plan.plx'    => 1,
             'missing_done_testing.plx'     => 254,
            );

chdir 't';
my $lib = File::Spec->catdir(qw(lib Test Simple sample_tests));
while( my($test_name, $exit_code) = each %Tests ) {
    my $file = File::Spec->catfile($lib, $test_name);
    my $wait_stat = system(qq{$Perl -"I../blib/lib" -"I../lib" -"I../t/lib" $file});
    my $actual_exit = exitstatus($wait_stat);

    if( $exit_code eq 'not zero' ) {
        $TB->isnt_num( $actual_exit, $Exit_Map{0},
                      "$test_name exited with $actual_exit ".
                      "(expected non-zero)");
    }
    else {
        $TB->is_num( $actual_exit, $Exit_Map{$exit_code}, 
                      "$test_name exited with $actual_exit ".
                      "(expected $Exit_Map{$exit_code})");
    }
}

$TB->done_testing( scalar keys(%Tests) + 256 );

# So any END block file cleanup works.
chdir $Orig_Dir;
