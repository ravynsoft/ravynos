BEGIN { chdir 't' if -d 't' }

use Test::More 'no_plan';
use strict;
use lib '../lib';

use Cwd;
use Config;
use IO::File;
use File::Copy;
use File::Path;
use File::Spec          ();
use File::Spec::Unix    ();
use File::Basename      ();
use Data::Dumper;

### need the constants at compile time;
use Archive::Tar::Constant;

my $Class   = 'Archive::Tar';
my $FClass  = $Class . '::File';
use_ok( $Class );



### XXX TODO:
### * change to fullname
### * add tests for global variables

### set up the environment ###
my @EXPECT_NORMAL = (
    ### dirs        filename    contents
    [   [],         'c',        qr/^iiiiiiiiiiii\s*$/ ],
    [   [],         'd',        qr/^uuuuuuuu\s*$/ ],
);

### includes binary data
my $ALL_CHARS = join '', "\r\n", map( chr, 1..255 ), "zzz\n\r";

### @EXPECTBIN is used to ensure that $tarbin is written in the right
### order and that the contents and order match exactly when extracted
my @EXPECTBIN = (
    ###  dirs   filename      contents       ###
    [    [],    'bIn11',      $ALL_CHARS x 11 ],
    [    [],    'bIn3',       $ALL_CHARS x  3 ],
    [    [],    'bIn4',       $ALL_CHARS x  4 ],
    [    [],    'bIn1',       $ALL_CHARS      ],
    [    [],    'bIn2',       $ALL_CHARS x  2 ],
);

### @EXPECTX is used to ensure that $tarx is written in the right
### order and that the contents and order match exactly when extracted
### the 'x/x' extraction used to fail before A::T 1.08
my @EXPECTX = (
    ###  dirs       filename    contents
    [    [ 'x' ],   'k',        '',     ],
    [    [ 'x' ],   'x',        'j',    ],   # failed before A::T 1.08
);

my $LONG_FILE = qq[directory/really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-really-long-directory-name/myfile];

### wintendo can't deal with too long paths, so we might have to skip tests ###
my $TOO_LONG    =   ($^O eq 'MSWin32' or $^O eq 'cygwin' or $^O eq 'VMS')
                    && length( cwd(). $LONG_FILE ) > 247;

if(!$TOO_LONG) {
    my $alt = File::Spec->catfile( cwd(), $LONG_FILE);
    eval 'mkpath([$alt]);';
    if($@)
    {
        $TOO_LONG = 1;
    }
    else
    {
        $@ = '';
        my $base = File::Spec->catfile( cwd(), 'directory');
        rmtree $base;
    }
}
### warn if we are going to skip long file names
if ($TOO_LONG) {
    diag("No long filename support - long filename extraction disabled") if ! $ENV{PERL_CORE};
} else {
    push @EXPECT_NORMAL, [ [], $LONG_FILE, qr/^hello\s*$/];
}

my @ROOT        = grep { length }   'src', $TOO_LONG ? 'short' : 'long';
my $NO_UNLINK   = $ARGV[0] ? 1 : 0;

### enable debugging?
### pesky warnings
$Archive::Tar::DEBUG = $Archive::Tar::DEBUG = 1 if $ARGV[1];

### tests for binary and x/x files
my $TARBIN      = $Class->new;
my $TARX        = $Class->new;

### paths to a .tar and .tgz file to use for tests
my $TAR_FILE        = File::Spec->catfile( @ROOT, 'bar.tar' );
my $TGZ_FILE        = File::Spec->catfile( @ROOT, 'foo.tgz' );
my $TBZ_FILE        = File::Spec->catfile( @ROOT, 'foo.tbz' );
my $TXZ_FILE        = File::Spec->catfile( @ROOT, 'foo.txz' );
my $OUT_TAR_FILE    = File::Spec->catfile( @ROOT, 'out.tar' );
my $OUT_TGZ_FILE    = File::Spec->catfile( @ROOT, 'out.tgz' );
my $OUT_TBZ_FILE    = File::Spec->catfile( @ROOT, 'out.tbz' );
my $OUT_TXZ_FILE    = File::Spec->catfile( @ROOT, 'out.txz' );

