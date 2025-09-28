#!perl

use Test::More;
BEGIN {
    if ( $ENV{PERL_CORE} ) {
    require Config;
	if ( $Config::Config{extensions} !~ /(?<!\S)Win32CORE(?!\S)/ ) {
	    plan skip_all => "Win32CORE extension not built";
	    exit();
	}
    }

    plan tests => 6;
};
use_ok( "Win32CORE" );

# Make sure that Win32 is not yet loaded
ok(!defined &Win32::ExpandEnvironmentStrings,
   "ensure other Win32::* functions aren't loaded yet");

$^E = 42;
$! = 4;
ok(eval { Win32::GetLastError(); 1 }, 'GetLastError() works on the first call');
my $errno = 0 + $!;
my $sys_errno = 0 + $^E;
SKIP: {
    $^O eq "cygwin"
        and skip q($^E isn't useful on cygwin), 1;
    # [perl #42925] - Loading Win32::GetLastError() via the forwarder function
    # should not affect the last error being retrieved
    is($sys_errno, 42, '$^E is preserved across Win32 autoload');
}
is($errno, 4, '$! is preserved across Win32 autoload');

# Now all Win32::* functions should be loaded
ok(defined &Win32::ExpandEnvironmentStrings,
   "check other Win32::* functions are loaded");
