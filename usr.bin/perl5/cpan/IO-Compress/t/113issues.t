BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 't' if -d 't';
        @INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;
use Data::Dumper;

use IO::Compress::Zip     qw($ZipError);
use IO::Uncompress::Unzip qw($UnzipError);

BEGIN {
    plan skip_all => "Encode is not available"
        if $] < 5.006 ;

    eval { require Encode; Encode->import(); };

    plan skip_all => "Encode is not available"
        if $@ ;

    plan skip_all => "Encode not working in perl $]"
        if $] >= 5.008 && $] < 5.008004 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 3 + $extra;
}

{
    title "github-34: Calling nextStream on an IO::Uncompress::Zip object in Transparent mode dies when input is uncompressed";
    # https://github.com/pmqs/IO-Compress/issues/34

    my $lex = LexFile->new( my $file1 );

    writeFile($file1, "1234\n5678\n");

    my $in = IO::Uncompress::Unzip->new( $file1,
                                        AutoClose => 1,
                                        Transparent => 1
                                     ) or
        die( "foo.txt: $UnzipError\n" );

    my $data;
    my $status;

    # read first stream
    $data .= $_
        while <$in> ;

    is $data, "1234\n5678\n" ;

    # This line triggers the error below without a fix
    #     Can't call method "reset" on an undefined value
    is $in->nextStream, 0 ;
}
