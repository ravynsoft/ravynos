#! perl

use strict;
use warnings;

use Test::More 0.89;

plan skip_all => 'This module is a no-op on perls earlier than 5.010' if "$]" < 5.010000;

local $SIG{__WARN__} = sub { fail("Got unexpected warning"); diag($_[0]) };

if ($] >= 5.010000) {
	is (eval <<'END', 1, 'state compiles') or diag $@;
	use experimental 'state';
	state $foo = 1;
	is($foo, 1, '$foo is 1');
	1;
END
}

use warnings ();
if ( $] >= 5.010001
    && ( $] < 5.017011 || exists $warnings::Offsets{"experimental::smartmatch"} ) ) {
	is (eval <<'END', 1, 'switch compiles') or diag $@;
	use experimental 'switch';
	sub bar { 1 };
	given(1) {
		when (\&bar) {
			pass("bar matches 1");
		}
		default {
			fail("bar matches 1");
		}
	}
	1;
END
}

if ( $] >= 5.010001
    && ( $] < 5.017011 || exists $warnings::Offsets{"experimental::smartmatch"} ) ) {
	is (eval <<'END', 1, 'smartmatch compiles') or diag $@;
	use experimental 'smartmatch';
	sub baz { 1 };
	is(1 ~~ \&baz, 1, "is 1");
	1;
END
}

if ($] >= 5.018) {
	is (eval <<'END', 1, 'lexical subs compiles') or diag $@;
	use experimental 'lexical_subs';
	my sub foo { 1 };
	is(foo(), 1, "foo is 1");
	1;
END
}

if ($] >= 5.026000) {
	is (eval <<'END', 1, 'declared refs compiles') or diag $@;
	use experimental 'declared_refs';
	my @b;
	my \@a = \@b;
	is(\@a, \@b, '@a and @b are the same after \@a=\@b');
	1;
END
}
elsif ($] >= 5.021005) {
	is (eval <<'END', 1, 'ref aliasing compiles') or diag $@;
	use experimental 'refaliasing';
	my (@a, @b);
	\@a = \@b;
	is(\@a, \@b, '@a and @b are the same after \@a=\@b');
	1;
END
}

done_testing;

