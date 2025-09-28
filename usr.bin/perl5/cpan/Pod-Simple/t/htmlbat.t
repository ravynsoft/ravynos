# Testing HTMLBatch
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

# Time-stamp: "2004-05-24 02:07:47 ADT"
use strict;
use warnings;
my $DEBUG = 0;

#sub Pod::Simple::HTMLBatch::DEBUG () {5};

use Test;
BEGIN { plan tests => 17 }

require Pod::Simple::HTMLBatch;;

use File::Spec;
use Cwd;
my $cwd = cwd();
print "# CWD: $cwd\n" if $DEBUG;

my $t_dir;
my $corpus_dir;

foreach my $t_maybe (
  File::Spec->catdir( File::Spec->updir(), 'lib','Pod','Simple','t'),
  File::Spec->catdir( $cwd ),
  File::Spec->catdir( $cwd, 't' ),
  'OHSNAP'
) {
  die "Can't find the test corpus" if $t_maybe eq 'OHSNAP';
  next unless -e $t_maybe;

  $t_dir = $t_maybe;
  $corpus_dir = File::Spec->catdir( $t_maybe, 'testlib1' );
  next unless -e $corpus_dir;
  last;
}
print "# OK, found the test corpus as $corpus_dir\n" if $DEBUG;
ok 1;

my $outdir;
while(1) {
  my $rand = sprintf "%05x", rand( 0x100000 );
  $outdir = File::Spec->catdir( $t_dir, "delme-$rand-out" );
  last unless -e $outdir;
}

END {
    use File::Path;
    rmtree $outdir, 0, 0;
}

ok 1;
print "# Output dir: $outdir\n" if $DEBUG;

mkdir $outdir, 0777 or die "Can't mkdir $outdir: $!";

print "# Converting $corpus_dir => $outdir\n" if $DEBUG;
my $conv = Pod::Simple::HTMLBatch->new;
$conv->verbose(0);
$conv->index(1);
$conv->batch_convert( [$corpus_dir], $outdir );
ok 1;
print "# OK, back from converting.\n" if $DEBUG;

my @files;
use File::Find;
find( sub {
      push @files, $File::Find::name;
      if (/[.]html$/ && $_ !~ /perl|index/) {
          # Make sure an index was generated.
          open HTML, $_ or die "Cannot open $_: $!\n";
          my $html = do { local $/; <HTML> };
          close HTML;
          ok $html =~ /<div class='indexgroup'>/;
      }
      return;
}, $outdir );

{
  my $long = ( grep m/zikzik\./i, @files )[0];
  ok($long) or print "# How odd, no zikzik file in $outdir!?\n";
  if($long) {
    $long =~ s{zikzik\.html?$}{}s;
    for(@files) { substr($_, 0, length($long)) = '' }
    @files = grep length($_), @files;
  }
}

if ($DEBUG) {
    print "#Produced in $outdir ...\n";
    foreach my $f (sort @files) {
        print "#   $f\n";
    }
    print "# (", scalar(@files), " items total)\n";
}

# Some minimal sanity checks:
ok scalar(grep m/\.css/i, @files) > 5;
ok scalar(grep m/\.html?/i, @files) > 5;
ok scalar grep m{squaa\W+Glunk.html?}i, @files;

if (my @long = grep { /^[^.]{9,}/ } map { s{^[^/]/}{} } @files) {
    ok 0;
    print "#    File names too long:\n",
        map { "#         $_\n" } @long;
} else {
    ok 1;
}

# use Pod::Simple;
# *pretty = \&Pod::Simple::BlackBox::pretty;

print "# Bye from ", __FILE__, "\n" if $DEBUG;
ok 1;
