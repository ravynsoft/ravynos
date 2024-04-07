BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib 't/compress';
use strict;
use warnings;

use IO::Uncompress::AnyUncompress qw($AnyUncompressError) ;

use IO::Compress::Zip     qw($ZipError) ;
use IO::Uncompress::Unzip qw($UnzipError) ;

sub getClass
{
    'AnyUncompress';
}


sub identify
{
    'IO::Compress::Zip';
}

require "any.pl" ;
run();
