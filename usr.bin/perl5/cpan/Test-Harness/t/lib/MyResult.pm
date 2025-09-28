# subclass for testing customizing & subclassing

package MyResult;

use strict;
use warnings;

use base qw( TAP::Parser::Result MyCustom );

sub _initialize {
    my $self = shift;
    $self->SUPER::_initialize(@_);
    $main::INIT{ ref($self) }++;
    $self->{initialized} = 1;
    return $self;
}

1;
