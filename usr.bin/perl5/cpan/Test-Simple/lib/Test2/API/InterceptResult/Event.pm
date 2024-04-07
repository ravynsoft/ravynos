package Test2::API::InterceptResult::Event;
use strict;
use warnings;

our $VERSION = '1.302194';

use List::Util   qw/first/;
use Test2::Util  qw/pkg_to_file/;
use Scalar::Util qw/reftype blessed/;

use Storable qw/dclone/;
use Carp     qw/confess croak/;

use Test2::API::InterceptResult::Facet;
use Test2::API::InterceptResult::Hub;

use Test2::Util::HashBase qw{
    +causes_failure
    <facet_data
    <result_class
};

my %FACETS;
BEGIN {
    local $@;
    local *plugins;
    if (eval { require Module::Pluggable; 1 }) {
        Module::Pluggable->import(
            # We will replace the sub later
            require          => 1,
            on_require_error => sub { 1 },
            search_path      => ['Test2::EventFacet'],
            max_depth        => 3,
            min_depth        => 3,
        );

        for my $facet_type (__PACKAGE__->plugins) {
            my ($key, $list);
            eval {
                $key  = $facet_type->facet_key;
                $list = $facet_type->is_list;
            };
            next unless $key && defined($list);

            $FACETS{$key} = {list => $list, class => $facet_type, loaded => 1};
        }
    }

    $FACETS{__GENERIC__} = {class => 'Test2::API::InterceptResult::Facet', loaded => 1};
}

sub facet_map { \%FACETS }

sub facet_info {
    my $facet = pop;

    return $FACETS{$facet} if exists $FACETS{$facet};

    my $mname = ucfirst(lc($facet));
    $mname =~ s/s$//;

    for my $name ($mname, "${mname}s") {
        my $file  = "Test2/EventFacet/$name.pm";
        my $class = "Test2::EventFacet::$name";

        local $@;
        my $ok = eval {
            require $file;

            my $key = $class->facet_key;
            my $list = $class->is_list;

            $FACETS{$key} = {list => $list, class => $class, loaded => 1};
            $FACETS{$facet} = $FACETS{$key} if $facet ne $key;

            1;
        };

        return $FACETS{$facet} if $ok && $FACETS{$facet};
    }

    return $FACETS{$facet} = $FACETS{__GENERIC__};
}

sub init {
    my $self = shift;

    my $rc = $self->{+RESULT_CLASS} ||= 'Test2::API::InterceptResult';
    my $rc_file = pkg_to_file($rc);
    require($rc_file) unless $INC{$rc_file};

    my $fd = $self->{+FACET_DATA} ||= {};

    for my $facet (keys %$fd) {
        my $finfo = $self->facet_info($facet);
        my $is_list = $finfo->{list};
        next unless defined $is_list;

        my $type = reftype($fd->{$facet});

        if ($is_list) {
            confess "Facet '$facet' is a list facet, but got '$type' instead of an arrayref"
                unless $type eq 'ARRAY';

            for my $item (@{$fd->{$facet}}) {
                my $itype = reftype($item);
                next if $itype eq 'HASH';

                confess "Got item type '$itype' in list-facet '$facet', all items must be hashrefs";
            }
        }
        else {
            confess "Facet '$facet' is an only-one facet, but got '$type' instead of a hashref"
                unless $type eq 'HASH';
        }
    }
}

sub clone {
    my $self = shift;
    my $class = blessed($self);

    my %data = %$self;

    $data{+FACET_DATA} = dclone($data{+FACET_DATA});

    return bless(\%data, $class);
}

sub _facet_class {
    my $self = shift;
    my ($name) = @_;

    my $spec  = $self->facet_info($name);
    my $class = $spec->{class};
    unless ($spec->{loaded}) {
        my $file = pkg_to_file($class);
        require $file unless $INC{$file};
        $spec->{loaded} = 1;
    }

    return $class;
}

