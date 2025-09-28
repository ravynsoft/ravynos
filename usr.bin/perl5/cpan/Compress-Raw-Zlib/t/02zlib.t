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

use Test::More  ;

use constant ZLIB_1_2_12_0 => 0x12C0;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };


    my $count = 0 ;
    if ($] < 5.005) {
        $count = 249 ;
    }
    elsif ($] >= 5.006) {
        $count = 353 ;
    }
    else {
        $count = 308 ;
    }

    plan tests => $count + $extra;

    use_ok('Compress::Raw::Zlib', 2) ;
}

use CompTestUtils;


my $Zlib_ver = Compress::Raw::Zlib::zlib_version ;

my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;

# Check zlib_version and ZLIB_VERSION are the same.
test_zlib_header_matches_library();

{
    title "Error Cases" ;

    eval { new Compress::Raw::Zlib::Deflate(-Level) };
    like $@,  mkErr("^Compress::Raw::Zlib::Deflate::new: Expected even number of parameters, got 1") ;

    eval { new Compress::Raw::Zlib::Inflate(-Level) };
    like $@, mkErr("^Compress::Raw::Zlib::Inflate::new: Expected even number of parameters, got 1");

    eval { new Compress::Raw::Zlib::Deflate(-Joe => 1) };
    like $@, mkErr('^Compress::Raw::Zlib::Deflate::new: unknown key value\(s\) Joe');

    eval { new Compress::Raw::Zlib::Inflate(-Joe => 1) };
    like $@, mkErr('^Compress::Raw::Zlib::Inflate::new: unknown key value\(s\) Joe');

    eval { new Compress::Raw::Zlib::Deflate(-Bufsize => 0) };
    like $@, mkErr("^Compress::Raw::Zlib::Deflate::new: Bufsize must be >= 1, you specified 0");

    eval { new Compress::Raw::Zlib::Inflate(-Bufsize => 0) };
    like $@, mkErr("^Compress::Raw::Zlib::Inflate::new: Bufsize must be >= 1, you specified 0");

    eval { new Compress::Raw::Zlib::Deflate(-Bufsize => -1) };
    like $@, mkErr("^Compress::Raw::Zlib::Deflate::new: Parameter 'Bufsize' must be an unsigned int, got '-1'");

    eval { new Compress::Raw::Zlib::Inflate(-Bufsize => -1) };
    like $@, mkErr("^Compress::Raw::Zlib::Inflate::new: Parameter 'Bufsize' must be an unsigned int, got '-1'");

    eval { new Compress::Raw::Zlib::Deflate(-Bufsize => "xxx") };
    like $@, mkErr("^Compress::Raw::Zlib::Deflate::new: Parameter 'Bufsize' must be an unsigned int, got 'xxx'");

    eval { new Compress::Raw::Zlib::Inflate(-Bufsize => "xxx") };
    like $@, mkErr("^Compress::Raw::Zlib::Inflate::new: Parameter 'Bufsize' must be an unsigned int, got 'xxx'");

    eval { new Compress::Raw::Zlib::Inflate(-Bufsize => 1, 2) };
    like $@, mkErr("^Compress::Raw::Zlib::Inflate::new: Expected even number of parameters, got 3");

    eval { new Compress::Raw::Zlib::Deflate(-Bufsize => 1, 2) };
    like $@, mkErr("^Compress::Raw::Zlib::Deflate::new: Expected even number of parameters, got 3");

}

{

    title  "deflate/inflate - small buffer";
    # ==============================

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( -Bufsize => 1 ), "Create deflate object" );
    ok $x, "Compress::Raw::Zlib::Deflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    ok ! defined $x->msg() ;
    is $x->total_in(), 0, "total_in() == 0" ;
    is $x->total_out(), 0, "total_out() == 0" ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->deflate($_, $X) ;
        last unless $status == Z_OK ;

        $Answer .= $X ;
    }

    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    cmp_ok  $x->flush($X), '==', Z_OK, "flush returned Z_OK" ;
    $Answer .= $X ;

    ok ! defined $x->msg()  ;
    is $x->total_in(), length $hello, "total_in ok" ;
    is $x->total_out(), length $Answer, "total_out ok" ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate( {-Bufsize => 1}) );
    ok $k, "Compress::Raw::Zlib::Inflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    ok ! defined $k->msg(), "No error messages" ;
    is $k->total_in(), 0, "total_in() == 0" ;
    is $k->total_out(), 0, "total_out() == 0" ;
    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->inflate($_, $Z) ;
        $GOT .= $Z ;
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
    ok ! defined $k->msg(), "No error messages" ;
    is $k->total_in(), length $Answer, "total_in ok" ;
    is $k->total_out(), length $hello , "total_out ok";

}