my $COMPRESS_FILE = 'copy';
$^O eq 'VMS' and $COMPRESS_FILE .= '.';
copy( File::Basename::basename($0), $COMPRESS_FILE );
chmod 0644, $COMPRESS_FILE;

### done setting up environment ###

### check for zlib/bzip2/xz support
{   for my $meth ( qw[has_zlib_support has_bzip2_support has_xz_support] ) {
        can_ok( $Class, $meth );
    }
}



### tar error tests
{   my $tar     = $Class->new;

    ok( $tar,                       "Object created" );
    isa_ok( $tar,                   $Class );

    local $Archive::Tar::WARN  = 0;

    ### should be empty to begin with
    is( $tar->error, '',            "The error string is empty" );

    ### try a read on nothing
    my @list = $tar->read();

    ok(!(scalar @list),             "Function read returns 0 files on error" );
    ok( $tar->error,                "   error string is non empty" );
    like( $tar->error, qr/No file to read from/,
                                    "   error string from create()" );
    unlike( $tar->error, qr/add/,   "   error string does not contain add" );

    ### now, add empty data
    my $obj = $tar->add_data( '' );

    ok( !$obj,                      "'add_data' returns undef on error" );
    ok( $tar->error,                "   error string is non empty" );
    like( $tar->error, qr/add/,     "   error string contains add" );
    unlike( $tar->error, qr/create/,"   error string does not contain create" );

    ### check if ->error eq $error
    is( $tar->error, $Archive::Tar::error,
                                    "Error '$Archive::Tar::error' matches $Class->error method" );

    ### check that 'contains_file' doesn't warn about missing files.
    {   ### turn on warnings in general!
        local $Archive::Tar::WARN  = 1;

        my $warnings = '';
        local $SIG{__WARN__} = sub { $warnings .= "@_" };

        my $rv = $tar->contains_file( $$ );
        ok( !$rv,                   "Does not contain file '$$'" );
        is( $warnings, '',          "   No warnings issued during lookup" );
    }
}

my $ebcdic_skip_msg = "File contains an alien character set";

### read tests ###
SKIP: {
    my @to_try;

    if (ord 'A' == 65) {
        push @to_try, $TAR_FILE;
        push @to_try, $TGZ_FILE if $Class->has_zlib_support;
        push @to_try, $TBZ_FILE if $Class->has_bzip2_support;
        push @to_try, $TXZ_FILE if $Class->has_xz_support;
    }
    else {
        skip $ebcdic_skip_msg, 4;
    }

    for my $type( @to_try ) {

        ### normal tar + gz compressed file
        my $tar             = $Class->new;

        ### check we got the object
        ok( $tar,               "Object created" );
        isa_ok( $tar,           $Class );

        ### ->read test
        my @list    = $tar->read( $type );
        my $cnt     = scalar @list;
        my $expect  = scalar __PACKAGE__->get_expect();

        ok( $cnt,               "Reading '$type' using 'read()'" );
        is( $cnt, $expect,      "   All files accounted for" );

        for my $file ( @list ) {
            ok( $file,          "       Got File object" );
            isa_ok( $file,  $FClass );

            ### whitebox test -- make sure find_entry gets the
            ### right files
            for my $test ( $file->full_path, $file ) {
                is( $tar->_find_entry( $test ), $file,
                                "           Found proper object" );
            }

            next unless $file->is_file;

            my $name = $file->full_path;
            my($expect_name, $expect_content) =
                get_expect_name_and_contents( $name, \@EXPECT_NORMAL );

            ### ->fullname!
            ok($expect_name,    "           Found expected file '$name'" );

            like($tar->get_content($name), $expect_content,
                                "           Content OK" );
        }


        ### list_archive test
        {   my @list    = $Class->list_archive( $type );
            my $cnt     = scalar @list;
            my $expect  = scalar __PACKAGE__->get_expect();

            ok( $cnt,           "Reading '$type' using 'list_archive'");
            is( $cnt, $expect,  "   All files accounted for" );

            for my $file ( @list ) {
                next if __PACKAGE__->is_dir( $file ); # directories

                my($expect_name, $expect_content) =
                    get_expect_name_and_contents( $file, \@EXPECT_NORMAL );

                ok( $expect_name,
                                "   Found expected file '$file'" );
            }
        }
    }
}

