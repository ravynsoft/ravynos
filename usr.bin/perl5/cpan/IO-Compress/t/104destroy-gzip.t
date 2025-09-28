BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use IO::Compress::Gzip     qw($GzipError) ;
use IO::Uncompress::Gunzip qw($GunzipError) ;

sub identify
{
    'IO::Compress::Gzip';
}

require "destroy.pl" ;
run();
