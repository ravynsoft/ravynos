use strict;
use warnings;

use ExtUtils::MakeMaker;

my %WriteMakefileArgs = (
  "NAME" => "Search::Dict",
  "VERSION_FROM" => "lib/Search/Dict.pm",
  "ABSTRACT_FROM" => "lib/Search/Dict.pm",
  "AUTHOR" => "The Perl 5 Porters",
  "EXE_FILES" => [],
  "LICENSE" => "perl",
  "PREREQ_PM" => {
    "Exporter" => 0,
    "Test::More" => 0.47,
    "Tie::Handle" => 0,
  },
  INSTALLDIRS => ( $] lt '5.012' ? 'perl' : 'site' ),
);

WriteMakefile(%WriteMakefileArgs);