sub the_facet {
    my $self = shift;
    my ($name) = @_;

    return undef unless defined $self->{+FACET_DATA}->{$name};

    my $data = $self->{+FACET_DATA}->{$name};

    my $type = reftype($data) or confess "Facet '$name' has a value that is not a reference, this should not happen";

    return $self->_facet_class($name)->new(%{dclone($data)})
        if $type eq 'HASH';

    if ($type eq 'ARRAY') {
        return undef unless @$data;
        croak "'the_facet' called for facet '$name', but '$name' has '" . @$data . "' items" if @$data != 1;
        return $self->_facet_class($name)->new(%{dclone($data->[0])});
    }

    die "Invalid facet data type: $type";
}

sub facet {
    my $self = shift;
    my ($name) = @_;

    return () unless exists $self->{+FACET_DATA}->{$name};

    my $data = $self->{+FACET_DATA}->{$name};

    my $type = reftype($data) or confess "Facet '$name' has a value that is not a reference, this should not happen";

    my @out;
    @out = ($data)  if $type eq 'HASH';
    @out = (@$data) if $type eq 'ARRAY';

    my $class = $self->_facet_class($name);

    return map { $class->new(%{dclone($_)}) } @out;
}

sub causes_failure {
    my $self = shift;

    return $self->{+CAUSES_FAILURE}
        if exists $self->{+CAUSES_FAILURE};

    my $hub = Test2::API::InterceptResult::Hub->new();
    $hub->process($self);

    return $self->{+CAUSES_FAILURE} = ($hub->is_passing ? 0 : 1);
}

sub causes_fail { shift->causes_failure }

sub trace         { $_[0]->facet('trace') }
sub the_trace     { $_[0]->the_facet('trace') }
sub frame         { my $t = $_[0]->the_trace or return undef; $t->{frame} || undef }
sub trace_details { my $t = $_[0]->the_trace or return undef; $t->{details} || undef }
sub trace_package { my $f = $_[0]->frame or return undef; $f->[0] || undef }
sub trace_file    { my $f = $_[0]->frame or return undef; $f->[1] || undef }
sub trace_line    { my $f = $_[0]->frame or return undef; $f->[2] || undef }
sub trace_subname { my $f = $_[0]->frame or return undef; $f->[3] || undef }
sub trace_tool    { my $f = $_[0]->frame or return undef; $f->[3] || undef }

sub trace_signature { my $t = $_[0]->the_trace or return undef; Test2::EventFacet::Trace::signature($t) || undef }

sub brief {
    my $self = shift;

    my @try = qw{
        bailout_brief
        error_brief
        assert_brief
        plan_brief
    };

    for my $meth (@try) {
        my $got = $self->$meth or next;
        return $got;
    }

    return;
}

sub flatten {
    my $self = shift;
    my %params = @_;

    my $todo = {%{$self->{+FACET_DATA}}};
    delete $todo->{hubs};
    delete $todo->{meta};
    delete $todo->{trace};

    my $out = $self->summary;
    delete $out->{brief};
    delete $out->{facets};
    delete $out->{trace_tool};
    delete $out->{trace_details} unless defined($out->{trace_details});

    for my $tagged (grep { my $finfo = $self->facet_info($_); $finfo->{list} && $finfo->{class}->can('tag') } keys %FACETS, keys %$todo) {
        my $set = delete $todo->{$tagged} or next;

        my $fd = $self->{+FACET_DATA};
        my $has_assert = $self->has_assert;
        my $has_parent = $self->has_subtest;
        my $has_fatal_error = $self->has_errors && grep { $_->{fail} } $self->errors;

        next if $tagged eq 'amnesty' && !($has_assert || $has_parent || $has_fatal_error);

        for my $item (@$set) {
            push @{$out->{lc($item->{tag})}} => $item->{fail} ? "FATAL: $item->{details}" : $item->{details};
        }
    }

    if (my $assert = delete $todo->{assert}) {
        $out->{pass} = $assert->{pass};
        $out->{name} = $assert->{details};
    }

    if (my $parent = delete $todo->{parent}) {
        delete $out->{subtest}->{bailed_out}  unless defined $out->{subtest}->{bailed_out};
        delete $out->{subtest}->{skip_reason} unless defined $out->{subtest}->{skip_reason};

        if (my $res = $self->subtest_result) {
            my $state = $res->state;
            delete $state->{$_} for grep { !defined($state->{$_}) } keys %$state;
            $out->{subtest} = $state;
            $out->{subevents} = $res->flatten(%params)
                if $params{include_subevents};
        }
    }

    if (my $control = delete $todo->{control}) {
        if ($control->{halt}) {
            $out->{bailed_out} = $control->{details} || 1;
        }
        elsif(defined $control->{details}) {
            $out->{control} = $control->{details};
        }
    }

    if (my $plan = delete $todo->{plan}) {
        $out->{plan} = $self->plan_brief;
        $out->{plan} =~ s/^PLAN\s*//;
    }

    for my $other (keys %$todo) {
        my $data = $todo->{$other} or next;

        if (reftype($data) eq 'ARRAY') {
            if (!$out->{$other} || reftype($out->{$other}) eq 'ARRAY') {
                for my $item (@$data) {
                    push @{$out->{$other}} => $item->{details} if defined $item->{details};
                }
            }
        }
        else {
            $out->{$other} = $data->{details} if defined($data->{details}) && !defined($out->{$other});
        }
    }

    if (my $fields = $params{fields}) {
        $out = { map {exists($out->{$_}) ? ($_ => $out->{$_}) : ()} @$fields };
    }

    if (my $remove = $params{remove}) {
        delete $out->{$_} for @$remove;
    }

    return $out;
}

