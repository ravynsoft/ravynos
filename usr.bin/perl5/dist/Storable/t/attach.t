#!./perl -w
#
# This file tests that Storable correctly uses STORABLE_attach hooks

sub BEGIN {
	unshift @INC, 't';
	unshift @INC, 't/compat' if $] < 5.006002;
	require Config; import Config;
	if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
		print "1..0 # Skip: Storable was not built\n";
		exit 0;
	}
}

use Test::More tests => 3;
use Storable ();

{
	my $destruct_cnt = 0;
	my $obj = bless {data => 'ok'}, 'My::WithDestructor';
	my $target = Storable::thaw( Storable::freeze( $obj ) );
	is( $target->{data}, 'ok', 'We got correct object after freeze/thaw' );
	is( $destruct_cnt, 0, 'No tmp objects created by Storable' );
	undef $obj;
	undef $target;
	is( $destruct_cnt, 2, 'Only right objects destroyed at the end' );

	package My::WithDestructor;

	sub STORABLE_freeze {
		my ($self, $clone) = @_;
		return $self->{data};
	}

	sub STORABLE_attach {
		my ($class, $clone, $string) = @_;
		return bless {data => $string}, 'My::WithDestructor';
	}

	sub DESTROY { $destruct_cnt++; }
}

