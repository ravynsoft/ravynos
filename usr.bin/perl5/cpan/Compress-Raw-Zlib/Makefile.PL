#! perl -w

use strict ;
require 5.006 ;

use lib '.';
use private::MakeUtil;
use ExtUtils::MakeMaker 5.16 ;
use ExtUtils::Install (); # only needed to check for version
use Config;

my $ZLIB_LIB ;
my $ZLIB_INCLUDE ;
my $BUILD_ZLIB = 0 ;
my $USE_ZLIB_NG = 0;
my $OLD_ZLIB = '' ;
my $WALL = '' ;
my $GZIP_OS_CODE = -1 ;
my $USE_PPPORT_H = ($ENV{PERL_CORE}) ? '' : '-DUSE_PPPORT_H';
my $OPTIMIZE = $Config{'optimize'};
if ($Config{'gccversion'} and $OPTIMIZE =~ /-g (gdb)? 3/x) {
  $OPTIMIZE =~ s/-g(gdb)?3/-g/g; # [88936] out of memory with -g3 since 2.062
}

# Windows uses a different name for the zlib library
my $ZLIB_LIBRARY_NAME = $^O eq 'MSWin32' ? 'zlib' : 'z' ;

#$WALL = ' -pedantic ' if $Config{'cc'} =~ /gcc/ ;
#$WALL = ' -Wall -Wno-comment ' if $Config{'cc'} =~ /gcc/ ;

# Ticket #18986 says that ExtUtils::Install 1.39 fixes the in-use issue
# on win32/cygwin, so make the code below conditional on the version of
# ExtUtils::Install.

# Don't ask if MM_USE_DEFAULT is set -- enables perl core building on cygwin
if ($^O =~ /cygwin/i and $ExtUtils::Install::VERSION < 1.39
        and not ($ENV{PERL_MM_USE_DEFAULT} or $ENV{PERL_CORE}))
{
    print <<EOM ;

I see you are running Cygwin.

Please note that this module cannot be installed on Cygwin using the CPAN
shell. The CPAN Shell uses Compress::Raw::Zlib internally and it is not
possible to delete an active DLL.

If you are running the CPAN shell, please exit it and install this module
by hand by running 'make install' under the directory

    ~/.cpan/build/Compress-Raw-Zlib-VERSION

EOM

    print "Do you want to continue? [Y/N]: " ;
    my $answer = <STDIN> ;

    if ($answer =~ /^yes|y/i)
    {
	print "continuing...\n"
    }
    else
    {
	print "exiting...\n" ;
	exit 1 ;
    }


}

ParseCONFIG() ;

UpDowngrade(getPerlFiles('MANIFEST'))
    unless $ENV{PERL_CORE};

my $OPT_Z_PREFIX =  '-DZ_PREFIX' ;
my $OPT_ZLIB_INCLUDE = "-I$ZLIB_INCLUDE";
my $OPT_ZLIB_LIB = "-L$ZLIB_LIB" ;
my $OPT_SOLO = '-DZ_SOLO';
my $OPT_USE_ZLIB_NG = "";

if (! $BUILD_ZLIB)
{
    # When using  the system supplied zlib, we don't want to use our bespoke prefix
    $OPT_Z_PREFIX =  '';

    $OPT_ZLIB_INCLUDE = ''
        if $ZLIB_INCLUDE eq './zlib-src' ;

    $OPT_ZLIB_LIB = ''
        if $ZLIB_LIB eq './zlib-src' ;

    $OPT_SOLO = '';
}

if ( $USE_ZLIB_NG)
{
    $OPT_USE_ZLIB_NG = '-DUSE_ZLIB_NG';
    $ZLIB_LIBRARY_NAME = $^O eq 'MSWin32' ? 'zlib-ng' : 'z-ng' ;
}

