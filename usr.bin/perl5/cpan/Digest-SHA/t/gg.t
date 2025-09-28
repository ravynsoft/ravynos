# Test against short bitwise vectors from Jim Gillogly and Francois Grieu

use strict;
use Digest::SHA;

#	SHA-1 Test Vectors
#
#	In the following we use the notation bitstring#n to mean a bitstring
#	repeated n (in decimal) times, and we use | for concatenation.
#	Therefore 110#3|1 is 1101101101.
#
#	110#148|11  : CE7387AE 577337BE 54EA94F8 2C842E8B E76BC3E1
#	110#149     : DE244F06 3142CB2F 4C903B7F 7660577F 9E0D8791
#	110#149|1   : A3D29824 27AE39C8 920CA5F4 99D6C2BD 71EBF03C
#	110#149|11  : 351AAB58 FF93CF12 AF7D5A58 4CFC8F7D 81023D10
#
#	110#170     : 99638692 1E480D4E 2955E727 5DF3522C E8F5AB6E
#	110#170|1   : BB5F4AD4 8913F51B 157EB985 A5C2034B 8243B01B
#	110#170|11  : 9E92C554 2237B957 BA2244E8 141FDB66 DEC730A5
#	110#171     : 2103E454 DA4491F4 E32DD425 A3341DC9 C2A90848
#
#	011#490     : B4B18049 DE405027 528CD9E7 4B2EC540 D4E6F06B
#	011#490|0   : 34C63356 B3087427 20AB9669 14EB0FC9 26E4294B
#	011#490|01  : 75FACE18 02B9F84F 326368AB 06E73E05 02E9EA34
#	011#491     : 7C2C3D62 F6AEC28D 94CDF93F 02E739E7 490698A1

my @vecs = (
	"110",148,"11","ce7387ae577337be54ea94f82c842e8be76bc3e1",
	"110",149,"","de244f063142cb2f4c903b7f7660577f9e0d8791",
	"110",149,"1","a3d2982427ae39c8920ca5f499d6c2bd71ebf03c",
	"110",149,"11","351aab58ff93cf12af7d5a584cfc8f7d81023d10",
	"110",170,"","996386921e480d4e2955e7275df3522ce8f5ab6e",
	"110",170,"1","bb5f4ad48913f51b157eb985a5c2034b8243b01b",
	"110",170,"11","9e92c5542237b957ba2244e8141fdb66dec730a5",
	"110",171,"","2103e454da4491f4e32dd425a3341dc9c2a90848",
	"011",490,"","b4b18049de405027528cd9e74b2ec540d4e6f06b",
	"011",490,"0","34c63356b308742720ab966914eb0fc926e4294b",
	"011",490,"01","75face1802b9f84f326368ab06e73e0502e9ea34",
	"011",491,"","7c2c3d62f6aec28d94cdf93f02e739e7490698a1",
);

my $numtests = scalar(@vecs) / 4;
print "1..$numtests\n";

my $testnum = 1;
my $sha = Digest::SHA->new(1);

while (@vecs) {
	my $frag = shift @vecs;
	my $reps = shift @vecs;
	my $tail = shift @vecs;
	my $bitstr = ($frag x $reps) . $tail;
	print "not " unless $sha->add_bits($bitstr)->hexdigest
		eq shift @vecs;
	print "ok ", $testnum++, "\n";
}
