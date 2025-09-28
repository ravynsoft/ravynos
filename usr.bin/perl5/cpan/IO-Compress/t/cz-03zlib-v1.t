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
use Symbol;

use constant ZLIB_1_2_12_0 => 0x12C0;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    my $count = 0 ;
    if ($] < 5.005) {
        $count = 453 ;
    }
    else {
        $count = 471 ;
    }


    plan tests => $count + $extra ;

    use_ok('Compress::Zlib', qw(:ALL memGunzip memGzip zlib_version));
    use_ok('IO::Compress::Gzip::Constants') ;

    use_ok('IO::Compress::Gzip', qw($GzipError)) ;
}


my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;

# Check zlib_version and ZLIB_VERSION are the same.
SKIP: {
    skip "TEST_SKIP_VERSION_CHECK is set", 1
        if $ENV{TEST_SKIP_VERSION_CHECK};
    is Compress::Zlib::zlib_version, ZLIB_VERSION,
        "ZLIB_VERSION matches Compress::Zlib::zlib_version" ;
}

# generate a long random string
my $contents = '' ;
foreach (1 .. 5000)
  { $contents .= chr int rand 256 }

my $x ;
my $fil;

# compress/uncompress tests
# =========================

eval { compress([1]); };
ok $@ =~ m#not a scalar reference#
    or print "# $@\n" ;;

eval { uncompress([1]); };
ok $@ =~ m#not a scalar reference#
    or print "# $@\n" ;;

$hello = "hello mum" ;
my $keep_hello = $hello ;

my $compr = compress($hello) ;
ok $compr ne "" ;

my $keep_compr = $compr ;

my $uncompr = uncompress ($compr) ;

ok $hello eq $uncompr ;

ok $hello eq $keep_hello ;
ok $compr eq $keep_compr ;

# compress a number
$hello = 7890 ;
$keep_hello = $hello ;

$compr = compress($hello) ;
ok $compr ne "" ;

$keep_compr = $compr ;

$uncompr = uncompress ($compr) ;

ok $hello eq $uncompr ;

ok $hello eq $keep_hello ;
ok $compr eq $keep_compr ;

# bigger compress

$compr = compress ($contents) ;
ok $compr ne "" ;

$uncompr = uncompress ($compr) ;

ok $contents eq $uncompr ;

# buffer reference

$compr = compress(\$hello) ;
ok $compr ne "" ;


$uncompr = uncompress (\$compr) ;
ok $hello eq $uncompr ;

# bad level
$compr = compress($hello, 1000) ;
ok ! defined $compr;

# change level
$compr = compress($hello, Z_BEST_COMPRESSION) ;
ok defined $compr;
$uncompr = uncompress (\$compr) ;
ok $hello eq $uncompr ;

# corrupt data
$compr = compress(\$hello) ;
ok $compr ne "" ;

substr($compr,0, 1) = "\xFF";
ok !defined uncompress (\$compr) ;

# deflate/inflate - small buffer
# ==============================

$hello = "I am a HAL 9000 computer" ;
my @hello = split('', $hello) ;
my ($err, $X, $status);

ok  (($x, $err) = deflateInit( {-Bufsize => 1} ) ) ;
ok $x ;
ok $err == Z_OK ;

my $Answer = '';
foreach (@hello)
{
    ($X, $status) = $x->deflate($_) ;
    last unless $status == Z_OK ;

    $Answer .= $X ;
}

ok $status == Z_OK ;

ok    ((($X, $status) = $x->flush())[1] == Z_OK ) ;
$Answer .= $X ;


my @Answer = split('', $Answer) ;

my $k;
ok (($k, $err) = inflateInit( {-Bufsize => 1}) ) ;
ok $k ;
ok $err == Z_OK ;

my $GOT = '';
my $Z;
foreach (@Answer)
{
    ($Z, $status) = $k->inflate($_) ;
    $GOT .= $Z ;
    last if $status == Z_STREAM_END or $status != Z_OK ;

}

ok $status == Z_STREAM_END ;
ok $GOT eq $hello ;


title 'deflate/inflate - small buffer with a number';
# ==============================

$hello = 6529 ;

