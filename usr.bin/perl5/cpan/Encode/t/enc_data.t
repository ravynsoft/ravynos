# $Id: enc_data.t,v 2.6 2022/04/07 03:06:40 dankogai Exp $

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
    if ($] <= 5.008 and !$Config{perl_patchlevel}){
    print "1..0 # Skip: Perl 5.8.1 or later required\n";
    exit 0;
    }
}


use strict;
no warnings "deprecated";
use encoding 'euc-jp';
use Test::More tests => 4;

my @a;

while (<DATA>) {
  chomp;
  tr/ぁ-んァ-ン/ァ-ンぁ-ん/;
  push @a, $_;
}

is(scalar @a, 3);
is($a[0], "コレハDATAふぁいるはんどるノてすとデス。");
is($a[1], "日本語ガチャント変換デキルカ");
is($a[2], "ドウカノてすとヲシテイマス。");

__DATA__
これはDATAファイルハンドルのテストです。
日本語がちゃんと変換できるか
どうかのテストをしています。
