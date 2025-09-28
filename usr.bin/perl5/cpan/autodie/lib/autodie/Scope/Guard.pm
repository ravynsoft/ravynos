package autodie::Scope::Guard;

use strict;
use warnings;

# ABSTRACT: Wrapper class for calling subs at end of scope
our $VERSION = '2.36'; # VERSION

# This code schedules the cleanup of subroutines at the end of
# scope.  It's directly inspired by chocolateboy's excellent
# Scope::Guard module.

sub new {
    my ($class, $handler) = @_;
    return bless($handler, $class);
}

sub DESTROY {
    my ($self) = @_;

    $self->();
}

1;

__END__

=head1 NAME

autodie::Scope::Guard - Wrapper class for calling subs at end of scope

=head1 SYNOPSIS

    use autodie::Scope::Guard;
    $^H{'my-key'} = autodie::Scope::Guard->new(sub {
        print "Hallo world\n";
    });

=head1 DESCRIPTION

This class is used to bless perl subs so that they are invoked when
they are destroyed.  This is mostly useful for ensuring the code is
invoked at end of scope.  This module is not a part of autodie's
public API.

This module is directly inspired by chocolateboy's excellent
Scope::Guard module.

=head2 Methods

=head3 new

  my $hook = autodie::Scope::Guard->new(sub {});

Creates a new C<autodie::Scope::Guard>, which will invoke the given
sub once it goes out of scope (i.e. its DESTROY handler is called).

=head1 AUTHOR

Copyright 2008-2009, Paul Fenwick E<lt>pjf@perltraining.com.auE<gt>

=head1 LICENSE

This module is free software.  You may distribute it under the
same terms as Perl itself.
