package my::pragma;

require autodie;
use Import::Into qw(into);

sub import {
    shift(@_);
    autodie->import::into(1, @_);
    return;
}

sub dont_die {
    open(my $fd, '<', 'random-file');
    return $fd;
}

1;
