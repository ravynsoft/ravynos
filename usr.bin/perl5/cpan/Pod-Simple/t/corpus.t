# Testing a corpus of Pod files
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }

    use Config;
    if ($Config::Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") != 65) {
      print "1..0 # Skip: Encode not fully working on non-ASCII platforms at this time\n";
      exit 0;
    }
}

#use Pod::Simple::Debug (10);
use Test qw(plan ok skip);

use File::Spec;
#use utf8;
use strict;
use warnings;
my(@testfiles, %xmlfiles, %wouldxml);
#use Pod::Simple::Debug (10);
BEGIN { 

sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
        require File::Spec;
        my $updir = File::Spec->updir;
        my $dir = File::Spec->catdir($updir, 'lib', 'Pod', 'Simple', 't');
        return File::Spec->catdir ($dir, $file);
    } else {
        return $file;
    }
} 
  my @bits;
  if(-e( File::Spec::->catdir( @bits =
    source_path('corpus') ) ) )
   {
    # OK
    print "# 1Bits: @bits\n";
  } elsif( -e (File::Spec::->catdir( @bits =
    (File::Spec::->curdir, 'corpus') ) )
  ) {
    # OK
    print "# 2Bits: @bits\n";
  } elsif ( -e (File::Spec::->catdir( @bits =
    (File::Spec::->curdir, 't', 'corpus') ) )
  ) {
    # OK
    print "# 3Bits: @bits\n";
  } else {
    die "Can't find the corpusdir";
  }
  my $corpusdir = File::Spec::->catdir( @bits);
  print "#Corpusdir: $corpusdir\n";

  opendir(INDIR, $corpusdir) or die "Can't opendir corpusdir : $!";
  my @f = map File::Spec::->catfile(@bits, $_), readdir(INDIR);
  closedir(INDIR);
  my %f;
  @f{@f} = ();
  foreach my $maybetest (sort @f) {
    my $xml = $maybetest;
    $xml =~ s/\.(txt|pod)$/\.xml/is  or  next;
    $wouldxml{$maybetest} = $xml;
    push @testfiles, $maybetest;
    foreach my $x ($xml, uc($xml), lc($xml)) {
      next unless exists $f{$x};
      $xmlfiles{$maybetest} = $x;
      last;
    }
  }
  die "Too few test files (".@testfiles.")" unless @ARGV or @testfiles > 20;

  @testfiles = @ARGV if @ARGV and !grep !m/\.txt/, @ARGV;

  plan tests => (2 + 2*@testfiles - 1);
}

my $HACK = 1;
#@testfiles = ('nonesuch.txt');

ok 1;

my $skippy =  ($] < 5.008) ? "skip because perl ($]) pre-dates v5.8.0" : 0;
if($skippy) {
  print "# This is just perl v$], so I'm skipping many many tests.\n";
}

{
  my @x = @testfiles;
  print "# Files to test:\n";
  while(@x) {  print "#  ", join(' ', splice @x,0,3), "\n" }
}

require Pod::Simple::DumpAsXML;


foreach my $f (@testfiles) {
  my $xml = $xmlfiles{$f};
  if($xml) {
    print "#\n#To test $f against $xml\n";
  } else {
    print "#\n# $f has no xml to test it against\n";
  }

  my $outstring;
  eval {
    my $p = Pod::Simple::DumpAsXML->new;
    $p->output_string( \$outstring );
    $p->parse_file( $f );
    undef $p;
  };
  
  if($@) {
    my $x = "#** Couldn't parse $f:\n $@";
    $x =~ s/([\n\r]+)/\n#** /g;
    print $x, "\n";
    ok 0;
    ok 0;
    next;
  } else {
    print "# OK, parsing $f generated ", length($outstring), " bytes\n";
    ok 1;
  }
  
  die "Null outstring?" unless $outstring;
  
  next if $f =~ /nonesuch/;

  my $outfilename = ($HACK > 1) ? $wouldxml{$f} : "$wouldxml{$f}\_out";
  if($HACK) {
    open OUT, ">$outfilename" or die "Can't write-open $outfilename: $!\n";
    binmode(OUT);
    print OUT $outstring;
    close(OUT);
  }
  unless($xml) {
    print "#  (no comparison done)\n";
    ok 1;
    next;
  }
  
  open(IN, "<$xml") or die "Can't read-open $xml: $!";
  #binmode(IN);
  local $/;
  my $xmlsource = <IN>;
  close(IN);
  
  print "# There's errata!\n" if $outstring =~ m/start_line="-321"/;
  
  if(
    $xmlsource eq $outstring
    or do {
      $xmlsource =~ s/[\n\r]+/\n/g;
      $outstring =~ s/[\n\r]+/\n/g;
      $xmlsource eq $outstring;
    }
  ) {
    print "#  (Perfect match to $xml)\n";
    unlink $outfilename unless $outfilename =~ m/\.xml$/is;
    ok 1;
    next;
  }

  if($skippy) {
    skip $skippy, 0;
  } else {
    print STDERR "#  $outfilename and $xml don't match!\n";
    print STDERR `diff $xml $outfilename`;
    ok 0;
  }

}


print "#\n# I've been using Encode v",
  $Encode::VERSION ? $Encode::VERSION : "(NONE)", "\n";
print "# Byebye\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

