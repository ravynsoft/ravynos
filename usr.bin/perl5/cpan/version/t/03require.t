#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More qw/no_plan/;
use File::Spec;

BEGIN {
    my $coretests = File::Spec->rel2abs(
        File::Spec->catpath(
            (File::Spec->splitpath($0))[0,1], 'coretests.pm'
        )
    );
    require $coretests;
}

# Don't want to use, because we need to make sure that the import doesn't
# fire just yet (some code does this to avoid importing qv() and delare()).
require_ok("version");
is $version::VERSION, 0.9929, "Make sure we have the correct class";
ok(!"main"->can("qv"), "We don't have the imported qv()");
ok(!"main"->can("declare"), "We don't have the imported declare()");

BaseTests("version","new",undef);
BaseTests("version","parse",undef);
