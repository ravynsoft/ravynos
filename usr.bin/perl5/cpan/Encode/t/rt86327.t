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

my @t = qw/230 13 90 65 34 239 86 15 8 26 181 25 305 123 22 139 111 6 3
100 37 1 20 1 166 1 300 19 1 42 153 81 106 114 67 1 32 34/;
my $str;
ok open OUT, '>:encoding(iso-8859-1)', \$str;
my $string = join "\x{fffd}", map { '.'x$_ } @t;
ok print OUT $string;
ok close OUT;
