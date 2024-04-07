BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 31 };

#use Pod::Simple::Debug (6);

ok 1;

require Pod::Simple::BlackBox;
ok 1;

require Pod::Simple; ok 1;

Pod::Simple->VERSION(.90); ok 1;

#print "# Pod::Simple version $Pod::Simple::VERSION\n";

require Pod::Simple::DumpAsXML; ok 1;

require Pod::Simple::XMLOutStream; ok 1;

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}

print "# Simple identity tests...\n";

&ok( e "", "" );
&ok( e "\n", "", );
&ok( e "\n", "\n", );
&ok( e "puppies\n\n\n\n", "", );


print "# Contentful identity tests...\n";

&ok( e "=pod\n\nFoo\n",         "=pod\n\nFoo\n"         );
&ok( e "=pod\n\n\n\nFoo\n\n\n", "=pod\n\n\n\nFoo\n\n\n" );
&ok( e "=pod\n\n\n\nFoo\n\n\n", "=pod\n\nFoo\n"         );

# Now with some more newlines
&ok( e "\n\n=pod\n\nFoo\n",     "\n\n=pod\n\nFoo\n"     );
&ok( e "=pod\n\n\n\nFoo\n\n\n", "=pod\n\n\n\nFoo\n\n\n" );
&ok( e "=pod\n\n\n\nFoo\n\n\n", "\n\n=pod\n\nFoo\n"     );


&ok( e "=head1 Foo\n",          "=head1 Foo\n"          );
&ok( e "=head1 Foo\n\n=cut\n",  "=head1 Foo\n\n=cut\n"  );
&ok( e "=head1 Foo\n\n=cut\n",  "=head1 Foo\n"          );

# Now just add some newlines...
&ok( e "\n\n\n\n=head1 Foo\n",  "\n\n\n\n=head1 Foo\n"  );
&ok( e "=head1 Foo\n\n=cut\n",  "=head1 Foo\n\n=cut\n"  );
&ok( e "=head1 Foo\n\n=cut\n",  "\n\n\n\n=head1 Foo\n"  );


print "# Simple XMLification tests...\n";

ok( Pod::Simple::XMLOutStream->_out("\n\n\nprint \$^T;\n\n\n"),
    qq{<Document\ncontentless="1"></Document>}
     # make sure the contentless flag is set
);
ok( Pod::Simple::XMLOutStream->_out("\n\n"),
    qq{<Document\ncontentless="1"></Document>}
     # make sure the contentless flag is set
);
ok( Pod::Simple::XMLOutStream->_out("\n"),
    qq{<Document\ncontentless="1"></Document>}
     # make sure the contentless flag is set
);
ok( Pod::Simple::XMLOutStream->_out(""),
    qq{<Document\ncontentless="1"></Document>}
     # make sure the contentless flag is set
);

ok( Pod::Simple::XMLOutStream->_out('', '<Document></Document>' ) );

ok( Pod::Simple::XMLOutStream->_out("=pod\n\nFoo\n"),
    '<Document><Para>Foo</Para></Document>'
);

ok( Pod::Simple::XMLOutStream->_out("=head1 Chacha\n\nFoo\n"),
    '<Document><head1>Chacha</head1><Para>Foo</Para></Document>'
);

# Make sure an obviously invalid Pod tag is invalid.
ok( Pod::Simple::XMLOutStream->_out("=F\0blah\n\nwhatever\n"),
    qq{<Document\ncontentless="1"></Document>}
);

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


