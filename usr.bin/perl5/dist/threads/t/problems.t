use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        print("1..0 # SKIP threads::shared not available\n");
        exit(0);
    }

    $| = 1;
    if ($] == 5.008) {
        print("1..6\n");    ### Number of tests that will be run ###
    } else {
        print("1..10\n");   ### Number of tests that will be run ###
    }
};

print("ok 1 - Loaded\n");

use Hash::Util 'lock_keys';

my $test :shared = 2;

# Note that we can't use Test::More here, as we would need to call is()
# from within the DESTROY() function at global destruction time, and
# parts of Test::* may have already been freed by then
sub is($$$)
{
    my ($got, $want, $desc) = @_;
    lock($test);
    if ($got ne $want) {
        print("# EXPECTED: $want\n");
        print("# GOT:      $got\n");
        print("not ");
    }
    print("ok $test - $desc\n");
    $test++;
}


# This tests for too much destruction which was caused by cloning stashes
# on join which led to double the dataspace under 5.8.0
if ($] != 5.008)
{
    sub Foo::DESTROY
    {
        my $self = shift;
        my ($package, $file, $line) = caller;
        is(threads->tid(), $self->{tid}, "In destroy[$self->{tid}] it should be correct too" );
    }

    my $foo = bless {tid => 0}, 'Foo';
    my $bar = threads->create(sub {
        is(threads->tid(), 1, "And tid be 1 here");
        $foo->{tid} = 1;
        return ($foo);
    })->join();
    $bar->{tid} = 0;
}


# This tests whether we can call Config::myconfig after threads have been
# started (interpreter cloned).  5.8.1 and 5.8.2 contained a bug that would
# disallow that to be done because an attempt was made to change a variable
# with the :unique attribute.

{
    lock($test);
    if ($] == 5.008 || $] >= 5.008003) {
        threads->create( sub {1} )->join;
        my $not = eval { Config::myconfig() } ? '' : 'not ';
        print "${not}ok $test - Are we able to call Config::myconfig after clone\n";
    } else {
        print "ok $test # SKIP Are we able to call Config::myconfig after clone\n";
    }
    $test++;
}


# Returning a closure from a thread caused problems. If the last index in
# the anon sub's pad wasn't for a lexical, then a core dump could occur.
# Otherwise, there might be leaked scalars.

# XXX DAPM 9-Jan-04 - backed this out for now - returning a closure from a
# thread seems to crash win32

# sub f {
#     my $x = "foo";
#     sub { $x."bar" };
# }
# 
# my $string = threads->create(\&f)->join->();
# print $string eq 'foobar' ?  '' : 'not ', "ok $test - returning closure\n";
# $test++;


# Nothing is checking that total keys gets cloned correctly.

my %h = (1,2,3,4);
is(keys(%h), 2, "keys correct in parent");

my $child = threads->create(sub { return (scalar(keys(%h))); })->join;
is($child, 2, "keys correct in child");

lock_keys(%h);
delete($h{1});

is(keys(%h), 1, "keys correct in parent with restricted hash");

$child = threads->create(sub { return (scalar(keys(%h))); })->join;
is($child, 1, "keys correct in child with restricted hash");

exit(0);

# EOF
