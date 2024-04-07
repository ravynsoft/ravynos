BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use Test::More ;

BEGIN {
    plan skip_all => "Lengthy Tests Disabled\n" .
                     "set COMPRESS_ZLIB_RUN_ALL or COMPRESS_ZLIB_RUN_MOST to run this test suite"
        unless defined $ENV{COMPRESS_ZLIB_RUN_ALL} or defined $ENV{COMPRESS_ZLIB_RUN_MOST};

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 3308 + $extra;

};


#use Test::More skip_all => "not implemented yet";


use IO::Compress::Bzip2   qw($Bzip2Error) ;
use IO::Uncompress::Bunzip2 qw($Bunzip2Error) ;

sub identify
{
    'IO::Compress::Bzip2';
}

require "truncate.pl" ;
run();
