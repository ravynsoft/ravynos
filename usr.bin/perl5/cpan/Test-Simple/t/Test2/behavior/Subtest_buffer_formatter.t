use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/run_subtest intercept test2_stack/;

{
    package Formatter::Hide;
    sub write { }
    sub hide_buffered { 1 }
    sub terminate { }
    sub finalize { }

    package Formatter::Show;
    sub write { }
    sub hide_buffered { 0 }
    sub terminate { }
    sub finalize { }

    package Formatter::NA;
    sub write { }
    sub terminate { }
    sub finalize { }
}

my %HAS_FORMATTER;

my $events = intercept {
    my $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{unbuffered_none} = $hub->format ? 1 : 0;
    };
    run_subtest('unbuffered', $code);

    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{buffered_none} = $hub->format ? 1 : 0;
    };
    run_subtest('buffered', $code, 'BUFFERED');


    #####################
    test2_stack->top->format(bless {}, 'Formatter::Hide');
    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{unbuffered_hide} = $hub->format ? 1 : 0;
    };
    run_subtest('unbuffered', $code);

    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{buffered_hide} = $hub->format ? 1 : 0;
    };
    run_subtest('buffered', $code, 'BUFFERED');


    #####################
    test2_stack->top->format(bless {}, 'Formatter::Show');
    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{unbuffered_show} = $hub->format ? 1 : 0;
    };
    run_subtest('unbuffered', $code);

    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{buffered_show} = $hub->format ? 1 : 0;
    };
    run_subtest('buffered', $code, 'BUFFERED');


    #####################
    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{unbuffered_na} = $hub->format ? 1 : 0;
    };
    run_subtest('unbuffered', $code);

    test2_stack->top->format(bless {}, 'Formatter::NA');
    $code = sub {
        my $hub = test2_stack->top;
        $HAS_FORMATTER{buffered_na} = $hub->format ? 1 : 0;
    };
    run_subtest('buffered', $code, 'BUFFERED');
};

ok(!$HAS_FORMATTER{unbuffered_none}, "Unbuffered with no parent formatter has no formatter");
ok( $HAS_FORMATTER{unbuffered_show}, "Unbuffered where parent has 'show' formatter has formatter");
ok( $HAS_FORMATTER{unbuffered_hide}, "Unbuffered where parent has 'hide' formatter has formatter");

ok(!$HAS_FORMATTER{buffered_none}, "Buffered with no parent formatter has no formatter");
ok( $HAS_FORMATTER{buffered_show}, "Buffered where parent has 'show' formatter has formatter");
ok(!$HAS_FORMATTER{buffered_hide}, "Buffered where parent has 'hide' formatter has no formatter");

done_testing;
