use ExtUtils::MakeMaker;

WriteMakefile(
              'NAME'      => 'XS::Typemap',
              'VERSION_FROM'   => 'Typemap.pm',
              'dist'      => { COMPRESS => "gzip -9f"},
              OBJECT      => 'stdio.o  Typemap.o',
	      ABSTRACT_FROM => 'Typemap.pm',
	      AUTHOR     => 'Tim Jenness <t.jenness@jach.hawaii.edu>',
);


# Nothing to install (except maybe the XS::Typemap.xs documentation)

sub MY::install { "install ::\n"  };
