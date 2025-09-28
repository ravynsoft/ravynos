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


    plan tests => 918 + $extra ;

    use_ok('Compress::Raw::Zlib') ;
    use_ok('IO::Compress::Gzip::Constants') ;

    use_ok('IO::Compress::Gzip', qw($GzipError)) ;
    use_ok('IO::Uncompress::Gunzip', qw($GunzipError)) ;

}



# Check the Gzip Header Parameters
#========================================

my $ThisOS_code = $Compress::Raw::Zlib::gzip_os_code;

my $lex = LexFile->new( my $name );

{
    title "Check Defaults";
    # Check Name defaults undef, no name, no comment
    # and Time can be explicitly set.

    my $hdr = readHeaderInfo($name, -Time => 1234);

    is $hdr->{Time}, 1234;
    ok ! defined $hdr->{Name};
    is $hdr->{MethodName}, 'Deflated';
    is $hdr->{ExtraFlags}, 0;
    is $hdr->{MethodID}, Z_DEFLATED;
    is $hdr->{OsID}, $ThisOS_code ;
    ok ! defined $hdr->{Comment} ;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{HeaderCRC} ;
    ok ! $hdr->{isMinimalHeader} ;
}

{

    title "Check name can be different from filename" ;
    # Check Name can be different from filename
    # Comment and Extra can be set
    # Can specify a zero Time

    my $comment = "This is a Comment" ;
    my $extra = "A little something extra" ;
    my $aname = "a new name" ;
    my $hdr = readHeaderInfo $name,
				      -Strict     => 0,
				      -Name       => $aname,
    				  -Comment    => $comment,
    				  -ExtraField => $extra,
    				  -Time       => 0 ;

    ok $hdr->{Time} == 0;
    ok $hdr->{Name} eq $aname;
    ok $hdr->{MethodName} eq 'Deflated';
    ok $hdr->{MethodID} == 8;
    is $hdr->{ExtraFlags}, 0;
    ok $hdr->{Comment} eq $comment ;
    is $hdr->{OsID}, $ThisOS_code ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! defined $hdr->{HeaderCRC} ;
}

{
    title "Check Time defaults to now" ;

    # Check Time defaults to now
    # and that can have empty name, comment and extrafield
    my $before = time ;
    my $hdr = readHeaderInfo $name,
		          -TextFlag   => 1,
		          -Name       => "",
    		      -Comment    => "",
    		      -ExtraField => "";
    my $after = time ;

    ok $hdr->{Time} >= $before ;
    ok $hdr->{Time} <= $after ;

    ok defined $hdr->{Name} ;
    ok $hdr->{Name} eq "";
    ok defined $hdr->{Comment} ;
    ok $hdr->{Comment} eq "";
    ok defined $hdr->{ExtraFieldRaw} ;
    ok $hdr->{ExtraFieldRaw} eq "";
    is $hdr->{ExtraFlags}, 0;

    ok ! $hdr->{isMinimalHeader} ;
    ok   $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;
    is $hdr->{OsID}, $ThisOS_code ;

}

{
    title "can have null extrafield" ;

    my $before = time ;
    my $hdr = readHeaderInfo $name,
				      -strict     => 0,
		              -Name       => "a",
    			      -Comment    => "b",
    			      -ExtraField => "\x00";
    my $after = time ;

    ok $hdr->{Time} >= $before ;
    ok $hdr->{Time} <= $after ;
    ok $hdr->{Name} eq "a";
    ok $hdr->{Comment} eq "b";
    is $hdr->{ExtraFlags}, 0;
    ok $hdr->{ExtraFieldRaw} eq "\x00";
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;
    is $hdr->{OsID}, $ThisOS_code ;

}

{
    title "can have undef name, comment, time and extrafield" ;

    my $hdr = readHeaderInfo $name,
	                  -Name       => undef,
    		          -Comment    => undef,
    		          -ExtraField => undef,
                      -Time       => undef;

    ok $hdr->{Time} == 0;
    ok ! defined $hdr->{Name} ;
    ok ! defined $hdr->{Comment} ;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;
    is $hdr->{OsID}, $ThisOS_code ;

}

