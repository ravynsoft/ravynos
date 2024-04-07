#! perl -T

# Taint tests for module File::Path

use strict;

use Test::More tests => 6;

BEGIN {
    use_ok('File::Path', qw(rmtree mkpath make_path remove_tree));
    use_ok('File::Spec::Functions');
}

# find a place to work
my $tmp_base = catdir(
    curdir(),
    sprintf( 'taint-%x-%x-%x', time, $$, rand(99999) ),
);

# invent some names
my @dir = (
    catdir($tmp_base, qw(a b)),
    catdir($tmp_base, qw(a c)),
    catdir($tmp_base, qw(z b)),
    catdir($tmp_base, qw(z c)),
);

# create them
my @created = make_path(@dir);
is(scalar(@created), 7, "created list of directories");

my $count = rmtree($tmp_base, {error => \(my $err), result => \my $res});
is( $count, 7, 'rmtree under taint' );
is( scalar(@$err), 0, 'no errors' );
is( scalar(@$res), 7, 'seven items' );
