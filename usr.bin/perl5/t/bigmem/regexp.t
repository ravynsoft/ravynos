#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
    require './test.pl';
}

use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 2
    or skip_all("Need ~2Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(6);

# [perl #116907]
# ${\2} to defeat constant folding, which in this case actually slows
# things down
my $x=" "x(${\2}**31) . "abcdefg";
ok $x =~ /./, 'match against long string succeeded';
is "$-[0]-$+[0]", '0-1', '@-/@+ after match against long string';

pos $x = 2**31-1;
my $result;
for(1..5) {
    $x =~ /./g;
    $result .= "$&-";
}
is $result," -a-b-c-d-", 'scalar //g hopping past the 2**31 threshold';
pos $x = 2**31+3;
$x =~ /./g;
is "$'", 'efg', q "$' after match against long string";
is "$-[0],$+[0]", '2147483651,2147483652',
   '@- and @+ after matches past 2**31';

# Substring optimisations
is $x =~ /(?:(?:.{32766}){32766}){2}(?:.{32766}){8}.{8}ef/, 1,
  'anchored substr past 2**31';