for my $value ( "0D", "0A", "0A0D", "0D0A", "0A0A", "0D0D")
{
    title "Comment with $value" ;

    my $v = pack "h*", $value;
    my $comment = "my${v}comment$v";
    my $hdr = readHeaderInfo $name,
                    Time => 0,
                  -TextFlag   => 1,
                  -Name       => "",
                  -Comment    => $comment,
                  -ExtraField => "";
    my $after = time ;

    is $hdr->{Time}, 0 ;

    ok defined $hdr->{Name} ;
    ok $hdr->{Name} eq "";
    ok defined $hdr->{Comment} ;
    is $hdr->{Comment}, $comment;
    ok defined $hdr->{ExtraFieldRaw} ;
    ok $hdr->{ExtraFieldRaw} eq "";
    is $hdr->{ExtraFlags}, 0;

    ok ! $hdr->{isMinimalHeader} ;
    ok   $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;
    is $hdr->{OsID}, $ThisOS_code ;
}

{
    title "Check crchdr" ;

    my $hdr = readHeaderInfo $name, -HeaderCRC  => 1;

    ok ! defined $hdr->{Name};
    is $hdr->{ExtraFlags}, 0;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok   defined $hdr->{HeaderCRC} ;
    is $hdr->{OsID}, $ThisOS_code ;
}

{
    title "Check ExtraFlags" ;

    my $hdr = readHeaderInfo $name, -Level  => Z_BEST_SPEED;

    ok ! defined $hdr->{Name};
    is $hdr->{ExtraFlags}, 4;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;

    $hdr = readHeaderInfo $name, -Level  => Z_BEST_COMPRESSION;

    ok ! defined $hdr->{Name};
    is $hdr->{ExtraFlags}, 2;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;

    $hdr = readHeaderInfo $name, -Level  => Z_BEST_COMPRESSION,
                                 -ExtraFlags => 42;

    ok ! defined $hdr->{Name};
    is $hdr->{ExtraFlags}, 42;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    ok ! $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok ! defined $hdr->{HeaderCRC} ;


}

{
    title "OS Code" ;

    for my $code ( -1, undef, '', 'fred' )
    {
        my $code_name = defined $code ? "'$code'" : "'undef'";
        eval { IO::Compress::Gzip->new( $name, -OS_Code => $code ) } ;
        like $@, mkErr("^IO::Compress::Gzip: Parameter 'OS_Code' must be an unsigned int, got $code_name"),
            " Trap OS Code $code_name";
    }

    for my $code ( qw( 256 ) )
    {
        eval { ok ! IO::Compress::Gzip->new($name, OS_Code => $code) };
        like $@, mkErr("OS_Code must be between 0 and 255, got '$code'"),
            " Trap OS Code $code";
        like $GzipError, "/OS_Code must be between 0 and 255, got '$code'/",
            " Trap OS Code $code";
    }

    for my $code ( qw(0 1 12 254 255) )
    {
        my $hdr = readHeaderInfo $name, OS_Code => $code;

        is $hdr->{OsID}, $code, "  Code is $code" ;
    }



}

