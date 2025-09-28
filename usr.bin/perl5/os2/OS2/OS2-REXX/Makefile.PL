use ExtUtils::MakeMaker;

WriteMakefile(
	      NAME => 'OS2::REXX',
	      VERSION_FROM => 'REXX.pm',
	      MAN3PODS 	=> {}, 	# Pods will be built by installman.
	      XSPROTOARG => '-noprototypes',
	      PERL_MALLOC_OK => 1,
);
