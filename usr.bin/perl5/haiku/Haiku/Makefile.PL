use 5.006;
use ExtUtils::MakeMaker;

unless ($^O eq "haiku") {
    die "OS unsupported\n";
}

#my @libs;
#push @libs, '-L/lib/w32api -lole32 -lversion' if $^O eq "cygwin";

WriteMakefile(
    NAME          => 'Haiku',
    VERSION_FROM  => 'Haiku.pm',
#    LIBS          => \@libs,
    INSTALLDIRS   => ($] >= 5.008004 ? 'perl' : 'site'),
    NO_META       => 1,

    AUTHOR        => 'Ingo Weinhold <ingo_weinhold@gmx.de>',
    ABSTRACT_FROM => 'Haiku.pm',
);
