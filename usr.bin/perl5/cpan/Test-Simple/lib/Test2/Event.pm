package Test2::Event;
use strict;
use warnings;

our $VERSION = '1.302194';

use Scalar::Util qw/blessed reftype/;
use Carp qw/croak/;

use Test2::Util::HashBase qw/trace -amnesty uuid -_eid -hubs/;
use Test2::Util::ExternalMeta qw/meta get_meta set_meta delete_meta/;
use Test2::Util qw/pkg_to_file gen_uid/;

use Test2::EventFacet::About();
use Test2::EventFacet::Amnesty();
use Test2::EventFacet::Assert();
use Test2::EventFacet::Control();
use Test2::EventFacet::Error();
use Test2::EventFacet::Info();
use Test2::EventFacet::Meta();
use Test2::EventFacet::Parent();
use Test2::EventFacet::Plan();
use Test2::EventFacet::Trace();
use Test2::EventFacet::Hub();

# Legacy tools will expect this to be loaded now
require Test2::Util::Trace;

my %LOADED_FACETS = (
    'about'   => 'Test2::EventFacet::About',
    'amnesty' => 'Test2::EventFacet::Amnesty',
    'assert'  => 'Test2::EventFacet::Assert',
    'control' => 'Test2::EventFacet::Control',
    'errors'  => 'Test2::EventFacet::Error',
    'info'    => 'Test2::EventFacet::Info',
    'meta'    => 'Test2::EventFacet::Meta',
    'parent'  => 'Test2::EventFacet::Parent',
    'plan'    => 'Test2::EventFacet::Plan',
    'trace'   => 'Test2::EventFacet::Trace',
    'hubs'    => 'Test2::EventFacet::Hub',
);

sub FACET_TYPES { sort values %LOADED_FACETS }

sub load_facet {
    my $class = shift;
    my ($facet) = @_;

    return $LOADED_FACETS{$facet} if exists $LOADED_FACETS{$facet};

    my @check = ($facet);
    if ('s' eq substr($facet, -1, 1)) {
        push @check => substr($facet, 0, -1);
    }
    else {
        push @check => $facet . 's';
    }

    my $found;
    for my $check (@check) {
        my $mod  = "Test2::EventFacet::" . ucfirst($facet);
        my $file = pkg_to_file($mod);
        next unless eval { require $file; 1 };
        $found = $mod;
        last;
    }

    return undef unless $found;
    $LOADED_FACETS{$facet} = $found;
}

sub causes_fail      { 0 }
sub increments_count { 0 }
sub diagnostics      { 0 }
sub no_display       { 0 }
sub subtest_id       { undef }

sub callback { }

sub terminate { () }
sub global    { () }
sub sets_plan { () }

sub summary { ref($_[0]) }

sub related {
    my $self = shift;
    my ($event) = @_;

    my $tracea = $self->trace  or return undef;
    my $traceb = $event->trace or return undef;

    my $uuida = $tracea->uuid;
    my $uuidb = $traceb->uuid;
    if ($uuida && $uuidb) {
        return 1 if $uuida eq $uuidb;
        return 0;
    }

    my $siga = $tracea->signature or return undef;
    my $sigb = $traceb->signature or return undef;

    return 1 if $siga eq $sigb;
    return 0;
}

sub add_hub {
    my $self = shift;
    unshift @{$self->{+HUBS}} => @_;
}

sub add_amnesty {
    my $self = shift;

    for my $am (@_) {
        $am = {%$am} if ref($am) ne 'ARRAY';
        $am = Test2::EventFacet::Amnesty->new($am);

        push @{$self->{+AMNESTY}} => $am;
    }
}

sub eid { $_[0]->{+_EID} ||= gen_uid() }

sub common_facet_data {
    my $self = shift;

    my %out;

    $out{about} = {package => ref($self) || undef};
    if (my $uuid = $self->uuid) {
        $out{about}->{uuid} = $uuid;
    }

    $out{about}->{eid} = $self->{+_EID} || $self->eid;

    if (my $trace = $self->trace) {
        $out{trace} = { %$trace };
    }

    if (my $hubs = $self->hubs) {
        $out{hubs} = $hubs;
    }

    $out{amnesty} = [map {{ %{$_} }} @{$self->{+AMNESTY}}]
        if $self->{+AMNESTY};

    if (my $meta = $self->meta_facet_data) {
        $out{meta} = $meta;
    }

    return \%out;
}

