BEGIN {
    if ($ENV{'PERL_CORE'}) {
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
      print "1..0 # Skip: EBCDIC\n";
      exit 0;
    }
    $| = 1;
}

use strict;
use warnings;

use Test::More tests => 8;

use Encode;

my $ascii = Encode::find_encoding("ascii");
my $orig = "str";

my $str = $orig;
ok !Encode::is_utf8($str), "UTF8 flag is not set on input string before ascii encode";
$ascii->encode($str);
ok !Encode::is_utf8($str), "UTF8 flag is not set on input string after ascii encode";

$str = $orig;
ok !Encode::is_utf8($str), "UTF8 flag is not set on input string before Encode::encode ascii";
Encode::encode("ascii", $str);
ok !Encode::is_utf8($str), "UTF8 flag is not set on input string after Encode::encode ascii";

$str = $orig;
Encode::_utf8_on($str);
ok Encode::is_utf8($str), "UTF8 flag is set on input string before ascii decode";
$ascii->decode($str);
ok Encode::is_utf8($str), "UTF8 flag is set on input string after ascii decode";

$str = $orig;
Encode::_utf8_on($str);
ok Encode::is_utf8($str), "UTF8 flag is set on input string before Encode::decode ascii";
Encode::decode("ascii", $str);
ok Encode::is_utf8($str), "UTF8 flag is set on input string after Encode::decode ascii";