WriteMakefile(
    NAME         => 'Compress::Raw::Zlib',
    VERSION_FROM => 'lib/Compress/Raw/Zlib.pm',
    INC          => $OPT_ZLIB_INCLUDE ,
    DEFINE       => "-DNO_VIZ $OPT_SOLO $OLD_ZLIB $WALL $OPT_Z_PREFIX $OPT_USE_ZLIB_NG -DGZIP_OS_CODE=$GZIP_OS_CODE $USE_PPPORT_H -DPerl_crz_BUILD_ZLIB=$BUILD_ZLIB" ,
    XS           => { 'Zlib.xs' => 'Zlib.c'},
    'depend'     => { 'Makefile'   => 'config.in' },
    'clean'      => { FILES        => '*.c constants.h constants.xs' },
    'dist'       => { COMPRESS     => 'gzip',
                      TARFLAGS     => '-chvf',
                      SUFFIX       => 'gz',
                      DIST_DEFAULT => 'MyTrebleCheck tardist',
                    },

    (
      $BUILD_ZLIB
        ? zlib_files($ZLIB_LIB)
        : (LIBS => [ "$OPT_ZLIB_LIB -l$ZLIB_LIBRARY_NAME " ])
    ),
    OPTIMIZE => $OPTIMIZE,

    INSTALLDIRS => ($] >= 5.009 && $] < 5.011 ? 'perl' : 'site'),

     ( eval { ExtUtils::MakeMaker->VERSION(6.46) }
        ? ( META_MERGE  => {

                "meta-spec" => { version => 2 },

                no_index => {
                    directory => [ 't', 'private' ],
                },

                resources   => {

                    bugtracker  => {
                        web     => 'https://github.com/pmqs/Compress-Raw-Zlib/issues'
                    },

                    homepage    => 'https://github.com/pmqs/Compress-Raw-Zlib',

                    repository  => {
                        type    => 'git',
                        url     => 'git://github.com/pmqs/Compress-Raw-Zlib.git',
                        web     => 'https://github.com/pmqs/Compress-Raw-Zlib',
                    },
                },
              }
            )
        : ()
    ),

    ((ExtUtils::MakeMaker->VERSION() gt '6.30') ?
        ('LICENSE'  => 'perl')         : ()),

) ;

sub version_Macro
{
    my $ver = shift ;

    return [ "#if ZLIB_VERNUM >= 0x$ver\n", "#endif\n" ];
}

my @names = qw(

    DEF_WBITS
    MAX_MEM_LEVEL
    MAX_WBITS
    OS_CODE

    Z_ASCII
    Z_BEST_COMPRESSION
    Z_BEST_SPEED
    Z_BINARY
    Z_BLOCK
    Z_BUF_ERROR
    Z_DATA_ERROR
    Z_DEFAULT_COMPRESSION
    Z_DEFAULT_STRATEGY
    Z_DEFLATED
    Z_ERRNO
    Z_FILTERED
    Z_FINISH
    Z_FIXED
    Z_FULL_FLUSH
    Z_HUFFMAN_ONLY
    Z_MEM_ERROR
    Z_NEED_DICT
    Z_NO_COMPRESSION
    Z_NO_FLUSH
    Z_OK
    Z_PARTIAL_FLUSH
    Z_RLE
    Z_STREAM_END
    Z_STREAM_ERROR
    Z_SYNC_FLUSH
    Z_UNKNOWN
    Z_VERSION_ERROR

    ZLIBNG_VERNUM
    ZLIBNG_VER_MAJOR
    ZLIBNG_VER_MINOR
    ZLIBNG_VER_REVISION
    ZLIBNG_VER_STATUS
    ZLIBNG_VER_MODIFIED
);

my %verSpecificNames = (
    Z_TREES => '1240',
);

