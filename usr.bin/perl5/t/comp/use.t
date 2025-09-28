#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = ('../lib', 'lib');
    $INC{"feature.pm"} = 1; # so we don't attempt to load feature.pm
}

print "1..85\n";

# Can't require test.pl, as we're testing the use/require mechanism here.

my $test = 1;

sub _ok {
    my ($type, $got, $expected, $name) = @_;

    my $result;
    if ($type eq 'is') {
	$result = $got eq $expected;
    } elsif ($type eq 'isnt') {
	$result = $got ne $expected;
    } elsif ($type eq 'like') {
	$result = $got =~ $expected;
    } elsif ($type eq 'ok') {
	$result = not not $got;
    } else {
	die "Unexpected type '$type'$name";
    }
    if ($result) {
	if ($name) {
	    print "ok $test - $name\n";
	} else {
	    print "ok $test\n";
	}
    } else {
	if ($name) {
	    print "not ok $test - $name\n";
	} else {
	    print "not ok $test\n";
	}
	my @caller = caller(1);
	print "# Failed test at $caller[1] line $caller[2]\n";
	print "# Got      '$got'\n";
	if ($type eq 'is') {
	    print "# Expected '$expected'\n";
	} elsif ($type eq 'isnt') {
	    print "# Expected not '$expected'\n";
	} elsif ($type eq 'like') {
	    print "# Expected $expected\n";
	} elsif ($type eq 'ok') {
	    print "# Expected a true value\n";
	}
    }
    $test = $test + 1;
    $result;
}

sub like ($$;$) {
    _ok ('like', @_);
}
sub is ($$;$) {
    _ok ('is', @_);
}
sub isnt ($$;$) {
    _ok ('isnt', @_);
}
sub ok($;$) {
    _ok ('ok', shift, undef, @_);
}

eval "use 5";           # implicit semicolon
is ($@, '');

eval "use 5;";
is ($@, '');

eval "{use 5}";         # [perl #70884]
is ($@, '');

eval "{use 5   }";      # [perl #70884]
is ($@, '');

# new style version numbers

eval q{ use v5.5.630; };
is ($@, '');

eval q{ use 10.0.2; };
like ($@, qr/^\QPerl v10.0.2 required\E/);

eval "use 5.000";	# implicit semicolon
is ($@, '');

eval "use 5.000;";
is ($@, '');

eval "use 6.000;";
like ($@, qr/\QPerl v6.0.0 required--this is only $^V, stopped\E/);

eval "no 6.000;";
is ($@, '');

eval "no 5.000;";
like ($@, qr/\QPerls since v5.0.0 too modern--this is $^V, stopped\E/);

eval "use 5.6;";
like ($@, qr/\QPerl v5.600.0 required (did you mean v5.6.0?)--this is only $^V, stopped\E/);

eval "use 5.8;";
like ($@, qr/\QPerl v5.800.0 required (did you mean v5.8.0?)--this is only $^V, stopped\E/);

eval "use 5.9;";
like ($@, qr/\QPerl v5.900.0 required (did you mean v5.9.0?)--this is only $^V, stopped\E/);

eval "use 5.10;";
like ($@, qr/\QPerl v5.100.0 required (did you mean v5.10.0?)--this is only $^V, stopped\E/);

eval "use 5.11;";
like ($@, qr/\QPerl v5.110.0 required (did you mean v5.11.0?)--this is only $^V, stopped\E/);

eval sprintf "use %.6f;", $];
is ($@, '');


eval sprintf "use %.6f;", $] - 0.000001;
is ($@, '');

eval sprintf("use %.6f;", $] + 1);
like ($@, qr/Perl v6\.\d+\.\d+ required--this is only \Q$^V\E, stopped/a);

eval sprintf "use %.6f;", $] + 0.00001;
like ($@, qr/Perl v5\.\d+\.\d+ required--this is only \Q$^V\E, stopped/a);

