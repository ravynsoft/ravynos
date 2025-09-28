package Test2::Event::Skip;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event::Ok; our @ISA = qw(Test2::Event::Ok) }
use Test2::Util::HashBase qw{reason};

sub init {
    my $self = shift;
    $self->SUPER::init;
    $self->{+EFFECTIVE_PASS} = 1;
}

sub causes_fail { 0 }

sub summary {
    my $self = shift;
    my $out = $self->SUPER::summary(@_);

    if (my $reason = $self->reason) {
        $out .= " (SKIP: $reason)";
    }
    else {
        $out .= " (SKIP)";
    }

    return $out;
}

sub extra_amnesty {
    my $self = shift;

    my @out;

    push @out => {
        tag       => 'TODO',
        details   => $self->{+TODO},
    } if defined $self->{+TODO};

    push @out => {
        tag       => 'skip',
        details   => $self->{+REASON},
        inherited => 0,
    };

    return @out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Skip - Skip event type

=head1 DESCRIPTION

Skip events bump test counts just like L<Test2::Event::Ok> events, but
they can never fail.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Skip;

    my $ctx = context();
    my $event = $ctx->skip($name, $reason);

or:

    my $ctx   = context();
    my $event = $ctx->send_event(
        'Skip',
        name   => $name,
        reason => $reason,
    );

=head1 ACCESSORS

=over 4

=item $reason = $e->reason

The original true/false value of whatever was passed into the event (but
reduced down to 1 or 0).

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

See F<http://www.perl.com/perl/misc/Artistic.html>

=cut
