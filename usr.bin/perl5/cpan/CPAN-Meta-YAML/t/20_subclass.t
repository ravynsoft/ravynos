# Testing documents that should fail
use strict;
use warnings;
use lib 't/lib/';
use Test::More 0.88;
use TestUtils;

use File::Spec::Functions ':ALL';



#####################################################################
# Customized Class

SCOPE: {
    package Foo;

    use CPAN::Meta::YAML;

    use vars qw{@ISA};
    BEGIN {
        @ISA = 'CPAN::Meta::YAML';
    }

    # XXX-INGY subclasses should not use private methods… or if they
    # do they should expect method name changes.
    # sub _write_scalar {

    sub _dump_scalar {
        my $self   = shift;
        my $string = shift;
        my $is_key = shift;
        if ( defined $is_key ) {
            return scalar reverse $string;
        } else {
            return $string;
        }
    }

    1;
}





#####################################################################
# Generate the value

my $object = Foo->new(
    { foo => 'bar' }
);
is( $object->write_string, "---\noof: bar\n", 'Subclassing works' );

done_testing;
