use strict;
use warnings;

use Test2::IPC::Driver::Files;

use Test2::Tools::Tiny;
use Test2::API qw/context test2_ipc_drivers/;

Test2::IPC::Driver::Files->import();
Test2::IPC::Driver::Files->import();
Test2::IPC::Driver::Files->import();

is_deeply(
    [test2_ipc_drivers()],
    ['Test2::IPC::Driver::Files'],
    "Driver not added multiple times"
);

for my $meth (qw/send cull add_hub drop_hub waiting is_viable/) {
    my $one = Test2::IPC::Driver->new;
    like(
        exception { $one->$meth },
        qr/'\Q$one\E' did not define the required method '$meth'/,
        "Require override of method $meth"
    );
}

SKIP: {
    last SKIP if $] lt "5.008";
tests abort => sub {
    my $one = Test2::IPC::Driver->new(no_fatal => 1);
    my ($err, $out) = ("", "");

    {
        local *STDERR;
        local *STDOUT;
        open(STDERR, '>', \$err);
        open(STDOUT, '>', \$out);
        $one->abort('foo');
    }

    is($err, "IPC Fatal Error: foo\n", "Got error");
    is($out, "Bail out! IPC Fatal Error: foo\n", "got 'bail-out' on stdout");

    ($err, $out) = ("", "");

    {
        local *STDERR;
        local *STDOUT;
        open(STDERR, '>', \$err);
        open(STDOUT, '>', \$out);
        $one->abort_trace('foo');
    }

    like($out, qr/Bail out! IPC Fatal Error: foo/, "got 'bail-out' on stdout");
    like($err, qr/IPC Fatal Error: foo/, "Got error");
};
}

done_testing;
