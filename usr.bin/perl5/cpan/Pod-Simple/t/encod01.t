# encoding nonesuch
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

#use Pod::Simple::Debug (10);
use Test;
use File::Spec;
#use utf8;
use strict;
use warnings;
#use Pod::Simple::Debug (10);

BEGIN { plan tests => 6 }

use Pod::Simple;
use Pod::Simple::DumpAsXML;

my $thefile;


BEGIN { 

  # Find the path to the test source files.  This requires some fiddling when
# these tests are run as part of Perl core.
sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
        require File::Spec;
        my $updir = File::Spec->updir;
        my $dir = File::Spec->catdir ($updir, 'lib', 'Pod', 'Simple', 't', 'corpus');
        return File::Spec->catfile ($dir, $file);
    } else {
        return $file;
    }
}
  if( -e
    ($thefile = source_path('nonesuch.txt'))
   #or (print("# Nope, not $thefile\n"), 0)
  ) {
    # okay,

  } elsif( -e
    ($thefile = File::Spec::->catfile( File::Spec::->curdir, 'corpus', 'nonesuch.txt' ))
   #or (print("# Nope, not $thefile\n"), 0)
  ) {
    # okay,
  } elsif (-e
    ($thefile = File::Spec::->catfile( File::Spec::->curdir, 't', 'corpus', 'nonesuch.txt' ))
   #or (print("# Nope, not $thefile\n"), 0)
  ) {
    # okay,
  } else {
    die "Can't find the corpus directory\n Aborting";
  }

}

print "# Testing that $thefile parses right.\n";
my $outstring;
{
  my $p = Pod::Simple::DumpAsXML->new;
  $p->output_string( \$outstring );
  $p->parse_file( $thefile );
  undef $p;
}
ok 1 ; # make sure it parsed at all
ok( $outstring && length($outstring) ); # make sure it parsed to something.
#print $outstring;
ok( $outstring =~ m/Blorp/ );
ok( $outstring =~ m/errata/ );
ok( $outstring =~ m/unsupported/ );
ok 1;
