BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use Test::More skip_all => "not implemented yet";


use IO::Compress::Zip     qw($ZipError) ;
use IO::Uncompress::Unzip qw($UnzipError) ;

sub identify
{
    'IO::Compress::Zip';
}

require "merge.pl" ;
run();
