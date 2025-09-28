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

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 88 + $extra ;

    use_ok('Compress::Raw::Bzip2') ;
}



my $hello = "I am a HAL 9000 computer" x 2001;
my $tmp = $hello ;

my ($err, $x, $X, $status);

ok( ($x, $err) = new Compress::Raw::Bzip2 (1));
ok $x ;
cmp_ok $err, '==', BZ_OK, "  status is BZ_OK" ;

my $out ;
$status = $x->bzdeflate($tmp, $out) ;
cmp_ok $status, '==', BZ_RUN_OK, "  status is BZ_RUN_OK" ;

cmp_ok $x->bzclose($out), '==', BZ_STREAM_END, "  bzflush returned BZ_STREAM_END" ;

{
    my $t = $out;
    my $b = new Compress::Raw::Bunzip2(0,0);

    my $GOT;
    my $status = $b->bzinflate($t, $GOT) ;
    cmp_ok $status, "==", BZ_STREAM_END;
    ok $GOT eq $hello;

}

sub getOut { my $x = ''; return \$x }

for my $bufsize (1, 2, 3, 13, 4096, 1024*10)
{
    print "#\n#Bufsize $bufsize\n#\n";
    $tmp = $out;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2( 1,1,0,0,1
                                                      #AppendOutput => 1,
                                                      #LimitOutput => 1,
                                                      #Bufsize => $bufsize
                                                    ));
    ok $k ;
    cmp_ok $err, '==', BZ_OK, "  status is BZ_OK" ;

    is $k->total_in_lo32(), 0, "  total_in_lo32 == 0" ;
    is $k->total_out_lo32(), 0, "  total_out_lo32 == 0" ;
    my $GOT = getOut();
    my $prev;
    my $deltaOK = 1;
    my $looped = 0;
    while (length $tmp)
    {
        ++ $looped;
        my $prev = length $GOT;
        $status = $k->bzinflate($tmp, $GOT) ;
        last if $status != BZ_OK;
        $deltaOK = 0 if length($GOT) - $prev > $bufsize;
    }

    ok $deltaOK, "  Output Delta never > $bufsize";
    cmp_ok $looped, '>=', 1, "  looped $looped";
    is length($tmp), 0, "  length of input buffer is zero";

    cmp_ok $status, "==", BZ_STREAM_END, "  status is BZ_STREAM_END" ;
    ok $$GOT eq $hello, "  got expected output" ;
    is $k->total_in_lo32(), length $out, "  length total_in_lo32 ok" ;
    is $k->total_out_lo32(), length $hello, "  length total_out_lo32 ok " .  $k->total_out_lo32() ;
}

sub getit
{
    my $obj = shift ;
    my $input = shift;

    my $data ;
    1 while $obj->bzinflate($input, $data) != BZ_STREAM_END ;
    return \$data ;
}

{
    title "regression test";

    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2 (1));
    ok $x ;
    cmp_ok $err, '==', BZ_OK, "  status is BZ_OK" ;

    my $line1 = ("abcdefghijklmnopq" x 1000) . "\n" ;
    my $line2 = "second line\n" ;
    my $text = $line1 . $line2 ;
    my $tmp = $text;

    my $out ;
    $status = $x->bzdeflate($tmp, $out) ;
    cmp_ok $status, '==', BZ_RUN_OK, "  status is BZ_RUN_OK" ;

    cmp_ok $x->bzclose($out), '==', BZ_STREAM_END, "  bzclose returned BZ_STREAM_END" ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2( 1,1,0,0,1
            #AppendOutput => 1,
            #LimitOutput => 1
                                                    ));


    my $c = getit($k, $out);
    is $$c, $text;


}
