#!/usr/bin/perl

# test.pl
# Run unit tests.

use strict;
use File::Basename;

# We use encode_json() to write BATS plist files.
# JSON::PP does not exist on iOS devices, but we need not write plists there.
# So we simply load JSON:PP if it exists.
if (eval { require JSON::PP; 1; }) {
    JSON::PP->import();
}


chdir dirname $0;
chomp (my $DIR = `pwd`);

if (scalar(@ARGV) == 1) {
    my $arg = $ARGV[0];
    if ($arg eq "-h" || $arg eq "-H" || $arg eq "-help" || $arg eq "help") {
        print(<<END);
usage: $0 [options] [testname ...]
       $0 help

testname:
    `testname` runs a specific test. If no testnames are given, runs all tests.

options:
    ARCH=<arch>
    OS=<sdk name>[sdk version][-<deployment target>[-<run target>]]
    ROOT=/path/to/project.roots/

    CC=<compiler name>

    LANGUAGE=c,c++,objective-c,objective-c++,swift
    MEM=mrc,arc
    GUARDMALLOC=0|1|before|after

    BUILD=0|1      (build the tests?)
    RUN=0|1        (run the tests?)
    VERBOSE=0|1|2  (0=quieter  1=print commands executed  2=full test output)
    BATS=0|1       (build for and/or run in BATS?)
    BUILD_SHARED_CACHE=0|1  (build a dyld shared cache with the root and test against that)
    DYLD=2|3       (test in dyld 2 or dyld 3 mode)

examples:

    test installed library, x86_64
    $0

    test buildit-built root, i386 and x86_64, MRC and ARC, clang compiler
    $0 ARCH=i386,x86_64 ROOT=/tmp/objc4.roots MEM=mrc,arc CC=clang

    test buildit-built root with iOS simulator, deploy to iOS 7, run on iOS 8
    $0 ARCH=x86_64 ROOT=/tmp/objc4.roots OS=iphonesimulator-7.0-8.0

    test buildit-built root on attached iOS device
    $0 ARCH=arm64 ROOT=/tmp/objc4.roots OS=iphoneos
END
        exit 0;
    }
}

#########################################################################
## Tests

# Maps test name => test's filename extension.
# ex: "msgSend" => "m"
# `keys %ALL_TESTS` is also used as the list of all tests found on disk.
my %ALL_TESTS;

#########################################################################
## Variables for use in complex build and run rules

# variable         # example value

# things you can multiplex on the command line
# ARCH=i386,x86_64,armv6,armv7
# OS=macosx,iphoneos,iphonesimulator (plus sdk/deployment/run versions)
# LANGUAGE=c,c++,objective-c,objective-c++,swift
# CC=clang
# MEM=mrc,arc
# GUARDMALLOC=0,1,before,after

# things you can set once on the command line
# ROOT=/path/to/project.roots
# BUILD=0|1
# RUN=0|1
# VERBOSE=0|1|2
# BATS=0|1

# environment variables from the command line
# DSTROOT
# OBJROOT
# (SRCROOT is ignored; test sources are assumed to
# be in the same directory as the test script itself.)
# fixme SYMROOT for dsymutil output?


# Some arguments as read from the command line.
my %args;
my $BUILD;
my $RUN;
my $VERBOSE;
my $BATS;

my $HOST;
my $PORT;

my @TESTLIBNAMES = ("libobjc.A.dylib", "libobjc-trampolines.dylib");
my $TESTLIBDIR = "/usr/lib";

# Top level directory for intermediate and final build products.
# Intermediate files must be kept separate for XBS BATS builds.
my $OBJROOT = $ENV{OBJROOT} || "";
my $DSTROOT = $ENV{DSTROOT} || "";

# Build product directory inside DSTROOT and OBJROOT.
# Each test config gets its own build directory inside this.
my $BUILDDIR;

# Local top-level directory.
# This is the default value for $BUILDDIR.
my $LOCALBASE = "/tmp/test-$TESTLIBNAMES[0]-build";

# Device-side top-level directory.
# This replaces $DSTROOT$BUILDDIR/ for on-device execution.
my $REMOTEBASE = "/AppleInternal/objctest";

# BATS top-level directory.
# This replaces $DSTROOT$BUILDDIR/ for BATS execution.
my $BATSBASE = "/AppleInternal/CoreOS/tests/objc4";


my $crashcatch = <<'END';
// interpose-able code to catch crashes, print, and exit cleanly
#include <signal.h>
#include <string.h>
#include <unistd.h>

// from dyld-interposing.h
#define DYLD_INTERPOSE(_replacement,_replacee) __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

static void catchcrash(int sig) 
{
    const char *msg;
    switch (sig) {
    case SIGILL:  msg = "CRASHED: SIGILL";  break;
    case SIGBUS:  msg = "CRASHED: SIGBUS";  break;
    case SIGSYS:  msg = "CRASHED: SIGSYS";  break;
    case SIGSEGV: msg = "CRASHED: SIGSEGV"; break;
    case SIGTRAP: msg = "CRASHED: SIGTRAP"; break;
    case SIGABRT: msg = "CRASHED: SIGABRT"; break;
    default: msg = "unknown signal"; break;
    }
    write(STDERR_FILENO, msg, strlen(msg));

    // avoid backslash-n newline due to escaping differences somewhere
    // in BATS versus local execution (perhaps different perl versions?)
    char newline = 0xa;
    write(STDERR_FILENO, &newline, 1);

    _exit(1);
}

static void setupcrash(void) __attribute__((constructor));
static void setupcrash(void) 
{
    signal(SIGILL, &catchcrash);
    signal(SIGBUS, &catchcrash);
    signal(SIGSYS, &catchcrash);
    signal(SIGSEGV, &catchcrash);
    signal(SIGTRAP, &catchcrash);
    signal(SIGABRT, &catchcrash);
}


static int hacked = 0;
ssize_t hacked_write(int fildes, const void *buf, size_t nbyte)
{
    if (!hacked) {
        setupcrash();
        hacked = 1;
    }
    return write(fildes, buf, nbyte);
}

DYLD_INTERPOSE(hacked_write, write);

END


#########################################################################
## Harness


# map language to buildable extensions for that language
my %extensions_for_language = (
    "c"     => ["c"],     
    "objective-c" => ["c", "m"], 
    "c++" => ["c", "cc", "cp", "cpp", "cxx", "c++"], 
    "objective-c++" => ["c", "m", "cc", "cp", "cpp", "cxx", "c++", "mm"], 
    "swift" => ["swift"], 

    "any" => ["c", "m", "cc", "cp", "cpp", "cxx", "c++", "mm", "swift"], 
    );

# map extension to languages
my %languages_for_extension = (
    "c" => ["c", "objective-c", "c++", "objective-c++"], 
    "m" => ["objective-c", "objective-c++"], 
    "mm" => ["objective-c++"], 
    "cc" => ["c++", "objective-c++"], 
    "cp" => ["c++", "objective-c++"], 
    "cpp" => ["c++", "objective-c++"], 
    "cxx" => ["c++", "objective-c++"], 
    "c++" => ["c++", "objective-c++"], 
    "swift" => ["swift"], 
    );

# Run some newline-separated commands like `make` would, stopping if any fail
# run("cmd1 \n cmd2 \n cmd3")
sub make {
    my $output = "";
    my @cmds = split("\n", $_[0]);
    die if scalar(@cmds) == 0;
    $? = 0;
    foreach my $cmd (@cmds) {
        chomp $cmd;
        next if $cmd =~ /^\s*$/;
        $cmd .= " 2>&1";
        print "$cmd\n" if $VERBOSE;
        eval {
            local $SIG{ALRM} = sub { die "alarm\n" };
            # Timeout after 600 seconds so a deadlocked test doesn't wedge the
            # entire test suite. Increase to an hour for B&I builds.
            if (exists $ENV{"RC_XBS"}) {
                alarm 3600;
            } else {
                alarm 600;
            }
            $output .= `$cmd`;
            alarm 0;
        };
        if ($@) {
            die unless $@ eq "alarm\n";
            $output .= "\nTIMED OUT";
        }
        last if $?;
    }
    print "$output\n" if $VERBOSE;
    return $output;
}