### add files tests ###
{   my @add     = map { File::Spec->catfile( @ROOT, @$_ ) } ['b'];
    my @addunix = map { File::Spec::Unix->catfile( @ROOT, @$_ ) } ['b'];
    my $tar     = $Class->new;

    ### check we got the object
    ok( $tar,                       "Object created" );
    isa_ok( $tar,                   $Class );

    ### add the files
    {   my @files = $tar->add_files( @add );

        is( scalar @files, scalar @add,
                                    "   Adding files");
        is( $files[0]->name,'b',    "      Proper name" );

        SKIP: {
            skip( "You are building perl using symlinks", 1)
                if ($ENV{PERL_CORE} and $Config{config_args} =~/Dmksymlinks/);

            is( $files[0]->is_file, 1,
                                    "       Proper type" );
        }

        like( $files[0]->get_content, qr/^bbbbbbbbbbb\s*$/,
                                    "       Content OK" );

        ### check if we have then in our tar object
        for my $file ( @addunix ) {
            ok( $tar->contains_file($file),
                                    "       File found in archive" );
        }
    }

    ### check adding files doesn't conflict with a secondary archive
    ### old A::T bug, we should keep testing for it
    {   my $tar2    = $Class->new;
        my @added   = $tar2->add_files( $COMPRESS_FILE );
        my @count   = $tar2->list_files;

        is( scalar @added, 1,       "   Added files to secondary archive" );
        is( scalar @added, scalar @count,
                                    "       No conflict with first archive" );

        ### check the adding of directories
        my @add_dirs  = File::Spec->catfile( @ROOT );
        my @dirs      = $tar2->add_files( @add_dirs );
        is( scalar @dirs, scalar @add_dirs,
                                    "       Adding dirs");
        ok( $dirs[0]->is_dir,       "           Proper type" );
    }

    ### check if we can add a A::T::File object
    {   my $tar2    = $Class->new;
        my($added)  = $tar2->add_files( $add[0] );

        ok( $added,                 "   Added a file '$add[0]' to new object" );
        isa_ok( $added, $FClass,    "       Object" );

        my($added2) = $tar2->add_files( $added );
        ok( $added2,                "       Added an $FClass object" );
        isa_ok( $added2, $FClass,   "           Object" );

        is_deeply( [$added, $added2], [$tar2->get_files],
                                    "       All files accounted for" );
        isnt( $added, $added2,      "       Different memory allocations" );
    }
}

### add data tests ###
{
    {   ### standard data ###
        my @to_add  = ( 'a', 'aaaaa' );
        my $tar     = $Class->new;

        ### check we got the object
        ok( $tar,                   "Object created" );
        isa_ok( $tar,               $Class );

        ### add a new file item as data
        my $obj = $tar->add_data( @to_add );

        ok( $obj,                   "   Adding data" );
        is( $obj->name, $to_add[0], "       Proper name" );
        is( $obj->is_file, 1,       "       Proper type" );
        like( $obj->get_content, qr/^$to_add[1]\s*$/,
                                    "       Content OK" );
    }

    {   ### binary data +
        ### dir/file structure -- x/y always went ok, x/x used to extract
        ### in the wrong way -- this test catches that
        for my $list (  [$TARBIN,   \@EXPECTBIN],
                        [$TARX,     \@EXPECTX],
        ) {
            ### XXX GLOBAL! changes may affect other tests!
            my($tar,$struct) = @$list;

            for my $aref ( @$struct ) {
                my ($dirs,$file,$data) = @$aref;

                my $path = File::Spec::Unix->catfile(
                                grep { length } @$dirs, $file );

                my $obj = $tar->add_data( $path, $data );

                ok( $obj,               "   Adding data '$file'" );
                is( $obj->full_path, $path,
                                        "       Proper name" );
                ok( $obj->is_file,      "       Proper type" );
                is( $obj->get_content, $data,
                                        "       Content OK" );
            }
        }
    }
}

