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

BEGIN {
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 114 + $extra ;

    use_ok('Compress::Raw::Zlib') ;

    use_ok('IO::Compress::Deflate', qw($DeflateError)) ;
    use_ok('IO::Uncompress::Inflate', qw($InflateError)) ;

    use_ok('IO::Compress::Zlib::Constants');

}


sub ReadHeaderInfo
{
    my $string = shift || '' ;
    my %opts = @_ ;

    my $buffer ;
    ok my $def = IO::Compress::Deflate->new( \$buffer, %opts );
    is $def->write($string), length($string), "write" ;
    ok $def->close, "closed" ;
    #print "ReadHeaderInfo\n"; hexDump(\$buffer);

    ok my $inf = IO::Uncompress::Inflate->new( \$buffer, Append => 1 );
    my $uncomp = "";
    #ok $inf->read($uncomp) ;
    my $actual = 0 ;
    my $status = 1 ;
    while (($status = $inf->read($uncomp)) > 0) {
        $actual += $status ;
    }

    is $actual, length($string) ;
    is $uncomp, $string;
    ok ! $inf->error(), "! error" ;
    ok $inf->eof(), "eof" ;
    ok my $hdr = $inf->getHeaderInfo();
    ok $inf->close ;

    return $hdr ;
}

sub ReadHeaderInfoZlib
{
    my $string = shift || '' ;
    my %opts = @_ ;

    my $buffer ;
    ok my $def = Compress::Raw::Zlib::Deflate->new( AppendOutput => 1, %opts );
    cmp_ok $def->deflate($string, $buffer), '==',  Z_OK;
    cmp_ok $def->flush($buffer), '==', Z_OK;
    #print "ReadHeaderInfoZlib\n"; hexDump(\$buffer);

    ok my $inf = IO::Uncompress::Inflate->new( \$buffer, Append => 1 );
    my $uncomp ;
    #ok $inf->read($uncomp) ;
    my $actual = 0 ;
    my $status = 1 ;
    while (($status = $inf->read($uncomp)) > 0) {
        $actual += $status ;
    }

    is $actual, length($string) ;
    is $uncomp, $string;
    ok ! $inf->error() ;
    ok $inf->eof() ;
    ok my $hdr = $inf->getHeaderInfo();
    ok $inf->close ;

    return $hdr ;
}

sub printHeaderInfo
{
    my $buffer = shift ;
    my $inf = IO::Uncompress::Inflate->new( \$buffer );
    my $hdr = $inf->getHeaderInfo();

    no warnings 'uninitialized' ;
    while (my ($k, $v) = each %$hdr) {
        print "  $k -> $v\n" ;
    }
}


# Check the Deflate Header Parameters
#========================================

#my $lex = LexFile->new( my $name );

{
    title "Check default header settings" ;

    my $string = <<EOM;
some text
EOM

    my $hdr = ReadHeaderInfo($string);

    is $hdr->{CM}, 8, "  CM is 8";
    is $hdr->{FDICT}, 0, "  FDICT is 0";

}

