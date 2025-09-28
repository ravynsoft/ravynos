#!/usr/bin/perl -w

use strict;

#######################################################################

sub array_diff {
    my ( $got, $expected ) = @_;
    push @$got,      ( '(missing)' )          x ( @$expected - @$got ) if @$got < @$expected;
    push @$expected, ( '(should not exist)' ) x ( @$got - @$expected ) if @$got > @$expected;
    join "\n    ", '  All differences:', (
        map +( "got  [$_] " . $got->[$_], 'expected'.(' ' x length).$expected->[$_] ),
        grep $got->[$_] ne $expected->[$_],
        0 .. $#$got
    );
}

#######################################################################

use Test::More tests => 8;  # some extra tests in t/lib/BaseInc*

use lib 't/lib', sub {()};

# make it look like an older perl
BEGIN { push @INC, '.' if $INC[-1] ne '.' }

BEGIN {
	my $x = sub { CORE::require $_[0] };
	my $y = sub { &$x };
	my $z = sub { &$y };
	*CORE::GLOBAL::require = $z;
}

my @expected; BEGIN { @expected = @INC }

use base 'BaseIncMandatory';

BEGIN {
    @t::lib::Dummy::ISA = (); # make it look like an optional load
    my $success = eval q{use base 't::lib::Dummy'}, my $err = $@;
    ok !$success, 'loading optional modules from . using base.pm fails';
    is_deeply \@INC, \@expected, '... without changes to @INC'
        or diag array_diff [@INC], [@expected];
    like $err, qr!Base class package "t::lib::Dummy" is not empty but "t/lib/Dummy\.pm" exists in the current directory\.!,
        '... and the proper error message';
}

BEGIN { @BaseIncOptional::ISA = () } # make it look like an optional load
use base 'BaseIncOptional';

BEGIN {
    @expected = ( 't/lib/on-head', @expected, 't/lib/on-tail' );
    is_deeply \@INC, \@expected, 'modules loaded by base can extend @INC at both ends'
        or diag array_diff [@INC], [@expected];
}
