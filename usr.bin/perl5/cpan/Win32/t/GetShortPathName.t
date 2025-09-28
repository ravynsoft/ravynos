use strict;
use Test;
use Win32;

BEGIN {
    Win32::CreateFile("8dot3test_canary_GetShortPathName $$");
    my $canary = Win32::GetShortPathName("8dot3test_canary_GetShortPathName $$");
    unlink("8dot3test_canary_GetShortPathName $$");
    if ( length $canary > 12 ) {
        print "1..0 # Skip: The system and/or current volume is not configured to support short names.\n";
        exit 0;        
    }
}

my $path = "Long Path $$";
unlink($path);
END { unlink $path }

plan tests => 5;

Win32::CreateFile($path);
ok(-f $path);

my $short = Win32::GetShortPathName($path);
ok($short, qr/^\S{1,8}(\.\S{1,3})?$/);
ok(-f $short);

unlink($path);
ok(!-f $path);
ok(!defined Win32::GetShortPathName($path));
