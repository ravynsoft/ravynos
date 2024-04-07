use strict;
use Digest::SHA qw(sha1_base64 sha224_base64 sha256_base64
	sha384_base64 sha512_base64);

my $in = "abc";
my @out = map { eval } <DATA>;
my @fcn = (\&sha1_base64, \&sha224_base64, \&sha256_base64,
	\&sha384_base64, \&sha512_base64);

print "1..", scalar @out, "\n";

my $testnum = 1;
while (@out) {
	my $fcn = shift @fcn;
	my $rsp = shift @out;
	my $skip = &$fcn("") ? 0 : 1;
	unless ($skip) {
		print "not " unless &$fcn($in) eq $rsp;
	}
	print "ok ", $testnum++, $skip ? " # skip: no 64-bit" : "", "\n";
}

__DATA__
"qZk+NkcGgWq6PiVxeFDCbJzQ2J0"
"Iwl9IjQF2CKGQqR3vaJVsyqtvOS9oLP342ydpw"
"ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0"
"ywB1P0WjXou1oD1pmsZQBycsMqsO3tFjGotgWkP/W+2AhgcroefMI1i67KE0yCWn"
"3a81oZNherrMQXNJriBBMRLm+k6JqX6iCp7u5ktV05ohkpkqJ0/BqDa6PCOj/uu9RU1EI2Q86A4qmslPpUyknw"

