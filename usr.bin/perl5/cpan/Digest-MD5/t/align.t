# Test that md5 works on unaligned memory blocks

use strict;
use warnings;

print "1..1\n";

use Digest::MD5 qw(md5_hex);

my $str = "\100" x 20;
substr($str, 0, 1) = "";  # chopping off first char makes the string unaligned

#use Devel::Peek; Dump($str); 

print "not " unless md5_hex($str) eq "c7ebb510e59ee96f404f288d14cc656a";
print "ok 1\n";