{
    title 'Check ExtraField';

    my @tests = (
        [1, ['AB' => '']                   => [['AB'=>'']] ],
        [1, {'AB' => ''}                   => [['AB'=>'']] ],
        [1, ['AB' => 'Fred']               => [['AB'=>'Fred']] ],
        [1, {'AB' => 'Fred'}               => [['AB'=>'Fred']] ],
        [1, ['Xx' => '','AB' => 'Fred']    => [['Xx' => ''],['AB'=>'Fred']] ],
        [1, ['Xx' => '','Xx' => 'Fred']    => [['Xx' => ''],['Xx'=>'Fred']] ],
        [1, ['Xx' => '',
             'Xx' => 'Fred',
             'Xx' => 'Fred']               => [['Xx' => ''],['Xx'=>'Fred'],
                                               ['Xx'=>'Fred']] ],
        [1, [ ['Xx' => 'a'],
              ['AB' => 'Fred'] ]           => [['Xx' => 'a'],['AB'=>'Fred']] ],
        [0, {'AB' => 'Fred',
             'Pq' => 'r',
             "\x01\x02" => "\x03"}         => [['AB'=>'Fred'],
                                               ['Pq'=>'r'],
                                               ["\x01\x02"=>"\x03"]] ],
        [1, ['AB' => 'z' x GZIP_FEXTRA_SUBFIELD_MAX_SIZE] =>
                            [['AB'=>'z' x GZIP_FEXTRA_SUBFIELD_MAX_SIZE]] ],
                );

    foreach my $test (@tests) {
        my ($order, $input, $result) = @$test ;
        ok my $x = IO::Compress::Gzip->new( $name,
                                -ExtraField  => $input,
                                -HeaderCRC   => 1 )
            or diag "GzipError is $GzipError" ;                            ;
        my $string = "abcd" ;
        ok $x->write($string) ;
        ok $x->close ;
        #is GZreadFile($name), $string ;

        ok $x = IO::Uncompress::Gunzip->new( $name,
                              #-Strict     => 1,
                               -ParseExtra => 1 )
            or diag "GunzipError is $GunzipError" ;                            ;
        my $hdr = $x->getHeaderInfo();
        ok $hdr;
        ok ! defined $hdr->{Name};
        ok ! defined $hdr->{Comment} ;
        ok ! $hdr->{isMinimalHeader} ;
        ok ! $hdr->{TextFlag} ;
        ok   defined $hdr->{HeaderCRC} ;

        ok   defined $hdr->{ExtraFieldRaw} ;
        ok   defined $hdr->{ExtraField} ;

        my $extra = $hdr->{ExtraField} ;

        if ($order) {
            eq_array $extra, $result;
        } else {
            eq_set $extra, $result;
        }
    }

}

{
    title 'Write Invalid ExtraField';

    my $prefix = 'Error with ExtraField Parameter: ';
    my @tests = (
            [ sub{ "abc" }        => "Not a scalar, array ref or hash ref"],
            [ [ "a" ]             => "Not even number of elements"],
            [ [ "a" => "fred" ]   => 'SubField ID not two chars long'],
            [ [ "a\x00" => "fred" ]   => 'SubField ID 2nd byte is 0x00'],
            [ [ [ {}, "abc" ]]    => "SubField ID is a reference"],
            [ [ [ "ab", \1 ]]     => "SubField Data is a reference"],
            [ [ {"a" => "fred"} ] => "Not list of lists"],
            [ [ ['ab'=>'x'],{"a" => "fred"} ] => "Not list of lists"],
            [ [ ["aa"] ]          => "SubField must have two parts"],
            [ [ ["aa", "b", "c"] ] => "SubField must have two parts"],
            [ [ ["ab" => 'x' x (GZIP_FEXTRA_SUBFIELD_MAX_SIZE + 1) ] ]
                                   => "SubField Data too long"],

            [ { 'abc', 1 }        => "SubField ID not two chars long"],
            [ { \1 , "abc" }    => "SubField ID not two chars long"],
            [ { "ab", \1 }     => "SubField Data is a reference"],
        );



    foreach my $test (@tests) {
        my ($input, $string) = @$test ;
        my $buffer ;
        my $x ;
        eval { $x = IO::Compress::Gzip->new( \$buffer, -ExtraField  => $input ); };
        like $@, mkErr("$prefix$string");
        like $GzipError, "/$prefix$string/";
        ok ! $x ;

    }

}

