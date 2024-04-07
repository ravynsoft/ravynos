#!perl -w
#
# This script is written intentionally in UTF-8
# -- dankogai

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_without_perlio();
}

use strict;
plan(tests => 8);
use utf8;

my @hiragana =  map {chr} ord("ぁ")..ord("ん");
my @katakana =  map {chr} ord("ァ")..ord("ン");
my $hiragana = join('' => @hiragana);
my $katakana = join('' => @katakana);
my %h2k; @h2k{@hiragana} = @katakana;
my %k2h; @k2h{@katakana} = @hiragana;

# print @hiragana, "\n";

my $str;

$str = $hiragana; $str =~ tr/ぁ-ん/ァ-ン/;
is($str, $katakana, "tr// # hiragana -> katakana");
$str = $katakana; $str =~ tr/ァ-ン/ぁ-ん/;
is($str, $hiragana, "tr// # hiragana -> katakana");

$str = $hiragana; eval qq(\$str =~ tr/ぁ-ん/ァ-ン/);
is($str, $katakana, "eval qq(tr//) # hiragana -> katakana");
$str = $katakana; eval qq(\$str =~ tr/ァ-ン/ぁ-ん/);
is($str, $hiragana, "eval qq(tr//) # hiragana -> katakana");

$str = $hiragana; $str =~ s/([ぁ-ん])/$h2k{$1}/go;
is($str, $katakana, "s/// # hiragana -> katakana");
$str = $katakana; $str =~ s/([ァ-ン])/$k2h{$1}/go;
is($str, $hiragana, "s/// # hiragana -> katakana");

{
  # [perl 16843]
  my $line = 'abcdefghijklmnopqrstuvwxyz$0123456789';
  $line =~ tr/bcdeghijklmnprstvwxyz$02578/בצדעגהיײקלמנפּרסטװשכיזשױתײח/;
  is($line, "aבצדעfגהיײקלמנoפqּרסuטװשכיזש1ױ34ת6ײח9", "[perl #16843]");
}

{
  # [perl #40641]
  my $str = qq/Gebääääääääääääääääääääude/;
  my $reg = qr/Gebääääääääääääääääääääude/;
  ok($str =~ /$reg/, "[perl #40641]");
}

__END__
