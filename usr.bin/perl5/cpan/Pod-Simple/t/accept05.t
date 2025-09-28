# Testing extend and accept_codes
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 24 };

#use Pod::Simple::Debug (2);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}

my $x = 'Pod::Simple::XMLOutStream';
sub accept_Q    { $_[0]->accept_codes('Q') }
sub accept_prok { $_[0]->accept_codes('prok') }
sub accept_zing_prok { $_[0]->accept_codes('zing:prok') }
sub accept_zing_superprok { $_[0]->accept_codes('z.i_ng:Prok-12') }
sub accept_zing_superduperprok {
  $_[0]->accept_codes('A');
  $_[0]->accept_codes('z.i_ng:Prok-12');
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


print "# Some sanity tests...\n";
ok( $x->_out( "=pod\n\nI like pie.\n"),
  '<Document><Para>I like pie.</Para></Document>'
);
ok( $x->_out( "=extend N C Y,W\n\nI like pie.\n"),
  '<Document><Para>I like pie.</Para></Document>'
);
ok( $x->_out( "=extend N C,F Y,W\n\nI like pie.\n"),
  '<Document><Para>I like pie.</Para></Document>'
);
ok( $x->_out( "=extend N C,F,I Y,W\n\nI like pie.\n"),
  '<Document><Para>I like pie.</Para></Document>'
);


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


print "## OK, actually trying to use an extended code...\n";

print "# extending but not accepted (so hitting fallback)\n";

ok( $x->_out( "=extend N B Y,W\n\nI N<like> pie.\n"),
  '<Document><Para>I <B>like</B> pie.</Para></Document>'
);
ok( $x->_out( "=extend N B,I Y,W\n\nI N<like> pie.\n"),
  '<Document><Para>I <B><I>like</I></B> pie.</Para></Document>'
);
ok( $x->_out( "=extend N C,B,I Y,W\n\nI N<like> pie.\n"),
  '<Document><Para>I <C><B><I>like</I></B></C> pie.</Para></Document>'
);



print "# extending to one-letter accepted (not hitting fallback)\n";

ok( $x->_out( \&accept_Q, "=extend N B Y,Q,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <Q>like</Q> pie.</Para></Document>'
);
ok( $x->_out( \&accept_Q, "=extend N B,I Y,Q,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <Q>like</Q> pie.</Para></Document>'
);
ok( $x->_out( \&accept_Q, "=extend N C,B,I Y,Q,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <Q>like</Q> pie.</Para></Document>'
);



print "# extending to many-letter accepted (not hitting fallback)\n";

ok( $x->_out( \&accept_prok, "=extend N B Y,prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <prok>like</prok> pie.</Para></Document>'
);
ok( $x->_out( \&accept_prok, "=extend N B,I Y,prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <prok>like</prok> pie.</Para></Document>'
);
ok( $x->_out( \&accept_prok, "=extend N C,B,I Y,prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <prok>like</prok> pie.</Para></Document>'
);



print "# extending to :-containing, many-letter accepted (not hitting fallback)\n";

ok( $x->_out( \&accept_zing_prok, "=extend N B Y,zing:prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <zing:prok>like</zing:prok> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_prok, "=extend N B,I Y,zing:prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <zing:prok>like</zing:prok> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_prok, "=extend N C,B,I Y,zing:prok,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <zing:prok>like</zing:prok> pie.</Para></Document>'
);




print "# extending to _:-0-9-containing, many-letter accepted (not hitting fallback)\n";

ok( $x->_out( \&accept_zing_superprok, "=extend N B Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_superprok, "=extend N B,I Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_superprok, "=extend N C,B,I Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);



print "#\n# Testing acceptance order\n";

ok( $x->_out( \&accept_zing_superduperprok, "=extend N B Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_superduperprok, "=extend N B,I Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);
ok( $x->_out( \&accept_zing_superduperprok, "=extend N C,B,I Y,z.i_ng:Prok-12,A,bzroch\n\nI N<like> pie.\n"),
  '<Document><Para>I <z.i_ng:Prok-12>like</z.i_ng:Prok-12> pie.</Para></Document>'
);



print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