### rename/replace_content tests ###

SKIP: {
    skip $ebcdic_skip_msg, 9 if ord "A" != 65;

    my $tar     = $Class->new;
    my $from    = 'c';
    my $to      = 'e';

    ### read in the file, check the proper files are there
    ok( $tar->read( $TAR_FILE ),    "Read in '$TAR_FILE'" );
    ok( $tar->get_files($from),     "   Found file '$from'" );
    {   local $Archive::Tar::WARN = 0;
        ok(!$tar->get_files($to),   "   File '$to' not yet found" );
    }

    ### rename an entry, check the rename has happened
    ok( $tar->rename( $from, $to ), "   Renamed '$from' to '$to'" );
    ok( $tar->get_files($to),       "   File '$to' now found" );
    {   local $Archive::Tar::WARN = 0;
        ok(!$tar->get_files($from), "   File '$from' no longer found'");
    }

    ### now, replace the content
    my($expect_name, $expect_content) =
                        get_expect_name_and_contents( $from, \@EXPECT_NORMAL );

    like( $tar->get_content($to), $expect_content,
                                    "Original content of '$from' in '$to'" );
    ok( $tar->replace_content( $to, $from ),
                                    "   Set content for '$to' to '$from'" );
    is( $tar->get_content($to), $from,
                                    "   Content for '$to' is indeed '$from'" );
}

### remove tests ###
SKIP: {
    skip $ebcdic_skip_msg, 3 if ord "A" != 65;

    my $remove  = 'c';
    my $tar     = $Class->new;

    ok( $tar->read( $TAR_FILE ),    "Read in '$TAR_FILE'" );

    ### remove returns the files left, which should be equal to list_files
    is( scalar($tar->remove($remove)), scalar($tar->list_files),
                                    "   Removing file '$remove'" );

    ### so what's left should be all expected files minus 1
    is( scalar($tar->list_files), scalar(__PACKAGE__->get_expect) - 1,
                                    "   Proper files remaining" );
}

### write + read + extract tests ###
SKIP: {                             ### pesky warnings
    skip $ebcdic_skip_msg, 326 if ord "A" != 65;

    skip('no IO::String', 326) if   !$Archive::Tar::HAS_PERLIO &&
                                    !$Archive::Tar::HAS_PERLIO &&
                                    !$Archive::Tar::HAS_IO_STRING &&
                                    !$Archive::Tar::HAS_IO_STRING;

    my $tar = $Class->new;
    my $new = $Class->new;
    ok( $tar->read( $TAR_FILE ),    "Read in '$TAR_FILE'" );

    for my $aref (  [$tar,    \@EXPECT_NORMAL],
                    [$TARBIN, \@EXPECTBIN],
                    [$TARX,   \@EXPECTX]
    ) {
        my($obj,$struct) = @$aref;

        ### check if we stringify it ok
        {   my $string = $obj->write;
            ok( $string,           "    Stringified tar file has size" );
            cmp_ok( length($string) % BLOCK, '==', 0,
                                    "       Tar archive stringified" );
        }

        ### write tar tests
        {   my $out = $OUT_TAR_FILE;

            ### bug #41798: 'Nonempty $\ when writing a TAR file produces a
            ### corrupt TAR file' shows that setting $\ breaks writing tar files
            ### set it here purposely so we can verify NOTHING breaks
            local $\ = 'FOOBAR';

            {   ### write()
                ok( $obj->write($out),
                                    "       Wrote tarfile using 'write'" );
                check_tar_file( $out );
                check_tar_object( $obj, $struct );

                ### now read it in again
                ok( $new->read( $out ),
                                    "       Read '$out' in again" );

                check_tar_object( $new, $struct );

                ### now extract it again
                ok( $new->extract,  "       Extracted '$out' with 'extract'" );
                check_tar_extract( $new, $struct );

                rm( $out ) unless $NO_UNLINK;
            }


            {   ### create_archive()
                ok( $Class->create_archive( $out, 0, $COMPRESS_FILE ),
                                    "       Wrote tarfile using 'create_archive'" );
                check_tar_file( $out );

                ### now extract it again
                ok( $Class->extract_archive( $out ),
                                    "       Extracted file using 'extract_archive'");
                rm( $out ) unless $NO_UNLINK;
            }
        }

        ## write tgz tests
        {   my @out;
            push @out, [ $OUT_TGZ_FILE => 1             ] if $Class->has_zlib_support;
            push @out, [ $OUT_TBZ_FILE => COMPRESS_BZIP ] if $Class->has_bzip2_support;
            push @out, [ $OUT_TXZ_FILE => COMPRESS_XZ   ] if $Class->has_xz_support;

            for my $entry ( @out ) {

                my( $out, $compression ) = @$entry;

                {   ### write()
                    ok($obj->write($out, $compression),
                                    "       Writing compressed file '$out' using 'write'" );
                    check_compressed_file( $out );

                    check_tar_object( $obj, $struct );

                    ### now read it in again
                    ok( $new->read( $out ),
                                    "       Read '$out' in again" );
                    check_tar_object( $new, $struct );

                    ### now extract it again
                    ok( $new->extract,
                                    "       Extracted '$out' again" );
                    check_tar_extract( $new, $struct );

                    rm( $out ) unless $NO_UNLINK;
                }

                {   ### create_archive()
                    ok( $Class->create_archive( $out, $compression, $COMPRESS_FILE ),
                                    "       Wrote '$out' using 'create_archive'" );
                    check_compressed_file( $out );

                    ### now extract it again
                    ok( $Class->extract_archive( $out, $compression ),
                                    "       Extracted file using 'extract_archive'");
                    rm( $out ) unless $NO_UNLINK;
                }
            }
        }
    }
}


