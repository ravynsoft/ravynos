use strict;
use warnings;
# HARNESS-NO-PRELOAD
# HARNESS-NO-STREAM

my $CLASS;
my %BEFORE_LOAD;

BEGIN {
    my $old = select STDOUT;
    $BEFORE_LOAD{STDOUT} = $|;
    select STDERR;
    $BEFORE_LOAD{STDERR} = $|;
    select $old;

    require Test2::Formatter::TAP;
    $CLASS   = 'Test2::Formatter::TAP';
    *OUT_STD = $CLASS->can('OUT_STD') or die "Could not get OUT_STD constant";
    *OUT_ERR = $CLASS->can('OUT_ERR') or die "Could not get OUT_ERR constant";
}

use Test2::Tools::Tiny;
use Test2::API qw/context/;

BEGIN {
    eval {
        require PerlIO;
        PerlIO->VERSION(1.02);    # required for PerlIO::get_layers
    } or do {
        print "1..0 # SKIP Don't have PerlIO 1.02\n";
        exit 0;
    }
}

sub grabber {
    my ($std, $err);
    open(my $stdh, '>', \$std) || die "Ooops";
    open(my $errh, '>', \$err) || die "Ooops";

    my $it = $CLASS->new(
        handles => [$stdh, $errh, $stdh],
    );

    return ($it, \$std, \$err);
}

tests "IO handle stuff" => sub {
    ok($CLASS->can($_), "$CLASS has the '$_' method") for qw/no_numbers handles/;
    ok($CLASS->isa('Test2::Formatter'), "$CLASS isa Test2::Formatter");

    ok(!$BEFORE_LOAD{STDOUT}, "AUTOFLUSH was not on for STDOUT before load");
    ok(!$BEFORE_LOAD{STDERR}, "AUTOFLUSH was not on for STDERR before load");
    my $old = select STDOUT;
    ok($|, "AUTOFLUSH was turned on for STDOUT");
    select STDERR;
    ok($|, "AUTOFLUSH was turned on for STDERR");
    select $old;

    ok(my $one = $CLASS->new, "Created a new instance");
    my $handles = $one->handles;
    is(@$handles, 2, "Got 2 handles");
    ok($handles->[0] != $handles->[1], "First and second handles are not the same");
    my $layers = {map { $_ => 1 } PerlIO::get_layers($handles->[0])};

    if (${^UNICODE} & 2) {    # 2 means STDIN
        ok($layers->{utf8}, "'S' is set in PERL_UNICODE, or in -C, honor it, utf8 should be on");
    }
    else {
        ok(!$layers->{utf8}, "Not utf8 by default");
    }

    $one->encoding('utf8');
    is($one->encoding, 'utf8', "Got encoding");
    $handles = $one->handles;
    is(@$handles, 2, "Got 2 handles");
    $layers = {map { $_ => 1 } PerlIO::get_layers($handles->[OUT_STD])};
    ok($layers->{utf8}, "Now utf8");

    my $two = $CLASS->new(encoding => 'utf8');
    $handles = $two->handles;
    is(@$handles, 2, "Got 2 handles");
    $layers = {map { $_ => 1 } PerlIO::get_layers($handles->[OUT_STD])};
    ok($layers->{utf8}, "Now utf8");

    $old = select $handles->[OUT_STD];
    ok($|, "AUTOFLUSH was turned on for copy-STDOUT");
    select select $handles->[OUT_ERR];
    ok($|, "AUTOFLUSH was turned on for copy-STDERR");
    select $old;

    ok($CLASS->hide_buffered,     "TAP will hide buffered events");
    ok(!$CLASS->no_subtest_space, "Default formatter does not have subtest space");
};