ok (($x, $err) = deflateInit( {-Bufsize => 1} ) ) ;
ok $x ;
ok $err == Z_OK ;

ok !defined $x->msg() ;
ok $x->total_in() == 0 ;
ok $x->total_out() == 0 ;
$Answer = '';
{
    ($X, $status) = $x->deflate($hello) ;

    $Answer .= $X ;
}

ok $status == Z_OK ;

ok   ((($X, $status) = $x->flush())[1] == Z_OK ) ;
$Answer .= $X ;

ok !defined $x->msg() ;
ok $x->total_in() == length $hello ;
ok $x->total_out() == length $Answer ;


@Answer = split('', $Answer) ;

ok (($k, $err) = inflateInit( {-Bufsize => 1}) ) ;
ok $k ;
ok $err == Z_OK ;

ok !defined $k->msg() ;
ok $k->total_in() == 0 ;
ok $k->total_out() == 0 ;

$GOT = '';
foreach (@Answer)
{
    ($Z, $status) = $k->inflate($_) ;
    $GOT .= $Z ;
    last if $status == Z_STREAM_END or $status != Z_OK ;

}

ok $status == Z_STREAM_END ;
ok $GOT eq $hello ;

ok !defined $k->msg() ;
is $k->total_in(), length $Answer ;
ok $k->total_out() == length $hello ;



title 'deflate/inflate - larger buffer';
# ==============================


ok $x = deflateInit() ;

ok ((($X, $status) = $x->deflate($contents))[1] == Z_OK) ;

my $Y = $X ;


ok ((($X, $status) = $x->flush() )[1] == Z_OK ) ;
$Y .= $X ;



ok $k = inflateInit() ;

($Z, $status) = $k->inflate($Y) ;

ok $status == Z_STREAM_END ;
ok $contents eq $Z ;

title 'deflate/inflate - preset dictionary';
# ===================================

my $dictionary = "hello" ;
ok $x = deflateInit({-Level => Z_BEST_COMPRESSION,
			 -Dictionary => $dictionary}) ;

my $dictID = $x->dict_adler() ;

($X, $status) = $x->deflate($hello) ;
ok $status == Z_OK ;
($Y, $status) = $x->flush() ;
ok $status == Z_OK ;
$X .= $Y ;
$x = 0 ;

ok $k = inflateInit(-Dictionary => $dictionary) ;

($Z, $status) = $k->inflate($X);
ok $status == Z_STREAM_END ;
ok $k->dict_adler() == $dictID;
ok $hello eq $Z ;

#$Z='';
#while (1) {
#    ($Z, $status) = $k->inflate($X) ;
#    last if $status == Z_STREAM_END or $status != Z_OK ;
#print "status=[$status] hello=[$hello] Z=[$Z]\n";
#}
#ok $status == Z_STREAM_END ;
#ok $hello eq $Z
# or print "status=[$status] hello=[$hello] Z=[$Z]\n";






title 'inflate - check remaining buffer after Z_STREAM_END';
# ===================================================

{
    ok $x = deflateInit(-Level => Z_BEST_COMPRESSION ) ;

    ($X, $status) = $x->deflate($hello) ;
    ok $status == Z_OK ;
    ($Y, $status) = $x->flush() ;
    ok $status == Z_OK ;
    $X .= $Y ;
    $x = 0 ;

    ok $k = inflateInit()  ;

    my $first = substr($X, 0, 2) ;
    my $last  = substr($X, 2) ;
    ($Z, $status) = $k->inflate($first);
    ok $status == Z_OK ;
    ok $first eq "" ;

    $last .= "appendage" ;
    my $T;
    ($T, $status) = $k->inflate($last);
    ok $status == Z_STREAM_END ;
    ok $hello eq $Z . $T ;
    ok $last eq "appendage" ;

}

