BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 62 };

my $d;
#use Pod::Simple::Debug (\$d, 0);


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
$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output


sub moj {$_[0]->accept_target('mojojojo')}
sub mojtext {$_[0]->accept_target_as_text('mojojojo')}
sub any {$_[0]->accept_target_as_text('*')}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "# Testing non-matching complaint...\n";
{

my $out;
ok( ($out = $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\nStuff\n\n=end blorp\n\nYup.\n"))
  =~ m/POD ERRORS/
) or print "# Didn't contain POD ERRORS:\n#  $out\n";

ok( ($out = $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :mojojojo\n\nStuff\n\n=end :blorp\n\nYup.\n"))
  =~ m/POD ERRORS/
) or print "# Didn't contain POD ERRORS:\n#  $out\n";
ok( ($out = $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :mojojojo\n\n=begin :zaz\n\nStuff\n\n=end :blorp\n\nYup.\n"))
  =~ m/POD ERRORS/
) or print "# Didn't contain POD ERRORS:\n#  $out\n";

}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


print "# Testing some trivial cases of non-acceptance...\n";

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\nStuff\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\n\nStuff\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :mojojojo\n\n\nStuff\n\n=end :mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\n  Stuff\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\n\n   Stuff\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :mojojojo\n\n\n   Stuff\n\n=end :mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<Stuff>\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin mojojojo\n\n\nI<Stuff>\n\n=end mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :mojojojo\n\n\nI<Stuff>\n\n=end :mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);



ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nStuff\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\n\nStuff\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\n\nStuff\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\n  Stuff\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\n\n   Stuff\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\n\n   Stuff\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<Stuff>\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\n\nI<Stuff>\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\n\nI<Stuff>\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><Para>Yup.</Para></Document>'
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# Testing matching because of negated non-acceptance...\n";
#$d = 5;
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !crunk\n\nstuff\n\n=end !crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin !crunk\n\nstuff\n\n=end !crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !mojojojo\n\nstuff\n\n=end !mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!mojojojo" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !mojojojo\n\nI<stuff>\n\n=end !mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!mojojojo" target_matching="!"><Data xml:space="preserve">I&#60;stuff&#62;</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !:mojojojo\n\nI<stuff>\n\n=end !:mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!:mojojojo" target_matching="!"><Para><I>stuff</I></Para></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin    :!mojojojo  \n\nI<stuff>\n\n=end  :!mojojojo \t \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target=":!mojojojo" target_matching="!"><Para><I>stuff</I></Para></for><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !crunk,zaz\n\nstuff\n\n=end !crunk,zaz\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk,zaz" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin !crunk\n\nstuff\n\n=end !crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin !crunk\n\nstuff\n\n=end !crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&any, "=pod\n\nI like pie.\n\n=begin !crunk\n\nstuff\n\n=end !crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!crunk" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !mojojojo\n\nstuff\n\n=end !mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!mojojojo" target_matching="!"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !mojojojo\n\nI<stuff>\n\n\n=end !mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!mojojojo" target_matching="!"><Data xml:space="preserve">I&#60;stuff&#62;</Data></for><Para>Yup.</Para></Document>'
);

ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !psketti,mojojojo,crunk\n\n\nI<stuff>\n\n=end !psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!psketti,mojojojo,crunk" target_matching="!"><Data xml:space="preserve">I&#60;stuff&#62;</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( "=pod\n\nI like pie.\n\n=begin !:psketti,mojojojo,crunk\n\nI<stuff>\n\n=end !:psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="!:psketti,mojojojo,crunk" target_matching="!"><Para><I>stuff</I></Para></for><Para>Yup.</Para></Document>'
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# Testing accept_target + simple ...\n";
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\n\n=end mojojojo \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\n=end psketti,mojojojo,crunk \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data></for><Para>Yup.</Para></Document>'
);

print "# Testing accept_target_as_text + simple ...\n";
ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\n\n=end  mojojojo \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Para><I>stuff</I></Para></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\n=end  psketti,mojojojo,crunk \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Para><I>stuff</I></Para></for><Para>Yup.</Para></Document>'
);

print "# Testing accept_target + two simples ...\n";
#$d = 10;
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\n\nHm, B<things>!\n\n=end mojojojo\n\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">Hm, B&#60;things&#62;!</Data></for><Para>Yup.</Para></Document>'
);

ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\nHm, B<things>!\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">Hm, B&#60;things&#62;!</Data></for><Para>Yup.</Para></Document>'
);

ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :mojojojo\n\nI<stuff>\n\nHm, B<things>!\n\n=end :mojojojo\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target=":mojojojo" target_matching="mojojojo"><Para><I>stuff</I></Para><Para>Hm, <B>things</B>!</Para></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\nI<stuff>\n\nHm, B<things>!\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target=":psketti,mojojojo,crunk" target_matching="mojojojo"><Para><I>stuff</I></Para><Para>Hm, <B>things</B>!</Para></for><Para>Yup.</Para></Document>'
);

