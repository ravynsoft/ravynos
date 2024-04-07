#!./perl -w
#
#  Copyright 2005, Adam Kennedy.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

# Tests freezing/thawing structures containing Singleton objects,
# which should see both structs pointing to the same object.

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Test::More tests => 16;
use Storable ();

# Get the singleton
my $object = My::Singleton->new;
isa_ok( $object, 'My::Singleton' );

# Confirm (for the record) that the class is actually a Singleton
my $object2 = My::Singleton->new;
isa_ok( $object2, 'My::Singleton' );
is( "$object", "$object2", 'Class is a singleton' );

############
# Main Tests

my $struct = [ 1, $object, 3 ];

# Freeze the struct
my $frozen = Storable::freeze( $struct );
ok( (defined($frozen) and ! ref($frozen) and length($frozen)), 'freeze returns a string' );

# Thaw the struct
my $thawed = Storable::thaw( $frozen );

# Now it should look exactly like the original
is_deeply( $struct, $thawed, 'Struct superficially looks like the original' );

# ... EXCEPT that the Singleton should be the same instance of the object
is( "$struct->[1]", "$thawed->[1]", 'Singleton thaws correctly' );

# We can also test this empirically
$struct->[1]->{value} = 'Goodbye cruel world!';
is_deeply( $struct, $thawed, 'Empiric testing confirms correct behaviour' );

$struct = [ $object, $object ];
$frozen = Storable::freeze($struct);
$thawed = Storable::thaw($frozen);
is("$thawed->[0]", "$thawed->[1]", "Multiple Singletons thaw correctly");

# End Tests
###########

package My::Singleton;

my $SINGLETON = undef;

sub new {
	$SINGLETON or
	$SINGLETON = bless { value => 'Hello World!' }, $_[0];
}

sub STORABLE_freeze {
	my $self = shift;

	# We don't actually need to return anything, but provide a null string
	# to avoid the null-list-return behaviour.
	return ('foo');
}

sub STORABLE_attach {
	my ($class, $clone, $string) = @_;
	Test::More::ok( ! ref $class, 'STORABLE_attach passed class, and not an object' );
	Test::More::is( $class, 'My::Singleton', 'STORABLE_attach is passed the correct class name' );
	Test::More::is( $clone, 0, 'We are not in a dclone' );
	Test::More::is( $string, 'foo', 'STORABLE_attach gets the string back' );

	# Get the Singleton object and return it
	return $class->new;
}
