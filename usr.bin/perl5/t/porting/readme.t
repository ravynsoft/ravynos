#!./perl -w
#
# Check whether all files mentioned in Porting/README.pod exist in Porting and
# vice versa.

BEGIN {
    @INC = '..' if -f '../TestInit.pm';
}

use TestInit qw(T); # T is chdir to the top level
use strict;
use warnings;
require './t/test.pl';

my @porting_files;
open my $man, "MANIFEST" or die "Can't open MANIFEST: $!";
while(<$man>) {
    /^Porting\// and s/[\t\n].*//s, push @porting_files, $_;
}
close $man or die "Can't close MANIFEST: $!";
# It seems that dying here is nicer than having several dozen failing tests
# later.  But that assumes one will see the message from die.
die "Can't get contents of Porting/ directory.\n" unless @porting_files > 1;

open(my $fh, '<', 'Porting/README.pod') or die("Can't open Porting/README.pod: $!");

my (@current_order, @sorted_order, %files_in_pod);
while(<$fh>) {
    next unless $_ =~ /^=head/;
    my @matches = $_ =~ m/F<([^>]+)>/g;
    for my $file (@matches) {
        $files_in_pod{$file} = 1;
        push @current_order, $file;
    }
}

for my $file (@porting_files) {
    $file =~ s!^Porting/!!;
    next if $file =~ /^perl[0-9]+delta\.pod$/;
    ok(exists($files_in_pod{$file}), "$file is mentioned in Porting/README.pod");
    delete $files_in_pod{$file};
}
for my $file (keys %files_in_pod) {
    fail("$file exists in Porting/");
}

# Check if the entries in the README are in some sort of order.
eval {
    require Unicode::Collate;
    my $Collator = Unicode::Collate->new();
    @sorted_order = $Collator->sort(@current_order);
};

if(@sorted_order) {
    ok(eq_array(\@current_order, \@sorted_order), "Files are referenced in order") or
        print_right_order();
}
else {
    note('Unicode collation did not work.  Not checking order of entries.');
}

# Frankly this is a bit much for a porting test, but it exists now.
sub print_right_order {
    my $max = 0;
    for(@current_order) {
        my $l = length $_;
        $max = $l if $l > $max;
    }
    $max = 36 if $max > 36;
    note(sprintf " N   %-${max}s %-${max}s\n", "Correct", "Current");
    for(0..$#current_order) {
        my $wrong = $sorted_order[$_] eq $current_order[$_] ? '' : 'X';
        my $line = sprintf "%2d %1s %-${max}s %-${max}s\n",
            $_, $wrong, $sorted_order[$_], $current_order[$_];
        $line =~ s{ ((?:  ){2,})}{" " . ". " x (length($1)/2)}e if $_&1;
        note($line);
    }
}

done_testing();
