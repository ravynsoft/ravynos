package autodie::Scope::GuardStack;

use strict;
use warnings;

use autodie::Scope::Guard;

# ABSTRACT: Hook stack for managing scopes via %^H
our $VERSION = '2.36'; # VERSION

my $H_KEY_STEM = __PACKAGE__ . '/guard';
my $COUNTER = 0;

# This code schedules the cleanup of subroutines at the end of
# scope.  It's directly inspired by chocolateboy's excellent
# Scope::Guard module.

sub new {
    my ($class) = @_;

    return bless([], $class);
}

sub push_hook {
    my ($self, $hook) = @_;
    my $h_key = $H_KEY_STEM . ($COUNTER++);
    my $size = @{$self};
    $^H{$h_key} = autodie::Scope::Guard->new(sub {
        # Pop the stack until we reach the right size
        # - this may seem weird, but it is to avoid relying
        #   on "destruction order" of keys in %^H.
        #
        # Example:
        #  {
        #     use autodie;  # hook 1
        #     no autodie;   # hook 2
        #     use autodie;  # hook 3
        #  }
        #
        #  Here we want call hook 3, then hook 2 and finally hook 1.
        #  Any other order could have undesired consequences.
        #
        #  Suppose hook 2 is destroyed first, it will pop hook 3 and
        #  then hook 2.  hook 3 will then be destroyed, but do nothing
        #  since its "frame" was already popped and finally hook 1
        #  will be popped and take its own frame with it.
        #
        #  We need to check that $self still exists since things can get weird
        #  during global destruction.
        $self->_pop_hook while $self && @{$self} > $size;
    });
    push(@{$self}, [$hook, $h_key]);
    return;
}

sub _pop_hook {
    my ($self) = @_;
    my ($hook, $key) = @{ pop(@{$self}) };
    my $ref = delete($^H{$key});
    $hook->();
    return;
}

sub DESTROY {
    my ($self) = @_;

    # To be honest, I suspect @{$self} will always be empty here due
    # to the subs in %^H having references to the stack (which would
    # keep the stack alive until those have been destroyed).  Anyhow,
    # it never hurt to be careful.
    $self->_pop_hook while @{$self};
    return;
}

1;

__END__

=head1 NAME

autodie::Scope::GuardStack -  Hook stack for managing scopes via %^H

=head1 SYNOPSIS

    use autodie::Scope::GuardStack;
    my $stack = autodie::Scope::GuardStack->new
    $^H{'my-key'} = $stack;

    $stack->push_hook(sub {});

=head1 DESCRIPTION

This class is a stack of hooks to be called in the right order as
scopes go away.  The stack is only useful when inserted into C<%^H>
and will pop hooks as their "scope" is popped.  This is useful for
uninstalling or reinstalling subs in a namespace as a pragma goes
out of scope.

Due to how C<%^H> works, this class is only useful during the
compilation phase of a perl module and relies on the internals of how
perl handles references in C<%^H>.  This module is not a part of
autodie's public API.

=head2 Methods

=head3 new

  my $stack = autodie::Scope::GuardStack->new;

Creates a new C<autodie::Scope::GuardStack>.  The stack is initially
empty and must be inserted into C<%^H> by the creator.

=head3 push_hook

  $stack->push_hook(sub {});

Add a sub to the stack.  The sub will be called once the current
compile-time "scope" is left.  Multiple hooks can be added per scope

=head1 AUTHOR

Copyright 2013, Niels Thykier E<lt>niels@thykier.netE<gt>

=head1 LICENSE

This module is free software.  You may distribute it under the
same terms as Perl itself.
