# fcodes L
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 101 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}

#use Pod::Simple::Debug (10);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

my $x = 'Pod::Simple::XMLOutStream';

print "##### Testing L codes via x class $x...\n";

$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output

print "# Simple/moderate L<stuff> tests...\n";

ok($x->_out(qq{=pod\n\nL<Net::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);

ok($x->_out(qq{=pod\n\nL<crontab(5)>\n}),
 '<Document><Para><L content-implicit="yes" raw="crontab(5)" to="crontab(5)" type="man">crontab(5)</L></Para></Document>'
);

ok($x->_out(qq{=pod\n\nL<login.conf(5)>\n}),
 '<Document><Para><L content-implicit="yes" raw="login.conf(5)" to="login.conf(5)" type="man">login.conf(5)</L></Para></Document>'
);

ok($x->_out(qq{=pod\n\nL<foo_bar(5)>\n}),
 '<Document><Para><L content-implicit="yes" raw="foo_bar(5)" to="foo_bar(5)" type="man">foo_bar(5)</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL<Net::Ping/Ping-pong>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/Ping-pong" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL<Net::Ping/"Ping-pong">\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/&#34;Ping-pong&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"Object Methods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;Object Methods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</Object Methods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/Object Methods" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"Object Methods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;Object Methods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);


print "# Complex L<stuff> tests...\n";
print "#  Ents in the middle...\n";

ok($x->_out(qq{=pod\n\nL<Net::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/Ping-E<112>ong>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/"Ping-E<112>ong">\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/&#34;Ping-E&#60;112&#62;ong&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"Object E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;Object E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</Object E<77>ethods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/Object E&#60;77&#62;ethods" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"Object E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;Object E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);



print "#  Ents in the middle and at the start...\n";

ok($x->_out(qq{=pod\n\nL<E<78>et::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::Ping/Ping-E<112>ong>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::Ping/"Ping-E<112>ong">\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping/&#34;Ping-E&#60;112&#62;ong&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"E<79>bject E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;E&#60;79&#62;bject E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</E<79>bject E<77>ethods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/E&#60;79&#62;bject E&#60;77&#62;ethods" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"E<79>bject E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;E&#60;79&#62;bject E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);


print "#  Ents in the middle and at the start and at the end...\n";

ok($x->_out(qq{=pod\n\nL<E<78>et::PinE<103>>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::PinE<103>/Ping-E<112>onE<103>>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;/Ping-E&#60;112&#62;onE&#60;103&#62;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::PinE<103>/"Ping-E<112>onE<103>">\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;/&#34;Ping-E&#60;112&#62;onE&#60;103&#62;&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"E<79>bject E<77>ethodE<115>">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;E&#60;79&#62;bject E&#60;77&#62;ethodE&#60;115&#62;&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</E<79>bject E<77>ethodE<115>>\n}),
 '<Document><Para><L content-implicit="yes" raw="/E&#60;79&#62;bject E&#60;77&#62;ethodE&#60;115&#62;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"E<79>bject E<77>ethodE<115>">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;E&#60;79&#62;bject E&#60;77&#62;ethodE&#60;115&#62;&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);


print "# Even more complex L<stuff> tests...\n";


print "#  Ents in the middle...\n";

ok($x->_out(qq{=pod\n\nL<Net::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/Ping-E<112>ong>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/Ping-E&#60;112&#62;ong" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/"Ping-E<112>ong">\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/&#34;Ping-E&#60;112&#62;ong&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-pong&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"Object E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;Object E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</Object E<77>ethods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/Object E&#60;77&#62;ethods" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"Object E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;Object E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;Object Methods&#34;</L></Para></Document>'
);


###########################################################################

print "# VERY complex L sequences...\n";
print "#  Ents in the middle and at the start...\n";


ok($x->_out(qq{=pod\n\nL<Net::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/Ping-B<E<112>ong>>\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/Ping-B&#60;E&#60;112&#62;ong&#62;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Net::Ping/"Ping-B<E<112>ong>">\n}),
 '<Document><Para><L content-implicit="yes" raw="Net::Ping/&#34;Ping-B&#60;E&#60;112&#62;ong&#62;&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"B<Object> E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;B&#60;Object&#62; E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</B<Object> E<77>ethods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/B&#60;Object&#62; E&#60;77&#62;ethods" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"B<Object> E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;B&#60;Object&#62; E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);



print "#  Ents in the middle and at the start...\n";

ok($x->_out(qq{=pod\n\nL<E<78>et::Ping>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::Ping/Ping-B<E<112>ong>>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping/Ping-B&#60;E&#60;112&#62;ong&#62;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::Ping/"Ping-B<E<112>ong>">\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::Ping/&#34;Ping-B&#60;E&#60;112&#62;ong&#62;&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"B<E<79>bject> E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</B<E<79>bject> E<77>ethods>\n}),
 '<Document><Para><L content-implicit="yes" raw="/B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethods" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"B<E<79>bject> E<77>ethods">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethods&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);


print "#  Ents in the middle and at the start and at the end...\n";

ok($x->_out(qq{=pod\n\nL<E<78>et::PinE<103>>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;" to="Net::Ping" type="pod">Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::PinE<103>/Ping-B<E<112>onE<103>>>\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;/Ping-B&#60;E&#60;112&#62;onE&#60;103&#62;&#62;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<E<78>et::PinE<103>/"Ping-B<E<112>onE<103>>">\n}),
 '<Document><Para><L content-implicit="yes" raw="E&#60;78&#62;et::PinE&#60;103&#62;/&#34;Ping-B&#60;E&#60;112&#62;onE&#60;103&#62;&#62;&#34;" section="Ping-pong" to="Net::Ping" type="pod">&#34;Ping-<B>pong</B>&#34; in Net::Ping</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL</"B<E<79>bject> E<77>ethodE<115>">\n}),
 '<Document><Para><L content-implicit="yes" raw="/&#34;B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethodE&#60;115&#62;&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL</B<E<79>bject> E<77>ethodE<115>>\n}),
 '<Document><Para><L content-implicit="yes" raw="/B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethodE&#60;115&#62;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<"B<E<79>bject> E<77>ethodE<115>">\n}),
 '<Document><Para><L content-implicit="yes" raw="&#34;B&#60;E&#60;79&#62;bject&#62; E&#60;77&#62;ethodE&#60;115&#62;&#34;" section="Object Methods" type="pod">&#34;<B>Object</B> Methods&#34;</L></Para></Document>'
);


###########################################################################

print "#\n# L<url> tests...\n";

ok( $x->_out(qq{=pod\n\nL<news:comp.lang.perl.misc>\n}),
 '<Document><Para><L content-implicit="yes" raw="news:comp.lang.perl.misc" to="news:comp.lang.perl.misc" type="url">news:comp.lang.perl.misc</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<http://www.perl.com>\n}),
 '<Document><Para><L content-implicit="yes" raw="http://www.perl.com" to="http://www.perl.com" type="url">http://www.perl.com</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/>\n}),
 '<Document><Para><L content-implicit="yes" raw="http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/" to="http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/" type="url">http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/</L></Para></Document>'
);

print "# L<url> tests with entities...\n";

ok( $x->_out(qq{=pod\n\nL<news:compE<46>lang.perl.misc>\n}),
 '<Document><Para><L content-implicit="yes" raw="news:compE&#60;46&#62;lang.perl.misc" to="news:comp.lang.perl.misc" type="url">news:comp.lang.perl.misc</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<http://wwwE<46>perl.com>\n}),
 '<Document><Para><L content-implicit="yes" raw="http://wwwE&#60;46&#62;perl.com" to="http://www.perl.com" type="url">http://www.perl.com</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<http://wwwE<46>perl.com/CPAN/authors/id/S/SB/SBURKE/>\n}),
 '<Document><Para><L content-implicit="yes" raw="http://wwwE&#60;46&#62;perl.com/CPAN/authors/id/S/SB/SBURKE/" to="http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/" type="url">http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<http://wwwE<46>perl.com/CPAN/authors/id/S/SB/SBURKEE<47>>\n}),
 '<Document><Para><L content-implicit="yes" raw="http://wwwE&#60;46&#62;perl.com/CPAN/authors/id/S/SB/SBURKEE&#60;47&#62;" to="http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/" type="url">http://www.perl.com/CPAN/authors/id/S/SB/SBURKE/</L></Para></Document>'
);


###########################################################################


print "# L<text|stuff> tests...\n";

ok($x->_out(qq{=pod\n\nL<things|crontab(5)>\n}),
 '<Document><Para><L raw="things|crontab(5)" to="crontab(5)" type="man">things</L></Para></Document>'
);
ok($x->_out(qq{=pod\n\nL<things|crontab(5)/ENVIRONMENT>\n}),
 '<Document><Para><L raw="things|crontab(5)/ENVIRONMENT" section="ENVIRONMENT" to="crontab(5)" type="man">things</L></Para></Document>'
);
ok($x->_out(qq{=pod\n\nL<things|crontab(5)/"ENVIRONMENT">\n}),
 '<Document><Para><L raw="things|crontab(5)/&#34;ENVIRONMENT&#34;" section="ENVIRONMENT" to="crontab(5)" type="man">things</L></Para></Document>'
);

ok( $x->_out(qq{=pod\n\nL<Perl Error Messages|perldiag>\n}),
 '<Document><Para><L raw="Perl Error Messages|perldiag" to="perldiag" type="pod">Perl Error Messages</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Perl\nError\nMessages|perldiag>\n}),
 '<Document><Para><L raw="Perl Error Messages|perldiag" to="perldiag" type="pod">Perl Error Messages</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Perl\nError\t  Messages|perldiag>\n}),
 '<Document><Para><L raw="Perl Error Messages|perldiag" to="perldiag" type="pod">Perl Error Messages</L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<SWITCH statements|perlsyn/"Basic BLOCKs and Switch Statements">\n}),
 '<Document><Para><L raw="SWITCH statements|perlsyn/&#34;Basic BLOCKs and Switch Statements&#34;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH statements</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<SWITCH statements|perlsyn/Basic BLOCKs and Switch Statements>\n}),
 '<Document><Para><L raw="SWITCH statements|perlsyn/Basic BLOCKs and Switch Statements" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH statements</L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<the various attributes|/"Member Data">\n}),
 '<Document><Para><L raw="the various attributes|/&#34;Member Data&#34;" section="Member Data" type="pod">the various attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<the various attributes|/Member Data>\n}),
 '<Document><Para><L raw="the various attributes|/Member Data" section="Member Data" type="pod">the various attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<the various attributes|"Member Data">\n}),
 '<Document><Para><L raw="the various attributes|&#34;Member Data&#34;" section="Member Data" type="pod">the various attributes</L></Para></Document>'
);


print "#\n# Now some very complex L<text|stuff> tests...\n";


ok( $x->_out(qq{=pod\n\nL<Perl B<Error E<77>essages>|perldiag>\n}),
 '<Document><Para><L raw="Perl B&#60;Error E&#60;77&#62;essages&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Perl\nB<Error\nE<77>essages>|perldiag>\n}),
 '<Document><Para><L raw="Perl B&#60;Error E&#60;77&#62;essages&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<Perl\nB<Error\t  E<77>essages>|perldiag>\n}),
 '<Document><Para><L raw="Perl B&#60;Error E&#60;77&#62;essages&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<SWITCH B<E<115>tatements>|perlsyn/"Basic I<BLOCKs> and Switch StatementE<115>">\n}),
 '<Document><Para><L raw="SWITCH B&#60;E&#60;115&#62;tatements&#62;|perlsyn/&#34;Basic I&#60;BLOCKs&#62; and Switch StatementE&#60;115&#62;&#34;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<SWITCH B<E<115>tatements>|perlsyn/Basic I<BLOCKs> and Switch StatementE<115>>\n}),
 '<Document><Para><L raw="SWITCH B&#60;E&#60;115&#62;tatements&#62;|perlsyn/Basic I&#60;BLOCKs&#62; and Switch StatementE&#60;115&#62;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<the F<various> attributes|/"Member Data">\n}),
 '<Document><Para><L raw="the F&#60;various&#62; attributes|/&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<the F<various> attributes|/Member Data>\n}),
 '<Document><Para><L raw="the F&#60;various&#62; attributes|/Member Data" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<the F<various> attributes|"Member Data">\n}),
 '<Document><Para><L raw="the F&#60;various&#62; attributes|&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);


print "#\n# Now some very complex L<text|stuff> tests with variant syntax...\n";


ok( $x->_out(qq{=pod\n\nL<< Perl B<<< Error E<77>essages >>>|perldiag >>\n}),
 '<Document><Para><L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<< Perl\nB<<< Error\nE<77>essages >>>|perldiag >>\n}),
 '<Document><Para><L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<< Perl\nB<<< Error\t  E<77>essages >>>|perldiag >>\n}),
 '<Document><Para><L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<< SWITCH B<<< E<115>tatements >>>|perlsyn/"Basic I<<<< BLOCKs >>>> and Switch StatementE<115>" >>\n}),
 '<Document><Para><L raw="SWITCH B&#60;&#60;&#60; E&#60;115&#62;tatements &#62;&#62;&#62;|perlsyn/&#34;Basic I&#60;&#60;&#60;&#60; BLOCKs &#62;&#62;&#62;&#62; and Switch StatementE&#60;115&#62;&#34;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<< SWITCH B<<< E<115>tatements >>>|perlsyn/Basic I<<<< BLOCKs >>>> and Switch StatementE<115> >>\n}),
 '<Document><Para><L raw="SWITCH B&#60;&#60;&#60; E&#60;115&#62;tatements &#62;&#62;&#62;|perlsyn/Basic I&#60;&#60;&#60;&#60; BLOCKs &#62;&#62;&#62;&#62; and Switch StatementE&#60;115&#62;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L></Para></Document>'
);


ok( $x->_out(qq{=pod\n\nL<<< the F<< various >> attributes|/"Member Data" >>>\n}),
 '<Document><Para><L raw="the F&#60;&#60; various &#62;&#62; attributes|/&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<<< the F<< various >> attributes|/Member Data >>>\n}),
 '<Document><Para><L raw="the F&#60;&#60; various &#62;&#62; attributes|/Member Data" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);
ok( $x->_out(qq{=pod\n\nL<<< the F<< various >> attributes|"Member Data" >>>\n}),
 '<Document><Para><L raw="the F&#60;&#60; various &#62;&#62; attributes|&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L></Para></Document>'
);

###########################################################################

print "#\n# Now some very complex L<text|stuff> tests with variant syntax and text around it...\n";


ok( $x->_out(qq{=pod\n\nI like L<< Perl B<<< Error E<77>essages >>>|perldiag >>.\n}),
 '<Document><Para>I like <L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<< Perl\nB<<< Error\nE<77>essages >>>|perldiag >>.\n}),
 '<Document><Para>I like <L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<< Perl\nB<<< Error\t  E<77>essages >>>|perldiag >>.\n}),
 '<Document><Para>I like <L raw="Perl B&#60;&#60;&#60; Error E&#60;77&#62;essages &#62;&#62;&#62;|perldiag" to="perldiag" type="pod">Perl <B>Error Messages</B></L>.</Para></Document>'
);


ok( $x->_out(qq{=pod\n\nI like L<< SWITCH B<<< E<115>tatements >>>|perlsyn/"Basic I<<<< BLOCKs >>>> and Switch StatementE<115>" >>.\n}),
 '<Document><Para>I like <L raw="SWITCH B&#60;&#60;&#60; E&#60;115&#62;tatements &#62;&#62;&#62;|perlsyn/&#34;Basic I&#60;&#60;&#60;&#60; BLOCKs &#62;&#62;&#62;&#62; and Switch StatementE&#60;115&#62;&#34;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<< SWITCH B<<< E<115>tatements >>>|perlsyn/Basic I<<<< BLOCKs >>>> and Switch StatementE<115> >>.\n}),
 '<Document><Para>I like <L raw="SWITCH B&#60;&#60;&#60; E&#60;115&#62;tatements &#62;&#62;&#62;|perlsyn/Basic I&#60;&#60;&#60;&#60; BLOCKs &#62;&#62;&#62;&#62; and Switch StatementE&#60;115&#62;" section="Basic BLOCKs and Switch Statements" to="perlsyn" type="pod">SWITCH <B>statements</B></L>.</Para></Document>'
);


ok( $x->_out(qq{=pod\n\nI like L<<< the F<< various >> attributes|/"Member Data" >>>.\n}),
 '<Document><Para>I like <L raw="the F&#60;&#60; various &#62;&#62; attributes|/&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< the F<< various >> attributes|/Member Data >>>.\n}),
 '<Document><Para>I like <L raw="the F&#60;&#60; various &#62;&#62; attributes|/Member Data" section="Member Data" type="pod">the <F>various</F> attributes</L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< the F<< various >> attributes|"Member Data" >>>.\n}),
 '<Document><Para>I like <L raw="the F&#60;&#60; various &#62;&#62; attributes|&#34;Member Data&#34;" section="Member Data" type="pod">the <F>various</F> attributes</L>.</Para></Document>'
);

ok( $x->_out(qq{=pod\n\nI like L<<< B<text>s|http://text.com >>>.\n}),
'<Document><Para>I like <L raw="B&#60;text&#62;s|http://text.com" to="http://text.com" type="url"><B>text</B>s</L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< text|https://text.com/1/2 >>>.\n}),
'<Document><Para>I like <L raw="text|https://text.com/1/2" to="https://text.com/1/2" type="url">text</L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< I<text>|http://text.com >>>.\n}),
'<Document><Para>I like <L raw="I&#60;text&#62;|http://text.com" to="http://text.com" type="url"><I>text</I></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< C<text>|http://text.com >>>.\n}),
'<Document><Para>I like <L raw="C&#60;text&#62;|http://text.com" to="http://text.com" type="url"><C>text</C></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< I<tI<eI<xI<t>>>>|mailto:earlE<64>text.com >>>.\n}),
'<Document><Para>I like <L raw="I&#60;tI&#60;eI&#60;xI&#60;t&#62;&#62;&#62;&#62;|mailto:earlE&#60;64&#62;text.com" to="mailto:earl@text.com" type="url"><I>t<I>e<I>x<I>t</I></I></I></I></L>.</Para></Document>'
);
ok( $x->_out(qq{=pod\n\nI like L<<< textZ<>|http://text.com >>>.\n}),
'<Document><Para>I like <L raw="textZ&#60;&#62;|http://text.com" to="http://text.com" type="url">text</L>.</Para></Document>'
);




#
# TODO: S testing.
#

###########################################################################

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


