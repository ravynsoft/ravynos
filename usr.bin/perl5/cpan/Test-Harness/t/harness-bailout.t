package My::Aggregator;
use strict;
use warnings;

sub new {
	my ($class) = @_;

	my $self = { results => {} };
	return bless( $self, $class );
}

sub start {}
sub stop {}

sub add {
	my ($self, $description, $parser) = @_;
	die "Test '$description' run twice" if exists $self->{results}{$description};
	$self->{results}{$description} = $parser;
}

1;

package My::Session;
use strict;
use warnings;

sub new {
	my ($class, %args) = @_;

	my $self = { %args };
	return bless( $self, $class );
}

sub result {
	my ($self, $result) = @_;
	return $self->{result} = $result || $self->{result};
}

sub close_test {
	shift->{closed} = 1;
}

1;

package My::Formatter;
use strict;
use warnings;

sub new {
	my ($class, $args) = @_;

	my $self = { %$args };
	return bless( $self, $class );
}

sub summary {
	my ($self, $aggregator, $interrupted) = @_;

	return sprintf(
		"My %sinterrupted formatter summary for %s",
		$interrupted ? '' : 'un',
		ref $aggregator
	);
}
sub verbosity { 0; }
sub prepare {};
sub open_test {
	my ($self, $test_name, $parser) = @_;

	return My::Session->new( name => $test_name, parser => $parser );
};

1;
package My::Multiplexer;
use strict;
use warnings;

sub new {
	my ($class) = @_;

	my $self = { parsers => [] };
	return bless( $self, $class );
}

sub add {
	my ( $self, $parser, $stash ) = @_;
	push @{ $self->{parsers} }, [ $parser, $stash ];
}

sub parsers { return scalar @{ shift->{parsers} }; }

sub next {
	my ($self) = @_;

	return unless $self->parsers;
	my ($parser, $stash) = @{ $self->{parsers}->[0] };
	my $result = $parser->next;
	shift @{ $self->{parsers} } unless $result;
	return ( $parser, $stash, $result );
}

1;

package My::Result;
use strict;
use warnings;

sub new {
	my ($class, %args) = @_;

	my $self = { %args };
	return bless( $self, $class );
}

sub is_bailout {
	return ( (shift->{source} || '') =~ '^bailout' );
}

sub explanation {
	return shift->{source};
}

1;

package My::Parser;
use strict;
use warnings;

sub new {
	my ($class, $args) = @_;

	my $self = { %$args, nexted => 0 };
	return bless( $self, $class );
}

sub next {
	my ($self) = @_;
	return if $self->{nexted};
	$self->{nexted} = 1;
	return My::Result->new( source => $self->{source} );
}

sub delete_spool {}

sub get_time { 0 }

sub get_times { 0 }

sub start_time {}

sub start_times {}

1;

package My::Job;
use strict;
use warnings;

our @finished_jobs;

sub new {
	my ($class, %args) = @_;

	my $self = { %args };
	return bless( $self, $class );
}
sub description { shift->{description} };
sub filename { shift->{filename} };
sub is_spinner {};
sub as_array_ref { return [ shift->description ] };
sub finish { push @finished_jobs, shift->filename; }

1;

package My::Scheduler;
use strict;
use warnings;

sub new {
	my ($class, %args) = @_;

	my @jobs = map
		{ My::Job->new( filename => $_->[0], description => $_->[1] ) }
		@{ delete( $args{tests} ) || [] };

	my $self = { %args, jobs => [ @jobs ] };
	return bless( $self, $class );
}

sub get_all { @{ shift->{jobs} || [] }; }
sub get_job { shift( @{ shift->{jobs} } ); }
1;

package main;
use strict;
use warnings;

use Test::More;
use TAP::Harness;

sub create_harness {
	my (%arg) = @_;

	return TAP::Harness->new({
		aggregator_class => 'My::Aggregator',
		formatter_class => 'My::Formatter',
		multiplexer_class => 'My::Multiplexer',
		parser_class => 'My::Parser',
		scheduler_class => 'My::Scheduler',
		jobs => $arg{jobs} || 1,
	});
}

my @after_test_callbacks;

my $harness = create_harness( jobs => 1 );
$harness->callback( after_test => sub { push @after_test_callbacks, $_[0] } );
eval { $harness->runtests( qw( no-bailout bailout not-executed ) ); };
my $err = $@;
like $err, qr/FAILED--Further testing stopped: bailout/;

$harness = create_harness( jobs => 2 );
$harness->callback( after_test => sub { push @after_test_callbacks, $_[0] } );
eval { $harness->runtests( qw( no-bailout-parallel bailout-parallel not-executed-parallel ) ); };
$err = $@;
like $err, qr/FAILED--Further testing stopped: bailout/;

is_deeply(
	[ @after_test_callbacks ],
	[ [ 'no-bailout' ], [ 'bailout' ], [ 'no-bailout-parallel' ], [ 'bailout-parallel' ],  ],
	'After test callbacks called OK'
);
is_deeply(
	[ @My::Job::finished_jobs ],
	[ 'no-bailout', 'bailout', 'no-bailout-parallel', 'bailout-parallel', ],
	'Jobs finished OK'
);

done_testing();
