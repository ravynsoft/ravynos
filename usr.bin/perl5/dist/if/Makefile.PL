use ExtUtils::MakeMaker;
WriteMakefile(
    NAME            => 'if',
    VERSION_FROM    => 'if.pm', # finds $VERSION
    ABSTRACT_FROM   => 'if.pm',
    # core since 5.7.3
    (($] >= 5.007003 && $] < 5.011 ) ? (
        'INSTALLDIRS'  => 'perl') : ()
    ),
    AUTHOR  => 'Ilya Zakharevich <ilyaz@cpan.org>',
    LICENSE => 'perl_5',

    META_MERGE => {
        'meta-spec' => { version => 2 },
        dynamic_config => 0,
        resources => {
            repository => {
                type => 'git',
                url => 'https://github.com/perl/perl5.git',
                web => 'https://github.com/perl/perl5',
            },
            bugtracker => {
                web => 'https://github.com/Perl/perl5/issues',
            },
        },
    },
);
