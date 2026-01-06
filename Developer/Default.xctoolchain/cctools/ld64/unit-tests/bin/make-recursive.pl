#!/usr/bin/perl 

use strict;
use Data::Dumper;
use File::Find;
use Cwd qw(realpath);
use English;

my @args = @ARGV;

$ENV{'LD_NO_CLASSSIC_LINKER'} = '1';
$ENV{'LD_NO_CLASSSIC_LINKER_STATIC'} = '1';

my $makefiles =
{
    'makefile' => undef,
    'Makefile' => undef,
};

my $find_opts =
{
    'wanted' => \&find_callback,
};

my $keywords =
{
    'root'   => '',
    'cwd'    => '',
    'cmd'    => '',
    'exit'   => '',
    'stdout' => [],
    'stderr' => [],
};

# Determine how many tests to run at a time in parallel. Default to cpu count.
my $max_concurrent_tests = $ENV{'LD_UNIT_TEST_CONCURRENCY'};
if (!defined $max_concurrent_tests) {
    # shell command returns cpu count in exit status
    system("/bin/csh", "-c", "set n=`sysctl hw.ncpu`; exit \$n[2]");
    if ($? == -1 || $? & 127) {
        die("could not determine cpu count");
    }
    $max_concurrent_tests = $? >> 8;
}

my $keyword;
my $max_keyword_len = 0;
foreach $keyword (keys %$keywords)
{ if($max_keyword_len < length($keyword)) { $max_keyword_len = length($keyword); } }
my $delim = ':';
$max_keyword_len += length($delim) + length(' ');

my $last_keyword = '';

sub print_line
{
    my ($file, $keyword, $val) = @_;
    
    if(!exists($$keywords{$keyword}))
    {
        print STDERR "error: keyword $keyword not in \$keywords set\n";
        exit(1);
    }
    
    my $keyword_len = 0;
    
    if($keyword ne $last_keyword)
    {
        print($file "$keyword"); print($file $delim);
        $keyword_len = length($keyword) + length($delim);
    }
    if($max_keyword_len > $keyword_len)
    {
        my $num_spaces = $max_keyword_len - $keyword_len;
        print($file ' ' x $num_spaces);
    }
    print($file "$val");
    if(0)
    {
        $last_keyword = $keyword;
    }
}

my $root = '.';
$root = &realpath($root);
print_line(*STDOUT, "root", "$root\n");
my $running_test_count=0;
find($find_opts, $root);
while ( $running_test_count > 0 ) {
    &reaper;
}

sub find_callback
{
    if(exists($$makefiles{$_}))
    {
        my $makefile = $_;
        my $reldir = $File::Find::dir;
        $reldir =~ s|^$root/||;
        
        my $cmd = [ "make" ];
        
        my $arg; foreach $arg (@ARGV) { push @$cmd, $arg; } # better way to do this?
        
        $ENV{UNIT_TEST_NAME} = $reldir;
        my $pid = fork();
        if (not defined $pid) {
            die "Couldn't fork"
        }
        elsif ($pid == 0) {
            # Child. Redirect stdout/stderr to files and exec test.
            open(STDOUT, ">/tmp/unit-tests-stdout.$PID") || die("$!");
            open(STDERR, ">/tmp/unit-tests-stderr.$PID") || die("$!");
            exec 'make', @ARGV;
            exit(-1);    #just to be sure
        }
        
        # Write the test cwd/cmd to a temporary file associated with the child's pid, to be retrieved later.
        my $info;
        open($info, ">/tmp/unit-tests-info.$pid") || die("$!");
        &print_line($info, "cwd", "\$root/$reldir\n"); # post filtering depends on this line being first
        &print_line($info, "cmd", "@$cmd\n");
        close($info) || die("$!");
        
        $running_test_count++;
        # if we have reached max # of concurrent tests, wait for one to exit
        if ( $running_test_count == $max_concurrent_tests ) {
            &reaper;
        }
    }
}

sub reaper {
	if ( $running_test_count > 0 ) {
		my $pid = wait;
        if ( $pid == -1 ) {
            die("no child\n");
        }
        my $exit = $?;
        
		$running_test_count--;
        
		open(INFO, "</tmp/unit-tests-info.$pid") || die("$!");
		while(<INFO>)
		{
            print $_;
		}
		close(INFO) || die("$!");
		unlink("/tmp/unit-tests-info.$pid");
        
        &print_line(*STDOUT, "exit", "$exit\n");
        
		open(OUT, "</tmp/unit-tests-stdout.$pid") || die("$!");
		while(<OUT>)
		{
		    &print_line(*STDOUT, "stdout", "$_");
		}
		close(OUT) || die("$!");
		unlink("/tmp/unit-tests-stdout.$pid");
        
		open(ERR, "</tmp/unit-tests-stderr.$pid") || die("$!");
		while(<ERR>)
		{
		    &print_line(*STDOUT, "stderr", "$_");
		}
		close(ERR) || die("$!");
		unlink("/tmp/unit-tests-stderr.$pid");
	}
}

