BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use IO::Uncompress::AnyInflate qw($AnyInflateError) ;

use IO::Compress::Zip     qw($ZipError) ;
use IO::Uncompress::Unzip qw($UnzipError) ;

sub getClass
{
    'AnyInflate';
}


sub identify
{
    'IO::Compress::Zip';
}

require "any.pl" ;
run();