{
    # Corrupt ExtraField

    my @tests = (
        ["Sub-field truncated",
            "Error with ExtraField Parameter: Truncated in FEXTRA Body Section",
            "Header Error: Truncated in FEXTRA Body Section",
            ['a', undef, undef]              ],
        ["Length of field incorrect",
            "Error with ExtraField Parameter: Truncated in FEXTRA Body Section",
            "Header Error: Truncated in FEXTRA Body Section",
            ["ab", 255, "abc"]               ],
        ["Length of 2nd field incorrect",
            "Error with ExtraField Parameter: Truncated in FEXTRA Body Section",
            "Header Error: Truncated in FEXTRA Body Section",
            ["ab", 3, "abc"], ["de", 7, "x"] ],
        ["Length of 2nd field incorrect",
            "Error with ExtraField Parameter: SubField ID 2nd byte is 0x00",
            "Header Error: SubField ID 2nd byte is 0x00",
            ["a\x00", 3, "abc"], ["de", 7, "x"] ],
        );

    foreach my $test (@tests)
    {
        my $name = shift @$test;
        my $gzip_error = shift @$test;
        my $gunzip_error = shift @$test;

        title "Read Corrupt ExtraField - $name" ;

        my $input = '';

        for my $field (@$test)
        {
            my ($id, $len, $data) = @$field;

            $input .= $id if defined $id ;
            $input .= pack("v", $len) if defined $len ;
            $input .= $data if defined $data;
        }
        #hexDump(\$input);

        my $buffer ;
        my $x ;
        eval {$x = IO::Compress::Gzip->new( \$buffer, -ExtraField  => $input, Strict => 1 ); };
        like $@, mkErr("$gzip_error"), "  $name";
        like $GzipError, "/$gzip_error/", "  $name";

        ok ! $x, "  IO::Compress::Gzip fails";
        like $GzipError, "/$gzip_error/", "  $name";

        foreach my $check (0, 1)
        {
            ok $x = IO::Compress::Gzip->new( \$buffer,
                                           ExtraField => $input,
                                           Strict     => 0 )
                or diag "GzipError is $GzipError" ;
            my $string = "abcd" ;
            $x->write($string) ;
            $x->close ;
            is anyUncompress(\$buffer), $string ;

            $x = IO::Uncompress::Gunzip->new( \$buffer,
                                       Strict      => 0,
                                       Transparent => 0,
                                       ParseExtra  => $check );
            if ($check) {
                ok ! $x ;
                like $GunzipError, "/^$gunzip_error/";
            }
            else {
                ok $x ;
            }

        }
    }
}


{
    title 'Check Minimal';

    ok my $x = IO::Compress::Gzip->new( $name, -Minimal => 1 );
    my $string = "abcd" ;
    ok $x->write($string) ;
    ok $x->close ;
    #is GZreadFile($name), $string ;

    ok $x = IO::Uncompress::Gunzip->new( $name );
    my $hdr = $x->getHeaderInfo();
    ok $hdr;
    ok $hdr->{Time} == 0;
    is $hdr->{ExtraFlags}, 0;
    ok ! defined $hdr->{Name} ;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    is $hdr->{OsName}, 'Unknown' ;
    is $hdr->{MethodName}, "Deflated";
    is $hdr->{Flags}, 0;
    ok $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok $x->close ;
}

{
    title "Check Minimal + no compressed data";
    # This is the smallest possible gzip file (20 bytes)

    ok my $x = IO::Compress::Gzip->new( $name, -Minimal => 1 );
    isa_ok $x, "IO::Compress::Gzip";
    ok $x->close, "closed" ;

    ok $x = IO::Uncompress::Gunzip->new( $name, -Append => 0 );
    isa_ok $x, "IO::Uncompress::Gunzip";
    my $data ;
    my $status  = 1;

    ok $x->eof(), "eof" ;
    $status = $x->read($data)
        while $status >  0;
    is $status, 0, "status == 0" ;
    is $data, '', "empty string";
    ok ! $x->error(), "no error" ;
    ok $x->eof(), "eof" ;

    my $hdr = $x->getHeaderInfo();
    ok $hdr;

    ok defined $hdr->{ISIZE} ;
    is $hdr->{ISIZE}, 0;

    ok defined $hdr->{CRC32} ;
    is $hdr->{CRC32}, 0;

    is $hdr->{Time}, 0;
    ok ! defined $hdr->{Name} ;
    ok ! defined $hdr->{ExtraFieldRaw} ;
    ok ! defined $hdr->{Comment} ;
    is $hdr->{OsName}, 'Unknown' ;
    is $hdr->{MethodName}, "Deflated";
    is $hdr->{Flags}, 0;
    ok $hdr->{isMinimalHeader} ;
    ok ! $hdr->{TextFlag} ;
    ok $x->close ;
}

