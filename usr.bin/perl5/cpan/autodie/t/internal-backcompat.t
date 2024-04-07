#!/usr/bin/perl -w
use strict;
use warnings;
use Fatal;
use Test::More 'no_plan';

# Tests to determine if Fatal's internal interfaces remain backwards
# compatible.
#
# WARNING: This file contains a lot of very ugly code, hard-coded
# strings, and nasty API calls.  It may frighten small children.
# Viewer discretion is advised.

# fill_protos.  This hasn't been changed since the original Fatal,
# and so should always be the same.

my %protos = (
    '$'     => [ [ 1, '$_[0]' ] ],
    '$$'    => [ [ 2, '$_[0]', '$_[1]' ] ],
    '$$@'   => [ [ 3, '$_[0]', '$_[1]', '@_[2..$#_]' ] ],
    '\$'    => [ [ 1, '${$_[0]}' ] ],
    '\%'    => [ [ 1, '%{$_[0]}' ] ],
    '\%;$*' => [ [ 1, '%{$_[0]}' ], [ 2, '%{$_[0]}', '$_[1]' ],
                 [ 3, '%{$_[0]}', '$_[1]', '$_[2]' ] ],
);

while (my ($proto, $code) = each %protos) {
    is_deeply( [ Fatal::fill_protos($proto) ], $code, $proto);
}

# write_invocation tests
no warnings 'qw';

# Technically the outputted code varies from the classical Fatal.
# However the changes are mostly whitespace.  Those that aren't are
# improvements to error messages or bug fixes.

my @write_invocation_calls = (
    [
        # Core  # Call          # Name  # Void  # Args
        [ 1,    'CORE::open',   'open', 0,      [ 1, qw($_[0]) ],
                                                [ 2, qw($_[0] $_[1]) ],
                                                [ 3, qw($_[0] $_[1] @_[2..$#_])]
        ],
        q{	if (@_ == 1) {
return CORE::open($_[0]) || Carp::croak("Can't open(@_): $!")	} elsif (@_ == 2) {
return CORE::open($_[0], $_[1]) || Carp::croak("Can't open(@_): $!")	} elsif (@_ >= 3) {
return CORE::open($_[0], $_[1], @_[2..$#_]) || Carp::croak("Can't open(@_): $!")
            }
            die "Internal error: open(@_): Do not expect to get ", scalar(@_), " arguments";
    }
    ]
);

foreach my $test (@write_invocation_calls) {
    is(Fatal::write_invocation( @{ $test->[0] } ), $test->[1], 'write_inovcation');
}

# one_invocation tests.

my @one_invocation_calls = (
        # Core  # Call          # Name  # Void   # Args
    [
        [ 1,    'CORE::open',   'open', 0,      qw($_[0] $_[1] @_[2..$#_]) ],
        q{return CORE::open($_[0], $_[1], @_[2..$#_]) || Carp::croak("Can't open(@_): $!")},
    ],
    [
        [ 1,    'CORE::open',   'open', 1,      qw($_[0] $_[1] @_[2..$#_]) ],
        q{return (defined wantarray)?CORE::open($_[0], $_[1], @_[2..$#_]):
                   CORE::open($_[0], $_[1], @_[2..$#_]) || Carp::croak("Can't open(@_): $!")},
    ],
);

foreach my $test (@one_invocation_calls) {
    is(Fatal::one_invocation( @{ $test->[0] } ), $test->[1], 'one_inovcation');
}

# TODO: _make_fatal
# Since this subroutine has always started with an underscore,
# I think it's pretty clear that it's internal-only.  I'm not
# testing it here, and it doesn't yet have backcompat.
