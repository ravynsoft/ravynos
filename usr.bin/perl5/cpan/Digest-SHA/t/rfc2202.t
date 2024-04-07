use strict;
use Digest::SHA qw(hmac_sha1_hex);

my @data = map { eval } <DATA>;

my $numtests = scalar @data;
print "1..$numtests\n";

my @keys = (
	chr(0x0b) x 20,
	"Jefe",
	chr(0xaa) x 20,
	join("", map { chr } (1 .. 25)),
	chr(0x0c) x 20,
	chr(0xaa) x 80,
	chr(0xaa) x 80
);

my @out = (
	"b617318655057264e28bc0b6fb378c8ef146be00",
	"effcdf6ae5eb2fa2d27416d5f184df9c259a7c79",
	"125d7342b9ac11cd91a39af48aa17b4f63f175d3",
	"4c9007f4026250c6bc8414f9bf50c86c2d7235da",
	"4c1a03424b55e07fe7f27be1d58bb9324a9a5a04",
	"aa4ae5e15272d00e95705637ce8a3b55ed402112",
	"e8e99d0f45237d786d6bbaa7965c7808bbff1a91"
);

my $testnum = 1;
while (@data) {
	print "not " unless hmac_sha1_hex(shift @data, shift @keys)
		eq shift @out;
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
