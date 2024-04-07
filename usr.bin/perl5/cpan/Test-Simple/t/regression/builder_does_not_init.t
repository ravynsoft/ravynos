use strict;
use warnings;

use Carp qw/confess/;
use Test2::API::Instance;

BEGIN {
    no warnings 'redefine';
    local *Test2::API::Instance::_finalize = sub { confess "_finalize called\n" };
    local *Test2::API::Instance::load = sub { confess "load called\n" };

    require Test::Builder;
}

use Test2::Tools::Tiny;

ok(1, "Did not die");
done_testing();
