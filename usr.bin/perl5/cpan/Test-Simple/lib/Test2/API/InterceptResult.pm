package Test2::API::InterceptResult;
use strict;
use warnings;

our $VERSION = '1.302194';

use Scalar::Util qw/blessed/;
use Test2::Util  qw/pkg_to_file/;
use Storable     qw/dclone/;
use Carp         qw/croak/;

use Test2::API::InterceptResult::Squasher;
use Test2::API::InterceptResult::Event;
use Test2::API::InterceptResult::Hub;

sub new {
    croak "Called a method that creates a new instance in void context" unless defined wantarray;
    my $class = shift;
    bless([@_], $class);
}

sub new_from_ref {
    croak "Called a method that creates a new instance in void context" unless defined wantarray;
    bless($_[1], $_[0]);
}

sub clone { blessed($_[0])->new(@{dclone($_[0])}) }

sub event_list { @{$_[0]} }

sub _upgrade {
    my $self = shift;
    my ($event, %params) = @_;

    my $blessed = blessed($event);

    my $upgrade_class = $params{upgrade_class} ||= 'Test2::API::InterceptResult::Event';

    return $event if $blessed && $event->isa($upgrade_class) && !$params{_upgrade_clone};

    my $fd = dclone($blessed ? $event->facet_data : $event);

    my $class = $params{result_class} ||= blessed($self);

    if (my $parent = $fd->{parent}) {
        $parent->{children} = $class->new_from_ref($parent->{children} || [])->upgrade(%params);
    }

    my $uc_file = pkg_to_file($upgrade_class);
    require($uc_file) unless $INC{$uc_file};
    return $upgrade_class->new(facet_data => $fd, result_class => $class);
}

sub hub {
    my $self = shift;

    my $hub = Test2::API::InterceptResult::Hub->new();
    $hub->process($_) for @$self;
    $hub->set_ended(1);

    return $hub;
}

sub state {
    my $self = shift;
    my %params = @_;

    my $hub = $self->hub;

    my $out = {
        map {($_ => scalar $hub->$_)} qw/count failed is_passing plan bailed_out skip_reason/
    };

    $out->{bailed_out} = $self->_upgrade($out->{bailed_out}, %params)->bailout_reason || 1
        if $out->{bailed_out};

    $out->{follows_plan} = $hub->check_plan;

    return $out;
}

sub upgrade {
    my $self = shift;
    my %params = @_;

    my @out = map { $self->_upgrade($_, %params, _upgrade_clone => 1) } @$self;

    return blessed($self)->new_from_ref(\@out)
        unless $params{in_place};

    @$self = @out;
    return $self;
}

sub squash_info {
    my $self = shift;
    my %params = @_;

    my @out;

    {
        my $squasher = Test2::API::InterceptResult::Squasher->new(events => \@out);
        # Clone to make sure we do not indirectly modify an existing one if it
        # is already upgraded
        $squasher->process($self->_upgrade($_, %params)->clone) for @$self;
        $squasher->flush_down();
    }

    return blessed($self)->new_from_ref(\@out)
        unless $params{in_place};

    @$self = @out;
    return $self;
}

sub asserts        { shift->grep(has_assert     => @_) }
sub subtests       { shift->grep(has_subtest    => @_) }
sub diags          { shift->grep(has_diags      => @_) }
sub notes          { shift->grep(has_notes      => @_) }
sub errors         { shift->grep(has_errors     => @_) }
sub plans          { shift->grep(has_plan       => @_) }
sub causes_fail    { shift->grep(causes_fail    => @_) }
sub causes_failure { shift->grep(causes_failure => @_) }

sub flatten         { shift->map(flatten        => @_) }
sub briefs          { shift->map(brief          => @_) }
sub summaries       { shift->map(summary        => @_) }
sub subtest_results { shift->map(subtest_result => @_) }
sub diag_messages   { shift->map(diag_messages  => @_) }
sub note_messages   { shift->map(note_messages  => @_) }
sub error_messages  { shift->map(error_messages => @_) }

