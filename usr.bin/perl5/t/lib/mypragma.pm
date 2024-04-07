=head1 NAME

mypragma - an example of a user pragma

=head1 SYNOPSIS

In your code

    use mypragma; # Enable the pragma
    
    mypragma::in_effect() # returns true; pragma is enabled

    no mypragma;
    
    mypragma::in_effect() # returns false; pragma is not enabled

=head1 DESCRIPTION

An example of how to write a pragma.

=head1 AUTHOR

Rafael Garcia-Suarez

=cut

package mypragma;

use strict;
use warnings;

sub import {
    $^H{mypragma} = 42;
}

sub unimport {
    $^H{mypragma} = 0;
}

sub in_effect {
    my $hinthash = (caller(0))[10];
    return $hinthash->{mypragma};
}

1;
