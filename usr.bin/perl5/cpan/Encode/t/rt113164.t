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

use Test::More tests => 2;

use Encode;

my $str = "You" . chr(8217) . "re doomed!";

my $data;

my $cb = sub {
    $data = [ ('?') x 12_500 ];
    return ";";
};

my $octets = encode('iso-8859-1', $str, $cb);
is $octets, "You;re doomed!", "stack was not overwritten";

$octets = encode('iso-8859-1', $str, $cb);
is $octets, "You;re doomed!", "stack was not overwritten";