title 'memGzip & memGunzip';
{
    my ($name, $name1, $name2, $name3);
    my $lex = LexFile->new( $name, $name1, $name2, $name3 );
    my $buffer = <<EOM;
some sample
text

EOM

    my $len = length $buffer ;
    my ($x, $uncomp) ;


    # create an in-memory gzip file
    my $dest = memGzip($buffer) ;
    ok length $dest ;
    is $gzerrno, 0;

    # write it to disk
    ok open(FH, ">$name") ;
    binmode(FH);
    print FH $dest ;
    close FH ;

    # uncompress with gzopen
    ok my $fil = gzopen($name, "rb") ;

    is $fil->gzread($uncomp, 0), 0 ;
    ok (($x = $fil->gzread($uncomp)) == $len) ;

    ok ! $fil->gzclose ;

    ok $uncomp eq $buffer ;

    #1 while unlink $name ;

    # now check that memGunzip can deal with it.
    my $ungzip = memGunzip($dest) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    # now do the same but use a reference

    $dest = memGzip(\$buffer) ;
    ok length $dest ;
    is $gzerrno, 0;

    # write it to disk
    ok open(FH, ">$name1") ;
    binmode(FH);
    print FH $dest ;
    close FH ;

    # uncompress with gzopen
    ok $fil = gzopen($name1, "rb") ;

    ok (($x = $fil->gzread($uncomp)) == $len) ;

    ok ! $fil->gzclose ;

    ok $uncomp eq $buffer ;

    # now check that memGunzip can deal with it.
    my $keep = $dest;
    $ungzip = memGunzip(\$dest) ;
    is $gzerrno, 0;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;

    # check memGunzip can cope with missing gzip trailer
    my $minimal = substr($keep, 0, -1) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -2) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -3) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -4) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -5) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -6) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -7) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -8) ;
    $ungzip = memGunzip(\$minimal) ;
    ok defined $ungzip ;
    ok $buffer eq $ungzip ;
    is $gzerrno, 0;

    $minimal = substr($keep, 0, -9) ;
    $ungzip = memGunzip(\$minimal) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;


    #1 while unlink $name ;

    # check corrupt header -- too short
    $dest = "x" ;
    my $result = memGunzip($dest) ;
    ok !defined $result ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # check corrupt header -- full of junk
    $dest = "x" x 200 ;
    $result = memGunzip($dest) ;
    ok !defined $result ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt header - 1st byte wrong
    my $bad = $keep ;
    substr($bad, 0, 1) = "\xFF" ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt header - 2st byte wrong
    $bad = $keep ;
    substr($bad, 1, 1) = "\xFF" ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt header - method not deflated
    $bad = $keep ;
    substr($bad, 2, 1) = "\xFF" ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt header - reserved bits used
    $bad = $keep ;
    substr($bad, 3, 1) = "\xFF" ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt trailer - length wrong
    $bad = $keep ;
    substr($bad, -8, 4) = "\xFF" x 4 ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    # corrupt trailer - CRC wrong
    $bad = $keep ;
    substr($bad, -4, 4) = "\xFF" x 4 ;
    $ungzip = memGunzip(\$bad) ;
    ok ! defined $ungzip ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
}

{
    title "Check all bytes can be handled";

    my $lex = LexFile->new( my $name );
    my $data = join '', map { chr } 0x00 .. 0xFF;
    $data .= "\r\nabd\r\n";

    my $fil;
    ok $fil = gzopen($name, "wb") ;
    is $fil->gzwrite($data), length $data ;
    ok ! $fil->gzclose();

    my $input;
    ok $fil = gzopen($name, "rb") ;
    is $fil->gzread($input), length $data ;
    ok ! $fil->gzclose();
    ok $input eq $data;

    title "Check all bytes can be handled - transparent mode";
    writeFile($name, $data);
    ok $fil = gzopen($name, "rb") ;
    is $fil->gzread($input), length $data ;
    ok ! $fil->gzclose();
    ok $input eq $data;

}

title 'memGunzip with a gzopen created file';
{
    my $name = "test.gz" ;
    my $buffer = <<EOM;
some sample
text

EOM

    ok $fil = gzopen($name, "wb") ;

    ok $fil->gzwrite($buffer) == length $buffer ;

    ok ! $fil->gzclose ;

    my $compr = readFile($name);
    ok length $compr ;
    my $unc = memGunzip($compr) ;
    is $gzerrno, 0;
    ok defined $unc ;
    ok $buffer eq $unc ;
    1 while unlink $name ;
}

