#!perl -w

use strict;
use warnings;

use Digest::MD5 qw(md5_hex);

print "1..6\n";

my $a = Digest::MD5->new;
$a->add("a");
my $b = $a->clone;

print "not " unless $b->clone->hexdigest eq md5_hex("a");
print "ok 1\n";

$a->add("a");
print "not " unless $a->hexdigest eq md5_hex("aa");
print "ok 2\n";

print "not " unless $a->hexdigest eq md5_hex("");
print "ok 3\n";

$b->add("b");
print "not " unless $b->clone->hexdigest eq md5_hex("ab");
print "ok 4\n";

$b->add("c");
print "not " unless $b->clone->hexdigest eq md5_hex("abc");
print "ok 5\n";

# Test that cloning picks up the correct class for subclasses.
{
   package MD5;
   @MD5::ISA = qw(Digest::MD5);
}

$a = MD5->new;
$a->add("a");
$b = $a->clone;

print "not " unless ref($b) eq "MD5" && $b->add("b")->hexdigest eq md5_hex("ab");
print "ok 6\n";
