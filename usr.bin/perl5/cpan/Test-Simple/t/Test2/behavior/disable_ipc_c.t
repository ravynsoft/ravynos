use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API qw/test2_ipc_disable/;
BEGIN { test2_ipc_disable() }
use Test2::IPC::Driver::Files;

ok(Test2::API::test2_ipc_disabled, "disabled IPC");
ok(!Test2::API::test2_ipc, "No IPC");

done_testing;