{
    # deflate/inflate - small buffer with a number
    # ==============================

    my $hello = 6529 ;

    ok  my ($x, $err) = new Compress::Raw::Zlib::Deflate ( -Bufsize => 1, -AppendOutput => 1 ) ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    my $status;
    my $Answer = '';

    cmp_ok $x->deflate($hello, $Answer), '==', Z_OK ;

    cmp_ok $x->flush($Answer), '==', Z_OK ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate( {-Bufsize => 1, -AppendOutput =>1}) );
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    #my $GOT = '';
    my $GOT ;
    foreach (@Answer)
    {
        $status = $k->inflate($_, $GOT) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $GOT, $hello ;

}

{

# deflate/inflate options - AppendOutput
# ================================

    # AppendOutput
    # CRC

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;

    ok  my ($x, $err) = new Compress::Raw::Zlib::Deflate ( {-Bufsize => 1, -AppendOutput =>1} ) ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    my $status;
    my $X;
    foreach (@hello)
    {
        $status = $x->deflate($_, $X) ;
        last unless $status == Z_OK ;
    }

    cmp_ok $status, '==', Z_OK ;

    cmp_ok $x->flush($X), '==', Z_OK ;


    my @Answer = split('', $X) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate( {-Bufsize => 1, -AppendOutput =>1}));
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $Z;
    foreach (@Answer)
    {
        $status = $k->inflate($_, $Z) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $Z, $hello ;
}


{

    title "deflate/inflate - larger buffer";
    # ==============================

    # generate a long random string
    my $contents = '' ;
    foreach (1 .. 50000)
      { $contents .= chr int rand 255 }


    ok my ($x, $err) = new Compress::Raw::Zlib::Deflate() ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    my (%X, $Y, %Z, $X, $Z);
    #cmp_ok $x->deflate($contents, $X{key}), '==', Z_OK ;
    cmp_ok $x->deflate($contents, $X), '==', Z_OK ;

    #$Y = $X{key} ;
    $Y = $X ;


    #cmp_ok $x->flush($X{key}), '==', Z_OK ;
    #$Y .= $X{key} ;
    cmp_ok $x->flush($X), '==', Z_OK ;
    $Y .= $X ;



    my $keep = $Y ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate() );
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    #cmp_ok $k->inflate($Y, $Z{key}), '==', Z_STREAM_END ;
    #ok $contents eq $Z{key} ;
    cmp_ok $k->inflate($Y, $Z), '==', Z_STREAM_END ;
    ok $contents eq $Z ;

    # redo deflate with AppendOutput

    ok (($k, $err) = new Compress::Raw::Zlib::Inflate(-AppendOutput => 1)) ;
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $s ;
    my $out ;
    my @bits = split('', $keep) ;
    foreach my $bit (@bits) {
        $s = $k->inflate($bit, $out) ;
    }

    cmp_ok $s, '==', Z_STREAM_END ;

    ok $contents eq $out ;


}

