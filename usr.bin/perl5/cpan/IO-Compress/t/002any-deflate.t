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

use IO::Compress::Deflate   qw($DeflateError) ;
use IO::Uncompress::Inflate qw($InflateError) ;

sub getClass
{
    'AnyInflate';
}

sub identify
{
    'IO::Compress::Deflate';
}

require "any.pl" ;
run();
