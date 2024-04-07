BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
    #@INC = ("../lib", "lib/compress");
	@INC = ("../lib");
    }
}

use lib 't';
use strict;
use warnings;
use bytes;

use Test::More  ;
#use CompTestUtils;


BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };


    my $count = 0 ;
    if ($] < 5.005) {
        $count = 127 ;
    }
    elsif ($] >= 5.006) {
        $count = 197 ;
    }
    else {
        $count = 155 ;
    }

    plan tests => $count + $extra;

    use_ok('Compress::Raw::Bzip2') ;
}

sub title
{
    #diag "" ;
    ok 1, $_[0] ;
    #diag "" ;
}

sub mkErr
{
    my $string = shift ;
    my ($dummy, $file, $line) = caller ;
    -- $line ;

    $string = quotemeta $string;
    $file = quotemeta($file);

    #return "/$string\\s+at $file line $line/" if $] >= 5.006 ;
    return "/$string\\s+at /" ;
}

sub mkEvalErr
{
    my $string = shift ;

    return "/$string\\s+at \\(eval /" if $] > 5.006 ;
    return "/$string\\s+at /" ;
}



my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;

{
    title "Error Cases" ;

    eval { new Compress::Raw::Bzip2(1,2,3,4,5,6) };
    like $@,  mkErr "Usage: Compress::Raw::Bzip2::new(className, appendOut=1, blockSize100k=1, workfactor=0, verbosity=0)";

}


{

    title  "bzdeflate/bzinflate - small buffer";
    # ==============================

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2(0), "Create bzdeflate object" );
    ok $x, "Compress::Raw::Bzip2 ok" ;
    cmp_ok $err, '==', BZ_OK, "status is BZ_OK" ;

    is $x->uncompressedBytes(), 0, "uncompressedBytes() == 0" ;
    is $x->compressedBytes(), 0, "compressedBytes() == 0" ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->bzdeflate($_, $X) ;
        last unless $status == BZ_RUN_OK ;

        $Answer .= $X ;
    }

    cmp_ok $status, '==', BZ_RUN_OK, "bzdeflate returned BZ_RUN_OK" ;

    cmp_ok  $x->bzflush($X), '==', BZ_RUN_OK, "bzflush returned BZ_RUN_OK" ;
    $Answer .= $X ;

    is $x->uncompressedBytes(), length $hello, "uncompressedBytes ok" ;
    is $x->compressedBytes(), length $Answer, "compressedBytes ok" ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END, "bzclose returned BZ_STREAM_END";
    $Answer .= $X ;

    #open F, ">/tmp/xx1"; print F $Answer ; close F;
    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(0, 0));
    ok $k, "Compress::Raw::Bunzip2 ok" ;
    cmp_ok $err, '==', BZ_OK, "status is BZ_OK" ;

    is $k->compressedBytes(), 0, "compressedBytes() == 0" ;
    is $k->uncompressedBytes(), 0, "uncompressedBytes() == 0" ;
    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, $Z) ;
        $GOT .= $Z ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;

    }

    cmp_ok $status, '==', BZ_STREAM_END, "Got BZ_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
    is $k->compressedBytes(), length $Answer, "compressedBytes ok" ;
    is $k->uncompressedBytes(), length $hello , "uncompressedBytes ok";

}


{
    # bzdeflate/bzinflate - small buffer with a number
    # ==============================

    my $hello = 6529 ;

    ok  my ($x, $err) = new Compress::Raw::Bzip2 (1) ;
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    my $status;
    my $Answer = '';

    cmp_ok $x->bzdeflate($hello, $Answer), '==', BZ_RUN_OK ;

    cmp_ok $x->bzclose($Answer), '==', BZ_STREAM_END, "bzclose returned BZ_STREAM_END";

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(1, 0) );
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    #my $GOT = '';
    my $GOT ;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, $GOT) ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;

    }

    cmp_ok $status, '==', BZ_STREAM_END ;
    is $GOT, $hello ;

}

{

# bzdeflate/bzinflate options - AppendOutput
# ================================

    # AppendOutput
    # CRC

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;

    ok  my ($x, $err) = new Compress::Raw::Bzip2 (1) ;
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    my $status;
    my $X;
    foreach (@hello)
    {
        $status = $x->bzdeflate($_, $X) ;
        last unless $status == BZ_RUN_OK ;
    }

    cmp_ok $status, '==', BZ_RUN_OK ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END ;


    my @Answer = split('', $X) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2( {-Bufsize => 1, -AppendOutput =>1}));
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    my $Z;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, $Z) ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;

    }

    cmp_ok $status, '==', BZ_STREAM_END ;
    is $Z, $hello ;
}


