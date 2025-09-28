#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 28;

use feature 'defer';
no warnings 'experimental::defer';

{
    my $x = "";
    {
        defer { $x = "a" }
    }
    is($x, "a", 'defer block is invoked');

    {
        defer {
            $x = "";
            $x .= "abc";
            $x .= "123";
        }
    }
    is($x, "abc123", 'defer block can contain multiple statements');

    {
       defer {}
    }
    ok(1, 'Empty defer block parses OK');
}

{
    my $x = "";
    {
        defer { $x .= "a" }
        defer { $x .= "b" }
        defer { $x .= "c" }
    }
    is($x, "cba", 'defer blocks happen in LIFO order');
}

{
    my $x = "";

    {
        defer { $x .= "a" }
        $x .= "A";
    }

    is($x, "Aa", 'defer blocks happen after the main body');
}

{
    my $x = "";

    foreach my $i (qw( a b c )) {
        defer { $x .= $i }
    }

    is($x, "abc", 'defer block happens for every iteration of foreach');
}

{
    my $x = "";

    my $cond = 0;
    if( $cond ) {
        defer { $x .= "XXX" }
    }

    is($x, "", 'defer block does not happen inside non-taken conditional branch');
}

{
    my $x = "";

    while(1) {
        last;
        defer { $x .= "a" }
    }

    is($x, "", 'defer block does not happen if entered but unencountered');
}

{
   my $x = "";

   my $counter = 1;
   {
      defer { $x .= "A" }
      redo if $counter++ < 5;
   }

   is($x, "AAAAA", 'defer block can happen multiple times');
}

{
    my $x = "";

    {
        defer {
            $x .= "a";
            defer {
                $x .= "b";
            }
        }
    }

    is($x, "ab", 'defer block can contain another defer');
}

{
    my $x = "";
    my $value = do {
        defer { $x .= "before" }
        "value";
    };

    is($x, "before", 'defer blocks run inside do { }');
    is($value, "value", 'defer block does not disturb do { } value');
}

{
    my $x = "";
    my $sub = sub {
        defer { $x .= "a" }
    };

    $sub->();
    $sub->();
    $sub->();

    is($x, "aaa", 'defer block inside sub');
}

{
    my $x = "";
    my $sub = sub {
        return;
        defer { $x .= "a" }
    };

    $sub->();

    is($x, "", 'defer block inside sub does not happen if entered but returned early');
}

{
   my $x = "";

   my sub after {
      $x .= "c";
   }

   my sub before {
      $x .= "a";
      defer { $x .= "b" }
      goto \&after;
   }

   before();

   is($x, "abc", 'defer block invoked before tail-call');
}

# Sequencing with respect to variable cleanup

{
    my $var = "outer";
    my $x;
    {
        my $var = "inner";
        defer { $x = $var }
    }

    is($x, "inner", 'defer block captures live value of same-scope lexicals');
}

{
    my $var = "outer";
    my $x;
    {
        defer { $x = $var }
        my $var = "inner";
    }

    is ($x, "outer", 'defer block correctly captures outer lexical when only shadowed afterwards');
}

{
    our $var = "outer";
    {
        local $var = "inner";
        defer { $var = "finally" }
    }

    is($var, "outer", 'defer after localization still unlocalizes');
}

{
    our $var = "outer";
    {
        defer { $var = "finally" }
        local $var = "inner";
    }

    is($var, "finally", 'defer before localization overwrites');
}

# Interactions with exceptions

{
    my $x = "";
    my $sub = sub {
        defer { $x .= "a" }
        die "Oopsie\n";
    };

    my $e = defined eval { $sub->(); 1 } ? undef : $@;

    is($x, "a", 'defer block still runs during exception unwind');
    is($e, "Oopsie\n", 'Thrown exception still occurs after defer');
}

{
    my $sub = sub {
        defer { die "Oopsie\n"; }
        return "retval";
    };

    my $e = defined eval { $sub->(); 1 } ? undef : $@;

    is($e, "Oopsie\n", 'defer block can throw exception');
}

{
    my $sub = sub {
        defer { die "Oopsie 1\n"; }
        die "Oopsie 2\n";
    };

    my $e = defined eval { $sub->(); 1 } ? undef : $@;

    # TODO: Currently the first exception gets lost without even a warning
    #   We should consider what the behaviour ought to be here
    # This test is happy for either exception to be seen, does not care which
    like($e, qr/^Oopsie \d\n/, 'defer block can throw exception during exception unwind');
}

# goto
{
    ok(defined eval 'sub { defer { goto HERE; HERE: 1; } }',
        'goto forwards within defer {} is permitted') or
        diag("Failure was $@");

    ok(defined eval 'sub { defer { HERE: 1; goto HERE; } }',
        'goto backwards within defer {} is permitted') or
        diag("Failure was $@");
}

{
    my $sub = sub {
        while(1) {
            goto HERE;
            defer { HERE: 1; }
        }
    };

    my $e = defined eval { $sub->(); 1 } ? undef : $@;
    like($e, qr/^Can't "goto" into a "defer" block /,
        'Cannot goto into defer block');
}

{
    # strictness failures are only checked at optree finalization time. This
    # is a good way to test if that happens.
    my $ok = eval 'defer { use strict; foo }';
    my $e = $@;

    ok(!$ok, 'defer BLOCK finalizes optree');
    like($e, qr/^Bareword "foo" not allowed while "strict subs" in use at /,
        'Error from finalization');
}
