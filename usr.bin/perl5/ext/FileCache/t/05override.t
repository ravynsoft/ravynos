#!./perl

use FileCache;

END { unlink("Foo_Bar_ov") }

use Test::More tests => 1;

{# Test 5: that close is overridden properly within the caller
     cacheout local $_ = "Foo_Bar_ov";
     print $_ "Hello World\n";
     close($_);
     ok(!fileno($_));
}
