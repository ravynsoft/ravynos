#!/usr/bin/perl

use warnings;
use strict;
no warnings 'experimental::builtin';
use builtin qw(reftype);

use Test::More;
use XS::APItest;

BEGIN { *my_caller = \&XS::APItest::my_caller }

{
    package DB;
    no strict "refs";
    sub sub { &$DB::sub }
}

sub try_caller {
    my @args = @_;
    my $l   = shift @args;
    my $n   = pop @args;
    my $hhv = pop @args;

    my @c  = my_caller $l;
    my $hh = pop @c;

    is_deeply \@c, [ @args, ($hhv) x 3 ], 
                                "caller_cx for $n";
    if (defined $hhv) {
	local $TODO; # these two work ok under the bebugger
        ok defined $hh,         "...with defined hinthash";
        is reftype $hh, "HASH", "...which is a HASH";
    }
    is $hh->{foo},  $hhv,       "...with correct hinthash value";
}

try_caller 0, qw/main try_caller/ x 2, undef, "current sub";
{
    BEGIN { $^H{foo} = "bar" }
    try_caller 0, qw/main try_caller/ x 2, "bar", "current sub w/hinthash";
}

sub one {
    my ($hh, $n) = @_;
    try_caller 1, qw/main one/ x 2, $hh, $n;
}

one undef, "upper sub";
{
    BEGIN { $^H{foo} = "baz" }
    one "baz", "upper sub w/hinthash";
}

BEGIN { $^P = 1 }
# This is really bizarre. One stack frame has the correct CV but the
# wrong stash, the other the other way round. At least pp_caller knows
# what to do with them...
try_caller 0, qw/main sub DB try_caller/, undef, "current sub w/DB::sub";
{
    BEGIN { $^H{foo} = "DB" }
    try_caller 0, qw/main sub DB try_caller/, "DB",
                                    "current sub w/hinthash, DB::sub";
}

sub dbone {
    my ($hh, $n) = @_;
    try_caller 1, qw/main sub DB dbone/, $hh, $n;
}

dbone undef, "upper sub w/DB::sub";
TODO: {
    local $TODO = "hinthash incorrect under debugger";
    BEGIN { $^{foo} = "DBu" }
    dbone "DBu", "upper sub w/hinthash, DB::sub";
}
BEGIN { $^P = 0 }

done_testing;
