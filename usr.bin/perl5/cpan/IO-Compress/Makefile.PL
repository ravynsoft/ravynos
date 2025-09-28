#! perl -w

use strict ;
require 5.006 ;

$::VERSION = '2.204' ;
$::DEP_VERSION = '2.204';

use lib '.';
use private::MakeUtil;
use ExtUtils::MakeMaker 5.16 ;

UpDowngrade(getPerlFiles('MANIFEST'))
    unless $ENV{PERL_CORE};

WriteMakefile(
    NAME         => 'IO::Compress',
    VERSION_FROM => 'lib/IO/Compress/Base.pm',
    'dist'       => { COMPRESS     => 'gzip',
                      TARFLAGS     => '-chvf',
                      SUFFIX       => 'gz',
                      DIST_DEFAULT => 'MyTrebleCheck tardist',
                    },

    (
      $ENV{SKIP_FOR_CORE}
        ? ()
	    : (PREREQ_PM   => { 'Compress::Raw::Bzip2' => $::DEP_VERSION,
		                    'Compress::Raw::Zlib'  => $::DEP_VERSION,
		                    'Scalar::Util'  => 0,
                            'Encode'        => 0,
                            'Time::Local'   => 0,
		                    $] >= 5.005 && $] < 5.006
                                ? ('File::BSDGlob' => 0)
                                : () }
	      )
    ),

    (
    $] >= 5.005
        ? (ABSTRACT => 'IO Interface to compressed data files/buffers',
            AUTHOR  => 'Paul Marquess <pmqs@cpan.org>')
        : ()
    ),

    INSTALLDIRS => ($] >= 5.009 && $] < 5.011 ? 'perl' : 'site'),

    EXE_FILES => ['bin/zipdetails', 'bin/streamzip'],

    (
    $] >= 5.009 && $] <= 5.011001 && ! $ENV{PERL_CORE}
        ? (INSTALLPRIVLIB    => '$(INSTALLARCHLIB)')
        : ()
    ),

     ( eval { ExtUtils::MakeMaker->VERSION(6.46) }
        ? ( META_MERGE  => {

                "meta-spec" => { version => 2 },

                no_index => {
                    directory => [ 't', 'private' ],
                },

                resources   => {

                    bugtracker  => {
                        web     => 'https://github.com/pmqs/IO-Compress/issues'
                    },

                    homepage    => 'https://github.com/pmqs/IO-Compress',

                    repository  => {
                        type    => 'git',
                        url     => 'git://github.com/pmqs/IO-Compress.git',
                        web     => 'https://github.com/pmqs/IO-Compress',
                    },
                },
              }
            )
        : ()
    ),

    ((ExtUtils::MakeMaker->VERSION() gt '6.30') ?
        ('LICENSE'  => 'perl')         : ()),

) ;

# end of file Makefile.PL