{

    title "deflate/inflate - preset dictionary";
    # ===================================

    my $dictionary = "hello" ;
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

title 'inflate - check remaining buffer after Z_STREAM_END';
#           and that ConsumeInput works.
# ===================================================

for my $consume ( 0 .. 1)
{
    ok my $x = new Compress::Raw::Zlib::Deflate(-Level => Z_BEST_COMPRESSION ) ;

    my ($X, $Y, $Z);
    cmp_ok $x->deflate($hello, $X), '==', Z_OK;
    cmp_ok $x->flush($Y), '==', Z_OK;
    $X .= $Y ;

    ok my $k = new Compress::Raw::Zlib::Inflate( -ConsumeInput => $consume) ;

    my $first = substr($X, 0, 2) ;
    my $remember_first = $first ;
    my $last  = substr($X, 2) ;
    cmp_ok $k->inflate($first, $Z), '==', Z_OK;
    if ($consume) {
        ok $first eq "" ;
    }
    else {
        ok $first eq $remember_first ;
    }

    my $T ;
    $last .= "appendage" ;
    my $remember_last = $last ;
    cmp_ok $k->inflate($last, $T),  '==', Z_STREAM_END;
    is $hello, $Z . $T  ;
    if ($consume) {
        is $last, "appendage" ;
    }
    else {
        is $last, $remember_last ;
    }

}



{

    title 'Check - MAX_WBITS';
    # =================

    my $hello = "Test test test test test";
    my @hello = split('', $hello) ;

    ok  my ($x, $err) =
       new Compress::Raw::Zlib::Deflate ( -Bufsize => 1,
                                     -WindowBits => -MAX_WBITS(),
                                     -AppendOutput => 1 ) ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    my $Answer = '';
    my $status;
    foreach (@hello)
    {
        $status = $x->deflate($_, $Answer) ;
        last unless $status == Z_OK ;
    }

    cmp_ok $status, '==', Z_OK ;

    cmp_ok $x->flush($Answer), '==', Z_OK ;

    my @Answer = split('', $Answer) ;
    # Undocumented corner -- extra byte needed to get inflate to return
    # Z_STREAM_END when done.
    push @Answer, " " ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate(
			{-Bufsize => 1,
			-AppendOutput =>1,
			-WindowBits => -MAX_WBITS()})) ;
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $GOT = '';
    foreach (@Answer)
    {
        $status = $k->inflate($_, $GOT) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $GOT, $hello ;

}

SKIP:
{
    title 'inflateSync';

    skip "inflateSync needs zlib 1.2.1 or better, you have $Zlib_ver", 22
        if ZLIB_VERNUM() < 0x1210 ;

    # create a deflate stream with flush points

    my $hello = "I am a HAL 9000 computer" x 2001 ;
    my $goodbye = "Will I dream?" x 2010;
    my ($x, $err, $answer, $X, $Z, $status);
    my $Answer ;

    #use Devel::Peek ;
    ok(($x, $err) = new Compress::Raw::Zlib::Deflate(AppendOutput => 1)) ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    cmp_ok $x->deflate($hello, $Answer), '==', Z_OK;

    # create a flush point
    cmp_ok $x->flush($Answer, Z_FULL_FLUSH), '==', Z_OK ;

    my $len1 = length $Answer;

    cmp_ok $x->deflate($goodbye, $Answer), '==', Z_OK;

    cmp_ok $x->flush($Answer), '==', Z_OK ;
    my $len2 = length($Answer) - $len1 ;

    my ($first, @Answer) = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate()) ;
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    cmp_ok  $k->inflate($first, $Z), '==', Z_OK;

    # skip to the first flush point.
    while (@Answer)
    {
        my $byte = shift @Answer;
        $status = $k->inflateSync($byte) ;
        last unless $status == Z_DATA_ERROR;
    }

    cmp_ok $status, '==', Z_OK;

    my $GOT = '';
    foreach (@Answer)
    {
        my $Z = '';
        $status = $k->inflate($_, $Z) ;
        $GOT .= $Z if defined $Z ;
        # print "x $status\n";
        last if $status == Z_STREAM_END or $status != Z_OK ;
    }

    # Z_STREAM_END returned by 1.12.2, Z_DATA_ERROR for older zlib
    # ZLIB_NG has the fix for all versions
    if (ZLIB_VERNUM >= ZLIB_1_2_12_0 ||  Compress::Raw::Zlib::is_zlibng)
    {
        cmp_ok $status, '==', Z_STREAM_END ;
    }
    else
    {
        cmp_ok $status, '==', Z_DATA_ERROR ;
    }

    is $GOT, $goodbye ;


    # Check inflateSync leaves good data in buffer
    my $rest = $Answer ;
    $rest =~ s/^(.)//;
    my $initial = $1 ;


    ok(($k, $err) = new Compress::Raw::Zlib::Inflate(ConsumeInput => 0)) ;
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    cmp_ok $k->inflate($initial, $Z), '==', Z_OK;

    # Skip to the flush point
    $status = $k->inflateSync($rest);
    cmp_ok $status, '==', Z_OK
     or diag "status '$status'\nlength rest is " . length($rest) . "\n" ;

    is length($rest), $len2, "expected compressed output";

    $GOT = '';
    $status = $k->inflate($rest, $GOT);
    # Z_STREAM_END returned by 1.12.2, Z_DATA_ERROR for older zlib
    if (ZLIB_VERNUM >= ZLIB_1_2_12_0 || Compress::Raw::Zlib::is_zlibng)
    {
        cmp_ok $status, '==', Z_STREAM_END ;
    }
    else
    {
        cmp_ok $status, '==', Z_DATA_ERROR ;
    }

    is $GOT, $goodbye ;
}