{
    title "Header Corruption Tests";

    my $string = <<EOM;
some text
EOM

    my $good = '';
    ok my $x = IO::Compress::Gzip->new( \$good, -HeaderCRC => 1 );
    ok $x->write($string) ;
    ok $x->close ;

    {
        title "Header Corruption - Fingerprint wrong 1st byte" ;
        my $buffer = $good ;
        substr($buffer, 0, 1) = 'x' ;

        ok ! IO::Uncompress::Gunzip->new( \$buffer, -Transparent => 0 );
        ok $GunzipError =~ /Header Error: Bad Magic/;
    }

    {
        title "Header Corruption - Fingerprint wrong 2nd byte" ;
        my $buffer = $good ;
        substr($buffer, 1, 1) = "\xFF" ;

        ok ! IO::Uncompress::Gunzip->new( \$buffer, -Transparent => 0 );
        ok $GunzipError =~ /Header Error: Bad Magic/;
        #print "$GunzipError\n";
    }

    {
        title "Header Corruption - CM not 8";
        my $buffer = $good ;
        substr($buffer, 2, 1) = 'x' ;

        ok ! IO::Uncompress::Gunzip->new( \$buffer, -Transparent => 0 );
        like $GunzipError, '/Header Error: Not Deflate \(CM is \d+\)/';
    }

    {
        title "Header Corruption - Use of Reserved Flags";
        my $buffer = $good ;
        substr($buffer, 3, 1) = "\xff";

        ok ! IO::Uncompress::Gunzip->new( \$buffer, -Transparent => 0 );
        like $GunzipError, '/Header Error: Use of Reserved Bits in FLG field./';
    }

    {
        title "Header Corruption - Fail HeaderCRC";
        my $buffer = $good ;
        substr($buffer, 10, 1) = chr((ord(substr($buffer, 10, 1)) + 1) & 0xFF);

        ok ! IO::Uncompress::Gunzip->new( \$buffer, -Transparent => 0, Strict => 1 )
         or print "# $GunzipError\n";
        like $GunzipError, '/Header Error: CRC16 mismatch/'
            #or diag "buffer length " . length($buffer);
            or hexDump(\$good), hexDump(\$buffer);
    }
}

{
    title "ExtraField max raw size";
    my $x ;
    my $store = "x" x GZIP_FEXTRA_MAX_SIZE ;
    {
        my $z = IO::Compress::Gzip->new(\$x, ExtraField => $store, Strict => 0) ;
        ok $z,  "Created IO::Compress::Gzip object" ;
    }
    my $gunz = IO::Uncompress::Gunzip->new( \$x, Strict => 0 );
    ok $gunz, "Created IO::Uncompress::Gunzip object" ;
    my $hdr = $gunz->getHeaderInfo();
    ok $hdr;

    is $hdr->{ExtraFieldRaw}, $store ;
}

{
    title "Header Corruption - ExtraField too big";
    my $x;
    eval { IO::Compress::Gzip->new(\$x, -ExtraField => "x" x (GZIP_FEXTRA_MAX_SIZE + 1)) ;};
    like $@, mkErr('Error with ExtraField Parameter: Too Large');
    like $GzipError, '/Error with ExtraField Parameter: Too Large/';
}

