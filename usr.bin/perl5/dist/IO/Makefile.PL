# This -*- perl -*- script makes the Makefile

BEGIN { require 5.008_001 }
use ExtUtils::MakeMaker;
use Config qw(%Config);
my $PERL_CORE = grep { $_ eq 'PERL_CORE=1' } @ARGV;

#--- Attempt to find <poll.h>

my $define = "";

unless ( $PERL_CORE or exists $Config{'i_poll'} ) {
    my @inc = split( /\s+/, join( " ", $Config{'usrinc'}, $Config{'incpth'}, $Config{'locincpth'} ) );
    foreach my $path (@inc) {
        if ( -f $path . "/poll.h" ) {
            $define .= "-DI_POLL ";
            last;
        }
    }
}

#--- Write the Makefile

WriteMakefile(
    VERSION_FROM => "IO.pm",
    NAME         => "IO",
    OBJECT       => '$(O_FILES)',
    ABSTRACT     => 'Perl core IO modules',
    AUTHOR       => 'Perl5 Porters <perl5-porters@perl.org>',
    PREREQ_PM    => {
        'Test::More' => 0,
        'File::Temp' => '0.15',
    },
    (
        $PERL_CORE
        ? ()
        : (
            INSTALLDIRS => ( $] < 5.011 ? 'perl' : 'site' ),
            clean       => { FILES => 'typemap' },
        )
    ),
    ( $define                                      ? ( DEFINE    => $define ) : () ),
    ( ( ExtUtils::MakeMaker->VERSION() gt '6.30' ) ? ( 'LICENSE' => 'perl' )  : () ),
    META_MERGE => {
        resources => {
            license     => 'http://dev.perl.org/licenses/',
            bugtracker  => 'https://github.com/perl/perl5/issues',
            repository  => 'https://github.com/Perl/perl5/tree/blead/dist/IO',
            MailingList => 'http://lists.perl.org/list/perl5-porters.html',
        },
    },
);
