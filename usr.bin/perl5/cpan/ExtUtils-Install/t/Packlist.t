#!/usr/bin/perl -w
use strict;

BEGIN {
    unshift @INC, 't/lib';
}

use Test::More tests => 35;

BEGIN { use_ok( 'ExtUtils::Packlist' ); }

is( ref(ExtUtils::Packlist::mkfh()), 'GLOB', 'mkfh() should return a FH' );

# new calls tie()
my $pl = ExtUtils::Packlist->new();
isa_ok( $pl, 'ExtUtils::Packlist' );
is( ref tied %$pl, 'ExtUtils::Packlist', 'obj should be tied underneath' );


$pl = ExtUtils::Packlist::TIEHASH( 'tieclass', 'packfile' );
is( ref($pl), 'tieclass', 'TIEHASH() should bless into class' );
is( $pl->{packfile}, 'packfile', 'TIEHASH() should store packfile name' );


ExtUtils::Packlist::STORE($pl, 'key', 'value');
is( $pl->{data}{key}, 'value', 'STORE() should stuff stuff in data member' );


$pl->{data}{foo} = 'bar';
is( ExtUtils::Packlist::FETCH($pl, 'foo'), 'bar', 'check FETCH()' );


# test FIRSTKEY and NEXTKEY
SKIP: {
	$pl->{data}{bar} = 'baz';
	skip('not enough keys to test FIRSTKEY', 2)
      unless keys %{ $pl->{data} } > 2;

	# get the first and second key
	my ($first, $second) = keys %{ $pl->{data} };

	# now get a couple of extra keys, to mess with the hash iterator
	my $i = 0;
	for (keys %{ $pl->{data} } ) {
		last if $i++;
	}

	# finally, see if it really can get the first key again
	is( ExtUtils::Packlist::FIRSTKEY($pl), $first,
		'FIRSTKEY() should be consistent' );

	is( ExtUtils::Packlist::NEXTKEY($pl), $second,
		'and NEXTKEY() should also be consistent' );
}


ok( ExtUtils::Packlist::EXISTS($pl, 'bar'), 'EXISTS() should find keys' );


ExtUtils::Packlist::DELETE($pl, 'bar');
ok( !(exists $pl->{data}{bar}), 'DELETE() should delete cleanly' );


ExtUtils::Packlist::CLEAR($pl);
is( keys %{ $pl->{data} }, 0, 'CLEAR() should wipe out data' );


# DESTROY does nothing...
can_ok( 'ExtUtils::Packlist', 'DESTROY' );


# write is a little more complicated
eval { ExtUtils::Packlist::write({}) };
like( $@, qr/No packlist filename/, 'write() should croak without packfile' );

eval { ExtUtils::Packlist::write({}, 'eplist') };
my $file_is_ready = $@ ? 0 : 1;
ok( $file_is_ready, 'write() can write a file' );

local *IN;

SKIP: {
	skip('cannot write files, some tests difficult', 3) unless $file_is_ready;

	# set this file to read-only
	chmod 0444, 'eplist';

	SKIP: {
	    skip("cannot write readonly files", 1) if -w 'eplist';

	    eval { ExtUtils::Packlist::write({}, 'eplist') };
	    like( $@, qr/Can't open file/, 'write() should croak on open failure' );
	}

	#'now set it back (tick here fixes vim syntax highlighting ;)
	chmod 0777, 'eplist';

	# and some test data to be read
	$pl->{data} = {
		single => 1,
		hash => {
			foo => 'bar',
			baz => 'bup',
		},
		'/./abc' => '',
	};
	eval { ExtUtils::Packlist::write($pl, 'eplist') };
	is( $@, '', 'write() should normally succeed' );
	is( $pl->{packfile}, 'eplist', 'write() should set packfile name' );

	$file_is_ready = open(IN, 'eplist');
}


eval { ExtUtils::Packlist::read({}) };
like( $@, qr/^No packlist filename/, 'read() should croak without packfile' );


eval { ExtUtils::Packlist::read({}, 'abadfilename') };
like( $@, qr/^Can't open file/, 'read() should croak with bad packfile name' );
#'open packfile for reading


# and more read() tests
SKIP: {
	skip("cannot open file for reading: $!", 5) unless $file_is_ready;
	my $file = do { local $/ = <IN> };

	like( $file, qr/single\n/, 'key with value should be available' );
	like( $file, qr!/\./abc\n!, 'key with no value should also be present' );
	like( $file, qr/hash.+baz=bup/, 'key with hash value should be present' );
	like( $file, qr/hash.+foo=bar/, 'second embedded hash value should appear');
	close IN;

	eval{ ExtUtils::Packlist::read($pl, 'eplist') };
	is( $@, '', 'read() should normally succeed' );
	is( $pl->{data}{single}, undef, 'single keys should have undef value' );
	is( ref($pl->{data}{hash}), 'HASH', 'multivalue keys should become hashes');

	is( $pl->{data}{hash}{foo}, 'bar', 'hash values should be set' );
	ok( exists $pl->{data}{'/abc'}, 'read() should resolve /./ to / in keys' );

	# give validate a valid and an invalid file to find
	$pl->{data} = {
		eplist => 1,
		fake => undef,
	};

	is( ExtUtils::Packlist::validate($pl), 1,
		'validate() should find missing files' );
	ExtUtils::Packlist::validate($pl, 1);
	ok( !exists $pl->{data}{fake},
		'validate() should remove missing files when prompted' );

	# one more new() test, to see if it calls read() successfully
	$pl = ExtUtils::Packlist->new('eplist');
}


# packlist_file, $pl should be set from write test
is( ExtUtils::Packlist::packlist_file({ packfile => 'pl' }), 'pl',
	'packlist_file() should fetch packlist from passed hash' );
is( ExtUtils::Packlist::packlist_file($pl), 'eplist',
	'packlist_file() should fetch packlist from ExtUtils::Packlist object' );

my $w  = 0;
BEGIN {
	# Call mkfh at BEGIN time, to make sure it does not trigger "Used
	# once" warnings.
	$SIG{__WARN__} = sub { ++$w; warn $_[0] };
	ExtUtils::Packlist::mkfh();
	
}
INIT {
	is $w, undef, '[perl #107410] no warnings from BEGIN-time mkfh';
	delete $SIG{__WARN__};
}

END {
	1 while unlink qw( eplist );
}
