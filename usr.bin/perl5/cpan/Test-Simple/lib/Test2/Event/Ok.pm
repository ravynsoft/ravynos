package Test2::Event::Ok;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw{
    pass effective_pass name todo
};

sub init {
    my $self = shift;

    # Do not store objects here, only true or false
    $self->{+PASS} = $self->{+PASS} ? 1 : 0;
    $self->{+EFFECTIVE_PASS} = $self->{+PASS} || (defined($self->{+TODO}) ? 1 : 0);
}

{
    no warnings 'redefine';
    sub set_todo {
        my $self = shift;
        my ($todo) = @_;
        $self->{+TODO} = $todo;
        $self->{+EFFECTIVE_PASS} = defined($todo) ? 1 : $self->{+PASS};
    }
}

sub increments_count { 1 };

sub causes_fail { !$_[0]->{+EFFECTIVE_PASS} }

sub summary {
    my $self = shift;

    my $name = $self->{+NAME} || "Nameless Assertion";

    my $todo = $self->{+TODO};
    if ($todo) {
        $name .= " (TODO: $todo)";
    }
    elsif (defined $todo) {
        $name .= " (TODO)"
    }

    return $name;
}

sub extra_amnesty {
    my $self = shift;
    return unless defined($self->{+TODO}) || ($self->{+EFFECTIVE_PASS} && !$self->{+PASS});
    return {
        tag       => 'TODO',
        details   => $self->{+TODO},
    };
}

sub facet_data {
    my $self = shift;

    my $out = $self->common_facet_data;

    $out->{assert}  = {
        no_debug => 1,                # Legacy behavior
        pass     => $self->{+PASS},
        details  => $self->{+NAME},
    };

    if (my @exra_amnesty = $self->extra_amnesty) {
        my %seen;

        # It is possible the extra amnesty can be a duplicate, so filter it.
        $out->{amnesty} = [
            grep { !$seen{defined($_->{tag}) ? $_->{tag} : ''}->{defined($_->{details}) ? $_->{details} : ''}++ }
                @exra_amnesty,
                @{$out->{amnesty}},
        ];
    }

    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Ok - Ok event type

=head1 DESCRIPTION

Ok events are generated whenever you run a test that produces a result.
Examples are C<ok()>, and C<is()>.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Ok;

    my $ctx = context();
    my $event = $ctx->ok($bool, $name, \@diag);

or:

    my $ctx   = context();
    my $event = $ctx->send_event(
        'Ok',
        pass => $bool,
        name => $name,
    );

=head1 ACCESSORS

=over 4

=item $rb = $e->pass

The original true/false value of whatever was passed into the event (but
reduced down to 1 or 0).

=item $name = $e->name

Name of the test.

=item $b = $e->effective_pass

This is the true/false value of the test after TODO and similar modifiers are
taken into account.

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
