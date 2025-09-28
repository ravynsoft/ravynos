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
BEGIN { plan tests => 11 }

print "# ", __FILE__,
 ": Testing the scanning of several (well, two) docroots...\n";

my $x = Pod::Simple::Search->new;
die "Couldn't make an object!?" unless ok defined $x;

$x->inc(0);

$x->callback(sub {
  print "#  ", join("  ", map "{$_}", @_), "\n";
  return;
});

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

my($here1, $here2);
if(        -e ($here1 = source_path('testlib1'))) {
  die "But where's $here2?"
    unless -e ($here2 = source_path('testlib2'));
} elsif(   -e ($here1 = File::Spec->catdir($cwd, 't', 'testlib1'      ))) {
  die "But where's $here2?"
    unless -e ($here2 = File::Spec->catdir($cwd, 't', 'testlib2'));
} else {
  die "Can't find the test corpora";
}
print "# OK, found the test corpora\n#  as $here1\n# and $here2\n";
ok 1;

print $x->_state_as_string;
#$x->verbose(12);

use Pod::Simple;
*pretty = \&Pod::Simple::BlackBox::pretty;

print "# OK, starting run...\n# [[\n";
my($name2where, $where2name) = $x->survey($here1, $here2);
print "# ]]\n#OK, run done.\n";

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
skip $^O eq 'VMS' ? '-- case may or may not be preserved' : 0,
     $names,
     "Blorm|Suzzle|Zonk::Pronk|hinkhonk::Glunk|hinkhonk::Vliff|perlflif|perlthng|perlzoned|perlzuk|squaa|squaa::Glunk|squaa::Vliff|squaa::Wowo|zikzik";
}

{
my $names = join "|", sort ascii_order keys %$name2where;
skip $^O eq 'VMS' ? '-- case may or may not be preserved' : 0,
     $names,
     "Blorm|Suzzle|Zonk::Pronk|hinkhonk::Glunk|hinkhonk::Vliff|perlflif|perlthng|perlzoned|perlzuk|squaa|squaa::Glunk|squaa::Vliff|squaa::Wowo|zikzik";
}

ok( ($name2where->{'squaa'} || 'huh???'), '/squaa\.pm$/');

ok grep( m/squaa\.pm/, keys %$where2name ), 1;

###### Now with recurse(0)

$x->recurse(0);

print "# OK, starting run without recurse...\n# [[\n";
($name2where, $where2name) = $x->survey($here1, $here2);
print "# ]]\n#OK, run without recurse done.\n";

$p = pretty( $where2name, $name2where )."\n";
$p =~ s/, +/,\n/g;
$p =~ s/^/#  /mg;
print $p;

{
my $names = join "|", sort ascii_order values %$where2name;
skip $^O eq 'VMS' ? '-- case may or may not be preserved' : 0, 
     $names, 
     "Blorm|Suzzle|squaa|zikzik";
}

{
my $names = join "|", sort ascii_order keys %$name2where;
skip $^O eq 'VMS' ? '-- case may or may not be preserved' : 0, 
     $names, 
     "Blorm|Suzzle|squaa|zikzik";
}

ok( ($name2where->{'squaa'} || 'huh???'), '/squaa\.pm$/');

ok grep( m/squaa\.pm/, keys %$where2name ), 1;

ok 1;

__END__

