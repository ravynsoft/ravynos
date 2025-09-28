# fcodes S
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 80 };

#use Pod::Simple::Debug (6);

ok 1;

use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";
my $x = 'Pod::Simple::XMLOutStream';

sub e { $x->_duo(@_) }

$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output


print "# S as such...\n";

ok( $x->_out("=pod\n\nI like S<bric-a-brac>.\n"),
 =>  '<Document><Para>I like <S>bric-a-brac</S>.</Para></Document>' );
ok( $x->_out("=pod\n\nI like S<bric-a-brac a gogo >.\n"),
 =>  '<Document><Para>I like <S>bric-a-brac a gogo </S>.</Para></Document>' );
ok( $x->_out("=pod\n\nI like S<< bric-a-brac a gogo >>.\n"),
 =>  '<Document><Para>I like <S>bric-a-brac a gogo</S>.</Para></Document>' );

my $unless_ascii = (chr(65) eq 'A') ? '' :
 "Skip because not in ASCIIland";

skip( $unless_ascii,
    $x->_out( sub { $_[0]->nbsp_for_S(1) },
    "=pod\n\nI like S<bric-a-brac a gogo>.\n"),
'<Document><Para>I like bric-a-brac&#160;a&#160;gogo.</Para></Document>'
);
skip( $unless_ascii,
    $x->_out( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L</"bric-a-brac a gogo">>.\n}),
'<Document><Para>I like <L content-implicit="yes" raw="/&#34;bric-a-brac a gogo&#34;" section="bric-a-brac a gogo" type="pod">&#34;bric-a-brac&#160;a&#160;gogo&#34;</L>.</Para></Document>'
);
skip( $unless_ascii,
    $x->_out( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L<Stuff like that|/"bric-a-brac a gogo">>.\n}),
'<Document><Para>I like <L raw="Stuff like that|/&#34;bric-a-brac a gogo&#34;" section="bric-a-brac a gogo" type="pod">Stuff&#160;like&#160;that</L>.</Para></Document>'
);
skip( $unless_ascii,
    $x->_out( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L<Stuff I<like that>|/"bric-a-brac a gogo">>.\n}),
'<Document><Para>I like <L raw="Stuff I&#60;like that&#62;|/&#34;bric-a-brac a gogo&#34;" section="bric-a-brac a gogo" type="pod">Stuff&#160;<I>like&#160;that</I></L>.</Para></Document>'
);