sub summary {
    my $self = shift;
    my %params = @_;

    my $out = {
        brief => $self->brief || '',

        causes_failure => $self->causes_failure,

        trace_line    => $self->trace_line,
        trace_file    => $self->trace_file,
        trace_tool    => $self->trace_subname,
        trace_details => $self->trace_details,

        facets => [ sort keys(%{$self->{+FACET_DATA}}) ],
    };

    if (my $fields = $params{fields}) {
        $out = { map {exists($out->{$_}) ? ($_ => $out->{$_}) : ()} @$fields };
    }

    if (my $remove = $params{remove}) {
        delete $out->{$_} for @$remove;
    }

    return $out;
}

sub has_assert { $_[0]->{+FACET_DATA}->{assert} ? 1 : 0 }
sub the_assert { $_[0]->the_facet('assert') }
sub assert     { $_[0]->facet('assert') }

sub assert_brief {
    my $self = shift;

    my $fd = $self->{+FACET_DATA};
    my $as = $fd->{assert} or return;
    my $am = $fd->{amnesty};

    my $out = $as->{pass} ? "PASS" : "FAIL";
    $out .= " with amnesty" if $am;
    return $out;
}

sub has_subtest { $_[0]->{+FACET_DATA}->{parent} ? 1 : 0 }
sub the_subtest { $_[0]->the_facet('parent') }
sub subtest     { $_[0]->facet('parent') }

sub subtest_result {
    my $self = shift;

    my $parent = $self->{+FACET_DATA}->{parent} or return;
    my $children = $parent->{children} || [];

    $children = $self->{+RESULT_CLASS}->new(@$children)->upgrade
        unless blessed($children) && $children->isa($self->{+RESULT_CLASS});

    return $children;
}

sub has_bailout { $_[0]->bailout ? 1 : 0 }
sub the_bailout { my ($b) = $_[0]->bailout; $b }

sub bailout {
    my $self = shift;
    my $control = $self->{+FACET_DATA}->{control} or return;
    return $control if $control->{halt};
    return;
}

sub bailout_brief {
    my $self = shift;
    my $bo = $self->bailout or return;

    my $reason = $bo->{details} or return "BAILED OUT";
    return "BAILED OUT: $reason";
}

sub bailout_reason {
    my $self = shift;
    my $bo = $self->bailout or return;
    return $bo->{details} || '';
}

sub has_plan { $_[0]->{+FACET_DATA}->{plan} ? 1 : 0 }
sub the_plan { $_[0]->the_facet('plan') }
sub plan     { $_[0]->facet('plan') }

sub plan_brief {
    my $self = shift;

    my $plan = $self->{+FACET_DATA}->{plan} or return;

    my $base = $self->_plan_brief($plan);

    my $reason = $plan->{details} or return $base;
    return "$base: $reason";
}

