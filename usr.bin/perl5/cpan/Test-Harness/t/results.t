#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 227;

use TAP::Parser::ResultFactory;
use TAP::Parser::Result;

use constant RESULT  => 'TAP::Parser::Result';
use constant PLAN    => 'TAP::Parser::Result::Plan';
use constant TEST    => 'TAP::Parser::Result::Test';
use constant COMMENT => 'TAP::Parser::Result::Comment';
use constant BAILOUT => 'TAP::Parser::Result::Bailout';
use constant UNKNOWN => 'TAP::Parser::Result::Unknown';

my $warning;
$SIG{__WARN__} = sub { $warning = shift };

#
# Note that the are basic unit tests.  More comprehensive path coverage is
# found in the regression tests.
#

my $factory           = TAP::Parser::ResultFactory->new;
my %inherited_methods = (
    is_plan    => '',
    is_test    => '',
    is_comment => '',
    is_bailout => '',
    is_unknown => '',
    is_ok      => 1,
);

my $abstract_class = bless { type => 'no_such_type' },
  RESULT;    # you didn't see this
run_method_tests( $abstract_class, {} );    # check the defaults

can_ok $abstract_class, 'type';
is $abstract_class->type, 'no_such_type',
  '... and &type should return the correct result';

can_ok $abstract_class, 'passed';
$warning = '';
ok $abstract_class->passed, '... and it should default to true';
like $warning, qr/^\Qpassed() is deprecated.  Please use "is_ok()"/,
  '... but it should emit a deprecation warning';

can_ok RESULT, 'new';

can_ok $factory, 'make_result';
eval { $factory->make_result( { type => 'no_such_type' } ) };
ok my $error = $@, '... and calling it with an unknown class should fail';
like $error, qr/^Could not determine class for.*no_such_type/s,
  '... with an appropriate error message';

# register new Result types:
can_ok $factory, 'class_for';
can_ok $factory, 'register_type';
{

    package MyResult;
    use strict;
    use warnings;
    our $VERSION;
    use base 'TAP::Parser::Result';
    TAP::Parser::ResultFactory->register_type( 'my_type' => __PACKAGE__ );
}

{
    my $r = eval { $factory->make_result( { type => 'my_type' } ) };
    my $error = $@;
    isa_ok( $r, 'MyResult', 'register custom type' );
    ok( !$error, '... and no error' );
}

#
# test unknown tokens
#

run_tests(
    {   class => UNKNOWN,
        data  => {
            type => 'unknown',
            raw  => '... this line is junk ... ',
        },
    },
    {   is_unknown    => 1,
        raw           => '... this line is junk ... ',
        as_string     => '... this line is junk ... ',
        type          => 'unknown',
        has_directive => '',
    }
);

#
# test comment tokens
#

run_tests(
    {   class => COMMENT,
        data  => {
            type    => 'comment',
            raw     => '#   this is a comment',
            comment => 'this is a comment',
        },
    },
    {   is_comment    => 1,
        raw           => '#   this is a comment',
        as_string     => '#   this is a comment',
        comment       => 'this is a comment',
        type          => 'comment',
        has_directive => '',
    }
);

#
# test bailout tokens
#

run_tests(
    {   class => BAILOUT,
        data  => {
            type    => 'bailout',
            raw     => 'Bailout!  This blows!',
            bailout => 'This blows!',
        },
    },
    {   is_bailout    => 1,
        raw           => 'Bailout!  This blows!',
        as_string     => 'This blows!',
        type          => 'bailout',
        has_directive => '',
    }
);

#
# test plan tokens
#

run_tests(
    {   class => PLAN,
        data  => {
            type          => 'plan',
            raw           => '1..20',
            tests_planned => 20,
            directive     => '',
            explanation   => '',
        },
    },
    {   is_plan       => 1,
        raw           => '1..20',
        tests_planned => 20,
        directive     => '',
        explanation   => '',
        has_directive => '',
    }
);

run_tests(
    {   class => PLAN,
        data  => {
            type          => 'plan',
            raw           => '1..0 # SKIP help me, Rhonda!',
            tests_planned => 0,
            directive     => 'SKIP',
            explanation   => 'help me, Rhonda!',
        },
    },
    {   is_plan       => 1,
        raw           => '1..0 # SKIP help me, Rhonda!',
        tests_planned => 0,
        directive     => 'SKIP',
        explanation   => 'help me, Rhonda!',
        has_directive => 1,
    }
);

#
# test 'test' tokens
#

my $test = run_tests(
    {   class => TEST,
        data  => {
            ok          => 'ok',
            test_num    => 5,
            description => '... and this test is fine',
            directive   => '',
            explanation => '',
            raw         => 'ok 5 and this test is fine',
            type        => 'test',
        },
    },
    {   is_test       => 1,
        type          => 'test',
        ok            => 'ok',
        number        => 5,
        description   => '... and this test is fine',
        directive     => '',
        explanation   => '',
        is_ok         => 1,
        is_actual_ok  => 1,
        todo_passed   => '',
        has_skip      => '',
        has_todo      => '',
        as_string     => 'ok 5 ... and this test is fine',
        is_unplanned  => '',
        has_directive => '',
    }
);

can_ok $test, 'actual_passed';
$warning = '';
is $test->actual_passed, $test->is_actual_ok,
  '... and it should return the correct value';
like $warning,
  qr/^\Qactual_passed() is deprecated.  Please use "is_actual_ok()"/,
  '... but issue a deprecation warning';

can_ok $test, 'todo_failed';
$warning = '';
is $test->todo_failed, $test->todo_passed,
  '... and it should return the correct value';
like $warning,
  qr/^\Qtodo_failed() is deprecated.  Please use "todo_passed()"/,
  '... but issue a deprecation warning';

# TODO directive

$test = run_tests(
    {   class => TEST,
        data  => {
            ok          => 'not ok',
            test_num    => 5,
            description => '... and this test is fine',
            directive   => 'TODO',
            explanation => 'why not?',
            raw         => 'not ok 5 and this test is fine # TODO why not?',
            type        => 'test',
        },
    },
    {   is_test      => 1,
        type         => 'test',
        ok           => 'not ok',
        number       => 5,
        description  => '... and this test is fine',
        directive    => 'TODO',
        explanation  => 'why not?',
        is_ok        => 1,
        is_actual_ok => '',
        todo_passed  => '',
        has_skip     => '',
        has_todo     => 1,
        as_string =>
          'not ok 5 ... and this test is fine # TODO why not?',
        is_unplanned  => '',
        has_directive => 1,
    }
);

sub run_tests {
    my ( $instantiated, $value_for ) = @_;
    my $result = instantiate($instantiated);
    run_method_tests( $result, $value_for );
    return $result;
}

sub instantiate {
    my $instantiated = shift;
    my $class        = $instantiated->{class};
    ok my $result = $factory->make_result( $instantiated->{data} ),
      'Creating $class results should succeed';
    isa_ok $result, $class, '.. and the object it returns';
    return $result;
}

sub run_method_tests {
    my ( $result, $value_for ) = @_;
    while ( my ( $method, $default ) = each %inherited_methods ) {
        can_ok $result, $method;
        if ( defined( my $value = delete $value_for->{$method} ) ) {
            is $result->$method(), $value,
              "... and $method should be correct";
        }
        else {
            is $result->$method(), $default,
              "... and $method default should be correct";
        }
    }
    while ( my ( $method, $value ) = each %$value_for ) {
        can_ok $result, $method;
        is $result->$method(), $value, "... and $method should be correct";
    }
}