no warnings 'once';

*map = sub {
    my $self = shift;
    my ($call, %params) = @_;

    my $args = $params{args} ||= [];

    return [map { local $_ = $self->_upgrade($_, %params); $_->$call(@$args) } @$self];
};

*grep = sub {
    my $self = shift;
    my ($call, %params) = @_;

    my $args = $params{args} ||= [];

    my @out = grep { local $_ = $self->_upgrade($_, %params); $_->$call(@$args) } @$self;

    return blessed($self)->new_from_ref(\@out)
        unless $params{in_place};

    @$self = @out;
    return $self;
};

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::API::InterceptResult - Representation of a list of events.

=head1 DESCRIPTION

This class represents a list of events, normally obtained using C<intercept()>
from L<Test2::API>.

This class is intended for people who with to verify the results of test tools
they write.

This class provides methods to normalize, summarize, or map the list of events.
The output of these operations makes verifying your testing tools and the
events they generate significantly easier. In most cases this spares you from
needing a deep understanding of the event/facet model.

=head1 SYNOPSIS

Usually you get an instance of this class when you use C<intercept()> from
L<Test2::API>.

    use Test2::V0;
    use Test2::API qw/intercept/;

    my $events = intercept {
        ok(1, "pass");
        ok(0, "fail");
        todo "broken" => sub { ok(0, "fixme") };
        plan 3;
    };

    # This is typically the most useful construct
    # squash_info() merges assertions and diagnostics that are associated
    #   (and returns a new instance with the modifications)
    # flatten() condenses the facet data into the key details for each event
    #   (and returns those structures in an arrayref)
    is(
        $events->squash_info->flatten(),
        [
            {
                causes_failure => 0,

                name => 'pass',
                pass => 1,

                trace_file => 'xxx.t',
                trace_line => 5,
            },
            {
                causes_failure => 1,

                name => 'fail',
                pass => 0,

                trace_file => 'xxx.t',
                trace_line => 6,

                # There can be more than one diagnostics message so this is
                # always an array when present.
                diag => ["Failed test 'fail'\nat xxx.t line 6."],
            },
            {
                causes_failure => 0,

                name => 'fixme',
                pass => 0,

                trace_file => 'xxx.t',
                trace_line => 7,

                # There can be more than one diagnostics message or todo
                # reason, so these are always an array when present.
                todo => ['broken'],

                # Diag message was turned into a note since the assertion was
                # TODO
                note => ["Failed test 'fixme'\nat xxx.t line 7."],
            },
            {
                causes_failure => 0,

                plan => 3,

                trace_file => 'xxx.t',
                trace_line => 8,
            },
        ],
        "Flattened events look like we expect"
    );

See L<Test2::API::InterceptResult::Event> for a full description of what
C<flatten()> provides for each event.

=head1 METHODS

Please note that no methods modify the original instance unless asked to do so.

=head2 CONSTRUCTION

=over 4

=item $events = Test2::API::InterceptResult->new(@EVENTS)

=item $events = Test2::API::InterceptResult->new_from_ref(\@EVENTS)

These create a new instance of Test2::API::InterceptResult from the given
events.

In the first form a new blessed arrayref is returned. In the 'new_from_ref'
form the reference you pass in is directly blessed.

Both of these will throw an exception if called in void context. This is mainly
important for the 'filtering' methods listed below which normally return a new
instance, they throw an exception in such cases as it probably means someone
meant to filter the original in place.

=item $clone = $events->clone()

Make a clone of the original events. Note that this is a deep copy, the entire
structure is duplicated. This uses C<dclone> from L<Storable> to achieve the
deep clone.

=back

=head2 NORMALIZATION

=over 4

=item @events = $events->event_list

This returns all the events in list-form.

=item $hub = $events->hub

This returns a new L<Test2::Hub> instance that has processed all the events
contained in the instance. This gives you a simple way to inspect the state
changes your events cause.

=item $state = $events->state