{

    # Check - MAX_WBITS
    # =================

    $hello = "Test test test test test";
    @hello = split('', $hello) ;

    ok (($x, $err) = deflateInit( -Bufsize => 1, -WindowBits => -MAX_WBITS() ) ) ;
    ok $x ;
    ok $err == Z_OK ;

    $Answer = '';
    foreach (@hello)
    {
        ($X, $status) = $x->deflate($_) ;
        last unless $status == Z_OK ;

        $Answer .= $X ;
    }

    ok $status == Z_OK ;

    ok   ((($X, $status) = $x->flush())[1] == Z_OK ) ;
    $Answer .= $X ;


    @Answer = split('', $Answer) ;
    # Undocumented corner -- extra byte needed to get inflate to return
    # Z_STREAM_END when done.
    push @Answer, " " ;

    ok (($k, $err) = inflateInit(-Bufsize => 1, -WindowBits => -MAX_WBITS()) ) ;
    ok $k ;
    ok $err == Z_OK ;

    $GOT = '';
    foreach (@Answer)
    {
        ($Z, $status) = $k->inflate($_) ;
        $GOT .= $Z ;
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    ok $status == Z_STREAM_END ;
    ok $GOT eq $hello ;

}

{
    # inflateSync

    # create a deflate stream with flush points

    my $hello = "I am a HAL 9000 computer" x 2001 ;
    my $goodbye = "Will I dream?" x 2010;
    my ($err, $answer, $X, $status, $Answer);

    ok (($x, $err) = deflateInit() ) ;
    ok $x ;
    ok $err == Z_OK ;

    ($Answer, $status) = $x->deflate($hello) ;
    ok $status == Z_OK ;

    # create a flush point
    ok ((($X, $status) = $x->flush(Z_FULL_FLUSH))[1] == Z_OK ) ;
    $Answer .= $X ;

    ($X, $status) = $x->deflate($goodbye) ;
    ok $status == Z_OK ;
    $Answer .= $X ;

    ok ((($X, $status) = $x->flush())[1] == Z_OK ) ;
    $Answer .= $X ;

    my ($first, @Answer) = split('', $Answer) ;

    my $k;
    ok (($k, $err) = inflateInit()) ;
    ok $k ;
    ok $err == Z_OK ;

    ($Z, $status) = $k->inflate($first) ;
    ok $status == Z_OK ;

    # skip to the first flush point.
    while (@Answer)
    {
        my $byte = shift @Answer;
        $status = $k->inflateSync($byte) ;
        last unless $status == Z_DATA_ERROR;

    }

    ok $status == Z_OK;

    my $GOT = '';
    my $Z = '';
    foreach (@Answer)
    {
        my $Z = '';
        ($Z, $status) = $k->inflate($_) ;
        $GOT .= $Z if defined $Z ;
        # print "x $status\n";
        last if $status == Z_STREAM_END or $status != Z_OK ;

    }

    # zlib 1.0.9 returns Z_STREAM_END here, all others return Z_DATA_ERROR
    ok $status == Z_DATA_ERROR || $status == Z_STREAM_END ;
    ok $GOT eq $goodbye ;


    # Check inflateSync leaves good data in buffer
    $Answer =~ /^(.)(.*)$/ ;
    my ($initial, $rest) = ($1, $2);


    ok (($k, $err) = inflateInit()) ;
    ok $k ;
    ok $err == Z_OK ;

    ($Z, $status) = $k->inflate($initial) ;
    ok $status == Z_OK ;

    $status = $k->inflateSync($rest) ;
    ok $status == Z_OK;

    ($GOT, $status) = $k->inflate($rest) ;

    # Z_STREAM_END returned by 1.12.2, Z_DATA_ERROR for older zlib
    # always Z_STREAM_ENDin zlib_ng
    if (ZLIB_VERNUM >= ZLIB_1_2_12_0 || Compress::Raw::Zlib::is_zlibng)
    {
        cmp_ok $status, '==', Z_STREAM_END ;
    }
    else
    {
        cmp_ok $status, '==', Z_DATA_ERROR ;
    }

    ok $Z . $GOT eq $goodbye ;
}

{
    # deflateParams

    my $hello = "I am a HAL 9000 computer" x 2001 ;
    my $goodbye = "Will I dream?" x 2010;
    my ($input, $err, $answer, $X, $status, $Answer);

    ok (($x, $err) = deflateInit(-Level    => Z_BEST_COMPRESSION,
                                     -Strategy => Z_DEFAULT_STRATEGY) ) ;
    ok $x ;
    ok $err == Z_OK ;

    ok $x->get_Level()    == Z_BEST_COMPRESSION;
    ok $x->get_Strategy() == Z_DEFAULT_STRATEGY;

    ($Answer, $status) = $x->deflate($hello) ;
    ok $status == Z_OK ;
    $input .= $hello;

    # error cases
    eval { $x->deflateParams() };
    #like $@, mkErr("^Compress::Raw::Zlib::deflateParams needs Level and/or Strategy");
    like $@, "/^Compress::Raw::Zlib::deflateParams needs Level and/or Strategy/";

    eval { $x->deflateParams(-Joe => 3) };
    like $@, "/^Compress::Raw::Zlib::deflateStream::deflateParams: unknown key value/";
    #like $@, mkErr("^Compress::Raw::Zlib::deflateStream::deflateParams: unknown key value(s) Joe");
    #ok $@ =~ /^Compress::Zlib::deflateStream::deflateParams: unknown key value\(s\) Joe at/
    #    or print "# $@\n" ;

    ok $x->get_Level()    == Z_BEST_COMPRESSION;
    ok $x->get_Strategy() == Z_DEFAULT_STRATEGY;

    # change both Level & Strategy
    $status = $x->deflateParams(-Level => Z_BEST_SPEED, -Strategy => Z_HUFFMAN_ONLY) ;
    ok $status == Z_OK ;

    ok $x->get_Level()    == Z_BEST_SPEED;
    ok $x->get_Strategy() == Z_HUFFMAN_ONLY;

    ($X, $status) = $x->deflate($goodbye) ;
    ok $status == Z_OK ;
    $Answer .= $X ;
    $input .= $goodbye;

    # change only Level
    $status = $x->deflateParams(-Level => Z_NO_COMPRESSION) ;
    ok $status == Z_OK ;

    ok $x->get_Level()    == Z_NO_COMPRESSION;
    ok $x->get_Strategy() == Z_HUFFMAN_ONLY;

    ($X, $status) = $x->deflate($goodbye) ;
    ok $status == Z_OK ;
    $Answer .= $X ;
    $input .= $goodbye;

    # change only Strategy
    $status = $x->deflateParams(-Strategy => Z_FILTERED) ;
    ok $status == Z_OK ;

    ok $x->get_Level()    == Z_NO_COMPRESSION;
    ok $x->get_Strategy() == Z_FILTERED;

    ($X, $status) = $x->deflate($goodbye) ;
    ok $status == Z_OK ;
    $Answer .= $X ;
    $input .= $goodbye;

    ok ((($X, $status) = $x->flush())[1] == Z_OK ) ;
    $Answer .= $X ;

    my ($first, @Answer) = split('', $Answer) ;

    my $k;
    ok (($k, $err) = inflateInit()) ;
    ok $k ;
    ok $err == Z_OK ;

    ($Z, $status) = $k->inflate($Answer) ;

    ok $status == Z_STREAM_END
        or print "# status $status\n";
    ok $Z  eq $input ;
}

{
    # error cases

    eval { deflateInit(-Level) };
    like $@, '/^Compress::Zlib::deflateInit: Expected even number of parameters, got 1/';

    eval { inflateInit(-Level) };
    like $@, '/^Compress::Zlib::inflateInit: Expected even number of parameters, got 1/';

    eval { deflateInit(-Joe => 1) };
    ok $@ =~ /^Compress::Zlib::deflateInit: unknown key value\(s\) Joe at/;

    eval { inflateInit(-Joe => 1) };
    ok $@ =~ /^Compress::Zlib::inflateInit: unknown key value\(s\) Joe at/;

    eval { deflateInit(-Bufsize => 0) };
    ok $@ =~ /^.*?: Bufsize must be >= 1, you specified 0 at/;

    eval { inflateInit(-Bufsize => 0) };
    ok $@ =~ /^.*?: Bufsize must be >= 1, you specified 0 at/;

    eval { deflateInit(-Bufsize => -1) };
    #ok $@ =~ /^.*?: Bufsize must be >= 1, you specified -1 at/;
    ok $@ =~ /^Compress::Zlib::deflateInit: Parameter 'Bufsize' must be an unsigned int, got '-1'/;

    eval { inflateInit(-Bufsize => -1) };
    ok $@ =~ /^Compress::Zlib::inflateInit: Parameter 'Bufsize' must be an unsigned int, got '-1'/;

    eval { deflateInit(-Bufsize => "xxx") };
    ok $@ =~ /^Compress::Zlib::deflateInit: Parameter 'Bufsize' must be an unsigned int, got 'xxx'/;

    eval { inflateInit(-Bufsize => "xxx") };
    ok $@ =~ /^Compress::Zlib::inflateInit: Parameter 'Bufsize' must be an unsigned int, got 'xxx'/;

    eval { gzopen([], 0) ; }  ;
    ok $@ =~ /^gzopen: file parameter is not a filehandle or filename at/
	or print "# $@\n" ;

#    my $x = Symbol::gensym() ;
#    eval { gzopen($x, 0) ; }  ;
#    ok $@ =~ /^gzopen: file parameter is not a filehandle or filename at/
#	or print "# $@\n" ;

}

if ($] >= 5.005)
{
    # test inflate with a substr

    ok my $x = deflateInit() ;

    ok ((my ($X, $status) = $x->deflate($contents))[1] == Z_OK) ;

    my $Y = $X ;



    ok ((($X, $status) = $x->flush() )[1] == Z_OK ) ;
    $Y .= $X ;

    my $append = "Appended" ;
    $Y .= $append ;

    ok $k = inflateInit() ;

    #($Z, $status) = $k->inflate(substr($Y, 0, -1)) ;
    ($Z, $status) = $k->inflate(substr($Y, 0)) ;

    ok $status == Z_STREAM_END ;
    ok $contents eq $Z ;
    is $Y, $append;

}

