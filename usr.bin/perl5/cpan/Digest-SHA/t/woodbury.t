# Test Vectors for HMAC-SHA-256/384/512
#
# Vectors and initial script courtesy of Adam Woodbury, The MITRE Corporation

use strict;
use Digest::SHA qw(hmac_sha256 hmac_sha384 hmac_sha512);

my @plex = map { eval } <DATA>;

my $numtests = scalar(@plex) / 3;
print "1..$numtests\n";

# Use RFC 2202 data/key values for 512-bit blocks

my @data_bs512 = splice(@plex, 0, 7);
my @keys_bs512 = splice(@plex, 0, 7);
my @hmac256rsp = splice(@plex, 0, 7);

# Lengthen final RFC 2202 data/key values for 1024-bit blocks

my @data_bs1024 = splice(@plex, 0, 7);
my @keys_bs1024 = splice(@plex, 0, 7);
my @hmac384rsp  = splice(@plex, 0, 7);

my @dat2_bs1024 = splice(@plex, 0, 7);
my @key2_bs1024 = splice(@plex, 0, 7);
my @hmac512rsp  = splice(@plex, 0, 7);

my $testnum = 1;

while (@data_bs512) {
	print "not " unless
		hmac_sha256(shift @data_bs512, shift @keys_bs512)
			eq pack("H*", shift @hmac256rsp);
	print "ok ", $testnum++, "\n";
}

my $skip = hmac_sha384("", "") ? 0 : 1;

while (@data_bs1024) {
	if ($skip) {
		print "ok ", $testnum++,
			$skip ? " # skip: no 64-bit" : "", "\n";
		shift @data_bs1024;
		next;
	}
	print "not " unless
		hmac_sha384(shift @data_bs1024, shift @keys_bs1024)
			eq pack("H*", shift @hmac384rsp);
	print "ok ", $testnum++, "\n";
}

while (@dat2_bs1024) {
	if ($skip) {
		print "ok ", $testnum++,
			$skip ? " # skip: no 64-bit" : "", "\n";
		shift @dat2_bs1024;
		next;
	}
	print "not " unless
		hmac_sha512(shift @dat2_bs1024, shift @key2_bs1024)
			eq pack("H*", shift @hmac512rsp);
	print "ok ", $testnum++, "\n";
}

__DATA__
"Hi There"
"what do ya want for nothing?"
chr(0xdd) x 50
chr(0xcd) x 50
"Test With Truncation"
"Test Using Larger Than Block-Size Key - Hash Key First"
"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data"
chr(0x0b) x 20
"Jefe"
chr(0xaa) x 20
join("", map { chr } (1 .. 25))
chr(0x0c) x 20
chr(0xaa) x 80
chr(0xaa) x 80
"b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"
"5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"
"773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe"
"82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b"
"a3b6167473100ee06e0c796c2955552bfa6f7c0a6a8aef8b93f860aab0cd20c5"
"6953025ed96f0c09f80a96f78e6538dbe2e7b820e3dd970e7ddd39091b32352f"
"6355ac22e890d0a3c8481a5ca4825bc884d3e7a1ff98a2fc2ac7d8e064c3b2e6"
"Hi There"
"what do ya want for nothing?"
chr(0xdd) x 50
chr(0xcd) x 50
"Test With Truncation"
"Test Using Larger Than Block-Size Key - Hash Key First"
"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data. The Larger Block-Sizes Make For Much Longer Test Vectors"
chr(0x0b) x 20
"Jefe"
chr(0xaa) x 20
join("", map { chr } (1 .. 25))
chr(0x0c) x 20
chr(0xaa) x 200
chr(0xaa) x 200
"afd03944d84895626b0825f4ab46907f15f9dadbe4101ec682aa034c7cebc59cfaea9ea9076ede7f4af152e8b2fa9cb6"
"af45d2e376484031617f78d2b58a6b1b9c7ef464f5a01b47e42ec3736322445e8e2240ca5e69e2c78b3239ecfab21649"
"88062608d3e6ad8a0aa2ace014c8a86f0aa635d947ac9febe83ef4e55966144b2a5ab39dc13814b94e3ab6e101a34f27"
"3e8a69b7783c25851933ab6290af6ca77a9981480850009cc5577c6e1f573b4e6801dd23c4a7d679ccf8a386c674cffb"
"3abf34c3503b2a23a46efc619baef897f4c8e42c934ce55ccbae9740fcbc1af4ca62269e2a37cd88ba926341efe4aeea"
"ec629fe0dc1fab504fc1c89572d6573cf15c3a4b5b69d53f0c13849561a6c13e153af48d2538ce056a3fe10d69da16c3"
"07109d2c6c2fdcac39c3a8b5f36fc9a69e029d3d8647cc3e4ddb77888418c5c09d807942e5f96d17ee9fd46aed64b7f2"
"Hi There"
"what do ya want for nothing?"
chr(0xdd) x 50
chr(0xcd) x 50
"Test With Truncation"
"Test Using Larger Than Block-Size Key - Hash Key First"
"Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data. The Larger Block-Sizes Make For Much Longer Test Vectors"
chr(0x0b) x 20
"Jefe"
chr(0xaa) x 20
join("", map { chr } (1 .. 25))
chr(0x0c) x 20
chr(0xaa) x 200
chr(0xaa) x 200
"87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854"
"164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf75c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737"
"fa73b0089d56a284efb0f0756c890be9b1b5dbdd8ee81a3655f83e33b2279d39bf3e848279a722c806b485a47e67c807b946a337bee8942674278859e13292fb"
"b0ba465637458c6990e5a8c5f61d4af7e576d97ff94b872de76f8050361ee3dba91ca5c11aa25eb4d679275cc5788063a5f19741120c4f2de2adebeb10a298dd"
"415fad6271580a531d4179bc891d87a650188707922a4fbb36663a1eb16da008711c5b50ddd0fc235084eb9d3364a1454fb2ef67cd1d29fe6773068ea266e96b"
"9dc6330f4c966b62b735d565343cb77413deccdf42a92d9ef5e4e2ae33f6c924bbc8e34c47111bc069482d4dbcfee148419a6547f2d01500e8160b39cc2e4ae8"
"396ed3a17cef82cddbd987ea66a5dd1f38b68167df31f049463b85fa10b531d0e90d1052f8c9c7cda263468ec3f980a8fec06213c2944c92a0ac95a2d8ade76d"
