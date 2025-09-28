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

use IO::Compress::Gzip     qw($GzipError) ;
use IO::Uncompress::Gunzip qw($GunzipError) ;

sub getClass
{
    'AnyInflate';
}


sub identify
{
    'IO::Compress::Gzip';
}

require "any.pl" ;
run();