sub _plan_brief {
    my $self = shift;
    my ($plan) = @_;

    return 'NO PLAN' if $plan->{none};
    return "SKIP ALL" if $plan->{skip} || !$plan->{count};
    return "PLAN $plan->{count}";
}

sub has_amnesty     { $_[0]->{+FACET_DATA}->{amnesty} ? 1 : 0 }
sub the_amnesty     { $_[0]->the_facet('amnesty') }
sub amnesty         { $_[0]->facet('amnesty') }
sub amnesty_reasons { map { $_->{details} } $_[0]->amnesty }

sub has_todos    { &first(sub { uc($_->{tag}) eq 'TODO' }, $_[0]->amnesty) ? 1 : 0 }
sub todos        {       grep { uc($_->{tag}) eq 'TODO' }  $_[0]->amnesty          }
sub todo_reasons {       map  { $_->{details} || 'TODO' }  $_[0]->todos            }

sub has_skips    { &first(sub { uc($_->{tag}) eq 'SKIP' }, $_[0]->amnesty) ? 1 : 0 }
sub skips        {       grep { uc($_->{tag}) eq 'SKIP' }  $_[0]->amnesty          }
sub skip_reasons {       map  { $_->{details} || 'SKIP' }  $_[0]->skips            }

my %TODO_OR_SKIP = (SKIP => 1, TODO => 1);
sub has_other_amnesty     { &first( sub { !$TODO_OR_SKIP{uc($_->{tag})}            }, $_[0]->amnesty) ? 1 : 0 }
sub other_amnesty         {        grep { !$TODO_OR_SKIP{uc($_->{tag})}            }  $_[0]->amnesty          }
sub other_amnesty_reasons {        map  { $_->{details} ||  $_->{tag} || 'AMNESTY' }  $_[0]->other_amnesty    }

sub has_errors     { $_[0]->{+FACET_DATA}->{errors} ? 1 : 0 }
sub the_errors     { $_[0]->the_facet('errors') }
sub errors         { $_[0]->facet('errors') }
sub error_messages { map { $_->{details} || $_->{tag} || 'ERROR' } $_[0]->errors }

sub error_brief {
    my $self = shift;

    my $errors = $self->{+FACET_DATA}->{errors} or return;

    my $base = @$errors > 1 ? "ERRORS" : "ERROR";

    return $base unless @$errors;

    my ($msg, @extra) = split /[\n\r]+/, $errors->[0]->{details};

    my $out = "$base: $msg";

    $out .= " [...]" if @extra || @$errors > 1;

    return $out;
}

sub has_info      { $_[0]->{+FACET_DATA}->{info} ? 1 : 0 }
sub the_info      { $_[0]->the_facet('info') }
sub info          { $_[0]->facet('info') }
sub info_messages { map { $_->{details} } $_[0]->info }

sub has_diags { &first(sub { uc($_->{tag}) eq 'DIAG' }, $_[0]->info) ? 1 : 0 }
sub diags         {   grep { uc($_->{tag}) eq 'DIAG' }  $_[0]->info          }
sub diag_messages {   map  { $_->{details} || 'DIAG' }  $_[0]->diags         }

sub has_notes { &first(sub { uc($_->{tag}) eq 'NOTE' }, $_[0]->info) ? 1 : 0 }
sub notes         {   grep { uc($_->{tag}) eq 'NOTE' }  $_[0]->info          }
sub note_messages {   map  { $_->{details} || 'NOTE' }  $_[0]->notes         }

my %NOTE_OR_DIAG = (NOTE => 1, DIAG => 1);
sub has_other_info { &first(sub { !$NOTE_OR_DIAG{uc($_->{tag})}         }, $_[0]->info) ? 1 : 0 }
sub other_info          {  grep { !$NOTE_OR_DIAG{uc($_->{tag})}         }  $_[0]->info          }
sub other_info_messages {  map  { $_->{details} ||  $_->{tag} || 'INFO' }  $_[0]->other_info    }

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::API::InterceptResult::Event - Representation of an event for use in
testing other test tools.

=head1 DESCRIPTION

