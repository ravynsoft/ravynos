use strict;
use warnings;
use ExtUtils::MakeMaker;
WriteMakefile(
  NAME          => 'encoding::warnings',
  VERSION_FROM  => 'lib/encoding/warnings.pm',
  ABSTRACT_FROM => 'lib/encoding/warnings.pm',
  INSTALLDIRS   => ( $] >= 5.009002 && $] < 5.011 ? 'perl' : 'site' ),
  LICENSE       => 'perl_5',
  TEST_REQUIRES => {
    'Test::More' => 0,
  },
  PREREQ_PM     => {},
  AUTHOR        => 'Audrey Tang <cpan@audreyt.org>',
  META_MERGE    => {
    resources => {
      repository  => 'https://github.com/perl/perl5.git',
      bugtracker  => 'https://github.com/perl/perl5/issues',
      MailingList => 'https://lists.perl.org/list/perl5-porters.html',
    },
  },
);
