#!/usr/bin/perl

use strict;
use warnings FATAL => 'all';
use English;

use Config;
use Test::More;
use ExtUtils::PL2Bat;
use Cwd qw/cwd/;

my @test_vals = ( 0, 1, 2, 3, -1, -2, 65535, 65536, 65537, 47, 100, 200, 255, 256, 257, 258, 511, 512, 513, -255, -256, -20012001 );

plan($OSNAME eq 'MSWin32' ? ( tests => (($#test_vals+1)*5)+2 ) : ( skip_all => 'Only usable on Windows' ));

# the method of execution of the test script is geared to cmd.exe so ensure
# this is used in case the user have some non-standard shell.
# E.g. TCC/4NT doesn't quite handle the invocation correctly producing errors.
$ENV{COMSPEC} = "$ENV{SystemRoot}\\System32\\cmd.exe";

my $perl_in_fname = 'test_perl_source';

open my $out, '>', $perl_in_fname or die qq{Couldn't create source file ("$perl_in_fname"): $!};
print $out "#! perl -w\nexit \$ARGV[0];\n";
close $out;

pl2bat(in => $perl_in_fname);

my $batch_out_fname = $perl_in_fname.'.bat';

ok (-e "$batch_out_fname", qq{Executable file exists ("$batch_out_fname")});

my $int_max_8bit = 2**8;
my $int_max_16bit = 2**16;

my $path_with_cwd = construct_test_PATH();

foreach my $input_val ( @test_vals ) {
    local $ENV{PATH} = $path_with_cwd;
    my $qx_output = q//;
    my $qx_retval = 0;
    my $error_level = 0;
    my $status = q//;
    my $success = 1;

    $success &&= eval { $qx_output = qx{"$batch_out_fname" $input_val}; $qx_retval = $CHILD_ERROR; $qx_retval != -1; };
    $qx_retval = ( $qx_retval > 0 ) ? ( $qx_retval >> 8 ) : $qx_retval;

    $success &&= eval { $error_level = qx{"$batch_out_fname" $input_val & call echo ^%ERRORLEVEL^%}; 1; };
    $error_level =~ s/\r?\n$//msx;

    $success &&= eval { $status = qx{"$batch_out_fname" $input_val && (echo PROCESS_SUCCESS) || (echo PROCESS_FAILURE)}; 1; };
    $status =~ s/\r?\n$//msx;

    # (for qx/.../) post-call status values ($CHILD_ERROR) can be [ 0 ... 255 ]; values outside that range will be returned as `value % 256`
    my $expected_qx_retval = ($input_val % $int_max_8bit);

    # `exit $value` will set ERRORLEVEL to $value for values of [ -1, 0 ... 65535 ]; values outside that range will set ERRORLEVEL to `$value % 65536`
    my $expected_error_level = ($input_val == -1) ? -1 : ($input_val % $int_max_16bit);

    is $success, 1, qq{`"$batch_out_fname" $input_val` executed successfully};
    is $qx_output, q//, qq{qx/"$batch_out_fname" $input_val/ returns expected empty output}; # assure no extraneous output from BAT wrap
    is $qx_retval, $expected_qx_retval, qq{qx/"$batch_out_fname" $input_val/ returns expected $CHILD_ERROR ($expected_qx_retval)};
    is $error_level, $expected_error_level, qq{"$batch_out_fname": `exit $input_val` set expected ERRORLEVEL ($expected_error_level)};
    is $status, (($input_val % $int_max_16bit) == 0) ? 'PROCESS_SUCCESS' : 'PROCESS_FAILURE', qq{`"$batch_out_fname" $input_val` process exit ($status) is correct};
}

unlink $perl_in_fname, $batch_out_fname;

# the test needs CWD in PATH to check the created .bat files, but under win2k
# PATH must not be too long. so to keep any win2k smokers happy, we construct
# a new PATH that contains the dirs which hold cmd.exe, perl.exe, and CWD

sub construct_test_PATH {
    my $perl_path = $^X;
    my $cmd_path = $ENV{ComSpec} ||  `where cmd`; # where doesn't seem to work on all windows versions
    $_ =~ s/[\\\/][^\\\/]+$// for $perl_path, $cmd_path; # strip executable names

    my @path_fallbacks = grep /\Q$ENV{SystemRoot}\E|system32|winnt|windows/i, split $Config{path_sep}, $ENV{PATH};

    my $path_with_cwd = join $Config{path_sep}, @path_fallbacks, $cmd_path, $perl_path, cwd();

    my ($perl) = ( $^X =~ /[\\\/]([^\\]+)$/ ); # in case the perl executable name differs
    note "using perl executable name: $perl";

    local $ENV{PATH} = $path_with_cwd;
    my $test_out = `$perl -e 1 2>&1`;
    is $test_out, "", "perl execution with temp path works"
        or diag "make_executable.t tmp path: $path_with_cwd";
    diag "make_executable.t PATH likely did not contain cmd.exe"
        if !defined $test_out;

    return $path_with_cwd;
}
