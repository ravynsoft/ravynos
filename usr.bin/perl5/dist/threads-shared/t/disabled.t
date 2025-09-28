use strict;
use warnings;

use Test::More tests => 27;

use threads::shared;

### Start of Testing ###

ok( !$INC{"threads.pm"}, 'make sure threads are really off' );

# Check each faked function.
foreach my $func (qw(share cond_wait cond_signal cond_broadcast)) {
    isnt( __PACKAGE__->can($func), 0, "Have $func" );

    eval qq{$func()};
    like( $@, qr/^Not enough arguments /, 'Expected error with no arguments');

    my %hash = (foo => 42, bar => 23);
    eval qq{$func(\%hash)};
    is( $@, '', 'no error' );
    is_deeply( \%hash, {foo => 42, bar => 23}, 'argument is unchanged' );
}

# These all have no return value.
foreach my $func (qw(cond_wait cond_signal cond_broadcast)) {
    my @array = qw(1 2 3 4);
    is( eval qq{$func(\@array)}, undef, "$func has no return value" );
    is_deeply( \@array, [1, 2, 3, 4], 'argument is unchanged' );
}

{
    my @array = qw(1 2 3 4);
    is_deeply( share(@array), \@array,
	'share() is supposed to return back its argument as a ref' );
    is( ref &share({}), 'HASH' );
    is_deeply( \@array, [1, 2, 3, 4], 'argument is unchanged' );
}

# lock() should be a no-op.  The return value is currently undefined.
{
    my @array = qw(1 2 3 4);
    lock(@array);
    is_deeply( \@array, [1, 2, 3, 4], 'lock() should be a no-op' );
}

exit(0);

# EOF