tests optimal_pass => sub {
    my ($it, $out, $err) = grabber();

    my $fail = Test2::Event::Fail->new;
    ok(!$it->print_optimal_pass($fail, 1), "Not gonna print a non-pass");

    $fail = Test2::Event::Ok->new(pass => 0);
    ok(!$it->print_optimal_pass($fail, 1), "Not gonna print a non-pass");

    my $pass = Test2::Event::Pass->new();
    $pass->add_amnesty({tag => 'foo', details => 'foo'});
    ok(!$it->print_optimal_pass($pass, 1), "Not gonna print amnesty");

    $pass = Test2::Event::Ok->new(pass => 1, todo => '');
    ok(!$it->print_optimal_pass($pass, 1), "Not gonna print todo (even empty todo)");

    $pass = Test2::Event::Ok->new(pass => 1, name => "foo # bar");
    ok(!$it->print_optimal_pass($pass, 1), "Not gonna pritn a name with a hash");

    $pass = Test2::Event::Ok->new(pass => 1, name => "foo \n bar");
    ok(!$it->print_optimal_pass($pass, 1), "Not gonna pritn a name with a newline");

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");

    $pass = Test2::Event::Pass->new();
    ok($it->print_optimal_pass($pass, 1), "Printed a simple pass without a name");

    $pass = Test2::Event::Pass->new(name => 'xxx');
    ok($it->print_optimal_pass($pass, 1), "Printed a simple pass with a name");

    $pass = Test2::Event::Ok->new(pass => 1, name => 'xxx');
    ok($it->print_optimal_pass($pass, 1), "Printed an 'Ok' pass with a name");

    $pass = Test2::Event::Pass->new(name => 'xxx', trace => {nested => 1});
    ok($it->print_optimal_pass($pass, 1), "Printed a nested pass");
    $pass = Test2::Event::Pass->new(name => 'xxx', trace => {nested => 3});
    ok($it->print_optimal_pass($pass, 1), "Printed a deeply nested pass");

    $pass = Test2::Event::Pass->new(name => 'xxx');
    $it->{no_numbers} = 1;
    ok($it->print_optimal_pass($pass, 1), "Printed a simple pass with a name");

    is($$out, <<"    EOT", "Got expected TAP output");
ok 1
ok 1 - xxx
ok 1 - xxx
    ok 1 - xxx
            ok 1 - xxx
ok - xxx
    EOT

    is($it->{_last_fh}, $it->handles->[OUT_STD], "Set the last filehandle");

    ok(!$$err, "No err output");
};

