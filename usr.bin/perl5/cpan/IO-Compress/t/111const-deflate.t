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

    plan tests => 390 + $extra ;
}


{
    use Compress::Raw::Zlib ;

    my %all;
    for my $symbol (@Compress::Raw::Zlib::DEFLATE_CONSTANTS)
    {
        eval "defined Compress::Raw::Zlib::$symbol" ;
        $all{$symbol} = ! $@ ;
    }

    my $pkg = 1;

    for my $module ( qw( Adapter::Deflate RawDeflate Deflate Gzip Zip ))
    {
        ++ $pkg ;
        eval <<EOM;
            package P$pkg;
            use Test::More ;
            use CompTestUtils;

            use IO::Compress::$module () ;

            ::title "IO::Compress::$module - no import" ;
EOM
        is $@, "", "create package P$pkg";
        for my $symbol (@Compress::Raw::Zlib::DEFLATE_CONSTANTS)
        {
            if ( $all{$symbol})
            {
                eval "package P$pkg; defined IO::Compress::${module}::$symbol ;";
                is $@, "", "  has $symbol";
            }
            else
            {
                ok 1, "  $symbol not available";
            }
        }
    }

    for my $module ( qw( Adapter::Deflate RawDeflate Deflate Gzip Zip ))
    {
        for my $label (keys %Compress::Raw::Zlib::DEFLATE_CONSTANTS)
        {
            ++ $pkg ;

            eval <<EOM;
                package P$pkg;
                use Test::More ;
                use CompTestUtils;

                use IO::Compress::$module qw(:$label) ;

                ::title "IO::Compress::$module - import :$label" ;

EOM
            is $@, "", "create package P$pkg";

            for my $symbol (@{ $Compress::Raw::Zlib::DEFLATE_CONSTANTS{$label} } )
            {
                if ( $all{$symbol})
                {
                    eval "package P$pkg; defined $symbol ;";
                    is $@, "", "  has $symbol";
                }
                else
                {
                    ok 1, "  $symbol not available";
                }
            }
        }
    }

}