&ok( $x->_duo( sub { $_[0]->nbsp_for_S(1) },
  "=pod\n\nI like S<bric-a-brac a gogo>.\n",
  "=pod\n\nI like bric-a-bracE<160>aE<160>gogo.\n",
));
&ok(
  map {my $z = $_; $z =~ s/content-implicit="yes" //g; $z =~ s/raw=".+?" //g; $z }
  $x->_duo( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L</"bric-a-brac a gogo">>.\n},
    qq{=pod\n\nI like L<"bric-a-bracE<160>aE<160>gogo"|/"bric-a-brac a gogo">.\n},
));
&ok( 
  map {my $z = $_; $z =~ s/raw=".+?" //g; $z }
  $x->_duo( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L<Stuff like that|"bric-a-brac a gogo">>.\n},
    qq{=pod\n\nI like L<StuffE<160>likeE<160>that|"bric-a-brac a gogo">.\n},
));
&ok(
  map {my $z = $_; $z =~ s/content-implicit="yes" //g; $z =~ s/raw=".+?" //g; $z }
  $x->_duo( sub { $_[0]->nbsp_for_S(1) },
    qq{=pod\n\nI like S<L<Stuff I<like that>|"bric-a-brac a gogo">>.\n},
    qq{=pod\n\nI like L<StuffE<160>I<likeE<160>that>|"bric-a-brac a gogo">.\n},
));

use Pod::Simple::Text;
$x = Pod::Simple::Text->new;
$x->preserve_whitespace(1);
# RT#25679
ok(
  $x->_out(<<END
=head1 The Tk::mega manpage showed me how C<< SE<lt> > foo >> is being rendered

Both pod2text and pod2man S<    > lose the rest of the line

=head1 Do they always S<    > lose the rest of the line?

=cut
END
  ),
  <<END
The Tk::mega manpage showed me how S< > foo is being rendered

    Both pod2text and pod2man      lose the rest of the line

Do they always      lose the rest of the line?

END
);

$x = 'Pod::Simple::Text';
# Test text output of links.
ok(
    $x->_out(qq{=pod\n\nL<Net::Ping>\n}),
    "    Net::Ping\n\n"
);

ok(
    $x->_out(qq{=pod\n\nBe sure to read the L<Net::Ping> docs\n}),
    "    Be sure to read the Net::Ping docs\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<http://www.perl.com>\n}),
    "    http://www.perl.com\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<crontab(5)>\n}),
    "    crontab(5)\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<Net::Ping/Ping-pong>\n}),
    qq{    "Ping-pong" in Net::Ping\n\n}
);

ok(
    $x->_out(qq{=pod\n\nL</"Object Methods">\n}),
    qq{    "Object Methods"\n\n}
);

ok(
    $x->_out(qq{=pod\n\nL</Object Methods>\n}),
    qq{    "Object Methods"\n\n}
);

ok(
    $x->_out(qq{=pod\n\nL<"Object Methods">\n}),
    qq{    "Object Methods"\n\n}
);

ok(
    $x->_out(qq{=pod\n\nL<Net::Ping/Ping-E<112>ong>\n}),
    qq{    "Ping-pong" in Net::Ping\n\n}
);

ok(
    $x->_out(qq{=pod\n\nL<news:comp.lang.perl.misc>\n}),
    "    news:comp.lang.perl.misc\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<http://www.perl.org>\n}),
    "    http://www.perl.org\n\n"
);

ok(
    $x->_out(qq{=pod\n\nSee L<http://www.perl.org>\n}),
    "    See http://www.perl.org\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/>\n}),
    "    http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<news:compE<46>lang.perl.misc>\n}),
    "    news:comp.lang.perl.misc\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<http://wwwE<46>perl.org>\n}),
    "    http://www.perl.org\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<things|crontab(5)>\n}),
    "    things\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<things|crontab(5)/ENVIRONMENT>\n}),
    "    things\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<Perl Error Messages|perldiag>\n}),
    "    Perl Error Messages\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<Perl\nError\nMessages|perldiag>\n}),
    "    Perl Error Messages\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<Perl\nError\t  Messages|perldiag>\n}),
    "    Perl Error Messages\n\n"
);

ok(
    $x->_out(qq{=pod\n\nL<perl.org|http://www.perl.org>\n}),
    "    perl.org <http://www.perl.org>\n\n"
);

ok(
    $x->_out(qq{=pod\n\nSee L<perl.org|http://www.perl.org>\n}),
    "    See perl.org <http://www.perl.org>\n\n"
);


# Test HTML output of links.
use Pod::Simple::HTML;
my $PERLDOC = "https://metacpan.org/pod";
my $MANURL = "http://man.he.net/man";
sub x {
    Pod::Simple::HTML->_out(
        sub {  $_[0]->bare_output(1)  },
        "=pod\n\n$_[0]",
    )
}

ok(
    x(qq{L<Net::Ping>\n}),
    qq{\n<p><a href="$PERLDOC/Net%3A%3APing" class="podlinkpod"\n>Net::Ping</a></p>\n}
);

ok(
    x(qq{Be sure to read the L<Net::Ping> docs\n}),
    qq{\n<p>Be sure to read the <a href="$PERLDOC/Net%3A%3APing" class="podlinkpod"\n>Net::Ping</a> docs</p>\n}
);

ok(
    x(qq{L<http://www.perl.com>\n}),
    qq{\n<p><a href="http://www.perl.com" class="podlinkurl"\n>http://www.perl.com</a></p>\n}
);

ok(
    x(qq{L<crontab(5)>\n}),
    qq{\n<p><a href="${MANURL}5/crontab" class="podlinkman"\n>crontab(5)</a></p>\n}
);

ok(
    x(qq{L<Net::Ping/Ping-pong>\n}),
    qq{\n<p><a href="$PERLDOC/Net%3A%3APing#Ping-pong" class="podlinkpod"\n>&#34;Ping-pong&#34; in Net::Ping</a></p>\n}
);

ok(
    x(qq{L</"Object Methods">\n}),
    qq{\n<p><a href="#Object_Methods" class="podlinkpod"\n>&#34;Object Methods&#34;</a></p>\n}
);

ok(
    x(qq{L</Object Methods>\n}),
    qq{\n<p><a href="#Object_Methods" class="podlinkpod"\n>&#34;Object Methods&#34;</a></p>\n}
);

ok(
    x(qq{L<"Object Methods">\n}),
    qq{\n<p><a href="#Object_Methods" class="podlinkpod"\n>&#34;Object Methods&#34;</a></p>\n}
);

ok(
    x(qq{L<Net::Ping/Ping-E<112>ong>\n}),
    qq{\n<p><a href="$PERLDOC/Net%3A%3APing#Ping-pong" class="podlinkpod"\n>&#34;Ping-pong&#34; in Net::Ping</a></p>\n}
);

ok(
    x(qq{L<news:comp.lang.perl.misc>\n}),
    qq{\n<p><a href="news:comp.lang.perl.misc" class="podlinkurl"\n>news:comp.lang.perl.misc</a></p>\n}
);

ok(
    x(qq{L<http://www.perl.org>\n}),
    qq{\n<p><a href="http://www.perl.org" class="podlinkurl"\n>http://www.perl.org</a></p>\n}
);

ok(
    x(qq{See L<http://www.perl.org>\n}),
    qq{\n<p>See <a href="http://www.perl.org" class="podlinkurl"\n>http://www.perl.org</a></p>\n}
);

ok(
    x(qq{L<http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/>\n}),
    qq{\n<p><a href="http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/" class="podlinkurl"\n>http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/</a></p>\n}
);

ok(
    x(qq{L<news:compE<46>lang.perl.misc>\n}),
    qq{\n<p><a href="news:comp.lang.perl.misc" class="podlinkurl"\n>news:comp.lang.perl.misc</a></p>\n}
);

ok(
    x(qq{L<http://wwwE<46>perl.org>\n}),
    qq{\n<p><a href="http://www.perl.org" class="podlinkurl"\n>http://www.perl.org</a></p>\n}
);

ok(
    x(qq{L<things|crontab(5)>\n}),
    qq{\n<p><a href="${MANURL}5/crontab" class="podlinkman"\n>things</a></p>\n}
);

ok(
    x(qq{L<things|crontab(5)/ENVIRONMENT>\n}),
    qq{\n<p><a href="${MANURL}5/crontab" class="podlinkman"\n>things</a></p>\n}
);

ok(
    x(qq{L<Perl Error Messages|perldiag>\n}),
    qq{\n<p><a href="$PERLDOC/perldiag" class="podlinkpod"\n>Perl Error Messages</a></p>\n}
);

ok(
    x(qq{L<Perl\nError\nMessages|perldiag>\n}),
    qq{\n<p><a href="$PERLDOC/perldiag" class="podlinkpod"\n>Perl Error Messages</a></p>\n}
);

ok(
    x(qq{L<Perl\nError\t  Messages|perldiag>\n}),
    qq{\n<p><a href="$PERLDOC/perldiag" class="podlinkpod"\n>Perl Error Messages</a></p>\n}
);

ok(
    x(qq{L<perl.org|http://www.perl.org>\n}),
    qq{\n<p><a href="http://www.perl.org" class="podlinkurl"\n>perl.org</a></p>\n}
);

ok(
    x(qq{See L<perl.org|http://www.perl.org>\n}),
    qq{\n<p>See <a href="http://www.perl.org" class="podlinkurl"\n>perl.org</a></p>\n}
);

# Test link output in XHTML.
use Pod::Simple::XHTML;
sub o ($) {
    my $p = Pod::Simple::XHTML->new;
    $p->html_header("");
    $p->html_footer("");
    my $results = '';
    $p->output_string( \$results ); # Send the resulting output to a string
    $p->parse_string_document("=pod\n\n$_[0]");
    return $results;
}

ok(
    o(qq{L<Net::Ping>}),
    qq{<p><a href="$PERLDOC/Net::Ping">Net::Ping</a></p>\n\n}
);

ok(
    o(qq{Be sure to read the L<Net::Ping> docs}),
    qq{<p>Be sure to read the <a href="$PERLDOC/Net::Ping">Net::Ping</a> docs</p>\n\n}
);

ok(
    o(qq{L<http://www.perl.com>}),
    qq{<p><a href="http://www.perl.com">http://www.perl.com</a></p>\n\n}
);

ok(
    o(qq{L<crontab(5)>}),
    qq{<p><a href="${MANURL}5/crontab">crontab(5)</a></p>\n\n}
);

ok(
    o(qq{L<Net::Ping/Ping-pong>}),
    qq{<p><a href="$PERLDOC/Net::Ping#Ping-pong">&quot;Ping-pong&quot; in Net::Ping</a></p>\n\n}
);

ok(
    o(qq{L</"Object Methods">}),
    qq{<p><a href="#Object-Methods">&quot;Object Methods&quot;</a></p>\n\n}
);

ok(
    o(qq{L</Object Methods>}),
    qq{<p><a href="#Object-Methods">&quot;Object Methods&quot;</a></p>\n\n}
);

ok(
    o(qq{L<"Object Methods">}),
    qq{<p><a href="#Object-Methods">&quot;Object Methods&quot;</a></p>\n\n}
);

ok(
    o(qq{L<Net::Ping/Ping-E<112>ong>}),
    qq{<p><a href="$PERLDOC/Net::Ping#Ping-pong">&quot;Ping-pong&quot; in Net::Ping</a></p>\n\n}
);

ok(
    o(qq{L<news:comp.lang.perl.misc>}),
    qq{<p><a href="news:comp.lang.perl.misc">news:comp.lang.perl.misc</a></p>\n\n}
);

ok(
    o(qq{L<http://www.perl.org>}),
    qq{<p><a href="http://www.perl.org">http://www.perl.org</a></p>\n\n}
);

ok(
    o(qq{See L<http://www.perl.org>}),
    qq{<p>See <a href="http://www.perl.org">http://www.perl.org</a></p>\n\n}
);

ok(
    o(qq{L<http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/>}),
    qq{<p><a href="http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/">http://www.perl.org/CPAN/authors/id/S/SB/SBURKE/</a></p>\n\n}
);

ok(
    o(qq{L<news:compE<46>lang.perl.misc>}),
    qq{<p><a href="news:comp.lang.perl.misc">news:comp.lang.perl.misc</a></p>\n\n}
);

ok(
    o(qq{L<http://wwwE<46>perl.org>}),
    qq{<p><a href="http://www.perl.org">http://www.perl.org</a></p>\n\n}
);

ok(
    o(qq{L<things|crontab(5)>}),
    qq{<p><a href="${MANURL}5/crontab">things</a></p>\n\n}
);

ok(
    o(qq{L<things|crontab(5)/ENVIRONMENT>}),
    qq{<p><a href="${MANURL}5/crontab">things</a></p>\n\n}
);

ok(
    o(qq{L<Perl Error Messages|perldiag>}),
    qq{<p><a href="$PERLDOC/perldiag">Perl Error Messages</a></p>\n\n}
);

ok(
    o(qq{L<Perl\nError\nMessages|perldiag>}),
    qq{<p><a href="$PERLDOC/perldiag">Perl Error Messages</a></p>\n\n}
);

ok(
    o(qq{L<Perl\nError\t  Messages|perldiag>}),
    qq{<p><a href="$PERLDOC/perldiag">Perl Error Messages</a></p>\n\n}
);

ok(
    o(qq{L<perl.org|http://www.perl.org>}),
    qq{<p><a href="http://www.perl.org">perl.org</a></p>\n\n}
);

ok(
    o(qq{See L<perl.org|http://www.perl.org>}),
    qq{<p>See <a href="http://www.perl.org">perl.org</a></p>\n\n}
);

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