print "# Testing accept_target_as_text + two simples ...\n";

ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\nHm, B<things>!\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Para><I>stuff</I></Para><Para>Hm, <B>things</B>!</Para></for><Para>Yup.</Para></Document>'
);
ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\nI<stuff>\n\nHm, B<things>!\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target=":psketti,mojojojo,crunk" target_matching="mojojojo"><Para><I>stuff</I></Para><Para>Hm, <B>things</B>!</Para></for><Para>Yup.</Para></Document>'
);



print "# Testing accept_target + two simples, latter with leading whitespace ...\n";
#$d = 10;

ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\n\n   Hm, B<things>!\nTrala.\n\n=end mojojojo\n\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n</Data></for><Para>Yup.</Para></Document>}
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\n   Hm, B<things>!\nTrala.\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n</Data></for><Para>Yup.</Para></Document>}
);

ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\n\n   Hm, B<things>!\nTrala.\n\n\n=end mojojojo\n\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n</Data></for><Para>Yup.</Para></Document>}
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk\n\nI<stuff>\n\n   Hm, B<things>!\nTrala.\n\n\n=end psketti,mojojojo,crunk\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target="psketti,mojojojo,crunk" target_matching="mojojojo"><Data xml:space="preserve">I&#60;stuff&#62;</Data><Data xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n</Data></for><Para>Yup.</Para></Document>}
);


print "# Testing :-target and accept_target + two simples, latter with leading whitespace ...\n";

ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :mojojojo\n\nI<stuff>\nTrala!\n\n   Hm, B<things>!\nTrala.\n\n=end :mojojojo\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target=":mojojojo" target_matching="mojojojo"><Para><I>stuff</I> Trala!</Para><Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.</Verbatim></for><Para>Yup.</Para></Document>}
);
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin :psketti,mojojojo,crunk\n\nI<stuff>\nTrala!\n\n   Hm, B<things>!\nTrala.\n\n=end :psketti,mojojojo,crunk\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target=":psketti,mojojojo,crunk" target_matching="mojojojo"><Para><I>stuff</I> Trala!</Para><Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.</Verbatim></for><Para>Yup.</Para></Document>}
);

