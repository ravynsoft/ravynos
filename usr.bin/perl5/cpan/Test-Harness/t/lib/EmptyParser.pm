package EmptyParser;

use strict;
use warnings;

use base qw(TAP::Parser);

sub _initialize {
    shift->_set_defaults;
}

# this should really be in TAP::Parser itself...
sub _set_defaults {
    my $self = shift;

    for my $key (qw( grammar_class result_factory_class )) {
        my $default_method = "_default_$key";
        $self->$key( $self->$default_method() );
    }

    return $self;
}

1;
