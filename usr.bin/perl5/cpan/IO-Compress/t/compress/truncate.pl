
use lib 't';
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

sub run
{
    my $CompressClass   = identify();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);

#    my $hello = <<EOM ;
#hello world
#this is a test
#some more stuff on this line
#and finally...
#EOM

    # ASCII hex equivalent of the text above. This makes the test
    # harness behave identically on an EBCDIC platform.
    my $hello =
      "\x68\x65\x6c\x6c\x6f\x20\x77\x6f\x72\x6c\x64\x0a\x74\x68\x69\x73" .
      "\x20\x69\x73\x20\x61\x20\x74\x65\x73\x74\x0a\x73\x6f\x6d\x65\x20" .
      "\x6d\x6f\x72\x65\x20\x73\x74\x75\x66\x66\x20\x6f\x6e\x20\x74\x68" .
      "\x69\x73\x20\x6c\x69\x6e\x65\x0a\x61\x6e\x64\x20\x66\x69\x6e\x61" .
      "\x6c\x6c\x79\x2e\x2e\x2e\x0a" ;

    my $blocksize = 10 ;


    my ($info, $compressed) = mkComplete($CompressClass, $hello);

    my $header_size  = $info->{HeaderLength};
    my $trailer_size = $info->{TrailerLength};
    my $fingerprint_size = $info->{FingerprintLength};
    ok 1, "Compressed size is " . length($compressed) ;
    ok 1, "Fingerprint size is $fingerprint_size" ;
    ok 1, "Header size is $header_size" ;
    ok 1, "Trailer size is $trailer_size" ;

    foreach my $fb ( qw( filehandle buffer ) )
    {
        for my $trans ( 0 .. 1)
        {
            title "Truncating $CompressClass, Source $fb, Transparent $trans";


            foreach my $i (1 .. $fingerprint_size-1)
            {
                my $lex = LexFile->new( my $name );
                my $input;

                title "Fingerprint Truncation - length $i, Transparent $trans";

                my $part = substr($compressed, 0, $i);
                if ($fb eq 'filehandle')
                {
                    writeFile($name, $part);
                    $input = $name ;
                }
                else
                {
                    $input = \$part;
                }

                my $gz = $UncompressClass->can('new')->( $UncompressClass, $input,
                                              -BlockSize   => $blocksize,
                                              -Transparent => $trans );
                if ($trans) {
                    ok $gz;
                    ok ! $gz->error() ;
                    my $buff ;
                    is $gz->read($buff, 5000), length($part) ;
                    ok $buff eq $part ;
                    ok $gz->eof() ;
                    $gz->close();
                }
                else {
                    ok !$gz;
                }

            }

            #
            # Any header corruption past the fingerprint is considered catastrophic
            # so even if Transparent is set, it should still fail
            #
            foreach my $i ($fingerprint_size .. $header_size -1)
            {
                my $lex = LexFile->new( my $name );
                my $input;

                title "Header Truncation - length $i, Source $fb, Transparent $trans";

                my $part = substr($compressed, 0, $i);
                if ($fb eq 'filehandle')
                {
                    writeFile($name, $part);
                    $input = $name ;
                }
                else
                {
                    $input = \$part;
                }

                ok ! defined $UncompressClass->can('new')->( $UncompressClass, $input,
                                                  -BlockSize   => $blocksize,
                                                  -Transparent => $trans );
                #ok $gz->eof() ;
            }

            # Test corruption directly after the header
            # In this case the uncompression object will have been created,
            # so need to check that subsequent reads from the object fail
            if ($header_size > 0)
            {
                for my $mode (qw(block line para record slurp))
                {

                    title "Corruption after header - Mode $mode, Source $fb, Transparent $trans";

                    my $lex = LexFile->new( my $name );
                    my $input;

                    my $part = substr($compressed, 0, $header_size);
                    # Append corrupt data
                    $part .= "\xFF" x 100 ;
                    if ($fb eq 'filehandle')
                    {
                        writeFile($name, $part);
                        $input = $name ;
                    }
                    else
                    {
                        $input = \$part;
                    }

                    ok my $gz = $UncompressClass->can('new')->( $UncompressClass, $input,
                                                     -Strict      => 1,
                                                     -BlockSize   => $blocksize,
                                                     -Transparent => $trans )
                         or diag $$UnError;

                    my $un ;
                    my $status = 1;
                    if ($mode eq 'block')
                    {
                        $status = $gz->read($un) ;
                        is $status, -1, "got -1";
                    }
                    else
                    {
                        if ($mode eq 'line')
                        {
                            $status = <$gz>;
                        }
                        elsif ($mode eq 'para')
                        {
                            local $/ = "\n\n";
                            $status = <$gz>;
                        }
                        elsif ($mode eq 'record')
                        {
                            local $/ = \ 4;
                            $status = <$gz>;
                        }
                        elsif ($mode eq 'slurp')
                        {
                            local $/ ;
                            $status = <$gz>;
                        }

                        is $status, undef, "got undef";
                    }

                    ok $gz->error() ;
                    $gz->close();
                }
            }

            # Back to truncation tests

            foreach my $i ($header_size .. length($compressed) - 1 - $trailer_size)
            {
                next if $i == 0 ;

                for my $mode (qw(block line))
                {

                    title "Compressed Data Truncation - length $i, MOde $mode, Source $fb, Transparent $trans";

                    my $lex = LexFile->new( my $name );
                    my $input;

                    my $part = substr($compressed, 0, $i);
                    if ($fb eq 'filehandle')
                    {
                        writeFile($name, $part);
                        $input = $name ;
                    }
                    else
                    {
                        $input = \$part;
                    }

                    ok my $gz = $UncompressClass->can('new')->( $UncompressClass, $input,
                                                     -Strict      => 1,
                                                     -BlockSize   => $blocksize,
                                                     -Transparent => $trans )
                         or diag $$UnError;

                    my $un ;
                    if ($mode eq 'block')
                    {
                        my $status = 1 ;
                        $status = $gz->read($un) while $status > 0 ;
                        cmp_ok $status, "<", 0 ;
                    }
                    else
                    {
                        1 while <$gz> ;
                    }
                    ok $gz->error() ;
                    cmp_ok $gz->errorNo(), '<', 0 ;
                    # ok $gz->eof()
                    #     or die "EOF";
                    $gz->close();
                }
            }

            # RawDeflate and Zstandard do not have a trailer
            next if $CompressClass eq 'IO::Compress::RawDeflate' ;
            next if $CompressClass eq 'IO::Compress::Zstd' ;

            title "Compressed Trailer Truncation";
            foreach my $i (length($compressed) - $trailer_size .. length($compressed) -1 )
            {
                foreach my $lax (0, 1)
                {
                    my $lex = LexFile->new( my $name );
                    my $input;

                    ok 1, "Compressed Trailer Truncation - Length $i, Lax $lax, Transparent $trans" ;
                    my $part = substr($compressed, 0, $i);
                    if ($fb eq 'filehandle')
                    {
                        writeFile($name, $part);
                        $input = $name ;
                    }
                    else
                    {
                        $input = \$part;
                    }

                    ok my $gz = $UncompressClass->can('new')->( $UncompressClass, $input,
                                                     -BlockSize   => $blocksize,
                                                     -Strict      => !$lax,
                                                     -Append      => 1,
                                                     -Transparent => $trans );
                    my $un = '';
                    my $status = 1 ;
                    $status = $gz->read($un) while $status > 0 ;

                    if ($lax)
                    {
                        is $un, $hello;
                        is $status, 0
                            or diag "Status $status Error is " . $gz->error() ;
                        ok $gz->eof()
                            or diag "Status $status Error is " . $gz->error() ;
                        ok ! $gz->error() ;
                    }
                    else
                    {
                        cmp_ok $status, "<", 0
                            or diag "Status $status Error is " . $gz->error() ;
                        ok $gz->eof()
                            or diag "Status $status Error is " . $gz->error() ;
                        ok $gz->error() ;
                    }

                    $gz->close();
                }
            }
        }
    }
}

1;