if (eval {require ExtUtils::Constant; 1}) {
    # Check the constants above all appear in @EXPORT in Zlib.pm
    my %names = %verSpecificNames, map { $_, 1} @names, 'ZLIB_VERSION';
    open F, "<lib/Compress/Raw/Zlib.pm" or die "Cannot open Zlib.pm: $!\n";
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
        die "The following names are missing from \@EXPORT in Zlib.pm\n" .
            "\t$missing\n" ;
    }

    push @names, { name => 'ZLIB_VERSION', type => 'PV' };

    push @names, map { { name => $_,
                         macro => version_Macro $verSpecificNames{$_}
                       }
                     }
                 keys %verSpecificNames ;

    push @names, { name => 'Z_NULL', type => 'UV' };
    push @names, { name => 'ZLIBNG_VERSION', type => 'PV' };

    ExtUtils::Constant::WriteConstants(
                                     NAME => 'Zlib',
                                     NAMES => \@names,
                                     C_FILE  => 'constants.h',
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

sub getBoolean
{
    my $name = shift ;
    my $info = shift;

    my $x = defined $ENV{$name}
                ? $ENV{$name}
                : $info->{$name} ;

    return ($x =~ /^yes|on|true|1$/i) ? 1 : 0;
}

sub ParseCONFIG
{
    my ($k, $v) ;
    my @badkey = () ;
    my %Info = () ;
    my @Options = qw( INCLUDE LIB BUILD_ZLIB OLD_ZLIB GZIP_OS_CODE USE_ZLIB_NG) ;
    my %ValidOption = map {$_, 1} @Options ;
    my %Parsed = %ValidOption ;
    my $CONFIG = 'config.in' ;

    print "Parsing $CONFIG...\n" ;

    open(F, "<$CONFIG") or die "Cannot open file $CONFIG: $!\n" ;
    while (<F>) {
	s/^\s*|\s*$//g ;
	next if /^\s*$/ or /^\s*#/ ;
	s/\s*#\s*$// ;

	($k, $v) = split(/\s+=\s+/, $_, 2) ;
	$k = uc $k ;
	if ($ValidOption{$k}) {
	    delete $Parsed{$k} ;
	    $Info{$k} = $v ;
	}
	else {
	    push(@badkey, $k) ;
	}
    }
    close F ;

    print "Unknown keys in $CONFIG ignored [@badkey]\n"
	if @badkey ;

    # check parsed values
    my @missing = () ;
    die "The following keys are missing from $CONFIG  [@missing]\n"
        if @missing = keys %Parsed ;

    $ZLIB_INCLUDE = defined $ENV{'ZLIB_INCLUDE'}
                        ? $ENV{'ZLIB_INCLUDE'}
                        : $Info{'INCLUDE'} ;

    $ZLIB_LIB = defined $ENV{'ZLIB_LIB'}
                    ?$ENV{'ZLIB_LIB'}
                    : $Info{'LIB'} ;

    $USE_ZLIB_NG = getBoolean('USE_ZLIB_NG', \%Info);

    if ($^O eq 'VMS') {
        $ZLIB_INCLUDE = VMS::Filespec::vmspath($ZLIB_INCLUDE);
        $ZLIB_LIB = VMS::Filespec::vmspath($ZLIB_LIB);
    }

    $OLD_ZLIB = '-DOLD_ZLIB'
        if getBoolean('OLD_ZLIB', \%Info);

    $BUILD_ZLIB = getBoolean('BUILD_ZLIB', \%Info);

    if ($BUILD_ZLIB ) {

        # ZLIB_LIB & ZLIB_INCLUDE must point to the same place when
        # BUILD_ZLIB is specified.
        die "INCLUDE & LIB must be the same when BUILD_ZLIB is True\n"
            if $ZLIB_LIB ne $ZLIB_INCLUDE ;

        # Check the zlib source directory exists
        die "LIB/INCLUDE directory '$ZLIB_LIB' does not exits\n"
            unless -d $ZLIB_LIB ;

        # check for a well known file
        if ($USE_ZLIB_NG)
        {
            die "LIB/INCLUDE directory, '$ZLIB_LIB', doesn't seem to have the zlib-ng source files\n"
                unless -e catfile($ZLIB_LIB, 'zlib-ng.h') ;
        }
        else
        {
            die "LIB/INCLUDE directory, '$ZLIB_LIB', doesn't seem to have the zlib source files\n"
                unless -e catfile($ZLIB_LIB, 'zlib.h') ;
        }

        # write the Makefile
        print "Building Zlib enabled\n" ;
    }

    $GZIP_OS_CODE = defined $ENV{'GZIP_OS_CODE'}
                          ? $ENV{'GZIP_OS_CODE'}
                          : $Info{'GZIP_OS_CODE'} ;

	die "GZIP_OS_CODE not 'AUTO_DETECT' or a number between 0 and 255\n"
	   unless uc $GZIP_OS_CODE eq 'AUTO_DETECT'
                    || ( $GZIP_OS_CODE =~ /^(\d+)$/ && $1 >= 0 && $1 <= 255) ;

    if (uc $GZIP_OS_CODE eq 'AUTO_DETECT')
    {
        print "Auto Detect Gzip OS Code..\n" ;
        $GZIP_OS_CODE = getOSCode() ;
    }

    my $name = getOSname($GZIP_OS_CODE);
    print "Setting Gzip OS Code to $GZIP_OS_CODE [$name]\n" ;

    print <<EOM if 0 ;
    INCLUDE         [$ZLIB_INCLUDE]
    LIB             [$ZLIB_LIB]
    GZIP_OS_CODE    [$GZIP_OS_CODE]
    OLD_ZLIB        [$OLD_ZLIB]
    BUILD_ZLIB      [$BUILD_ZLIB]
    USE_ZLIB_NG     [$USE_ZLIB_NG]

EOM

    print "Looks Good.\n" ;

}



sub zlib_files
{
    my $dir = shift ;

    my @h_files = ();
    my @c_files = ();

    if (-f catfile($dir, "infback.c")) {
        # zlib 1.2.0 or greater
        #
        @h_files = qw(crc32.h    inffast.h inflate.h  trees.h    zconf.in.h
    	              zutil.h    deflate.h inffixed.h inftrees.h zconf.h
    		      zlib.h
    		 );
        @c_files = qw(adler32  crc32   infback  inflate  uncompr
    		      compress deflate inffast  inftrees
    		      trees    zutil
    		 );
    }
    else {
        # zlib 1.1.x

        @h_files = qw(deflate.h  infcodes.h inftrees.h zconf.h zutil.h
    		      infblock.h inffast.h  infutil.h  zlib.h
    		 );
        @c_files = qw(adler32  compress crc32    uncompr
    		      deflate  trees    zutil    inflate infblock
    		      inftrees infcodes infutil  inffast
    		 );
    }

    @h_files = map { catfile($dir, $_)  } @h_files ;
    my @o_files = map { "$_\$(OBJ_EXT)" } 'Zlib', @c_files;
    @c_files = map { "$_.c" } 'Zlib', @c_files ;

    foreach my $file (@c_files)
      { copy(catfile($dir, $file), '.') }

    return (
        #'H'         =>  [ @h_files ],
    	'C'         =>  [ @c_files ] ,
        #'OBJECT'    => qq[ @o_files ],
        'OBJECT'    => q[ $(O_FILES) ],


           ) ;
}



use vars qw ( @GZIP_OS_Names  %OSnames) ;

BEGIN
{
  @GZIP_OS_Names = (
    [ ''        => 0,    'MS-DOS'                       ],
    [ 'amigaos' => 1,    'Amiga'                        ],
    [ 'VMS'     => 2,    'VMS'                          ],
    [ ''        => 3,    'Unix/Default'                 ],
    [ ''        => 4,    'VM/CMS'                       ],
    [ ''        => 5,    'Atari TOS'                    ],
    [ 'os2'     => 6,    'HPFS (OS/2, NT)'              ],
    [ 'MacOS'   => 7,    'Macintosh'                    ],
    [ ''        => 8,    'Z-System'                     ],
    [ ''        => 9,    'CP/M'                         ],
    [ ''        => 10,   'TOPS-20'                      ],
    [ ''        => 11,   'NTFS (NT)'                    ],
    [ ''        => 12,   'SMS QDOS'                     ],
    [ ''        => 13,   'Acorn RISCOS'                 ],
    [ 'MSWin32' => 14,   'VFAT file system (Win95, NT)' ],
    [ ''        => 15,   'MVS'                          ],
    [ 'beos'    => 16,   'BeOS'                         ],
    [ ''        => 17,   'Tandem/NSK'                   ],
    [ ''        => 18,   'THEOS'                        ],
    [ ''        => 255,  'Unknown OS'                   ],
  );

  %OSnames = map { $$_[1] => $$_[2] }
             @GZIP_OS_Names ;
}

sub getOSCode
{
    my $default = 3 ; # Unix is the default

    my $uname = $^O;

    for my $h (@GZIP_OS_Names)
    {
        my ($pattern, $code, $name) = @$h;

        return $code
            if $pattern && $uname eq $pattern ;
    }

    return $default ;
}

sub getOSname
{
    my $code = shift ;

    return $OSnames{$code} || 'Unknown OS' ;
}

# end of file Makefile.PL
