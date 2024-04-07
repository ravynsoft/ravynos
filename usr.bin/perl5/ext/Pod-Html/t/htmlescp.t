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
    podstub => "htmlescp",
    description => "html escape",
    expect => $expect_raw,
    debug => $debug,
};

xconvert($args);

done_testing;

__DATA__
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Escape Sequences Test: Recalling the Glory &quot;&lt;&amp;&amp;&amp;&gt;&quot; of Ampersands</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:[PERLADMIN]" />
</head>

<body>



<ul id="index">
  <li><a href="#NAME">NAME</a></li>
  <li><a href="#DESCRIPTION">DESCRIPTION</a></li>
</ul>

<h1 id="NAME">NAME</h1>

<p>Escape Sequences Test: Recalling the Glory &quot;&lt;&amp;&amp;&amp;&gt;&quot; of Ampersands</p>

<h1 id="DESCRIPTION">DESCRIPTION</h1>

<p>I am a stupid fool who puts naked &lt; &amp; &gt; characters in my POD instead of escaping them as &lt; and &gt;.</p>

<p>Here is some <b>bold</b> text, some <i>italic</i> plus <i>/etc/fstab</i> file and something that looks like an &lt;html&gt; tag. This is some <code>$code($arg1)</code>.</p>

<p>Some numeric escapes: P e r l</p>


</body>

</html>