# check that "use 5.11.0" (and higher) loads strictures
eval 'use 5.11.0; ${"foo"} = "bar";';
like ($@, qr/\QCan't use string ("foo") as a SCALAR ref while "strict refs" in use\E/);
# but that they can be disabled
eval 'use 5.11.0; no strict "refs"; ${"foo"} = "bar";';
is ($@, "");
# and they are properly scoped
eval '{use 5.11.0;} ${"foo"} = "bar";';
is ($@, "");
eval 'no strict; use 5.012; ${"foo"} = "bar"';
is $@, "", 'explicit "no strict" overrides later ver decl';
eval 'use strict; use 5.01; ${"foo"} = "bar"';
like $@, qr/^Can't use string/,
    'explicit use strict overrides later use 5.01';
eval 'use strict "subs"; use 5.012; ${"foo"} = "bar"';
like $@, qr/^Can't use string/,
    'explicit use strict "subs" does not stop ver decl from enabling refs';
{
    my $warnings = "";
    local $SIG{__WARN__} = sub { $warnings .= $_[0] };
    eval 'use 5.012; use 5.01; ${"foo"} = "bar"';
    is $@, "", 'use 5.01 overrides implicit strict from prev ver decl';
    like $warnings, qr/^Downgrading a use VERSION declaration to below v5.11 is deprecated, and will become fatal in Perl 5.40 at /,
        'use 5.01 after use 5.012 provokes deprecation warning';
}
eval 'no strict "subs"; use 5.012; ${"foo"} = "bar"';
ok $@, 'no strict subs allows ver decl to enable refs';
eval 'no strict "subs"; use 5.012; $nonexistent_pack_var';
ok $@, 'no strict subs allows ver decl to enable vars';
eval 'no strict "refs"; use 5.012; fancy_bareword';
ok $@, 'no strict refs allows ver decl to enable subs';
eval 'no strict "refs"; use 5.012; $nonexistent_pack_var';
ok $@, 'no strict refs allows ver decl to enable subs';
eval 'no strict "vars"; use 5.012; ${"foo"} = "bar"';
ok $@, 'no strict vars allows ver decl to enable refs';
eval 'no strict "vars"; use 5.012; ursine_word';
ok $@, 'no strict vars allows ver decl to enable subs';


{ use test_use }	# check that subparse saves pending tokens

use test_use { () };
is ref $test_use::got[0], 'HASH', 'use parses arguments in term lexing cx';

local $test_use::VERSION = 1.0;

eval "use test_use 0.9";
is ($@, '');

eval "use test_use 1.0";
is ($@, '');

eval "use test_use 1.01";
isnt ($@, '');

eval "use test_use 0.9 qw(fred)";
is ($@, '');

is("@test_use::got", "fred");

eval "use test_use 1.0 qw(joe)";
is ($@, '');

is("@test_use::got", "joe");

eval "use test_use 1.01 qw(freda)";
isnt($@, '');

is("@test_use::got", "joe");

{
    local $test_use::VERSION = 35.36;
    eval "use test_use v33.55";
    is ($@, '');

    eval "use test_use v100.105";
    like ($@, qr/\Qtest_use version v100.105.0 required--this is only version v35.360.0\E/);

    eval "use test_use 33.55";
    is ($@, '');

    eval "use test_use 100.105";
    like ($@, qr/\Qtest_use version 100.105 required--this is only version 35.36\E/);

    local $test_use::VERSION = '35.36';
    eval "use test_use v33.55";
    like ($@, '');

    eval "use test_use v100.105";
    like ($@, qr/\Qtest_use version v100.105.0 required--this is only version v35.360.0\E/);

    eval "use test_use 33.55";
    is ($@, '');

    eval "use test_use 100.105";
    like ($@, qr/\Qtest_use version 100.105 required--this is only version 35.36\E/);

    local $test_use::VERSION = v35.36;
    eval "use test_use v33.55";
    is ($@, '');

    eval "use test_use v100.105";
    like ($@, qr/\Qtest_use version v100.105.0 required--this is only version v35.36.0\E/);

    eval "use test_use 33.55";
    is ($@, '');

    eval "use test_use 100.105";
    like ($@, qr/\Qtest_use version 100.105 required--this is only version v35.36\E/);
}


{
    # Regression test for patch 14937: 
    #   Check that a .pm file with no package or VERSION doesn't core.
    # (git commit 2658f4d9934aba5f8b23afcc078dc12b3a40223)
    eval "use test_use_14937 3";
    like ($@, qr/^\Qtest_use_14937 defines neither package nor VERSION--version check failed at\E/);
}

my @ver = split /\./, sprintf "%vd", $^V;

foreach my $index (-3..+3) {
    foreach my $v (0, 1) {
	my @parts = @ver;
	if ($index) {
	    if ($index < 0) {
		# Jiggle one of the parts down
		--$parts[-$index - 1];
		if ($parts[-$index - 1] < 0) {
		    # perl's version number ends with '.0'
		    $parts[-$index - 1] = 0;
		    $parts[-$index - 2] -= 2;
		}
	    } else {
		# Jiggle one of the parts up
		++$parts[$index - 1];
	    }
	}
	my $v_version = sprintf "v%d.%d.%d", @parts;
	my $version;
	if ($v) {
	    $version = $v_version;
	} else {
	    $version = $parts[0] + $parts[1] / 1000 + $parts[2] / 1000000;
	}

	eval "use $version";
	if ($index > 0) {
	    # The future
	    like ($@,
		  qr/\QPerl $v_version required--this is only $^V, stopped\E/,
		  "use $version");
	} else {
	    # The present or past
	    is ($@, '', "use $version");
	}

	eval "no $version";
	if ($index <= 0) {
	    # The present or past
	    like ($@,
		  qr/\QPerls since $v_version too modern--this is $^V, stopped\E/,
		  "no $version");
	} else {
	    # future
	    is ($@, '', "no $version");
	}
    }
}