This returns a summary of the state of a hub after processing all the events.

    {
        count        => 2,      # Number of assertions made
        failed       => 1,      # Number of test failures seen
        is_passing   => 0,      # Boolean, true if the test would be passing
                                # after the events are processed.

        plan         => 2,      # Plan, either a number, undef, 'SKIP', or 'NO PLAN'
        follows_plan => 1,      # True if there is a plan and it was followed.
                                # False if the plan and assertions did not
                                # match, undef if no plan was present in the
                                # event list.

        bailed_out   => undef,  # undef unless there was a bail-out in the
                                # events in which case this will be a string
                                # explaining why there was a bailout, if no
                                # reason was given this will simply be set to
                                # true (1).

        skip_reason  => undef,  # If there was a skip_all this will give the
                                # reason.
    }


=item $new = $events->upgrade

=item $events->upgrade(in_place => $BOOL)

B<Note:> This normally returns a new instance, leaving the original unchanged.
If you call it in void context it will throw an exception. If you want to
modify the original you must pass in the C<< in_place => 1 >> option. You may
call this in void context when you ask to modify it in place. The in-place form
returns the instance that was modified so you can chain methods.

This will create a clone of the list where all events have been converted into
L<Test2::API::InterceptResult::Event> instances. This is extremely helpful as
L<Test2::API::InterceptResult::Event> provide a much better interface for
working with events. This allows you to avoid thinking about legacy event
types.