C<intercept { ... }> from L<Test2::API> returns an instance of
L<Test2::API::InterceptResult> which is a blessed arrayref of
L<Test2::API::InterceptResult::Event> objects.

This POD documents the methods of these events, which are mainly provided for
you to use when testing your test tools.

=head1 SYNOPSIS

    use Test2::V0;
    use Test2::API qw/intercept/;

    my $events = intercept {
        ok(1, "A passing assertion");
        plan(1);
    };

    # This will convert all events into instances of
    # Test2::API::InterceptResult::Event. Until we do this they are the
    # original Test::Event::* instances
    $events->upgrade(in_place => 1);

    # Now we can get individual events in this form
    my $assert = $events->[0];
    my $plan   = $events->[1];

    # Or we can operate on all events at once:
    my $flattened = $events->flatten;
    is(
        $flattened,
        [
          {
            causes_failure => 0,

            name => 'A passing assertion',
            pass => 1,

            trace_file => 'xxx.t',
            trace_line => 5,
          },
          {
            causes_failure => 0,

            plan => 1,

            trace_file => 'xxx.t',
            trace_line => 6,
          },
        ],
        "Flattened both events and returned an arrayref of the results
    );

=head1 METHODS

=head2 !!! IMPORTANT NOTES ON DESIGN !!!

Please pay attention to what these return, many return a scalar when
applicable or an empty list when not (as opposed to undef). Many also always
return a list of 0 or more items. Some always return a scalar. Note that none
of the methods care about context, their behavior is consistent regardless of
scalar, list, or void context.

This was done because this class was specifically designed to be used in a list
and generate more lists in bulk operations. Sometimes in a map you want nothing
to show up for the event, and you do not want an undef in its place. In general
single event instances are not going to be used alone, though that is allowed.

As a general rule any method prefixed with C<the_> implies the event should
have exactly 1 of the specified item, and and exception will be thrown if there
are 0, or more than 1 of the item.

=head2 ATTRIBUTES

=over 4

=item $hashref = $event->facet_data

This will return the facet data hashref, which is all Test2 cares about for any
given event.

=item $class = $event->result_class

This is normally L<Test2::API::InterceptResult>. This is set at construction so
that subtest results can be turned into instances of it on demand.

=back

=head2 DUPLICATION

=over 4

=item $copy = $event->clone

Create a deep copy of the event. Modifying either event will not effect the
other.

=back

=head2 CONDENSED MULTI-FACET DATA

=over 4

=item $bool = $event->causes_failure

=item $bool = $event->causes_fail

These are both aliases of the same functionality.

This will always return either a true value, or a false value. This never
returns a list.

This method may be relatively slow (still super fast) because it determines
pass or fail by creating an instance of L<Test2::Hub> and asking it to process
the event, and then asks the hub for its pass/fail state. This is slower than
bulding in logic to do the check, but it is more reliable as it will always
tell you what the hub thinks, so the logic will never be out of date relative
to the Test2 logic that actually cares.

=item STRING_OR_EMPTY_LIST = $event->brief

Not all events have a brief, some events are not rendered by the formatter,
others have no "brief" data worth seeing. When this is the case an empty list
is returned. This is done intentionally so it can be used in a map operation
without having C<undef> being included in the result.

When a brief can be generated it is always a single 1-line string, and is
returned as-is, not in a list.

Possible briefs:

    # From control facets
    "BAILED OUT"
    "BAILED OUT: $why"

    # From error facets
    "ERROR"
    "ERROR: $message"
    "ERROR: $partial_message [...]"
    "ERRORS: $first_error_message [...]"

    # From assert facets
    "PASS"
    "FAIL"
    "PASS with amnesty"
    "FAIL with amnesty"

    # From plan facets
    "PLAN $count"
    "NO PLAN"
    "SKIP ALL"
    "SKIP ALL: $why"

Note that only the first applicable brief is returned. This is essnetially a
poor-mans TAP that only includes facets that could (but not necessarily do)
cause a failure.

=item $hashref = $event->flatten

=item $hashref = $event->flatten(include_subevents => 1)

This ALWAYS returns a hashref. This puts all the most useful data for the most
interesting facets into a single hashref for easy validation.

If there are no meaningful facets this will return an empty hashref.

