use strict;
use Digest::SHA qw(sha512_hex sha512224_hex sha512256_hex);

my @vecs = map { eval } <DATA>;

my $numtests = scalar(@vecs) / 2;
print "1..$numtests\n";

my $skip = sha512_hex("") ? 0 : 1;

for (1 .. $numtests) {
	my $data = shift @vecs;
	my $digest = shift @vecs;
	unless ($skip) {
		my $rsp = ($_ <= $numtests/2) ?
			sha512224_hex($data) : sha512256_hex($data);
		print "not " unless $rsp eq $digest;
	}
	print "ok ", $_, $skip ? " # skip: no 64-bit" : "", "\n";
}

__DATA__
"abc"
"4634270f707b6a54daae7530460842e20e37ed265ceee9a43e8924aa"
"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
"23fec5bb94d60b23308192640b0c453335d664734fe40e7268674af9"
"abc"
"53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23"
"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
"3928e184fb8690f840da3988121d31be65cb9d3ef83ee6146feac861e19b563a"
