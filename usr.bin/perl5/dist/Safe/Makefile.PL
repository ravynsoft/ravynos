use ExtUtils::MakeMaker;

my $core = grep { $_ eq 'PERL_CORE=1' } @ARGV;

WriteMakefile(
    NAME => 'Safe',
    VERSION_FROM => 'Safe.pm',
    INSTALLDIRS => ($] < 5.011 ? 'perl' : 'site'),
    ($core || $] >= 5.011) ? () : (INST_LIB => '$(INST_ARCHLIB)'),
);
