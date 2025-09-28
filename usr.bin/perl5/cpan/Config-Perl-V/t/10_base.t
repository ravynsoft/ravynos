#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 42;
    unless ($ENV{PERL_CORE}) {
	require Test::NoWarnings;
	Test::NoWarnings->import ();
	$tests++;
	}

    plan tests => $tests;

    use_ok ("Config::Perl::V");
    }

ok (my $conf = Config::Perl::V::myconfig,	"Read config");
ok (exists $conf->{$_},	"Has $_ entry") for qw( build environment config inc );
is (lc $conf->{build}{osname}, lc $conf->{config}{osname}, "osname");

# Test summary
ok (my $info1 = Config::Perl::V::summary ($conf), "Get a summary for \$conf");
ok (my $info2 = Config::Perl::V::summary,         "Get a summary for \$^X");
is_deeply ($info1, $info2, "Info should match");

ok (my $sig = Config::Perl::V::signature, "Get signature");
like ($sig, qr{^[0-9a-f]{32}$}, "Valid md5");

my $no_md5 = "0" x 32;
ok (my $bad = Config::Perl::V::signature ({ cfg => 0 }), "Signature on invalid data");
is ($bad, $no_md5, "Invalid md5");
ok (   $bad = Config::Perl::V::signature ({ config => {} }), "Signature on incomplete data");
is ($bad, $no_md5, "Invalid md5");
ok (   $bad = Config::Perl::V::signature ({ config => 0, build => {} }), "Signature on invalid data");
is ($bad, $no_md5, "Invalid md5");
ok (   $bad = Config::Perl::V::signature ({ config => {}, build => 0 }), "Signature on invalid data");
is ($bad, $no_md5, "Invalid md5");

SKIP: {
    # Test that the code that shells out to perl -V and parses the output
    # gives the same results as the code that calls Config::* routines directly.
    defined &Config::compile_date or
	skip "This perl doesn't provide perl -V in the Config module", 2;
    eval q{no warnings "redefine"; sub Config::compile_date { return undef }};
    is (Config::compile_date (), undef, "Successfully overriden compile_date");
    is_deeply (Config::Perl::V::myconfig, $conf,
	"perl -V parsing code produces same result as the Config module");
    }

$ENV{CPV_TEST_ENV} = 42;
ok ($conf = Config::Perl::V::myconfig ({ env => qr{^CPV_TEST_ENV$} }), "Read config plus ENV");
ok (exists $conf->{$_},	"Has $_ entry") for qw( build environment config inc environment );
ok (my $eh = $conf->{environment}, "Get ENV from conf");
is ($eh->{CPV_TEST_ENV}, 42, "Valid entry");

ok ($conf = Config::Perl::V::myconfig ([ env => qr{^CPV_TEST_ENV$} ]), "Read config plus ENV");
ok (exists $conf->{$_},	"Has $_ entry") for qw( build environment config inc environment );
ok ($eh = $conf->{environment}, "Get ENV from conf");
is ($eh->{CPV_TEST_ENV}, 42, "Valid entry");

ok ($conf = Config::Perl::V::myconfig (  env => qr{^CPV_TEST_ENV$}  ), "Read config invalid arguments");
is ($conf->{environment}{CPV_TEST_ENV}, undef, "No entry");

delete $INC{"Digest/MD5.pm"};
delete $INC{"Digest/base.pm"};
$INC{"Digest/MD5"} = "./flooble/blurgh/Digest/MD5.pm";
local @INC = ("xyzzy$$"); # Should be unable to find Digest::MD5
ok ($sig = Config::Perl::V::signature, "Get signature (No Digest::MD5)");
is ($sig, $no_md5, "Valid md5");