sub meta_facet_data {
    my $self = shift;

    my $key = Test2::Util::ExternalMeta::META_KEY();

    my $hash = $self->{$key} or return undef;
    return {%$hash};
}

sub facet_data {
    my $self = shift;

    my $out = $self->common_facet_data;

    $out->{about}->{details}    = $self->summary    || undef;
    $out->{about}->{no_display} = $self->no_display || undef;

    # Might be undef, we want to preserve that
    my $terminate = $self->terminate;
    $out->{control} = {
        global    => $self->global    || 0,
        terminate => $terminate,
        has_callback => $self->can('callback') == \&callback ? 0 : 1,
    };

    $out->{assert} = {
        no_debug => 1,                     # Legacy behavior
        pass     => $self->causes_fail ? 0 : 1,
        details  => $self->summary,
    } if $self->increments_count;

    $out->{parent} = {hid => $self->subtest_id} if $self->subtest_id;

    if (my @plan = $self->sets_plan) {
        $out->{plan} = {};

        $out->{plan}->{count}   = $plan[0] if defined $plan[0];
        $out->{plan}->{details} = $plan[2] if defined $plan[2];

        if ($plan[1]) {
            $out->{plan}->{skip} = 1 if $plan[1] eq 'SKIP';
            $out->{plan}->{none} = 1 if $plan[1] eq 'NO PLAN';
        }

        $out->{control}->{terminate} ||= 0 if $out->{plan}->{skip};
    }

    if ($self->causes_fail && !$out->{assert}) {
        $out->{errors} = [
            {
                tag     => 'FAIL',
                fail    => 1,
                details => $self->summary,
            }
        ];
    }

    my %IGNORE = (trace => 1, about => 1, control => 1);
    my $do_info = !grep { !$IGNORE{$_} } keys %$out;

    if ($do_info && !$self->no_display && $self->diagnostics) {
        $out->{info} = [
            {
                tag     => 'DIAG',
                debug   => 1,
                details => $self->summary,
            }
        ];
    }

    return $out;
}

sub facets {
    my $self = shift;
    my %out;

    my $data = $self->facet_data;
    my @errors = $self->validate_facet_data($data);
    die join "\n" => @errors if @errors;

    for my $facet (keys %$data) {
        my $class = $self->load_facet($facet);
        my $val = $data->{$facet};

        unless($class) {
            $out{$facet} = $val;
            next;
        }

        my $is_list = reftype($val) eq 'ARRAY' ? 1 : 0;
        if ($is_list) {
            $out{$facet} = [map { $class->new($_) } @$val];
        }
        else {
            $out{$facet} = $class->new($val);
        }
    }

    return \%out;
}

sub validate_facet_data {
    my $class_or_self = shift;
    my ($f, %params);

    $f = shift if @_ && (reftype($_[0]) || '') eq 'HASH';
    %params = @_;

    $f ||= $class_or_self->facet_data if blessed($class_or_self);
    croak "No facet data" unless $f;

    my @errors;

    for my $k (sort keys %$f) {
        my $fclass = $class_or_self->load_facet($k);

        push @errors => "Could not find a facet class for facet '$k'"
            if $params{require_facet_class} && !$fclass;

        next unless $fclass;

        my $v = $f->{$k};
        next unless defined($v); # undef is always fine

        my $is_list = $fclass->is_list();
        my $got_list = reftype($v) eq 'ARRAY' ? 1 : 0;

        push @errors => "Facet '$k' should be a list, but got a single item ($v)"
            if $is_list && !$got_list;

        push @errors => "Facet '$k' should not be a list, but got a a list ($v)"
            if $got_list && !$is_list;
    }

    return @errors;
}

sub nested {
    my $self = shift;

    Carp::cluck("Use of Test2::Event->nested() is deprecated, use Test2::Event->trace->nested instead")
        if $ENV{AUTHOR_TESTING};

    if (my $hubs = $self->{+HUBS}) {
        return $hubs->[0]->{nested} if @$hubs;
    }

    my $trace = $self->{+TRACE} or return undef;
    return $trace->{nested};
}