sub chdir_verbose {
    my $dir = shift || die;
    print "cd $dir\n" if $VERBOSE;
    chdir $dir || die "couldn't cd $dir";
}

sub rm_rf_verbose {
    my $dir = shift || die;
    print "mkdir -p $dir\n" if $VERBOSE;
    `rm -rf '$dir'`;
    die "couldn't rm -rf $dir" if $?;
}

sub mkdir_verbose {
    my $dir = shift || die;
    print "mkdir -p $dir\n" if $VERBOSE;
    `mkdir -p '$dir'`;
    die "couldn't mkdir $dir" if $?;
}


# xterm colors
my $red = "\e[41;37m";
my $yellow = "\e[43;30m";
my $nocolor = "\e[0m";
if (! -t STDIN) {
    # Not isatty. Don't use colors.
    $red = "";
    $yellow = "";
    $nocolor = "";
}

# print text with a colored prefix on each line
# fixme some callers pass an array of lines and some don't
sub colorprefix {
    my $color = shift;
    while (defined(my $lines = shift)) {
        $lines = "\n" if ($lines eq "");
        for my $line (split(/^/, $lines)) {
            chomp $line;
            print "$color $nocolor$line\n";
        }
    }
}

# print text colored
# fixme some callers pass an array of lines and some don't
sub colorprint {
    my $color = shift;
    while (defined(my $lines = shift)) {
        $lines = "\n" if ($lines eq "");
        for my $line (split(/^/, $lines)) {
            chomp $line;
            print "$color$line$nocolor\n";
        }
    }
}

# Return test names from the command line.
# Returns all tests if no tests were named.
sub gettests {
    my @tests;

    foreach my $arg (@ARGV) {
        push @tests, $arg  if ($arg !~ /=/  &&  $arg !~ /^-/);
    }

    opendir(my $dir, $DIR) || die;
    while (my $file = readdir($dir)) {
        my ($name, $ext) = ($file =~ /^([^.]+)\.([^.]+)$/);
        next if ! $languages_for_extension{$ext};

        open(my $in, "< $file") || die "$file";
        my $contents = join "", <$in>;
        if (defined $ALL_TESTS{$name}) {
            colorprint $yellow, "SKIP: multiple tests named '$name'; skipping file '$file'.";
        } else {
            $ALL_TESTS{$name} = $ext  if ($contents =~ m#^[/*\s]*TEST_#m);
        }
        close($in);
    }
    closedir($dir);

    if (scalar(@tests) == 0) {
        @tests = keys %ALL_TESTS;
    }

    @tests = sort @tests;

    return @tests;
}


# Turn a C compiler name into a C++ compiler name.
sub cplusplus {
    my ($c) = @_;
    if ($c =~ /cc/) {
        $c =~ s/cc/\+\+/;
        return $c;
    }
    return $c . "++";                         # e.g. clang => clang++
}

# Turn a C compiler name into a Swift compiler name
sub swift {
    my ($c) = @_;
    $c =~ s#[^/]*$#swift#;
    return $c;
}

# Returns an array of all sdks from `xcodebuild -showsdks`
my @sdks_memo;
sub getsdks {
    if (!@sdks_memo) {
        @sdks_memo = (`xcodebuild -showsdks` =~ /-sdk (.+)$/mg);
    }
    return @sdks_memo;
}

my %sdk_path_memo = {};
sub getsdkpath {
    my ($sdk) = @_;
    if (!defined $sdk_path_memo{$sdk}) {
        ($sdk_path_memo{$sdk}) = (`xcodebuild -version -sdk '$sdk' Path` =~ /^\s*(.+?)\s*$/);
    }
    return $sdk_path_memo{$sdk};
}

# Extract a version number from a string.
# Ignore trailing "internal".
sub versionsuffix {
    my ($str) = @_;
    my ($vers) = ($str =~ /([0-9]+\.[0-9]+)(?:\.?internal)?$/);
    return $vers;
}
sub majorversionsuffix {
    my ($str) = @_;
    my ($vers) = ($str =~ /([0-9]+)\.[0-9]+(?:\.?internal)?$/);
    return $vers;
}
sub minorversionsuffix {
    my ($str) = @_;
    my ($vers) = ($str =~ /[0-9]+\.([0-9]+)(?:\.?internal)?$/);
    return $vers;
}

# Compares two SDK names and returns the newer one.
# Assumes the two SDKs are the same OS.
sub newersdk {
    my ($lhs, $rhs) = @_;

    # Major version wins.
    my $lhsMajor = majorversionsuffix($lhs);
    my $rhsMajor = majorversionsuffix($rhs);
    if ($lhsMajor > $rhsMajor) { return $lhs; }
    if ($lhsMajor < $rhsMajor) { return $rhs; }

    # Minor version wins.
    my $lhsMinor = minorversionsuffix($lhs);
    my $rhsMinor = minorversionsuffix($rhs);
    if ($lhsMinor > $rhsMinor) { return $lhs; }
    if ($lhsMinor < $rhsMinor) { return $rhs; }

    # Lexically-last wins (i.e. internal is better than not internal)
    if ($lhs gt $rhs) { return $lhs; }
    return $rhs;
}

sub rewind {
    seek($_[0], 0, 0);
}

# parse name=value,value pairs
sub readconditions {
    my ($conditionstring) = @_;

    my %results;
    my @conditions = ($conditionstring =~ /\w+=(?:[^\s,]+,?)+/g);
    for my $condition (@conditions) {
        my ($name, $values) = ($condition =~ /(\w+)=(.+)/);
        $results{$name} = [split ',', $values];
    }

    return %results;
}

sub check_output {
    my %C = %{shift()};
    my $name = shift;
    my @output = @_;

    my %T = %{$C{"TEST_$name"}};

    # Quietly strip MallocScribble before saving the "original" output 
    # because it is distracting.
    filter_malloc(\@output);

    my @original_output = @output;

    # Run result-checking passes, reducing @output each time
    my $xit = 1;
    my $bad = "";
    my $warn = "";
    my $runerror = $T{TEST_RUN_OUTPUT};
    filter_hax(\@output);
    filter_verbose(\@output);
    filter_simulator(\@output);
    $warn = filter_warn(\@output);
    $bad |= filter_guardmalloc(\@output) if ($C{GUARDMALLOC});
    $bad |= filter_valgrind(\@output) if ($C{VALGRIND});
    $bad = filter_expected(\@output, \%C, $name) if ($bad eq "");
    $bad = filter_bad(\@output)  if ($bad eq "");

    # OK line should be the only one left
    $bad = "(output not 'OK: $name')" if ($bad eq ""  &&  (scalar(@output) != 1  ||  $output[0] !~ /^OK: $name/));
    
    if ($bad ne "") {
        colorprint  $red, "FAIL: /// test '$name' \\\\\\";
        colorprefix $red, @original_output;
        colorprint  $red, "FAIL: \\\\\\ test '$name' ///";
        colorprint  $red, "FAIL: $name: $bad";
        $xit = 0;
    } 
    elsif ($warn ne "") {
        colorprint  $yellow, "PASS: /// test '$name' \\\\\\";
        colorprefix $yellow, @original_output;
        colorprint  $yellow, "PASS: \\\\\\ test '$name' ///";
        print "PASS: $name (with warnings)\n";
    }
    else {
        print "PASS: $name\n";
    }
    return $xit;
}

sub filter_expected
{
    my $outputref = shift;
    my %C = %{shift()};
    my $name = shift;

    my %T = %{$C{"TEST_$name"}};
    my $runerror = $T{TEST_RUN_OUTPUT}  ||  return "";

    my $bad = "";

    my $output = join("\n", @$outputref) . "\n";
    if ($output !~ /$runerror/) {
	$bad = "(run output does not match TEST_RUN_OUTPUT)";
	@$outputref = ("FAIL: $name");
    } else {
	@$outputref = ("OK: $name");  # pacify later filter
    }

    return $bad;
}

sub filter_bad
{
    my $outputref = shift;
    my $bad = "";

    my @new_output;
    for my $line (@$outputref) {
	if ($line =~ /^BAD: (.*)/) {
	    $bad = "(failed)";
	} else {
	    push @new_output, $line;
	}
    }

    @$outputref = @new_output;
    return $bad;
}

sub filter_warn
{
    my $outputref = shift;
    my $warn = "";

    my @new_output;
    for my $line (@$outputref) {
	if ($line !~ /^WARN: (.*)/) {
	    push @new_output, $line;
        } else {
	    $warn = "(warned)";
	}
    }

    @$outputref = @new_output;
    return $warn;
}

sub filter_verbose
{
    my $outputref = shift;

    my @new_output;
    for my $line (@$outputref) {
	if ($line !~ /^VERBOSE: (.*)/) {
	    push @new_output, $line;
	}
    }

    @$outputref = @new_output;
}

sub filter_simulator
{
    my $outputref = shift;

    my @new_output;
    for my $line (@$outputref) {
	if (($line !~ /No simulator devices appear to be running/)  &&  
            ($line !~ /CoreSimulator is attempting to unload a stale CoreSimulatorService job/)  &&  
            ($line !~ /Failed to locate a valid instance of CoreSimulatorService/))
        {
	    push @new_output, $line;
	}
    }

    @$outputref = @new_output;
}

sub filter_hax
{
    my $outputref = shift;

    my @new_output;
    for my $line (@$outputref) {
	if ($line !~ /Class OS_tcp_/) {
	    push @new_output, $line;
	}
    }

    @$outputref = @new_output;
}

sub filter_valgrind
{
    my $outputref = shift;
    my $errors = 0;
    my $leaks = 0;

    my @new_output;
    for my $line (@$outputref) {
	if ($line =~ /^Approx: do_origins_Dirty\([RW]\): missed \d bytes$/) {
	    # --track-origins warning (harmless)
	    next;
	}
	if ($line =~ /^UNKNOWN __disable_threadsignal is unsupported. This warning will not be repeated.$/) {
	    # signals unsupported (harmless)
	    next;
	}
	if ($line =~ /^UNKNOWN __pthread_sigmask is unsupported. This warning will not be repeated.$/) {
	    # signals unsupported (harmless)
	    next;
	}
	if ($line !~ /^^\.*==\d+==/) {
	    # not valgrind output
	    push @new_output, $line;
	    next;
	}

	my ($errcount) = ($line =~ /==\d+== ERROR SUMMARY: (\d+) errors/);
	if (defined $errcount  &&  $errcount > 0) {
	    $errors = 1;
	}

	(my $leakcount) = ($line =~ /==\d+==\s+(?:definitely|possibly) lost:\s+([0-9,]+)/);
	if (defined $leakcount  &&  $leakcount > 0) {
	    $leaks = 1;
	}
    }

    @$outputref = @new_output;

    my $bad = "";
    $bad .= "(valgrind errors)" if ($errors);
    $bad .= "(valgrind leaks)" if ($leaks);
    return $bad;
}



sub filter_malloc
{
    my $outputref = shift;
    my $errors = 0;

    my @new_output;
    my $count = 0;
    for my $line (@$outputref) {
        # Ignore MallocScribble prologue.
        # Ignore MallocStackLogging prologue.
        if ($line =~ /malloc: enabling scribbling to detect mods to free/  ||  
            $line =~ /Deleted objects will be dirtied by the collector/  ||
            $line =~ /malloc: stack logs being written into/  ||  
            $line =~ /malloc: stack logs deleted from/  ||  
            $line =~ /malloc: process \d+ no longer exists/  ||  
            $line =~ /malloc: recording malloc and VM allocation stacks/)
        {
            next;
	}

        # not malloc output
        push @new_output, $line;

    }

    @$outputref = @new_output;
}

sub filter_guardmalloc
{
    my $outputref = shift;
    my $errors = 0;

    my @new_output;
    my $count = 0;
    for my $line (@$outputref) {
	if ($line !~ /^GuardMalloc\[[^\]]+\]: /) {
	    # not guardmalloc output
	    push @new_output, $line;
	    next;
	}

        # Ignore 4 lines of guardmalloc prologue.
        # Anything further is a guardmalloc error.
        if (++$count > 4) {
            $errors = 1;
        }
    }

    @$outputref = @new_output;

    my $bad = "";
    $bad .= "(guardmalloc errors)" if ($errors);
    return $bad;
}

