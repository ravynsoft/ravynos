#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use Test::More tests => 7;

my $tb = Test::Builder->create;

# TB methods expect to be wrapped
my $ok           = sub { shift->ok(@_) };
my $plan         = sub { shift->plan(@_) };
my $done_testing = sub { shift->done_testing(@_) };

#line 20
ok !eval { $tb->$plan(tests => undef) };
is($@, "Got an undefined number of tests at $0 line 20.\n");

#line 24
ok !eval { $tb->$plan(tests => 0) };
is($@, "You said to run 0 tests at $0 line 24.\n");

{
    my $warning = '';
    local $SIG{__WARN__} = sub { $warning .= join '', @_ };

#line 31
    ok $tb->$plan(no_plan => 1);
    is( $warning, "no_plan takes no arguments at $0 line 31.\n" );
    is $tb->has_plan, 'no_plan';
}
