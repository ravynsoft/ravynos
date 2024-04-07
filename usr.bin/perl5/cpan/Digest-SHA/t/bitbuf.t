use strict;
use Digest::SHA;

my $numtests = 4;
print "1..$numtests\n";

	# Here's the bitstring to test against, and its SHA-1 digest

my $ONEBITS = pack("B*", "1" x 80000);
my $digest = "11003389959355c2773af6b0f36d842fe430ec49";

my $state = Digest::SHA->new("sHa1");
my $testnum = 1;

$state->add_bits($ONEBITS, 80000);
print "not " unless $state->hexdigest eq $digest;
print "ok ", $testnum++, "\n";

	# buffer using a series of increasingly large bitstrings

# Note that (1 + 2 + ... + 399) + 200 = 80000

for (1 .. 399) {
	$state->add_bits($ONEBITS, $_);
}
$state->add_bits($ONEBITS, 200);

print "not " unless $state->hexdigest eq $digest;
print "ok ", $testnum++, "\n";

	# create a buffer-alignment nuisance

$state = Digest::SHA->new("1");

$state->add_bits($ONEBITS, 1);
for (1 .. 99) {
	$state->add_bits($ONEBITS, 800);
}
$state->add_bits($ONEBITS, 799);

print "not " unless $state->hexdigest eq $digest;
print "ok ", $testnum++, "\n";

	# buffer randomly-sized bitstrings

my $reps = 80000;
my $maxbits = 8 * 127;

$state = Digest::SHA->new(1);

while ($reps > $maxbits) {
	my $num = int(rand($maxbits));
	$state->add_bits($ONEBITS, $num);
	$reps -= $num;
}
$state->add_bits($ONEBITS, $reps);

print "not " unless $state->hexdigest eq $digest;
print "ok ", $testnum++, "\n";
