#!/usr/bin/perl

use warnings;
use strict;

use Test::More tests => 25;
use XS::APItest;

my $ppaddr = xop_ppaddr;

my $av = xop_build_optree;

is $av->[2], "NAME:custom",     "unregistered XOPs have default name";
is $av->[3], "DESC:unknown custom operator",
                                "unregistered XOPs have default desc";
is $av->[4], "CLASS:0",         "unregistered XOPs are BASEOPs";
is scalar @$av, 5,              "unregistered XOPs don't call peep";

my $names = xop_custom_op_names;
$names->{$ppaddr} = "foo";
$av = xop_build_optree;

is $av->[2], "NAME:foo",        "PL_custom_op_names honoured";
is $av->[3], "DESC:unknown custom operator",
                                "PL_custom_op_descs can be empty";
is $av->[4], "CLASS:0",         "class fallback still works";

# this will segfault if the HV isn't there
my $ops = xop_custom_ops;
pass                            "PL_custom_ops created OK";

my $descs = xop_custom_op_descs;
$descs->{$ppaddr} = "bar";
# this is not generally a supported operation
delete $ops->{$ppaddr};
$av = xop_build_optree;

is $av->[3], "DESC:bar",        "PL_custom_op_descs honoured";

my $xop = xop_my_xop;
delete $ops->{$ppaddr};
delete $names->{$ppaddr};
delete $descs->{$ppaddr};
xop_register;

is $ops->{$ppaddr}, $xop,       "XOP registered OK";

is xop_from_custom_op, $xop,    "XOP lookup from OP roundtrips";

$av = xop_build_optree;
my $OA_UNOP = xop_OA_UNOP;
my ($unop, $kid) = ("???" x 2);

# we can't use 'like', since that runs the match in a different scope
# and so doesn't set $1
ok $av->[0] =~ /unop:([0-9a-f]+)/,  "got unop address"
    and $unop = $1;
ok $av->[1] =~ /kid:([0-9a-f]+)/,   "got kid address"
    and $kid = $1;

is $av->[2], "NAME:my_xop",     "OP_NAME returns registered name";
is $av->[3], "DESC:XOP for testing", "OP_DESC returns registered desc";
is $av->[4], "CLASS:$OA_UNOP",  "OP_CLASS returns registered class";
is scalar @$av, 7,              "registered peep called";
is $av->[5], "peep:$unop",      "...with correct 'o' param";
is $av->[6], "oldop:$kid",      "...and correct 'oldop' param";

xop_clear;

is $ops->{$ppaddr}, $xop,       "clearing XOP doesn't remove it";

$av = xop_build_optree;

is $av->[2], "NAME:custom",     "clearing XOP resets name";
is $av->[3], "DESC:unknown custom operator",
                                "clearing XOP resets desc";
is $av->[4], "CLASS:0",         "clearing XOP resets class";
is scalar @$av, 5,              "clearing XOP removes peep";

ok test_newOP_CUSTOM(),
  'newOP et al. do not fail assertions with OP_CUSTOM';