{
    title 'deflateParams';

    my $hello = "I am a HAL 9000 computer" x 2001 ;
    my $goodbye = "Will I dream?" x 2010;
    my ($x, $input, $err, $answer, $X, $status, $Answer);

    ok(($x, $err) = new Compress::Raw::Zlib::Deflate(
                       -AppendOutput   => 1,
                       -Level    => Z_DEFAULT_COMPRESSION,
                       -Strategy => Z_DEFAULT_STRATEGY)) ;
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    ok $x->get_Level()    == Z_DEFAULT_COMPRESSION;
    ok $x->get_Strategy() == Z_DEFAULT_STRATEGY;

    $status = $x->deflate($hello, $Answer) ;
    cmp_ok $status, '==', Z_OK ;
    $input .= $hello;

    # error cases
    eval { $x->deflateParams() };
    like $@, mkErr('^Compress::Raw::Zlib::deflateParams needs Level and\/or Strategy');

    eval { $x->deflateParams(-Bufsize => 0) };
    like $@, mkErr('^Compress::Raw::Zlib::Inflate::deflateParams: Bufsize must be >= 1, you specified 0');

    eval { $x->deflateParams(-Joe => 3) };
    like $@, mkErr('^Compress::Raw::Zlib::deflateStream::deflateParams: unknown key value\(s\) Joe');

    is $x->get_Level(),    Z_DEFAULT_COMPRESSION;
    is $x->get_Strategy(), Z_DEFAULT_STRATEGY;

    # change both Level & Strategy
    $status = $x->deflateParams(-Level => Z_BEST_SPEED, -Strategy => Z_HUFFMAN_ONLY, -Bufsize => 1234) ;
    cmp_ok $status, '==', Z_OK ;

    is $x->get_Level(),    Z_BEST_SPEED;
    is $x->get_Strategy(), Z_HUFFMAN_ONLY;

    # change both Level & Strategy again without any calls to deflate
    $status = $x->deflateParams(-Level => Z_DEFAULT_COMPRESSION, -Strategy => Z_DEFAULT_STRATEGY, -Bufsize => 1234) ;
    cmp_ok $status, '==', Z_OK ;

    is $x->get_Level(),    Z_DEFAULT_COMPRESSION;
    is $x->get_Strategy(), Z_DEFAULT_STRATEGY;

    $status = $x->deflate($goodbye, $Answer) ;
    cmp_ok $status, '==', Z_OK ;
    $input .= $goodbye;

    # change only Level
    $status = $x->deflateParams(-Level => Z_NO_COMPRESSION) ;
    cmp_ok $status, '==', Z_OK ;

    is $x->get_Level(),    Z_NO_COMPRESSION;
    is $x->get_Strategy(), Z_DEFAULT_STRATEGY;

    $status = $x->deflate($goodbye, $Answer) ;
    cmp_ok $status, '==', Z_OK ;
    $input .= $goodbye;

    # change only Strategy
    $status = $x->deflateParams(-Strategy => Z_FILTERED) ;
    cmp_ok $status, '==', Z_OK ;

    is $x->get_Level(),    Z_NO_COMPRESSION;
    is $x->get_Strategy(), Z_FILTERED;

    $status = $x->deflate($goodbye, $Answer) ;
    cmp_ok $status, '==', Z_OK ;
    $input .= $goodbye;

    cmp_ok $x->flush($Answer), '==', Z_OK ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate()) ;
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $Z;
    $status = $k->inflate($Answer, $Z) ;

    cmp_ok $status, '==', Z_STREAM_END ;
    is $Z, $input ;
}


{
    title "ConsumeInput and a read-only buffer trapped" ;

    ok my $k = new Compress::Raw::Zlib::Inflate(-ConsumeInput => 1) ;

    my $Z;
    eval { $k->inflate("abc", $Z) ; };
    like $@, mkErr("Compress::Raw::Zlib::Inflate::inflate input parameter cannot be read-only when ConsumeInput is specified");

}

