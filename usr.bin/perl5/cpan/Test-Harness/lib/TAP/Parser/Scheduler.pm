package TAP::Parser::Scheduler;

use strict;
use warnings;

use Carp;
use TAP::Parser::Scheduler::Job;
use TAP::Parser::Scheduler::Spinner;

=head1 NAME

TAP::Parser::Scheduler - Schedule tests during parallel testing

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

    use TAP::Parser::Scheduler;

=head1 DESCRIPTION

=head1 METHODS

=head2 Class Methods

=head3 C<new>

    my $sched = TAP::Parser::Scheduler->new(tests => \@tests);
    my $sched = TAP::Parser::Scheduler->new(
        tests => [ ['t/test_name.t','Test Description'], ... ],
        rules => \%rules,
    );

Given 'tests' and optional 'rules' as input, returns a new
C<TAP::Parser::Scheduler> object.  Each member of C<@tests> should be either a
a test file name, or a two element arrayref, where the first element is a test
file name, and the second element is a test description. By default, we'll use
the test name as the description.

The optional C<rules> attribute provides direction on which tests should be run
in parallel and which should be run sequentially. If no rule data structure is
provided, a default data structure is used which makes every test eligible to
be run in parallel:

    { par => '**' },

The rules data structure is documented more in the next section.

=head2 Rules data structure

The "C<rules>" data structure is the the heart of the scheduler. It allows you
to express simple rules like "run all tests in sequence" or "run all tests in
parallel except these five tests.". However, the rules structure also supports
glob-style pattern matching and recursive definitions, so you can also express
arbitarily complicated patterns.

The rule must only have one top level key: either 'par' for "parallel" or 'seq'
for "sequence".

Values must be either strings with possible glob-style matching, or arrayrefs
of strings or hashrefs which follow this pattern recursively.

Every element in an arrayref directly below a 'par' key is eligible to be run
in parallel, while vavalues directly below a 'seq' key must be run in sequence.

=head3 Rules examples

Here are some examples:

    # All tests be run in parallel (the default rule)
    { par => '**' },

    # Run all tests in sequence, except those starting with "p"
    { par => 't/p*.t' },

    # Run all tests in parallel, except those starting with "p"
    {
        seq => [
                  { seq => 't/p*.t' },
                  { par => '**'     },
               ],
    }

    # Run some  startup tests in sequence, then some parallel tests then some
    # teardown tests in sequence.
    {
        seq => [
            { seq => 't/startup/*.t' },
            { par => ['t/a/*.t','t/b/*.t','t/c/*.t'], }
            { seq => 't/shutdown/*.t' },
        ],
    },


=head3 Rules resolution

=over 4

=item * By default, all tests are eligible to be run in parallel. Specifying any of your own rules removes this one.

=item * "First match wins". The first rule that matches a test will be the one that applies.

=item * Any test which does not match a rule will be run in sequence at the end of the run.

=item * The existence of a rule does not imply selecting a test. You must still specify the tests to run.

=item * Specifying a rule to allow tests to run in parallel does not make the run in parallel. You still need specify the number of parallel C<jobs> in your Harness object.

=back

=head3 Glob-style pattern matching for rules

We implement our own glob-style pattern matching. Here are the patterns it supports:

    ** is any number of characters, including /, within a pathname
    * is zero or more characters within a filename/directory name
    ? is exactly one character within a filename/directory name
    {foo,bar,baz} is any of foo, bar or baz.
    \ is an escape character

=cut

sub new {
    my $class = shift;

    croak "Need a number of key, value pairs" if @_ % 2;

    my %args  = @_;
    my $tests = delete $args{tests} || croak "Need a 'tests' argument";
    my $rules = delete $args{rules} || { par => '**' };

    croak "Unknown arg(s): ", join ', ', sort keys %args
      if keys %args;

    # Turn any simple names into a name, description pair. TODO: Maybe
    # construct jobs here?
    my $self = bless {}, $class;

    $self->_set_rules( $rules, $tests );

    return $self;
}

