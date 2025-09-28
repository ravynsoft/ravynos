# Testing accept_codes
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 13 };

#use Pod::Simple::Debug (6);

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
sub accept_N { $_[0]->accept_codes('N') }

print "# Some sanity tests...\n";
ok( $x->_out( "=pod\n\nI like pie.\n"), # without acceptor
  '<Document><Para>I like pie.</Para></Document>'
);
ok( $x->_out( \&accept_N, "=pod\n\nI like pie.\n"),
  '<Document><Para>I like pie.</Para></Document>'
);
ok( $x->_out( "=pod\n\nB<foo\t>\n"), # without acceptor
  '<Document><Para><B>foo </B></Para></Document>'
);
ok( $x->_out( \&accept_N,  "=pod\n\nB<foo\t>\n"),
  '<Document><Para><B>foo </B></Para></Document>'
);

print "# Some real tests...\n";

ok( $x->_out( \&accept_N,  "=pod\n\nN<foo\t>\n"),
  '<Document><Para><N>foo </N></Para></Document>'
);
ok( $x->_out( \&accept_N,  "=pod\n\nB<N<foo\t>>\n"),
  '<Document><Para><B><N>foo </N></B></Para></Document>'
);
ok( $x->_out( "=pod\n\nB<N<foo\t>>\n") # without the mutor
  ne '<Document><Para><B><N>foo </N></B></Para></Document>'
  # make sure it DOESN'T pass thru the N<...> when not accepted
);
ok( $x->_out( \&accept_N,  "=pod\n\nB<pieF<zorch>N<foo>I<pling>>\n"),
  '<Document><Para><B>pie<F>zorch</F><N>foo</N><I>pling</I></B></Para></Document>'
);

print "# Tests of nonacceptance...\n";

sub starts_with {
  my($large, $small) = @_;
  print("# supahstring is undef\n"),
   return '' unless defined $large;
  print("# supahstring $large is smaller than target-starter $small\n"),
   return '' if length($large) < length($small);
  if( substr($large, 0, length($small)) eq $small ) {
    #print "# Supahstring $large\n#  indeed starts with $small\n";
    return 1;
  } else {
    print "# Supahstring $large\n#  !starts w/ $small\n";
    return '';
  }
}


ok( starts_with( $x->_out( "=pod\n\nB<N<foo\t>>\n"), # without the mutor
  '<Document><Para><B>foo </B></Para>'
  # make sure it DOESN'T pass thru the N<...>, when not accepted
));

ok( starts_with( $x->_out( "=pod\n\nB<pieF<zorch>N<foo>I<pling>>\n"), # !mutor
  '<Document><Para><B>pie<F>zorch</F>foo<I>pling</I></B></Para>'
  # make sure it DOESN'T pass thru the N<...>, when not accepted
));

ok( starts_with( $x->_out( "=pod\n\nB<pieF<zorch>N<C<foo>>I<pling>>\n"), # !mutor
  '<Document><Para><B>pie<F>zorch</F><C>foo</C><I>pling</I></B></Para>'
  # make sure it DOESN'T pass thru the N<...>, when not accepted
));





print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

