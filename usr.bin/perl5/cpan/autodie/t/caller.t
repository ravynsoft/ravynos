#!/usr/bin/perl -w
use strict;
use warnings;
use autodie;
use Test::More 'no_plan';
use FindBin qw($Bin);
use lib "$Bin/lib";
use Caller_helper;

use constant NO_SUCH_FILE => "kiwifoo_is_so_much_fun";

eval {
    foo();
};

isa_ok($@, 'autodie::exception');

is($@->caller, 'main::foo', "Caller should be main::foo");

sub foo {
    use autodie;
    open(my $fh, '<', NO_SUCH_FILE);
}

eval {
    Caller_helper::foo();
};

isa_ok($@, 'autodie::exception');

is($@->line,     $Caller_helper::line,     "External line number check");
is($@->file,     $INC{"Caller_helper.pm"}, "External filename check");
is($@->package, "Caller_helper",           "External package check");
is($@->caller,  "Caller_helper::foo",      "External subname check");
