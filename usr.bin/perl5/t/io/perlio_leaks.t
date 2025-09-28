#!perl
# ioleaks.t

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;
plan 'no_plan';

# :unix   -> not ok
# :stdio  -> not ok
# :perlio -> ok
# :crlf   -> ok

TODO: {
    foreach my $layer(qw(:unix :stdio  :perlio :crlf)){
        my $base_fd = do{ open my $in, '<', $0 or die $!; fileno $in };

        for(1 .. 3){
	    local $::TODO;
	    if ($_ > 1 && $layer =~ /^:(unix|stdio)$/) {
		$::TODO = "[perl #56644] PerlIO resource leaks on open() and then :pop in :unix and :stdio"
	    }
	    open my $fh, "<$layer", $0 or die $!;

	    is fileno($fh), $base_fd, $layer;
	    binmode $fh, ':pop';
        }
    }
}

