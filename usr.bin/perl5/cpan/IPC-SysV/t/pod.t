################################################################################
#
#  Version 2.x, Copyright (C) 2007-2013, Marcus Holland-Moritz <mhx@cpan.org>.
#  Version 1.x, Copyright (C) 1999, Graham Barr <gbarr@pobox.com>.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use strict;
use warnings;

our %Config;
BEGIN {
  if ($ENV{'PERL_CORE'}) {
    chdir 't' if -d 't';
    @INC = '../lib' if -d '../lib' && -d '../ext';
  }

  require Test::More; Test::More->import;
  require Config; Config->import;

  if ($ENV{'PERL_CORE'} && $Config{'extensions'} !~ m[\bIPC/SysV\b]) {
    plan(skip_all => 'IPC::SysV was not built');
  }
}

my @pods;

# find all potential pod files
if (open F, "MANIFEST") {
  chomp(my @files = <F>);
  close F;
  for my $f (@files) {
    next if $f =~ /ppport/;
    if (open F, $f) {
      while (<F>) {
        if (/^=\w+/) {
          push @pods, $f;
          last;
        }
      }
      close F;
    }
  }
}

# load Test::Pod if possible, otherwise load Test::More
eval {
  require Test::Pod;
  $Test::Pod::VERSION >= 0.95
      or die "Test::Pod version only $Test::Pod::VERSION";
  Test::Pod->import( tests => scalar @pods );
};

if ($@) {
  require Test::More;
  Test::More->import( skip_all => "testing pod requires Test::Pod" );
}
else {
  for my $pod (@pods) {
    pod_file_ok($pod);
  }
}

