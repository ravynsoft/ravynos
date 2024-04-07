BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 't' if -d 't';
        @INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict ;
use warnings ;

use Test::More ;

sub gotScalarUtilXS
{
    eval ' use Scalar::Util "dualvar" ';
    return $@ ? 0 : 1 ;
}

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };


    my $VERSION = '2.204';
    my @NAMES = qw(
			Compress::Raw::Bzip2
			Compress::Raw::Zlib

			Compress::Zlib

            IO::Compress::Adapter::Bzip2
            IO::Compress::Adapter::Deflate
            IO::Compress::Adapter::Identity
            IO::Compress::Base::Common
            IO::Compress::Base
            IO::Compress::Bzip2
            IO::Compress::Deflate
            IO::Compress::Gzip::Constants
            IO::Compress::Gzip
            IO::Compress::RawDeflate
            IO::Compress::Zip::Constants
            IO::Compress::Zip
            IO::Compress::Zlib::Constants
            IO::Compress::Zlib::Extra
            IO::Uncompress::Adapter::Bunzip2
            IO::Uncompress::Adapter::Identity
            IO::Uncompress::Adapter::Inflate
            IO::Uncompress::AnyInflate
            IO::Uncompress::AnyUncompress
            IO::Uncompress::Base
            IO::Uncompress::Bunzip2
            IO::Uncompress::Gunzip
            IO::Uncompress::Inflate
            IO::Uncompress::RawInflate
            IO::Uncompress::Unzip

			);

    my @OPT = qw(
			);

    plan tests => 1 + 2 + @NAMES + @OPT + $extra ;

    foreach my $name (@NAMES)
    {
        use_ok($name, $VERSION);
    }


    foreach my $name (@OPT)
    {
        eval " require $name " ;
        if ($@)
        {
            ok 1, "$name not available"
        }
        else
        {
            my $ver = eval("\$${name}::VERSION");
            is $ver, $VERSION, "$name version should be $VERSION"
                or diag "$name version is $ver, need $VERSION" ;
        }
    }

    # need zlib 1.2.0 or better or zlib-ng

    ok Compress::Raw::Zlib::is_zlibng() || Compress::Raw::Zlib::ZLIB_VERNUM() >= 0x1200
        or diag "IO::Compress needs zlib 1.2.0 or better, you have " . Compress::Raw::Zlib::zlib_version();

    use_ok('Scalar::Util') ;

}

ok gotScalarUtilXS(), "Got XS Version of Scalar::Util"
    or diag <<EOM;
You don't have the XS version of Scalar::Util
EOM
