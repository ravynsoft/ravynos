package autodie::test::au::exception;
use strict;
use warnings;

use parent qw(autodie::exception);

sub time_for_a_beer {
    return "Now's a good time for a beer.";
}

sub stringify {
    my ($this) = @_;

    my $base_str = $this->SUPER::stringify;

    return "$base_str\n" . $this->time_for_a_beer;
}

1;
