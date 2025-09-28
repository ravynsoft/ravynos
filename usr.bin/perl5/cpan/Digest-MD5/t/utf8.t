#!perl -w

BEGIN {
    if ($] < 5.006) {
	print "1..0 # Skipped: your perl don't know unicode\n";
	exit;
    }
}

use strict;
use warnings;

use Digest::MD5 qw(md5_hex);

print "1..5\n";

my $str;
$str = "foo\xFF\x{100}";

eval {
    print md5_hex($str);
    print "not ok 1\n";  # should not run
};
print "not " unless $@ && $@ =~ /^(Big byte|Wide character)/;
print "ok 1\n";

my $exp = ord "A" == 193 ? # EBCDIC
	   "c307ec81deba65e9a222ca81cd8f6ccd" :
	   "503debffe559537231ed24f25651ec20"; # Latin 1

chop($str);  # only bytes left
print "not " unless md5_hex($str) eq $exp;
print "ok 2\n";

# reference
print "not " unless md5_hex("foo\xFF") eq $exp;
print "ok 3\n";

# autopromotion
if ($] >= 5.007003) {

my $unistring = "Oslo.pm har sosialt medlemsmøte onsdag 1. April 2008, klokken 18:30. Vi treffes på Marhaba Café, Keysersgate 1.";

require Encode;
$unistring = Encode::decode_utf8($unistring);
print "not " if ( not utf8::is_utf8($unistring));
print "ok 4\n";

md5_hex($unistring, "");
print "not " if ( not utf8::is_utf8($unistring));
print "ok 5\n"

} else {
    print "ok 4 # SKIP Your perl is too old to properly test unicode semantics\nok 5 # SKIP No, really\n";
}
