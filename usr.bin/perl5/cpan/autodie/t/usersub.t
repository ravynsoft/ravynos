#!/usr/bin/perl -w
use strict;

use Test::More 'no_plan';

sub mytest {
    return $_[0];
}

is(mytest(q{foo}),q{foo},"Mytest returns input");

my $return = eval { mytest(undef); };

ok(!defined($return), "mytest returns undef without autodie");
is($@,"","Mytest doesn't throw an exception without autodie");

$return = eval {
    use autodie qw(mytest);

    mytest('foo');
};

is($return,'foo',"Mytest returns input with autodie");
is($@,"","No error should be thrown");

$return = eval {
    use autodie qw(mytest);

    mytest(undef);
};

isa_ok($@,'autodie::exception',"autodie mytest/undef throws exception");

# We set initial values here because we're expecting $data to be
# changed to undef later on.   Having it as undef to begin with means
# we can't see mytest(undef) working correctly.

my ($data, $data2) = (1,1);

eval {
    use autodie qw(mytest);

    {
        no autodie qw(mytest);

        $data  = mytest(undef);
        $data2 = mytest('foo');
    }
};

is($@,"","no autodie can counter use autodie for user subs");
ok(!defined($data), "mytest(undef) should return undef");
is($data2, "foo", "mytest(foo) should return foo");

eval {
    mytest(undef);
};

is($@,"","No lingering failure effects");

$return = eval {
    mytest("bar");
};

is($return,"bar","No lingering return effects");
