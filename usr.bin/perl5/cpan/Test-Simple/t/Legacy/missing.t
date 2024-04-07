# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

# Can't use Test.pm, that's a 5.005 thing.
package My::Test;

# This has to be a require or else the END block below runs before
# Test::Builder's own and the ending diagnostics don't come out right.
require Test::Builder;
my $TB = Test::Builder->create;
$TB->plan(tests => 2);

sub is { $TB->is_eq(@_) }


package main;

require Test::Simple;

require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();
local $ENV{HARNESS_ACTIVE} = 0;

Test::Simple->import(tests => 5);

#line 30
ok(1, 'Foo');
ok(0, 'Bar');
ok(1, '1 2 3');

END {
    My::Test::is($$out, <<OUT);
1..5
ok 1 - Foo
not ok 2 - Bar
ok 3 - 1 2 3
OUT

    My::Test::is($$err, <<ERR);
#   Failed test 'Bar'
#   at $0 line 31.
#     You named your test '1 2 3'.  You shouldn't use numbers for your test names.
#     Very confusing.
# Looks like you planned 5 tests but ran 3.
# Looks like you failed 1 test of 3 run.
ERR

    exit 0;
}
