use strict;
use Digest::SHA qw(sha256_hex);

my @vecs = map { eval } <DATA>;

my $numtests = scalar(@vecs) / 2;
print "1..$numtests\n";

for (1 .. $numtests) {
	my $data = shift @vecs;
	my $digest = shift @vecs;
	print "not " unless sha256_hex($data) eq $digest;
	print "ok ", $_, "\n";
}

__DATA__
"abc"
"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"
"a" x 1000000
"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"
