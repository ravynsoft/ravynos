# Test against SHA-1 Sample Vectors from NIST

use strict;
use Digest::SHA;

my $nist_hashes = <<END_OF_NIST_HASHES;
DA39A3EE5E6B4B0D3255BFEF95601890AFD80709 ^
59C4526AA2CC59F9A5F56B5579BA7108E7CCB61A ^
6E42FB84067CFF056C43A49E484997AF23190879 ^
C63FBB9A87171A176E6E054890E29A8C5F125F6C ^
3109E33C1C4B9A0169D1599169D0E5A520A1E71C ^
9195E1E73CC68D7170F44BD1D83CB624BC87FA0B ^
64F7C374527278C0436DBC8DE5AABEC2BBF634BC ^
154B622EA426FB151B1FF1BE1CE871752B9EDEB4 ^
12BDD00FD4038756CBCF8ECDAD1B0CD862603CD8 ^
6700F93E1691E83735279E167F67AF61FEE9813B ^
END_OF_NIST_HASHES

my @hashes = $nist_hashes =~ /\b[0-9A-F]{40}\b/g;

my $nist_messages = <<END_OF_NIST_MESSAGES;
0 1 ^
1 1 1 ^
2 1 1 1 ^
3 0 1 1 1 ^
2 0 2 2 ^
4 1 1 1 2 1 ^
3 0 2 2 2 ^
4 1 1 2 2 2 ^
5 1 2 2 1 1 2 ^
5 0 2 2 1 1 3 ^
END_OF_NIST_MESSAGES

my @lines = split(/\n/, $nist_messages);

print "1..", scalar(@hashes), "\n";
my $testnum = 1;

my $message = "";
my $sha = Digest::SHA->new(1);
for (@lines) {
	next unless /^[\d ^]/;
	$message .= $_;
	next unless /\^\s*$/;
	my @vals = $message =~ /\d+/g; $message = "";
	my $count = shift(@vals);
	my $bit = shift(@vals);
	my $bitstr = "";
	while (@vals) {
		$bitstr .= $bit x shift(@vals);
		$bit = 1 - $bit;
	}
	print "not " unless uc($sha->add_bits($bitstr)->hexdigest)
		eq shift(@hashes);
	print "ok ", $testnum++, "\n";
}
