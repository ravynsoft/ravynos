use strict; use warnings;

package DBMTest;

my ($module, $is_scalar_only);

use Memoize qw(memoize unmemoize);
use Test::More;

sub errlines { split /\n/, $@ }

my $ARG = 'Keith Bostic is a pinhead';

sub c5 { 5 }
sub c23 { 23 }

sub test_dbm { SKIP: {
	tie my %cache, $module, @_ or die $!;

	my $sub = eval { unmemoize memoize sub {}, LIST_CACHE => [ HASH => \%cache ] };
	my $errx = qr/^You can't use \Q$module\E for LIST_CACHE because it can only store scalars/;
	if ($is_scalar_only) {
		is $sub, undef, "use as LIST_CACHE fails";
		like $@, $errx, '... with the expected error';
	} else {
		ok $sub, "use as LIST_CACHE succeeds";
	}

	$sub = eval { no warnings; unmemoize memoize sub {}, LIST_CACHE => [ TIE => $module, @_ ] };
	if ($is_scalar_only) {
		is $sub, undef, '... including under the TIE option';
		like $@, $errx, '... with the expected error';
	} else {
		ok $sub, 'use as LIST_CACHE succeeds';
	}

	eval { exists $cache{'dummy'}; 1 }
		or skip join("\n", 'exists() unsupported', errlines), 3;

	memoize 'c5',
		SCALAR_CACHE => [ HASH => \%cache ],
		LIST_CACHE => 'FAULT';

	is c5($ARG), 5, 'store value during first memoization';
	unmemoize 'c5';

	untie %cache;

	tie %cache, $module, @_ or die $!;

	# Now something tricky---we'll memoize c23 with the wrong table that
	# has the 5 already cached.
	memoize 'c23',
		SCALAR_CACHE => [ HASH => \%cache ],
		LIST_CACHE => 'FAULT';

	is c23($ARG), 5, '... and find it still there after second memoization';
	unmemoize 'c23';

	untie %cache;

	{ no warnings; memoize 'c23',
		SCALAR_CACHE => [ TIE => $module, @_ ],
		LIST_CACHE => 'FAULT';
	}

	is c23($ARG), 5, '... as well as a third memoization via TIE';
	unmemoize 'c23';
} }

my @file;

sub cleanup { 1 while unlink @file }

sub import {
	(undef, $module, my %arg) = (shift, @_);

	$is_scalar_only = $arg{'is_scalar_only'} ? 2 : 0;
	eval "require $module"
		? plan tests => 5 + $is_scalar_only + ($arg{extra_tests}||0)
		: plan skip_all => join "\n# ", "Could not load $module", errlines;

	my ($basename) = map { s/.*:://; s/_file\z//; 'm_'.$_.$$ } lc $module;
	my $dirfext = $^O eq 'VMS' ? '.sdbm_dir' : '.dir'; # copypaste from DBD::DBM
	@file = map { $_, "$_.db", "$_.pag", $_.$dirfext } $basename;
	cleanup;

	my $pkg = caller;
	no strict 'refs';
	*{$pkg.'::'.$_} = \&$_ for qw(test_dbm cleanup);
	*{$pkg.'::file'} = \$basename;
}

END {
	cleanup;
	if (my @failed = grep -e, @file) {
		@failed = grep !unlink, @failed; # to set $!
		warn "Can't unlink @failed! ($!)\n" if @failed;
	}
}

1;
