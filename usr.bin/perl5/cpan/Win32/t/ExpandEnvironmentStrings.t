use strict;
use Test;
use Win32;

plan tests => 1;

ok(Win32::ExpandEnvironmentStrings("%WINDIR%"), $ENV{WINDIR});
