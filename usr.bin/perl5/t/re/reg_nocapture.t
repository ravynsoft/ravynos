#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

plan tests => 25;

# Some /qr/ tests
my $re = qr/(.*) b c d/;
ok("a b c d" =~ /$re/n, "/n still matches");
is($1, "a", "Outer /n doesn't affect inner qr//");

$re = qr/(.*) b c d/n;
ok("a b c d" =~ /$re/, "qr//n matches");
is($1, undef, "qr//n prevents capturing");

ok("a b c d" =~ $re, "qr// out of // matches");
is($1, undef, "qr//n prevents capturing");

# Some // tests
ok("a b c d" =~ /(a) b c d/n, "//n matches");
is($1, undef, "/n prevents capture");

ok("a b c d" =~ /(a) (b) c d/n, "//n matches with multiple ()");
is($1, undef, "/n prevents capture in \$1");
is($2, undef, "/n prevents capture in \$2");

# ?n
ok("a b c d" =~ /(?n:a) b c (d)/, "?n matches");
is($1, 'd', "?n: blocked capture");

# ?-n:()
ok("a b c d" =~ /(?-n:(a)) b c (d)/n, "?-n matches");
is($1, 'a', "?-n:() disabled nocapture");

ok("a b c d" =~ /(?<a>.) (?<b>.) (.*)/n, "named capture...");
is($1, 'a', "named capture allows $1 with /n");
is($2, 'b', "named capture allows $2 with /n");
is($3, undef, "(.*) didn't capture with /n");

SKIP: {
    skip "no %+ under miniperl", 2 if is_miniperl();
    no strict 'refs';
    is(${"+"}{a}, 'a', "\$+{a} is correct");
    is(${"+"}{b}, 'b', "\$+{b} is correct");
}

is(qr/(what)/n,     '(?^n:(what))',
  'qr//n stringified is correct');

is(qr/(?n:what)/,   '(?^:(?n:what))',
  'qr/(?n:...)/ stringified is correct');

is(qr/(?-n:what)/,  '(?^:(?-n:what))',
  'qr/(?-n:...)/ stringified is correct');

is(qr/(?-n:what)/n, '(?^n:(?-n:what))',
  'qr/(?-n:...)/n stringified is correct');

