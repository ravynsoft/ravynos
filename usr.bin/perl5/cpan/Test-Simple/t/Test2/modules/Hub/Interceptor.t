use strict;
use warnings;
use Test2::Tools::Tiny;

use Test2::Hub::Interceptor;

my $one = Test2::Hub::Interceptor->new();

ok($one->isa('Test2::Hub'), "inheritence");;

my $e = exception { $one->terminate(55) };
ok($e->isa('Test2::Hub::Interceptor::Terminator'), "exception type");
like($$e, 'Label not found for "last T2_SUBTEST_WRAPPER"', "Could not find label");

done_testing;
