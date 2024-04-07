# Testing verbatim formatted sections
use strict;
use warnings;
use Test;
BEGIN { plan tests => 62 };

#use Pod::Simple::Debug (6);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;

print "# Pod::Simple version $Pod::Simple::VERSION\n";

sub e  { Pod::Simple::DumpAsXML->_duo(\&without_vf, @_) }
sub ev { Pod::Simple::DumpAsXML->_duo(\&with_vf,    @_) }

sub with_vf    { $_[0]->  accept_codes('VerbatimFormatted') }
sub without_vf { $_[0]->unaccept_codes('VerbatimFormatted') }

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

print "# Testing VerbatimFormatted...\n";
    # A formatty line has to have #: in the first two columns, and uses
    # "^" to mean bold, "/" to mean underline, and "%" to mean bold italic.
    # Example:
    #   What do you want?  i like pie. [or whatever]
    # #:^^^^^^^^^^^^^^^^^              /////////////         


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              /////////////         
  Hooboy.

=cut

}) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or whatever]</VerbatimI>\n  Hooboy.</VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              /////////////
  Hooboy.

=cut

}) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or whatever]</VerbatimI>\n  Hooboy.</VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              /////////////

=cut

}) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or whatever]</VerbatimI></VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              /////////////}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or whatever]</VerbatimI></VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              //////////////////}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or whatever]</VerbatimI></VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              ///}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or</VerbatimI> whatever]</VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^              ///
#:^^^^^^^^^^^^^^^^^              ///}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or</VerbatimI> whatever]\n#:^^^^^^^^^^^^^^^^^              ///</VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
# with a tab:
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^		 /// }
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i like pie. <VerbatimI>[or</VerbatimI> whatever]</VerbatimFormatted></Document>}
);



# Now testing the % too:
&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

  What do you want?  i like pie. [or whatever]
#:^^^^^^^^^^^^^^^^^    %%%%      //////////////////}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">  <VerbatimB>What do you want?</VerbatimB>  i <VerbatimBI>like</VerbatimBI> pie. <VerbatimI>[or whatever]</VerbatimI></VerbatimFormatted></Document>}
);


&ok( Pod::Simple::XMLOutStream->_out(\&with_vf,
q{=pod

   Hooboy!
  What do you want?  i like pie. [or whatever]
#:	      ^^^^^    %%%%      //////////////////}
) => qq{<Document><VerbatimFormatted\nxml:space="preserve">   Hooboy!\n  What do you <VerbatimB>want?</VerbatimB>  i <VerbatimBI>like</VerbatimBI> pie. <VerbatimI>[or whatever]</VerbatimI></VerbatimFormatted></Document>}
);



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~





# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

print "# Now running some tests adapted from verbatims.t...\n#\n#\n";

print "# Without VerbatimFormatted...\n";
&ok(  e "", "" );
&ok(  e "\n", "", );
&ok(  e "\n=pod\n\n foo bar baz", "\n=pod\n\n foo bar baz" );
&ok(  e "\n=pod\n\n foo bar baz", "\n=pod\n\n foo bar baz\n" );
print "# With VerbatimFormatted...\n";
&ok( ev "", "" );
&ok( ev "\n", "", );
&ok( ev "\n=pod\n\n foo bar baz", "\n=pod\n\n foo bar baz" );
&ok( ev "\n=pod\n\n foo bar baz", "\n=pod\n\n foo bar baz\n" );


print "# Now testing via XMLOutStream without VerbatimFormatted...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz</Verbatim></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n quux\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz\n quux</Verbatim></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\nquux\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz\nquux</Verbatim></Document>}
);

print "# Contiguous verbatims...\n";
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n quux\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz\n\n quux</Verbatim></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n\n quux\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz\n\n\n quux</Verbatim></Document>}
);

print "# Testing =cut...\n";
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\n quux\n"),
  qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz</Verbatim></Document>}
);




print "#\n# Now retesting with VerbatimFormatted...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz</VerbatimFormatted></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n quux\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz\n quux</VerbatimFormatted></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\nquux\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz\nquux</VerbatimFormatted></Document>}
);

print "# Contiguous verbatims...\n";
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n quux\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz\n\n quux</VerbatimFormatted></Document>}
);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n\n quux\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz\n\n\n quux</VerbatimFormatted></Document>}
);

print "# Testing =cut...\n";
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\n quux\n"),
  qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz</VerbatimFormatted></Document>}
);




# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

{
my $it =
qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz</Verbatim><head1>Foo</head1><Verbatim\nxml:space="preserve"> quux\nquum</Verbatim></Document>}
;


print "# Various \\n-(in)significance sanity checks...\n";

print "#  verbatim/cut/head/verbatim sanity zero...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity one...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity two...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\n\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity three...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity four...\n";

ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n\n=cut\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&without_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);

}


# : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : :

print "#\n# Now retesting with VerbatimFormatted...\n";

{
my $it =
qq{<Document><VerbatimFormatted\nxml:space="preserve"> foo bar baz</VerbatimFormatted><head1>Foo</head1><VerbatimFormatted\nxml:space="preserve"> quux\nquum</VerbatimFormatted></Document>}
;


print "# Various \\n-(in)significance sanity checks...\n";

print "#  verbatim/cut/head/verbatim sanity zero...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity one...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\nsome code here...\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity two...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\n\nsome code here...\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity three...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);

print "#  verbatim/cut/head/verbatim sanity four...\n";

ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n\n=cut\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);
ok( Pod::Simple::XMLOutStream->_out(\&with_vf, "\n=pod\n\n foo bar baz\n\n\n\n\n\n=cut\n\nsome code here...\n\n\n=head1 Foo\n\n quux\nquum\n"), $it);

}



print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


