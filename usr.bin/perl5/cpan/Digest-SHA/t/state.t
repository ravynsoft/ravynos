use strict;
use Digest::SHA qw(sha384_hex sha512_hex);

my @sharsp = (
	"34aa973cd4c4daa4f61eeb2bdbad27316534016f",
	"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
	"9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985",
	"e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"
);

my $numtests = scalar @sharsp;
print "1..$numtests\n";

my($state001, $state256, $state384, $state512) = ('', '', '', '');
for (1 .. 8) { my $line = <DATA>; $state001 .= $line }
for (1 .. 8) { my $line = <DATA>; $state256 .= $line }
for (1 .. 8) { my $line = <DATA>; $state384 .= $line }
for (1 .. 8) { my $line = <DATA>; $state512 .= $line }
my @states = ($state001, $state256, $state384, $state512);

my @alg = (1, 256, 384, 512);
my $data = "a" x 990000;

my $testnum = 1;
while (@sharsp) {
	my $skip = 0;
	my $alg = shift @alg;
	my $rsp = shift @sharsp;
	if ($alg == 384) { $skip = sha384_hex("") ? 0 : 1 }
	if ($alg == 512) { $skip = sha512_hex("") ? 0 : 1 }
	if ($skip) {
		print "ok ", $testnum++, " # skip: no 64-bit\n";
		next;
	}
	my $digest;
	my $state;
	unless ($state = Digest::SHA->putstate(shift @states)) {
		print "not ok ", $testnum++, "\n";
		next;
	}
	my $statestr = $state->add_bits($data, 79984)->getstate;
	$state->putstate($statestr)->add_bits($data, 16);
	$digest = $state->hexdigest;
	print "not " unless $digest eq $rsp;
	print "ok ", $testnum++, "\n";
}

__DATA__
alg:1
H:9d6f7d2f:65e21307:c6f41af6:7c7fd3a9:8dec6058:00000000:00000000:00000000
block:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
blockcnt:384
lenhh:0
lenhl:0
lenlh:0
lenll:7920000
alg:256
H:2d6c0def:4244ade7:fc8c121c:108f4493:ec3fbec2:91425a6e:b8d30d2a:9db24273
block:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
blockcnt:384
lenhh:0
lenhl:0
lenlh:0
lenll:7920000
alg:384
H:598147f4583a61f7:8d194a4d7c9008cb:39725c96557d600f:d7f2079ce8251f19:bd735d446f9a3c7c:234de90b9060898d:a5b481b9d635d190:81c6e74ee4556125
block:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
blockcnt:384
lenhh:0
lenhl:0
lenlh:0
lenll:7920000
alg:512
H:0442fe29a02b8c30:13553e6dbedc2aa0:8f891a0cb2ac3107:6fa1762b40ac04dd:dcbf420d729eea79:34703e9672dcf145:7bf9aaa14d400433:2aa65f044825466d
block:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:61:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
blockcnt:384
lenhh:0
lenhl:0
lenlh:0
lenll:7920000
