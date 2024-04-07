BEGIN {
    chdir 't' if -d 't';
    if($ENV{PERL_CORE}) {
        @INC = '../lib';
    }
}

use lib '../lib';

use strict;
use warnings;
use Test;
BEGIN { plan tests => 5 };

sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
        require File::Spec;
        my $updir = File::Spec->updir;
        my $dir = File::Spec->catdir ($updir, 'lib', 'Pod', 'Simple', 't');
        return File::Spec->catfile ($dir, $file);
    } else {
        return $file;
    }
}

use Pod::Simple::Text;
$Pod::Simple::Text::FREAKYMODE = 1;

my $parser  = Pod::Simple::Text->new();
 
foreach my $file (
  "junk1.pod",
  "junk2.pod",
  "perlcyg.pod",
  "perlfaq.pod",
  "perlvar.pod",
) {

  unless(-e source_path($file)) {
    ok 0;
    print "# But $file doesn't exist!!\n";
    next;
  }

    my $precooked = $file;
    my $outstring;
    my $compstring;
    $precooked =~ s<\.pod><o.txt>s;
    $parser->reinit;
    $parser->output_string(\$outstring);
    $parser->parse_file(source_path($file));

    open(IN, $precooked) or die "Can't read-open $precooked: $!";
    {
      local $/;
      $compstring = <IN>;
    }
    close(IN);

    for ($outstring,$compstring) { s/\s+/ /g; s/^\s+//s; s/\s+$//s; }

    if($outstring eq $compstring) {
      ok 1;
      next;
    } elsif( do{
      for ($outstring, $compstring) { tr/ //d; };
      $outstring eq $compstring;
    }){
      print "# Differ only in whitespace.\n";
      ok 1;
      next;
    } else {
    
      my $x = $outstring ^ $compstring;
      $x =~ m/^(\x00*)/s or die;
      my $at = length($1);
      print "# Difference at byte $at...\n";
      if($at > 10) {
        $at -= 5;
      }
      {
        print "# ", substr($outstring,$at,20), "\n";
        print "# ", substr($compstring,$at,20), "\n";
        print "#      ^...";
      }
    
      ok 0;
      printf "# Unequal lengths %s and %s\n", length($outstring), length($compstring);
      next;
    }
  }
