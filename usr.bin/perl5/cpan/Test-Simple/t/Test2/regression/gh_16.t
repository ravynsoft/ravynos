use strict;
use warnings;

# This test checks for a pretty rare condition, one that was mainly a problem
# on 5.20+ (though a 5.8 also had the problem). I am not too worried about this
# breaking again. That said I still want it run on newer perls (where it is
# less likely to fail for an unrelated reason) and when I have AUTHOR_TESTING
# set.
BEGIN {
    unless($ENV{AUTHOR_TESTING} || eval "no warnings 'portable'; require 5.20; 1") {
        print "1..0 # Skip Crazy test, only run on 5.20+, or when AUTHOR_TESTING is set\n";
        exit 0;
    }
}

# This test is for gh #16
# Also see https://rt.perl.org/Public/Bug/Display.html?id=127774

# Ceate this END before anything else so that $? gets set to 0
END { $? = 0 }

BEGIN {
    print "\n1..1\n";
    close(STDERR);
    open(STDERR, '>&STDOUT');
}

use Test2::API;

eval(' sub { die "xxx" } ')->();
END {
    sub { my $ctx = Test2::API::context(); $ctx->release; }->();
    print "ok 1 - Did not segv\n";
    $? = 0;
}
