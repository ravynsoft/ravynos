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
    plan skip_all => "Encode is not available"
        if $] < 5.006 ;

    eval { require Encode; Encode->import(); };

    plan skip_all => "Encode is not available"
        if $@ ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 29 + $extra ;

    use_ok('Compress::Zlib', qw(:ALL zlib_version memGunzip memGzip));
}




# Check zlib_version and ZLIB_VERSION are the same.
SKIP: {
    skip "TEST_SKIP_VERSION_CHECK is set", 1
        if $ENV{TEST_SKIP_VERSION_CHECK};
    is Compress::Zlib::zlib_version, ZLIB_VERSION,
        "ZLIB_VERSION matches Compress::Zlib::zlib_version" ;
}

{
    title "memGzip" ;
    # length of this string is 2 characters
    my $s = "\x{df}\x{100}";

    my $cs = memGzip(Encode::encode_utf8($s));

    # length stored at end of gzip file should be 4
    my ($crc, $len) = unpack ("VV", substr($cs, -8, 8));

    is $len, 4, "  length is 4";
}

{
    title "memGunzip when compressed gzip has been encoded" ;
    my $s = "hello world" ;

    my $co = memGzip($s);
    is memGunzip(my $x = $co), $s, "  match uncompressed";

    utf8::upgrade($co);

    my $un = memGunzip($co);
    ok $un, "  got uncompressed";

    is $un, $s, "  uncompressed matched original";
}

{
    title "compress/uncompress";

    my $s = "\x{df}\x{100}";
    my $s_copy = $s ;

    my $ces = compress(Encode::encode_utf8($s_copy));

    ok $ces, "  compressed ok" ;

    my $un = Encode::decode_utf8(uncompress($ces));
    is $un, $s, "  decode_utf8 ok";

    utf8::upgrade($ces);
    $un = Encode::decode_utf8(uncompress($ces));
    is $un, $s, "  decode_utf8 ok";

}

{
    title "gzopen" ;

    my $s = "\x{df}\x{100}";
    my $byte_len = length( Encode::encode_utf8($s) );
    my ($uncomp) ;

    my $lex = LexFile->new( my $name );
    ok my $fil = gzopen($name, "wb"), "  gzopen for write ok" ;

    is $fil->gzwrite(Encode::encode_utf8($s)), $byte_len, "  wrote $byte_len bytes" ;

    ok ! $fil->gzclose, "  gzclose ok" ;

    ok $fil = gzopen($name, "rb"), "  gzopen for read ok" ;

    is $fil->gzread($uncomp), $byte_len, "  read $byte_len bytes" ;
    is length($uncomp), $byte_len, "  uncompress is $byte_len bytes";

    ok ! $fil->gzclose, "gzclose ok" ;

    is $s, Encode::decode_utf8($uncomp), "  decode_utf8 ok" ;
}

{
    title "Catch wide characters";

    my $a = "a\xFF\x{100}";
    eval { memGzip($a) };
    like($@, qr/Wide character in memGzip/, "  wide characters in memGzip");

    eval { memGunzip($a) };
    like($@, qr/Wide character in memGunzip/, "  wide characters in memGunzip");

    eval { compress($a) };
    like($@, qr/Wide character in compress/, "  wide characters in compress");

    eval { uncompress($a) };
    like($@, qr/Wide character in uncompress/, "  wide characters in uncompress");

    my $lex = LexFile->new( my $name );
    ok my $fil = gzopen($name, "wb"), "  gzopen for write ok" ;

    eval { $fil->gzwrite($a); } ;
    like($@, qr/Wide character in gzwrite/, "  wide characters in gzwrite");

    ok ! $fil->gzclose, "  gzclose ok" ;
}
