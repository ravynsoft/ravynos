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
    podstub => "htmllink",
    description => "html links",
    expect => $expect_raw,
    debug => 1,
};

xconvert($args);

done_testing;

__DATA__
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>htmllink - Test HTML links</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:[PERLADMIN]" />
</head>

<body>



<ul id="index">
  <li><a href="#NAME">NAME</a></li>
  <li><a href="#LINKS">LINKS</a></li>
  <li><a href="#TARGETS">TARGETS</a>
    <ul>
      <li><a href="#section1">section1</a></li>
      <li><a href="#section-2">section 2</a></li>
      <li><a href="#section-three">section three</a></li>
    </ul>
  </li>
</ul>

<h1 id="NAME">NAME</h1>

<p>htmllink - Test HTML links</p>

<h1 id="LINKS">LINKS</h1>

<p><a href="#section1">&quot;section1&quot;</a></p>

<p><a href="#section-2">&quot;section 2&quot;</a></p>

<p><a href="#section-three">&quot;section three&quot;</a></p>

<p><a href="#item1">&quot;item1&quot;</a></p>

<p><a href="#item-2">&quot;item 2&quot;</a></p>

<p><a href="#item-three">&quot;item three&quot;</a></p>

<p><a href="#section1">&quot;section1&quot;</a></p>

<p><a href="#section-2">&quot;section 2&quot;</a></p>

<p><a href="#section-three">&quot;section three&quot;</a></p>

<p><a href="#item1">&quot;item1&quot;</a></p>

<p><a href="#item-2">&quot;item 2&quot;</a></p>

<p><a href="#item-three">&quot;item three&quot;</a></p>

<p><a href="#section1">&quot;section1&quot;</a></p>

<p><a href="#section-2">&quot;section 2&quot;</a></p>

<p><a href="#section-three">&quot;section three&quot;</a></p>

<p><a href="#item1">&quot;item1&quot;</a></p>

<p><a href="#item-2">&quot;item 2&quot;</a></p>

<p><a href="#item-three">&quot;item three&quot;</a></p>

<p><a href="#section1">text</a></p>

<p><a href="#section-2">text</a></p>

<p><a href="#section-three">text</a></p>

<p><a href="#item1">text</a></p>

<p><a href="#item-2">text</a></p>

<p><a href="#item-three">text</a></p>

<p><a href="#section1">text</a></p>

<p><a href="#section-2">text</a></p>

<p><a href="#section-three">text</a></p>

<p><a href="#item1">text</a></p>

<p><a href="#item-2">text</a></p>

<p><a href="#item-three">text</a></p>

<p><a href="#section1">text</a></p>

<p><a href="#section-2">text</a></p>

<p><a href="#section-three">text</a></p>

<p><a href="#item1">text</a></p>

<p><a href="#item-2">text</a></p>

<p><a href="#item-three">text</a></p>

<h1 id="TARGETS">TARGETS</h1>

<h2 id="section1">section1</h2>

<p>This is section one.</p>

<h2 id="section-2">section 2</h2>

<p>This is section two.</p>

<h2 id="section-three">section three</h2>

<p>This is section three.</p>

<dl>

<dt id="item1">item1  </dt>
<dd>

<p>This is item one.</p>

</dd>
<dt id="item-2">item 2  </dt>
<dd>

<p>This is item two.</p>

</dd>
<dt id="item-three">item three  </dt>
<dd>

<p>This is item three.</p>

</dd>
</dl>


</body>

</html>


