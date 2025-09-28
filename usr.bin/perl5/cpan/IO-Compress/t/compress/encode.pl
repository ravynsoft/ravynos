
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

    my $st = eval { require Test::NoWarnings ;  import Test::NoWarnings; 1; };
    $extra = 1
        if $st ;

    plan(tests => 29 + $extra) ;
}

sub run
{
    my $CompressClass   = identify();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);


    my $string = "\x{df}\x{100}\x80";
    my $encString = Encode::encode_utf8($string);
    my $buffer = $encString;

    #for my $from ( qw(filename filehandle buffer) )
    {
#        my $input ;
#        my $lex = LexFile->new( my $name );
#
#
#        if ($from eq 'buffer')
#          { $input = \$buffer }
#        elsif ($from eq 'filename')
#        {
#            $input = $name ;
#            writeFile($name, $buffer);
#        }
#        elsif ($from eq 'filehandle')
#        {
#            $input = IO::File->new( "<$name" );
#        }

        for my $to ( qw(filehandle buffer))
        {
            title "OO Mode: To $to, Encode by hand";

            my $lex2 = LexFile->new( my $name2 );
            my $output;
            my $buffer;

            if ($to eq 'buffer')
              { $output = \$buffer }
            elsif ($to eq 'filename')
            {
                $output = $name2 ;
            }
            elsif ($to eq 'filehandle')
            {
                $output = IO::File->new( ">$name2" );
            }


            my $out ;
            my $cs = $CompressClass->can('new')->( $CompressClass, $output, AutoClose =>1);
            $cs->print($encString);
            $cs->close();

            my $input;
            if ($to eq 'buffer')
              { $input = \$buffer }
            else
            {
                $input = $name2 ;
            }

            my $ucs = $UncompressClass->can('new')->( $UncompressClass, $input, Append => 1);
            my $got;
            1 while $ucs->read($got) > 0 ;

            is  $got, $encString, "  Expected output";

            my $decode = Encode::decode_utf8($got);


            is $decode, $string, "  Expected output";


        }
    }

    {
        title "Catch wide characters";

        my $out;
        my $cs = $CompressClass->can('new')->( $CompressClass, \$out);
        my $a = "a\xFF\x{100}";
        eval { $cs->syswrite($a) };
        like($@, qr/Wide character in ${CompressClass}::write/,
                 "  wide characters in ${CompressClass}::write");

    }

    {
        title "Unknown encoding";
        my $output;
        eval { my $cs = $CompressClass->can('new')->( $CompressClass, \$output, Encode => 'fred'); } ;
        like($@, qr/${CompressClass}: Encoding 'fred' is not available/,
                 "  Encoding 'fred' is not available");
    }

    {
        title "Encode option";

        for my $to ( qw(filehandle filename buffer))
        {
            title "Encode: To $to, Encode option";

            my $lex2 = LexFile->new( my $name2 );
            my $output;
            my $buffer;

            if ($to eq 'buffer')
            {
                $output = \$buffer
            }
            elsif ($to eq 'filename')
            {
                $output = $name2 ;
            }
            elsif ($to eq 'filehandle')
            {
                $output = IO::File->new( ">$name2" );
            }

            my $out ;
            my $cs = $CompressClass->can('new')->( $CompressClass, $output, AutoClose =>1, Encode => 'utf8');
            ok $cs->print($string);
            ok $cs->close();

            my $input;
            if ($to eq 'buffer')
            {
                $input = \$buffer
            }
            elsif ($to eq 'filename')
            {
                $input = $name2 ;
            }
            else
            {
                $input = IO::File->new( "<$name2" );
            }

            {
                my $ucs = $UncompressClass->can('new')->( $UncompressClass, $input, AutoClose =>1, Append => 1);
                my $got;
                1 while $ucs->read($got) > 0 ;
                ok length($got) > 0;
                is  $got, $encString, "  Expected output";

                my $decode = Encode::decode_utf8($got);

                is  $decode, $string, "  Expected output";
            }


#            {
#                my $ucs = $UncompressClass->can('new')->( $UncompressClass, $input, Append => 1, Decode => 'utf8');
#                my $got;
#                1 while $ucs->read($got) > 0 ;
#                ok length($got) > 0;
#                is  $got, $string, "  Expected output";
#            }
        }
    }

}



1;
