# This code is used by lib/charnames.t, lib/croak.t, lib/feature.t,
# lib/subs.t, lib/strict.t and lib/warnings.t
#
# On input, $::local_tests is the number of tests in the caller; or
# 'no_plan' if unknown, in which case it is the caller's responsibility
# to call cur_test() to find out how many this executed

BEGIN {
    require './test.pl'; require './charset_tools.pl';
}

use Config;
use File::Path;
use File::Spec::Functions qw(catfile curdir rel2abs);

use strict;
use warnings;
my (undef, $file) = caller;
my ($pragma_name) = $file =~ /([A-Za-z_0-9]+)\.t$/
    or die "Can't identify pragma to test from file name '$file'";

$| = 1;

my @w_files;

if (@ARGV) {
    print "ARGV = [@ARGV]\n";
    @w_files = map { "./lib/$pragma_name/$_" } @ARGV;
} else {
    @w_files = sort grep !/( \.rej | ~ | \ \(Autosaved\)\.txt ) \z/nx,
			 glob catfile(curdir(), "lib", $pragma_name, "*");
}

if ($::IS_EBCDIC) { # Skip Latin1 files
    @w_files = grep { $_ !~ / _l1 $/x } @w_files
}

my ($tests, @prgs) = setup_multiple_progs(@w_files);

$^X = rel2abs($^X);
@INC = map { rel2abs($_) } @INC;
my $tempdir = tempfile;

mkdir $tempdir, 0700 or die "Can't mkdir '$tempdir': $!";
chdir $tempdir or die die "Can't chdir '$tempdir': $!";
my $cleanup = 1;

END {
    if ($cleanup) {
	chdir '..' or die "Couldn't chdir .. for cleanup: $!";
	rmtree($tempdir);
    }
}

if ($::local_tests && $::local_tests =~ /\D/) {
    # If input is 'no_plan', pass it on unchanged
    plan $::local_tests;
} else {
    plan $tests + ($::local_tests || 0);
}

run_multiple_progs('../..', @prgs);

1;
