#!perl -wT
use strict;
use File::Spec;
use Test::More;

my $macrosall = 'macros.all';
open(MACROS, $macrosall) or plan skip_all => "can't read '$macrosall': $!";
my @names = map {chomp;$_} <MACROS>;
close(MACROS);
plan tests => @names * 2 + 2;

my $callpack = my $testpack = 'Sys::Syslog';
eval "use $callpack";

eval "${callpack}::This()";
like( $@, "/^This is not a valid $testpack macro/", "trying a non-existing macro");

eval "${callpack}::NOSUCHNAME()";
like( $@, "/^NOSUCHNAME is not a valid $testpack macro/", "trying a non-existing macro");

# Testing all macros
if(@names) {
    for my $name (@names) {
        SKIP: {
            $name =~ /^(\w+)$/ or skip "invalid name '$name'", 2;
            $name = $1;
            my $v = eval "${callpack}::$name()";

            if(defined $v and $v =~ /^\d+$/) {
                is( $@, '', "calling the constant $name as a function" );
                like( $v, '/^\d+$/', "checking that $name is a number ($v)" );

            } else {
                like( $@, "/^Your vendor has not defined $testpack macro $name/", 
                    "calling the constant via its name" );
                skip "irrelevant test in this case", 1
            }
        }
    }
}
