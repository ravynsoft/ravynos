## This -*- perl -*- script writes the Makefile for SelfLoader
# You should read "perldoc perlmodinstall" for instructions on
#  how to install modules like this.

require 5.008;
use strict;
use ExtUtils::MakeMaker;
WriteMakefile(
  'NAME' => 'SelfLoader',
  'VERSION_FROM' => 'lib/SelfLoader.pm', # finds $VERSION
  'LICENSE' => 'perl_5',
  'PREREQ_PM' => {},
  'ABSTRACT_FROM' => 'lib/SelfLoader.pm',
  'AUTHOR' => 'Steffen Mueller <smueller@cpan.org>',
  'INSTALLDIRS' => ( $] < 5.011 ? 'perl' : 'site' ),
);

__END__

