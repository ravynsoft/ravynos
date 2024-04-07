
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

    plan tests => 49 + $extra ;
}



my $CompressClass   = identify();
my $UncompressClass = getInverse($CompressClass);
my $Error           = getErrorRef($CompressClass);
my $UnError         = getErrorRef($UncompressClass);

use Compress::Raw::Zlib;
use IO::Handle qw(SEEK_SET SEEK_CUR SEEK_END);

sub myGZreadFile
{
    my $filename = shift ;
    my $init = shift ;


    my $fil = $UncompressClass->can('new')->( $UncompressClass, $filename,
                                    -Strict   => 1,
                                    -Append   => 1
                                    );

    my $data = '';
    $data = $init if defined $init ;
    1 while $fil->read($data) > 0;

    $fil->close ;
    return $data ;
}


{

    title "Testing $CompressClass Errors";

}


{
    title "Testing $UncompressClass Errors";

}

{
    title "Testing $CompressClass and $UncompressClass";

    {
        title "flush" ;


        my $lex = LexFile->new( my $name );

        my $hello = <<EOM ;
hello world
this is a test
EOM

        {
          my $x ;
          ok $x = $CompressClass->can('new')->( $CompressClass, $name );

          ok $x->write($hello), "write" ;
          ok $x->flush(Z_FINISH), "flush";
          ok $x->close, "close" ;
        }

        {
          my $uncomp;
          ok my $x = $UncompressClass->can('new')->( $UncompressClass, $name, -Append => 1 );

          my $len ;
          1 while ($len = $x->read($uncomp)) > 0 ;

          is $len, 0, "read returned 0";

          ok $x->close ;
          is $uncomp, $hello ;
        }
    }


    if ($CompressClass ne 'RawDeflate')
    {
        # write empty file
        #========================================

        my $buffer = '';
        {
          my $x ;
          ok $x = $CompressClass->can('new')->( $CompressClass, \$buffer);
          ok $x->close ;

        }

        my $keep = $buffer ;
        my $uncomp= '';
        {
          my $x ;
          ok $x = $UncompressClass->can('new')->( $UncompressClass, \$buffer, Append => 1)  ;

          1 while $x->read($uncomp) > 0  ;

          ok $x->close ;
        }

        ok $uncomp eq '' ;
        ok $buffer eq $keep ;

    }


    {
        title "inflateSync on plain file";

        my $hello = "I am a HAL 9000 computer" x 2001 ;

        my $k = $UncompressClass->can('new')->( $UncompressClass, \$hello, Transparent => 1);
        ok $k ;

        # Skip to the flush point -- no-op for plain file
        my $status = $k->inflateSync();
        is $status, 1
            or diag $k->error() ;

        my $rest;
        is $k->read($rest, length($hello)), length($hello)
            or diag $k->error() ;
        ok $rest eq $hello ;

        ok $k->close();
    }

    {
        title "$CompressClass: inflateSync for real";

        # create a deflate stream with flush points

        my $hello = "I am a HAL 9000 computer" x 2001 ;
        my $goodbye = "Will I dream?" x 2010;
        my ($x, $err, $answer, $X, $Z, $status);
        my $Answer ;

        ok ($x = $CompressClass->can('new')->( $CompressClass, \$Answer));
        ok $x ;

        is $x->write($hello), length($hello);

        # create a flush point
        ok $x->flush(Z_FULL_FLUSH) ;

        is $x->write($goodbye), length($goodbye);

        ok $x->close() ;

        my $k;
        $k = $UncompressClass->can('new')->( $UncompressClass, \$Answer, BlockSize => 1);
        ok $k ;

        my $initial;
        is $k->read($initial, 1), 1 ;
        is $initial, substr($hello, 0, 1);

        # Skip to the flush point
        $status = $k->inflateSync();
        is $status, 1, "   inflateSync returned 1"
            or diag $k->error() ;

        my $rest;
        is $k->read($rest, length($hello) + length($goodbye)),
                length($goodbye)
            or diag $k->error() ;
        ok $rest eq $goodbye, " got expected output" ;

        ok $k->close();
    }

    {
        title "$CompressClass: inflateSync no FLUSH point";

        # create a deflate stream with flush points

        my $hello = "I am a HAL 9000 computer" x 2001 ;
        my ($x, $err, $answer, $X, $Z, $status);
        my $Answer ;

        ok ($x = $CompressClass->can('new')->( $CompressClass, \$Answer));
        ok $x ;

        is $x->write($hello), length($hello);

        ok $x->close() ;

        my $k = $UncompressClass->can('new')->( $UncompressClass, \$Answer, BlockSize => 1);
        ok $k ;

        my $initial;
        is $k->read($initial, 1), 1 ;
        is $initial, substr($hello, 0, 1);

        # Skip to the flush point
        $status = $k->inflateSync();
        is $status, 0
            or diag $k->error() ;

        ok $k->close();
        is $k->inflateSync(), 0 ;
    }

}


1;
