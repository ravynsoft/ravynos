use strict;
use warnings;

use Test2::Util qw/CAN_THREAD/;
use Test2::API qw/context/;

BEGIN {
    sub plan {
        my $ctx = context();
        $ctx->plan(@_);
        $ctx->release;
    }

    unless (CAN_THREAD()) {
        plan(0, skip_all => 'System does not have threads');
        exit 0;
    }
}

use threads;
no Test2::IPC;
use Test::More;

ok(Test2::API::test2_ipc_disabled, "disabled IPC");
ok(!Test2::API::test2_ipc, "No IPC");

done_testing;
