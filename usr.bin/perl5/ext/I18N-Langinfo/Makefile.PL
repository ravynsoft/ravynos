use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23;

WriteMakefile(
    'NAME'		=> 'I18N::Langinfo',
    'VERSION_FROM'	=> 'Langinfo.pm', # finds $VERSION
     ABSTRACT_FROM	=> 'Langinfo.pm', # retrieve abstract from module
     AUTHOR		=> 'Jarkko Hietaniemi <jhi@hut.fi>',
    realclean => {FILES=> 'const-c.inc const-xs.inc'},
);

# Some older versions of glibc use only enums, no defines, hence all this
# hassle (so old glibc that the define is GNU_LIBRARY, not GLIBC):
my @names = 'CODESET'; # CODESET isn't an enum in old glibc's langinfo.h
push @names,           # This lot are always enums in old langinfo.h:
    {name=>$_, type=>"IV",
     macro=>["#if defined($_) || (defined(__GNU_LIBRARY__) && defined(_NL_ITEM))\n",
             "#endif\n"]}
      foreach qw (ABDAY_1 ABDAY_2 ABDAY_3 ABDAY_4 ABDAY_5 ABDAY_6 ABDAY_7
                  ABMON_1 ABMON_10 ABMON_11 ABMON_12 ABMON_2 ABMON_3 ABMON_4
                  ABMON_5 ABMON_6 ABMON_7 ABMON_8 ABMON_9 ALT_DIGITS AM_STR
                  DAY_1 DAY_2 DAY_3 DAY_4 DAY_5 DAY_6 DAY_7 D_FMT D_T_FMT ERA
                  ERA_D_FMT ERA_D_T_FMT ERA_T_FMT MON_1 MON_10 MON_11 MON_12
                  MON_2 MON_3 MON_4 MON_5 MON_6 MON_7 MON_8 MON_9 NOEXPR NOSTR
                  PM_STR T_FMT T_FMT_AMPM YESEXPR YESSTR);
push @names,           # This lot are only enums for __SVR4_I386_ABI_L1__:
    {name=>$_, type=>"IV",
     macro=>["#if defined($_) || (defined(__GNU_LIBRARY__) && defined(_NL_ITEM) && defined(__SVR4_I386_ABI_L1__))\n",
             "#endif\n"]}
      foreach qw (CRNCYSTR THOUSEP RADIXCHAR);

ExtUtils::Constant::WriteConstants(
				   PROXYSUBS => {autoload => 1},
				   NAME => 'I18N::Langinfo',
				   NAMES => \@names,
				  );
