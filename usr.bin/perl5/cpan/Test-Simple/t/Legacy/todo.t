#!perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::More;

use strict;
use warnings;

plan tests => 36;


my $Why = 'Just testing the todo interface.';

my $is_todo;
TODO: {
    local $TODO = $Why;

    fail("Expected failure");
    fail("Another expected failure");

    $is_todo = Test::More->builder->todo;
}

pass("This is not todo");
ok( $is_todo, 'TB->todo' );


TODO: {
    local $TODO = $Why;

    fail("Yet another failure");
}

pass("This is still not todo");


TODO: {
    local $TODO = "testing that error messages don't leak out of todo";

    ok( 'this' eq 'that',   'ok' );

    like( 'this', qr/that/, 'like' );
    is(   'this', 'that',   'is' );
    isnt( 'this', 'this',   'isnt' );

    can_ok('Fooble', 'yarble');
    isa_ok('Fooble', 'yarble');
    use_ok('Fooble');
    require_ok('Fooble');
}


TODO: {
    todo_skip "Just testing todo_skip", 2;

    fail("Just testing todo");
    die "todo_skip should prevent this";
    pass("Again");
}


{
    my $warning;
    local $SIG{__WARN__} = sub { $warning = join "", @_ };
    TODO: {
        # perl gets the line number a little wrong on the first
        # statement inside a block.
        1 == 1;
#line 74
        todo_skip "Just testing todo_skip";
        fail("So very failed");
    }
    is( $warning, "todo_skip() needs to know \$how_many tests are in the ".
                  "block at $0 line 74\n",
        'todo_skip without $how_many warning' );
}

my $builder = Test::More->builder;
my $exported_to = $builder->exported_to;
TODO: {
    $builder->exported_to("Wibble");
    
    local $TODO = "testing \$TODO with an incorrect exported_to()";
    
    fail("Just testing todo");
}

$builder->exported_to($exported_to);

$builder->todo_start('Expected failures');
fail('Testing todo_start()');
ok 0, 'Testing todo_start() with more than one failure';
$is_todo = $builder->todo;
$builder->todo_end;
is $is_todo, 'Expected failures',
  'todo_start should have the correct TODO message';
ok 1, 'todo_end() should not leak TODO behavior';

my @nested_todo;
my ( $level1, $level2 ) = ( 'failure level 1', 'failure_level 2' );
TODO: {
    local $TODO = 'Nesting TODO';
    fail('fail 1');

    $builder->todo_start($level1);
    fail('fail 2');

    push @nested_todo => $builder->todo;
    $builder->todo_start($level2);
    fail('fail 3');

    push @nested_todo => $builder->todo;
    $builder->todo_end;
    fail('fail 4');

    push @nested_todo => $builder->todo;
    $builder->todo_end;
    $is_todo = $builder->todo;
    fail('fail 4');
}
is_deeply \@nested_todo, [ $level1, $level2, $level1 ],
  'Nested TODO message should be correct';
is $is_todo, 'Nesting TODO',
  '... and original TODO message should be correct';

{
    $builder->todo_start;
    fail("testing todo_start() with no message");
    my $reason  = $builder->todo;
    my $in_todo = $builder->in_todo;
    $builder->todo_end;

    is $reason, '', "  todo() reports no reason";
    ok $in_todo,    "  but we're in_todo()";
}

eval {
    $builder->todo_end;
};
is $@, sprintf "todo_end() called without todo_start() at %s line %d.\n", $0, __LINE__ - 3;


{
    my($reason, $in_todo);

    TODO: {
        local $TODO = '';
        $reason  = $builder->todo;
        $in_todo = $builder->in_todo;
    }

    is $reason, '';
    ok !$in_todo, '$TODO = "" is not considered TODO';
}
