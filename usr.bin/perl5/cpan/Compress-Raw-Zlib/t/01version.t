BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict ;
use warnings ;

use Test::More ;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 9 + $extra ;

    use_ok('Compress::Raw::Zlib', 2) ;
}

use CompTestUtils;


# Check zlib_version and ZLIB_VERSION are the same.
test_zlib_header_matches_library();

SKIP:
{
    # If running a github workflow that tests upstream zlib/zlib-ng, check we have the version requested

    # Not github or not asking for explicit verson, so skip
    skip "Not github", 7
        if ! (defined $ENV{GITHUB_ACTION} && defined $ENV{ZLIB_VERSION}) ;

    my $expected_version =  $ENV{ZLIB_VERSION} ;
    # zlib prefixes tags with a "v", so remove
    $expected_version =~ s/^v//i;

    skip "Skipping version tests for 'develop' branch", 7
        if ($expected_version eq 'develop') ;

    if ($ENV{USE_ZLIB_NG})
    {
        # zlib-ng native
        my $zv = Compress::Raw::Zlib::zlibng_version();
        is substr($zv, 0, length($expected_version)), $expected_version, "Expected version is $expected_version";
        ok ! Compress::Raw::Zlib::is_zlib_native(), "! is_zlib_native";
        ok   Compress::Raw::Zlib::is_zlibng(), "is_zlibng";
        ok   Compress::Raw::Zlib::is_zlibng_native(), "is_zlibng_native";
        ok ! Compress::Raw::Zlib::is_zlibng_compat(), "! is_zlibng_compat";
        is   Compress::Raw::Zlib::zlib_version(), '', "zlib_version() should be empty";
        is   Compress::Raw::Zlib::ZLIB_VERSION, '', "ZLIB_VERSION should be empty";
    }
    elsif ($ENV{ZLIB_NG_PRESENT})
    {
        # zlib-ng compat
        my %zlibng2zlib = (
            '2.0.0' => '1.2.11.zlib-ng',
            '2.0.1' => '1.2.11.zlib-ng',
            '2.0.2' => '1.2.11.zlib-ng',
            '2.0.3' => '1.2.11.zlib-ng',
            '2.0.4' => '1.2.11.zlib-ng',
            '2.0.5' => '1.2.11.zlib-ng',
            '2.0.6' => '1.2.11.zlib-ng',
        );

        my $zv = Compress::Raw::Zlib::zlibng_version();

        my $compat_ver = $zlibng2zlib{$expected_version};

        is substr($zv, 0, length($expected_version)), $expected_version, "Expected Version is $expected_version";
        ok ! Compress::Raw::Zlib::is_zlib_native(), "! is_zlib_native";
        ok   Compress::Raw::Zlib::is_zlibng(), "is_zlibng";
        ok ! Compress::Raw::Zlib::is_zlibng_native(), "! is_zlibng_native";
        ok   Compress::Raw::Zlib::is_zlibng_compat(), "is_zlibng_compat";
        is   Compress::Raw::Zlib::zlib_version(), $compat_ver, "zlib_version() should be $compat_ver";
        is   Compress::Raw::Zlib::ZLIB_VERSION, $compat_ver, "ZLIB_VERSION should be $compat_ver";
    }
    else
    {
        # zlib native
        my $zv = Compress::Raw::Zlib::zlib_version();
        is substr($zv, 0, length($expected_version)), $expected_version, "Expected Version is $expected_version";
        ok   Compress::Raw::Zlib::is_zlib_native(), "is_zlib_native";
        ok ! Compress::Raw::Zlib::is_zlibng(), "! is_zlibng";
        ok ! Compress::Raw::Zlib::is_zlibng_native(), "! is_zlibng_native";
        ok ! Compress::Raw::Zlib::is_zlibng_compat(), "! is_zlibng_compat";
        is   Compress::Raw::Zlib::zlibng_version(), '', "zlibng_version() should be empty";
        is   Compress::Raw::Zlib::ZLIBNG_VERSION, '', "ZLIBNG_VERSION should be empty";    }

}
