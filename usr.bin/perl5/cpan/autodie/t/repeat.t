#!/usr/bin/perl -w
use strict;
use Test::More 'no_plan';
use constant NO_SUCH_FILE => "this_file_had_better_not_exist";

eval {
    use autodie qw(open open open);
    open(my $fh, '<', NO_SUCH_FILE);
};

isa_ok($@,q{autodie::exception});
ok($@->matches('open'),"Exception from open");

eval {
    open(my $fh, '<', NO_SUCH_FILE);
};

is($@,"","Repeated autodie should not leak");

