BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Pod::Simple::Search;
use Test;
BEGIN { plan tests => 4 }

print "# ", __FILE__,
 ": Testing limit_glob ...\n";

my $x = Pod::Simple::Search->new;
die "Couldn't make an object!?" unless ok defined $x;

$x->inc(0);
$x->shadows(1);

use File::Spec;
use Cwd;
my $cwd = cwd();
print "# CWD: $cwd\n";

sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
        return "../lib/Pod/Simple/t/$file";
    } else {
        return $file;
    }
}

my($here1, $here2, $here3);

if(        -e ($here1 = source_path(  'testlib1'      ))) {
  die "But where's $here2?"
    unless -e ($here2 = source_path(  'testlib2'));
  die "But where's $here3?"
    unless -e ($here3 = source_path(  'testlib3'));

} elsif(   -e ($here1 = File::Spec->catdir($cwd, 't', 'testlib1'      ))) {
  die "But where's $here2?"
    unless -e ($here2 = File::Spec->catdir($cwd, 't', 'testlib2'));
  die "But where's $here3?"
    unless -e ($here3 = File::Spec->catdir($cwd, 't', 'testlib3'));

} else {
  die "Can't find the test corpora";
}
print "# OK, found the test corpora\n#  as $here1\n# and $here2\n# and $here3\n#\n";
ok 1;

print $x->_state_as_string;
#$x->verbose(12);

use Pod::Simple;
*pretty = \&Pod::Simple::BlackBox::pretty;

my $glob = '*z*k*';
print "# Limiting to $glob\n";
$x->limit_glob($glob);

my($name2where, $where2name) = $x->survey($here1, $here2, $here3);

my $p = pretty( $where2name, $name2where )."\n";
$p =~ s/, +/,\n/g;
$p =~ s/^/#  /mg;
print $p;

my $ascii_order;
if(     -e ($ascii_order = source_path('ascii_order.pl'))) {
  #
} elsif(-e ($ascii_order = File::Spec->catfile($cwd, 't', 'ascii_order.pl'))) {
  #
} else {
  die "Can't find ascii_order.pl";
}

require $ascii_order;

{
my $names = join "|", sort ascii_order values %$where2name;
ok $names, "Zonk::Pronk|perlzuk|zikzik";
}


print "# OK, bye from ", __FILE__, "\n";
ok 1;

__END__

