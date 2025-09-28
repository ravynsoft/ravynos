# subclass for testing TAP::Harness custom sources

package MyFileSourceHandler;

use strict;
use warnings;
our ($LAST_OBJ, $CAN_HANDLE, $MAKE_ITER, $LAST_SOURCE);

use TAP::Parser::IteratorFactory;

use base qw( TAP::Parser::SourceHandler::File MyCustom );
$LAST_OBJ    = undef;
$CAN_HANDLE  = undef;
$MAKE_ITER   = undef;
$LAST_SOURCE = undef;

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

sub can_handle {
    my $class = shift;
    $class->SUPER::can_handle(@_);
    $CAN_HANDLE++;
    return 1;
}

sub make_iterator {
    my ( $class, $source ) = @_;
    my $iter = $class->SUPER::make_iterator($source);
    $MAKE_ITER++;
    $LAST_SOURCE = $source;
    return $iter;
}

1;
