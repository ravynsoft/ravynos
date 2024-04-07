use strict;
use FileHandle;
use Digest::SHA;

my @out = (
	"ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0",
	"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
);

my $numtests = 6 + scalar @out;
print "1..$numtests\n";

	# attempt to use an invalid algorithm, and check for failure

my $testnum = 1;
my $NSA = "SHA-42";	# No Such Algorithm
print "not " if Digest::SHA->new($NSA);
print "ok ", $testnum++, "\n";

my $tempfile = "methods.tmp";
END { unlink $tempfile if $tempfile }

	# test OO methods using first two SHA-256 vectors from NIST

my $fh = FileHandle->new($tempfile, "w");
binmode($fh);
print $fh "bcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
$fh->close;

my $sha = Digest::SHA->new()->reset("SHA-256")->new();
$sha->add_bits("a", 5)->add_bits("001");

my $rsp = shift(@out);
print "not " unless $sha->clone->add("b", "c")->b64digest eq $rsp;
print "ok ", $testnum++, "\n";

$rsp = shift(@out);

	# test addfile with bareword filehandle

open(FILE, "<$tempfile");
binmode(FILE);
print "not " unless
	$sha->clone->addfile(*FILE)->hexdigest eq $rsp;
print "ok ", $testnum++, "\n";
close(FILE);

	# test addfile with indirect filehandle

$fh = FileHandle->new($tempfile, "r");
binmode($fh);
print "not " unless $sha->clone->addfile($fh)->hexdigest eq $rsp;
print "ok ", $testnum++, "\n";
$fh->close;

	# test addfile using file name instead of handle

print "not " unless $sha->addfile($tempfile, "b")->hexdigest eq $rsp;
print "ok ", $testnum++, "\n";

	# test addfile "universal newlines" mode

$fh = FileHandle->new($tempfile, "w");
binmode($fh);
print $fh "MacOS\r" . "MSDOS\r\n" . "UNIX\n" . "Quirky\r\r\n";
$fh->close;

my $d = $sha->new(1)->addfile($tempfile, "U")->hexdigest;
if ($d eq "f4c6855783c737c7e224873c90e80a9df5c2bc97") {
	print "ok ", $testnum++, "\n";
}
elsif ($d eq "42335d4a517a5e31399e948e9d842bafd9194d8f") {
	print "ok ", $testnum++, " # skip:  flaky -T\n";
}
else {
	print "not ok ", $testnum++, "\n";
}

	# test addfile BITS mode

$fh = FileHandle->new($tempfile, "w");
print $fh "0100010";			# using NIST 7-bit test vector
$fh->close;

print "not " unless $sha->new(1)->addfile($tempfile, "0")->hexdigest eq
	"04f31807151181ad0db278a1660526b0aeef64c2";
print "ok ", $testnum++, "\n";

$fh = FileHandle->new($tempfile, "w");
binmode($fh);
print $fh map(chr, (0..127));		# this is actually NIST 2-bit test
$fh->close;				# vector "01" (other chars ignored)

print "not " unless $sha->new(1)->addfile($tempfile, "0")->hexdigest eq
	"ec6b39952e1a3ec3ab3507185cf756181c84bbe2";
print "ok ", $testnum++, "\n";
