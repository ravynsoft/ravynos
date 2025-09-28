use strict;
use warnings;
use Test::More;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use threads;

# test that the version documented in threads.pm pod matches
# that of the code.

open my $fh, "<", $INC{"threads.pm"}
    or die qq(Failed to open '$INC{"threads.pm"}': $!);
my $file= do { local $/; <$fh> };
close $fh;
my $pod_version = 0; 
if ($file=~/This document describes threads version (\d.\d+)/) {
    $pod_version = $1;
}
is($pod_version, $threads::VERSION, 
   "Check that pod and \$threads::VERSION match");
done_testing();


    
