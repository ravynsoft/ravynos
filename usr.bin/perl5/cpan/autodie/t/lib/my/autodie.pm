package my::autodie;
use strict;
use warnings;

use parent qw(autodie);
use autodie::exception;
use autodie::hints;

autodie::hints->set_hints_for(
    'Some::Module::some_sub' => {
        scalar => sub { 1 },        # No calling in scalar/void context
        list   => sub { @_ == 2 and not defined $_[0] }
    },
);

autodie::exception->register(
    'Some::Module::some_sub' => sub {
        my ($E) = @_;

        if ($E->context eq "scalar") {
            return "some_sub() can't be called in scalar context";
        }

        my $error = $E->return->[1];

        return "some_sub() failed: $error";
    }
);

1;
