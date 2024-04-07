use strict;
use warnings;
use IO::Handle;

STDOUT->autoflush(1);
STDERR->autoflush(1);

my $max = shift || 4;
for ( 1..$max ) {
    $_ % 2 
        ? print STDOUT $_
        : print STDERR $_;
}
