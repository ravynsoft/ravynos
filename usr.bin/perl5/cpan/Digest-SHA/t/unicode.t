use strict;
use Digest::SHA qw(sha1_hex);

my $skip = $] < 5.006 ? 1 : 0;

my $TEMPLATE = $] >= 5.006 ? 'U*' : 'C*';
my $empty_unicode = pack($TEMPLATE, ());
my $ok_unicode    = pack($TEMPLATE, (0..255));
my $wide_unicode  = pack($TEMPLATE, (0..256));

print "1..3\n";

unless ($skip) {
	print "not " unless sha1_hex($empty_unicode."abc") eq
		"a9993e364706816aba3e25717850c26c9cd0d89d";
}
print "ok 1", $skip ? " # skip: no Unicode" : "", "\n";

unless ($skip) {
	print "not " unless sha1_hex($ok_unicode) eq
		"4916d6bdb7f78e6803698cab32d1586ea457dfc8";
}
print "ok 2", $skip ? " # skip: no Unicode" : "", "\n";

unless ($skip) {
	eval { sha1_hex($wide_unicode) };
	print "not " unless $@ =~ /Wide character/;
}
print "ok 3", $skip ? " # skip: no Unicode" : "", "\n";
