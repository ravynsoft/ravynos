use Test2::API qw(run_subtest context intercept);
use Test::More;
use Test2::Tools::Tiny qw/todo/;

sub aaa {
    my $ctx = context();
    run_subtest(
        "bad pass",
        sub {
            local $TODO = "third test";
            ok(1, "ok");
        }
    );
    $ctx->release;
}

sub bbb {
    my $ctx = context();
    run_subtest(
        "bad fail",
        sub {
            local $TODO = "fourth test";
            ok(0, "ok");
        }
    );

    $ctx->release;
}

my $events = intercept {
    Test::Builder->new->_add_ts_hooks();
    aaa();
    bbb();
};

is_deeply(
    $events->[1]->{subevents}->[0]->{amnesty}->[0],
    { tag => 'TODO', details => "third test" },
    "Amnesty was set properly for first subtest assertion",
);

is_deeply(
    $events->[3]->{subevents}->[0]->{amnesty}->[0],
    { tag => 'TODO', details => "fourth test" },
    "Amnesty was set properly for second subtest assertion",
);

done_testing;