{
    title "Header Corruption - Create Name with Illegal Chars";

    my $x;
    eval { IO::Compress::Gzip->new( \$x, -Name => "fred\x02" ) };
    like $@, mkErr('Non ISO 8859-1 Character found in Name');
    like $GzipError, '/Non ISO 8859-1 Character found in Name/';

    ok  my $gz = IO::Compress::Gzip->new( \$x,
		                      -Strict => 0,
		                      -Name => "fred\x02" );
    ok $gz->close();

    ok ! IO::Uncompress::Gunzip->new( \$x,
                        -Transparent => 0,
                        -Strict => 1 );

    like $GunzipError, '/Header Error: Non ISO 8859-1 Character found in Name/';
    ok my $gunzip = IO::Uncompress::Gunzip->new( \$x,
                                   -Strict => 0 );

    my $hdr = $gunzip->getHeaderInfo() ;

    is $hdr->{Name}, "fred\x02";

}

{
    title "Header Corruption - Null Chars in Name";
    my $x;
    eval { IO::Compress::Gzip->new( \$x, -Name => "\x00" ) };
    like $@, mkErr('Null Character found in Name');
    like $GzipError, '/Null Character found in Name/';

    eval { IO::Compress::Gzip->new( \$x, -Name => "abc\x00" ) };
    like $@, mkErr('Null Character found in Name');
    like $GzipError, '/Null Character found in Name/';

    ok my $gz = IO::Compress::Gzip->new( \$x,
		                     -Strict  => 0,
		                     -Name => "abc\x00de" );
    ok $gz->close() ;
    ok my $gunzip = IO::Uncompress::Gunzip->new( \$x,
                                   -Strict => 0 );

    my $hdr = $gunzip->getHeaderInfo() ;

    is $hdr->{Name}, "abc";

}

{
    title "Header Corruption - Create Comment with Illegal Chars";

    my $x;
    eval { IO::Compress::Gzip->new( \$x, -Comment => "fred\x02" ) };
    like $@, mkErr('Non ISO 8859-1 Character found in Comment');
    like $GzipError, '/Non ISO 8859-1 Character found in Comment/';

    ok  my $gz = IO::Compress::Gzip->new( \$x,
		                      -Strict => 0,
		                      -Comment => "fred\x02" );
    ok $gz->close();

    ok ! IO::Uncompress::Gunzip->new( \$x, Strict => 1,
                        -Transparent => 0 );

    like $GunzipError, '/Header Error: Non ISO 8859-1 Character found in Comment/';
    ok my $gunzip = IO::Uncompress::Gunzip->new( \$x, Strict => 0 );

    my $hdr = $gunzip->getHeaderInfo() ;

    is $hdr->{Comment}, "fred\x02";

}

{
    title "Header Corruption - Null Char in Comment";
    my $x;
    eval { IO::Compress::Gzip->new( \$x, -Comment => "\x00" ) };
    like $@, mkErr('Null Character found in Comment');
    like $GzipError, '/Null Character found in Comment/';

    eval { IO::Compress::Gzip->new( \$x, -Comment => "abc\x00" ) } ;
    like $@, mkErr('Null Character found in Comment');
    like $GzipError, '/Null Character found in Comment/';

    ok my $gz = IO::Compress::Gzip->new( \$x,
		                     -Strict  => 0,
		                     -Comment => "abc\x00de" );
    ok $gz->close() ;
    ok my $gunzip = IO::Uncompress::Gunzip->new( \$x,
                                   -Strict => 0 );

    my $hdr = $gunzip->getHeaderInfo() ;

    is $hdr->{Comment}, "abc";

}


for my $index ( GZIP_MIN_HEADER_SIZE + 1 ..  GZIP_MIN_HEADER_SIZE + GZIP_FEXTRA_HEADER_SIZE + 1)
{
    title "Header Corruption - Truncated in Extra";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok my $x = IO::Compress::Gzip->new( \$truncated, -HeaderCRC => 1, Strict => 0,
				-ExtraField => "hello" x 10 );
    ok $x->write($string) ;
    ok $x->close ;

    substr($truncated, $index) = '' ;
    #my $lex = LexFile->new( my $name );
    #writeFile($name, $truncated) ;

    #my $g = IO::Uncompress::Gunzip->new( $name, -Transparent => 0 );
    my $g = IO::Uncompress::Gunzip->new( \$truncated, -Transparent => 0 );
    ok ! $g
	or print "# $g\n" ;

    like($GunzipError, '/^Header Error: Truncated in FEXTRA/');


}

