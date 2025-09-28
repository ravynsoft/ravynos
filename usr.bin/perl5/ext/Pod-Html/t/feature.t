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

my $cwd = cwd();

$args = {
    podstub => "feature",
    description => "misc pod-html features",
    expect => $expect_raw,
    p2h => {
        backlink        => 1,
        css             => 'style.css',
        header          => 1, # no styling b/c of --ccs
        htmldir         => catdir($cwd, 't'),
        noindex         => 1,
        podpath         => 't',
        podroot         => $cwd,
        title           => 'a title',
        quiet           => 1,
    },
    debug => $debug,
};
xconvert($args);

done_testing;

__DATA__
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>a title</title>
<link rel="stylesheet" href="style.css" type="text/css" />
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:[PERLADMIN]" />
</head>

<body id="_podtop_">
<table border="0" width="100%" cellspacing="0" cellpadding="3">
<tr><td class="_podblock_" valign="middle">
<big><strong><span class="_podblock_">&nbsp;a title</span></strong></big>
</td></tr>
</table>



<a href="#_podtop_"><h1 id="Head-1">Head 1</h1></a>

<p>A paragraph</p>



some html

<p>Another paragraph</p>

<a href="#_podtop_"><h1 id="Another-Head-1">Another Head 1</h1></a>

<p>some text and a link <a href="t/crossref.html">crossref</a></p>

<table border="0" width="100%" cellspacing="0" cellpadding="3">
<tr><td class="_podblock_" valign="middle">
<big><strong><span class="_podblock_">&nbsp;a title</span></strong></big>
</td></tr>
</table>

</body>

</html>


