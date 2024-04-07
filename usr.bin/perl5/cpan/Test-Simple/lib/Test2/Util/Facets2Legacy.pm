package Test2::Util::Facets2Legacy;
use strict;
use warnings;

our $VERSION = '1.302194';

use Carp qw/croak confess/;
use Scalar::Util qw/blessed/;

use base 'Exporter';
our @EXPORT_OK = qw{
    causes_fail
    diagnostics
    global
    increments_count
    no_display
    sets_plan
    subtest_id
    summary
    terminate
    uuid
};
our %EXPORT_TAGS = ( ALL => \@EXPORT_OK );

our $CYCLE_DETECT = 0;
sub _get_facet_data {
    my $in = shift;

    if (blessed($in) && $in->isa('Test2::Event')) {
        confess "Cycle between Facets2Legacy and $in\->facet_data() (Did you forget to override the facet_data() method?)"
            if $CYCLE_DETECT;

        local $CYCLE_DETECT = 1;
        return $in->facet_data;
    }

    return $in if ref($in) eq 'HASH';

    croak "'$in' Does not appear to be either a Test::Event or an EventFacet hashref";
}

sub causes_fail {
    my $facet_data = _get_facet_data(shift @_);

    return 1 if $facet_data->{errors} && grep { $_->{fail} } @{$facet_data->{errors}};

    if (my $control = $facet_data->{control}) {
        return 1 if $control->{halt};
        return 1 if $control->{terminate};
    }

    return 0 if $facet_data->{amnesty} && @{$facet_data->{amnesty}};
    return 1 if $facet_data->{assert} && !$facet_data->{assert}->{pass};
    return 0;
}

sub diagnostics {
    my $facet_data = _get_facet_data(shift @_);
    return 1 if $facet_data->{errors} && @{$facet_data->{errors}};
    return 0 unless $facet_data->{info} && @{$facet_data->{info}};
    return (grep { $_->{debug} } @{$facet_data->{info}}) ? 1 : 0;
}

sub global {
    my $facet_data = _get_facet_data(shift @_);
    return 0 unless $facet_data->{control};
    return $facet_data->{control}->{global};
}

sub increments_count {
    my $facet_data = _get_facet_data(shift @_);
    return $facet_data->{assert} ? 1 : 0;
}

sub no_display {
    my $facet_data = _get_facet_data(shift @_);
    return 0 unless $facet_data->{about};
    return $facet_data->{about}->{no_display};
}

sub sets_plan {
    my $facet_data = _get_facet_data(shift @_);
    my $plan = $facet_data->{plan} or return;
    my @out = ($plan->{count} || 0);

    if ($plan->{skip}) {
        push @out => 'SKIP';
        push @out => $plan->{details} if defined $plan->{details};
    }
    elsif ($plan->{none}) {
        push @out => 'NO PLAN'
    }

    return @out;
}

sub subtest_id {
    my $facet_data = _get_facet_data(shift @_);
    return undef unless $facet_data->{parent};
    return $facet_data->{parent}->{hid};
}

sub summary {
    my $facet_data = _get_facet_data(shift @_);
    return '' unless $facet_data->{about} && $facet_data->{about}->{details};
    return $facet_data->{about}->{details};
}

sub terminate {
    my $facet_data = _get_facet_data(shift @_);
    return undef unless $facet_data->{control};
    return $facet_data->{control}->{terminate};
}

sub uuid {
    my $in = shift;

    if ($CYCLE_DETECT) {
        if (blessed($in) && $in->isa('Test2::Event')) {
            my $meth = $in->can('uuid');
            $meth = $in->can('SUPER::uuid') if $meth == \&uuid;
            my $uuid = $in->$meth if $meth && $meth != \&uuid;
            return $uuid if $uuid;
        }

        return undef;
    }

    my $facet_data = _get_facet_data($in);
    return $facet_data->{about}->{uuid} if $facet_data->{about} && $facet_data->{about}->{uuid};

    return undef;
}

1;

=pod

=encoding UTF-8

=head1 NAME

Test2::Util::Facets2Legacy - Convert facet data to the legacy event API.

=head1 DESCRIPTION

This module exports several subroutines from the older event API (see
L<Test2::Event>). These subroutines can be used as methods on any object that
provides a custom C<facet_data()> method. These subroutines can also be used as
functions that take a facet data hashref as arguments.

=head1 SYNOPSIS

=head2 AS METHODS

    package My::Event;

    use Test2::Util::Facets2Legacy ':ALL';

    sub facet_data { return { ... } }

Then to use it:

    my $e = My::Event->new(...);

    my $causes_fail = $e->causes_fail;
    my $summary     = $e->summary;
    ....

=head2 AS FUNCTIONS

    use Test2::Util::Facets2Legacy ':ALL';

    my $f = {
        assert => { ... },
        info => [{...}, ...],
        control => {...},
        ...
    };

    my $causes_fail = causes_fail($f);
    my $summary     = summary($f);

=head1 NOTE ON CYCLES

When used as methods, all these subroutines call C<< $e->facet_data() >>. The
default C<facet_data()> method in L<Test2::Event> relies on the legacy methods
this module emulates in order to work. As a result of this it is very easy to
create infinite recursion bugs.

These methods have cycle detection and will throw an exception early if a cycle
is detected. C<uuid()> is currently the only subroutine in this library that
has a fallback behavior when cycles are detected.

=head1 EXPORTS

Nothing is exported by default. You must specify which methods to import, or
use the ':ALL' tag.

=over 4

=item $bool = $e->causes_fail()

=item $bool = causes_fail($f)

Check if the event or facets result in a failing state.

=item $bool = $e->diagnostics()

=item $bool = diagnostics($f)

Check if the event or facets contain any diagnostics information.

=item $bool = $e->global()

=item $bool = global($f)

Check if the event or facets need to be globally processed.

=item $bool = $e->increments_count()

=item $bool = increments_count($f)

Check if the event or facets make an assertion.

=item $bool = $e->no_display()

=item $bool = no_display($f)

Check if the event or facets should be rendered or hidden.

=item ($max, $directive, $reason) = $e->sets_plan()

=item ($max, $directive, $reason) = sets_plan($f)

Check if the event or facets set a plan, and return the plan details.

=item $id = $e->subtest_id()

=item $id = subtest_id($f)

Get the subtest id, if any.

=item $string = $e->summary()

=item $string = summary($f)

Get the summary of the event or facets hash, if any.

=item $undef_or_int = $e->terminate()

=item $undef_or_int = terminate($f)

Check if the event or facets should result in process termination, if so the
exit code is returned (which could be 0). undef is returned if no termination
is requested.

=item $uuid = $e->uuid()

=item $uuid = uuid($f)

Get the UUID of the facets or event.

B<Note:> This will fall back to C<< $e->SUPER::uuid() >> if a cycle is
detected and an event is used as the argument.

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
