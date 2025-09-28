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
use CompTestUtils;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 21 + $extra ;

    use_ok('Compress::Raw::Bzip2', 2) ;
}



my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;


{

    title  "bzdeflate/bzinflate - non-PV buffers";
    # ==============================

    my $hello = *hello;
    $hello = *hello;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2(0), "Create bzdeflate object" );
    ok $x, "Compress::Raw::Bzip2 ok" ;
    cmp_ok $err, '==', BZ_OK, "status is BZ_OK" ;

    is $x->uncompressedBytes(), 0, "uncompressedBytes() == 0" ;
    is $x->compressedBytes(), 0, "compressedBytes() == 0" ;

    my $Answer = *Answer;
    $Answer = *Answer;
    $status = $x->bzdeflate($hello, $Answer) ;
    cmp_ok $status, '==', BZ_RUN_OK, "bzdeflate returned BZ_RUN_OK" ;

    $X = *X;
    cmp_ok  $x->bzflush($X), '==', BZ_RUN_OK, "bzflush returned BZ_RUN_OK" ;
    $Answer .= $X ;

    is $x->uncompressedBytes(), length $hello, "uncompressedBytes ok" ;
    is $x->compressedBytes(), length $Answer, "compressedBytes ok" ;

    $X = *X;
    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END, "bzclose returned BZ_STREAM_END";
    $Answer .= $X ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(0, 0));
    ok $k, "Compress::Raw::Bunzip2 ok" ;
    cmp_ok $err, '==', BZ_OK, "status is BZ_OK" ;

    is $k->compressedBytes(), 0, "compressedBytes() == 0" ;
    is $k->uncompressedBytes(), 0, "uncompressedBytes() == 0" ;
    my $GOT = *GOT;
    $GOT = *GOT;
    my $Z;
    $status = $k->bzinflate($Answer, $GOT) ;


    cmp_ok $status, '==', BZ_STREAM_END, "Got BZ_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
    is $k->compressedBytes(), length $Answer, "compressedBytes ok" ;
    is $k->uncompressedBytes(), length $hello , "uncompressedBytes ok";

}