### limited read + extract tests ###
SKIP: {                             ### pesky warnings
    skip $ebcdic_skip_msg, 8 if ord "A" != 65;

    my $tar     = $Class->new;
    my @files   = $tar->read( $TAR_FILE, 0, { limit => 1 } );
    my $obj     = $files[0];

    is( scalar @files, 1,           "Limited read" );

    my ($name,$content) = get_expect_name_and_contents(
                                $obj->full_path, \@EXPECT_NORMAL );

    is( $obj->name, $name,          "   Expected file found" );


    ### extract this single file to cwd()
    for my $meth (qw[extract extract_file]) {

        ### extract it by full path and object
        for my $arg ( $obj, $obj->full_path ) {

            ok( $tar->$meth( $arg ),
                                    "   Extract '$name' to cwd() with $meth" );
            ok( -e $obj->full_path, "       Extracted file exists" );
            rm( $obj->full_path ) unless $NO_UNLINK;
        }
    }

    ### extract this file to @ROOT
    ### can only do that with 'extract_file', not with 'extract'
    for my $meth (qw[extract_file]) {
        my $outpath = File::Spec->catdir( @ROOT );
        my $outfile = File::Spec->catfile( $outpath, $$ ); #$obj->full_path );

        ok( $tar->$meth( $obj->full_path, $outfile ),
                                    "   Extract file '$name' to $outpath with $meth" );
        ok( -e $outfile,            "       Extracted file '$outfile' exists" );
        rm( $outfile ) unless $NO_UNLINK;
    }

}


### clear tests ###
SKIP: {                             ### pesky warnings
    skip $ebcdic_skip_msg, 3 if ord "A" != 65;

    my $tar     = $Class->new;
    my @files   = $tar->read( $TAR_FILE );

    my $cnt = $tar->list_files();
    ok( $cnt,                       "Found old data" );
    ok( $tar->clear,                "   Clearing old data" );

    my $new_cnt = $tar->list_files;
    ok( !$new_cnt,                  "   Old data cleared" );
}

