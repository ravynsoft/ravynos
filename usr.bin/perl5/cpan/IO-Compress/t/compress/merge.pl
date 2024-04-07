use lib 't';
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

use Compress::Raw::Zlib 2 ;

BEGIN
{
    plan(skip_all => "Merge needs Zlib 1.2.1 or better - you have Zlib "
                . Compress::Raw::Zlib::zlib_version())
        if ZLIB_VERNUM() < 0x1210 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 165 + $extra ;

}


sub run
{

    my $CompressClass   = identify();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);

    # Tests
    #   destination is a file that doesn't exist -- should work ok unless AnyDeflate
    #   destination isn't compressed at all
    #   destination is compressed but wrong format
    #   destination is corrupt - error messages should be correct
    #   use apend mode with old zlib - check that this is trapped
    #   destination is not seekable, readable, writable - test for filename & handle

    {
        title "Misc error cases";

        eval { Compress::Raw::Zlib::InflateScan->new( Bufsize => 0 ) } ;
        like $@, mkErr("^Compress::Raw::Zlib::InflateScan::new: Bufsize must be >= 1, you specified 0"), "  catch bufsize == 0";

        eval { Compress::Raw::Zlib::inflateScanStream::createDeflateStream(undef, Bufsize => 0) } ;
        like $@, mkErr("^Compress::Raw::Zlib::InflateScan::createDeflateStream: Bufsize must be >= 1, you specified 0"), "  catch bufsize == 0";

    }

    # output file/handle not writable
    {

        foreach my $to_file (0,1)
        {
            if ($to_file)
              { title "$CompressClass - Merge to filename that isn't writable" }
            else
              { title "$CompressClass - Merge to filehandle that isn't writable" }

            my $lex = LexFile->new( my $out_file );

            # create empty file
            open F, ">$out_file" ; print F "x"; close F;
            ok   -e $out_file, "  file exists" ;
            ok  !-z $out_file, "  and is not empty" ;

            # make unwritable
            is chmod(0444, $out_file), 1, "  chmod worked" ;
            ok   -e $out_file, "  still exists after chmod" ;

            SKIP:
            {
                skip "Cannot create non-writable file", 3
                    if -w $out_file ;

                ok ! -w $out_file, "  chmod made file unwritable" ;

                my $dest ;
                if ($to_file)
                  { $dest = $out_file }
                else
                  { $dest = IO::File->new( "<$out_file" ) }

                my $gz = $CompressClass->new($dest, Merge => 1) ;

                ok ! $gz, "  Did not create $CompressClass object";

                ok $$Error, "  Got error message" ;
            }

            chmod 0777, $out_file ;
        }
    }

    # output is not compressed at all
    {

        my $lex = LexFile->new( my $out_file );

        foreach my $to_file ( qw(buffer file handle ) )
        {
            title "$CompressClass to $to_file, content is not compressed";

            my $content = "abc" x 300 ;
            my $buffer ;
            my $disp_content = defined $content ? $content : '<undef>' ;
            my $str_content = defined $content ? $content : '' ;

            if ($to_file eq 'buffer')
            {
                $buffer = \$content ;
            }
            else
            {
                writeFile($out_file, $content);

                if ($to_file eq 'handle')
                {
                    $buffer = IO::File->new( "+<$out_file" )
                        or die "# Cannot open $out_file: $!";
                }
                else
                  { $buffer = $out_file }
            }

            ok ! $CompressClass->new($buffer, Merge => 1), "  constructor fails";
            {
                like $$Error, '/Cannot create InflateScan object: (Header Error|unexpected end of file|Inflation Error: data error)?/', "  got Bad Magic" ;
            }

        }
    }

    # output is empty
    {

        my $lex = LexFile->new( my $out_file );

        foreach my $to_file ( qw(buffer file handle ) )
        {
            title "$CompressClass to $to_file, content is empty";

            my $content = '';
            my $buffer ;
            my $dest ;

            if ($to_file eq 'buffer')
            {
                $dest = $buffer = \$content ;
            }
            else
            {
                writeFile($out_file, $content);
                $dest = $out_file;

                if ($to_file eq 'handle')
                {
                    $buffer = IO::File->new( "+<$out_file" )
                        or die "# Cannot open $out_file: $!";
                }
                else
                  { $buffer = $out_file }
            }

            ok my $gz = $CompressClass->new($buffer, Merge => 1, AutoClose => 1), "  constructor passes"
                or diag $$Error;

            $gz->write("FGHI");
            $gz->close();

            #hexDump($buffer);
            my $out = anyUncompress($dest);

            is $out, "FGHI", '  Merge OK';
        }
    }

    {
        title "$CompressClass - Merge to file that doesn't exist";

        my $lex = LexFile->new( my $out_file );

        ok ! -e $out_file, "  Destination file, '$out_file', does not exist";

        ok my $gz1 = $CompressClass->can('new')->( $CompressClass, $out_file, Merge => 1)
            or die "# $CompressClass->new(...) failed: $$Error\n";
        #hexDump($buffer);
        $gz1->write("FGHI");
        $gz1->close();

        #hexDump($buffer);
        my $out = anyUncompress($out_file);

        is $out, "FGHI", '  Merged OK';
    }

    {

        my $lex = LexFile->new( my $out_file );

        foreach my $to_file ( qw( buffer file handle ) )
        {
            foreach my $content (undef, '', 'x', 'abcde')
            {
                #next if ! defined $content && $to_file;

                my $buffer ;
                my $disp_content = defined $content ? $content : '<undef>' ;
                my $str_content = defined $content ? $content : '' ;

                if ($to_file eq 'buffer')
                {
                    my $x ;
                    $buffer = \$x ;
                    title "$CompressClass to Buffer, content is '$disp_content'";
                }
                else
                {
                    $buffer = $out_file ;
                    if ($to_file eq 'handle')
                    {
                        title "$CompressClass to Filehandle, content is '$disp_content'";
                    }
                    else
                    {
                        title "$CompressClass to File, content is '$disp_content'";
                    }
                }

                my $gz = $CompressClass->new($buffer);
                my $len = defined $content ? length($content) : 0 ;
                is $gz->write($content), $len, "  write ok";
                ok $gz->close(), " close ok";

                #hexDump($buffer);
                is anyUncompress($buffer), $str_content, '  Destination is ok';

                #if ($corruption)
                #{
                    #    next if $TopTypes eq 'RawDeflate' && $content eq '';
                    #
                    #}

                my $dest = $buffer ;
                if ($to_file eq 'handle')
                {
                    $dest = IO::File->new( "+<$buffer" );
                }

                my $gz1 = $CompressClass->new($dest, Merge => 1, AutoClose => 1)
                    or die "## Error is  $$Error\n";

                #print "YYY\n";
                #hexDump($buffer);
                #print "XXX\n";
                is $gz1->write("FGHI"), 4, "  write returned 4";
                ok $gz1->close(), "  close ok";

                #hexDump($buffer);
                my $out = anyUncompress($buffer);

                is $out, $str_content . "FGHI", '  Merged OK';
                #exit;
            }
        }

    }



    {
        my $Func = getTopFuncRef($CompressClass);
        my $TopType = getTopFuncName($CompressClass);

        my $buffer ;

        my $lex = LexFile->new( my $out_file );

        foreach my $to_file (0, 1)
        {
            foreach my $content (undef, '', 'x', 'abcde')
            {
                my $disp_content = defined $content ? $content : '<undef>' ;
                my $str_content = defined $content ? $content : '' ;
                my $buffer ;
                if ($to_file)
                {
                    $buffer = $out_file ;
                    title "$TopType to File, content is '$disp_content'";
                }
                else
                {
                    my $x = '';
                    $buffer = \$x ;
                    title "$TopType to Buffer, content is '$disp_content'";
                }


                ok $Func->(\$content, $buffer), " Compress content";
                #hexDump($buffer);
                is anyUncompress($buffer), $str_content, '  Destination is ok';


                ok $Func->(\"FGHI", $buffer, Merge => 1), "  Merge content";

                #hexDump($buffer);
                my $out = anyUncompress($buffer);

                is $out, $str_content . "FGHI", '  Merged OK';
            }
        }

    }

}


1;
