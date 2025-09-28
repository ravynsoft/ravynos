use strict;
use warnings;
use Test2::IPC;
use Test2::Tools::Tiny;
use Test2::API::Stack;
use Test2::API qw/test2_ipc/;

ok(my $stack = Test2::API::Stack->new, "Create a stack");

ok(!@$stack, "Empty stack");
ok(!$stack->peek, "Nothing to peek at");

ok(!exception { $stack->cull },  "cull lives when stack is empty");
ok(!exception { $stack->all },   "all lives when stack is empty");
ok(!exception { $stack->clear }, "clear lives when stack is empty");

like(
    exception { $stack->pop(Test2::Hub->new) },
    qr/No hubs on the stack/,
    "No hub to pop"
);

my $hub = Test2::Hub->new;
ok($stack->push($hub), "pushed a hub");

like(
    exception { $stack->pop($hub) },
    qr/You cannot pop the root hub/,
    "Root hub cannot be popped"
);

$stack->push($hub);
like(
    exception { $stack->pop(Test2::Hub->new) },
    qr/Hub stack mismatch, attempted to pop incorrect hub/,
    "Must specify correct hub to pop"
);

is_deeply(
    [ $stack->all ],
    [ $hub, $hub ],
    "Got all hubs"
);

ok(!exception { $stack->pop($hub) }, "Popped the correct hub");

is_deeply(
    [ $stack->all ],
    [ $hub ],
    "Got all hubs"
);

is($stack->peek, $hub, "got the hub");
is($stack->top, $hub, "got the hub");

$stack->clear;

is_deeply(
    [ $stack->all ],
    [ ],
    "no hubs"
);

ok(my $top = $stack->top, "Generated a top hub");
is($top->ipc, test2_ipc, "Used sync's ipc");
ok($top->format, 'Got formatter');

is($stack->top, $stack->top, "do not generate a new top if there is already a top");

ok(my $new = $stack->new_hub(), "Add a new hub");
is($stack->top, $new, "new one is on top");
is($new->ipc, $top->ipc, "inherited ipc");
is($new->format, $top->format, "inherited formatter");

my $new2 = $stack->new_hub(formatter => undef, ipc => undef);
ok(!$new2->ipc, "built with no ipc");
ok(!$new2->format, "built with no formatter");

done_testing;
