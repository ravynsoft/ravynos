use strict;
use warnings;

use Test2::IPC;

use Test2::Tools::Tiny;

{
    local $! = 100;

    is(0 + $!, 100, 'set $!');
    is(0 + $!, 100, 'preserved $!');
}

done_testing;
