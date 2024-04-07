BEGIN {
    use File::Spec::Functions ':ALL';
    @INC = map { rel2abs($_) }
             (qw| ./lib ./t/lib ../../lib |);
}

use strict;
use warnings;
use Test::More;
use Testing qw( setup_testing_dir xconvert );
use Cwd;

my $debug = 0;
my $startdir = cwd();
END { chdir($startdir) or die("Cannot change back to $startdir: $!"); }
my ($expect_raw, $args);
{ local $/; $expect_raw = <DATA>; }

my $tdir = setup_testing_dir( {
    debug       => $debug,
} );

$args = {
    podstub => "htmlview",
    description => "html rendering",
    expect => $expect_raw,
    p2h => {
        podpath => 't',
        quiet  => 1,
    },
};

xconvert($args);

done_testing;

__DATA__
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Test HTML Rendering</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:[PERLADMIN]" />
</head>

<body>



<ul id="index">
  <li><a href="#NAME">NAME</a></li>
  <li><a href="#SYNOPSIS">SYNOPSIS</a></li>
  <li><a href="#DESCRIPTION">DESCRIPTION</a></li>
  <li><a href="#METHODS-OTHER-STUFF">METHODS =&gt; OTHER STUFF</a>
    <ul>
      <li><a href="#new">new()</a></li>
      <li><a href="#old">old()</a></li>
    </ul>
  </li>
  <li><a href="#TESTING-FOR-AND-BEGIN">TESTING FOR AND BEGIN</a></li>
  <li><a href="#TESTING-URLs-hyperlinking">TESTING URLs hyperlinking</a></li>
  <li><a href="#SEE-ALSO">SEE ALSO</a></li>
  <li><a href="#POD-ERRORS">POD ERRORS</a></li>
</ul>

<h1 id="NAME">NAME</h1>

<p>Test HTML Rendering</p>

<h1 id="SYNOPSIS">SYNOPSIS</h1>

<pre><code>use My::Module;

my $module = My::Module-&gt;new();</code></pre>

<h1 id="DESCRIPTION">DESCRIPTION</h1>

<p>This is the description.</p>

<pre><code>Here is a verbatim section.</code></pre>

<p>This is some more regular text.</p>

<p>Here is some <b>bold</b> text, some <i>italic</i> and something that looks like an &lt;html&gt; tag. This is some <code>$code($arg1)</code>.</p>

<p>This <code>text contains embedded <b>bold</b> and <i>italic</i> tags</code>. These can be nested, allowing <b>bold and <i>bold &amp; italic</i> text</b>. The module also supports the extended <b>syntax</b> and permits <i>nested tags &amp; other <b>cool</b> stuff</i></p>

<h1 id="METHODS-OTHER-STUFF">METHODS =&gt; OTHER STUFF</h1>

<p>Here is a list of methods</p>

<h2 id="new">new()</h2>

<p>Constructor method. Accepts the following config options:</p>

<dl>

<dt id="foo">foo</dt>
<dd>

<p>The foo item.</p>

</dd>
<dt id="bar">bar</dt>
<dd>

<p>The bar item.</p>

<ul>

<p>This is a list within a list</p>

<p>*</p>

<p>The wiz item.</p>

<p>*</p>

<p>The waz item.</p>

</ul>

</dd>
<dt id="baz">baz</dt>
<dd>

<p>The baz item.</p>

<ul>

<li><p>A correct list within a list</p>

</li>
<li><p>Boomerang</p>

</li>
</ul>

</dd>
</dl>

<p>Title on the same line as the =item + * bullets</p>

<ul>

<li><p><code>Black</code> Cat</p>

</li>
<li><p>Sat <span style="white-space: nowrap;"><i>on</i> the</span></p>

</li>
<li><p>Mat&lt;!&gt;</p>

</li>
</ul>

<p>Title on the same line as the =item + numerical bullets</p>

<ol>

<li><p>Cat</p>

</li>
<li><p>Sat</p>

</li>
<li><p>Mat</p>

</li>
</ol>

<p>Numbered list with text on the same line</p>

<dl>

<dt id="Cat">1 Cat</dt>
<dd>

</dd>
<dt id="Sat">2 Sat</dt>
<dd>

</dd>
<dt id="Mat">3 Mat</dt>
<dd>

</dd>
</dl>

<p>No bullets, no title</p>

<ul>

<li><p>Cat</p>

</li>
<li><p>Sat</p>

</li>
<li><p>Mat</p>

</li>
</ul>

<h2 id="old">old()</h2>

<p>Destructor method</p>

<h1 id="TESTING-FOR-AND-BEGIN">TESTING FOR AND BEGIN</h1>



<br />
<p>
blah blah
</p>

<p>intermediate text</p>

<more>
HTML
</more>

some text

<h1 id="TESTING-URLs-hyperlinking">TESTING URLs hyperlinking</h1>

<p>This is an href link1: http://example.com</p>

<p>This is an href link2: http://example.com/foo/bar.html</p>

<p>This is an email link: mailto:foo@bar.com</p>

<pre><code>This is a link in a verbatim block &lt;a href=&quot;http://perl.org&quot;&gt; Perl &lt;/a&gt;</code></pre>

<h1 id="SEE-ALSO">SEE ALSO</h1>

<p>See also <a href="/t/htmlescp.html">Test Page 2</a>, the <a>Your::Module</a> and <a>Their::Module</a> manpages and the other interesting file <i>/usr/local/my/module/rocks</i> as well.</p>

<h1 id="POD-ERRORS">POD ERRORS</h1>

<p>Hey! <b>The above document had some coding errors, which are explained below:</b></p>

<dl>

<dt id="Around-line-45">Around line 45:</dt>
<dd>

<p>You can&#39;t have =items (as at line 49) unless the first thing after the =over is an =item</p>

</dd>
</dl>


</body>

</html>


