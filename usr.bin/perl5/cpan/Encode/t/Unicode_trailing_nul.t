BEGIN {
    if ( $] < 5.009 ) {
        print "1..0 # Skip: Perl <= 5.9 or later required\n";
        exit 0;
    }
}
use strict;
use Test::More;

use Encode;
use File::Temp;
use File::Spec;

# This test relies on https://github.com/Perl/perl5/issues/10623;
# if that bug is ever fixed then this test may never fail again.

my $foo = Encode::decode("UTF-16LE", "/\0v\0a\0r\0/\0f\0f\0f\0f\0f\0f\0/\0u\0s\0e\0r\0s\0/\0s\0u\0p\0e\0r\0m\0a\0n\0");

my ($fh, $path) = File::Temp::tempfile( CLEANUP => 1 );

note "temp file: $path";

# Perl gives the internal PV to exec .. which is buggy/wrong but
# useful here:
system( $^X, '-e', "open my \$fh, '>>', '$path' or die \$!; print {\$fh} \$ARGV[0]", $foo );
die if $?;

my $output = do { local $/; <$fh> };

is( $output, "/var/ffffff/users/superman", 'UTF-16 decodes with trailing NUL' );

done_testing();
