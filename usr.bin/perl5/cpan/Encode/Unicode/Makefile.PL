use 5.7.2;
use strict;
use ExtUtils::MakeMaker;

WriteMakefile(
          INC		=> "-I../Encode",
          NAME		=> 'Encode::Unicode',
          VERSION_FROM	=> "Unicode.pm",
          MAN3PODS  => {},
          );

