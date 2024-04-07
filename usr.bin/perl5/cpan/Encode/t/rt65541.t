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

use Encode;
use PerlIO::encoding;
$PerlIO::encoding::fallback &= ~Encode::WARN_ON_ERR;

use Test::More tests => 3;

ok open my $fh, ">:encoding(cp1250)", do{\(my $str)};
ok print $fh ("a" x 1023) . "\x{0378}";
ok close $fh;