### $DO_NOT_USE_PREFIX tests
{   my $tar     = $Class->new;


    ### first write a tar file without prefix
    {   my ($obj)   = $tar->add_files( $COMPRESS_FILE );
        my $dir     = '';   # dir is empty!
        my $file    = File::Basename::basename( $COMPRESS_FILE );

        ok( $obj,                   "File added" );
        isa_ok( $obj,               $FClass );

        ### internal storage ###
        is( $obj->name, $file,      "   Name set to '$file'" );
        is( $obj->prefix, $dir,     "   Prefix set to '$dir'" );

        ### write the tar file without a prefix in it
        ### pesky warnings
        local $Archive::Tar::DO_NOT_USE_PREFIX = 1;
        local $Archive::Tar::DO_NOT_USE_PREFIX = 1;

        ok( $tar->write( $OUT_TAR_FILE ),
                                    "   Tar file written" );

        ### and forget all about it...
        $tar->clear;
    }

    ### now read it back in, there should be no prefix
    {   ok( $tar->read( $OUT_TAR_FILE ),
                                    "   Tar file read in again" );

        my ($obj) = $tar->get_files;
        ok( $obj,                   "       File retrieved" );
        isa_ok( $obj, $FClass,      "       Object" );

        is( $obj->name, $COMPRESS_FILE,
                                    "       Name now set to '$COMPRESS_FILE'" );
        is( $obj->prefix, '',       "       Prefix now empty" );

        my $re = quotemeta $COMPRESS_FILE;
        like( $obj->raw, qr/^$re/,  "       Prefix + name in name slot of header" );
    }

    rm( $OUT_TAR_FILE ) unless $NO_UNLINK;
}

### clean up stuff
END {
    for my $struct ( \@EXPECT_NORMAL, \@EXPECTBIN, \@EXPECTX ) {
        for my $aref (@$struct) {

            my $dir = $aref->[0]->[0];
            rmtree $dir if $dir && -d $dir && not $NO_UNLINK;
        }
    }

    my ($dir) = File::Spec::Unix->splitdir( $LONG_FILE );
    rmtree $dir if $dir && -d $dir && not $NO_UNLINK;
    1 while unlink $COMPRESS_FILE;
}

###########################
###     helper subs     ###
###########################
sub get_expect {
    return  map {
                split '/', $_
            } map {
                File::Spec::Unix->catfile(
                    grep { defined } @{$_->[0]}, $_->[1]
                )
            } @EXPECT_NORMAL;
}

sub is_dir {
    my $file = pop();
    return $file =~ m|/$| ? 1 : 0;
}

sub rm {
    my $x = shift;
    if  ( is_dir($x) ) {
         rmtree($x);
    } else {
         1 while unlink $x;
    }
}

sub check_tar_file {
    my $file        = shift;
    my $filesize    = -s $file;
    my $contents    = slurp_binfile( $file );

    ok( defined( $contents ),   "   File read" );
    ok( $filesize,              "   File written size=$filesize" );

    cmp_ok( $filesize % BLOCK,     '==', 0,
                        "   File size is a multiple of 512" );

    cmp_ok( length($contents), '==', $filesize,
                        "   File contents match size" );

    is( TAR_END x 2, substr( $contents, -(BLOCK*2) ),
                        "   Ends with 1024 null bytes" );

    return $contents;
}

sub check_compressed_file {
    my $file                = shift;
    my $filesize            = -s $file;
    my $contents            = slurp_compressed_file( $file );
    my $uncompressedsize    = length $contents;

    ok( defined( $contents ),   "   File read and uncompressed" );
    ok( $filesize,              "   File written size=$filesize uncompressed size=$uncompressedsize" );

    cmp_ok( $uncompressedsize % BLOCK, '==', 0,
                                "   Uncompressed size is a multiple of 512" );

    is( TAR_END x 2, substr($contents, -(BLOCK*2)),
                                "   Ends with 1024 null bytes" );

    cmp_ok( $filesize, '<',  $uncompressedsize,
                                "   Compressed size < uncompressed size" );

    return $contents;
}