if ($] >= 5.005)
{
    # deflate/inflate in scalar context

    ok my $x = deflateInit() ;

    my $X = $x->deflate($contents);

    my $Y = $X ;



    $X = $x->flush();
    $Y .= $X ;

    my $append = "Appended" ;
    $Y .= $append ;

    ok $k = inflateInit() ;

    $Z = $k->inflate(substr($Y, 0, -1)) ;
    #$Z = $k->inflate(substr($Y, 0)) ;

    ok $contents eq $Z ;
    is $Y, $append;

}

{
    title 'CRC32' ;

    # CRC32 of this data should have the high bit set
    # value in ascii is ZgRNtjgSUW
    my $data = "\x5a\x67\x52\x4e\x74\x6a\x67\x53\x55\x57";
    my $expected_crc = 0xCF707A2B ; # 3480255019

    my $crc = crc32($data) ;
    is $crc, $expected_crc;
}

{
    title 'Adler32' ;

    # adler of this data should have the high bit set
    # value in ascii is lpscOVsAJiUfNComkOfWYBcPhHZ[bT
    my $data = "\x6c\x70\x73\x63\x4f\x56\x73\x41\x4a\x69\x55\x66" .
               "\x4e\x43\x6f\x6d\x6b\x4f\x66\x57\x59\x42\x63\x50" .
               "\x68\x48\x5a\x5b\x62\x54";
    my $expected_crc = 0xAAD60AC7 ; # 2866154183
    my $crc = adler32($data) ;
    is $crc, $expected_crc;
}

