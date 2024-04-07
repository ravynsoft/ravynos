BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;

use IO::Compress::RawDeflate   qw($RawDeflateError) ;
use IO::Uncompress::RawInflate qw($RawInflateError) ;

sub identify
{
    'IO::Compress::RawDeflate';
}

require "prime.pl" ;
run();
