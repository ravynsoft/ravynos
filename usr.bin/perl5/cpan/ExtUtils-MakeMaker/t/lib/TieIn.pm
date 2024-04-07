package TieIn;

use strict;
use warnings;

sub TIEHANDLE {
    bless( \(my $scalar), $_[0]);
}

sub write {
    my $self = shift;
    $$self .= join '', @_;
}

sub READLINE {
    my $self = shift;
    $$self =~ s/^(.*\n?)//;
    return $1;
}

sub EOF {
    my $self = shift;
    return !length $$self;
}

1;