{
    # memGunzip - input > 4K

    my $contents = '' ;
    foreach (1 .. 20000)
      { $contents .= chr int rand 256 }

    ok my $compressed = memGzip(\$contents) ;
    is $gzerrno, 0;

    ok length $compressed > 4096 ;
    ok my $out = memGunzip(\$compressed) ;
    is $gzerrno, 0;

    ok $contents eq $out ;
    is length $out, length $contents ;


}


{
    # memGunzip Header Corruption Tests

    my $string = <<EOM;
some text
EOM

    my $good ;
    ok my $x = IO::Compress::Gzip->new( \$good, Append => 1, -HeaderCRC => 1 );
    ok $x->write($string) ;
    ok  $x->close ;

    {
        title "Header Corruption - Fingerprint wrong 1st byte" ;
        my $buffer = $good ;
        substr($buffer, 0, 1) = 'x' ;

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
    }

    {
        title "Header Corruption - Fingerprint wrong 2nd byte" ;
        my $buffer = $good ;
        substr($buffer, 1, 1) = "\xFF" ;

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
    }

    {
        title "Header Corruption - CM not 8";
        my $buffer = $good ;
        substr($buffer, 2, 1) = 'x' ;

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
    }

    {
        title "Header Corruption - Use of Reserved Flags";
        my $buffer = $good ;
        substr($buffer, 3, 1) = "\xff";

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
    }

}