print "#   now with accept_target_as_text\n";
ok( $x->_out( \&mojtext, "=pod\n\nI like pie.\n\n=begin mojojojo\n\nI<stuff>\nTrala!\n\n   Hm, B<things>!\nTrala.\n\n=end mojojojo\n\nYup.\n"),
  qq{<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo"><Para><I>stuff</I> Trala!</Para><Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.</Verbatim></for><Para>Yup.</Para></Document>}
);
ok( $x->_out( \&mojtext,  join "\n\n" =>
  "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk",
  "I<stuff>\nTrala!",
  "   Hm, B<things>!\nTrala.",
  "=end psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target="psketti,mojojojo,crunk" target_matching="mojojojo">}.
 qq{<Para><I>stuff</I> Trala!</Para>}.
 qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.</Verbatim>}.
 qq{</for><Para>Yup.</Para></Document>}
);

print "# Now with five paragraphs (p,v,v,p,p) and accept_target_as_text\n";

ok( $x->_out( \&mojtext,  join "\n\n" =>
  "=pod\n\nI like pie.\n\n=begin psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "Woohah S<thwack\nwoohah>squim!",
  "=end psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target="psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<Para>Woohah <S>thwack woohah</S>squim!</Para>}.
 qq{</for><Para>Yup.</Para></Document>}
);



print "#\n# Now nested begin...end regions...\n";

sub mojprok { shift->accept_targets(qw{mojojojo prok}) }

ok( $x->_out( \&mojprok,  join "\n\n" =>
  "=pod\n\nI like pie.",
  "=begin :psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "=begin :prok",
      "Woohah S<thwack\nwoohah>squim!",
    "=end :prok",
    "ZubZ<>aaz.",
  "=end :psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target=":psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<for target=":prok" target_matching="prok">}.
     qq{<Para>Woohah <S>thwack woohah</S>squim!</Para>}.
   qq{</for>}.
   qq{<Para>Zubaaz.</Para>}.
 qq{</for>}.
 qq{<Para>Yup.</Para></Document>}
);


print "# a little more complex this time...\n";

ok( $x->_out( \&mojprok,  join "\n\n" =>
  "=pod\n\nI like pie.",
  "=begin :psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "=begin :prok",
      "   Blorp, B<things>!\nTrala.",
      "    Khh, F<< dodads >>!\nHurf.",
      "Woohah S<thwack\nwoohah>squim!",
    "=end :prok",
    "ZubZ<>aaz.",
  "=end :psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target=":psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<for target=":prok" target_matching="prok">}.
     qq{<Verbatim xml:space="preserve">   Blorp, B&#60;things&#62;!\nTrala.\n\n}.
     qq{    Khh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
     qq{<Para>Woohah <S>thwack woohah</S>squim!</Para>}.
   qq{</for>}.
   qq{<Para>Zubaaz.</Para>}.
 qq{</for>}.
 qq{<Para>Yup.</Para></Document>}
);


$d = 10;
print "# Now with nesting where inner region is non-resolving...\n";

ok( $x->_out( \&mojprok,  join "\n\n" =>
  "=pod\n\nI like pie.",
  "=begin :psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "=begin prok",
      "   Blorp, B<things>!\nTrala.",
      "    Khh, F<< dodads >>!\nHurf.",
      "Woohah S<thwack\nwoohah>squim!",
    "=end prok",
    "ZubZ<>aaz.",
  "=end :psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target=":psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<for target="prok" target_matching="prok">}.
     qq{<Data xml:space="preserve">   Blorp, B&#60;things&#62;!\nTrala.\n\n}.
     qq{    Khh, F&#60;&#60; dodads &#62;&#62;!\nHurf.\n</Data>}.
     qq{<Data xml:space="preserve">Woohah S&#60;thwack\nwoohah&#62;squim!</Data>}.
   qq{</for>}.
   qq{<Para>Zubaaz.</Para>}.
 qq{</for>}.
 qq{<Para>Yup.</Para></Document>}
);



print "# Now a begin...end with a non-resolving for inside\n";

ok( $x->_out( \&mojprok,  join "\n\n" =>
  "=pod\n\nI like pie.",
  "=begin :psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "=for prok"
     . "   Blorp, B<things>!\nTrala.\n    Khh, F<< dodads >>!\nHurf.",
    "ZubZ<>aaz.",
  "=end :psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target=":psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<for target="prok" target_matching="prok">}.
     qq{<Data xml:space="preserve">Blorp, B&#60;things&#62;!\nTrala.\n}.
     qq{    Khh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Data>}.
   qq{</for>}.
   qq{<Para>Zubaaz.</Para>}.
 qq{</for>}.
 qq{<Para>Yup.</Para></Document>}
);




print "# Now a begin...end with a resolving for inside\n";

ok( $x->_out( \&mojprok,  join "\n\n" =>
  "=pod\n\nI like pie.",
  "=begin :psketti,mojojojo,crunk",
    "I<stuff>\nTrala!",
    "   Hm, B<things>!\nTrala.",
    "    Oh, F<< dodads >>!\nHurf.",
    "Boing C<spr-\t\n\t\t\toink>\n Blorg!",
    "=for :prok"
     . "   Blorp, B<things>!\nTrala.\n    Khh, F<< dodads >>!\nHurf.",
    "ZubZ<>aaz.",
  "=end :psketti,mojojojo,crunk",
  "Yup.\n"
 ),
 qq{<Document><Para>I like pie.</Para>}.
 qq{<for target=":psketti,mojojojo,crunk" target_matching="mojojojo">}.
   qq{<Para><I>stuff</I> Trala!</Para>}.
   qq{<Verbatim xml:space="preserve">   Hm, B&#60;things&#62;!\nTrala.\n\n}.
   qq{    Oh, F&#60;&#60; dodads &#62;&#62;!\nHurf.</Verbatim>}.
   qq{<Para>Boing <C>spr- oink</C> Blorg!</Para>}.
   qq{<for target=":prok" target_matching="prok">}.
     qq{<Para>Blorp, <B>things</B>! Trala. Khh, }.
     qq{<F>dodads</F>! Hurf.</Para>}.
   qq{</for>}.
   qq{<Para>Zubaaz.</Para>}.
 qq{</for>}.
 qq{<Para>Yup.</Para></Document>}
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# Testing matching of begin block titles\n";
ok( $x->_out( \&moj, "=pod\n\nI like pie.\n\n=begin mojojojo Title\n\nstuff\n\n=end mojojojo \n\nYup.\n"),
  '<Document><Para>I like pie.</Para><for target="mojojojo" target_matching="mojojojo" title="Title"><Data xml:space="preserve">stuff</Data></for><Para>Yup.</Para></Document>'
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

