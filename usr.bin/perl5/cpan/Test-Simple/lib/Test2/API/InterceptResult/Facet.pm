package Test2::API::InterceptResult::Facet;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN {
    require Test2::EventFacet;
    our @ISA = ('Test2::EventFacet');
}

our $AUTOLOAD;
sub AUTOLOAD {
    my $self = shift;

    my $name = $AUTOLOAD;
    $name =~ s/^.*:://g;

    return undef unless exists $self->{$name};
    return $self->{$name};
}

sub DESTROY {}

1;