tests plan_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply([$it->plan_tap({})], [], "Nothing with no plan facet");

    is_deeply(
        [$it->plan_tap({plan => {none => 1}})],
        [],
        "no-plan has no output"
    );

    is_deeply(
        [$it->plan_tap({plan => {count => 20}})],
        [[OUT_STD, "1..20\n"]],
        "Wrote the plan from, count"
    );

    is_deeply(
        [$it->plan_tap({plan => {count => 'anything', skip => 1}})],
        [[OUT_STD, "1..0 # SKIP\n"]],
        "Skip, no reason"
    );

    is_deeply(
        [$it->plan_tap({plan => {count => 'anything', skip => 1, details => 'I said so'}})],
        [[OUT_STD, "1..0 # SKIP I said so\n"]],
        "Skip with reason"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests assert_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [$it->assert_tap({assert => {pass => 1}}, 1)],
        [[OUT_STD, "ok 1\n"]],
        "Pass",
    );

    is_deeply(
        [$it->assert_tap({assert => {pass => 0}}, 1)],
        [[OUT_STD, "not ok 1\n"]],
        "Fail",
    );

    tests amnesty => sub {
        tests pass_no_name => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 1}, amnesty => [{tag => 'skip', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 # skip xxx\n"]],
                "Pass with skip (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1}, amnesty => [{tag => 'skip'}]}, 1)],
                [[OUT_STD, "ok 1 # skip\n"]],
                "Pass with skip (without details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1}, amnesty => [{tag => 'TODO', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 # TODO xxx\n"]],
                "Pass with TODO (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1}, amnesty => [{tag => 'TODO'}]}, 1)],
                [[OUT_STD, "ok 1 # TODO\n"]],
                "Pass with TODO (without details)",
            );

            is_deeply(
                [
                    $it->assert_tap(
                        {
                            assert  => {pass => 1},
                            amnesty => [
                                {tag => 'TODO', details => 'xxx'},
                                {tag => 'skip', details => 'yyy'},
                            ]
                        },
                        1
                    )
                ],
                [[OUT_STD, "ok 1 # TODO & SKIP yyy\n"]],
                "Pass with skip and TODO",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1}, amnesty => [{tag => 'foo', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 # foo xxx\n"]],
                "Pass with other amnesty",
            );
        };

        tests pass_with_name => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => 'bob'}, amnesty => [{tag => 'skip', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 - bob # skip xxx\n"]],
                "Pass with skip (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => 'bob'}, amnesty => [{tag => 'skip'}]}, 1)],
                [[OUT_STD, "ok 1 - bob # skip\n"]],
                "Pass with skip (without details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => 'bob'}, amnesty => [{tag => 'TODO', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 - bob # TODO xxx\n"]],
                "Pass with TODO (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => 'bob'}, amnesty => [{tag => 'TODO'}]}, 1)],
                [[OUT_STD, "ok 1 - bob # TODO\n"]],
                "Pass with TODO (without details)",
            );

            is_deeply(
                [
                    $it->assert_tap(
                        {
                            assert  => {pass => 1, details => 'bob'},
                            amnesty => [
                                {tag => 'TODO', details => 'xxx'},
                                {tag => 'skip', details => 'yyy'},
                            ]
                        },
                        1
                    )
                ],
                [[OUT_STD, "ok 1 - bob # TODO & SKIP yyy\n"]],
                "Pass with skip and TODO",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => 'bob'}, amnesty => [{tag => 'foo', details => 'xxx'}]}, 1)],
                [[OUT_STD, "ok 1 - bob # foo xxx\n"]],
                "Pass with other amnesty",
            );
        };

        tests fail_no_name => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 0}, amnesty => [{tag => 'skip', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 # skip xxx\n"]],
                "Pass with skip (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0}, amnesty => [{tag => 'skip'}]}, 1)],
                [[OUT_STD, "not ok 1 # skip\n"]],
                "Pass with skip (without details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0}, amnesty => [{tag => 'TODO', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 # TODO xxx\n"]],
                "Pass with TODO (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0}, amnesty => [{tag => 'TODO'}]}, 1)],
                [[OUT_STD, "not ok 1 # TODO\n"]],
                "Pass with TODO (without details)",
            );

            is_deeply(
                [
                    $it->assert_tap(
                        {
                            assert  => {pass => 0},
                            amnesty => [
                                {tag => 'TODO', details => 'xxx'},
                                {tag => 'skip', details => 'yyy'},
                            ]
                        },
                        1
                    )
                ],
                [[OUT_STD, "not ok 1 # TODO & SKIP yyy\n"]],
                "Pass with skip and TODO",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0}, amnesty => [{tag => 'foo', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 # foo xxx\n"]],
                "Pass with other amnesty",
            );
        };

        tests fail_with_name => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => 'bob'}, amnesty => [{tag => 'skip', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 - bob # skip xxx\n"]],
                "Pass with skip (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => 'bob'}, amnesty => [{tag => 'skip'}]}, 1)],
                [[OUT_STD, "not ok 1 - bob # skip\n"]],
                "Pass with skip (without details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => 'bob'}, amnesty => [{tag => 'TODO', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 - bob # TODO xxx\n"]],
                "Pass with TODO (with details)",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => 'bob'}, amnesty => [{tag => 'TODO'}]}, 1)],
                [[OUT_STD, "not ok 1 - bob # TODO\n"]],
                "Pass with TODO (without details)",
            );

            is_deeply(
                [
                    $it->assert_tap(
                        {
                            assert  => {pass => 0, details => 'bob'},
                            amnesty => [
                                {tag => 'TODO', details => 'xxx'},
                                {tag => 'skip', details => 'yyy'},
                            ]
                        },
                        1
                    )
                ],
                [[OUT_STD, "not ok 1 - bob # TODO & SKIP yyy\n"]],
                "Pass with skip and TODO",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => 'bob'}, amnesty => [{tag => 'foo', details => 'xxx'}]}, 1)],
                [[OUT_STD, "not ok 1 - bob # foo xxx\n"]],
                "Pass with other amnesty",
            );
        };
    };

    tests newline_and_hash => sub {
        tests pass => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo\nbar"}}, 1)],
                [
                    [OUT_STD, "ok 1 - foo\n"],
                    [OUT_STD, "#      bar\n"],
                ],
                "Pass with newline",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo\nbar"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [
                    [OUT_STD, "ok 1 - foo # baz bat\n"],
                    [OUT_STD, "#      bar\n"],
                ],
                "Pass with newline and amnesty",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo#bar"}}, 1)],
                [[OUT_STD, "ok 1 - foo\\#bar\n"]],
                "Pass with hash",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo#bar"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [[OUT_STD, "ok 1 - foo\\#bar # baz bat\n"]],
                "Pass with hash and amnesty",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo#x\nbar#boo"}}, 1)],
                [
                    [OUT_STD, "ok 1 - foo\\#x\n"],
                    [OUT_STD, "#      bar#boo\n"],
                ],
                "Pass with newline and hash",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 1, details => "foo#x\nbar#boo"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [
                    [OUT_STD, "ok 1 - foo\\#x # baz bat\n"],
                    [OUT_STD, "#      bar#boo\n"],
                ],
                "Pass with newline and hash and amnesty",
            );
        };

        tests fail => sub {
            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo\nbar"}}, 1)],
                [
                    [OUT_STD, "not ok 1 - foo\n"],
                    [OUT_STD, "#          bar\n"],
                ],
                "Pass with newline",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo\nbar"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [
                    [OUT_STD, "not ok 1 - foo # baz bat\n"],
                    [OUT_STD, "#          bar\n"],
                ],
                "Pass with newline and amnesty",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo#bar"}}, 1)],
                [[OUT_STD, "not ok 1 - foo\\#bar\n"]],
                "Pass with hash",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo#bar"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [[OUT_STD, "not ok 1 - foo\\#bar # baz bat\n"]],
                "Pass with hash and amnesty",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo#x\nbar#boo"}}, 1)],
                [
                    [OUT_STD, "not ok 1 - foo\\#x\n"],
                    [OUT_STD, "#          bar#boo\n"],
                ],
                "Pass with newline and hash",
            );

            is_deeply(
                [$it->assert_tap({assert => {pass => 0, details => "foo#x\nbar#boo"}, amnesty => [{tag => 'baz', details => 'bat'}]}, 1)],
                [
                    [OUT_STD, "not ok 1 - foo\\#x # baz bat\n"],
                    [OUT_STD, "#          bar#boo\n"],
                ],
                "Pass with newline and hash and amnesty",
            );
        };
    };

    tests parent => sub {
        is_deeply(
            [
                $it->assert_tap(
                    {
                        assert => {pass => 1, details  => 'bob'},
                        parent => {hid  => 1, buffered => 1, children => [{assert => {pass => 1, details => 'bob2'}}]},
                    },
                    1
                )
            ],
            [
                [OUT_STD, "ok 1 - bob {\n"],
                [OUT_STD, "    ok 1 - bob2\n"],
                [OUT_STD, "}\n"],
            ],
            "Parent (buffered)",
        );

        is_deeply(
            [
                $it->assert_tap(
                    {
                        assert => {pass => 1, details  => 'bob'},
                        parent => {hid  => 1, buffered => 0, children => [{assert => {pass => 1, details => 'bob2'}}]},
                    },
                    1
                )
            ],
            [[OUT_STD, "ok 1 - bob\n"]],
            "Parent (un-buffered)",
        );
    };

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests debug_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {pass  => 0},
                    trace  => {frame => ['foo', 'foo.t', 42]},
                },
                1
            )
        ],
        [
            [OUT_ERR, "# Failed test at foo.t line 42.\n"],
        ],
        "debug tap, nameless test"
    );

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {details => 'foo bar', pass => 0},
                    trace => {frame => ['foo', 'foo.t', 42]},
                },
                1
            )
        ],
        [
            [OUT_ERR, "# Failed test 'foo bar'\n# at foo.t line 42.\n"],
        ],
        "Debug tap, named test"
    );

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {details => 'foo bar', pass => 0},
                    trace => {frame => ['foo', 'foo.t', 42], details => 'I say hi!'},
                },
                1
            )
        ],
        [
            [OUT_ERR, "# Failed test 'foo bar'\n# I say hi!\n"],
        ],
        "Debug tap with details"
    );

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {details => 'foo bar', pass => 0},
                },
                1
            )
        ],
        [
            [OUT_ERR, "# Failed test 'foo bar'\n# [No trace info available]\n"],
        ],
        "Debug tap no trace"
    );

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {details => 'foo bar', pass => 0},
                    trace   => {frame => ['foo', 'foo.t', 42]},
                    amnesty => [],
                },
                1
            )
        ],
        [
            [OUT_ERR, "# Failed test 'foo bar'\n# at foo.t line 42.\n"],
        ],
        "Debug empty amnesty"
    );

    is_deeply(
        [
            $it->debug_tap(
                {
                    assert => {details => 'foo bar', pass => 0},
                    trace   => {frame => ['foo', 'foo.t', 42]},
                    amnesty => [{tag => 'TODO', details => 'xxx'}],
                },
                1
            )
        ],
        [
            [OUT_STD, "# Failed test (with amnesty) 'foo bar'\n# at foo.t line 42.\n"],
        ],
        "Debug empty amnesty"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");

    my $event = Test2::Event::Fail->new(trace => {frame => ['foo', 'foo.pl', 42]});

    {
        local $ENV{HARNESS_ACTIVE}     = 0;
        local $ENV{HARNESS_IS_VERBOSE} = 0;

        $event->{name} = 'no harness';
        $it->write($event, 1);

        $ENV{HARNESS_ACTIVE}     = 0;
        $ENV{HARNESS_IS_VERBOSE} = 1;

        $event->{name} = 'no harness, but strangely verbose';
        $it->write($event, 1);

        $ENV{HARNESS_ACTIVE}     = 1;
        $ENV{HARNESS_IS_VERBOSE} = 0;

        $event->{name} = 'harness, but not verbose';
        $it->write($event, 1);

        $ENV{HARNESS_ACTIVE}     = 1;
        $ENV{HARNESS_IS_VERBOSE} = 1;

        $event->{name} = 'harness that is verbose';
        $it->write($event, 1);
    }

    is($$out, <<"    EOT", "Got 4 failures to STDERR");
not ok 1 - no harness
not ok 1 - no harness, but strangely verbose
not ok 1 - harness, but not verbose
not ok 1 - harness that is verbose
    EOT

    is($$err, <<"    EOT", "Got expected diag to STDERR, newline for non-verbose harness");
# Failed test 'no harness'
# at foo.pl line 42.
# Failed test 'no harness, but strangely verbose'
# at foo.pl line 42.

# Failed test 'harness, but not verbose'
# at foo.pl line 42.

# Failed test 'harness that is verbose'
# at foo.pl line 42.
    EOT
};

