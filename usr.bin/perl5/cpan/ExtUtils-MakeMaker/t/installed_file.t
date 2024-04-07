#!/usr/bin/perl -w

# Test MM->_installed_file_for_module()

BEGIN {
    chdir 't' if -d 't';
}

use strict;
use warnings;
use less;

use lib './lib';
use ExtUtils::MakeMaker;
use Test::More tests => 4;
use File::Spec;


sub path_is {
    my($have, $want, $name) = @_;

    $have = File::Spec->canonpath($have);
    $want = File::Spec->canonpath($want);

    my $builder = Test::More->builder;
    return $builder->is_eq( $have, $want, $name );
}

# Test when a module is not installed
{
    ok !MM->_installed_file_for_module("aaldkfjaldj"), "Module not installed";
    ok !MM->_installed_file_for_module("aaldkfjaldj::dlajldkj");
}

# Try a single name module
{
    my $want = $INC{'less.pm'};
    path_is( MM->_installed_file_for_module("less"), $want,  "single name module" );
}

# And a tuple
{
    my $want = $INC{"Test/More.pm"};
    path_is( MM->_installed_file_for_module("Test::More"), $want, "Foo::Bar style" );
}
