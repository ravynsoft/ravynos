use strict;
use Test;
use Win32;

my $path = "testing-$$";
rmdir($path)  if -d $path;
unlink($path) if -f $path;

plan tests => 15;

ok(!-d $path);
ok(!-f $path);

ok(Win32::CreateDirectory($path));
ok(-d $path);

ok(!Win32::CreateDirectory($path));
ok(!Win32::CreateFile($path));

ok(rmdir($path));
ok(!-d $path);

ok(Win32::CreateFile($path));
ok(-f $path);
ok(-s $path, 0);

ok(!Win32::CreateDirectory($path));
ok(!Win32::CreateFile($path));

ok(unlink($path));
ok(!-f $path);