tests halt_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [$it->halt_tap({trace => {nested => 1},})],
        [],
        "No output when nested"
    );

    is_deeply(
        [$it->halt_tap({trace => {nested => 1, buffered => 1}})],
        [[OUT_STD, "Bail out!\n"]],
        "Got tap for nested buffered bail"
    );

    is_deeply(
        [$it->halt_tap({control => {details => ''}})],
        [[OUT_STD, "Bail out!\n"]],
        "Empty details"
    );

    is_deeply(
        [$it->halt_tap({control => {details => undef}})],
        [[OUT_STD, "Bail out!\n"]],
        "undef details"
    );

    is_deeply(
        [$it->halt_tap({control => {details => 0}})],
        [[OUT_STD, "Bail out!  0\n"]],
        "falsy details"
    );

    is_deeply(
        [$it->halt_tap({control => {details => 'foo bar baz'}})],
        [[OUT_STD, "Bail out!  foo bar baz\n"]],
        "full details"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests summary_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [$it->summary_tap({about => {no_display => 1, details => "Should not see me"}})],
        [],
        "no display"
    );

    is_deeply(
        [$it->summary_tap({about => {no_display => 0, details => ""}})],
        [],
        "no summary"
    );

    is_deeply(
        [$it->summary_tap({about => {no_display => 0, details => "foo bar"}})],
        [[OUT_STD, "# foo bar\n"]],
        "summary"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests info_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [
            $it->info_tap(
                {
                    info => [
                        {debug => 0, details => "foo"},
                        {debug => 1, details => "foo"},
                        {debug => 0, details => "foo\nbar\nbaz"},
                        {debug => 1, details => "foo\nbar\nbaz"},
                    ]
                }
            )
        ],
        [
            [OUT_STD, "# foo\n"],
            [OUT_ERR, "# foo\n"],
            [OUT_STD, "# foo\n# bar\n# baz\n"],
            [OUT_ERR, "# foo\n# bar\n# baz\n"],
        ],
        "Got all infos"
    );

    my @TAP = $it->info_tap(
        {
            info => [
                {debug => 0, details => {structure => 'yes'}},
                {debug => 1, details => {structure => 'yes'}},
            ]
        }
    );

    is($TAP[0]->[0], OUT_STD, "First went to STDOUT");
    is($TAP[1]->[0], OUT_ERR, "Second went to STDOUT");

    like($TAP[0]->[1], qr/structure.*=>.*yes/, "We see the structure in some form");
    like($TAP[1]->[1], qr/structure.*=>.*yes/, "We see the structure in some form");

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests error_tap => sub {
    my ($it, $out, $err) = grabber();

    # Data::Dumper behavior can change from version to version, specifically
    # the Data::Dumper in 5.8.9 produces different whitespace from other
    # versions.
    require Data::Dumper;
    my $dumper = Data::Dumper->new([{structure => 'yes'}])->Indent(2)->Terse(1)->Pad('# ')->Useqq(1)->Sortkeys(1);
    chomp(my $struct = $dumper->Dump);

    is_deeply(
        [
            $it->error_tap(
                {
                    errors => [
                        {details => "foo"},
                        {details => "foo\nbar\nbaz"},
                        {details => {structure => 'yes'}},
                    ]
                }
            )
        ],
        [
            [OUT_ERR, "# foo\n"],
            [OUT_ERR, "# foo\n# bar\n# baz\n"],
            [OUT_ERR, "$struct\n"],
        ],
        "Got all errors"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests event_tap => sub {
    my ($it, $out, $err) = grabber();

    is_deeply(
        [$it->event_tap({plan => {count => 5}, assert => {pass => 1}}, 1)],
        [
            [OUT_STD, "1..5\n"],
            [OUT_STD, "ok 1\n"],
        ],
        "Plan then assertion for first assertion"
    );

    $it->{made_assertion} = 1;

    is_deeply(
        [$it->event_tap({plan => {count => 5}, assert => {pass => 1}}, 2)],
        [
            [OUT_STD, "ok 2\n"],
            [OUT_STD, "1..5\n"],
        ],
        "Assertion then plan for additional assertions"
    );

    $it->{made_assertion} = 0;
    is_deeply(
        [
            $it->event_tap(
                {
                    plan   => {count    => 5},
                    assert => {pass     => 0},
                    errors => [{details => "foo"}],
                    info   => [
                        {tag => 'DIAG', debug => 1, details => 'xxx'},
                        {tag => 'NOTE', debug => 0, details => 'yyy'},
                    ],
                    control => {halt    => 1, details => 'blah'},
                    about   => {details => 'xyz'},
                },
                1
            )
        ],
        [
            [OUT_STD, "1..5\n"],
            [OUT_STD, "not ok 1\n"],
            [OUT_ERR, "# Failed test [No trace info available]\n"],
            [OUT_ERR, "# foo\n"],
            [OUT_ERR, "# xxx\n"],
            [OUT_STD, "# yyy\n"],
            [OUT_STD, "Bail out!  blah\n"],
        ],
        "All facets displayed"
    );

    is_deeply(
        [
            $it->event_tap(
                {
                    plan  => {count   => 5},
                    about => {details => 'xyz'},
                },
                1
            )
        ],
        [[OUT_STD, "1..5\n"]],
        "Plan blocks details"
    );

    is_deeply(
        [
            $it->event_tap(
                {
                    assert => {pass    => 0, no_debug => 1},
                    about  => {details => 'xyz'},
                },
                1
            )
        ],
        [[OUT_STD, "not ok 1\n"]],
        "Assert blocks details"
    );

    is_deeply(
        [
            $it->event_tap(
                {
                    errors => [{details => "foo"}],
                    about  => {details => 'xyz'},
                },
                1
            )
        ],
        [[OUT_ERR, "# foo\n"]],
        "Error blocks details"
    );

    is_deeply(
        [
            $it->event_tap(
                {
                    info => [
                        {tag => 'DIAG', debug => 1, details => 'xxx'},
                        {tag => 'NOTE', debug => 0, details => 'yyy'},
                    ],
                    about => {details => 'xyz'},
                },
                1
            )
        ],
        [
            [OUT_ERR, "# xxx\n"],
            [OUT_STD, "# yyy\n"],
        ],
        "Info blocks details"
    );

    is_deeply(
        [
            $it->event_tap(
                {
                    control => {halt    => 1, details => 'blah'},
                    about   => {details => 'xyz'},
                },
                1
            )
        ],
        [[OUT_STD, "Bail out!  blah\n"]],
        "Halt blocks details"
    );

    is_deeply(
        [$it->event_tap({about => {details => 'xyz'}}, 1)],
        [[OUT_STD, "# xyz\n"]],
        "Fallback to summary"
    );

    ok(!$$out, "No std output yet");
    ok(!$$err, "No err output yet");
};

tests write => sub {
    my ($it, $out, $err) = grabber();

    local $ENV{HARNESS_ACTIVE}     = 0;
    local $ENV{HARNESS_IS_VERBOSE} = 0;

    {
        local $\ = 'oops1';
        local $, = 'oops2';
        $it->write(
            undef, 1,
            {
                plan   => {count    => 5},
                assert => {pass     => 0},
                errors => [{details => "foo"}],
                info   => [
                    {tag => 'DIAG', debug => 1, details => 'xxx'},
                    {tag => 'NOTE', debug => 0, details => 'yyy'},
                ],
                control => {halt    => 1, details => 'blah'},
                about   => {details => 'xyz'},
            },
        );

        $it->write(undef, 2, {assert => {pass => 1}, trace => {nested => 1}});
    }

    is($it->{_last_fh}, $it->handles->[OUT_STD], "Set last handle");

    is($$out, <<"    EOT", "STDOUT is as expected");
1..5
not ok 1
# yyy
Bail out!  blah
    ok 2
    EOT

    is($$err, <<"    EOT", "STDERR is as expected");
# Failed test [No trace info available]
# foo
# xxx
    EOT
};

my $can_table      = $CLASS->supports_tables;
my $author_testing = $ENV{AUTHOR_TESTING};

if ($author_testing && !$can_table) {
    die "You are running this test under AUTHOR_TESTING, doing so requires Term::Table to be installed, but it is not currently installed, this is a fatal error. Please install Term::Table before attempting to run this test under AUTHOR_TESTING.";
}
elsif ($can_table) {
    tests tables => sub {
        my ($it, $out, $err) = grabber();

        no warnings 'redefine';
        local *Term::Table::Util::term_size = sub { 70 };

        my %table_data = (
            header => ['H1', 'H2'],
            rows   => [
                ["R1C1\n", 'R1C2'],
                ['R2C1', 'R2C2'],
                [('x' x 30), ('y' x 30)],
            ],
        );

        {
            local *Test2::Formatter::TAP::supports_tables = sub { 0 };
            $it->write(
                undef, 1, {
                    info => [
                        {
                            tag     => 'DIAG',
                            details => 'should see only this',
                            debug   => 1,
                            table   => \%table_data,
                        },
                        {
                            tag     => 'NOTE',
                            details => 'should see only this',
                            table   => \%table_data,
                        },
                    ]
                },
            );
        }

        $it->write(
            undef, 1, {
                info => [
                    {
                        tag     => 'DIAG',
                        details => 'should not see',
                        debug   => 1,
                        table   => \%table_data,
                    },
                    {
                        tag     => 'NOTE',
                        details => 'should not see',
                        table   => \%table_data,
                    },
                ]
            },
        );

        $it->write(
            undef, 1, {
                trace => {nested => 2},
                info  => [
                    {
                        tag     => 'DIAG',
                        details => 'should not see',
                        debug   => 1,
                        table   => \%table_data,
                    },
                    {
                        tag     => 'NOTE',
                        details => 'should not see',
                        table   => \%table_data,
                    },
                ]
            },
        );

        my $table1 = join "\n" => map { "# $_" } Term::Table->new(
            %table_data,
            max_width => Term::Table::Util::term_size() - 2,    # 2 for '# '
            collapse  => 1,
            sanitize  => 1,
            mark_tail => 1,
        )->render;

        my $table2 = join "\n" => map { "        # $_" } Term::Table->new(
            %table_data,
            max_width => Term::Table::Util::term_size() - 10,    # 2 for '# ', 8 for indentation
            collapse  => 1,
            sanitize  => 1,
            mark_tail => 1,
        )->render;

        is($$out, <<"        EOT", "Showed detail OR tables, properly sized and indented in STDOUT");
# should see only this
$table1
$table2
        EOT

        is($$err, <<"        EOT", "Showed detail OR tables, properly sized and indented in STDERR");
# should see only this
$table1
$table2
        EOT
    };
}

done_testing;
