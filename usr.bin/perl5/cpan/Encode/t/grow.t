#!../perl
our $POWER;
BEGIN {
     if ($ENV{'PERL_CORE'}){
         chdir 't';
         unshift @INC, '../lib';
     }
     require Config; import Config;
     if ($Config{'extensions'} !~ /\bEncode\b/) {
         print "1..0 # Skip: Encode was not built\n";
             exit 0;
     }
     $POWER = 12; # up to 1 MB.  You may adjust the figure here
}

use strict;
use Encode;

my $seed = "";
for my $i (0x00..0xff){
     my $c = chr($i);
     $seed .= ($c =~ /^\p{IsPrint}/o) ? $c : " ";
}

use Test::More tests => $POWER*2;
my $octs = $seed;
use bytes ();
for my $i (1..$POWER){
     $octs .= $octs;
     my $len = bytes::length($octs);
     my $utf8 = Encode::decode('latin1', $octs);
     ok(1, "decode $len bytes");
     is($octs,
        Encode::encode('latin1', $utf8),
        "encode $len bytes");
}
__END__