if (0) # disable these tests: IO::Compress::Deflate doesn't create the zlib header itself so no need to test
{
    title "Check user-defined header settings match zlib" ;

    my $string = <<EOM;
some text
EOM

    my @tests = (
        [ {-Level => 0}, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
        [ {-Level => 1}, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
        [ {-Level => 2}, { FLEVEL => ZLIB_FLG_LEVEL_FAST   } ],
        [ {-Level => 3}, { FLEVEL => ZLIB_FLG_LEVEL_FAST   } ],
        [ {-Level => 4}, { FLEVEL => ZLIB_FLG_LEVEL_FAST   } ],
        [ {-Level => 5}, { FLEVEL => ZLIB_FLG_LEVEL_FAST   } ],
        [ {-Level => 6}, { FLEVEL => ZLIB_FLG_LEVEL_DEFAULT} ],
        [ {-Level => 7}, { FLEVEL => ZLIB_FLG_LEVEL_SLOWEST} ],
        [ {-Level => 8}, { FLEVEL => ZLIB_FLG_LEVEL_SLOWEST} ],
        [ {-Level => 9}, { FLEVEL => ZLIB_FLG_LEVEL_SLOWEST} ],

        [ {-Level => Z_NO_COMPRESSION  }, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
        [ {-Level => Z_BEST_SPEED      }, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
        [ {-Level => Z_BEST_COMPRESSION}, { FLEVEL => ZLIB_FLG_LEVEL_SLOWEST} ],
        [ {-Level => Z_DEFAULT_COMPRESSION}, { FLEVEL => ZLIB_FLG_LEVEL_DEFAULT} ],

        [ {-Strategy => Z_HUFFMAN_ONLY}, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
        [ {-Strategy => Z_HUFFMAN_ONLY,
           -Level    => 3             }, { FLEVEL => ZLIB_FLG_LEVEL_FASTEST} ],
    );

    foreach my $test (@tests)
    {
        my $opts = $test->[0] ;
        my $expect = $test->[1] ;

        my @title ;
        while (my ($k, $v) = each %$opts)
        {
            push @title, "$k => $v";
        }
        title " Set @title";

        my $hdr = ReadHeaderInfo($string, %$opts);

        my $hdr1 = ReadHeaderInfoZlib($string, %$opts);

        # zlib-ng <= 2.0.6 with Level 1 sets the CINFO value to 5 . All other zlib & zlib-ng use expected value of 7
        # Note that zlib-ng 2.0.x uses a 16-bit encoding for ZLIBNG_VERNUM
        my $cinfoValue =  Compress::Raw::Zlib::is_zlibng() && Compress::Raw::Zlib::ZLIBNG_VERNUM() <= 0x2060 && defined $opts->{'-Level'} && $opts->{'-Level'} == 1 ? 5 : 7;
        is $hdr->{CM},     8, "  CM is 8";
        is $hdr->{CINFO},  $cinfoValue, "  CINFO is $cinfoValue";
        is $hdr->{FDICT},  0, "  FDICT is 0";

        while (my ($k, $v) = each %$expect)
        {
            if (Compress::Raw::Zlib::is_zlibng() || ZLIB_VERNUM >= 0x1220)
              { is $hdr->{$k}, $v, "  $k is $v" }
            else
              { ok 1, "  Skip test for $k" }
        }

        is $hdr->{CM},     $hdr1->{CM},     "  CM matches";
        is $hdr->{CINFO},  $hdr1->{CINFO},  "  CINFO matches";
        is $hdr->{FDICT},  $hdr1->{FDICT},  "  FDICT matches";
        is $hdr->{FLEVEL}, $hdr1->{FLEVEL}, "  FLEVEL matches";
        is $hdr->{FCHECK}, $hdr1->{FCHECK}, "  FCHECK matches";
    }


}

{
    title "No compressed data at all";

    my $hdr = ReadHeaderInfo("");

    is $hdr->{CM}, 8, "  CM is 8";
    is $hdr->{FDICT}, 0, "  FDICT is 0";

    ok defined $hdr->{ADLER32}, "  ADLER32 is defined" ;
    is $hdr->{ADLER32}, 1, "  ADLER32 is 1";
}

{
    # Header Corruption Tests

    my $string = <<EOM;
some text
EOM

    my $good ;
    ok my $x = IO::Compress::Deflate->new( \$good );
    ok $x->write($string) ;
    ok $x->close ;

    {
        title "Header Corruption - FCHECK failure - 1st byte wrong";
        my $buffer = $good ;
        substr($buffer, 0, 1) = "\x00" ;

        ok ! IO::Uncompress::Inflate->new( \$buffer, -Transparent => 0 );
        like $IO::Uncompress::Inflate::InflateError, '/Header Error: CRC mismatch/',
            "CRC mismatch";
    }

    {
        title "Header Corruption - FCHECK failure - 2nd byte wrong";
        my $buffer = $good ;
        substr($buffer, 1, 1) = "\x00" ;

        ok ! IO::Uncompress::Inflate->new( \$buffer, -Transparent => 0 );
        like $IO::Uncompress::Inflate::InflateError, '/Header Error: CRC mismatch/',
            "CRC mismatch";
    }


    sub mkZlibHdr
    {
        my $method = shift ;
        my $cinfo  = shift ;
        my $fdict  = shift ;
        my $level  = shift ;

        my $cmf  = ($method & 0x0F) ;
           $cmf |= (($cinfo  & 0x0F) << 4) ;
        my $flg  = (($level & 0x03) << 6) ;
           $flg |= (($fdict & 0x01) << 5) ;
        my $fcheck = 31 - ($cmf * 256 + $flg) % 31 ;
        $flg |= $fcheck ;
        #print "check $fcheck\n";

        return pack("CC", $cmf, $flg) ;
    }

    {
        title "Header Corruption - CM not 8";
        my $buffer = $good ;
        my $header = mkZlibHdr(3, 6, 0, 3);

        substr($buffer, 0, 2) = $header;

        my $un = IO::Uncompress::Inflate->new( \$buffer, -Transparent => 0 );
        ok ! IO::Uncompress::Inflate->new( \$buffer, -Transparent => 0 );
        like $IO::Uncompress::Inflate::InflateError, '/Header Error: Not Deflate \(CM is 3\)/',
            "  Not Deflate";
    }

}

{
    # Trailer Corruption tests

    my $string = <<EOM;
some text
EOM

    $string = $string x 1000;
    my $good ;
    ok my $x = IO::Compress::Deflate->new( \$good );
    ok $x->write($string) ;
    ok $x->close ;

    foreach my $trim (-4 .. -1)
    {
        my $got = $trim + 4 ;
        foreach my $s (0, 1)
        {
            title "Trailer Corruption - Trailer truncated to $got bytes, strict $s" ;
		    my $lex = LexFile->new( my $name );
            my $buffer = $good ;
            my $expected_trailing = substr($good, -4, 4) ;
            substr($expected_trailing, $trim) = '';

            substr($buffer, $trim) = '';
            writeFile($name, $buffer) ;

            ok my $gunz = IO::Uncompress::Inflate->new( $name, Append => 1, Strict => $s );
            my $uncomp ;
            if ($s)
            {
                my $status ;
                1 while ($status = $gunz->read($uncomp)) > 0;
                cmp_ok $status, "<", 0 ;
                like $IO::Uncompress::Inflate::InflateError,"/Trailer Error: trailer truncated. Expected 4 bytes, got $got/",
                    "Trailer Error";
            }
            else
            {
                1 while $gunz->read($uncomp) > 0;
                is $uncomp, $string ;
            }
            ok $gunz->eof() ;
            ok $uncomp eq $string;
            ok $gunz->close ;
        }

    }

    {
        title "Trailer Corruption - CRC Wrong, strict" ;
        my $buffer = $good ;
        my $crc = unpack("N", substr($buffer, -4, 4));
        substr($buffer, -4, 4) = pack('N', $crc+1);
		my $lex = LexFile->new( my $name );
        writeFile($name, $buffer) ;

        ok my $gunz = IO::Uncompress::Inflate->new( $name, Append => 1, Strict => 1 );
        my $uncomp ;
        my $status ;
        1 while ($status = $gunz->read($uncomp)) > 0;
        cmp_ok $status, "<", 0 ;
        like $IO::Uncompress::Inflate::InflateError,'/Trailer Error: CRC mismatch/',
            "Trailer Error: CRC mismatch";
        ok $gunz->eof() ;
        ok ! $gunz->trailingData() ;
        ok $uncomp eq $string;
        ok $gunz->close ;
    }

    {
        title "Trailer Corruption - CRC Wrong, no strict" ;
        my $buffer = $good ;
        my $crc = unpack("N", substr($buffer, -4, 4));
        substr($buffer, -4, 4) = pack('N', $crc+1);
		my $lex = LexFile->new( my $name );
        writeFile($name, $buffer) ;

        ok my $gunz = IO::Uncompress::Inflate->new( $name, Append => 1, Strict => 0 );
        my $uncomp ;
        my $status ;
        1 while ($status = $gunz->read($uncomp)) > 0;
        cmp_ok $status, '>=', 0  ;
        ok $gunz->eof() ;
        ok ! $gunz->trailingData() ;
        ok $uncomp eq $string;
        ok $gunz->close ;
    }
}
