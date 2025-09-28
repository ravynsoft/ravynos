#!/usr/bin/perl

# Check that the various config.sh-clones have (at least) all the
# same symbols as the top-level config_h.SH so that the (potentially)
# needed symbols are not lagging after how Configure thinks the world
# is laid out.
#
# VMS is probably not handled properly here, due to their own
# rather elaborate DCL scripting.

use strict;
use warnings;
use autodie;

sub usage {
    my $err = shift and select STDERR;
    print "usage: $0 [--list] [--regen] [--default=value]\n";
    exit $err;
    } # usage

use Getopt::Long qw(:config bundling);
GetOptions (
    "help|?"      => sub { usage (0); },
    "l|list!"     => \(my $opt_l = 0),
    "regen"       => \(my $opt_r = 0),
    "default=s"   => \ my $default,
    "tap"         => \(my $tap   = 0),
    "v|verbose:1" => \(my $opt_v = 0),
    ) or usage (1);

$default and $default =~ s/^'(.*)'$/$1/; # Will be quoted on generation
my $test;

require './regen/regen_lib.pl' if $opt_r;

my $MASTER_CFG = "config_h.SH";
# Inclusive bounds on the main part of the file, $section == 1 below:
my $first = qr/^Author=/;
my $last = qr/^zip=/;

my @CFG = (
	   # we check from MANIFEST whether they are expected to be present.
	   # We can't base our check on $], because that's the version of the
	   # perl that we are running, not the version of the source tree.
	   "Cross/config.sh-arm-linux",
	   "Cross/config.sh-arm-linux-n770",
	   "uconfig.sh",
	   "uconfig64.sh",
	   "plan9/config_sh.sample",
	   "win32/config.gc",
	   "win32/config.vc",
	   "configure.com",
	   "Porting/config.sh",
	  );

my @MASTER_CFG;
{
    my %seen;
    $opt_v and warn "Reading $MASTER_CFG ...\n";
    open my $fh, '<', $MASTER_CFG;
    while (<$fh>) {
	while (/[^\\]\$([a-z]\w+)/g) {
	    my $v = $1;
	    next if $v =~ /^(CONFIG_H|CONFIG_SH)$/;
	    $seen{$v}++;
	}
    }
    close $fh;
    @MASTER_CFG = sort keys %seen;
}

my %MANIFEST;

{
    $opt_v and warn "Reading MANIFEST ...\n";
    open my $fh, '<', 'MANIFEST';
    while (<$fh>) {
	$MANIFEST{$1}++ if /^(.+?)\t/;
    }
    close $fh;
}

printf "1..%d\n", 2 * @CFG if $tap;

for my $cfg (sort @CFG) {
    unless (exists $MANIFEST{$cfg}) {
	warn "[skipping not-expected '$cfg']\n";
	next;
    }
    my %cfg;
    my $section = 0;
    my @lines;

    $opt_v and warn "Reading $cfg ...\n";
    open my $fh, '<', $cfg or die "$cfg: $!\n";

    if ($cfg eq 'configure.com') {
	++$cfg{startperl}; # Cheat.

	while (<$fh>) {
	    next if /^\#/ || /^\s*$/ || /^\:/;
	    s/(\s*!.*|\s*)$//; # remove trailing comments or whitespace
	    ++$cfg{$1} if /^\$\s+WC "(\w+)='(?:.*)'"$/;
	}
    } else {
	while (<$fh>) {
	    if ($_ =~ $first) {
		die "$cfg:$.:section=$section:$_" unless $section == 0;
		$section = 1;
	    }
	    push @{$lines[$section]}, $_;
	    next if /^\#/ || /^\s*$/ || /^\:/;
	    if ($_ =~ $last) {
		die "$cfg:$.:section=$section:$_" unless $section == 1;
		$section = 2;
	    }
	    # foo='bar'
	    # foo=bar
	    # (optionally with a trailing comment)
	    if (/^(\w+)=(?:'.*'|[^'].*)(?: #.*)?$/) {
		++$cfg{$1};
	    } else {
		warn "$cfg:$.:$_";
	    }
	}
    }
    close $fh;

    ++$test;
    my $missing;
    if ($cfg eq 'configure.com') {
	print "ok $test # skip $cfg doesn't need to be sorted\n"
	    if $tap;
    } elsif (join("", @{$lines[1]}) eq join("", sort @{$lines[1]})) {
	print "ok $test - $cfg sorted\n"
	    if $tap;
    } elsif ($tap) {
	print "not ok $test - $cfg is not sorted\n";
    } elsif ($opt_r || $opt_l) {
	# A reference to an empty array is true, hence this flags the
	# file for later attention by --regen and --list, even if
	# nothing is missing. Actual sort and output are done later.
	$missing = [];
    } else {
	print "$cfg: unsorted\n"
    }

    for my $v (@MASTER_CFG) {
	# This only creates a reference in $missing if something is missing:
	push @$missing, $v unless exists $cfg{$v};
    }

    ++$test;
    if ($missing) {
	if ($tap) {
	    print "not ok $test - $cfg missing keys @$missing\n";
	} elsif ($opt_l) {
	    # print the name once, however many problems
	    print "$cfg\n";
	} elsif ($opt_r && $cfg ne 'configure.com') {
	    if (defined $default) {
		push @{$lines[1]}, map {"$_='$default'\n"} @$missing;
	    } else {
		print "$cfg: missing '$_', use --default to add it\n"
		    foreach @$missing;
	    }

	    @{$lines[1]} = sort @{$lines[1]};
	    my $fh = open_new($cfg);
	    print $fh @{$_} foreach @lines;
	    close_and_rename($fh);
	} else {
	    print "$cfg: missing '$_'\n" foreach @$missing;
	}
    } elsif ($tap) {
	print "ok $test - $cfg has no missing keys\n";
    }
}
