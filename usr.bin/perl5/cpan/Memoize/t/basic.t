use strict; use warnings;
use Memoize;
use Test::More tests => 27;

# here we test memoize() itself i.e. whether it sets everything up as requested
# (except for the (LIST|SCALAR)_CACHE options which are tested elsewhere)

my ( $sub, $wrapped );

sub dummy {1}
$sub = \&dummy;
$wrapped = memoize 'dummy';
isnt \&dummy, $sub, 'memoizing replaces the sub';
is ref $wrapped, 'CODE', '... and returns a coderef';
is \&dummy, $wrapped, '... which is the replacement';

sub dummy_i {1}
$sub = \&dummy_i;
$wrapped = memoize 'dummy_i', INSTALL => 'another';
is \&dummy_i, $sub, 'INSTALL does not replace the sub';
is \&another, $wrapped, '... but installs the memoized version where requested';

sub dummy_p {1}
$sub = \&dummy_p;
$wrapped = memoize 'dummy_p', INSTALL => 'another::package::too';
is \&another::package::too, $wrapped, '... even if that is a whole other package';

sub find_sub {
	my ( $needle, $symtbl ) = ( @_, *main::{'HASH'} );
	while ( my ( $name, $glob ) = each %$symtbl ) {
		if ( $name =~ /::\z/ ) {
			find_sub( $needle, *$glob{'HASH'} ) unless *$glob{'HASH'} == $symtbl;
		} elsif ( defined( my $sub = eval { *$glob{'CODE'} } ) ) {
			return 1 if $needle == $sub;
		}
	}
	return !1;
}

sub dummy_u {1}
$sub = \&dummy_u;
$wrapped = memoize 'dummy_u', INSTALL => undef;
is \&dummy_u, $sub, '... unless the passed name is undef';
ok !find_sub( $wrapped ), '... which does not install the memoized version anywhere';

$sub = sub {1};
$wrapped = memoize $sub;
is ref $wrapped, 'CODE', 'memoizing a $coderef wraps it';
ok !find_sub( $wrapped ), '... without installing the memoized version anywhere';

$sub = sub {1};
$wrapped = memoize $sub, INSTALL => 'another';
is \&another, $wrapped, '... unless requested using INSTALL';

my $num_args;
sub fake_normalize { $num_args = @_ }
$wrapped = memoize sub {1}, NORMALIZER => 'fake_normalize';
$wrapped->( ('x') x 7 );
is $num_args, 7, 'NORMALIZER installs the requested normalizer; both by name';
$wrapped = memoize sub {1}, NORMALIZER => \&fake_normalize;
$wrapped->( ('x') x 23 );
is $num_args, 23, '... as well as by reference';

$wrapped = eval { memoize 'dummy_none' };
is $wrapped, undef, 'memoizing a non-existent function fails';
like $@, qr/^Cannot operate on nonexistent function `dummy_none'/, '... with the expected error';

for my $nonsub ({}, [], \my $x) {
	is eval { memoize $nonsub }, undef, "memoizing ${\ref $nonsub} ref fails";
	like $@, qr/^Usage: memoize 'functionname'\|coderef \{OPTIONS\}/, '... with the expected error';
}

sub no_warnings_ok (&$) {
	my $w;
	local $SIG{'__WARN__'} = sub { push @$w, @_; &diag };
	shift->();
	local $Test::Builder::Level = $Test::Builder::Level + 1;
	is( $w, undef, shift ) or diag join '', @$w;
}

sub q1 ($) { $_[0] + 1 }
sub q2 ()  { time }
sub q3     { join "--", @_ }

no_warnings_ok { memoize 'q1' } 'no warnings with $ protype';
no_warnings_ok { memoize 'q2' } 'no warnings with empty protype';
no_warnings_ok { memoize 'q3' } 'no warnings without protype';
is q1(@{['a'..'z']}), 27, '$ prototype is honored';
is eval('q2("test")'), undef, 'empty prototype is honored';
like $@, qr/^Too many arguments for main::q2 /, '... with the expected error';