for my $index ( GZIP_MIN_HEADER_SIZE + 1 ..  GZIP_MIN_HEADER_SIZE + GZIP_FEXTRA_HEADER_SIZE + 1)
{
    title "Header Corruption - Truncated in Extra";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok  my $x = IO::Compress::Gzip->new( \$truncated, Append => 1, -HeaderCRC => 1, Strict => 0,
				-ExtraField => "hello" x 10 );
    ok  $x->write($string) ;
    ok  $x->close ;

    substr($truncated, $index) = '' ;

    ok ! memGunzip(\$truncated) ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;


}

my $Name = "fred" ;
for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + length($Name) -1)
{
    title "Header Corruption - Truncated in Name";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok  my $x = IO::Compress::Gzip->new( \$truncated, Append => 1, -Name => $Name );
    ok  $x->write($string) ;
    ok  $x->close ;

    substr($truncated, $index) = '' ;

    ok ! memGunzip(\$truncated) ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
}

my $Comment = "comment" ;
for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + length($Comment) -1)
{
    title "Header Corruption - Truncated in Comment";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok  my $x = IO::Compress::Gzip->new( \$truncated, -Comment => $Comment );
    ok  $x->write($string) ;
    ok  $x->close ;

    substr($truncated, $index) = '' ;
    ok ! memGunzip(\$truncated) ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
}

for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + GZIP_FHCRC_SIZE -1)
{
    title "Header Corruption - Truncated in CRC";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok  my $x = IO::Compress::Gzip->new( \$truncated, -HeaderCRC => 1 );
    ok  $x->write($string) ;
    ok  $x->close ;

    substr($truncated, $index) = '' ;

    ok ! memGunzip(\$truncated) ;
    cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
}

{
    title "memGunzip can cope with a gzip header with all possible fields";
    my $string = <<EOM;
some text
EOM

    my $buffer ;
    ok  my $x = IO::Compress::Gzip->new( \$buffer,
                             -Append     => 1,
                             -Strict     => 0,
                             -HeaderCRC  => 1,
                             -Name       => "Fred",
                             -ExtraField => "Extra",
                             -Comment    => 'Comment' );
    ok  $x->write($string) ;
    ok  $x->close ;

    ok defined $buffer ;

    ok my $got = memGunzip($buffer)
        or diag "gzerrno is $gzerrno" ;
    is $got, $string ;
    is $gzerrno, 0;
}


{
    # Trailer Corruption tests

    my $string = <<EOM;
some text
EOM

    my $good ;
    ok  my $x = IO::Compress::Gzip->new( \$good, Append => 1 );
    ok  $x->write($string) ;
    ok  $x->close ;

    foreach my $trim (-8 .. -1)
    {
        my $got = $trim + 8 ;
        title "Trailer Corruption - Trailer truncated to $got bytes" ;
        my $buffer = $good ;

        substr($buffer, $trim) = '';

        ok my $u = memGunzip(\$buffer) ;
        is $gzerrno, 0;
        ok $u eq $string;

    }

    {
        title "Trailer Corruption - Length Wrong, CRC Correct" ;
        my $buffer = $good ;
        substr($buffer, -4, 4) = pack('V', 1234);

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;
    }

    {
        title "Trailer Corruption - Length Wrong, CRC Wrong" ;
        my $buffer = $good ;
        substr($buffer, -4, 4) = pack('V', 1234);
        substr($buffer, -8, 4) = pack('V', 1234);

        ok ! memGunzip(\$buffer) ;
        cmp_ok $gzerrno, "==", Z_DATA_ERROR ;

    }
}


