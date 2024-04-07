#!/usr/bin/perl

use strict;
use warnings;

# Demonstrate correctness of SYNOPSIS in documentation
$| = 1;
my $file = "tf42-$$.txt";
my $dupe = "ft42-$$.txt";
1 while unlink $file;
1 while unlink $dupe;

print "1..21\n";

my $MAX = 42;
open my $F, ">", $file or die "Unable to open $file for writing: $!";
for my $i (0..$MAX) {
    print $F "PERL-${i}\n";
}
close $F or die "Unable to close $file after writing: $!";

my $N = 1;
use Tie::File;
print "ok $N - use Tie::File\n"; $N++;

my $desc = 'Tie::File';

my @array;
my $o = tie @array, 'Tie::File', $file;
defined ($o)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

    $desc = "first element in array corresponds to first line of file";
    ($array[0] eq "PERL-0")
        ? print "ok $N - $desc\n"
        : print "not ok $N - $desc\n";
    $N++;

    $desc = "last element in array corresponds to last line of file";
    ($array[$MAX] eq "PERL-$MAX")
        ? print "ok $N - $desc\n"
        : print "not ok $N - $desc\n";
    $N++;

    $desc = "got expected amount of records in file";
    my $n_recs = @array;
    ($n_recs == $MAX + 1)
        ? print "ok $N - $desc\n"
        : print "not ok $N - $desc\n";
    $N++;

    my $chop = 2;
    $#array -= $chop;
    $desc = "chop records off end of file";
    $n_recs = @array;
    ($n_recs == $MAX + 1 - $chop)
        ? print "ok $N - $desc\n"
        : print "not ok $N - $desc\n";
    $N++;

    $desc = "replace PERL with Perl everywhere in the file";
for (@array) { s/PERL/Perl/g; }
my $exp = "Perl-" . ($MAX - 2);
($array[-1] eq $exp)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

# push @array, new recs...;
# my $r1 = pop @array;
# unshift @array, new recs...;
# my $r2 = shift @array;
# @old_recs = splice @array, 3, 7, new recs...;
# Demonstrate that the tied file has changed in the way we expect

$desc = "push new records onto end of file";
my @end_recs = (qw| alpha beta gamma |);
push @array, @end_recs;
$n_recs = @array;
($n_recs == $MAX + 1 - $chop + @end_recs)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "last element in array corresponds to last line of file";
($array[-1] eq $end_recs[-1])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "pop last record off";
my $r1 = pop @array;
($array[-1] eq $end_recs[-2])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "unshift new records onto beginning of file";
my @start_recs = (qw| albemarle beverly cortelyou |);
unshift @array, @start_recs;
$n_recs = @array;
$exp = $MAX + 1 - $chop + @end_recs - 1 + @start_recs;
($n_recs == $exp)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "first element in array corresponds to first line of file";
($array[0] eq $start_recs[0])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "shift one record off beginning of file";
my $r2 = shift @array;
$n_recs = @array;
$exp = $MAX + 1 - $chop + @end_recs - 1 + @start_recs - 1;
($n_recs == $exp)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "new first element in array";
($array[0] eq $start_recs[1])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

my @splice_in = (qw| delta epsilon zeta eta theta |);
my $offset = 2;
my $length = 3;
$desc = "splice out $length elements and splice in " . @splice_in . " new elements";
my @old_recs = splice @array, $offset, $length, @splice_in;
$n_recs = @array;
$exp = $MAX + 1 - $chop + @end_recs - 1 + @start_recs - 1 - 3 + @splice_in;
($n_recs == $exp)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "got expected element";
($array[6] eq $splice_in[4])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$o = undef;  # destroy Tie::File object holding file open
# Untie the first file
my $u = untie @array;
# TODO: perldoc -f untie does not specify return value for untie

open my $G, "<", $file or die "Unable to open $file for reading: $!";
open my $H, ">", $dupe or die "Unable to open $dupe for writing: $!";
while (my $l = <$G>) {
    chomp $l;
    print $H "$l\n";
}
close $H or die "Unable to close $dupe after writing: $!";
close $G or die "Unable to close $file after reading: $!";

$desc = 'tie to dupe file';
my @dupe;
my $p = tie @dupe, 'Tie::File', $file;
defined ($p)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "same number of records in dupe file as in original file";
my $o_recs = @dupe;
($o_recs == $n_recs)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "first element in dupe array corresponds to first line of dupe file";
($dupe[0] eq $start_recs[1])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$exp = $splice_in[4];
$desc = "got expected element $exp";
($dupe[6] eq $exp)
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

$desc = "last element in dupe array corresponds to last line of dupe file";
($dupe[-1] eq $end_recs[-2])
    ? print "ok $N - $desc\n"
    : print "not ok $N - $desc\n";
$N++;

END {
    undef $o;
    undef $p;
    untie @array;
    untie @dupe;
    1 while unlink $file;
    1 while unlink $dupe;
}

