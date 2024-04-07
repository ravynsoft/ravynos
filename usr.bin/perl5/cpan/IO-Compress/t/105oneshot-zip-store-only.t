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

BEGIN {
    plan(skip_all => "oneshot needs Perl 5.005 or better - you have Perl $]" )
        if $] < 5.005 ;

    plan skip_all => "Lengthy Tests Disabled\n" .
                     "set COMPRESS_ZLIB_RUN_ALL or COMPRESS_ZLIB_RUN_MOST to run this test suite"
        unless defined $ENV{COMPRESS_ZLIB_RUN_ALL} or defined $ENV{COMPRESS_ZLIB_RUN_MOST};

    plan(skip_all => "IO::Compress::Bzip2 not available" )
        unless eval { require IO::Compress::Bzip2;
                      require IO::Uncompress::Bunzip2;
                      1
                    } ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 1058 + $extra ;

    use_ok('IO::Compress::Zip', qw(:all)) ;
    use_ok('IO::Uncompress::Unzip', qw(unzip $UnzipError)) ;
}

my @contents;
my $content = "x" x 1025;
$content .= "\x50" ;

push @contents, $content ;

$content .= "y" x 321 ;
$content .= "\x50\x4b" ;
push @contents, $content ;

$content .= "z" x 21 ;
$content .= "\x50\x4b\x07" . "a" x 73 ;
push @contents, $content ;

$content .= "a" x 73 ;
$content .= "\x50\x4b\x07\x08" ;
push @contents, $content ;

$content .= "b" x 102 ;
$content .= "\x50\x4b\x07\x08" . "\x50\x4b\x07\x08" ;
push @contents, $content ;

$content .= "c" x 102 ;
push @contents, $content ;


my $index = 0;
for $content (@contents)
{
    ++ $index ;
    my $contentLen = length $content ;


    for my $stream (0, 1)
    {
        for my $zip64 (0, 1)
        {
            for my $blockSize (1 .. 7, $contentLen, $contentLen-1, $contentLen +1, 16*1024)
            {
                title "Index $index, Stream $stream, Zip64 $zip64, BlockSize $blockSize";

                my $crc = Compress::Raw::Zlib::crc32($content);
                $content .= "\x50\x4b\x07\x08" . pack("V", $crc) . "b" x 53 ;

                my $zipped ;

                ok zip(\$content => \$zipped , Method => ZIP_CM_STORE,
                                               Zip64  => $zip64,
                                               Stream => $stream), " zip ok"
                    or diag $ZipError ;

                my $got ;
                ok unzip(\$zipped => \$got, BlockSize => $blockSize), "  unzip ok"
                    or diag $UnzipError ;

                is $got, $content, "  content ok";

            }
        }
    }
}
