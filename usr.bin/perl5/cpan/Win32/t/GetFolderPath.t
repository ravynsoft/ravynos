use strict;
use Test;
use Win32;

plan tests => 1;

# "windir" exists back to Win9X; "SystemRoot" only exists on WinNT and later.
ok(Win32::GetFolderPath(Win32::CSIDL_WINDOWS), $ENV{WINDIR});