foreach (1 .. 2)
{
    next if $] < 5.005 ;

    title 'test inflate/deflate with a substr';

    my $contents = '' ;
    foreach (1 .. 5000)
      { $contents .= chr int rand 255 }
    ok  my $x = new Compress::Raw::Zlib::Deflate(-AppendOutput => 1) ;

    my $X ;
    my $status = $x->deflate(substr($contents,0), $X);
    cmp_ok $status, '==', Z_OK ;

    cmp_ok $x->flush($X), '==', Z_OK  ;

    my $append = "Appended" ;
    $X .= $append ;

    ok my $k = new Compress::Raw::Zlib::Inflate(-AppendOutput => 1) ;

    my $Z;
    my $keep = $X ;
    $status = $k->inflate(substr($X, 0), $Z) ;

    cmp_ok $status, '==', Z_STREAM_END ;
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

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( -Bufsize => 1 ) );
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->deflate($_, $X) ;
        last unless $status == Z_OK ;

        $Answer .= $X ;
    }

    cmp_ok $status, '==', Z_OK ;

    cmp_ok  $x->flush($X), '==', Z_OK ;
    $Answer .= $X ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate(-AppendOutput => 1) );
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $GOT ;
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->inflate($_, $GOT) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;
    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $GOT, $hello ;

}

if ($] >= 5.005)
{
    title 'test inflate input parameter via substr';

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    ok my $x = new Compress::Raw::Zlib::Deflate ( -AppendOutput => 1 );

    cmp_ok $x->deflate($data, $X), '==',  Z_OK ;

    cmp_ok $x->flush($X), '==', Z_OK ;

    my $append = "Appended" ;
    $X .= $append ;
    my $keep = $X ;

    ok my $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1,
                                             -ConsumeInput => 1 ) ;

    cmp_ok $k->inflate(substr($X, 0, -1), $Z), '==', Z_STREAM_END ; ;

    ok $hello eq $Z ;
    is $X, $append;

    $X = $keep ;
    $Z = '';
    ok $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1,
                                          -ConsumeInput => 0 ) ;

    cmp_ok $k->inflate(substr($X, 0, -1), $Z), '==', Z_STREAM_END ; ;
    #cmp_ok $k->inflate(substr($X, 0), $Z), '==', Z_STREAM_END ; ;

    ok $hello eq $Z ;
    is $X, $keep;

}

{
    title 'RT#132734: test inflate append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Zlib/issues/3

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    ok my $x = new Compress::Raw::Zlib::Deflate ( -AppendOutput => 1 );

    cmp_ok $x->deflate($data, $X), '==',  Z_OK ;

    cmp_ok $x->flush($X), '==', Z_OK ;

    ok my $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1,
                                             -ConsumeInput => 1 ) ;
    $Z = 'prev. ' ;
    substr($Z, 0, 4, ''); # chop off first 4 characters using offset
    cmp_ok $Z, 'eq', '. ' ;

    # use Devel::Peek ; Dump($Z) ; # shows OOK flag

    # if (1) { # workaround
    #     my $prev = $Z;
    #     undef $Z ;
    #     $Z = $prev ;
    # }

    cmp_ok $k->inflate($X, $Z), '==', Z_STREAM_END ;
    # use Devel::Peek ; Dump($Z) ; # No OOK flag

    cmp_ok $Z, 'eq', ". $hello" ;
}


{
    title 'RT#132734: test deflate append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Zlib/issues/3

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

    ok my $x = new Compress::Raw::Zlib::Deflate ( -AppendOutput => 1 );

    cmp_ok $x->deflate($data, $X), '==',  Z_OK ;

    cmp_ok $x->flush($X), '==', Z_OK ;

    ok my $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1,
                                             -ConsumeInput => 1 ) ;
    cmp_ok $k->inflate($X, $Z), '==', Z_STREAM_END ;

    is $Z, $hello ;
}


{
    title 'RT#132734: test flush append OOK output parameter';
    # https://github.com/pmqs/Compress-Raw-Zlib/issues/3

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);

    my $F = 'prev. ' ;
    substr($F, 0, 6, ''); # chop off all characters using offset
    cmp_ok $F, 'eq', '' ;

    # use Devel::Peek ; Dump($F) ; # shows OOK flag

    ok my $x = new Compress::Raw::Zlib::Deflate ( -AppendOutput => 1 );

    cmp_ok $x->deflate($data, $X), '==',  Z_OK ;

    cmp_ok $x->flush($F), '==', Z_OK ;

    ok my $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1,
                                             -ConsumeInput => 1 ) ;
    cmp_ok $k->inflate($X . $F, $Z), '==', Z_STREAM_END ;

    is $Z, $hello ;
}

