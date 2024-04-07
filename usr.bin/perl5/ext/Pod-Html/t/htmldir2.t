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
    podstub => "htmldir2",
    description => "test --htmldir and --htmlroot 2a",
    expect => $expect_raw,
    p2h => {
        podpath => 't',
        htmldir => 't',
        quiet   => 1,
    },
    debug => $debug,
};
xconvert($args);

$args = {
    podstub => "htmldir2",
    description => "test --htmldir and --htmlroot 2b",
    expect => $expect_raw,
    p2h => {
        podpath => 't',
        quiet   => 1,
    },
    debug => $debug,
};
xconvert($args);

my $cwd = cwd();
$args = {
    podstub => "htmldir2",
    description => "test --htmldir and --htmlroot 2c",
    expect => $expect_raw,
    p2h => {
        podpath     => 't',
        podroot     => $cwd,
        norecurse   => 1, # testing --norecurse, too
        quiet       => 1,
    },
    debug   => $debug,
};
xconvert($args);

done_testing;

__DATA__
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>htmldir - Test --htmldir feature</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:[PERLADMIN]" />
</head>

<body>



<ul id="index">
  <li><a href="#NAME">NAME</a></li>
  <li><a href="#LINKS">LINKS</a></li>
</ul>

<h1 id="NAME">NAME</h1>

<p>htmldir - Test --htmldir feature</p>

<h1 id="LINKS">LINKS</h1>

<p>Normal text, a <a>link</a> to nowhere,</p>

<p>a link to <a>perlvar-copy</a>,</p>

<p><a href="/t/htmlescp.html">htmlescp</a>,</p>

<p><a href="/t/feature.html#Another-Head-1">&quot;Another Head 1&quot; in feature</a>,</p>

<p>and another <a href="/t/feature.html#Another-Head-1">&quot;Another Head 1&quot; in feature</a>.</p>


</body>

</html>


