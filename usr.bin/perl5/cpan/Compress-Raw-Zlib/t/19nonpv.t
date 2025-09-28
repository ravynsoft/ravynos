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

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 38 + $extra ;

    use_ok('Compress::Raw::Zlib', 2) ;
}

use CompTestUtils;


my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;

# Check zlib_version and ZLIB_VERSION are the same.
test_zlib_header_matches_library();


{
    title 'non-PV dictionary';
    # ==============================

    my $dictionary = *hello ;

    ok my $x = new Compress::Raw::Zlib::Deflate({-Level => Z_BEST_COMPRESSION,
			     -Dictionary => $dictionary}) ;

    my $dictID = $x->dict_adler() ;

    my ($X, $Y, $Z);
    cmp_ok $x->deflate($hello, $X), '==', Z_OK;
    cmp_ok $x->flush($Y), '==', Z_OK;
    $X .= $Y ;

    ok my $k = new Compress::Raw::Zlib::Inflate(-Dictionary => $dictionary) ;

    cmp_ok $k->inflate($X, $Z), '==', Z_STREAM_END;
    is $k->dict_adler(), $dictID;
    is $hello, $Z ;

}

{

    title  "deflate/inflate - non-PV buffers";
    # ==============================

    my $hello = *hello ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate, "Create deflate object" );
    ok $x, "Compress::Raw::Zlib::Deflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    ok ! defined $x->msg() ;
    is $x->total_in(), 0, "total_in() == 0" ;
    is $x->total_out(), 0, "total_out() == 0" ;

    $X = *X;
    my $Answer = '';
    $status = $x->deflate($hello, $X) ;
    $Answer .= $X ;

    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    $X = *X;
    cmp_ok  $x->flush($X), '==', Z_OK, "flush returned Z_OK" ;
    $Answer .= $X ;

    ok ! defined $x->msg()  ;
    is $x->total_in(), length $hello, "total_in ok" ;
    is $x->total_out(), length $Answer, "total_out ok" ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate);
    ok $k, "Compress::Raw::Zlib::Inflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    ok ! defined $k->msg(), "No error messages" ;
    is $k->total_in(), 0, "total_in() == 0" ;
    is $k->total_out(), 0, "total_out() == 0" ;
    my $GOT = '';
    my $Z;
    $Z = *Z;
    my $Alen = length $Answer;
    $status = $k->inflate($Answer, $Z) ;
    $GOT .= $Z ;

    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
    ok ! defined $k->msg(), "No error messages" ;
    is $k->total_in(), $Alen, "total_in ok" ;
    is $k->total_out(), length $hello , "total_out ok";


    ok(($k, $err) = new Compress::Raw::Zlib::Inflate);
    ok $k, "Compress::Raw::Zlib::Inflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $Z = *Z;
    $status = $k->inflate($hello, $Z);
    is $Z, "", 'inflating *hello does not crash';

    $hello = *hello;
    $status = $k->inflateSync($hello);
    cmp_ok $status, "!=", Z_OK,
       "inflateSync on *hello returns error (and does not crash)";
}
