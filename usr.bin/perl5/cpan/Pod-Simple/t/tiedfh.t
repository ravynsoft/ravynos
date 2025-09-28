# Testing tied output filehandle
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 8 };

use Pod::Simple::TiedOutFH;
ok 1;

print "# Sanity test of Perl and Pod::Simple::TiedOutFH\n";

{
  my $x = 'abc';
  my $out = Pod::Simple::TiedOutFH->handle_on($x);
  print $out "Puppies\n";
  print $out "rrrrr";
  print $out "uffuff!";
  ok $x, "abcPuppies\nrrrrruffuff!";
  undef $out;
  ok $x, "abcPuppies\nrrrrruffuff!";
}

# Now test that we can have two different strings.
{
  my $x1 = 'abc';
  my $x2 = 'xyz';
  my $out1 = Pod::Simple::TiedOutFH->handle_on($x1);
  my $out2 = Pod::Simple::TiedOutFH->handle_on($x2);

  print $out1 "Puppies\n";
  print $out2 "Kitties\n";
  print $out2 "mmmmm";
  print $out1 "rrrrr";
  print $out2 "iaooowwlllllllrrr!\n";
  print $out1 "uffuff!";

  ok $x1, "abcPuppies\nrrrrruffuff!",              "out1 test";
  ok $x2, "xyzKitties\nmmmmmiaooowwlllllllrrr!\n", "out2 test";

  undef $out1;
  undef $out2;

  ok $x1, "abcPuppies\nrrrrruffuff!",              "out1 test";
  ok $x2, "xyzKitties\nmmmmmiaooowwlllllllrrr!\n", "out2 test";
}


print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