# TEST_SOMETHING
# text
# text
# END
sub extract_multiline {
    my ($flag, $contents, $name) = @_;
    if ($contents =~ /$flag\n/) {
        my ($output) = ($contents =~ /$flag\n(.*?\n)END[ *\/]*\n/s);
        die "$name used $flag without END\n"  if !defined($output);
        return $output;
    }
    return undef;
}


# TEST_SOMETHING
# text
# OR
# text
# END
sub extract_multiple_multiline {
    my ($flag, $contents, $name) = @_;
    if ($contents =~ /$flag\n/) {
        my ($output) = ($contents =~ /$flag\n(.*?\n)END[ *\/]*\n/s);
        die "$name used $flag without END\n"  if !defined($output);

        $output =~ s/\nOR\n/\n|/sg;
        $output = "^(" . $output . ")\$";
        return $output;
    }
    return undef;
}


sub gather_simple {
    my $CREF = shift;
    my %C = %{$CREF};
    my $name = shift;
    chdir_verbose $DIR;

    my $ext = $ALL_TESTS{$name};
    my $file = "$name.$ext";
    return 0 if !$file;

    # search file for 'TEST_CONFIG' or '#include "test.h"'
    # also collect other values:
    # TEST_DISABLED disable test with an optional message
    # TEST_CRASHES test is expected to crash
    # TEST_CONFIG test conditions
    # TEST_ENV environment prefix
    # TEST_CFLAGS compile flags
    # TEST_BUILD build instructions
    # TEST_BUILD_OUTPUT expected build stdout/stderr
    # TEST_RUN_OUTPUT expected run stdout/stderr
    open(my $in, "< $file") || die;
    my $contents = join "", <$in>;
    
    my $test_h = ($contents =~ /^\s*#\s*(include|import)\s*"test\.h"/m);
    my ($disabled) = ($contents =~ /\b(TEST_DISABLED\b.*)$/m);
    my $crashes = ($contents =~ /\bTEST_CRASHES\b/m);
    my ($conditionstring) = ($contents =~ /\bTEST_CONFIG\b(.*)$/m);
    my ($envstring) = ($contents =~ /\bTEST_ENV\b(.*)$/m);
    my ($cflags) = ($contents =~ /\bTEST_CFLAGS\b(.*)$/m);
    my ($buildcmd) = extract_multiline("TEST_BUILD", $contents, $name);
    my ($builderror) = extract_multiple_multiline("TEST_BUILD_OUTPUT", $contents, $name);
    my ($runerror) = extract_multiple_multiline("TEST_RUN_OUTPUT", $contents, $name);

    return 0 if !$test_h && !$disabled && !$crashes && !defined($conditionstring) && !defined($envstring) && !defined($cflags) && !defined($buildcmd) && !defined($builderror) && !defined($runerror);

    if ($disabled) {
        colorprint $yellow, "SKIP: $name    (disabled by $disabled)";
        return 0;
    }

    # check test conditions

    my $run = 1;
    my %conditions = readconditions($conditionstring);
    if (! $conditions{LANGUAGE}) {
        # implicit language restriction from file extension
        $conditions{LANGUAGE} = $languages_for_extension{$ext};
    }
    for my $condkey (keys %conditions) {
        my @condvalues = @{$conditions{$condkey}};

        # special case: RUN=0 does not affect build
        if ($condkey eq "RUN"  &&  @condvalues == 1  &&  $condvalues[0] == 0) {
            $run = 0;
            next;
        }

        my $testvalue = $C{$condkey};
        next if !defined($testvalue);
        # testvalue is the configuration being run now
        # condvalues are the allowed values for this test
        
        my $ok = 0;
        for my $condvalue (@condvalues) {

            # special case: objc and objc++
            if ($condkey eq "LANGUAGE") {
                $condvalue = "objective-c" if $condvalue eq "objc";
                $condvalue = "objective-c++" if $condvalue eq "objc++";
            }

            $ok = 1  if ($testvalue eq $condvalue);

            # special case: CC and CXX allow substring matches
            if ($condkey eq "CC"  ||  $condkey eq "CXX") {
                $ok = 1  if ($testvalue =~ /$condvalue/);
            }

            last if $ok;
        }

        if (!$ok) {
            my $plural = (@condvalues > 1) ? "one of: " : "";
            print "SKIP: $name    ($condkey=$testvalue, but test requires $plural", join(' ', @condvalues), ")\n";
            return 0;
        }
    }

    # save some results for build and run phases
    $$CREF{"TEST_$name"} = {
        TEST_BUILD => $buildcmd, 
        TEST_BUILD_OUTPUT => $builderror, 
        TEST_CRASHES => $crashes, 
        TEST_RUN_OUTPUT => $runerror, 
        TEST_CFLAGS => $cflags,
        TEST_ENV => $envstring,
        TEST_RUN => $run,
        DSTDIR => "$C{DSTDIR}/$name.build",
        OBJDIR => "$C{OBJDIR}/$name.build",
    };

    return 1;
}


# Test description plist to write when building for BATS execution.
my %bats_plist;
$bats_plist{'Project'} = "objc4";
$bats_plist{'Tests'} = [];  # populated by append_bats_test()

# Saves run instructions for a single test in all configurations as a BATS test.
sub append_bats_test {
    my $name = shift;

    my $arch = join(',', @{$args{ARCH}});
    my $os = join(',', @{$args{OSVERSION}});
    my $mem = join(',', @{$args{MEM}});
    my $language = join(',', @{$args{LANGUAGE}});

    push @{$bats_plist{'Tests'}}, {
        "TestName" => "$name",
        "Command" => [
            "/usr/bin/perl",
            "$BATSBASE/test/test.pl",
            $name,
            "ARCH=$arch",
            "OS=$os",
            "MEM=$mem",
            "LANGUAGE=$language",
            "BUILD=0",
            "RUN=1",
            "VERBOSE=1",
            "BATS=1",
        ]
    };
}


# Builds a simple test
sub build_simple {
    my %C = %{shift()};
    my $name = shift;
    my %T = %{$C{"TEST_$name"}};

    mkdir_verbose $T{DSTDIR};
    chdir_verbose $T{DSTDIR};
    # we don't mkdir $T{OBJDIR} because most tests don't use it

    my $ext = $ALL_TESTS{$name};
    my $file = "$DIR/$name.$ext";

    if ($T{TEST_CRASHES}) {
        `echo '$crashcatch' > crashcatch.c`;
        make("$C{COMPILE_C} -dynamiclib -o libcrashcatch.dylib -x c crashcatch.c");
        die "$?" if $?;
    }

    my $cmd = $T{TEST_BUILD} ? eval "return \"$T{TEST_BUILD}\"" : "$C{COMPILE}   $T{TEST_CFLAGS} $file -o $name.exe";

    my $output = make($cmd);

    # ignore out-of-date text-based stubs (caused by ditto into SDK)
    $output =~ s/ld: warning: text-based stub file.*\n//g;
    # rdar://10163155
    $output =~ s/ld: warning: could not create compact unwind for [^\n]+: does not use standard frame\n//g;
    # rdar://37937122
    $output =~ s/^warning: Cannot lower [^\n]+\n//g;
    $output =~ s/^warning:     key: [^\n]+\n//g;
    $output =~ s/^warning:     discriminator: [^\n]+\n//g;
    $output =~ s/^warning:     callee: [^\n]+\n//g;
    # rdar://38710948
    $output =~ s/ld: warning: ignoring file [^\n]*libclang_rt\.bridgeos\.a[^\n]*\n//g;
    # ignore compiler logging of CCC_OVERRIDE_OPTIONS effects
    if (defined $ENV{CCC_OVERRIDE_OPTIONS}) {
        $output =~ s/### (CCC_OVERRIDE_OPTIONS:|Adding argument|Deleting argument|Replacing) [^\n]*\n//g;
    }

    my $ok;
    if (my $builderror = $T{TEST_BUILD_OUTPUT}) {
        # check for expected output and ignore $?
        if ($output =~ /$builderror/) {
            $ok = 1;
        } elsif (defined $ENV{CCC_OVERRIDE_OPTIONS} && $builderror =~ /warning:/) {
            # CCC_OVERRIDE_OPTIONS manipulates compiler diagnostics.
            # Don't enforce any TEST_BUILD_OUTPUT that looks for warnings.
            colorprint  $yellow, "WARN: /// test '$name' \\\\\\";
            colorprefix $yellow, $output;
            colorprint  $yellow, "WARN: \\\\\\ test '$name' ///";
            colorprint  $yellow, "WARN: $name (build output does not match TEST_BUILD_OUTPUT; not fatal because CCC_OVERRIDE_OPTIONS is set)";
            $ok = 1;
        } else {
            colorprint  $red, "FAIL: /// test '$name' \\\\\\";
            colorprefix $red, $output;
            colorprint  $red, "FAIL: \\\\\\ test '$name' ///";
            colorprint  $red, "FAIL: $name (build output does not match TEST_BUILD_OUTPUT)";
            $ok = 0;
        }
    } elsif ($?) {
        colorprint  $red, "FAIL: /// test '$name' \\\\\\";
        colorprefix $red, $output;
        colorprint  $red, "FAIL: \\\\\\ test '$name' ///";
        colorprint  $red, "FAIL: $name (build failed)";
        $ok = 0;
    } elsif ($output ne "") {
        colorprint  $red, "FAIL: /// test '$name' \\\\\\";
        colorprefix $red, $output;
        colorprint  $red, "FAIL: \\\\\\ test '$name' ///";
        colorprint  $red, "FAIL: $name (unexpected build output)";
        $ok = 0;
    } else {
        $ok = 1;
    }

    if ($ok) {
        foreach my $file (glob("*.exe *.dylib *.bundle")) {
            if (!$BATS) {
                # not for BATS to save space and build time
                # fixme use SYMROOT?
                make("xcrun dsymutil $file");
            }
            if ($C{OS} eq "macosx"  ||  $C{OS} =~ /simulator/) {
                # setting any entitlements disables dyld environment variables
            } else {
                # get-task-allow entitlement is required
                # to enable dyld environment variables
                make("xcrun codesign -s - --entitlements $DIR/get_task_allow_entitlement.plist $file");
                die "$?" if $?;
            }
        }
    }

    return $ok;
}

# Run a simple test (testname.exe, with error checking of stdout and stderr)
sub run_simple {
    my %C = %{shift()};
    my $name = shift;
    my %T = %{$C{"TEST_$name"}};

    if (! $T{TEST_RUN}) {
        print "PASS: $name (build only)\n";
        return 1;
    }

    my $testdir = $T{DSTDIR};
    chdir_verbose $testdir;

    my $env = "$C{ENV} $T{TEST_ENV}";

    if ($T{TEST_CRASHES}) {
        $env .= " OBJC_DEBUG_DONT_CRASH=YES";
    }

    if ($C{DYLD} eq "2") {
        $env .= " DYLD_USE_CLOSURES=0";
    }
    elsif ($C{DYLD} eq "3") {
        $env .= " DYLD_USE_CLOSURES=1";
    }
    else {
        die "unknown DYLD setting $C{DYLD}";
    }

    my $output;

    if ($C{ARCH} =~ /^arm/ && `uname -p` !~ /^arm/) {
        # run on iOS or watchos or tvos device
        # fixme device selection and verification
        my $remotedir = "$REMOTEBASE/" . basename($C{DSTDIR}) . "/$name.build";

        # Add test dir and libobjc's dir to DYLD_LIBRARY_PATH.
        # Insert libcrashcatch.dylib if necessary.
        $env .= " DYLD_LIBRARY_PATH=$remotedir";
        $env .= ":$REMOTEBASE"  if ($C{TESTLIBDIR} ne $TESTLIBDIR);
        if ($T{TEST_CRASHES}) {
            $env .= " DYLD_INSERT_LIBRARIES=$remotedir/libcrashcatch.dylib";
        }

        my $cmd = "ssh -p $PORT $HOST 'cd $remotedir && env $env ./$name.exe'";
        $output = make("$cmd");
    }
    elsif ($C{OS} =~ /simulator/) {
        # run locally in a simulator
        # fixme selection of simulated OS version
        my $simdevice;
        if ($C{OS} =~ /iphonesimulator/) {
            $simdevice = 'iPhone X';
        } elsif ($C{OS} =~ /watchsimulator/) {
            $simdevice = 'Apple Watch Series 4 - 40mm';
        } elsif ($C{OS} =~ /tvsimulator/) {
            $simdevice = 'Apple TV 1080p';
        } else {
            die "unknown simulator $C{OS}\n";
        }
        my $sim = "xcrun -sdk iphonesimulator simctl spawn '$simdevice'";
        # Add test dir and libobjc's dir to DYLD_LIBRARY_PATH.
        # Insert libcrashcatch.dylib if necessary.
        $env .= " DYLD_LIBRARY_PATH=$testdir";
        $env .= ":" . $C{TESTLIBDIR}  if ($C{TESTLIBDIR} ne $TESTLIBDIR);
        if ($T{TEST_CRASHES}) {
            $env .= " DYLD_INSERT_LIBRARIES=$testdir/libcrashcatch.dylib";
        }

        my $simenv = "";
        foreach my $keyvalue (split(' ', $env)) {
            $simenv .= "SIMCTL_CHILD_$keyvalue ";
        }
        # Use the full path here so hack_cwd in test.h works.
        $output = make("env $simenv $sim $testdir/$name.exe");
    }
    else {
        # run locally

        # Add test dir and libobjc's dir to DYLD_LIBRARY_PATH.
        # Insert libcrashcatch.dylib if necessary.
        $env .= " DYLD_LIBRARY_PATH=$testdir";
        $env .= ":" . $C{TESTLIBDIR}  if ($C{TESTLIBDIR} ne $TESTLIBDIR);
        if ($T{TEST_CRASHES}) {
            $env .= " DYLD_INSERT_LIBRARIES=$testdir/libcrashcatch.dylib";
        }

        $output = make("sh -c '$env ./$name.exe'");
    }

    return check_output(\%C, $name, split("\n", $output));
}


my %compiler_memo;
sub find_compiler {
    my ($cc, $toolchain, $sdk_path) = @_;

    # memoize
    my $key = $cc . ':' . $toolchain;
    my $result = $compiler_memo{$key};
    return $result if defined $result;
    
    $result  = make("xcrun -toolchain $toolchain -find $cc 2>/dev/null");

    chomp $result;
    $compiler_memo{$key} = $result;
    return $result;
}

sub dirContainsAllTestLibs {
    my $dir = shift;

    foreach my $testlib (@TESTLIBNAMES) {
        my $found = (-e "$dir/$testlib");
        my $foundstr = ($found ? "found" : "didn't find");
        print "note: $foundstr $testlib in $dir\n"  if ($VERBOSE);
        return 0  if (!$found);
    }

    return 1;
}

sub findIncludeDir {
    my ($root, $includePath) = @_;

    foreach my $candidate ("$root/../SDKContentRoot/$includePath", "$root/$includePath") {
        my $found = -e $candidate;
        my $foundstr = ($found ? "found" : "didn't find");
        print "note:	$foundstr $includePath at $candidate\n" if $VERBOSE;
        return $candidate if $found;
    }

    die "Unable to find $includePath in $root.\n";
}

sub buildSharedCache {
    my $Cref = shift;
    my %C = %$Cref;
    
    make("update_dyld_shared_cache -verbose -cache_dir $BUILDDIR -overlay $C{TESTLIBDIR}/../..");
}

sub make_one_config {
    my $configref = shift;
    my $root = shift;
    my %C = %{$configref};

    # Aliases
    $C{LANGUAGE} = "objective-c"  if $C{LANGUAGE} eq "objc";
    $C{LANGUAGE} = "objective-c++"  if $C{LANGUAGE} eq "objc++";
    
    # Interpret OS version string from command line.
    my ($sdk_arg, $deployment_arg, $run_arg, undef) = split('-', $C{OSVERSION});
    delete $C{OSVERSION};
    my ($os_arg) = ($sdk_arg =~ /^([^\.0-9]+)/);
    $deployment_arg = "default" if !defined($deployment_arg);
    $run_arg = "default" if !defined($run_arg);

    my %allowed_os_args = (
        "macosx" => "macosx", "osx" => "macosx", "macos" => "macosx",
        "iphoneos" => "iphoneos", "ios" => "iphoneos",
        "iphonesimulator" => "iphonesimulator", "iossimulator" => "iphonesimulator",
        "watchos" => "watchos",
        "watchsimulator" => "watchsimulator", "watchossimulator" => "watchsimulator",
        "appletvos" => "appletvos", "tvos" => "appletvos",
        "appletvsimulator" => "appletvsimulator", "tvsimulator" => "appletvsimulator",
        "bridgeos" => "bridgeos",
        );

    $C{OS} = $allowed_os_args{$os_arg} || die "unknown OS '$os_arg' (expected " . join(', ', sort keys %allowed_os_args) . ")\n";

    # set the config name now, after massaging the language and OS versions, 
    # but before adding other settings
    my $configname = config_name(%C);
    die if ($configname =~ /'/);
    die if ($configname =~ / /);
    ($C{NAME} = $configname) =~ s/~/ /g;
    (my $configdir = $configname) =~ s#/##g;
    $C{DSTDIR} = "$DSTROOT$BUILDDIR/$configdir";
    $C{OBJDIR} = "$OBJROOT$BUILDDIR/$configdir";

    # Allow tests to see BATS-edness in TEST_CONFIG.
    $C{BATS} = $BATS;

    if ($C{OS} eq "iphoneos" || $C{OS} eq "iphonesimulator") {
        $C{TOOLCHAIN} = "ios";
    } elsif ($C{OS} eq "watchos" || $C{OS} eq "watchsimulator") {
        $C{TOOLCHAIN} = "watchos";
    } elsif ($C{OS} eq "appletvos" || $C{OS} eq "appletvsimulator") {
        $C{TOOLCHAIN} = "appletvos";
    } elsif ($C{OS} eq "bridgeos") {
        $C{TOOLCHAIN} = "bridgeos";
    } elsif ($C{OS} eq "macosx") {
        $C{TOOLCHAIN} = "osx";
    } else {
        colorprint $yellow, "WARN: don't know toolchain for OS $C{OS}";
        $C{TOOLCHAIN} = "default";
    }

    if ($BUILD) {
        # Look up SDK.
        # Try exact match first.
        # Then try lexically-last prefix match 
        #   (so "macosx" => "macosx10.7internal")

        $sdk_arg =~ s/$os_arg/$C{OS}/;

        my @sdks = getsdks();
        if ($VERBOSE) {
            print "note: Installed SDKs: @sdks\n";
        }
        my $exactsdk = undef;
        my $prefixsdk = undef;
        foreach my $sdk (@sdks) {
            $exactsdk = $sdk  if ($sdk eq $sdk_arg);
            $prefixsdk = newersdk($sdk, $prefixsdk)  if ($sdk =~ /^$sdk_arg/);
        }

        my $sdk;
        if ($exactsdk) {
            $sdk = $exactsdk;
        } elsif ($prefixsdk) {
            $sdk = $prefixsdk;
        } else {
            die "unknown SDK '$sdk_arg'\nInstalled SDKs: @sdks\n";
        }

        # Set deployment target.
        # fixme can't enforce version when run_arg eq "default" 
        # because we don't know it yet
        $deployment_arg = versionsuffix($sdk) if $deployment_arg eq "default";
        if ($run_arg ne "default") {
            die "Deployment target '$deployment_arg' is newer than run target '$run_arg'\n"  if $deployment_arg > $run_arg;
        }
        $C{DEPLOYMENT_TARGET} = $deployment_arg;
        $C{SDK_PATH} = getsdkpath($sdk);
    } else {
        # not $BUILD
        $C{DEPLOYMENT_TARGET} = "unknown_deployment_target";
        $C{SDK_PATH} = "/unknown/sdk";
    }

    # Set run target.
    $C{RUN_TARGET} = $run_arg;

    # Look up test library (possible in root or SDK_PATH)
    
    my $rootarg = $root;
    my $symroot;
    my @sympaths = ( (glob "$root/*~sym")[0], 
                     (glob "$root/BuildRecords/*_install/Symbols")[0], 
                     "$root/Symbols" );
    my @dstpaths = ( (glob "$root/*~dst")[0], 
                     (glob "$root/BuildRecords/*_install/Root")[0], 
                     "$root/Root" );
    for(my $i = 0; $i < scalar(@sympaths); $i++) {
        if (-e $sympaths[$i]  &&  -e $dstpaths[$i]) {
            $symroot = $sympaths[$i];
            $root = $dstpaths[$i];
            last;
        }
    }

    if ($root ne "") {
        # Root specified. Require that it contain our dylibs.
        if (dirContainsAllTestLibs("$root$C{SDK_PATH}$TESTLIBDIR")) {
            $C{TESTLIBDIR} = "$root$C{SDK_PATH}$TESTLIBDIR";
        } elsif (dirContainsAllTestLibs("$root$TESTLIBDIR")) {
            $C{TESTLIBDIR} = "$root$TESTLIBDIR";
        } elsif (dirContainsAllTestLibs($root)) {
            $C{TESTLIBDIR} = "$root";
        } else {
            die "Didn't find some libs in root '$rootarg' for sdk '$C{SDK_PATH}'\n";
        }
    }
    else {
        # No root specified. Use the SDK or / for our dylibs.
        if (dirContainsAllTestLibs("$C{SDK_PATH}$TESTLIBDIR")) {
            $C{TESTLIBDIR} = "$C{SDK_PATH}$TESTLIBDIR";
        } else {
            # We don't actually check in / because on devices
            # there are no dylib files there.
            $C{TESTLIBDIR} = $TESTLIBDIR;
        }
    }

    @{$C{TESTLIBS}} = map { "$C{TESTLIBDIR}/$_" } @TESTLIBNAMES;
    # convenience for tests that want libobjc.dylib's path
    $C{TESTLIB} = @{$C{TESTLIBS}}[0];

    foreach my $testlibname (@TESTLIBNAMES) {
        if (-e "$symroot/$testlibname.dSYM") {
            push(@{$C{TESTDSYMS}}, "$symroot/$testlibname.dSYM");
        }
    }

    if ($VERBOSE) {
        foreach my $testlib (@{$C{TESTLIBS}}) {
            my @uuids = `/usr/bin/dwarfdump -u '$testlib'`;
            while (my $uuid = shift @uuids) {
                print "note: $uuid";
            }
        }
    }

    # Look up compilers
    my $cc = $C{CC};
    my $cxx = cplusplus($C{CC});
    my $swift = swift($C{CC});
    if (! $BUILD) {
        $C{CC} = $cc;
        $C{CXX} = $cxx;
        $C{SWIFT} = $swift
    } else {
        $C{CC} = find_compiler($cc, $C{TOOLCHAIN}, $C{SDK_PATH});
        $C{CXX} = find_compiler($cxx, $C{TOOLCHAIN}, $C{SDK_PATH});
        $C{SWIFT} = find_compiler($swift, $C{TOOLCHAIN}, $C{SDK_PATH});

        die "No C compiler '$cc' ('$C{CC}') in toolchain '$C{TOOLCHAIN}'\n" if !-e $C{CC};
        die "No C++ compiler '$cxx' ('$C{CXX}') in toolchain '$C{TOOLCHAIN}'\n" if !-e $C{CXX};
        die "No Swift compiler '$swift' ('$C{SWIFT}') in toolchain '$C{TOOLCHAIN}'\n" if !-e $C{SWIFT};
    }    

    if ($C{ARCH} eq "i386" && $C{OS} eq "macosx") {
        # libarclite no longer available on i386
        # fixme need an archived copy for bincompat testing
        $C{FORCE_LOAD_ARCLITE} = "";
    } elsif ($C{OS} eq "bridgeos") {
        # no libarclite on bridgeOS
        $C{FORCE_LOAD_ARCLITE} = "";
    } else {
        $C{FORCE_LOAD_ARCLITE} = "-Xlinker -force_load -Xlinker " . dirname($C{CC}) . "/../lib/arc/libarclite_$C{OS}.a";
    }

    # Populate cflags

    my $cflags = "-I$DIR -W -Wall -Wno-objc-weak-compat -Wno-arc-bridge-casts-disallowed-in-nonarc -Wshorten-64-to-32 -Qunused-arguments -fno-caret-diagnostics -Os -arch $C{ARCH} ";
    if (!$BATS) {
        # save-temps so dsymutil works so debug info works.
        # Disabled in BATS to save disk space.
        # rdar://45656803 -save-temps causes bad -Wstdlibcxx-not-found warnings
        $cflags .= "-g -save-temps -Wno-stdlibcxx-not-found";
    }
    my $objcflags = "";
    my $swiftflags = "-g ";
    
    $cflags .= " -isysroot '$C{SDK_PATH}'";
    $cflags .= " '-Wl,-syslibroot,$C{SDK_PATH}'";
    $swiftflags .= " -sdk '$C{SDK_PATH}'";
    
    # Set deployment target cflags
    my $target = undef;
    die "No deployment target" if $C{DEPLOYMENT_TARGET} eq "";
    if ($C{OS} eq "iphoneos") {
        $cflags .= " -mios-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-ios$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "iphonesimulator") {
        $cflags .= " -mios-simulator-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-ios$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "watchos") {
        $cflags .= " -mwatchos-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-watchos$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "watchsimulator") {
        $cflags .= " -mwatchos-simulator-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-watchos$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "appletvos") {
        $cflags .= " -mtvos-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-tvos$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "appletvsimulator") {
        $cflags .= " -mtvos-simulator-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-tvos$C{DEPLOYMENT_TARGET}";
    }
    elsif ($C{OS} eq "bridgeos") {
        $cflags .= " -mbridgeos-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-bridgeos$C{DEPLOYMENT_TARGET}";
    }
    else {
        $cflags .= " -mmacosx-version-min=$C{DEPLOYMENT_TARGET}";
        $target = "$C{ARCH}-apple-macosx$C{DEPLOYMENT_TARGET}";
    }
    $swiftflags .= " -target $target";

    $C{TESTINCLUDEDIR} = "$C{SDK_PATH}/usr/include";
    $C{TESTLOCALINCLUDEDIR} = "$C{SDK_PATH}/usr/local/include";
    if ($root ne "") {
        if ($C{SDK_PATH} ne "/") {
            $cflags .= " -isystem '$root$C{SDK_PATH}/usr/include'";
            $cflags .= " -isystem '$root$C{SDK_PATH}/usr/local/include'";
        }

        my $library_path = $C{TESTLIBDIR};
        $cflags .= " -L$library_path";
        $C{TESTINCLUDEDIR} = findIncludeDir($root, "usr/include");
        $C{TESTLOCALINCLUDEDIR} = findIncludeDir($root, "usr/local/include");
        $cflags .= " -isystem '$C{TESTINCLUDEDIR}'";
        $cflags .= " -isystem '$C{TESTLOCALINCLUDEDIR}'";
    }

    
    # Populate objcflags
    
    $objcflags .= " -lobjc";
    if ($C{MEM} eq "arc") {
        $objcflags .= " -fobjc-arc";
    }
    elsif ($C{MEM} eq "mrc") {
        # nothing
    }
    else {
        die "unrecognized MEM '$C{MEM}'\n";
    }
    
    # Populate ENV_PREFIX
    $C{ENV} = "LANG=C MallocScribble=1";
    $C{ENV} .= " VERBOSE=$VERBOSE"  if $VERBOSE;
    if ($root ne "") {
        die "no spaces allowed in root" if $C{TESTLIBDIR} =~ /\s+/;
    }
    if ($C{GUARDMALLOC}) {
        $C{ENV} .= " GUARDMALLOC=1";  # checked by tests and errcheck.pl
        $C{ENV} .= " DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib";
        if ($C{GUARDMALLOC} eq "before") {
            $C{ENV} .= " MALLOC_PROTECT_BEFORE=1";
        } elsif ($C{GUARDMALLOC} eq "after") {
            # protect after is the default
        } else {
            die "Unknown guard malloc mode '$C{GUARDMALLOC}'\n";
        }
    }

    # Populate compiler commands
    $C{XCRUN} = "env LANG=C /usr/bin/xcrun -toolchain '$C{TOOLCHAIN}'";

    $C{COMPILE_C}   = "$C{XCRUN} '$C{CC}'  $cflags -x c -std=gnu99";
    $C{COMPILE_CXX} = "$C{XCRUN} '$C{CXX}' $cflags -x c++";
    $C{COMPILE_M}   = "$C{XCRUN} '$C{CC}'  $cflags $objcflags -x objective-c -std=gnu99";
    $C{COMPILE_MM}  = "$C{XCRUN} '$C{CXX}' $cflags $objcflags -x objective-c++";
    $C{COMPILE_SWIFT} = "$C{XCRUN} '$C{SWIFT}' $swiftflags";
    
    $C{COMPILE} = $C{COMPILE_C}      if $C{LANGUAGE} eq "c";
    $C{COMPILE} = $C{COMPILE_CXX}    if $C{LANGUAGE} eq "c++";
    $C{COMPILE} = $C{COMPILE_M}      if $C{LANGUAGE} eq "objective-c";
    $C{COMPILE} = $C{COMPILE_MM}     if $C{LANGUAGE} eq "objective-c++";
    $C{COMPILE} = $C{COMPILE_SWIFT}  if $C{LANGUAGE} eq "swift";
    die "unknown language '$C{LANGUAGE}'\n" if !defined $C{COMPILE};

    ($C{COMPILE_NOMEM} = $C{COMPILE}) =~ s/ -fobjc-arc\S*//g;
    ($C{COMPILE_NOLINK} = $C{COMPILE}) =~ s/ '?-(?:Wl,|l)\S*//g;
    ($C{COMPILE_NOLINK_NOMEM} = $C{COMPILE_NOMEM}) =~ s/ '?-(?:Wl,|l)\S*//g;


    # Reject some self-inconsistent and disallowed configurations
    if ($C{MEM} !~ /^(mrc|arc)$/) {
        die "unknown MEM=$C{MEM} (expected one of mrc arc)\n";
    }

    if ($C{MEM} eq "arc"  &&  $C{CC} !~ /clang/) {
        print "note: skipping configuration $C{NAME}\n";
        print "note:   because CC=$C{CC} does not support MEM=$C{MEM}\n";
        return 0;
    }

    if ($C{ARCH} eq "i386"  &&  $C{OS} eq "macosx") {
        colorprint $yellow, "WARN: skipping configuration $C{NAME}\n";
        colorprint $yellow, "WARN:   because 32-bit Mac is dead\n";
        return 0;
    }

    # fixme 
    if ($C{LANGUAGE} eq "swift"  &&  $C{ARCH} =~ /^arm/) {
        print "note: skipping configuration $C{NAME}\n";
        print "note:   because ARCH=$C{ARCH} does not support LANGUAGE=SWIFT\n";
        return 0;
    }

    # fixme unimplemented run targets
    if ($C{RUN_TARGET} ne "default" &&  $C{OS} !~ /simulator/) {
        colorprint $yellow, "WARN: skipping configuration $C{NAME}";
        colorprint $yellow, "WARN:   because OS=$C{OS} does not yet implement RUN_TARGET=$C{RUN_TARGET}";
    }

    %$configref = %C;
}    

sub make_configs {
    my ($root, %args) = @_;

    my @results = ({});  # start with one empty config

    for my $key (keys %args) {
        my @newresults;
        my @values = @{$args{$key}};
        for my $configref (@results) {
            my %config = %{$configref};
            for my $value (@values) {
                my %newconfig = %config;
                $newconfig{$key} = $value;
                push @newresults, \%newconfig;
            }
        }
        @results = @newresults;
    }

    my @newresults;
    for my $configref(@results) {
        if (make_one_config($configref, $root)) {
            push @newresults, $configref;
        }
    }

    return @newresults;
}

sub config_name {
    my %config = @_;
    my $name = "";
    for my $key (sort keys %config) {
        $name .= '~'  if $name ne "";
        $name .= "$key=$config{$key}";
    }
    return $name;
}

sub rsync_ios {
    my ($src, $timeout) = @_;
    for (my $i = 0; $i < 10; $i++) {
        make("$DIR/timeout.pl $timeout rsync -e 'ssh -p $PORT' -av $src $HOST:/$REMOTEBASE/");
        return if $? == 0;
        colorprint $yellow, "WARN: RETRY\n"  if $VERBOSE;
    }
    die "Couldn't rsync tests to device. Check: device is connected; tcprelay is running; device trusts your Mac; device is unlocked; filesystem is mounted r/w\n";
}

sub build_and_run_one_config {
    my %C = %{shift()};
    my @tests = @_;

    # Build and run
    my $testcount = 0;
    my $failcount = 0;
    my $skipconfig = 0;

    my @gathertests;
    foreach my $test (@tests) {
        if ($VERBOSE) {
            print "\nGATHER $test\n";
        }

        if ($ALL_TESTS{$test}) {
            gather_simple(\%C, $test) || next;  # not pass, not fail
            push @gathertests, $test;
        } else {
            die "No test named '$test'\n";
        }
    }

    my @builttests;
    if (!$BUILD) {
        @builttests = @gathertests;
        $testcount = scalar(@gathertests);
    } else {
        foreach my $test (@gathertests) {
            if ($VERBOSE) {
                print "\nBUILD $test\n";
            }
            
            if ($ALL_TESTS{$test}) {
                $testcount++;
                if (!build_simple(\%C, $test)) {
                    $failcount++;
                } else {
                    push @builttests, $test;
                }
            } else {
                die "No test named '$test'\n";
            }
        }
    }
    
    if (!$RUN  ||  !scalar(@builttests)) {
        # nothing to do
    }
    else {
        if ($C{ARCH} =~ /^arm/ && `uname -p` !~ /^arm/) {
            # upload timeout - longer for slow watch devices
            my $timeout = ($C{OS} =~ /watch/) ? 120 : 20;
            
            # upload all tests to iOS device
            rsync_ios($C{DSTDIR}, $timeout);

            # upload library to iOS device
            if ($C{TESTLIBDIR} ne $TESTLIBDIR) {
                foreach my $thing (@{$C{TESTLIBS}}, @{$C{TESTDSYMS}}) {
                    rsync_ios($thing, $timeout);
                }
            }
        }
        elsif ($C{OS} =~ /simulator/) {
            # run locally in a simulator
        }
        else {
            # run locally
            if ($BATS) {
                # BATS execution tries to run architectures that
                # aren't supported by the device. Skip those configs here.
                my $machine = `machine`;
                chomp $machine;
                # unsupported:
                # running arm64e on non-arm64e device
                # running arm64 on non-arm64* device
                # running armv7k on non-armv7k device
                # running arm64_32 on armv7k device
                # We don't need to handle all mismatches here, 
                # only mismatches that arise within a single OS.
                $skipconfig = 
                    (($C{ARCH} eq "arm64e"   && $machine ne "arm64e")  ||
                     ($C{ARCH} eq "arm64"    && $machine !~ /^arm64/)  ||
                     ($C{ARCH} eq "armv7k"   && $machine ne "armv7k")  ||
                     ($C{ARCH} eq "arm64_32" && $machine eq "armv7k"));
                if ($skipconfig) {
                    print "note: skipping configuration $C{NAME}\n";
                    print "note:   because test arch $C{ARCH} is not " .
                        "supported on device arch $machine\n";
                    $testcount = 0;
                }
            }
        }

        if (!$skipconfig) {
            foreach my $test (@builttests) {
                print "\nRUN $test\n"  if ($VERBOSE);

                if ($ALL_TESTS{$test}) {
                    if (!run_simple(\%C, $test)) {
                        $failcount++;
                    }
                } else {
                    die "No test named '$test'\n";
                }
            }
        }
    }
    
    return ($testcount, $failcount, $skipconfig);
}



# Return value if set by "$argname=value" on the command line
# Return $default if not set.
sub getargs {
    my ($argname, $default) = @_;

    foreach my $arg (@ARGV) {
        my ($value) = ($arg =~ /^$argname=(.+)$/);
        return [split ',', $value] if defined $value;
    }

    return [split ',', $default];
}

# Return 1 or 0 if set by "$argname=1" or "$argname=0" on the 
# command line. Return $default if not set.
sub getbools {
    my ($argname, $default) = @_;

    my @values = @{getargs($argname, $default)};
    return [( map { ($_ eq "0") ? 0 : 1 } @values )];
}

# Return an integer if set by "$argname=value" on the 
# command line. Return $default if not set.
sub getints {
    my ($argname, $default) = @_;

    my @values = @{getargs($argname, $default)};
    return [( map { int($_) } @values )];
}

sub getarg {
    my ($argname, $default) = @_;
    my @values = @{getargs($argname, $default)};
    die "Only one value allowed for $argname\n"  if @values > 1;
    return $values[0];
}

sub getbool {
    my ($argname, $default) = @_;
    my @values = @{getbools($argname, $default)};
    die "Only one value allowed for $argname\n"  if @values > 1;
    return $values[0];
}

sub getint {
    my ($argname, $default) = @_;
    my @values = @{getints($argname, $default)};
    die "Only one value allowed for $argname\n"  if @values > 1;
    return $values[0];
}


my $default_arch = "x86_64";
$args{ARCH} = getargs("ARCH", 0);
$args{ARCH} = getargs("ARCHS", $default_arch)  if !@{$args{ARCH}}[0];

$args{OSVERSION} = getargs("OS", "macosx-default-default");

$args{MEM} = getargs("MEM", "mrc,arc");
$args{LANGUAGE} = [ map { lc($_) } @{getargs("LANGUAGE", "c,objective-c,c++,objective-c++")} ];

$args{BUILD_SHARED_CACHE} = getargs("BUILD_SHARED_CACHE", 0);

$args{DYLD} = getargs("DYLD", "2,3");

$args{CC} = getargs("CC", "clang");

$HOST = getarg("HOST", "iphone");
$PORT = getarg("PORT", "10022");

{
    my $guardmalloc = getargs("GUARDMALLOC", 0);    
    # GUARDMALLOC=1 is the same as GUARDMALLOC=before,after
    my @guardmalloc2 = ();
    for my $arg (@$guardmalloc) {
        if ($arg == 1) { push @guardmalloc2, "before"; 
                         push @guardmalloc2, "after"; }
        else { push @guardmalloc2, $arg }
    }
    $args{GUARDMALLOC} = \@guardmalloc2;
}

$BUILD = getbool("BUILD", 1);
$RUN = getbool("RUN", 1);
$VERBOSE = getint("VERBOSE", 0);
$BATS = getbool("BATS", 0);
$BUILDDIR = getarg("BUILDDIR", $BATS ? $BATSBASE : $LOCALBASE);

my $root = getarg("ROOT", "");
$root =~ s#/*$##;

my @tests = gettests();

if ($BUILD) {
    rm_rf_verbose "$DSTROOT$BUILDDIR";
    rm_rf_verbose "$OBJROOT$BUILDDIR";
}

print "note: -----\n";
print "note: testing root '$root'\n";

my @configs = make_configs($root, %args);

print "note: -----\n";
print "note: testing ", scalar(@configs), " configurations:\n";
for my $configref (@configs) {
    my $configname = $$configref{NAME};
    print "note: configuration $configname\n";
}

my $failed = 0;

my $testconfigs = @configs;
my $failconfigs = 0;
my $skipconfigs = 0;
my $testcount = 0;
my $failcount = 0;
for my $configref (@configs) {
    my $configname = $$configref{NAME};
    print "note: -----\n";
    print "note: \nnote: $configname\nnote: \n";

    (my $t, my $f, my $skipconfig) = 
        eval { build_and_run_one_config($configref, @tests); };
    $skipconfigs += $skipconfig;
    if ($@) {
        chomp $@;
        colorprint $red, "FAIL: $configname";
        colorprint $red, "FAIL: $@";
        $failconfigs++;
    } else {
        my $color = ($f ? $red : "");
        print "note:\n";
        colorprint $color, "note: $configname\n";
        colorprint $color, "note: $t tests, $f failures";
        $testcount += $t;
        $failcount += $f;
        $failconfigs++ if ($f);
    }
}

print "note: -----\n";
my $color = ($failconfigs ? $red : "");
colorprint $color, "note: $testconfigs configurations, " . 
    "$failconfigs with failures, $skipconfigs skipped";
colorprint $color, "note: $testcount tests, $failcount failures";

$failed = ($failconfigs ? 1 : 0);


if ($BUILD && $BATS && !$failed) {
    # Collect BATS execution instructions for all tests.
    # Each BATS "test" is all configurations together of one of our tests.
    for my $testname (@tests) {
        append_bats_test($testname);
    }

    # Write the BATS plist to disk.
    my $json = encode_json(\%bats_plist);
    my $filename = "$DSTROOT$BATSBASE/objc4.plist";
    print "note: writing BATS config to $filename\n";
    open(my $file, '>', $filename);
    print $file $json;
    close $file;
}

exit ($failed ? 1 : 0);