{

    title "bzdeflate/bzinflate - larger buffer";
    # ==============================

    # generate a long random string
    my $contents = '' ;
    foreach (1 .. 50000)
      { $contents .= chr int rand 255 }


    ok my ($x, $err) = new Compress::Raw::Bzip2(0) ;
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    my (%X, $Y, %Z, $X, $Z);
    #cmp_ok $x->bzdeflate($contents, $X{key}), '==', BZ_RUN_OK ;
    cmp_ok $x->bzdeflate($contents, $X), '==', BZ_RUN_OK ;

    #$Y = $X{key} ;
    $Y = $X ;


    #cmp_ok $x->bzflush($X{key}), '==', BZ_RUN_OK ;
    #$Y .= $X{key} ;
    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END ;
    $Y .= $X ;



    my $keep = $Y ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(0, 0) );
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    #cmp_ok $k->bzinflate($Y, $Z{key}), '==', BZ_STREAM_END ;
    #ok $contents eq $Z{key} ;
    cmp_ok $k->bzinflate($Y, $Z), '==', BZ_STREAM_END ;
    ok $contents eq $Z ;

    # redo bzdeflate with AppendOutput

    ok (($k, $err) = new Compress::Raw::Bunzip2(1, 0)) ;
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    my $s ;
    my $out ;
    my @bits = split('', $keep) ;
    foreach my $bit (@bits) {
        $s = $k->bzinflate($bit, $out) ;
    }

    cmp_ok $s, '==', BZ_STREAM_END ;

    ok $contents eq $out ;


}


for my $consume ( 0 .. 1)
{
    title "bzinflate - check remaining buffer after BZ_STREAM_END, Consume $consume";

    ok my $x = new Compress::Raw::Bzip2(0) ;

    my ($X, $Y, $Z);
    cmp_ok $x->bzdeflate($hello, $X), '==', BZ_RUN_OK;
    cmp_ok $x->bzclose($Y), '==', BZ_STREAM_END;
    $X .= $Y ;

    ok my $k = new Compress::Raw::Bunzip2(0, $consume) ;

    my $first = substr($X, 0, 2) ;
    my $remember_first = $first ;
    my $last  = substr($X, 2) ;
    cmp_ok $k->bzinflate($first, $Z), '==', BZ_OK;
    if ($consume) {
        ok $first eq "" ;
    }
    else {
        ok $first eq $remember_first ;
    }

    my $T ;
    $last .= "appendage" ;
    my $remember_last = $last ;
    cmp_ok $k->bzinflate($last, $T),  '==', BZ_STREAM_END;
    is $hello, $Z . $T  ;
    if ($consume) {
        is $last, "appendage" ;
    }
    else {
        is $last, $remember_last ;
    }

}


{
    title "ConsumeInput and a read-only buffer trapped" ;

    ok my $k = new Compress::Raw::Bunzip2(0, 1) ;

    my $Z;
    eval { $k->bzinflate("abc", $Z) ; };
    like $@, mkErr("Compress::Raw::Bunzip2::bzinflate input parameter cannot be read-only when ConsumeInput is specified");

}

foreach (1 .. 2)
{
    next if $] < 5.005 ;

    title 'test bzinflate/bzdeflate with a substr';

    my $contents = '' ;
    foreach (1 .. 5000)
      { $contents .= chr int rand 255 }
    ok  my $x = new Compress::Raw::Bzip2(1) ;

    my $X ;
    my $status = $x->bzdeflate(substr($contents,0), $X);
    cmp_ok $status, '==', BZ_RUN_OK ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END  ;

    my $append = "Appended" ;
    $X .= $append ;

    ok my $k = new Compress::Raw::Bunzip2(1, 1) ;

    my $Z;
    my $keep = $X ;
    $status = $k->bzinflate(substr($X, 0), $Z) ;

    cmp_ok $status, '==', BZ_STREAM_END ;
    #print "status $status X [$X]\n" ;
    is $contents, $Z ;
    ok $X eq $append;
    #is length($X), length($append);
    #ok $X eq $keep;
    #is length($X), length($keep);
}

title 'Looping Append test - checks that deRef_l resets the output buffer';
foreach (1 .. 2)
{

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2 (0) );
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->bzdeflate($_, $X) ;
        last unless $status == BZ_RUN_OK ;

        $Answer .= $X ;
    }

    cmp_ok $status, '==', BZ_RUN_OK ;

    cmp_ok  $x->bzclose($X), '==', BZ_STREAM_END ;
    $Answer .= $X ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(1, 0) );
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    my $GOT ;
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, $GOT) ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;
    }

    cmp_ok $status, '==', BZ_STREAM_END ;
    is $GOT, $hello ;

}

