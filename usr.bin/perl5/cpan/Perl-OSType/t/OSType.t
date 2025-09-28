use strict;
use warnings;

use Test::More 0.88;

use constant NON_EXISTENT_OS => 'titanix'; #the system they said could not go down...

#--------------------------------------------------------------------------#
# API tests
#--------------------------------------------------------------------------#

require_ok('Perl::OSType');

can_ok( 'Perl::OSType', 'os_type' );

my @functions = qw/os_type is_os_type/;
for my $sub (@functions) {
    ok( eval { Perl::OSType->import($sub); 1 }, "importing $sub()" );
    can_ok( 'main', $sub );
}

my $test_pkg = "testpackage$$";

ok( eval "package $test_pkg; use Perl::OSType ':all'; 1",
    "Testing 'use Perl::OSType qw/:all/'" );

can_ok( $test_pkg, @functions );

#--------------------------------------------------------------------------#
# os_type
#--------------------------------------------------------------------------#

{
    my $fcn = 'os_type()';

    ok( my $current_type = os_type(), "$fcn: without arguments" );

    is( $current_type, os_type($^O), "... matches os_type($^O)" );

    is( os_type(NON_EXISTENT_OS), '', "$fcn: unknown OS returns empty string" );

    is( os_type(''), '', "$fcn: empty string returns empty string" );

    local $^O = 'linux';

    is( os_type(undef), 'Unix', "$fcn: explicit undef uses $^O" );
}

#--------------------------------------------------------------------------#
# is_os_type
#--------------------------------------------------------------------------#

{
    my $fcn = 'is_os_type()';

    is( is_os_type(NON_EXISTENT_OS), '', "$fcn: non-existent type is false" );

    is( is_os_type(''), undef, "$fcn: empty string type is false" );

    is( is_os_type( 'Unix', NON_EXISTENT_OS ), '', "$fcn: non-existent OS is false" );

    local $^O = 'vos';
    ok( !is_os_type('Unix'), "$fcn: false" );
    ok( is_os_type('VOS'),   "$fcn: true" );
    ok( !is_os_type(),       "$fcn: false if no type provided" );
}

done_testing;

