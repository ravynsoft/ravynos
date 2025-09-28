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
 ": Testing forced case sensitivity ...\n";

my $x = Pod::Simple::Search->new;
die "Couldn't make an object!?" unless ok defined $x;

$x->inc(0);
$x->is_case_insensitive(0);

use File::Spec;
use Cwd;
my $cwd = cwd();
print "# CWD: $cwd\n";

sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
        my $updir = File::Spec->updir;
        my $dir = File::Spec->catdir($updir, 'lib', 'Pod', 'Simple', 't');
        return File::Spec->catdir ($dir, $file);
    } else {
        return $file;
    }
}

my($A, $B);

if(        -e ($A = source_path(  'search60/A'      ))) {
  die "But where's $B?"
    unless -e ($B = source_path(  'search60/B'));
} elsif(   -e ($A = File::Spec->catdir($cwd, 't', 'search60', 'A'      ))) {
  die "But where's $B?"
    unless -e ($B = File::Spec->catdir($cwd, 't', 'search60', 'B'));
} else {
  die "Can't find the test corpora";
}
print "# OK, found the test corpora\n#  as $A\n# and $B\n#\n";
ok 1;

my($name2where, $where2name) = $x->survey($A, $B);

ok ($name2where->{x} =~ m{^\Q$A\E[\\/]x\.pod$});

ok ($name2where->{X} =~ m{^\Q$B\E[\\/]X\.pod$});
