
use lib 't';

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

    plan tests => 36 + $extra ;
}

sub run
{
    my $CompressClass   = identify();
    my $AnyClass        = getClass();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);

    my $AnyConstruct = "IO::Uncompress::${AnyClass}" ;
    no strict refs;
    my $AnyError = \${ "IO::Uncompress::${AnyClass}::${AnyClass}Error" };

    for my $trans ( 0, 1 )
    {
        for my $file ( 0, 1 )
        {
            title "$AnyClass(Transparent => $trans, File=>$file) with $CompressClass" ;
            my $string = "some text" x 100 ;

            my $buffer ;
            my $x = $CompressClass->can('new')->( $CompressClass, \$buffer) ;
            ok $x, "  create $CompressClass object" ;
            ok $x->write($string), "  write to object" ;
            ok $x->close, "  close ok" ;

            my $lex = LexFile->new( my $output );
            my $input ;

            if ($file) {
                writeFile($output, $buffer);
                $input = $output;
            }
            else {
                $input = \$buffer;
            }

            {
                my $unc = $AnyConstruct->can('new')->( $AnyConstruct, $input, Transparent => $trans
                                                    Append => 1 );

                ok $unc, "  Created $AnyClass object"
                    or print "# $$AnyError\n";
                my $uncomp ;
                1 while $unc->read($uncomp) > 0 ;
                #ok $unc->read($uncomp) > 0
                #    or print "# $$AnyError\n";
                my $y;
                is $unc->read($y, 1), 0, "  at eof" ;
                ok $unc->eof(), "  at eof" ;
                #ok $unc->type eq $Type;

                is $uncomp, $string, "  expected output" ;
            }

            {
                my $unc = $AnyConstruct->can('new')->( $AnyConstruct, $input, Transparent => $trans,
                                                     Append =>1 );

                ok $unc, "  Created $AnyClass object"
                    or print "# $$AnyError\n";
                my $uncomp ;
                1 while $unc->read($uncomp, 10) > 0 ;
                my $y;
                is $unc->read($y, 1), 0, "  at eof" ;
                ok $unc->eof(), "  at eof" ;
                #ok $unc->type eq $Type;

                is $uncomp, $string, "  expected output" ;
            }
        }
    }
}

1;
