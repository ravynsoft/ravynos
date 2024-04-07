use strict;
use Test::More 'no_plan';
use Encode ':fallbacks';

my $str = "\x{0647}";
my @data = grep length, map { chomp; $_ } <DATA>;

while (my($in, $out) = splice(@data, 0, 2)) {
    my $copy = $str;
    is Encode::encode("iso-2022-jp", $copy, eval $in), $out;
}

__DATA__
FB_PERLQQ
\x{0647}

FB_HTMLCREF
&#1607;

FB_XMLCREF
&#x647;
