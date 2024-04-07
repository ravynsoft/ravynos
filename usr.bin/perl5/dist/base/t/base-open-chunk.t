#!/usr/bin/perl -w

my $file = __FILE__;

open my $fh, '<', $file or die "Can't open $file: $!";
$/ = \1;
<$fh>;
(my $test_file = $file) =~ s/-open-chunk//;

unless (my $return = do "./$test_file") {
    warn "couldn't parse $test_file: $@" if $@;
    warn "couldn't do $test_file: $!"    unless defined $return;
    warn "couldn't run $test_file"       unless $return;
}
