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

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 108 + $extra ;

    use_ok('Compress::Raw::Zlib', 2) ;
}

use CompTestUtils;

test_zlib_header_matches_library();

my $hello = "I am a HAL 9000 computer" x 2001;
my $tmp = $hello ;

my ($err, $x, $X, $status);

ok( ($x, $err) = new Compress::Raw::Zlib::Deflate (-AppendOutput => 1));
ok $x ;
cmp_ok $err, '==', Z_OK, "  status is Z_OK" ;

my $out ;
$status = $x->deflate($tmp, $out) ;
cmp_ok $status, '==', Z_OK, "  status is Z_OK" ;

cmp_ok $x->flush($out), '==', Z_OK, "  flush returned Z_OK" ;


sub getOut { my $x = ''; return \$x }

for my $bufsize (1, 2, 3, 13, 4096, 1024*10)
{
    print "#\n#Bufsize $bufsize\n#\n";
    $tmp = $out;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate( AppendOutput => 1,
                                                      LimitOutput => 1,
                                                      Bufsize => $bufsize
                                                    ));
    ok $k ;
    cmp_ok $err, '==', Z_OK, "  status is Z_OK" ;

    ok ! defined $k->msg(), "  no msg" ;
    is $k->total_in(), 0, "  total_in == 0" ;
    is $k->total_out(), 0, "  total_out == 0" ;
    my $GOT = getOut();
    my $prev;
    my $deltaOK = 1;
    my $looped = 0;
    while (length $tmp)
    {
        ++ $looped;
        my $prev = length $GOT;
        $status = $k->inflate($tmp, $GOT) ;
        last if $status == Z_STREAM_END || $status == Z_DATA_ERROR || $status == Z_STREAM_ERROR ;
        $deltaOK = 0 if length($GOT) - $prev > $bufsize;
    }

    ok $deltaOK, "  Output Delta never > $bufsize";
    cmp_ok $looped, '>=', 1, "  looped $looped";
    is length($tmp), 0, "  length of input buffer is zero";

    cmp_ok $status, '==', Z_STREAM_END, "  status is Z_STREAM_END" ;
    is $$GOT, $hello, "  got expected output" ;
    ok ! defined $k->msg(), "  no msg" ;
    is $k->total_in(), length $out, "  length total_in ok" ;
    is $k->total_out(), length $hello, "  length total_out ok " .  $k->total_out() ;
}

sub getit
{
    my $obj = shift ;
    my $input = shift;

    my $data ;
    1 while $obj->inflate($input, $data) != Z_STREAM_END ;
    return \$data ;
}

{
    title "regression test";

    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate (-AppendOutput => 1));
    ok $x ;
    cmp_ok $err, '==', Z_OK, "  status is Z_OK" ;

    my $line1 = ("abcdefghijklmnopq" x 1000) . "\n" ;
    my $line2 = "second line\n" ;
    my $text = $line1 . $line2 ;
    my $tmp = $text;

    my $out ;
    $status = $x->deflate($tmp, $out) ;
    cmp_ok $status, '==', Z_OK, "  status is Z_OK" ;

    cmp_ok $x->flush($out), '==', Z_OK, "  flush returned Z_OK" ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate( AppendOutput => 1,
                                                      LimitOutput => 1
                                                    ));


    my $c = getit($k, $out);
    is $$c, $text;


}

{
    title "regression test for #92521: Z_OK instead of Z_BUF_ERROR";

    # 1M "aaa..."
    my $in = 'a' x 100000;
    my ($deflate, $err) = Compress::Raw::Zlib::Deflate->new(WindowBits => -15,
        MemLevel => 8);
    ok $deflate ;
    cmp_ok $err, '==', Z_OK, "  status is Z_OK" ;

    my $status = $deflate->deflate($in, my $zip);
    cmp_ok $status, '==', Z_OK, "  status is Z_OK" ;

    cmp_ok $deflate->flush($zip, Z_SYNC_FLUSH), "==", Z_OK;

    # Compression should stop after 10K "aaa..." with Z_BUF_ERROR
    my $inflate;
    ($inflate, $err)  = Compress::Raw::Zlib::Inflate->new( Bufsize => 10000,
        LimitOutput => 1, WindowBits => -15 );
    ok $inflate ;
    cmp_ok $err, '==', Z_OK, "  status is Z_OK" ;

    $status = $inflate->inflate($zip, my $out);

    cmp_ok length($out), ">=", 10000;
    #warn 'RESULT: ', length($out), ' of ', length($in), "\n";
    cmp_ok $status, '==', Z_BUF_ERROR, "  status is Z_BUF_ERROR" ;

}
