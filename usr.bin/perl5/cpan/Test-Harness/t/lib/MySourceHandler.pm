# subclass for testing customizing & subclassing

package MySourceHandler;

use strict;
use warnings;

use MyIterator;
use TAP::Parser::SourceHandler;
use TAP::Parser::IteratorFactory;

#use base qw( TAP::Parser::SourceHandler MyCustom );
use base qw( MyCustom );

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

sub can_handle {
    my ( $class, $source ) = @_;
    my $meta   = $source->meta;
    my $config = $source->config_for($class);

    if ( $config->{accept_all} ) {
        return 1;
    }
    elsif ( my $accept = $config->{accept} ) {
        return 0 unless $meta->{is_scalar};
        return 1 if ${ $source->raw } eq $accept;
    }
    return 0;
}

sub make_iterator {
    my ( $class, $source ) = @_;
    $class->custom;
    return MyIterator->new( [ $source->raw ] );
}

1;
