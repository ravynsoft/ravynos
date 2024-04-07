#!./perl -w

# Check that we can "upgrade" from anything to anything else.
# Curiously, before this, lib/Math/Trig.t was the only code anywhere in the
# build or testsuite that upgraded an NV to an RV

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

my $null;

$! = 1;
my %types = (
    null => $null,
    iv => 3,
    nv => .5,
    rv => [],
    pv => "Perl rules",
    pviv => 3,
    pvnv => 1==1,
    pvmg => $^,
);

# This is somewhat cheating but I can't think of anything built in that I can
# copy that already has type PVIV
$types{pviv} = "Perl rules!";

# use Devel::Peek; Dump $pvmg;

my @keys = keys %types;
plan tests => @keys * @keys;

foreach my $source_type (@keys) {
    foreach my $dest_type (@keys) {
	# Pads re-using variables might contaminate this
	my $vars = {};
	$vars->{dest} = $types{$dest_type};
	$vars->{source} = $types{$source_type};
	# The assignment can potentially trigger assertion failures, so it's
	# useful to have the diagnostics about what was attempted printed first
	print "# Assigning $source_type to $dest_type\n";
	$vars->{dest} = $vars->{source};
	is ($vars->{dest}, $vars->{source});
    }
}
