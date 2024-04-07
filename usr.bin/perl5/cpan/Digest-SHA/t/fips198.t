use strict;
use Digest::SHA qw(hmac_sha1_hex);

my @data = map { "Sample #$_" } (1 .. 4);

my @out = (
	"4f4ca3d5d68ba7cc0a1208c9c61e9c5da0403c0a",
	"0922d3405faa3d194f82a45830737d5cc6c75d24",
	"bcf41eab8bb2d802f3d05caf7cb092ecf8d1a3aa",
	"9ea886efe268dbecce420c7524df32e0751a2a26"
);

my @keys = ("", "", "", "");

for (0x00 .. 0x00+63) { $keys[0] .= chr($_) }
for (0x30 .. 0x30+19) { $keys[1] .= chr($_) }
for (0x50 .. 0x50+99) { $keys[2] .= chr($_) }
for (0x70 .. 0x70+48) { $keys[3] .= chr($_) }

my $numtests = scalar @data;
print "1..$numtests\n";

my $testnum = 1;
while (@data) {
	print "not " unless hmac_sha1_hex(shift @data, shift @keys)
		eq shift @out;
	print "ok ", $testnum++, "\n";
}
