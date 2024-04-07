use ExtUtils::MakeMaker;
WriteMakefile(
    NAME	=> 'NDBM_File',
    LIBS => ["-L/usr/local/lib -lndbm", "-ldbm -lucb"],
    XSPROTOARG => '-noprototypes', 		# XXX remove later?
    VERSION_FROM => 'NDBM_File.pm',
);
