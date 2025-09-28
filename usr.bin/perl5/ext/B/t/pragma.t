#!./perl -w

BEGIN {    ## no critic strict
    if ( $ENV{PERL_CORE} ) {
	unshift @INC, '../../t/lib';
    } else {
        unshift @INC, 't';
    }
    require Config;
    if ( ( $Config::Config{'extensions'} !~ /\bB\b/ ) ) {
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use strict;
use warnings;
use Test::More tests => 4 * 3;
use B 'svref_2object';

# use Data::Dumper 'Dumper';

sub foo {
    my ( $x, $y, $z );

    # hh => {},
    $z = $x * $y;

    # hh => { mypragma => 42 }
    use mypragma;
    $z = $x + $y;

    # hh => { mypragma => 0 }
    no mypragma;
    $z = $x - $y;
}

{

    # Pragmas don't appear til they're used.
    my $cop = find_op_cop( \&foo, qr/multiply/ );
    isa_ok( $cop, 'B::COP', 'found pp_multiply opnode' );

    my $rhe = $cop->hints_hash;
    isa_ok( $rhe, 'B::RHE', 'got hints_hash' );

    my $hints_hash = $rhe->HASH;
    is( ref($hints_hash), 'HASH', 'Got hash reference' );

    ok( not( exists $hints_hash->{mypragma} ), q[! exists mypragma] );
}

{

    # Pragmas can be fetched.
    my $cop = find_op_cop( \&foo, qr/add/ );
    isa_ok( $cop, 'B::COP', 'found pp_add opnode' );

    my $rhe = $cop->hints_hash;
    isa_ok( $rhe, 'B::RHE', 'got hints_hash' );

    my $hints_hash = $rhe->HASH;
    is( ref($hints_hash), 'HASH', 'Got hash reference' );

    is( $hints_hash->{mypragma}, 42, q[mypragma => 42] );
}

{

    # Pragmas can be changed.
    my $cop = find_op_cop( \&foo, qr/subtract/ );
    isa_ok( $cop, 'B::COP', 'found pp_subtract opnode' );

    my $rhe = $cop->hints_hash;
    isa_ok( $rhe, 'B::RHE', 'got hints_hash' );

    my $hints_hash = $rhe->HASH;
    is( ref($hints_hash), 'HASH', 'Got hash reference' );

    is( $hints_hash->{mypragma}, 0, q[mypragma => 0] );
}
exit;

our $COP;

sub find_op_cop {
    my ( $sub, $op ) = @_;
    my $cv = svref_2object($sub);
    local $COP;

    if ( not _find_op_cop( $cv->ROOT, $op ) ) {
        $COP = undef;
    }

    return $COP;
}

{

    # Make B::NULL objects evaluate as false.
    package B::NULL;
    use overload 'bool' => sub () { !!0 };
}

sub _find_op_cop {
    my ( $op, $name ) = @_;

    # Fail on B::NULL or whatever.
    return 0 if not $op;

    # Succeed when we find our match.
    return 1 if $op->name =~ $name;

    # Stash the latest seen COP opnode. This has our hints hash.
    if ( $op->isa('B::COP') ) {

        # print Dumper(
        #     {   cop   => $op,
        #         hints => $op->hints_hash->HASH
        #     }
        # );
        $COP = $op;
    }

    # Recurse depth first passing success up if it happens.
    if ( $op->can('first') ) {
        return 1 if _find_op_cop( $op->first, $name );
    }
    return 1 if _find_op_cop( $op->sibling, $name );

    # Oh well. Hopefully our caller knows where to try next.
    return 0;
}

