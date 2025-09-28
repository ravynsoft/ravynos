# Test against SHA-1 Sample Vectors from NIST

use strict;
use Digest::SHA;

my $nist_hashes = <<END_OF_NIST_HASHES;
DA39A3EE5E6B4B0D3255BFEF95601890AFD80709 ^
3CDF2936DA2FC556BFA533AB1EB59CE710AC80E5 ^
19C1E2048FA7393CFBF2D310AD8209EC11D996E5 ^
CA775D8C80FAA6F87FA62BECA6CA6089D63B56E5 ^
71AC973D0E4B50AE9E5043FF4D615381120A25A0 ^
A6B5B9F854CFB76701C3BDDBF374B3094EA49CBA ^
D87A0EE74E4B9AD72E6847C87BDEEB3D07844380 ^
1976B8DD509FE66BF09C9A8D33534D4EF4F63BFD ^
5A78F439B6DB845BB8A558E4CEB106CD7B7FF783 ^
F871BCE62436C1E280357416695EE2EF9B83695C ^
END_OF_NIST_HASHES

my @hashes = $nist_hashes =~ /\b[0-9A-F]{40}\b/g;

my $nist_messages = <<END_OF_NIST_MESSAGES;
0 1 ^
5 0 2 1 2 1 2 ^
5 0 1 3 4 4 4 ^
7 0 4 3 4 4 1 4 4 ^
10 0 4 1 5 3 4 4 3 1 3 4 ^
10 0 3 1 6 5 5 1 3 6 6 4 ^
13 1 3 2 5 3 3 3 4 6 6 1 4 6 2 ^
16 1 3 5 5 1 2 1 3 3 6 3 5 2 3 5 7 2 ^
15 1 8 1 5 3 2 7 4 5 6 7 3 3 1 6 3 ^
15 1 4 6 8 2 1 4 2 5 1 6 8 8 6 4 7 ^
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
	print "not " unless
		uc($sha->add(pack("B*", $bitstr))->hexdigest)
			eq shift(@hashes);
	print "ok ", $testnum++, "\n";
}
