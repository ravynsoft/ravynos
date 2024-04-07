# Test against long bitwise vectors from Jim Gillogly and Francois Grieu

use strict;
use Digest::SHA;

#	SHA-1 Test Vectors
#
#	In the following we use the notation bitstring#n to mean a bitstring
#	repeated n (in decimal) times, and we use | for concatenation.
#	Therefore 110#3|1 is 1101101101.
#
#	Here is a set near 2^32 bits to test the roll-over in the length
#	field from one to two 32-bit words:
#
#	110#1431655764|11 1eef5a18 969255a3 b1793a2a 955c7ec2 8cd221a5
#	110#1431655765|   7a1045b9 14672afa ce8d90e6 d19b3a6a da3cb879
#	110#1431655765|1  d5e09777 a94f1ea9 240874c4 8d9fecb6 b634256b
#	110#1431655765|11 eb256904 3c3014e5 1b2862ae 6eb5fb4e 0b851d99
#
#	011#1431655764|01 4CB0C4EF 69143D5B F34FC35F 1D4B19F6 ECCAE0F2
#	011#1431655765    47D92F91 1FC7BB74 DE00ADFC 4E981A81 05556D52
#	011#1431655765|0  A3D7438C 589B0B93 2AA91CC2 446F06DF 9ABC73F0
#	011#1431655765|01 3EEE3E1E 28DEDE2C A444D68D A5675B2F AAAB3203

my @vec110 = (	# 110 rep 1431655764
	"11", "1eef5a18969255a3b1793a2a955c7ec28cd221a5",
	"110", "7a1045b914672aface8d90e6d19b3a6ada3cb879",
	"1101", "d5e09777a94f1ea9240874c48d9fecb6b634256b",
	"11011", "eb2569043c3014e51b2862ae6eb5fb4e0b851d99"
);

my @vec011 = (	# 011 rep 1431655764
	"01", "4cb0c4ef69143d5bf34fc35f1d4b19f6eccae0f2",
	"011", "47d92f911fc7bb74de00adfc4e981a8105556d52",
	"0110", "a3d7438c589b0b932aa91cc2446f06df9abc73f0",
	"01101", "3eee3e1e28dede2ca444d68da5675b2faaab3203"
);

my($STATE110, $STATE011) = ('', '');
for (1 .. 8) { my $line = <DATA>; $STATE110 .= $line }
for (1 .. 8) { my $line = <DATA>; $STATE011 .= $line }

my $testnum = 1;
print "1..", scalar(@vec110)/2 + scalar(@vec011)/2, "\n";

my $state110 = Digest::SHA->putstate($STATE110);
while (@vec110) {
	my $state = $state110->clone;
	$state->add_bits(shift @vec110);
	print "not " unless $state->hexdigest eq (shift @vec110);
	print "ok ", $testnum++, "\n";
}

my $state011 = Digest::SHA->putstate($STATE011);
while (@vec011) {
	my $state = $state011->clone;
	$state->add_bits(shift @vec011);
	print "not " unless $state->hexdigest eq (shift @vec011);
	print "ok ", $testnum++, "\n";
}

__DATA__
alg:1
H:dfc51a14:87b4a4b7:ecf19acd:8cbbe40e:03a435f8:00000000:00000000:00000000
block:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d
blockcnt:508
lenhh:0
lenhl:0
lenlh:0
lenll:4294967292
alg:1
H:7950cbe2:86a45aa0:91ff7dff:29015b42:3912e764:00000000:00000000:00000000
block:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6:db:6d:b6
blockcnt:508
lenhh:0
lenhl:0
lenlh:0
lenll:4294967292
