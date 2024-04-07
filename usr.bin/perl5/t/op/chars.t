#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 34;

# because of ebcdic.c these should be the same on asciiish 
# and ebcdic machines.
# Peter Prymmer <pvhp@best.com>.

my $c = "\c@";
is (ord($c), 0, '\c@');
$c = "\cA";
is (ord($c), 1, '\cA');
$c = "\cB";
is (ord($c), 2, '\cB');
$c = "\cC";
is (ord($c), 3, '\cC');
$c = "\cD";
is (ord($c), 4, '\cD');
$c = "\cE";
is (ord($c), 5, '\cE');
$c = "\cF";
is (ord($c), 6, '\cF');
$c = "\cG";
is (ord($c), 7, '\cG');
$c = "\cH";
is (ord($c), 8, '\cH');
$c = "\cI";
is (ord($c), 9, '\cI');
$c = "\cJ";
is (ord($c), 10, '\cJ');
$c = "\cK";
is (ord($c), 11, '\cK');
$c = "\cL";
is (ord($c), 12, '\cL');
$c = "\cM";
is (ord($c), 13, '\cM');
$c = "\cN";
is (ord($c), 14, '\cN');
$c = "\cO";
is (ord($c), 15, '\cO');
$c = "\cP";
is (ord($c), 16, '\cP');
$c = "\cQ";
is (ord($c), 17, '\cQ');
$c = "\cR";
is (ord($c), 18, '\cR');
$c = "\cS";
is (ord($c), 19, '\cS');
$c = "\cT";
is (ord($c), 20, '\cT');
$c = "\cU";
is (ord($c), 21, '\cU');
$c = "\cV";
is (ord($c), 22, '\cV');
$c = "\cW";
is (ord($c), 23, '\cW');
$c = "\cX";
is (ord($c), 24, '\cX');
$c = "\cY";
is (ord($c), 25, '\cY');
$c = "\cZ";
is (ord($c), 26, '\cZ');
$c = "\c[";
is (ord($c), 27, '\c[');
$c = "\c\\";
is (ord($c), 28, '\c\\');
$c = "\c]";
is (ord($c), 29, '\c]');
$c = "\c^";
is (ord($c), 30, '\c^');
$c = "\c_";
is (ord($c), 31, '\c_');

# '\c?' is an outlier, and is treated differently on each platform.
# It's DEL on ASCII, and APC on EBCDIC
$c = "\c?";
is (ord($c), ($::IS_ASCII)
             ? 127
             : utf8::unicode_to_native(0x9F),
              '\c?');
$c = '';
is (ord($c), 0, 'ord("") is 0');
