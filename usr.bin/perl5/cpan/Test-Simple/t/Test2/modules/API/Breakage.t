use strict;
use warnings;

if ($] lt "5.008") {
    print "1..0 # SKIP Test cannot run on perls below 5.8.0 because local doesn't work on hash keys.\n";
    exit 0;
}

use Test2::IPC;
use Test2::Tools::Tiny;
use Test2::API::Breakage;
my $CLASS = 'Test2::API::Breakage';

for my $meth (qw/upgrade_suggested upgrade_required known_broken/) {
    my @list = $CLASS->$meth;
    ok(!(@list % 2), "Got even list ($meth)");
    ok(!(grep {!defined($_)} @list), "No undefined items ($meth)");
}

{
    no warnings 'redefine';
    local *Test2::API::Breakage::upgrade_suggested = sub {
        return ('T2Test::UG1' => '1.0', 'T2Test::UG2' => '0.5');
    };

    local *Test2::API::Breakage::upgrade_required = sub {
        return ('T2Test::UR1' => '1.0', 'T2Test::UR2' => '0.5');
    };

    local *Test2::API::Breakage::known_broken = sub {
        return ('T2Test::KB1' => '1.0', 'T2Test::KB2' => '0.5');
    };
    use warnings 'redefine';

    ok(!$CLASS->report, "Nothing to report");
    ok(!$CLASS->report(1), "Still nothing to report");

    {
        local $INC{"T2Test/UG1.pm"} = "T2Test/UG1.pm";
        local $INC{"T2Test/UG2.pm"} = "T2Test/UG2.pm";
        local $INC{"T2Test/UR1.pm"} = "T2Test/UR1.pm";
        local $INC{"T2Test/UR2.pm"} = "T2Test/UR2.pm";
        local $INC{"T2Test/KB1.pm"} = "T2Test/KB1.pm";
        local $INC{"T2Test/KB2.pm"} = "T2Test/KB2.pm";

        local $T2Test::UG1::VERSION = '0.9';
        local $T2Test::UG2::VERSION = '0.9';
        local $T2Test::UR1::VERSION = '0.9';
        local $T2Test::UR2::VERSION = '0.9';
        local $T2Test::KB1::VERSION = '0.9';
        local $T2Test::KB2::VERSION = '0.9';

        my @report = $CLASS->report;

        $_ =~ s{\S+/Breakage\.pm}{Breakage.pm}g for @report;

        is_deeply(
            [sort @report],
            [
                sort
                " * Module 'T2Test::UR1' is outdated and known to be broken, please update to 1.0 or higher.",
                " * Module 'T2Test::KB1' is known to be broken in version 1.0 and below, newer versions have not been tested. You have: 0.9",
                " * Module 'T2Test::KB2' is known to be broken in version 0.5 and below, newer versions have not been tested. You have: 0.9",
                " * Module 'T2Test::UG1' is outdated, we recommed updating above 1.0. error was: 'T2Test::UG1 version 1.0 required--this is only version 0.9 at Breakage.pm line 75.'; INC is T2Test/UG1.pm",
            ],
            "Got expected report items"
        );
    }

    my %look;
    unshift @INC => sub {
        my ($this, $file) = @_;
        $look{$file}++ if $file =~ m{T2Test};
        return;
    };
    ok(!$CLASS->report, "Nothing to report");
    is_deeply(\%look, {}, "Did not try to load anything");

    ok(!$CLASS->report(1), "Nothing to report");
    is_deeply(
        \%look,
        {
            'T2Test/UG1.pm' => 1,
            'T2Test/UG2.pm' => 1,
            'T2Test/UR1.pm' => 1,
            'T2Test/UR2.pm' => 1,
            'T2Test/KB1.pm' => 1,
            'T2Test/KB2.pm' => 1,
        },
        "Tried to load modules"
    );
}

done_testing;
