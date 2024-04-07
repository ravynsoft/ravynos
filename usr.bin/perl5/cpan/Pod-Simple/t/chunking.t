BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

#use Pod::Simple::Debug (2);

use strict;
use warnings;
use Test;
BEGIN { plan tests => 11 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

ok( Pod::Simple::XMLOutStream->_out("=head1 =head1"),
    '<Document><head1>=head1</head1></Document>'
);

ok( Pod::Simple::XMLOutStream->_out("\n=head1 =head1"),
    '<Document><head1>=head1</head1></Document>'
);

ok( Pod::Simple::XMLOutStream->_out("\n=head1 =head1\n"),
    '<Document><head1>=head1</head1></Document>'
);

ok( Pod::Simple::XMLOutStream->_out("\n=head1 =head1\n\n"),
    '<Document><head1>=head1</head1></Document>'
);

&ok(e "\n=head1 =head1\n\n" , "\n=head1 =head1\n\n");

&ok(e "\n=head1\n=head1\n\n", "\n=head1 =head1\n\n");

&ok(e "\n=pod\n\nCha cha cha\n\n" , "\n=pod\n\nCha cha cha\n\n");
&ok(e "\n=pod\n\nCha\tcha  cha\n\n" , "\n=pod\n\nCha cha cha\n\n");
&ok(e "\n=pod\n\nCha\ncha  cha\n\n" , "\n=pod\n\nCha cha cha\n\n");

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

