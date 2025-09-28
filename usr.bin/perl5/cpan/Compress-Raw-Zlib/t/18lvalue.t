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

BEGIN
{
    plan(skip_all => "lvalue sub tests need Perl ??")
        if $] < 5.006 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 10 + $extra ;

    use_ok('Compress::Raw::Zlib', 2) ;
}

use CompTestUtils;



my $hello = <<EOM ;
hello world
this is a test
EOM

my $len   = length $hello ;

# Check zlib_version and ZLIB_VERSION are the same.
test_zlib_header_matches_library();

{
    title 'deflate/inflate with lvalue sub';

    my $hello = "I am a HAL 9000 computer" ;
    my $data = $hello ;

    my($X, $Z);
    sub getData : lvalue { $data }
    sub getX    : lvalue { $X }
    sub getZ    : lvalue { $Z }

    ok my $x = new Compress::Raw::Zlib::Deflate ( -AppendOutput => 1 );

    cmp_ok $x->deflate(getData, getX), '==',  Z_OK ;

    cmp_ok $x->flush(getX), '==', Z_OK ;

    my $append = "Appended" ;
    $X .= $append ;

    ok my $k = new Compress::Raw::Zlib::Inflate ( -AppendOutput => 1 ) ;

    cmp_ok $k->inflate(getX, getZ), '==', Z_STREAM_END ; ;

    ok $hello eq $Z ;
    is $X, $append;

}