SKIP:
{
    skip "InflateScan needs zlib 1.2.1 or better, you have $Zlib_ver", 1
        if ZLIB_VERNUM() < 0x1210 ;

    # regression - check that resetLastBlockByte can cope with a NULL
    # pointer.
    Compress::Raw::Zlib::InflateScan->new->resetLastBlockByte(undef);
    ok 1, "resetLastBlockByte(undef) is ok" ;
}

SKIP:
{

    title "gzip mode";
    # ================

    skip "gzip mode needs zlib 1.2.1 or better, you have $Zlib_ver", 13
        if ZLIB_VERNUM() < 0x1210 ;

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate (
            WindowBits => WANT_GZIP ,
            AppendOutput => 1
        ), "Create deflate object" );
    ok $x, "Compress::Raw::Zlib::Deflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $x->deflate($hello, $X) ;
    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    cmp_ok  $x->flush($X), '==', Z_OK, "flush returned Z_OK" ;

    my ($k, $GOT);
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => WANT_GZIP ,
            ConsumeInput => 0 ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;

    $GOT = '';
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => WANT_GZIP_OR_ZLIB ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP_OR_ZLIB ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
}

SKIP:
{

    title "gzip error mode";
    # Create gzip -
    # read with no special windowbits setting - this will fail
    # then read with WANT_GZIP_OR_ZLIB - thi swill work
    # ================

    skip "gzip mode needs zlib 1.2.1 or better, you have $Zlib_ver", 12
        if ZLIB_VERNUM() < 0x1210 ;

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate (
            WindowBits => WANT_GZIP ,
            AppendOutput => 1
        ), "Create deflate object" );
    ok $x, "Compress::Raw::Zlib::Deflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $x->deflate($hello, $X) ;
    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    cmp_ok  $x->flush($X), '==', Z_OK, "flush returned Z_OK" ;

    my ($k, $GOT);
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => MAX_WBITS ,
            ConsumeInput => 0 ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_DATA_ERROR, "Got Z_DATA_ERROR" ;

    $GOT = '';
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => WANT_GZIP_OR_ZLIB ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP_OR_ZLIB ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
}

SKIP:
{
    title "gzip/zlib error mode";
    # Create zlib -
    # read with no WANT_GZIP windowbits setting - this will fail
    # then read with WANT_GZIP_OR_ZLIB - thi swill work
    # ================

    skip "gzip mode needs zlib 1.2.1 or better, you have $Zlib_ver", 12
        if ZLIB_VERNUM() < 0x1210 ;

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate (
            AppendOutput => 1
        ), "Create deflate object" );
    ok $x, "Compress::Raw::Zlib::Deflate ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $x->deflate($hello, $X) ;
    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    cmp_ok  $x->flush($X), '==', Z_OK, "flush returned Z_OK" ;

    my ($k, $GOT);
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => WANT_GZIP ,
            ConsumeInput => 0 ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_DATA_ERROR, "Got Z_DATA_ERROR" ;

    $GOT = '';
    ($k, $err) = new Compress::Raw::Zlib::Inflate(
            WindowBits => WANT_GZIP_OR_ZLIB ,
            AppendOutput => 1);
    ok $k, "Compress::Raw::Zlib::Inflate WANT_GZIP_OR_ZLIB ok" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $k->inflate($X, $GOT) ;
    cmp_ok $status, '==', Z_STREAM_END, "Got Z_STREAM_END" ;
    is $GOT, $hello, "uncompressed data matches ok" ;
}

{
    title "zlibCompileFlags";

    my $flags = Compress::Raw::Zlib::zlibCompileFlags;

    if (!Compress::Raw::Zlib::is_zlibng && ZLIB_VERNUM() < 0x1210)
    {
        is $flags, 0, "zlibCompileFlags == 0 if < 1.2.1";
    }
    else
    {
        ok $flags, "zlibCompileFlags != 0 if < 1.2.1";
    }
}

{
    title "repeated calls to flush after some compression";

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( ), "Create deflate object" );
    isa_ok $x, "Compress::Raw::Zlib::deflateStream" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    $status = $x->deflate($hello, $X) ;
    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "flush returned Z_OK" ;
    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "second flush returned Z_OK" ;
    is $X, "", "no output from second flush";
}

