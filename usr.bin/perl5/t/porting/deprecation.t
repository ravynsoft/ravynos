#!/usr/bin/perl

BEGIN {
  if (-f './TestInit.pm') {
    @INC = '.';
  } elsif (-f '../TestInit.pm') {
    @INC = '..';
  }
}
use TestInit qw(T); # T is chdir to the top level

use warnings;
use strict;
use Config;
use Data::Dumper;
require './t/test.pl';

plan("no_plan");

# Test that all deprecations in regen/warnings.pl are mentioned in
# pod/perldeprecation.pod and that there is sufficient time between them.

my $pod_file = "./pod/perldeprecation.pod";
my $warnings_file = "./regen/warnings.pl";

do $warnings_file;
our $WARNING_TREE;

my $deprecated = $WARNING_TREE->{all}[1]{deprecated}[2];

open my $fh, "<", $pod_file
    or die "failed to open '$pod_file': $!";
my $removed_in_version;
my $subject;
my %category_seen;
my %subject_has_category;
my $in_legacy;

while (<$fh>) {
    if (/^=head2 (?|Perl (5\.\d+)(?:\.\d+)?|(Unscheduled))/) { # ignore minor version
        $removed_in_version = lc $1;
        if ($removed_in_version eq "5.38") {
            $in_legacy = 1;
        }
    }
    elsif (/^=head3 (.*)/) {
        my $new_subject = $1;
        if (!$in_legacy and $subject) {
            ok($subject_has_category{$subject},
                "Subject '$subject' has a category specified");
        }
        $subject = $new_subject;
    }
    elsif (/^Category: "([::\w]+)"/) {
        my $category = $1;
        $category_seen{$category} = $removed_in_version;
        $subject_has_category{$subject} = $category;
        next if $removed_in_version eq "unscheduled";
        my $tuple = $deprecated->{$category};
        ok( $tuple, "Deprecated category '$category' ($subject) exists in $warnings_file")
            or next;
        my $added_in_version = $tuple->[0];
        $added_in_version =~ s/(5\.\d{3})\d+/$1/;

        my $diff = $removed_in_version - $added_in_version;
        cmp_ok($diff, ">=", 0.004, # two production cycles
            "Version change for '$category' ($subject) is sufficiently after deprecation date")
    }
}
# make sure that all the deprecated categories have an entry of some sort
foreach my $category (sort keys %$deprecated) {
    ok($category_seen{$category},"Deprecated category '$category' is documented in $pod_file");
}
# make sure that there arent any new uses of WARN_DEPRECATED,
# note that \< and \> are ERE expressions roughly equivalent to perl regex \b
if (-e ".git") {
    chomp(my @warn_deprecated = `git grep "\<WARN_DEPRECATED\>"`);
    my %files;
    foreach my $line (@warn_deprecated) {
        my ($file, $text) = split /:/, $line, 2;
        if ($file =~ m!^dist/Devel-PPPort! ||
            $file eq "t/porting/diag.t" ||
            ($file eq "warnings.h" && $text=~/^[=#]/)
        ) {
            next;
        }
        $files{$file}++;
    }
    is(0+keys %files, 0,
        "There should not be any new files which mention WARN_DEPRECATED");
}

# Test that deprecation warnings are produced under "use warnings"
# (set above)
{
    my $warning = "nada";
    local $SIG{__WARN__} = sub { $warning = $_[0] };
    my $count = 0;
    while ($count<1) {
        LABEL: $count++;
        goto DONE if $count>1;
    }
    goto LABEL;
    DONE:
    like($warning,
        qr/Use of "goto" to jump into a construct is deprecated/,
        "Got expected deprecation warning");
}
# Test that we can silence deprecation warnings with "no warnings 'deprecated'"
# as we used to.
{
    no warnings 'deprecated';
    my $warning = "nada";
    local $SIG{__WARN__} = sub { $warning = $_[0] };
    my $count = 0;
    while ($count<1) {
        LABEL: $count++;
        goto DONE if $count>1;
    }
    goto LABEL;
    DONE:
    like($warning, qr/nada/,
        "no warnings 'deprecated'; silenced deprecation warning as expected");
}

# Test that we can silence a specific deprecation warnings with "no warnings 'deprecated::$subcategory'"
# and that by doing so we don't silence any other deprecation warnings.
{
    no warnings 'deprecated::goto_construct';
    my $warning = "nada";
    local $SIG{__WARN__} = sub { $warning = $_[0] };
    my $count = 0;
    while ($count<1) {
        LABEL: $count++;
        goto DONE if $count>1;
    }
    goto LABEL;
    DONE:
    like($warning, qr/nada/,
        "no warnings 'deprecated::goto_construct'; silenced deprecation warning as expected");
    @INC = ();
    do "regen.pl"; # this should produce a deprecation warning
    like($warning, qr/is no longer in \@INC/,
        "no warnings 'deprecated::goto_construct'; did not silence deprecated::dot_in_inc warnings");
}
