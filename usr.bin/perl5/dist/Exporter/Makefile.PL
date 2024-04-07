use 5.006;
use ExtUtils::MakeMaker;

my $EUMM_VERSION = $ExtUtils::MakeMaker::VERSION;
$EUMM_VERSION =~ s/_//g;

WriteMakefile(
    NAME         => 'Exporter',
    VERSION_FROM => 'lib/Exporter.pm',
    ( $] > 5.011 ) ? () : ( INSTALLDIRS => 'perl' ),    # CPAN sourced versions should now install to site
    PREREQ_PM => {
        'Carp' => '1.05',
    },
    ABSTRACT_FROM => 'lib/Exporter.pm',
    ( $EUMM_VERSION >= 6.31 ? ( LICENSE => 'perl_5' ) : () ),
    dist       => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
    META_MERGE => {
        recommends => {
            'Test::Pod'           => 1.18,
            'Test::Pod::Coverage' => 1.04
        },
        resources => {
            license     => 'http://dev.perl.org/licenses/',
            bugtracker  => 'https://github.com/Perl/perl5/issues',
            repository  => 'https://github.com/Perl/perl5/tree/blead/dist/Exporter',
            MailingList => 'http://lists.perl.org/list/perl5-porters.html',
        },
    },
);
