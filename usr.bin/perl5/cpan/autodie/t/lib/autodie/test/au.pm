package autodie::test::au;
use strict;
use warnings;

use parent qw(autodie);

use autodie::test::au::exception;

sub throw {
    my ($this, @args) = @_;
    return autodie::test::au::exception->new(@args);
}

1;
