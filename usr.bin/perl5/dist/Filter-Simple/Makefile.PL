require 5.006; # uses 'our'
use strict;
use ExtUtils::MakeMaker;
WriteMakefile(
    'NAME'          => 'Filter::Simple',
    'VERSION_FROM'  => 'lib/Filter/Simple.pm',
    'INSTALLDIRS'   => 'perl',
    'LICENSE'       => 'perl_5',
    'INSTALLDIRS'   => ( $] < 5.011 ? 'perl' : 'site' ),
    'ABSTRACT_FROM' => 'lib/Filter/Simple.pm',
    'AUTHOR'        => 'Damian Conway',
    'PREREQ_PM'     => {
        'Text::Balanced'     => '1.97',
        'Filter::Util::Call' => 0,
    },
);