my $Name = "fred" ;
    my $truncated ;
for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + length($Name) -1)
{
    title "Header Corruption - Truncated in Name";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok my $x = IO::Compress::Gzip->new( \$truncated, -Name => $Name );
    ok $x->write($string) ;
    ok $x->close ;

    substr($truncated, $index) = '' ;

    my $g = IO::Uncompress::Gunzip->new( \$truncated, -Transparent => 0 );
    ok ! $g
	or print "# $g\n" ;

    like $GunzipError, '/^Header Error: Truncated in FNAME Section/';

}

my $Comment = "comment" ;
for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + length($Comment) -1)
{
    title "Header Corruption - Truncated in Comment";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok my $x = IO::Compress::Gzip->new( \$truncated, -Comment => $Comment );
    ok $x->write($string) ;
    ok $x->close ;

    substr($truncated, $index) = '' ;
    #my $lex = LexFile->new( my $name );
    #writeFile($name, $truncated) ;

    #my $g = IO::Uncompress::Gunzip->new( $name, -Transparent => 0 );
    my $g = IO::Uncompress::Gunzip->new( \$truncated, -Transparent => 0 );
    ok ! $g
	or print "# $g\n" ;

    like $GunzipError, '/^Header Error: Truncated in FCOMMENT Section/';

}

for my $index ( GZIP_MIN_HEADER_SIZE ..  GZIP_MIN_HEADER_SIZE + GZIP_FHCRC_SIZE -1)
{
    title "Header Corruption - Truncated in CRC";
    my $string = <<EOM;
some text
EOM

    my $truncated ;
    ok my $x = IO::Compress::Gzip->new( \$truncated, -HeaderCRC => 1 );
    ok $x->write($string) ;
    ok $x->close ;

    substr($truncated, $index) = '' ;
    my $lex = LexFile->new( my $name );
    writeFile($name, $truncated) ;

    my $g = IO::Uncompress::Gunzip->new( $name, -Transparent => 0 );
    ok ! $g
	or print "# $g\n" ;

    like $GunzipError, '/^Header Error: Truncated in FHCRC Section/';

}


