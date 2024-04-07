use strict;
use Digest::SHA qw(sha384_hex);

my @vecs = map { eval } <DATA>;

my $numtests = scalar(@vecs) / 2;
print "1..$numtests\n";

my $skip = sha384_hex("") ? 0 : 1;

for (1 .. $numtests) {
	my $data = shift @vecs;
	my $digest = shift @vecs;
	unless ($skip) {
		print "not " unless sha384_hex($data) eq $digest;
	}
	print "ok ", $_, $skip ? " # skip: no 64-bit" : "", "\n";
}

__DATA__
"abc"
"cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7"
"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
"09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039"
"a" x 1000000
"9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985"