if ($] >= 5.005)
{
    title 'test bzinflate input parameter via substr';

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    ok my $x = new Compress::Raw::Bzip2 (1);

    cmp_ok $x->bzdeflate($data, $X), '==',  BZ_RUN_OK ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END ;

    my $append = "Appended" ;
    $X .= $append ;
    my $keep = $X ;

    ok my $k = new Compress::Raw::Bunzip2 ( 1, 1);

#    cmp_ok $k->bzinflate(substr($X, 0, -1), $Z), '==', BZ_STREAM_END ; ;
    cmp_ok $k->bzinflate(substr($X, 0), $Z), '==', BZ_STREAM_END ; ;

    ok $hello eq $Z ;
    is $X, $append;

    $X = $keep ;
    $Z = '';
    ok $k = new Compress::Raw::Bunzip2 ( 1, 0);

    cmp_ok $k->bzinflate(substr($X, 0, -1), $Z), '==', BZ_STREAM_END ; ;
    #cmp_ok $k->bzinflate(substr($X, 0), $Z), '==', BZ_STREAM_END ; ;

    ok $hello eq $Z ;
    is $X, $keep;

}


{
    title 'RT#132734: test inflate append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Bzip2/issues/2

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    ok my $x = new Compress::Raw::Bzip2 ( {-AppendOutput => 1} );

    cmp_ok $x->bzdeflate($data, $X), '==',  BZ_RUN_OK ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END ;

    ok my $k = new Compress::Raw::Bunzip2 ( {-AppendOutput => 1,
                                             -ConsumeInput => 1} ) ;
    $Z = 'prev. ' ;
    substr($Z, 0, 4, ''); # chop off first 4 characters using offset
    cmp_ok $Z, 'eq', '. ' ;

    # use Devel::Peek ; Dump($Z) ; # shows OOK flag

    # if (1) { # workaround
    #     my $prev = $Z;
    #     undef $Z ;
    #     $Z = $prev ;
    # }

    cmp_ok $k->bzinflate($X, $Z), '==', BZ_STREAM_END ;
    # use Devel::Peek ; Dump($Z) ; # No OOK flag

    cmp_ok $Z, 'eq', ". $hello" ;
}


{
    title 'RT#132734: test deflate append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Bzip2/issues/2

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    $X = 'prev. ' ;
    substr($X, 0, 6, ''); # chop off all characters using offset
    cmp_ok $X, 'eq', '' ;

    # use Devel::Peek ; Dump($X) ; # shows OOK flag

    # if (1) { # workaround
    #     my $prev = $Z;
    #     undef $Z ;
    #     $Z = $prev ;
    # }

    ok my $x = new Compress::Raw::Bzip2 ( { -AppendOutput => 1 } );

    cmp_ok $x->bzdeflate($data, $X), '==',  BZ_RUN_OK ;

    cmp_ok $x->bzclose($X), '==', BZ_STREAM_END ;

    ok my $k = new Compress::Raw::Bunzip2 ( {-AppendOutput => 1,
                                             -ConsumeInput => 1} ) ;
    cmp_ok $k->bzinflate($X, $Z), '==', BZ_STREAM_END ;

    is $Z, $hello ;
}


{
    title 'RT#132734: test flush append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Bzip2/issues/2

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    my $F = 'prev. ' ;
    substr($F, 0, 6, ''); # chop off all characters using offset
    cmp_ok $F, 'eq', '' ;

    # use Devel::Peek ; Dump($F) ; # shows OOK flag

    ok my $x = new Compress::Raw::Bzip2 ( {-AppendOutput => 1 });

    cmp_ok $x->bzdeflate($data, $X), '==',  BZ_RUN_OK ;

    cmp_ok $x->bzclose($F), '==', BZ_STREAM_END ;

    ok my $k = new Compress::Raw::Bunzip2 ( {-AppendOutput => 1,
                                             -ConsumeInput => 1} ) ;
    cmp_ok $k->bzinflate($X . $F, $Z), '==', BZ_STREAM_END ;

    is $Z, $hello ;
}

exit if $] < 5.006 ;

title 'Looping Append test with substr output - substr the end of the string';
foreach (1 .. 2)
{

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2 (1) );
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->bzdeflate($_, substr($Answer, length($Answer))) ;
        last unless $status == BZ_RUN_OK ;

    }

    cmp_ok $status, '==', BZ_RUN_OK ;

    cmp_ok  $x->bzclose(substr($Answer, length($Answer))), '==', BZ_STREAM_END ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(1, 0) );
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, substr($GOT, length($GOT))) ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;
    }

    cmp_ok $status, '==', BZ_STREAM_END ;
    is $GOT, $hello ;

}

title 'Looping Append test with substr output - substr the complete string';
foreach (1 .. 2)
{

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Bzip2 (1) );
    ok $x ;
    cmp_ok $err, '==', BZ_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->bzdeflate($_, substr($Answer, 0)) ;
        last unless $status == BZ_RUN_OK ;

    }

    cmp_ok $status, '==', BZ_RUN_OK ;

    cmp_ok  $x->bzclose(substr($Answer, 0)), '==', BZ_STREAM_END ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Bunzip2(1, 0) );
    ok $k ;
    cmp_ok $err, '==', BZ_OK ;

    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->bzinflate($_, substr($GOT, 0)) ;
        last if $status == BZ_STREAM_END or $status != BZ_OK ;
    }

    cmp_ok $status, '==', BZ_STREAM_END ;
    is $GOT, $hello ;
}