{
    # Trailer Corruption tests

    my $string = <<EOM;
some text
EOM
    $string = $string x 1000;

    my $good ;
    {
        ok my $x = IO::Compress::Gzip->new( \$good );
        ok $x->write($string) ;
        ok $x->close ;
    }

    writeFile($name, $good) ;
    ok my $gunz = IO::Uncompress::Gunzip->new( $name,
                                       -Append   => 1,
                                       -Strict   => 1 );
    my $uncomp ;
    1 while  $gunz->read($uncomp) > 0 ;
    ok $gunz->close() ;
    ok $uncomp eq $string
	or print "# got [$uncomp] wanted [$string]\n";;

    foreach my $trim (-8 .. -1)
    {
        my $got = $trim + 8 ;
        title "Trailer Corruption - Trailer truncated to $got bytes" ;
        my $buffer = $good ;
        my $expected_trailing = substr($good, -8, 8) ;
        substr($expected_trailing, $trim) = '';

        substr($buffer, $trim) = '';
        writeFile($name, $buffer) ;

        foreach my $strict (0, 1)
        {
            ok my $gunz = IO::Uncompress::Gunzip->new( $name, Append => 1, -Strict   => $strict );
            my $uncomp ;
            my $status = 1;
            $status = $gunz->read($uncomp) while $status > 0;
            if ($strict)
            {
                cmp_ok $status, '<', 0, "status 0" ;
                like $GunzipError, "/Trailer Error: trailer truncated. Expected 8 bytes, got $got/", "got Trailer Error";
            }
            else
            {
                is $status, 0, "status 0";
                ok ! $GunzipError, "no error"
                    or diag "$GunzipError";
                my $expected = substr($buffer, - $got);
                is  $gunz->trailingData(),  $expected_trailing, "trailing data";
            }
            ok $gunz->eof() ;
            ok $uncomp eq $string;
            ok $gunz->close ;
        }

    }

    {
        title "Trailer Corruption - Length Wrong, CRC Correct" ;
        my $buffer = $good ;
        my $actual_len = unpack("V", substr($buffer, -4, 4));
        substr($buffer, -4, 4) = pack('V', $actual_len + 1);
        writeFile($name, $buffer) ;

        foreach my $strict (0, 1)
        {
            ok my $gunz = IO::Uncompress::Gunzip->new( $name,
                                               Append   => 1,
                                               -Strict   => $strict );
            my $uncomp ;
            my $status = 1;
            $status = $gunz->read($uncomp) while $status > 0;
            if ($strict)
            {
                cmp_ok $status, '<', 0 ;
                my $got_len = $actual_len + 1;
                like $GunzipError, "/Trailer Error: ISIZE mismatch. Got $got_len, expected $actual_len/";
            }
            else
            {
                is $status, 0;
                ok ! $GunzipError ;
                #is   $gunz->trailingData(), substr($buffer, - $got) ;
            }
            ok ! $gunz->trailingData() ;
            ok $gunz->eof() ;
            ok $uncomp eq $string;
            ok $gunz->close ;
        }

    }

    {
        title "Trailer Corruption - Length Correct, CRC Wrong" ;
        my $buffer = $good ;
        my $actual_crc = unpack("V", substr($buffer, -8, 4));
        substr($buffer, -8, 4) = pack('V', $actual_crc+1);
        writeFile($name, $buffer) ;

        foreach my $strict (0, 1)
        {
            ok my $gunz = IO::Uncompress::Gunzip->new( $name,
                                               -Append   => 1,
                                               -Strict   => $strict );
            my $uncomp ;
            my $status = 1;
            $status = $gunz->read($uncomp) while $status > 0;
            if ($strict)
            {
                cmp_ok $status, '<', 0 ;
                like $GunzipError, '/Trailer Error: CRC mismatch/';
            }
            else
            {
                is $status, 0;
                ok ! $GunzipError ;
            }
            ok ! $gunz->trailingData() ;
            ok $gunz->eof() ;
            ok $uncomp eq $string;
            ok $gunz->close ;
        }

    }

    {
        title "Trailer Corruption - Length Wrong, CRC Wrong" ;
        my $buffer = $good ;
        my $actual_len = unpack("V", substr($buffer, -4, 4));
        my $actual_crc = unpack("V", substr($buffer, -8, 4));
        substr($buffer, -4, 4) = pack('V', $actual_len+1);
        substr($buffer, -8, 4) = pack('V', $actual_crc+1);
        writeFile($name, $buffer) ;

        foreach my $strict (0, 1)
        {
            ok my $gunz = IO::Uncompress::Gunzip->new( $name,
                                               -Append   => 1,
                                               -Strict   => $strict );
            my $uncomp ;
            my $status = 1;
            $status = $gunz->read($uncomp) while $status > 0;
            if ($strict)
            {
                cmp_ok $status, '<', 0 ;
                like $GunzipError, '/Trailer Error: CRC mismatch/';
            }
            else
            {
                is $status, 0;
                ok ! $GunzipError ;
            }
            ok $gunz->eof() ;
            ok $uncomp eq $string;
            ok $gunz->close ;
        }

    }

    {
        # RT #72329
        my $error = 'Error with ExtraField Parameter: ' .
                    'SubField ID not two chars long' ;
        my $buffer ;
        my $x ;
        eval { $x = IO::Compress::Gzip->new( \$buffer,
                -ExtraField  => [ at => 'mouse', bad => 'dog'] );
             };
        like $@, mkErr("$error");
        like $GzipError, "/$error/";
        ok ! $x ;
    }
}



