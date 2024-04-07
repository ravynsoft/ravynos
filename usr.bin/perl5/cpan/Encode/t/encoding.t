BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    unless (find PerlIO::Layer 'perlio') {
    print "1..0 # Skip: PerlIO was not built\n";
    exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: encoding pragma does not support EBCDIC platforms\n";
    exit(0);
    }
    if ($] >= 5.025 and !$Config{usecperl}) {
    print "1..0 # Skip: encoding pragma not supported in Perl 5.25 or later\n";
    exit(0);
    }
}

print "1..33\n";
 

no warnings "deprecated";
use encoding "latin1"; # ignored (overwritten by the next line)
use encoding "greek";  # iso 8859-7 (no "latin" alias, surprise...)

# "greek" is "ISO 8859-7", and \xDF in ISO 8859-7 is
# \x{3AF} in Unicode (GREEK SMALL LETTER IOTA WITH TONOS),
# instead of \xDF in Unicode (LATIN SMALL LETTER SHARP S)

$a = "\xDF";
$b = "\x{100}";

print "not " unless ord($a) == 0x3af;
print "ok 1\n";

print "not " unless ord($b) == 0x100;
print "ok 2\n";

my $c;

$c = $a . $b;

print "not " unless ord($c) == 0x3af;
print "ok 3\n";

print "not " unless length($c) == 2;
print "ok 4\n";

print "not " unless ord(substr($c, 1, 1)) == 0x100;
print "ok 5\n";

print "not " unless ord(chr(0xdf)) == 0x3af; # spooky
print "ok 6\n";

print "not " unless ord(pack("C", 0xdf)) == 0x3af;
print "ok 7\n";

# we didn't break pack/unpack, I hope

print "not " unless unpack("C", pack("C", 0xdf)) == 0xdf;
print "ok 8\n";

# the first octet of UTF-8 encoded 0x3af 
print "not " unless unpack("U0 C", chr(0xdf)) == 0xce;
print "ok 9\n";

print "not " unless unpack("U", pack("U", 0xdf)) == 0xdf;
print "ok 10\n";

print "not " unless unpack("U", chr(0xdf)) == 0x3af;
print "ok 11\n";

# charnames must still work
use charnames ':full';
print "not " unless ord("\N{LATIN SMALL LETTER SHARP S}") == 0xdf;
print "ok 12\n";

# combine

$c = "\xDF\N{LATIN SMALL LETTER SHARP S}" . chr(0xdf);

print "not " unless ord($c) == 0x3af;
print "ok 13\n";

print "not " unless ord(substr($c, 1, 1)) == 0xdf;
print "ok 14\n";

print "not " unless ord(substr($c, 2, 1)) == 0x3af;
print "ok 15\n";

# regex literals

print "not " unless "\xDF"    =~ /\x{3AF}/;
print "ok 16\n";

print "not " unless "\x{3AF}" =~ /\xDF/;
print "ok 17\n";

print "not " unless "\xDF"    =~ /\xDF/;
print "ok 18\n";

print "not " unless "\x{3AF}" =~ /\x{3AF}/;
print "ok 19\n";

# eq, cmp

my ($byte,$bytes,$U,$Ub,$g1,$g2,$l) = ( 
    pack("C*", 0xDF ),       # byte
    pack("C*", 0xDF, 0x20),  # ($bytes2 cmp $U) > 0
    pack("U*", 0x3AF),       # $U eq $byte
    pack("U*", 0xDF ),       # $Ub would eq $bytev w/o use encoding
    pack("U*", 0x3B1),       # ($g1 cmp $byte) > 0; === chr(0xe1)
    pack("U*", 0x3AF, 0x20), # ($g2 cmp $byte) > 0;
    pack("U*", 0x3AB),       # ($l  cmp $byte) < 0; === chr(0xdb)
);

# all the tests in this section that compare a byte encoded string 
# ato UTF-8 encoded are run in all possible vairants 
# all of the eq, ne, cmp operations tested,
# $v z $u tested as well as $u z $v

sub alleq($$){
    my ($a,$b)    =    (shift, shift);
     $a  eq  $b        &&     $b  eq  $a         && 
  !( $a  ne  $b )      &&  !( $b  ne  $a )       &&
   ( $a  cmp $b ) == 0 &&   ( $b  cmp $a ) == 0;
}
   
sub anyeq($$){
    my ($a,$b)    =    (shift, shift);
     $a  eq  $b        ||     $b  eq  $a         ||
  !( $a  ne  $b )      ||  !( $b  ne  $a )       ||
   ( $a  cmp $b ) == 0 ||   ( $b  cmp $a ) == 0;
}

sub allgt($$){
    my ($a,$b)    =    (shift, shift);
    ( $a cmp $b ) == 1 && ( $b cmp $a ) == -1;
}
#match the correct UTF-8 string
print "not " unless  alleq($byte, $U);
print "ok 20\n";

#do not match a wrong UTF-8 string
print "not " if anyeq($byte, $Ub);
print "ok 21\n";

#string ordering
print "not " unless allgt ( $g1,    $byte  )  &&
                    allgt ( $g2,    $byte  )  &&
                    allgt ( $byte,  $l     )  &&
                    allgt ( $bytes, $U     );
print "ok 22\n";

# upgrade, downgrade

my ($u,$v,$v2);
$u = $v = $v2 = pack("C*", 0xDF);
utf8::upgrade($v);                   #explicit upgrade
$v2 = substr( $v2."\x{410}", 0, -1); #implicit upgrade

# implicit upgrade === explicit upgrade
print "not "  if do{{use bytes; $v ne $v2}} || $v ne $v2;
print "ok 23\n";

# utf8::upgrade is transparent and does not break equality
print "not " unless alleq( $u, $v );
print "ok 24\n";

$u = $v = pack("C*", 0xDF);
utf8::upgrade($v);
#test for a roundtrip, we should get back from where we left
eval {utf8::downgrade( $v )};
print "not " if $@ !~ /^Wide / || do{{use bytes; $u eq $v}} || $u ne $v;
print "ok 25\n";

# some more eq, cmp

$byte=pack("C*", 0xDF);

print "not " unless pack("U*", 0x3AF) eq $byte;
print "ok 26\n";

print "not " if chr(0xDF) cmp $byte;
print "ok 27\n";

print "not " unless ((pack("U*", 0x3B0)       cmp $byte) ==  1) &&
                    ((pack("U*", 0x3AE)       cmp $byte) == -1) &&
                    ((pack("U*", 0x3AF, 0x20) cmp $byte) ==  1) &&
                ((pack("U*", 0x3AF) cmp pack("C*",0xDF,0x20))==-1);
print "ok 28\n";


{
    # Used to core dump in 5.7.3
    no warnings; # so test goes noiselessly
    print ord(undef) == 0 ? "ok 29\n" : "not ok 29\n";
}

{
    my %h1;
    my %h2;
    $h1{"\xdf"}    = 41;
    $h2{"\x{3af}"} = 42;
    print $h1{"\x{3af}"} == 41 ? "ok 30\n" : "not ok 30\n";
    print $h2{"\xdf"}    == 42 ? "ok 31\n" : "not ok 31\n";
}

# Order of finding the above-Latin1 code point should not matter: both should
# assume Latin1/Unicode encoding
{
    use bytes;
    print "not " if "\xDF\x{100}" =~ /\x{3af}\x{100}/;
    print "ok 32\n";
    print "not " if "\x{100}\xDF" =~ /\x{100}\x{3af}/;
    print "ok 33\n";
}
