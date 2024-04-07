#!/usr/bin/perl -w
use strict;
use warnings;
use File::Spec;
use Test::More tests =>  1;
use lib (-d 't' ? File::Spec->catdir(qw(t lib)) : 'lib');
use ExtUtils::ParseXS qw(process_file);

chdir('t') if -d 't';

# Module-Build uses ExtUtils::ParseXS with $^W set, try to avoid
# warning in that case.

{
  my $out;
  open my $out_fh, ">", \$out;
  my @warnings;
  local $SIG{__WARN__} = sub { push @warnings, "@_" };
  process_file(filename => "XSWarn.xs", output => $out_fh);
  is_deeply(\@warnings, [], "shouldn't be any warnings");
}
