# avoid cut-n-paste exhaustion with this mixin

package MyCustom;
use strict;
use warnings;

sub custom {
    my $self = shift;
    $main::CUSTOM{ ref($self) || $self }++;
    return $self;
}

1;
