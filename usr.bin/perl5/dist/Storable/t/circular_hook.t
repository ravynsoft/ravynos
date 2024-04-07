#!./perl -w
#
#  Copyright 2005, Adam Kennedy.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

# Man, blessed.t scared the hell out of me. For a second there I thought
# I'd lose Test::More...

# This file tests several known-error cases relating to STORABLE_attach, in
# which Storable should (correctly) throw errors.

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Storable ();
use Test::More tests => 9;

my $ddd = bless { }, 'Foo';
my $eee = bless { Bar => $ddd }, 'Bar';
$ddd->{Foo} = $eee;

my $array = [ $ddd ];

my $string = Storable::freeze( $array );
my $thawed = Storable::thaw( $string );

# is_deeply infinite loops in circulars, so do it manually
# is_deeply( $array, $thawed, 'Circular hooked objects work' );
is( ref($thawed), 'ARRAY', 'Top level ARRAY' );
is( scalar(@$thawed), 1, 'ARRAY contains one element' );
isa_ok( $thawed->[0], 'Foo' );
is( scalar(keys %{$thawed->[0]}), 1, 'Foo contains one element' );
isa_ok( $thawed->[0]->{Foo}, 'Bar' );
is( scalar(keys %{$thawed->[0]->{Foo}}), 1, 'Bar contains one element' );
isa_ok( $thawed->[0]->{Foo}->{Bar}, 'Foo' );
is( $thawed->[0], $thawed->[0]->{Foo}->{Bar}, 'Circular is... well... circular' );

# Make sure the thawing went the way we expected
is_deeply( \@Foo::order, [ 'Bar', 'Foo' ], 'thaw order is correct (depth first)' );





package Foo;

@order = ();

sub STORABLE_freeze {
	my ($self, $clone) = @_;
	my $class = ref $self;
	
	# print "# Freezing $class\n";

	return ($class, $self->{$class});
}

sub STORABLE_thaw {
	my ($self, $clone, $string, @refs) = @_;
	my $class = ref $self;

	# print "# Thawing $class\n";

	$self->{$class} = shift @refs;

	push @order, $class;

 	return;
}

package Bar;

BEGIN {
@ISA = 'Foo';
}

1;
