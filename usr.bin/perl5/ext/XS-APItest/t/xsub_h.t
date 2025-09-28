#!perl -w
use strict;

use Test::More;

BEGIN { use_ok('XS::APItest') };

our ($XS_VERSION, $VERSION);

# This is what the code expects
my $real_version = $XS::APItest::VERSION;

sub default {
    return ($_[0], undef) if @_;
    return ($XS_VERSION, 'XS_VERSION') if defined $XS_VERSION;
    return ($VERSION, 'VERSION');
}

sub expect_good {
    my $package = $_[0];
    my $version = @_ >= 2 ? ", $_[1]" : '';
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    is_deeply([XS_VERSION_defined(@_)], [],
	      "Is good for $package$version");

    is_deeply([XS_VERSION_undef(@_)], [],
	      "Is good for $package$version with #undef XS_VERSION");
}

sub expect_bad {
    my $what = shift;
    my $package = $_[0];
    my $desc; # String to use in test descriptions

    if (defined $what) {
	$what = quotemeta('$' . $package . '::' . $what);
    } else {
	$what = 'bootstrap parameter';
    }
    if (@_ >= 2) {
	$desc = "$_[0], $_[1]";
    } else {
	$desc = $_[0];
    }

    is(eval {XS_VERSION_defined(@_); "Oops"}, undef, "Is bad for $desc");
    like($@,
	 qr/$package object version $real_version does not match $what/,
	 'expected error message');

    is_deeply([XS_VERSION_undef(@_)], [],
	      "but is good for $desc with #undef XS_VERSION");
}

# With neither $VERSION nor $XS_VERSION defined, no check is made if no version
# is passed in
expect_good('dummy_package');

foreach ($real_version, version->new($real_version)) {
    expect_good('dummy_package', $_);
}

foreach (3.14, version->new(3.14)) {
    expect_bad(undef, 'dummy_package', $_);
}

my @versions = ($real_version, version->new($real_version),
		3.14, version->new(3.14));

# Package variables
foreach $XS_VERSION (undef, @versions) {
    foreach $VERSION (undef, @versions) {
	my ($expect, $what) = default();
	if (defined $expect) {
	    if ($expect eq $real_version) {
		expect_good('main');
	    } else {
		expect_bad($what, 'main');
	    }
	}
	foreach my $param (@versions) {
	    my ($expect, $what) = default($param);
	    if ($expect eq $real_version) {
		expect_good('main', $param);
	    } else {
		expect_bad($what, 'main', $param);
	    }
	}
    }
}

{
    my $count = 0;
    {
	package Counter;
	our @ISA = 'version';
	sub new {
	    ++$count;
	    return version::new(@_);
	}

	sub DESTROY {
	    --$count;
	}
    }

    {
	my $var = Counter->new();
	is ($count, 1, "1 object exists");
	is (eval {XS_VERSION_empty('main', $var); 1}, undef);
	like ($@, qr/Invalid version format \(version required\)/);
    }

    is ($count, 0, "no objects exist");
}

is_deeply([XS_APIVERSION_valid("Pie")], [], "XS_APIVERSION_BOOTCHECK passes");
is(eval {XS_APIVERSION_invalid("Pie"); 1}, undef,
   "XS_APIVERSION_BOOTCHECK croaks for an invalid version");
like($@, qr/Perl API version v1.0.16 of Pie does not match v5\.\d+\.\d+/,
     "expected error");

my @xsreturn;
@xsreturn = XS::APItest::XSUB::xsreturn(2);
is scalar @xsreturn, 2, 'returns a list of 2 elements';
is $xsreturn[0], 0;
is $xsreturn[1], 1;

my $xsreturn = XS::APItest::XSUB::xsreturn(3);
is $xsreturn, 2, 'returns the last item on the stack';

( $xsreturn ) = XS::APItest::XSUB::xsreturn(3);
is $xsreturn, 0, 'gets the first item on the stack';

is XS::APItest::XSUB::xsreturn_iv(), -2**31+1, 'XSRETURN_IV returns signed int';
is XS::APItest::XSUB::xsreturn_uv(), 2**31+1, 'XSRETURN_UV returns unsigned int';
is XS::APItest::XSUB::xsreturn_nv(), 0.25, 'XSRETURN_NV returns double';
is XS::APItest::XSUB::xsreturn_pv(), "returned", 'XSRETURN_PV returns string';
is XS::APItest::XSUB::xsreturn_pvn(), "returned", 'XSRETURN_PVN returns string with length';
ok !XS::APItest::XSUB::xsreturn_no(), 'XSRETURN_NO returns falsey';
ok XS::APItest::XSUB::xsreturn_yes(), 'XSRETURN_YES returns truthy';

is XS::APItest::XSUB::xsreturn_undef(), undef, 'XSRETURN_UNDEF returns undef in scalar context';
my @xs_undef = XS::APItest::XSUB::xsreturn_undef();
is scalar @xs_undef, 1, 'XSRETURN_UNDEF returns a single-element list';
is $xs_undef[0], undef, 'XSRETURN_UNDEF returns undef in list context';

my @xs_empty = XS::APItest::XSUB::xsreturn_empty();
is scalar @xs_empty, 0, 'XSRETURN_EMPTY returns empty list in array context';
my $xs_empty = XS::APItest::XSUB::xsreturn_empty();
is $xs_empty, undef, 'XSRETURN_EMPTY returns undef in scalar context';


done_testing();
