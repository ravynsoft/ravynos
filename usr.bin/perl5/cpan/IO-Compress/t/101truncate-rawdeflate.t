BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use Test::More ;

use Compress::Raw::Zlib;

BEGIN {
    plan skip_all => "Lengthy Tests Disabled\n" .
                     "set COMPRESS_ZLIB_RUN_ALL or COMPRESS_ZLIB_RUN_MOST to run this test suite"
        unless defined $ENV{COMPRESS_ZLIB_RUN_ALL} or defined $ENV{COMPRESS_ZLIB_RUN_MOST};

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    my $tests = Compress::Raw::Zlib::is_zlibng() ? 615 : 625;
    plan tests => $tests + $extra;
};


use IO::Compress::RawDeflate   qw($RawDeflateError) ;
use IO::Uncompress::RawInflate qw($RawInflateError) ;

#sub identify
#{
#    'IO::Compress::RawDeflate';
#}
#
#require "truncate.pl" ;
#run();

use CompTestUtils;

my $hello = <<EOM ;
hello world
this is a test
some more stuff on this line
ad finally...
EOM

my $blocksize = 10 ;


foreach my $CompressClass ( 'IO::Compress::RawDeflate')
{
    my $UncompressClass = getInverse($CompressClass);
    my $Error = getErrorRef($UncompressClass);

    my $compressed ;
        ok( my $x = IO::Compress::RawDeflate->new( \$compressed ) );
        ok $x->write($hello) ;
        ok $x->close ;


    my $cc = $compressed ;

    my $gz ;
    ok($gz = $UncompressClass->can('new')->( $UncompressClass, \$cc,
                                  -Transparent => 0))
            or diag "$$Error\n";
    my $un;
    is $gz->read($un, length($hello)), length($hello);
    ok $gz->close();
    is $un, $hello ;

    for my $trans (0 .. 1)
    {
        title "Testing $CompressClass, Transparent = $trans";

        my $info = $gz->getHeaderInfo() ;
        my $header_size = $info->{HeaderLength};
        my $trailer_size = $info->{TrailerLength};
        ok 1, "Compressed size is " . length($compressed) ;
        ok 1, "Header size is $header_size" ;
        ok 1, "Trailer size is $trailer_size" ;


        title "Compressed Data Truncation";
        foreach my $i (0 .. $blocksize)
        {

            my $lex = LexFile->new( my $name );

            ok 1, "Length $i" ;
            my $part = substr($compressed, 0, $i);
            writeFile($name, $part);
            my $gz = $UncompressClass->can('new')->( $UncompressClass, $name,
                                       -BlockSize   => $blocksize,
                                       -Transparent => $trans );
            if ($trans) {
                ok $gz;
                ok ! $gz->error() ;
                my $buff = '';
                is $gz->read($buff, length $part), length $part ;
                is $buff, $part ;
                ok $gz->eof() ;
                $gz->close();
            }
            else {
                ok !$gz;
            }
        }

        foreach my $i ($blocksize+1 .. length($compressed)-1)
        {

            my $lex = LexFile->new( my $name );

            ok 1, "Length $i" ;
            my $part = substr($compressed, 0, $i);
            writeFile($name, $part);
            ok my $gz = $UncompressClass->can('new')->( $UncompressClass, $name,
                                             -BlockSize   => $blocksize,
                                             -Transparent => $trans );
            my $un ;
            my $status = 1 ;
            $status = $gz->read($un) while $status > 0 ;
            ok $status < 0 ;
            ok $gz->eof() ;
            ok $gz->error() ;
            $gz->close();
        }
    }

}