If given the 'include_subevents' parameter it will also include subtest data:

Here is a list of EVERY possible field. If a field is not applicable it will
not be present.

=over 4

=item always present

        causes_failure => 1,    # Always present

=item Present if the event has a trace facet

        trace_line    => 42,
        trace_file    => 'Foo/Bar.pm',
        trace_details => 'Extra trace details',    # usually not present

=item If an assertion is present

        pass => 0,
        name => "1 + 1 = 2, so math works",

=item If a plan is present:

        plan => $count_or_SKIP_ALL_or_NO_PLAN,

=item If amnesty facets are present

You get an array for each type that is present.

        todo => [    # Yes you could be under multiple todos, this will list them all.
            "I will fix this later",
            "I promise to fix these",
        ],

        skip => ["This will format the main drive, do not run"],

        ... => ["Other amnesty"]

=item If Info (note/diag) facets are present

You get an arrayref for any that are present, the key is not defined if they are not present.

        diag => [
            "Test failed at Foo/Bar.pm line 42",
            "You forgot to tie your boots",
        ],

        note => ["Your boots are red"],

        ...  => ["Other info"],

=item If error facets are present

Always an arrayref

        error => [
            "non fatal error (does not cause test failure, just an FYI",
            "FATAL: This is a fatal error (causes failure)",
        ],

        # Errors can have alternative tags, but in practice are always 'error',
        # listing this for completeness.
        ... => [ ... ]

=item Present if the event is a subtest

        subtest => {
            count      => 2,    # Number of assertions made
            failed     => 1,    # Number of test failures seen
            is_passing => 0,    # Boolean, true if the test would be passing
                                # after the events are processed.

            plan         => 2,  # Plan, either a number, undef, 'SKIP', or 'NO PLAN'
            follows_plan => 1,  # True if there is a plan and it was followed.
                                # False if the plan and assertions did not
                                # match, undef if no plan was present in the
                                # event list.

            bailed_out => "foo",    # if there was a bail-out in the
                                    # events in this will be a string explaining
                                    # why there was a bailout, if no reason was
                                    # given this will simply be set to true (1).

            skip_reason => "foo",   # If there was a skip_all this will give the
                                    # reason.
        },

if C<< (include_subtest => 1) >> was provided as a parameter then the following
will be included. This is the result of turning all subtest child events into
an L<Test2::API::InterceptResult> instance and calling the C<flatten> method on
it.

        subevents => Test2::API::InterceptResult->new(@child_events)->flatten(...),

=item If a bail-out is being requested

If no reason was given this will be set to 1.

        bailed_out => "reason",

=back

=item $hashref = $event->summary()

This returns a limited summary. See C<flatten()>, which is usually a better
option.

    {
        brief => $event->brief || '',

        causes_failure => $event->causes_failure,

        trace_line    => $event->trace_line,
        trace_file    => $event->trace_file,
        trace_tool    => $event->trace_subname,
        trace_details => $event->trace_details,

        facets => [ sort keys(%{$event->{+FACET_DATA}}) ],
    }

=back

=head2 DIRECT ARBITRARY FACET ACCESS

=over 4

=item @list_of_facets = $event->facet($name)

This always returns a list of 0 or more items. This fetches the facet instances
from the event. For facets like 'assert' this will always return 0 or 1
item. For events like 'info' (diags, notes) this will return 0 or more
instances, once for each instance of the facet.

These will be blessed into the proper L<Test2::EventFacet> subclass. If no
subclass can be found it will be blessed as an
L<Test2::API::InterceptResult::Facet> generic facet class.

=item $undef_or_facet = $event->the_facet($name)

If you know you will have exactly 1 instance of a facet you can call this.

If you are correct and there is exactly one instance of the facet it will
always return the hashref.

If there are 0 instances of the facet this will reutrn undef, not an empty
list.

If there are more than 1 instance this will throw an exception because your
assumption was incorrect.

=back

=head2 TRACE FACET

=over 4

=item @list_of_facets = $event->trace

TODO

=item $undef_or_hashref = $event->the_trace

This returns the trace hashref, or undef if it is not present.

=item $undef_or_arrayref = $event->frame

