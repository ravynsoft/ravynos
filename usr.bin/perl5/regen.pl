#!/usr/bin/perl -w
#
# regen.pl - a wrapper that runs all *.pl scripts to autogenerate files

require 5.004;	# keep this compatible, an old perl is all we may have before
                # we build the new one

# The idea is to move the regen_headers target out of the Makefile so that
# it is possible to rebuild the headers before the Makefile is available.
# (and the Makefile is unavailable until after Configure is run, and we may
# wish to make a clean source tree but with current headers without running
# anything else.

use strict;

my $tap = $ARGV[0] && $ARGV[0] eq '--tap' ? '# ' : '';
foreach my $pl (map {chomp; "regen/$_"} <DATA>) {
  my @command =  ($^X, '-I.', $pl, @ARGV);
  print "$tap@command\n";
  system @command
    and die "@command failed: $?" 
}

__END__
embed.pl
feature.pl
mg_vtable.pl
miniperlmain.pl
opcode.pl
overload.pl
reentr.pl
regcomp.pl
scope_types.pl
tidy_embed.pl
warnings.pl
