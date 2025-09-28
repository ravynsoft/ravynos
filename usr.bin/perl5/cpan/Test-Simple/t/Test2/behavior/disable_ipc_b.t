use strict;
use warnings;

BEGIN { $ENV{T2_NO_IPC} = 1 };
use Test2::Tools::Tiny;
use Test2::IPC::Driver::Files;

ok(Test2::API::test2_ipc_disabled, "disabled IPC");
ok(!Test2::API::test2_ipc, "No IPC");

done_testing;
