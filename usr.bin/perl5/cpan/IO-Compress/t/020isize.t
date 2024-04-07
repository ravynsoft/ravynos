BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict ;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

BEGIN
{
    plan skip_all => "Lengthy Tests Disabled\n" .
                     "set COMPRESS_ZLIB_RUN_ALL to run this test suite"
        unless defined $ENV{COMPRESS_ZLIB_RUN_ALL} ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 76 + $extra ;


    use_ok('Compress::Zlib', 2) ;
    use_ok('IO::Compress::Gzip', qw($GzipError)) ;
    use_ok('IO::Uncompress::Gunzip', qw($GunzipError)) ;
    use_ok('IO::Compress::Gzip::Constants');
}

my $compressed ;
my $expected_crc ;

for my $wrap (0 .. 2)
{
    for my $offset ( -1 .. 1 )
    {
        next if $wrap == 0 && $offset < 0 ;

        title "Wrap $wrap, Offset $offset" ;

        my $size = (GZIP_ISIZE_MAX * $wrap) + $offset ;

        my $expected_isize ;
        if ($wrap == 0) {
            $expected_isize = $offset ;
        }
        elsif ($wrap == 1 && $offset <= 0) {
            $expected_isize = GZIP_ISIZE_MAX + $offset ;
        }
        elsif ($wrap > 1) {
            $expected_isize = GZIP_ISIZE_MAX + $offset - 1;
        }
        else {
            $expected_isize = $offset - 1;
        }

        sub gzipClosure
        {
            my $gzip = shift ;
            my $max = shift  ;

            my $index = 0 ;
            my $inc = 1024 * 5000 ;
            my $buff = 'x' x $inc ;
            my $left = $max ;

            return
                sub {

                    if ($max == 0 && $index == 0) {
                        $expected_crc = crc32('') ;
                        ok $gzip->close(), '  IO::Compress::Gzip::close ok X' ;
                        ++ $index ;
                        $_[0] .= $compressed;
                        return length $compressed ;
                    }

                    return 0 if $index >= $max ;

                    while ( ! length $compressed )
                    {
                        $index += $inc ;

                        if ($index <= $max) {
                            $gzip->write($buff) ;
                            #print "Write " . length($buff) . "\n" ;
                            #print "# LEN Compressed " . length($compressed) . "\n" ;
                            $expected_crc = crc32($buff, $expected_crc) ;
                            $left -= $inc ;
                        }
                        else  {
                            #print "Write $left\n" ;
                            $gzip->write('x' x $left) ;
                            #print "# LEN Compressed " . length($compressed) . "\n" ;
                            $expected_crc = crc32('x' x $left, $expected_crc) ;
                            ok $gzip->close(), '  IO::Compress::Gzip::close ok ' ;
                            last ;
                        }
                    }

                    my $len = length $compressed ;
                    $_[0] .= $compressed ;
                    $compressed = '';
                    #print "# LEN $len\n" if $len <=0 ;

                    return $len ;
                };
        }

        my $gzip = IO::Compress::Gzip->new( \$compressed,
                                -Append     => 0,
                                -HeaderCRC  => 1 );

        ok $gzip, "  Created IO::Compress::Gzip object";

        my $gunzip = IO::Uncompress::Gunzip->new( gzipClosure($gzip, $size),
                                    -BlockSize  => 1024 * 500 ,
                                    -Append => 0,
                                    -Strict => 1 );

        ok $gunzip, "  Created IO::Uncompress::Gunzip object";

        my $inflate = *$gunzip->{Inflate} ;
        my $deflate = *$gzip->{Deflate} ;

        my $status ;
        my $uncompressed;
        my $actual = 0 ;
        while (($status = $gunzip->read($uncompressed)) > 0) {
            #print "# READ $status\n" ;
            $actual += $status ;
        }

        is $status, 0, '  IO::Uncompress::Gunzip::read returned 0'
            or diag "error status is $status, error is $GunzipError" ;

        ok $gunzip->close(), "  IO::Uncompress::Gunzip Closed ok" ;

        is $actual, $size, "  Length of Gunzipped data is $size"
            or diag "Expected $size, got $actual";

        my $gunzip_hdr = $gunzip->getHeaderInfo();

        is $gunzip_hdr->{ISIZE}, $expected_isize,
            sprintf("  ISIZE is $expected_isize [0x%X]", $expected_isize);
        is $gunzip_hdr->{CRC32}, $expected_crc,
            sprintf("  CRC32 is $expected_crc [0x%X]", $expected_crc);

        $expected_crc = 0 ;
    }
}
