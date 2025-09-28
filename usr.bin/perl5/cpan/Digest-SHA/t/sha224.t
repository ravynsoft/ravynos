use strict;
use Digest::SHA qw(sha224_hex);

my @vecs = map { eval } <DATA>;

my $numtests = scalar(@vecs) / 2;
print "1..$numtests\n";

for (1 .. $numtests) {
	my $data = shift @vecs;
	my $digest = shift @vecs;
	print "not " unless sha224_hex($data) eq $digest;
	print "ok ", $_, "\n";
}

__DATA__
"abc"
"23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7"
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
"75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525"
"a" x 1000000
"20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67"