If a trace is present, and has a caller frame, this will be an arrayref:

    [$package, $file, $line, $subname]

If the trace is not present, or has no caller frame this will return undef.

=item $undef_or_string = $event->trace_details

This is usually undef, but occasionally has a string that overrides the
file/line number debugging a trace usually provides on test failure.

=item $undef_or_string = $event->trace_package

Same as C<(caller())[0]>, the first element of the trace frame.

Will be undef if not present.

=item $undef_or_string = $event->trace_file

Same as C<(caller())[1]>, the second element of the trace frame.

Will be undef if not present.

=item $undef_or_integer = $event->trace_line

Same as C<(caller())[2]>, the third element of the trace frame.

Will be undef if not present.

=item $undef_or_string = $event->trace_subname

=item $undef_or_string = $event->trace_tool

Aliases for the same thing

Same as C<(caller($level))[4]>, the fourth element of the trace frame.

Will be undef if not present.

=item $undef_or_string = $event->trace_signature

A string that is a unique signature for the trace. If a single context
generates multiple events they will all have the same signature. This can be
used to tie assertions and diagnostics sent as seperate events together after
the fact.

=back

=head2 ASSERT FACET

=over 4

=item $bool = $event->has_assert

Returns true if the event has an assert facet, false if it does not.

=item $undef_or_hashref = $event->the_assert

Returns the assert facet if present, undef if it is not.

=item @list_of_facets = $event->assert

TODO

=item EMPTY_LIST_OR_STRING = $event->assert_brief

Returns a string giving a brief of the assertion if an assertion is present.
Returns an empty list if no assertion is present.

=back

=head2 SUBTESTS (PARENT FACET)

=over 4

=item $bool = $event->has_subtest

True if a subetest is present in this event.

=item $undef_or_hashref = $event->the_subtest

Get the one subtest if present, otherwise undef.

=item @list_of_facets = $event->subtest

TODO

=item EMPTY_LIST_OR_OBJECT = $event->subtest_result

Returns an empty list if there is no subtest.

Get an instance of L<Test2::API::InterceptResult> representing the subtest.

=back

=head2 CONTROL FACET (BAILOUT, ENCODING)

=over 4

=item $bool = $event->has_bailout

True if there was a bailout

=item $undef_hashref = $event->the_bailout

Return the control facet if it requested a bailout.

=item EMPTY_LIST_OR_HASHREF = $event->bailout

Get a list of 0 or 1 hashrefs. The hashref will be the control facet if a
bail-out was requested.

=item EMPTY_LIST_OR_STRING = $event->bailout_brief

Get the brief of the balout if present.

=item EMPTY_LIST_OR_STRING = $event->bailout_reason

Get the reason for the bailout, an empty string if no reason was provided, or
an empty list if there was no bailout.

=back

=head2 PLAN FACET

TODO

=over 4

=item $bool = $event->has_plan

=item $undef_or_hashref = $event->the_plan

=item @list_if_hashrefs = $event->plan

=item EMPTY_LIST_OR_STRING $event->plan_brief

=back

=head2 AMNESTY FACET (TODO AND SKIP)

TODO

=over 4

=item $event->has_amnesty

=item $event->the_amnesty

=item $event->amnesty

=item $event->amnesty_reasons

=item $event->has_todos

=item $event->todos

=item $event->todo_reasons

=item $event->has_skips

=item $event->skips

=item $event->skip_reasons

=item $event->has_other_amnesty

=item $event->other_amnesty

=item $event->other_amnesty_reasons

=back

=head2 ERROR FACET (CAPTURED EXCEPTIONS)

TODO

=over 4

=item $event->has_errors

=item $event->the_errors

=item $event->errors

=item $event->error_messages

=item $event->error_brief

=back

=head2 INFO FACET (DIAG, NOTE)

TODO

=over 4

=item $event->has_info

=item $event->the_info

=item $event->info

=item $event->info_messages

=item $event->has_diags

=item $event->diags

=item $event->diag_messages

=item $event->has_notes

=item $event->notes

=item $event->note_messages

=item $event->has_other_info

=item $event->other_info

=item $event->other_info_messages

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
