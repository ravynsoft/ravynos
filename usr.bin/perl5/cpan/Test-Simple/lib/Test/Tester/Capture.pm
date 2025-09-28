use strict;

package Test::Tester::Capture;

our $VERSION = '1.302194';


use Test::Builder;

use vars qw( @ISA );
@ISA = qw( Test::Builder );

# Make Test::Tester::Capture thread-safe for ithreads.
BEGIN {
	use Config;
	*share = sub { 0 };
	*lock  = sub { 0 };
}

my $Curr_Test = 0;      share($Curr_Test);
my @Test_Results = ();  share(@Test_Results);
my $Prem_Diag = {diag => ""};	 share($Curr_Test);

sub new
{
  # Test::Tester::Capgture::new used to just return __PACKAGE__
  # because Test::Builder::new enforced its singleton nature by
  # return __PACKAGE__. That has since changed, Test::Builder::new now
  # returns a blessed has and around version 0.78, Test::Builder::todo
  # started wanting to modify $self. To cope with this, we now return
  # a blessed hash. This is a short-term hack, the correct thing to do
  # is to detect which style of Test::Builder we're dealing with and
  # act appropriately.

  my $class = shift;
  return bless {}, $class;
}

sub ok {
	my($self, $test, $name) = @_;

	my $ctx = $self->ctx;

	# $test might contain an object which we don't want to accidentally
	# store, so we turn it into a boolean.
	$test = $test ? 1 : 0;

	lock $Curr_Test;
	$Curr_Test++;

	my($pack, $file, $line) = $self->caller;

	my $todo = $self->todo();

	my $result = {};
	share($result);

	unless( $test ) {
		@$result{ 'ok', 'actual_ok' } = ( ( $todo ? 1 : 0 ), 0 );
	}
	else {
		@$result{ 'ok', 'actual_ok' } = ( 1, $test );
	}

	if( defined $name ) {
		$name =~ s|#|\\#|g;	 # # in a name can confuse Test::Harness.
		$result->{name} = $name;
	}
	else {
		$result->{name} = '';
	}

	if( $todo ) {
		my $what_todo = $todo;
		$result->{reason} = $what_todo;
		$result->{type}   = 'todo';
	}
	else {
		$result->{reason} = '';
		$result->{type}   = '';
	}

	$Test_Results[$Curr_Test-1] = $result;

	unless( $test ) {
		my $msg = $todo ? "Failed (TODO)" : "Failed";
		$result->{fail_diag} = ("	$msg test ($file at line $line)\n");
	} 

	$result->{diag} = "";
	$result->{_level} = $Test::Builder::Level;
	$result->{_depth} = Test::Tester::find_run_tests();

	$ctx->release;

	return $test ? 1 : 0;
}

sub skip {
	my($self, $why) = @_;
	$why ||= '';

	my $ctx = $self->ctx;

	lock($Curr_Test);
	$Curr_Test++;

	my %result;
	share(%result);
	%result = (
		'ok'	  => 1,
		actual_ok => 1,
		name	  => '',
		type	  => 'skip',
		reason	=> $why,
		diag    => "",
		_level   => $Test::Builder::Level,
		_depth => Test::Tester::find_run_tests(),
	);
	$Test_Results[$Curr_Test-1] = \%result;

	$ctx->release;
	return 1;
}

sub todo_skip {
	my($self, $why) = @_;
	$why ||= '';

	my $ctx = $self->ctx;

	lock($Curr_Test);
	$Curr_Test++;

	my %result;
	share(%result);
	%result = (
		'ok'	  => 1,
		actual_ok => 0,
		name	  => '',
		type	  => 'todo_skip',
		reason	=> $why,
		diag    => "",
		_level   => $Test::Builder::Level,
		_depth => Test::Tester::find_run_tests(),
	);

	$Test_Results[$Curr_Test-1] = \%result;

	$ctx->release;
	return 1;
}

sub diag {
	my($self, @msgs) = @_;
	return unless @msgs;

	# Prevent printing headers when compiling (i.e. -c)
	return if $^C;

	my $ctx = $self->ctx;

	# Escape each line with a #.
	foreach (@msgs) {
		$_ = 'undef' unless defined;
	}

	push @msgs, "\n" unless $msgs[-1] =~ /\n\Z/;

	my $result = $Curr_Test ? $Test_Results[$Curr_Test - 1] : $Prem_Diag;

	$result->{diag} .= join("", @msgs);

	$ctx->release;
	return 0;
}

sub details {
	return @Test_Results;
}


# Stub. Feel free to send me a patch to implement this.
sub note {
}

sub explain {
	return Test::Builder::explain(@_);
}

sub premature
{
	return $Prem_Diag->{diag};
}

sub current_test
{
	if (@_ > 1)
	{
		die "Don't try to change the test number!";
	}
	else
	{
		return $Curr_Test;
	}
}

sub reset
{
	$Curr_Test = 0;
	@Test_Results = ();
	$Prem_Diag = {diag => ""};
}

1;

__END__

=head1 NAME

Test::Tester::Capture - Help testing test modules built with Test::Builder

=head1 DESCRIPTION

This is a subclass of Test::Builder that overrides many of the methods so
that they don't output anything. It also keeps track of its own set of test
results so that you can use Test::Builder based modules to perform tests on
other Test::Builder based modules.

=head1 AUTHOR

Most of the code here was lifted straight from Test::Builder and then had
chunks removed by Fergal Daly <fergal@esatclear.ie>.

=head1 LICENSE

Under the same license as Perl itself

See http://www.perl.com/perl/misc/Artistic.html

=cut
