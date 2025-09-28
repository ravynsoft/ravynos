# subclass for testing customizing & subclassing

package MyIterator;

use strict;
use warnings;

use base qw( TAP::Parser::Iterator MyCustom );

sub _initialize {
    my $self = shift;
    $self->SUPER::_initialize(@_);
    $main::INIT{ ref($self) }++;
    $self->{initialized} = 1;
    $self->{content} = [ 'whats TAP all about then?', '1..1', 'ok 1' ];
    return $self;
}

sub next {
    return shift @{ $_[0]->{content} };
}

1;