sub slurp
{
    my $name = shift ;

    my $input;
    my $fil = gzopen($name, "rb") ;
    ok $fil , "opened $name";
    cmp_ok $fil->gzread($input, 50000), ">", 0, "read more than zero bytes";
    ok ! $fil->gzclose(), "closed ok";

    return $input;
}

sub trickle
{
    my $name = shift ;

    my $got;
    my $input;
    $fil = gzopen($name, "rb") ;
    ok $fil, "opened ok";
    while ($fil->gzread($input, 50000) > 0)
    {
        $got .= $input;
        $input = '';
    }
    ok ! $fil->gzclose(), "closed ok";

    return $got;

    return $input;
}

{

    title "Append & MultiStream Tests";
    # rt.24041

    my $lex = LexFile->new( my $name );
    my $data1 = "the is the first";
    my $data2 = "and this is the second";
    my $trailing = "some trailing data";

    my $fil;

    title "One file";
    $fil = gzopen($name, "wb") ;
    ok $fil, "opened first file";
    is $fil->gzwrite($data1), length $data1, "write data1" ;
    ok ! $fil->gzclose(), "Closed";

    is slurp($name), $data1, "got expected data from slurp";
    is trickle($name), $data1, "got expected data from trickle";

    title "Two files";
    $fil = gzopen($name, "ab") ;
    ok $fil, "opened second file";
    is $fil->gzwrite($data2), length $data2, "write data2" ;
    ok ! $fil->gzclose(), "Closed";

    is slurp($name), $data1 . $data2, "got expected data from slurp";
    is trickle($name), $data1 . $data2, "got expected data from trickle";

    title "Trailing Data";
    open F, ">>$name";
    print F $trailing;
    close F;

    is slurp($name), $data1 . $data2 . $trailing, "got expected data from slurp" ;
    is trickle($name), $data1 . $data2 . $trailing, "got expected data from trickle" ;
}

{
    title "gzclose & gzflush return codes";
    # rt.29215

    my $lex = LexFile->new( my $name );
    my $data1 = "the is some text";
    my $status;

    $fil = gzopen($name, "wb") ;
    ok $fil, "opened first file";
    is $fil->gzwrite($data1), length $data1, "write data1" ;
    $status = $fil->gzflush(0xfff);
    ok   $status, "flush not ok" ;
    is $status, Z_STREAM_ERROR;
    ok ! $fil->gzflush(), "flush ok" ;
    ok ! $fil->gzclose(), "Closed";
}



{
    title "repeated calls to flush - no compression";

    my ($err, $x, $X, $status, $data);

    ok( ($x, $err) = deflateInit ( ), "Create deflate object" );
    isa_ok $x, "Compress::Raw::Zlib::deflateStream" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;


    ($data, $status) = $x->flush(Z_SYNC_FLUSH) ;
    cmp_ok  $status, '==', Z_OK, "flush returned Z_OK" ;
    ($data, $status) = $x->flush(Z_SYNC_FLUSH) ;
    cmp_ok  $status, '==', Z_OK, "second flush returned Z_OK" ;
    is $data, "", "no output from second flush";
}

{
    title "repeated calls to flush - after compression";

    my $hello = "I am a HAL 9000 computer" ;
    my ($err, $x, $X, $status, $data);

    ok( ($x, $err) = deflateInit ( ), "Create deflate object" );
    isa_ok $x, "Compress::Raw::Zlib::deflateStream" ;
    cmp_ok $err, '==', Z_OK, "status is Z_OK" ;

    ($data, $status) = $x->deflate($hello) ;
    cmp_ok $status, '==', Z_OK, "deflate returned Z_OK" ;

    ($data, $status) = $x->flush(Z_SYNC_FLUSH) ;
    cmp_ok  $status, '==', Z_OK, "flush returned Z_OK" ;
    ($data, $status) = $x->flush(Z_SYNC_FLUSH) ;
    cmp_ok  $status, '==', Z_OK, "second flush returned Z_OK" ;
    is $data, "", "no output from second flush";
}
