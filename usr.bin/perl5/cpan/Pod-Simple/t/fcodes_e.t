# fcodes E
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 20 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}
#use Pod::Simple::Debug (6);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;

print "# Pod::Simple version $Pod::Simple::VERSION\n";

print "# Pod::Escapes version $Pod::Escapes::VERSION\n",
 if $Pod::Escapes::VERSION;
# Presumably that's the library being used

&ok( e "", "" );
&ok( e "\n", "", );


print "# Testing some basic mnemonic E sequences...\n";

&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<lt>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1<2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<gt>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1>2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<verbar>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1|2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<sol>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1/2\n")
);


print "# Testing some more mnemonic E sequences...\n";

&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<apos>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1'2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<quot>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1\"2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1&2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<amp>2\n")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<eacute>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<233>2\n")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<infin>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<8734>2\n")
);

&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<lchevron>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<171>2\n")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<rchevron>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<187>2\n")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<laquo>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<171>2\n")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<raquo>2"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1E<187>2\n")
);



print "# Testing numeric E sequences...\n";
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<0101>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1A2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<65>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1A2")
);
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\n1E<0x41>2\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\n1A2")
);



print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


