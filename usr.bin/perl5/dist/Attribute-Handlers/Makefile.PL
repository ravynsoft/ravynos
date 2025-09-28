use 5.006;
use strict;
use ExtUtils::MakeMaker;
WriteMakefile(
  NAME          => 'Attribute::Handlers',
  VERSION_FROM  => 'lib/Attribute/Handlers.pm',
  ABSTRACT_FROM => 'lib/Attribute/Handlers.pm',
  INSTALLDIRS   => ( $] < 5.011 ? 'perl' : 'site' ),
  LICENSE       => 'perl_5',
  PREREQ_PM => {
    'parent' => 0,
  },
  AUTHOR        => 'Damian Conway',
  META_MERGE => {
    resources => {
      repository  => 'https://github.com/perl/perl5.git',
      bugtracker  => 'https://github.com/perl/perl5/issues',
      MailingList => 'https://lists.perl.org/list/perl5-porters.html',
    },
    no_index => {
      directory => [ 'demo' ],
    },
  },
);
