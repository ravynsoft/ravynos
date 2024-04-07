BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 23 };

#use Pod::Simple::Debug (6);

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


print "# Simple tests for head1 - head6...\n";
ok( Pod::Simple::XMLOutStream->_out("\n=head1 Chacha\n\n"),
    '<Document><head1>Chacha</head1></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("\n=head2 Chacha\n\n"),
    '<Document><head2>Chacha</head2></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("\n=head3 Chacha\n\n"),
    '<Document><head3>Chacha</head3></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("\n=head4 Chacha\n\n"),
    '<Document><head4>Chacha</head4></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("\n=head5 Chacha\n\n"),
    '<Document><head5>Chacha</head5></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("\n=head6 Chacha\n\n"),
    '<Document><head6>Chacha</head6></Document>'
);

print "# Testing whitespace equivalence...\n";

&ok(e "\n=head1 Chacha\n\n", "\n=head1       Chacha\n\n");
&ok(e "\n=head1 Chacha\n\n", "\n=head1\tChacha\n\n");
&ok(e "\n=head1 Chacha\n\n", "\n=head1\tChacha      \n\n");



ok( Pod::Simple::XMLOutStream->_out("=head1     Chachacha"),
    '<Document><head1>Chachacha</head1></Document>'
);


print "# Testing whitespace variance ...\n";
ok( Pod::Simple::XMLOutStream->_out("=head1     Cha cha cha   \n"),
    '<Document><head1>Cha cha cha</head1></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=head1     Cha   cha\tcha   \n"),
    '<Document><head1>Cha cha cha</head1></Document>'
);




print "# Testing head2 ... head6 more...\n";

ok( Pod::Simple::XMLOutStream->_out("=head2     Cha   cha\tcha   \n"),
    '<Document><head2>Cha cha cha</head2></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=head3     Cha   cha\tcha   \n"),
    '<Document><head3>Cha cha cha</head3></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=head4     Cha   cha\tcha   \n"),
    '<Document><head4>Cha cha cha</head4></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=head5     Cha   cha\tcha   \n"),
    '<Document><head5>Cha cha cha</head5></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=head6     Cha   cha\tcha   \n"),
    '<Document><head6>Cha cha cha</head6></Document>'
);

print "# Testing entity expansion...\n";

ok( Pod::Simple::XMLOutStream->_out("=head4 fooE<64>bar!\n"),
    Pod::Simple::XMLOutStream->_out("\n=head4  foo\@bar!\n\n"),
);

# TODO: a mode so that DumpAsXML can ask for all contiguous string
#  sequences to be fused?
# &ok( e "=head4 fooE<64>bar!\n", "\n=head4  foo\@bar!\n\n");

print "# Testing formatting sequences...\n";

# True only if the sequences resolve, as they should...
&ok( e "=head4 C<foobar!>\n", "\n=head4 C<< foobar!    >>\n\n");
&ok( e "=head4 C<foobar!>\n", "\n\n=head4 C<<<  foobar! >>>\n");
&ok( e "=head4 C<foobar!>\n", "\n=head4 C<< foobar!\n\t>>\n\n");

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