sub check_tar_object {
    my $obj     = shift;
    my $struct  = shift or return;

    ### amount of files (not dirs!) there should be in the object
    my $expect  = scalar @$struct;
    my @files   = grep { $_->is_file } $obj->get_files;

    ### count how many files there are in the object
    ok( scalar @files,          "   Found some files in the archive" );
    is( scalar @files, $expect, "   Found expected number of files" );

    for my $file (@files) {

        ### XXX ->fullname
        #my $path = File::Spec::Unix->catfile(
        #            grep { length } $file->prefix, $file->name );
        my($ename,$econtent) =
            get_expect_name_and_contents( $file->full_path, $struct );

        ok( $file->is_file,     "   It is a file" );
        is( $file->full_path, $ename,
                                "   Name matches expected name" );
        like( $file->get_content, $econtent,
                                "   Content as expected" );
    }
}

sub check_tar_extract {
    my $tar     = shift;
    my $struct  = shift;

    my @dirs;
    for my $file ($tar->get_files) {
        push @dirs, $file && next if $file->is_dir;


        my $path = $file->full_path;
        my($ename,$econtent) =
            get_expect_name_and_contents( $path, $struct );


        is( $ename, $path,          "   Expected file found" );
        ok( -e $path,               "   File '$path' exists" );

        my $fh;
        open $fh, "$path" or warn "Error opening file '$path': $!\n";
        binmode $fh;

        ok( $fh,                    "   Opening file" );

        my $content = do{local $/;<$fh>}; chomp $content;
        like( $content, qr/$econtent/,
                                    "   Contents OK" );

        close $fh;
        $NO_UNLINK or 1 while unlink $path;

        ### alternate extract path tests
        ### to abs and rel paths
        {   for my $outpath (   File::Spec->catdir( @ROOT ),
                                File::Spec->rel2abs(
                                    File::Spec->catdir( @ROOT )
                                )
            ) {

                my $outfile = File::Spec->catfile( $outpath, $$ );

                ok( $tar->extract_file( $file->full_path, $outfile ),
                                "   Extracted file '$path' to $outfile" );
                ok( -e $outfile,"   Extracted file '$outfile' exists" );

                rm( $outfile ) unless $NO_UNLINK;
            }
        }
    }

    ### now check if list_files is returning the same info as get_files
    is_deeply( [$tar->list_files], [ map { $_->full_path } $tar->get_files],
                                    "   Verified via list_files as well" );

    #do { rmtree $_->full_path if -d $_->full_path && not $NO_UNLINK }
    #    for @dirs;
}

sub slurp_binfile {
    my $file    = shift;
    my $fh      = IO::File->new;

    $fh->open( $file ) or warn( "Error opening '$file': $!" ), return undef;

    binmode $fh;
    local $/;
    return <$fh>;
}

sub slurp_compressed_file {
    my $file = shift;
    my $fh;

    ### xz
    if( $file =~ /.txz$/ ) {
        require IO::Uncompress::UnXz;
        $fh = IO::Uncompress::UnXz->new( $file )
            or warn( "Error opening '$file' with IO::Uncompress::UnXz" ), return

    ### bzip2
    } elsif( $file =~ /.tbz$/ ) {
        require IO::Uncompress::Bunzip2;
        $fh = IO::Uncompress::Bunzip2->new( $file )
            or warn( "Error opening '$file' with IO::Uncompress::Bunzip2" ), return

    ### gzip
    } else {
        require IO::Zlib;
        $fh = IO::Zlib->new();
        $fh->open( $file, READ_ONLY->(1) )
            or warn( "Error opening '$file' with IO::Zlib" ), return
    }

    my $str;
    my $buff;
    $str .= $buff while $fh->read( $buff, 4096 ) > 0;
    $fh->close();

    return $str;
}

sub get_expect_name_and_contents {
    my $find    = shift;
    my $struct  = shift or return;

    ### find the proper name + contents for this file from
    ### the expect structure
    my ($name, $content) =
        map {
            @$_;
        } grep {
            $_->[0] eq $find
        } map {
            [   ### full path ###
                File::Spec::Unix->catfile(
                    grep { length } @{$_->[0]}, $_->[1]
                ),
                ### regex
                $_->[2],
            ]
        } @$struct;

    ### not a qr// yet?
    unless( ref $content ) {
        my $x     = quotemeta ($content || '');
        $content = qr/$x/;
    }

    unless( $name ) {
        warn "Could not find '$find' in " . Dumper $struct;
    }

    return ($name, $content);
}

__END__
