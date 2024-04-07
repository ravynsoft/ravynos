#! perl -w

use strict ;
require 5.006 ;

use lib '.';
use private::MakeUtil;
use ExtUtils::MakeMaker 5.16 ;

my $WALL= '';
$WALL = ' -Wall -Wno-comment ' if $Config{'cc'} =~ /gcc/ ;
my $USE_PPPORT_H = ($ENV{PERL_CORE}) ? '' : '-DUSE_PPPORT_H';


my $BUILD_BZIP2 = defined($ENV{BUILD_BZIP2}) ? $ENV{BUILD_BZIP2} : 1;
my $BZIP2_LIB = defined($ENV{BZIP2_LIB}) ? $ENV{BZIP2_LIB} : 'bzip2-src';
my $BZIP2_INCLUDE = defined($ENV{BZIP2_INCLUDE}) ? $ENV{BZIP2_INCLUDE} : '.';

#ParseCONFIG() ;

UpDowngrade(getPerlFiles('MANIFEST'))
    unless $ENV{PERL_CORE};

WriteMakefile(
    NAME         => 'Compress::Raw::Bzip2',
    VERSION_FROM => 'lib/Compress/Raw/Bzip2.pm',
    INC          => "-I$BZIP2_INCLUDE" ,
    DEFINE       => "$WALL -DBZ_NO_STDIO $USE_PPPORT_H" ,
    XS           => { 'Bzip2.xs' => 'Bzip2.c'},
    'clean'      => { FILES      => '*.c bzip2.h bzlib.h bzlib_private.h constants.h constants.xs' },
   #'depend'     => { 'Makefile'   => 'config.in' },
    'dist'       => { COMPRESS     => 'gzip',
                      TARFLAGS     => '-chvf',
                      SUFFIX       => 'gz',
                      DIST_DEFAULT => 'MyTrebleCheck tardist',
                    },

    (
      $BUILD_BZIP2
        ? bzip2_files($BZIP2_LIB)
        : (LIBS => [ "-L$BZIP2_LIB -lbz2 " ])
    ),

    (
      $] >= 5.005
        ? (ABSTRACT_FROM => 'lib/Compress/Raw/Bzip2.pm',
            AUTHOR       => 'Paul Marquess <pmqs@cpan.org>')
        : ()
    ),

    INSTALLDIRS => ($] > 5.010  && $] < 5.011 ? 'perl' : 'site'),

     ( eval { ExtUtils::MakeMaker->VERSION(6.46) }
        ? ( META_MERGE  => {

                "meta-spec" => { version => 2 },

                no_index => {
                    directory => [ 't', 'private' ],
                },

                resources   => {

                    bugtracker  => {
                        web     => 'https://github.com/pmqs/Compress-Raw-Bzip2/issues'
                    },

                    homepage    => 'https://github.com/pmqs/Compress-Raw-Bzip2',

                    repository  => {
                        type    => 'git',
                        url     => 'git://github.com/pmqs/Compress-Raw-Bzip2.git',
                        web     => 'https://github.com/pmqs/Compress-Raw-Bzip2',
                    },
                },
              }
            )
        : ()
    ),

    ((ExtUtils::MakeMaker->VERSION() gt '6.30') ?
        ('LICENSE'  => 'perl')         : ()),

) ;

my @names = qw(
		BZ_RUN
		BZ_FLUSH
		BZ_FINISH

		BZ_OK
		BZ_RUN_OK
		BZ_FLUSH_OK
		BZ_FINISH_OK
		BZ_STREAM_END
		BZ_SEQUENCE_ERROR
		BZ_PARAM_ERROR
		BZ_MEM_ERROR
		BZ_DATA_ERROR
		BZ_DATA_ERROR_MAGIC
		BZ_IO_ERROR
		BZ_UNEXPECTED_EOF
		BZ_OUTBUFF_FULL
		BZ_CONFIG_ERROR
    	);

if (eval {require ExtUtils::Constant; 1}) {
    # Check the constants above all appear in @EXPORT in Bzip2.pm
    my %names = map { $_, 1} @names ; #, 'BZ_VERSION';
    open F, "<lib/Compress/Raw/Bzip2.pm" or die "Cannot open Bzip2.pm: $!\n";
    while (<F>)
    {
        last if /^\s*\@EXPORT\s+=\s+qw\(/ ;
    }

    while (<F>)
    {
        last if /^\s*\)/ ;
        /(\S+)/ ;
        delete $names{$1} if defined $1 ;
    }
    close F ;

    if ( keys %names )
    {
        my $missing = join ("\n\t", sort keys %names) ;
        die "The following names are missing from \@EXPORT in Bzip2.pm\n" .
            "\t$missing\n" ;
    }

    #push @names, {name => 'BZ_VERSION', type => 'PV' };

    ExtUtils::Constant::WriteConstants(
                                     NAME     => 'Bzip2',
                                     NAMES    => \@names,
                                     C_FILE   => 'constants.h',
                                     XS_FILE  => 'constants.xs',

                                    );
}
else {
    foreach my $name (qw( constants.h constants.xs ))
    {
        my $from = catfile('fallback', $name);
        copy ($from, $name)
          or die "Can't copy $from to $name: $!";
    }
}


sub bzip2_files
{
    my $dir = shift ;

    my @c_files = qw(
        blocksort.c
        huffman.c
        crctable.c
        randtable.c
        compress.c
        decompress.c
        bzlib.c
    );

    my @h_files = qw( bzlib.h  bzlib_private.h );

    foreach my $file (@c_files, @h_files)
      { copy(catfile($dir, $file), '.') }


    @h_files = map { catfile($dir, $_)  } @h_files ;
    my @o_files = map { "$_\$(OBJ_EXT)" } 'Bzip2', @c_files;
    push @c_files, 'Bzip2.c' ;

    return (
        #'H'         =>  [ @h_files ],
    	'C'         =>  [ @c_files ] ,
        #'OBJECT'    => qq[ @o_files ],
        'OBJECT'    => q[ $(O_FILES) ],


           ) ;
}
