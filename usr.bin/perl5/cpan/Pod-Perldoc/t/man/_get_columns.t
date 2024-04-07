use Test::More;
use strict;
use warnings;

{
package Local::ToMan;
use parent 'Pod::Perldoc::ToMan';
use vars qw( $stty_text $is_linux $warning );
no warnings 'redefine';
no strict 'refs';
sub _get_stty { $stty_text }
sub is_linux { $is_linux }
sub warn { shift; $warning = join '', @_ }
}

BEGIN {
our @columns = qw( EXPECTED IS_LINUX MANWIDTH MANWIDTH_EXPECTED STTY STTY_EXPECTED );
foreach my $i ( 0 .. $#columns ) {
	no strict 'refs';
	*{"$columns[$i]"} = sub () { $i };
	}
}

my @tests = (
    # for linux, choose between manwidth and stty
	[ 62, 1, undef,  0, "; 62 columns;", 62 ],
	[ 63, 1, undef,  0, "columns 63",    63 ],	
	[ 57, 1, 57,    57, "columns 63",    63 ],
	[ 73, 1, undef,  0, " ",              0 ],

    # for not linux, the same
	[ 62, 0, undef,  0, "; 62 columns;", 62 ],
	[ 63, 0, undef,  0, "columns 63",    63 ],	
	[ 57, 0, 57,    57, "columns 63",    63 ],
	[ 73, 0, undef,  0, " ",              0 ],

	# bad manwidths	
	[ 62, 1, -1,     0, "; 62 columns;", 62 ],
	[ 63, 1, 'abc',  0, "columns 63",    63 ],	
	[ 64, 1, '',     0, "columns 64",    64 ],
	[ 73, 1, undef,  0, " ",              0 ],
	);

plan tests => 3 * @tests;


foreach my $test ( @tests ) {
	local $ENV{MANWIDTH}               = $test->[MANWIDTH];
	local $Local::ToMan::stty_text     = $test->[STTY];
	local $Local::ToMan::is_linux      = $test->[IS_LINUX];

	{
	no warnings 'uninitialized';
	diag( sprintf 
		"MANWIDTH: %s STTY: %s LINUX: %s",
		defined $ENV{MANWIDTH} ? $ENV{MANWIDTH} : "(undef)",
		$Local::ToMan::stty_text,
		$Local::ToMan::is_linux,
		) if $ENV{PERLDOCDEBUG};
	}

	is( Local::ToMan->_get_columns_from_manwidth(), $test->[MANWIDTH_EXPECTED],
		"_get_columns_from_manwidth returns the right number" );
	is( Local::ToMan->_get_columns_from_stty(),     $test->[STTY_EXPECTED],
	    "_get_columns_from_stty returns the right number" );
	is( Local::ToMan->_get_columns,                 $test->[EXPECTED],
	    "_get_columns returns the right number" );
	}
