#!./perl

use strict;
use Test::More tests => 18;

my $v_plus  = $] + 1;
my $v_minus = $] - 1;

unless (eval 'use open ":std"; 1') {
  # pretend that open.pm is present
  $INC{'open.pm'} = 'open.pm';
  eval 'sub open::foo{}';		# Just in case...
}

{
    no strict;

    is( eval "use if ($v_minus > \$]), strict => 'subs'; \${'f'} = 12", 12,
        '"use if" with a false condition, fake pragma');
    is( eval "use if ($v_minus > \$]), strict => 'refs'; \${'f'} = 12", 12,
        '"use if" with a false condition and a pragma');

    is( eval "use if ($v_plus > \$]), strict => 'subs'; \${'f'} = 12", 12,
        '"use if" with a true condition, fake pragma');

    is( eval "use if ($v_plus > \$]), strict => 'refs'; \${'f'} = 12", undef,
        '"use if" with a true condition and a pragma');
    like( $@, qr/while "strict refs" in use/, 'expected error message'),

    # Old version had problems with the module name 'open', which is a keyword too
    # Use 'open' =>, since pre-5.6.0 could interpret differently
    is( (eval "use if ($v_plus > \$]), 'open' => IN => ':crlf'; 12" || 0), 12,
        '"use if" with open');

    is(eval "use if ($v_plus > \$])", undef,
       "Too few args to 'use if' returns <undef>");
    like($@, qr/Too few arguments to 'use if'/, "  ... and returns correct error");

    is(eval "no if ($v_plus > \$])", undef,
       "Too few args to 'no if' returns <undef>");
    like($@, qr/Too few arguments to 'no if'/, "  ... and returns correct error");
}

{
    note(q|RT 132732: strict 'subs'|);
    use strict "subs";

    {
        SKIP: {
            unless ($] >= 5.018) {
                skip "bigrat apparently not testable prior to perl-5.18", 4;
            }
            note(q|strict "subs" : 'use if' : condition false|);
            eval "use if (0 > 1), q|bigrat|, qw(hex oct);";
            ok (! main->can('hex'), "Cannot call bigrat::hex() in importing package");
            ok (! main->can('oct'), "Cannot call bigrat::oct() in importing package");

            note(q|strict "subs" : 'use if' : condition true|);
            eval "use if (1 > 0), q|bigrat|, qw(hex oct);";
            ok (  main->can('hex'), "Can call bigrat::hex() in importing package");
            ok (  main->can('oct'), "Can call bigrat::oct() in importing package");
        }
    }

    {
        note(q|strict "subs" : 'no if' : condition variable|);
        note(($] >= 5.022) ? "Recent enough Perl: $]" : "Older Perl: $]");
        use warnings;
        SKIP: {
            unless ($] >= 5.022) {
                skip "Redundant argument warning not available in pre-5.22 perls", 4;
            }

            {
                no if $] >= 5.022, q|warnings|, qw(redundant);
                my ($test, $result, $warn);
                local $SIG{__WARN__} = sub { $warn = shift };
                $test = { fmt  => "%s", args => [ qw( x y ) ] };
                $result = sprintf $test->{fmt}, @{$test->{args}};
                is($result, $test->{args}->[0], "Got expected string");
                ok(! $warn, "Redundant argument warning suppressed");
            }

            {
                use if $] >= 5.022, q|warnings|, qw(redundant);
                my ($test, $result, $warn);
                local $SIG{__WARN__} = sub { $warn = shift };
                $test = { fmt  => "%s", args => [ qw( x y ) ] };
                $result = sprintf $test->{fmt}, @{$test->{args}};
                is($result, $test->{args}->[0], "Got expected string");
                like($warn, qr/Redundant argument in sprintf/,
                    "Redundant argument warning generated and captured");
            }
        }
    }
}
