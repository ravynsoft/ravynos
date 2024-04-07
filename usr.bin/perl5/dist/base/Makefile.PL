use 5.008;
use strict;
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME            => 'base',
    VERSION_FROM    => "lib/base.pm", # finds $VERSION
    PREREQ_PM       => { 'Test::More' => 0.40 },
    ($] > 5.011) ? () : ( INSTALLDIRS => 'perl' ), # CPAN sourced versions should now install to site

);