# Build the scheduler data structure.
#
# SCHEDULER-DATA ::= JOB
#                ||  ARRAY OF ARRAY OF SCHEDULER-DATA
#
# The nested arrays are the key to scheduling. The outer array contains
# a list of things that may be executed in parallel. Whenever an
# eligible job is sought any element of the outer array that is ready to
# execute can be selected. The inner arrays represent sequential
# execution. They can only proceed when the first job is ready to run.

sub _set_rules {
    my ( $self, $rules, $tests ) = @_;

    # Convert all incoming tests to job objects. 
    # If no test description is provided use the file name as the description. 
    my @tests = map { TAP::Parser::Scheduler::Job->new(@$_) }
      map { 'ARRAY' eq ref $_ ? $_ : [ $_, $_ ] } @$tests;
    my $schedule = $self->_rule_clause( $rules, \@tests );

    # If any tests are left add them as a sequential block at the end of
    # the run.
    $schedule = [ [ $schedule, @tests ] ] if @tests;

    $self->{schedule} = $schedule;
}

sub _rule_clause {
    my ( $self, $rule, $tests ) = @_;
    croak 'Rule clause must be a hash'
      unless 'HASH' eq ref $rule;

    my @type = keys %$rule;
    croak 'Rule clause must have exactly one key'
      unless @type == 1;

    my %handlers = (
        par => sub {
            [ map { [$_] } @_ ];
        },
        seq => sub { [ [@_] ] },
    );

    my $handler = $handlers{ $type[0] }
      || croak 'Unknown scheduler type: ', $type[0];
    my $val = $rule->{ $type[0] };

    return $handler->(
        map {
            'HASH' eq ref $_
              ? $self->_rule_clause( $_, $tests )
              : $self->_expand( $_, $tests )
          } 'ARRAY' eq ref $val ? @$val : $val
    );
}

sub _glob_to_regexp {
    my ( $self, $glob ) = @_;
    my $nesting;
    my $pattern;

    while (1) {
        if ( $glob =~ /\G\*\*/gc ) {

            # ** is any number of characters, including /, within a pathname
            $pattern .= '.*?';
        }
        elsif ( $glob =~ /\G\*/gc ) {

            # * is zero or more characters within a filename/directory name
            $pattern .= '[^/]*';
        }
        elsif ( $glob =~ /\G\?/gc ) {

            # ? is exactly one character within a filename/directory name
            $pattern .= '[^/]';
        }
        elsif ( $glob =~ /\G\{/gc ) {

            # {foo,bar,baz} is any of foo, bar or baz.
            $pattern .= '(?:';
            ++$nesting;
        }
        elsif ( $nesting and $glob =~ /\G,/gc ) {

            # , is only special inside {}
            $pattern .= '|';
        }
        elsif ( $nesting and $glob =~ /\G\}/gc ) {

            # } that matches { is special. But unbalanced } are not.
            $pattern .= ')';
            --$nesting;
        }
        elsif ( $glob =~ /\G(\\.)/gc ) {

            # A quoted literal
            $pattern .= $1;
        }
        elsif ( $glob =~ /\G([\},])/gc ) {

            # Sometimes meta characters
            $pattern .= '\\' . $1;
        }
        else {

            # Eat everything that is not a meta character.
            $glob =~ /\G([^{?*\\\},]*)/gc;
            $pattern .= quotemeta $1;
        }
        return $pattern if pos $glob == length $glob;
    }
}

sub _expand {
    my ( $self, $name, $tests ) = @_;

    my $pattern = $self->_glob_to_regexp($name);
    $pattern = qr/^ $pattern $/x;
    my @match = ();

    for ( my $ti = 0; $ti < @$tests; $ti++ ) {
        if ( $tests->[$ti]->filename =~ $pattern ) {
            push @match, splice @$tests, $ti, 1;
            $ti--;
        }
    }

    return @match;
}

