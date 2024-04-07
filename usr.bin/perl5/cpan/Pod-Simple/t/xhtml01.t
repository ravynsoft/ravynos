#!/usr/bin/perl -w

# t/xhtml01.t - check basic output from Pod::Simple::XHTML

BEGIN {
    chdir 't' if -d 't';
}

use strict;
use warnings;
use lib '../lib';
use Test::More tests => 64;
#use Test::More 'no_plan';

use_ok('Pod::Simple::XHTML') or exit;

my $parser = Pod::Simple::XHTML->new ();
isa_ok ($parser, 'Pod::Simple::XHTML');

my $results;

my $PERLDOC = "https://metacpan.org/pod";
my $MANURL = "http://man.he.net/man";

initialize($parser, $results);
$parser->parse_string_document( "=head1 Poit!" );
is($results, qq{<h1 id="Poit">Poit!</h1>\n\n}, "head1 level output");

initialize($parser, $results);
$parser->parse_string_document( "=head2 Yada Yada Operator
X<...> X<... operator> X<yada yada operator>" );
is($results, qq{<h2 id="Yada-Yada-Operator">Yada Yada Operator   </h2>\n\n}, "head ID with X<>");

initialize($parser, $results);
$parser->parse_string_document( "=head2 Platforms with no supporting programmers:");
is($results, qq{<h2 id="Platforms-with-no-supporting-programmers">Platforms with no supporting programmers:</h2>\n\n}, "head ID ending in colon");

initialize($parser, $results);
$parser->html_h_level(2);
$parser->parse_string_document( "=head1 Poit!" );
is($results, qq{<h2 id="Poit">Poit!</h2>\n\n}, "head1 level output h_level 2");

initialize($parser, $results);
$parser->parse_string_document( "=head2 I think so Brain." );
is($results, qq{<h2 id="I-think-so-Brain">I think so Brain.</h2>\n\n}, "head2 level output");

initialize($parser, $results);
$parser->parse_string_document( "=head3 I say, Brain..." );
is($results, qq{<h3 id="I-say-Brain">I say, Brain...</h3>\n\n}, "head3 level output");

initialize($parser, $results);
$parser->parse_string_document( "=head4 Zort & Zog!" );
is($results, qq{<h4 id="Zort-Zog">Zort &amp; Zog!</h4>\n\n}, "head4 level output");

initialize($parser, $results);
$parser->parse_string_document( "=head5 I think so Brain, but..." );
is($results, qq{<h5 id="I-think-so-Brain-but">I think so Brain, but...</h5>\n\n}, "head5 level output");

initialize($parser, $results);
$parser->parse_string_document( "=head6 Narf!" );
is($results, qq{<h6 id="Narf">Narf!</h6>\n\n}, "head6 level output");

sub x {
  my $code = $_[1];
  Pod::Simple::XHTML->_out(
  sub { $code->($_[0]) if $code },
  "=pod\n\n$_[0]",
) }

like(
  x("=head1 Header\n\n=for html <div>RAW<span>!</span></div>\n\nDone."),
  qr/.+<\/h1>\s+<div>RAW<span>!<\/span><\/div>\s+.*/sm,
  "heading building"
) or exit;

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

Gee, Brain, what do you want to do tonight?
EOPOD

is($results, <<'EOHTML', "simple paragraph");
<p>Gee, Brain, what do you want to do tonight?</p>

EOHTML


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

B: Now, Pinky, if by any chance you are captured during this mission,
remember you are Gunther Heindriksen from Appenzell. You moved to
Grindelwald to drive the cog train to Murren. Can you repeat that?

P: Mmmm, no, Brain, dont think I can.
EOPOD

is($results, <<'EOHTML', "multiple paragraphs");
<p>B: Now, Pinky, if by any chance you are captured during this mission, remember you are Gunther Heindriksen from Appenzell. You moved to Grindelwald to drive the cog train to Murren. Can you repeat that?</p>

<p>P: Mmmm, no, Brain, dont think I can.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item *

P: Gee, Brain, what do you want to do tonight?

=item *

B: The same thing we do every night, Pinky. Try to take over the world!

=back

EOPOD

is($results, <<'EOHTML', "simple bulleted list");
<ul>

<li><p>P: Gee, Brain, what do you want to do tonight?</p>

</li>
<li><p>B: The same thing we do every night, Pinky. Try to take over the world!</p>

</li>
</ul>

EOHTML


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item *

P: Gee, Brain, what do you want to do tonight?

=item *

B: The same thing we do every night, Pinky. Try to take over the world!

=over

=item *

Take over world

=item *

Do laundry

=back

=back

EOPOD

is($results, <<'EOHTML', "nested bulleted list");
<ul>

<li><p>P: Gee, Brain, what do you want to do tonight?</p>

</li>
<li><p>B: The same thing we do every night, Pinky. Try to take over the world!</p>

<ul>

<li><p>Take over world</p>

</li>
<li><p>Do laundry</p>

</li>
</ul>

</li>
</ul>

EOHTML



initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item 1

P: Gee, Brain, what do you want to do tonight?

=item 2

B: The same thing we do every night, Pinky. Try to take over the world!

=back

EOPOD

is($results, <<'EOHTML', "numbered list");
<ol>

<li><p>P: Gee, Brain, what do you want to do tonight?</p>

</li>
<li><p>B: The same thing we do every night, Pinky. Try to take over the world!</p>

</li>
</ol>

EOHTML


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item 1

P: Gee, Brain, what do you want to do tonight?

=item 2

B: The same thing we do every night, Pinky. Try to take over the world!

=over

=item 1

Take over world

=item 2

Do laundry

=back

=back

EOPOD

is($results, <<'EOHTML', "nested numbered list");
<ol>

<li><p>P: Gee, Brain, what do you want to do tonight?</p>

</li>
<li><p>B: The same thing we do every night, Pinky. Try to take over the world!</p>

<ol>

<li><p>Take over world</p>

</li>
<li><p>Do laundry</p>

</li>
</ol>

</li>
</ol>

EOHTML


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item Pinky

Gee, Brain, what do you want to do tonight?

=item Brain

The same thing we do every night, Pinky. Try to take over the world!

=back

EOPOD

is($results, <<'EOHTML', "list with text headings");
<dl>

<dt>Pinky</dt>
<dd>

<p>Gee, Brain, what do you want to do tonight?</p>

</dd>
<dt>Brain</dt>
<dd>

<p>The same thing we do every night, Pinky. Try to take over the world!</p>

</dd>
</dl>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item * Pinky

Gee, Brain, what do you want to do tonight?

=item * Brain

The same thing we do every night, Pinky. Try to take over the world!

=back

EOPOD

is($results, <<'EOHTML', "list with bullet and text headings");
<ul>

<li><p>Pinky</p>

<p>Gee, Brain, what do you want to do tonight?</p>

</li>
<li><p>Brain</p>

<p>The same thing we do every night, Pinky. Try to take over the world!</p>

</li>
</ul>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item * Brain <brain@binkyandthebrain.com>

=item * Pinky <pinky@binkyandthebrain.com>

=back

EOPOD

is($results, <<'EOHTML', "bulleted author list");
<ul>

<li><p>Brain &lt;brain@binkyandthebrain.com&gt;</p>

</li>
<li><p>Pinky &lt;pinky@binkyandthebrain.com&gt;</p>

</li>
</ul>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item Pinky

=over

=item World Domination

=back

=item Brain

=back

EOPOD

is($results, <<'EOHTML', 'nested lists');
<dl>

<dt>Pinky</dt>
<dd>

<dl>

<dt>World Domination</dt>
<dd>

</dd>
</dl>

</dd>
<dt>Brain</dt>
<dd>

</dd>
</dl>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=over

=item Pinky

On the list:

=over

=item World Domination

Fight the good fight

=item Go to Europe

(Steve Martin joke)

=back

=item Brain

Not so much

=back

EOPOD

is($results, <<'EOHTML', 'multiparagraph nested lists');
<dl>

<dt>Pinky</dt>
<dd>

<p>On the list:</p>

<dl>

<dt>World Domination</dt>
<dd>

<p>Fight the good fight</p>

</dd>
<dt>Go to Europe</dt>
<dd>

<p>(Steve Martin joke)</p>

</dd>
</dl>

</dd>
<dt>Brain</dt>
<dd>

<p>Not so much</p>

</dd>
</dl>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

  1 + 1 = 2;
  2 + 2 = 4;

EOPOD

is($results, <<'EOHTML', "code block");
<pre><code>  1 + 1 = 2;
  2 + 2 = 4;</code></pre>

EOHTML


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a C<functionname>.

C<< This code is B<important> to E<lt>me>! >>

EOPOD
is($results, <<"EOHTML", "code entity in a paragraph");
<p>A plain paragraph with a <code>functionname</code>.</p>

<p><code>This code is <b>important</b> to &lt;me&gt;!</code></p>

EOHTML


initialize($parser, $results);
$parser->html_header("<html>\n<body>");
$parser->html_footer("</body>\n</html>");
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with body tags turned on.
EOPOD
is($results, <<"EOHTML", "adding html body tags");
<html>
<body>

<p>A plain paragraph with body tags turned on.</p>

</body>
</html>

EOHTML


initialize($parser, $results);
$parser->html_css('style.css');
$parser->html_header(undef);
$parser->html_footer(undef);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with body tags and css tags turned on.
EOPOD
like($results, qr/<link rel="stylesheet" href="style.css" type="text\/css" \/>/,
"adding html body tags and css tags");


initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with S<non breaking text>.
EOPOD
is($results, <<"EOHTML", "Non breaking text in a paragraph");
<p>A plain paragraph with <span style="white-space: nowrap;">non breaking text</span>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a L<Newlines>.
EOPOD
is($results, <<"EOHTML", "Link entity in a paragraph");
<p>A plain paragraph with a <a href="$PERLDOC/Newlines">Newlines</a>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a L<perlport/Newlines>.
EOPOD
is($results, <<"EOHTML", "Link entity in a paragraph");
<p>A plain paragraph with a <a href="$PERLDOC/perlport#Newlines">&quot;Newlines&quot; in perlport</a>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a L<Boo|http://link.included.here>.
EOPOD
is($results, <<"EOHTML", "A link in a paragraph");
<p>A plain paragraph with a <a href="http://link.included.here">Boo</a>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a L<http://link.included.here>.
EOPOD
is($results, <<"EOHTML", "A link in a paragraph");
<p>A plain paragraph with a <a href="http://link.included.here">http://link.included.here</a>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a L<http://link.included.here?o=1&p=2>.
EOPOD
is($results, <<"EOHTML", "A link in a paragraph");
<p>A plain paragraph with a <a href="http://link.included.here?o=1&amp;p=2">http://link.included.here?o=1&amp;p=2</a>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with B<bold text>.
EOPOD
is($results, <<"EOHTML", "Bold text in a paragraph");
<p>A plain paragraph with <b>bold text</b>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with I<italic text>.
EOPOD
is($results, <<"EOHTML", "Italic text in a paragraph");
<p>A plain paragraph with <i>italic text</i>.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A plain paragraph with a F<filename>.
EOPOD
is($results, <<"EOHTML", "File name in a paragraph");
<p>A plain paragraph with a <i>filename</i>.</p>

EOHTML

# It's not important that 's (apostrophes) be encoded for XHTML output.
initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

  # this header is very important & dont you forget it
  my $text = "File is: " . <FILE>;
EOPOD
is($results, <<"EOHTML", "Verbatim text with encodable entities");
<pre><code>  # this header is very important &amp; dont you forget it
  my \$text = &quot;File is: &quot; . &lt;FILE&gt;;</code></pre>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A text paragraph using E<sol> and E<verbar> special POD entities.

EOPOD
is($results, <<"EOHTML", "Text with decodable entities");
<p>A text paragraph using / and | special POD entities.</p>

EOHTML

initialize($parser, $results);
$parser->parse_string_document(<<'EOPOD');
=pod

A text paragraph using numeric POD entities: E<60>, E<62>.

EOPOD
is($results, <<"EOHTML", "Text with numeric entities");
<p>A text paragraph using numeric POD entities: &lt;, &gt;.</p>

EOHTML

my $html = q{<tt>
<pre>
#include &lt;stdio.h&gt;

int main(int argc,char *argv[]) {

        printf("Hellow World\n");
        return 0;

}
</pre>
</tt>};
initialize($parser, $results);
$parser->parse_string_document("=begin html\n\n$html\n\n=end html\n");
is($results, "$html\n\n", "Text with =begin html");

SKIP: for my $use_html_entities (0, 1) {
  if ($use_html_entities and not $Pod::Simple::XHTML::HAS_HTML_ENTITIES) {
    skip("HTML::Entities not installed", 3);
  }
  local $Pod::Simple::XHTML::HAS_HTML_ENTITIES = $use_html_entities;
  initialize($parser, $results);
  $parser->codes_in_verbatim(1);
  $parser->parse_string_document(<<'EOPOD');
=pod

  # this header is very important & dont you forget it
  B<my $file = <FILEE<gt> || Blank!;>
  my $text = "File is: " . <FILE>;
EOPOD
is($results, <<"EOHTML", "Verbatim text with markup and embedded formatting");
<pre><code>  # this header is very important &amp; dont you forget it
  <b>my \$file = &lt;FILE&gt; || Blank!;</b>
  my \$text = &quot;File is: &quot; . &lt;FILE&gt;;</code></pre>

EOHTML

  # Specify characters to encode.
  initialize($parser, $results);
  $parser->html_encode_chars('><"&T');
  $parser->parse_string_document(<<'EOPOD');
=pod

This is Anna's "Answer" to the <q>Question</q>.

=cut

EOPOD
my $T = $use_html_entities ? ord('T') : sprintf ("x%X", ord('T'));
is($results, <<"EOHTML", 'HTML Entities should be only for specified characters');
<p>&#$T;his is Anna's &quot;Answer&quot; to the &lt;q&gt;Question&lt;/q&gt;.</p>

EOHTML

  # Keep =encoding out of content.
  initialize($parser, $results);
  $parser->parse_string_document("=encoding ascii\n\n=head1 NAME\n");
  is($results, <<"EOHTML", 'Encoding should not be in content')
<h1 id="NAME">NAME</h1>

EOHTML

}


ok $parser = Pod::Simple::XHTML->new, 'Construct a new parser';
$results = '';
$parser->output_string( \$results ); # Send the resulting output to a string
ok $parser->parse_string_document( "=head1 Poit!" ), 'Parse with headers';
like $results, qr{\Q<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1" />},
    'Should have proper http-equiv meta tag';

ok $parser = Pod::Simple::XHTML->new, 'Construct a new parser again';
ok $parser->html_charset('UTF-8'), 'Set the html charset to UTF-8';
$results = '';
$parser->output_string( \$results ); # Send the resulting output to a string
ok $parser->parse_string_document( "=head1 Poit!" ), 'Parse with headers';
like $results, qr{\Q<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />},
    'Should have http-equiv meta tag with UTF-8';

# Test the link generation methods.
is $parser->resolve_pod_page_link('Net::Ping', 'INSTALL'),
    "$PERLDOC/Net::Ping#INSTALL",
    'POD link with fragment';
is $parser->resolve_pod_page_link('perlpodspec'),
    "$PERLDOC/perlpodspec", 'Simple POD link';
is $parser->resolve_pod_page_link(undef, 'SYNOPSIS'), '#SYNOPSIS',
    'Simple fragment link';
is $parser->resolve_pod_page_link(undef, 'this that'), '#this-that',
    'Fragment link with space';
is $parser->resolve_pod_page_link('perlpod', 'this that'),
    "$PERLDOC/perlpod#this-that",
    'POD link with fragment with space';

is $parser->resolve_man_page_link('crontab(5)', 'EXAMPLE CRON FILE'),
    "${MANURL}5/crontab", 'Man link with fragment';
is $parser->resolve_man_page_link('crontab(5)'),
    "${MANURL}5/crontab", 'Man link without fragment';
is $parser->resolve_man_page_link('crontab'),
    "${MANURL}1/crontab", 'Man link without section';

# Make sure that batch_mode_page_object_init() works.
ok $parser->batch_mode_page_object_init(0, 0, 0, 0, 6),
    'Call batch_mode_page_object_init()';
ok $parser->batch_mode, 'We should be in batch mode';
is $parser->batch_mode_current_level, 6,
    'The level should have been set';

######################################

sub initialize {
	$_[0] = Pod::Simple::XHTML->new ();
        $_[0]->html_header("");
        $_[0]->html_footer("");
	$_[0]->output_string( \$results ); # Send the resulting output to a string
	$_[1] = '';
	return;
}