{
    title "repeated calls to flush - no compression";

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( ), "Create deflate object" );
    isa_ok $x, "Compress::Raw::Zlib::deflateStream" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "flush returned Z_OK" ;
    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "second flush returned Z_OK" ;
    is $X, "", "no output from second flush";
}

{
    title "crc32";

    is eval('Compress::Raw::Zlib::crc32("A" x 0x100, 0, 0x100); 0x1234'), 0x1234;
    is $@,  '';

    is eval('Compress::Raw::Zlib::crc32("A" x 0x100, 0, 0x101); 0x1234'), undef;
    like $@,  mkErr("^Offset out of range in Compress::Raw::Zlib::crc32") ;

}

SKIP:
{
    title "crc32_combine";

   skip "crc32_combine needs zlib 1.2.3 or better, you have $Zlib_ver", 1
        if ZLIB_VERNUM() < 0x1230 ;

    my $first = "1234";
    my $second = "5678";

    my $crc1 = Compress::Raw::Zlib::crc32($first);
    my $crc2 = Compress::Raw::Zlib::crc32($second);

    my $composite_crc = Compress::Raw::Zlib::crc32($first . $second);

    my $combined_crc = Compress::Raw::Zlib::crc32_combine($crc1, $crc2, length $second);

    is $combined_crc, $composite_crc ;
}

SKIP:
{
    title "adler32_combine";

   skip "adler32_combine needs zlib 1.2.3 or better, you have $Zlib_ver", 1
        if ZLIB_VERNUM() < 0x1230 ;

    my $first = "1234";
    my $second = "5678";

    my $adler1 = Compress::Raw::Zlib::adler32($first);
    my $adler2 = Compress::Raw::Zlib::adler32($second);

    my $composite_adler = Compress::Raw::Zlib::adler32($first . $second);

    my $combined_adler = Compress::Raw::Zlib::adler32_combine($adler1, $adler2, length $second);

    is $combined_adler, $composite_adler ;
}

if (0)
{
    title "RT #122695: sync flush appending extra empty uncompressed block";

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( ), "Create deflate object" );
    isa_ok $x, "Compress::Raw::Zlib::deflateStream" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "flush returned Z_OK" ;
    cmp_ok  $x->flush($X, Z_SYNC_FLUSH), '==', Z_OK, "second flush returned Z_OK" ;
    is $X, "", "no output from second flush";
}

exit if $] < 5.006 ;

title 'Looping Append test with substr output - substr the end of the string';
foreach (1 .. 2)
{

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( -Bufsize => 1,
                                            -AppendOutput => 1 ) );
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->deflate($_, substr($Answer, length($Answer))) ;
        last unless $status == Z_OK ;

    }

    cmp_ok $status, '==', Z_OK ;

    cmp_ok  $x->flush(substr($Answer, length($Answer))), '==', Z_OK ;

    #cmp_ok length $Answer, ">", 0 ;

    my @Answer = split('', $Answer) ;


    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate(-AppendOutput => 1) );
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->inflate($_, substr($GOT, length($GOT))) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;
    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $GOT, $hello ;

}

title 'Looping Append test with substr output - substr the complete string';
foreach (1 .. 2)
{

    my $hello = "I am a HAL 9000 computer" ;
    my @hello = split('', $hello) ;
    my ($err, $x, $X, $status);

    ok( ($x, $err) = new Compress::Raw::Zlib::Deflate ( -Bufsize => 1,
                                            -AppendOutput => 1 ) );
    ok $x ;
    cmp_ok $err, '==', Z_OK ;

    $X = "" ;
    my $Answer = '';
    foreach (@hello)
    {
        $status = $x->deflate($_, substr($Answer, 0)) ;
        last unless $status == Z_OK ;

    }

    cmp_ok $status, '==', Z_OK ;

    cmp_ok  $x->flush(substr($Answer, 0)), '==', Z_OK ;

    my @Answer = split('', $Answer) ;

    my $k;
    ok(($k, $err) = new Compress::Raw::Zlib::Inflate(-AppendOutput => 1) );
    ok $k ;
    cmp_ok $err, '==', Z_OK ;

    my $GOT = '';
    my $Z;
    $Z = 1 ;#x 2000 ;
    foreach (@Answer)
    {
        $status = $k->inflate($_, substr($GOT, 0)) ;
        last if $status == Z_STREAM_END or $status != Z_OK ;
    }

    cmp_ok $status, '==', Z_STREAM_END ;
    is $GOT, $hello ;
}