=head2 Instance Methods

=head3 C<get_all>

Get a list of all remaining tests.

=cut

sub get_all {
    my $self = shift;
    my @all  = $self->_gather( $self->{schedule} );
    $self->{count} = @all;
    @all;
}

sub _gather {
    my ( $self, $rule ) = @_;
    return unless defined $rule;
    return $rule unless 'ARRAY' eq ref $rule;
    return map { defined() ? $self->_gather($_) : () } map {@$_} @$rule;
}

=head3 C<get_job>

Return the next available job as L<TAP::Parser::Scheduler::Job> object or
C<undef> if none are available. Returns a L<TAP::Parser::Scheduler::Spinner> if
the scheduler still has pending jobs but none are available to run right now.

=cut

sub get_job {
    my $self = shift;
    $self->{count} ||= $self->get_all;
    my @jobs = $self->_find_next_job( $self->{schedule} );
    if (@jobs) {
        --$self->{count};
        return $jobs[0];
    }

    return TAP::Parser::Scheduler::Spinner->new
      if $self->{count};

    return;
}

sub _not_empty {
    my $ar = shift;
    return 1 unless 'ARRAY' eq ref $ar;
    for (@$ar) {
        return 1 if _not_empty($_);
    }
    return;
}

sub _is_empty { !_not_empty(@_) }

sub _find_next_job {
    my ( $self, $rule ) = @_;

    my @queue = ();
    my $index = 0;
    while ( $index < @$rule ) {
        my $seq = $rule->[$index];

        # Prune any exhausted items.
        shift @$seq while @$seq && _is_empty( $seq->[0] );
        if (@$seq) {
            if ( defined $seq->[0] ) {
                if ( 'ARRAY' eq ref $seq->[0] ) {
                    push @queue, $seq;
                }
                else {
                    my $job = splice @$seq, 0, 1, undef;
                    $job->on_finish( sub { shift @$seq } );
                    return $job;
                }
            }
            ++$index;
        }
        else {

            # Remove the empty sub-array from the array
            splice @$rule, $index, 1;
        }
    }

    for my $seq (@queue) {
        if ( my @jobs = $self->_find_next_job( $seq->[0] ) ) {
            return @jobs;
        }
    }

    return;
}

=head3 C<as_string>

Return a human readable representation of the scheduling tree.
For example:

    my @tests = (qw{
        t/startup/foo.t 
        t/shutdown/foo.t
    
        t/a/foo.t t/b/foo.t t/c/foo.t t/d/foo.t
    });
    my $sched = TAP::Parser::Scheduler->new(
        tests => \@tests,
        rules => {
            seq => [
                { seq => 't/startup/*.t' },
                { par => ['t/a/*.t','t/b/*.t','t/c/*.t'] },
                { seq => 't/shutdown/*.t' },
            ],
        },
    );

Produces:

    par:
      seq:
        par:
          seq:
            par:
              seq:
                't/startup/foo.t'
            par:
              seq:
                't/a/foo.t'
              seq:
                't/b/foo.t'
              seq:
                't/c/foo.t'
            par:
              seq:
                't/shutdown/foo.t'
        't/d/foo.t'


=cut


sub as_string {
    my $self = shift;
    return $self->_as_string( $self->{schedule} );
}

sub _as_string {
    my ( $self, $rule, $depth ) = ( shift, shift, shift || 0 );
    my $pad    = ' ' x 2;
    my $indent = $pad x $depth;
    if ( !defined $rule ) {
        return "$indent(undef)\n";
    }
    elsif ( 'ARRAY' eq ref $rule ) {
        return unless @$rule;
        my $type = ( 'par', 'seq' )[ $depth % 2 ];
        return join(
            '', "$indent$type:\n",
            map { $self->_as_string( $_, $depth + 1 ) } @$rule
        );
    }
    else {
        return "$indent'" . $rule->filename . "'\n";
    }
}

1;
