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

#use Pod::Simple::Debug (5);
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

print "# With weird leading whitespace...\n";
# With weird whitespace
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nI<foo>\n"),
 '<Document><Para><I>foo</I></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB< foo>\n"),
 '<Document><Para><B> foo</B></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB<\tfoo>\n"),
 '<Document><Para><B> foo</B></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB<\nfoo>\n"),
 '<Document><Para><B> foo</B></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB<foo>\n"),
 '<Document><Para><B>foo</B></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB<foo\t>\n"),
 '<Document><Para><B>foo </B></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nB<foo\n>\n"),
 '<Document><Para><B>foo </B></Para></Document>'
);


print "#\n# Tests for wedges outside of formatting codes...\n";
&ok( Pod::Simple::XMLOutStream->_out("=pod\n\nX < 3 and N > 19\n"),
     Pod::Simple::XMLOutStream->_out("=pod\n\nX E<lt> 3 and N E<gt> 19\n")
);


print "# A complex test with internal whitespace...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nI<foo>B< bar>C<baz >F< quux\t?>\n"),
 '<Document><Para><I>foo</I><B> bar</B><C>baz </C><F> quux ?</F></Para></Document>'
);


print "# Without any nesting...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nF<a>C<b>I<c>B<d>X<e>\n"),
 '<Document><Para><F>a</F><C>b</C><I>c</I><B>d</B><X>e</X></Para></Document>'
);

print "# Without any nesting, but with Z's...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nZ<>F<a>C<b>I<c>B<d>X<e>\n"),
 '<Document><Para><F>a</F><C>b</C><I>c</I><B>d</B><X>e</X></Para></Document>'
);


print "# With lots of nesting, and Z's...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nZ<>F<C<Z<>foo> I<bar>> B<X<thingZ<>>baz>\n"),
 '<Document><Para><F><C>foo</C> <I>bar</I></F> <B><X>thing</X>baz</B></Para></Document>'
);



print "#\n# *** Now testing different numbers of wedges ***\n";
print "# Without any nesting...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nF<< a >>C<<< b >>>I<<<< c >>>>B<< d >>X<< e >>\n"),
 '<Document><Para><F>a</F><C>b</C><I>c</I><B>d</B><X>e</X></Para></Document>'
);

print "# Without any nesting, but with Z's, and odder whitespace...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nF<< aZ<> >>C<<< Z<>b >>>I<<<< c  >>>>B<< d \t >>X<<\ne >>\n"),
  '<Document><Para><F>a</F><C>b</C><I>c</I><B>d</B><X>e</X></Para></Document>'
);

print "# With nesting and Z's, and odder whitespace...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nF<< aZ<> >>C<<< Z<>bZ<>B<< d \t >>X<<\ne >> >>>I<<<< c  >>>>\n"),
 "<Document><Para><F>a</F><C>b<B>d</B><X>e</X></C><I>c</I></Para></Document>"
);

print "# Regression https://rt.cpan.org/Ticket/Display.html?id=55602 (vs 12239)\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nC<<< foo->bar >>>\n"),
 '<Document><Para><C>foo-&#62;bar</C></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nC<<< C<foo> >>>\n"),
 '<Document><Para><C><C>foo</C></C></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nC<<< C<<foo>> >>>\n"),
 '<Document><Para><C><C>&#60;foo</C>&#62;</C></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nC<<< CZ<><<foo>> >>>\n"),
 '<Document><Para><C>C&#60;&#60;foo&#62;&#62;</C></Para></Document>'
);
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nC<<< CE<lt><foo>> >>>\n"),
 '<Document><Para><C>C&#60;&#60;foo&#62;&#62;</C></Para></Document>'
);

print "# Misc...\n";
ok( Pod::Simple::XMLOutStream->_out(
 "=pod\n\nI like I<PIE> with B<cream> and Stuff and N < 3 and X<< things >> hoohah\n"
."And I<pie is B<also> a happy time>.\n"
."And B<I<<< I like pie >>>.>\n"
) =>
"<Document><Para>I like <I>PIE</I> with <B>cream</B> and Stuff and N &#60; 3 and <X>things</X> hoohah "
."And <I>pie is <B>also</B> a happy time</I>. "
."And <B><I>I like pie</I>.</B></Para></Document>"
);





print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