sub in_subtest {
    my $self = shift;

    Carp::cluck("Use of Test2::Event->in_subtest() is deprecated, use Test2::Event->trace->hid instead")
        if $ENV{AUTHOR_TESTING};

    my $hubs = $self->{+HUBS};
    if ($hubs && @$hubs) {
        return undef unless $hubs->[0]->{nested};
        return $hubs->[0]->{hid}
    }

    my $trace = $self->{+TRACE} or return undef;
    return undef unless $trace->{nested};
    return $trace->{hid};
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event - Base class for events

=head1 DESCRIPTION

Base class for all event objects that get passed through
L<Test2>.

=head1 SYNOPSIS

    package Test2::Event::MyEvent;
    use strict;
    use warnings;

    # This will make our class an event subclass (required)
    use base 'Test2::Event';

    # Add some accessors (optional)
    # You are not obligated to use HashBase, you can use any object tool you
    # want, or roll your own accessors.
    use Test2::Util::HashBase qw/foo bar baz/;

    # Use this if you want the legacy API to be written for you, for this to
    # work you will need to implement a facet_data() method.
    use Test2::Util::Facets2Legacy;

    # Chance to initialize some defaults
    sub init {
        my $self = shift;
        # no other args in @_

        $self->set_foo('xxx') unless defined $self->foo;

        ...
    }

    # This is the new way for events to convey data to the Test2 system
    sub facet_data {
        my $self = shift;

        # Get common facets such as 'about', 'trace' 'amnesty', and 'meta'
        my $facet_data = $self->common_facet_data();

        # Are you making an assertion?
        $facet_data->{assert} = {pass => 1, details => 'my assertion'};
        ...

        return $facet_data;
    }

    1;

=head1 METHODS

=head2 GENERAL

=over 4

=item $trace = $e->trace

Get a snapshot of the L<Test2::EventFacet::Trace> as it was when this event was
generated

=item $bool_or_undef = $e->related($e2)

Check if 2 events are related. In this case related means their traces share a
signature meaning they were created with the same context (or at the very least
by contexts which share an id, which is the same thing unless someone is doing
something very bad).

This can be used to reliably link multiple events created by the same tool. For
instance a failing test like C<ok(0, "fail"> will generate 2 events, one being
a L<Test2::Event::Ok>, the other being a L<Test2::Event::Diag>, both of these
events are related having been created under the same context and by the same
initial tool (though multiple tools may have been nested under the initial
one).

This will return C<undef> if the relationship cannot be checked, which happens
if either event has an incomplete or missing trace. This will return C<0> if
the traces are complete, but do not match. C<1> will be returned if there is a
match.

=item $e->add_amnesty({tag => $TAG, details => $DETAILS});

This can be used to add amnesty to this event. Amnesty only effects failing
assertions in most cases, but some formatters may display them for passing
assertions, or even non-assertions as well.

Amnesty will prevent a failed assertion from causing the overall test to fail.
In other words it marks a failure as expected and allowed.

B<Note:> This is how 'TODO' is implemented under the hood. TODO is essentially
amnesty with the 'TODO' tag. The details are the reason for the TODO.

=item $uuid = $e->uuid

If UUID tagging is enabled (See L<Test::API>) then any event that has made its
way through a hub will be tagged with a UUID. A newly created event will not
yet be tagged in most cases.

=item $class = $e->load_facet($name)

This method is used to load a facet by name (or key). It will attempt to load
the facet class, if it succeeds it will return the class it loaded. If it fails
it will return C<undef>. This caches the result at the class level so that
future calls will be faster.

The C<$name> variable should be the key used to access the facet in a facets
hashref. For instance the assertion facet has the key 'assert', the information
facet has the 'info' key, and the error facet has the key 'errors'. You may
include or omit the 's' at the end of the name, the method is smart enough to
try both the 's' and no-'s' forms, it will check what you provided first, and
if that is not found it will add or strip the 's and try again.

=item @classes = $e->FACET_TYPES()

=item @classes = Test2::Event->FACET_TYPES()

This returns a list of all facets that have been loaded using the
C<load_facet()> method. This will not return any classes that have not been
loaded, or have been loaded directly without a call to C<load_facet()>.

B<Note:> The core facet types are automatically loaded and populated in this
list.

=back

=head2 NEW API

=over 4

=item $hashref = $e->common_facet_data();

This can be used by subclasses to generate a starting facet data hashref. This
will populate the hashref with the trace, meta, amnesty, and about facets.
These facets are nearly always produced the same way for all events.

=item $hashref = $e->facet_data()

If you do not override this then the default implementation will attempt to
generate facets from the legacy API. This generation is limited only to what
the legacy API can provide. It is recommended that you override this method and
write out explicit facet data.

=item $hashref = $e->facets()

This takes the hashref from C<facet_data()> and blesses each facet into the
proper C<Test2::EventFacet::*> subclass. If no class can be found for any given
facet it will be passed along unchanged.

=item @errors = $e->validate_facet_data();

=item @errors = $e->validate_facet_data(%params);

=item @errors = $e->validate_facet_data(\%facets, %params);

=item @errors = Test2::Event->validate_facet_data(%params);

=item @errors = Test2::Event->validate_facet_data(\%facets, %params);

This method will validate facet data and return a list of errors. If no errors
are found this will return an empty list.

This can be called as an object method with no arguments, in which case the
C<facet_data()> method will be called to get the facet data to be validated.

When used as an object method the C<\%facet_data> argument may be omitted.

When used as a class method the C<\%facet_data> argument is required.

Remaining arguments will be slurped into a C<%params> hash.

Currently only 1 parameter is defined:

=over 4

=item require_facet_class => $BOOL

When set to true (default is false) this will reject any facets where a facet
class cannot be found. Normally facets without classes are assumed to be custom
and are ignored.

=back

=back

=head3 WHAT ARE FACETS?

Facets are how events convey their purpose to the Test2 internals and
formatters. An event without facets will have no intentional effect on the
overall test state, and will not be displayed at all by most formatters, except
perhaps to say that an event of an unknown type was seen.

Facets are produced by the C<facet_data()> subroutine, which you should
nearly-always override. C<facet_data()> is expected to return a hashref where
each key is the facet type, and the value is either a hashref with the data for
that facet, or an array of hashrefs. Some facets must be defined as single
hashrefs, some must be defined as an array of hashrefs, No facets allow both.

C<facet_data()> B<MUST NOT> bless the data it returns, the main hashref, and
nested facet hashrefs B<MUST> be bare, though items contained within each
facet may be blessed. The data returned by this method B<should> also be copies
of the internal data in order to prevent accidental state modification.

C<facets()> takes the data from C<facet_data()> and blesses it into the
C<Test2::EventFacet::*> packages. This is rarely used however, the EventFacet
packages are primarily for convenience and documentation. The EventFacet
classes are not used at all internally, instead the raw data is used.

Here is a list of facet types by package. The packages are not used internally,
but are where the documentation for each type is kept.

B<Note:> Every single facet type has the C<'details'> field. This field is
always intended for human consumption, and when provided, should explain the
'why' for the facet. All other fields are facet specific.

=over 4

=item about => {...}

L<Test2::EventFacet::About>

This contains information about the event itself such as the event package
name. The C<details> field for this facet is an overall summary of the event.

=item assert => {...}

L<Test2::EventFacet::Assert>

This facet is used if an assertion was made. The C<details> field of this facet
is the description of the assertion.

=item control => {...}

L<Test2::EventFacet::Control>

This facet is used to tell the L<Test2::Event::Hub> about special actions the
event causes. Things like halting all testing, terminating the current test,
etc. In this facet the C<details> field explains why any special action was
taken.

B<Note:> This is how bail-out is implemented.

=item meta => {...}

L<Test2::EventFacet::Meta>

The meta facet contains all the meta-data attached to the event. In this case
the C<details> field has no special meaning, but may be present if something
sets the 'details' meta-key on the event.

=item parent => {...}

L<Test2::EventFacet::Parent>

This facet contains nested events and similar details for subtests. In this
facet the C<details> field will typically be the name of the subtest.

=item plan => {...}

L<Test2::EventFacet::Plan>

This facet tells the system that a plan has been set. The C<details> field of
this is usually left empty, but when present explains why the plan is what it
is, this is most useful if the plan is to skip-all.

=item trace => {...}

L<Test2::EventFacet::Trace>

This facet contains information related to when and where the event was
generated. This is how the test file and line number of a failure is known.
This facet can also help you to tell if tests are related.

In this facet the C<details> field overrides the "failed at test_file.t line
42." message provided on assertion failure.

=item amnesty => [{...}, ...]

L<Test2::EventFacet::Amnesty>

The amnesty facet is a list instead of a single item, this is important as
amnesty can come from multiple places at once.

For each instance of amnesty the C<details> field explains why amnesty was
granted.

B<Note:> Outside of formatters amnesty only acts to forgive a failing
assertion.

=item errors => [{...}, ...]

L<Test2::EventFacet::Error>

The errors facet is a list instead of a single item, any number of errors can
be listed. In this facet C<details> describes the error, or may contain the raw
error message itself (such as an exception). In perl exception may be blessed
objects, as such the raw data for this facet may contain nested items which are
blessed.

Not all errors are considered fatal, there is a C<fail> field that must be set
for an error to cause the test to fail.

B<Note:> This facet is unique in that the field name is 'errors' while the
package is 'Error'. This is because this is the only facet type that is both a
list, and has a name where the plural is not the same as the singular. This may
cause some confusion, but I feel it will be less confusing than the
alternative.

=item info => [{...}, ...]

L<Test2::EventFacet::Info>

The 'info' facet is a list instead of a single item, any quantity of extra
information can be attached to an event. Some information may be critical
diagnostics, others may be simply commentary in nature, this is determined by
the C<debug> flag.

For this facet the C<details> flag is the info itself. This info may be a
string, or it may be a data structure to display. This is one of the few facet
types that may contain blessed items.

=back

=head2 LEGACY API

=over 4

=item $bool = $e->causes_fail

Returns true if this event should result in a test failure. In general this
should be false.

=item $bool = $e->increments_count

Should be true if this event should result in a test count increment.

=item $e->callback($hub)

If your event needs to have extra effects on the L<Test2::Hub> you can override
this method.

This is called B<BEFORE> your event is passed to the formatter.

=item $num = $e->nested

If this event is nested inside of other events, this should be the depth of
nesting. (This is mainly for subtests)

=item $bool = $e->global

Set this to true if your event is global, that is ALL threads and processes
should see it no matter when or where it is generated. This is not a common
thing to want, it is used by bail-out and skip_all to end testing.

=item $code = $e->terminate

This is called B<AFTER> your event has been passed to the formatter. This
should normally return undef, only change this if your event should cause the
test to exit immediately.

If you want this event to cause the test to exit you should return the exit
code here. Exit code of 0 means exit success, any other integer means exit with
failure.

This is used by L<Test2::Event::Plan> to exit 0 when the plan is
'skip_all'. This is also used by L<Test2::Event:Bail> to force the test
to exit with a failure.

This is called after the event has been sent to the formatter in order to
ensure the event is seen and understood.

=item $msg = $e->summary

This is intended to be a human readable summary of the event. This should
ideally only be one line long, but you can use multiple lines if necessary. This
is intended for human consumption. You do not need to make it easy for machines
to understand.

The default is to simply return the event package name.

=item ($count, $directive, $reason) = $e->sets_plan()

Check if this event sets the testing plan. It will return an empty list if it
does not. If it does set the plan it will return a list of 1 to 3 items in
order: Expected Test Count, Test Directive, Reason for directive.

=item $bool = $e->diagnostics

True if the event contains diagnostics info. This is useful because a
non-verbose harness may choose to hide events that are not in this category.
Some formatters may choose to send these to STDERR instead of STDOUT to ensure
they are seen.

=item $bool = $e->no_display

False by default. This will return true on events that should not be displayed
by formatters.

=item $id = $e->in_subtest

If the event is inside a subtest this should have the subtest ID.

=item $id = $e->subtest_id

If the event is a final subtest event, this should contain the subtest ID.

=back

=head1 THIRD PARTY META-DATA

This object consumes L<Test2::Util::ExternalMeta> which provides a consistent
way for you to attach meta-data to instances of this class. This is useful for
tools, plugins, and other extensions.

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
