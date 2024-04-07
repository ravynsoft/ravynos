use strict;
use Test;
use Win32;

plan tests => 1;

my $scalar = Win32::GetOSVersion();
my @array  = Win32::GetOSVersion();

print "not " unless $scalar == $array[4];
print "ok 1\n";
