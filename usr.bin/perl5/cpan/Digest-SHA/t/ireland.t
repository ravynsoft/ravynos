# David Ireland's test vector - SHA-256 digest of "a" x 536870912

# Adapted from Julius Duque's original script (t/24-ireland.tmp)
#	- modified to use state cache via putstate method

use strict;
use Digest::SHA;

print "1..1\n";

my $rsp = "b9045a713caed5dff3d3b783e98d1ce5778d8bc331ee4119d707072312af06a7";

my $sha;
if ($sha = Digest::SHA->putstate(join('', <DATA>))) {
	$sha->add("aa");
	print "not " unless $sha->hexdigest eq $rsp;
	print "ok 1\n";
}
else { print "not ok 1\n" }

__DATA__

	# Verify comments/blank lines ignored in state data
			
alg:256
H:dd75eb45:02d4f043:06b41193:6fda751d:73064db9:787d54e1:52dc3fe0:48687dfa

block:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:00:00
blockcnt:496
                            
lenhh:0
lenhl:0
lenlh:0

# Note: add'ing two more bytes will cause lenll (below) to overflow

lenll:4294967280
