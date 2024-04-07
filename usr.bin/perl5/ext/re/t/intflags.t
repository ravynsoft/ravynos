#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ /\bre\b/) ){
	print "1..0 # Skip -- Perl configured without re module\n";
	exit 0;
    }
}

use strict;

# must use a BEGIN or the prototypes wont be respected meaning
# tests could pass that shouldn't.
BEGIN { require "../../t/test.pl"; }
my $out = runperl(progfile => "t/intflags.pl", stderr => 1 );
like($out,qr/-OK-\n/, "intflags.pl ran to completion");

my %seen;
foreach my $line (split /\n/, $out) {
    $line=~s/^r->intflags:\s+// or next;
    length($_) and $seen{$_}++ for split /\s+/, $line;
}
is(0+keys %seen,14);
done_testing;
