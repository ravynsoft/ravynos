#!./perl

use FileCache;

END { unlink('foo_2arg') }

use Test::More tests => 1;

{# Test 4: that 2 arg format works, and that we cycle on mode change
     cacheout '>', "foo_2arg";
     print foo_2arg "foo 4\n";
     cacheout '+>', "foo_2arg";
     print foo_2arg "foo 44\n";
     seek(foo_2arg, 0, 0);
     ok(<foo_2arg> eq "foo 44\n");
     close foo_2arg;
}
