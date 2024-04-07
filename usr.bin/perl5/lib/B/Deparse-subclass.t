#!./perl

# These tests are not necessarily normative, but until such time as we
# publicise an API for subclassing B::Deparse they can prevent us from
# gratuitously breaking conventions that CPAN modules already use.

use Test::More;

use B::Deparse;

package B::Deparse::NameMangler {
  @ISA = "B::Deparse";
  sub padname { SUPER::padname{@_} . '_groovy' }
}

my $nm = 'B::Deparse::NameMangler'->new;

like  $nm->coderef2text(sub { my($a, $b, $c) }),
      qr/\$a_groovy, \$b_groovy, \$c_groovy/,
     'overriding padname works for renaming lexicals';

like  $nm->coderef2text(sub { my $c; /(??{ $c })/; }),
      qr/\Q(??{\E \$c_groovy/,
     'overriding padname works for renaming lexicals in regexp blocks';

done_testing();
