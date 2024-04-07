

use strict;
use warnings;
use Test;
BEGIN { plan tests => 11 };

my $d;
#use Pod::Simple::Debug (\$d, 0);

ok 1;

use Pod::Simple::XMLOutStream;
use Pod::Simple::DumpAsXML;


$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output


print "# A simple sanity test...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
 '<Document><Para><F><C>foo</C> <I>bar</I></F> <B>stuff <X>thing</X>baz</B></Para></Document>'
);

print "# With lots of nesting, and Z's...\n";
ok( Pod::Simple::XMLOutStream->_out("=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
 '<Document><Para><F><C>foo</C> <I>bar</I></F> <B>stuff <X>thing</X>baz</B></Para></Document>'
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub mergy {$_[0]->merge_text(1)}
sub nixy  {$_[0]->nix_X_codes(1)}
sub nixy_mergy {$_[0]->merge_text(1); $_[0]->nix_X_codes(1);}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# With no F/X\n";

ok( Pod::Simple::DumpAsXML->_out( "=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
  join "\n",

  '<Document>',
  '  <Para>',
  '    <F>',
  '      <C>',
  '        foo',
  '      </C>',
  '      ',
  '      <I>',
  '        bar',
  '      </I>',
  '    </F>',
  '    ',
  '    <B>',
  '      stuff ',
  '      <X>',
  '        thing',
  '      </X>',
  '      baz',
  '    </B>',
  '  </Para>',
  '</Document>',
  '',
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "#  with just X-nixing...\n";

ok( Pod::Simple::DumpAsXML->_out( \&nixy, "=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
  join "\n",

  '<Document>',
  '  <Para>',
  '    <F>',
  '      <C>',
  '        foo',
  '      </C>',
  '      ',
  '      <I>',
  '        bar',
  '      </I>',
  '    </F>',
  '    ',
  '    <B>',
  '      stuff baz',
  '    </B>',
  '  </Para>',
  '</Document>',
  '',
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# With merging...\n";

ok( Pod::Simple::DumpAsXML->_out( \&mergy, "=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
  join "\n",

  '<Document>',
  '  <Para>',
  '    <F>',
  '      <C>',
  '        foo',
  '      </C>',
  '      ',
  '      <I>',
  '        bar',
  '      </I>',
  '    </F>',
  '    ',
  '    <B>',
  '      stuff ',
  '      <X>',
  '        thing',
  '      </X>',
  '      baz',
  '    </B>',
  '  </Para>',
  '</Document>',
  '',
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "# With nixing and merging...\n";
#$d = 10;
ok( Pod::Simple::DumpAsXML->_out( \&nixy_mergy, "=pod\n\nZ<>F<C<Z<>fE<111>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
  join "\n",

  '<Document>',
  '  <Para>',
  '    <F>',
  '      <C>',
  '        foo',
  '      </C>',
  '      ',
  '      <I>',
  '        bar',
  '      </I>',
  '    </F>',
  '    ',
  '    <B>',
  '      stuff baz',
  '    </B>',
  '  </Para>',
  '</Document>',
  '',
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Now the scary bits... with L's!
print "# A wee L<...> sanity test...\n";
ok( Pod::Simple::XMLOutStream->_out(qq{=pod\n\nL<E<78>et::Ping/Ping-E<112>ong>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);
print "# Now a wee L<...> with mergy...\n";

$d = 10;

ok( Pod::Simple::DumpAsXML->_out(\&mergy, qq{=pod\n\nL<E<78>et::Ping/Ping-E<112>ong>\n}),
 join "\n",

 '<Document>',
 '  <Para>',
 '    <L content-implicit="yes" raw="E&#60;78&#62;et::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">',
 '      &#34;Ping-pong&#34; in Net::Ping',
 '    </L>',
 '  </Para>',
 '</Document>',
 ''
);


print "# Now a complex tree with L's, with nixy+mergy...\n";

ok( Pod::Simple::DumpAsXML->_out( \&nixy_mergy, "=pod\n\nZ<>F<C<Z<>fE<111>L<E<78>et::Ping/Ping-E<112>ong>o> I<bar>> B<stuff X<thingZ<>>baz>\n"),
  join "\n",

  '<Document>',
  '  <Para>',
  '    <F>',
  '      <C>',
  '        fo',
  '        <L content-implicit="yes" raw="E&#60;78&#62;et::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">',
  '          &#34;Ping-pong&#34; in Net::Ping',
  '        </L>',
  '        o',
  '      </C>',
  '      ',
  '      <I>',
  '        bar',
  '      </I>',
  '    </F>',
  '    ',
  '    <B>',
  '      stuff baz',
  '    </B>',
  '  </Para>',
  '</Document>',
  '',
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

