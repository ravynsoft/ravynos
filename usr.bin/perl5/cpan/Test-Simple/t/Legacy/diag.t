#!perl -w
use strict;

use Test2::Util qw/CAN_THREAD/;

# Turn on threads here, if available, since this test tends to find
# lots of threading bugs.
BEGIN {
    if (CAN_THREAD) {
        require threads;
        threads->import;
    }
}

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use Test::Builder::NoOutput;
use Test::More tests => 7;

my $test = Test::Builder::NoOutput->create;

# Test diag() goes to todo_output() in a todo test.
{
    $test->todo_start();

    $test->diag("a single line");
    is( $test->read('todo'), <<'DIAG',   'diag() with todo_output set' );
# a single line
DIAG

    my $ret = $test->diag("multiple\n", "lines");
    is( $test->read('todo'), <<'DIAG',   '  multi line' );
# multiple
# lines
DIAG
    ok( !$ret, 'diag returns false' );

    $test->todo_end();
}


# Test diagnostic formatting
{
    $test->diag("# foo");
    is( $test->read('err'), "# # foo\n", "diag() adds # even if there's one already" );

    $test->diag("foo\n\nbar");
    is( $test->read('err'), <<'DIAG', "  blank lines get escaped" );
# foo
# 
# bar
DIAG

    $test->diag("foo\n\nbar\n\n");
    is( $test->read('err'), <<'DIAG', "  even at the end" );
# foo
# 
# bar
# 
DIAG
}


# [rt.cpan.org 8392] diag(@list) emulates print
{
    $test->diag(qw(one two));

    is( $test->read('err'), <<'DIAG' );
# onetwo
DIAG
}
