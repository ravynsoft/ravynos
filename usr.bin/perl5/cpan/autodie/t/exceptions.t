#!/usr/bin/perl -w
use strict;
use Test::More;

BEGIN { plan skip_all => "Perl 5.10 only tests" if $] < 5.010; }

# These are tests that depend upon 5.10 (eg, smart-match).
# Basic tests should go in basic_exceptions.t

use 5.010;
use warnings ();
use constant NO_SUCH_FILE => 'this_file_had_better_not_exist_xyzzy';
no if $] >= 5.017011, warnings => "experimental::smartmatch";
no if exists $warnings::Offsets{"deprecated::smartmatch"},
  warnings => "deprecated";

plan 'no_plan';

eval {
	use autodie ':io';
	open(my $fh, '<', NO_SUCH_FILE);
};

ok($@,			"Exception thrown"		        );
ok('open' ~~ $@,	"Exception from open"		        );
ok(':file' ~~ $@,	"Exception from open / class :file"	);
ok(':io' ~~ $@,		"Exception from open / class :io"	);
ok(':all' ~~ $@,	"Exception from open / class :all"	);

eval {
    no warnings 'once';    # To prevent the following close from complaining.
	close(THIS_FILEHANDLE_AINT_OPEN);
};

ok(! $@, "Close without autodie should fail silent");

eval {
	use autodie ':io';
	close(THIS_FILEHANDLE_AINT_OPEN);
};

like($@, qr{Can't close filehandle 'THIS_FILEHANDLE_AINT_OPEN'},"Nice msg from close");

ok($@,			"Exception thrown"		        );
ok('close' ~~ $@,	"Exception from close"		        );
ok(':file' ~~ $@,	"Exception from close / class :file"	);
ok(':io' ~~ $@,		"Exception from close / class :io"	);
ok(':all' ~~ $@,	"Exception from close / class :all"	);

ok $@ eq $@.'',                 "string overloading is complete (eq)";
ok( ($@ cmp $@.'') == 0,        "string overloading is complete (cmp)" );