This also means your tests against the list are not fragile if the tool
you are testing randomly changes what type of events it generates (IE Changing
from L<Test2::Event::Ok> to L<Test2::Event::Pass>, both make assertions and
both will normalize to identical (or close enough)
L<Test2::API::InterceptResult::Event> instances.

Really you almost always want this, the only reason it is not done
automatically is to make sure the C<intercept()> tool is backwards compatible.

=item $new = $events->squash_info

=item $events->squash_info(in_place => $BOOL)

B<Note:> This normally returns a new instance, leaving the original unchanged.
If you call it in void context it will throw an exception. If you want to
modify the original you must pass in the C<< in_place => 1 >> option. You may
call this in void context when you ask to modify it in place. The in-place form
returns the instance that was modified so you can chain methods.

B<Note:> All events in the new or modified instance will be converted to
L<Test2::API::InterceptResult::Event> instances. There is no way to avoid this,
the squash operation requires the upgraded event class.

L<Test::More> and many other legacy tools would send notes, diags, and
assertions as seperate events. A subtest in L<Test::More> would send a note
with the subtest name, the subtest assertion, and finally a diagnostics event
if the subtest failed. This method will normalize things by squashing the note
and diag into the same event as the subtest (This is different from putting
them into the subtest, which is not what happens).

=back

=head2 FILTERING

B<Note:> These normally return new instances, leaving the originals unchanged.
If you call them in void context they will throw exceptions. If you want to
modify the originals you must pass in the C<< in_place => 1 >> option. You may
call these in void context when you ask to modify them in place. The in-place
forms return the instance that was modified so you can chain methods.

=head3 %PARAMS

These all accept the same 2 optional parameters:

=over 4

=item in_place => $BOOL

When true the method will modify the instance in place instead of returning a
new instance.

=item args => \@ARGS

If you wish to pass parameters into the event method being used for filtering,
you may do so here.

=back

=head3 METHODS

=over 4

=item $events->grep($CALL, %PARAMS)

This is essentially:

    Test2::API::InterceptResult->new(
        grep { $_->$CALL( @{$PARAMS{args}} ) } $self->event_list,
    );

B<Note:> that $CALL is called on an upgraded version of the event, though
the events returned will be the original ones, not the upgraded ones.

$CALL may be either the name of a method on
L<Test2::API::InterceptResult::Event>, or a coderef.

=item $events->asserts(%PARAMS)

This is essentially:

    $events->grep(has_assert => @{$PARAMS{args}})

It returns a new instance containing only the events that made assertions.

=item $events->subtests(%PARAMS)

This is essentially:

    $events->grep(has_subtest => @{$PARAMS{args}})

It returns a new instance containing only the events that have subtests.

=item $events->diags(%PARAMS)

This is essentially:

    $events->grep(has_diags => @{$PARAMS{args}})

It returns a new instance containing only the events that have diags.

=item $events->notes(%PARAMS)

This is essentially:

    $events->grep(has_notes => @{$PARAMS{args}})

It returns a new instance containing only the events that have notes.

=item $events->errors(%PARAMS)

B<Note:> Errors are NOT failing assertions. Failing assertions are a different
thing.

This is essentially:

    $events->grep(has_errors => @{$PARAMS{args}})

It returns a new instance containing only the events that have errors.

=item $events->plans(%PARAMS)

This is essentially:

    $events->grep(has_plan => @{$PARAMS{args}})

It returns a new instance containing only the events that set the plan.

=item $events->causes_fail(%PARAMS)

=item $events->causes_failure(%PARAMS)

These are essentially:

    $events->grep(causes_fail    => @{$PARAMS{args}})
    $events->grep(causes_failure => @{$PARAMS{args}})

B<Note:> C<causes_fail()> and C<causes_failure()> are both aliases for
eachother in events, so these methods are effectively aliases here as well.

It returns a new instance containing only the events that cause failure.

=back

=head2 MAPPING

These methods B<ALWAYS> return an arrayref.

B<Note:> No methods on L<Test2::API::InterceptResult::Event> alter the event in
any way.

B<Important Notes about Events>:

L<Test2::API::InterceptResult::Event> was tailor-made to be used in
event-lists. Most methods that are not applicable to a given event will return
an empty list, so you normally do not need to worry about unwanted C<undef>
values or exceptions being thrown. Mapping over event methods is an entended
use, so it works well to produce lists.

B<Exceptions to the rule:>

Some methods such as C<causes_fail> always return a boolean true or false for
all events. Any method prefixed with C<the_> conveys the intent that the event
should have exactly 1 of something, so those will throw an exception when that
condition is not true.

=over 4

=item $arrayref = $events->map($CALL, %PARAMS)

This is essentially:

    [ map { $_->$CALL(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

$CALL may be either the name of a method on
L<Test2::API::InterceptResult::Event>, or a coderef.

=item $arrayref = $events->flatten(%PARAMS)

This is essentially:

    [ map { $_->flatten(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of flattened structures.

See L<Test2::API::InterceptResult::Event> for details on what C<flatten()>
returns.

=item $arrayref = $events->briefs(%PARAMS)

This is essentially:

    [ map { $_->briefs(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of event briefs.

See L<Test2::API::InterceptResult::Event> for details on what C<brief()>
returns.

=item $arrayref = $events->summaries(%PARAMS)

This is essentially:

    [ map { $_->summaries(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of event summaries.

See L<Test2::API::InterceptResult::Event> for details on what C<summary()>
returns.

=item $arrayref = $events->subtest_results(%PARAMS)

This is essentially:

    [ map { $_->subtest_result(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of event summaries.

See L<Test2::API::InterceptResult::Event> for details on what
C<subtest_result()> returns.

=item $arrayref = $events->diag_messages(%PARAMS)

This is essentially:

    [ map { $_->diag_messages(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of diagnostic messages (strings).

See L<Test2::API::InterceptResult::Event> for details on what
C<diag_messages()> returns.

=item $arrayref = $events->note_messages(%PARAMS)

This is essentially:

    [ map { $_->note_messages(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of notification messages (strings).

See L<Test2::API::InterceptResult::Event> for details on what
C<note_messages()> returns.

=item $arrayref = $events->error_messages(%PARAMS)

This is essentially:

    [ map { $_->error_messages(@{ $PARAMS{args} }) } $events->upgrade->event_list ];

It returns a new list of error messages (strings).

See L<Test2::API::InterceptResult::Event> for details on what
C<error_messages()> returns.

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
