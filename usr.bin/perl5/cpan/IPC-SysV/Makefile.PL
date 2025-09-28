################################################################################
#
#  Version 2.x, Copyright (C) 2007-2013, Marcus Holland-Moritz <mhx@cpan.org>.
#  Version 1.x, Copyright (C) 1999, Graham Barr <gbarr@pobox.com>.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

require 5.004_05;

use strict;
use ExtUtils::MakeMaker;

unless ($ENV{'PERL_CORE'}) {
  $ENV{'PERL_CORE'} = 1 if grep { $_ eq 'PERL_CORE=1' } @ARGV;
}

if ($^O eq 'MSWin32') {
  die "OS unsupported\n";
}

WriteMakefile(
  NAME         => 'IPC::SysV',
  VERSION_FROM => 'lib/IPC/SysV.pm',
  PREREQ_PM    => {
                    'Test::More' => 0.45,
                  },
  CONFIGURE    => \&configure,
);

sub configure
{
  my @moreopts;
  my %depend;

  if (eval $ExtUtils::MakeMaker::VERSION >= 6) {
    push @moreopts, AUTHOR => 'Marcus Holland-Moritz <mhx@cpan.org>',
                    ABSTRACT_FROM => 'lib/IPC/SysV.pm';
  }

  if (eval $ExtUtils::MakeMaker::VERSION >= 6.30_01) {
    print "Setting license tag...\n";
    push @moreopts, LICENSE => 'perl';
  }

  if ($ENV{'PERL_CORE'}) {
    # clean out const-* files in the core
    push @moreopts, realclean => { FILES => "const-c.inc const-xs.inc" },
                    DEFINE => '-DNO_PPPORT_H';
  }
  else {
    # IPC::SysV is in the core since 5.005
    # 5.11.0+ has site before perl
    push @moreopts, INSTALLDIRS => (
      ($] >= 5.005 and $] < 5.011)
        ? 'perl'
        : 'site'
    );
  }

  $depend{'SysV.xs'} = 'const-c.inc const-xs.inc';

  return {
    depend => \%depend,
    @moreopts
  };
}


#--- MY package

sub MY::libscan
{
  package MY;
  my($self, $path) = @_;
  return $path if $self->SUPER::libscan($path) and
                  $path !~ m! [~%]$
                            | \.(cache\.cm|swp|orig|rej)$
                            | regen\.pl$
                            !x;
  return '';
}

sub MY::postamble
{
  package MY;
  my $post = shift->SUPER::postamble(@_);
  $post .= <<'POSTAMBLE';

purge_all: realclean
	@$(RM_F) const-c.inc const-xs.inc

regen:
	$(PERL) -I$(PERL_ARCHLIB) -I$(PERL_LIB) regen.pl

const-c.inc: lib/IPC/SysV.pm regen.pl
	@$(MAKE) regen

const-xs.inc: lib/IPC/SysV.pm regen.pl
	@$(MAKE) regen

POSTAMBLE
  return $post;
}

