use strict;
use Test;
use Win32;

plan tests => 16;

my $cwd = Win32::GetCwd;
my @cwd = split/\\/, $cwd;
my $file = pop @cwd;
my $dir = join('\\', @cwd);

ok(scalar Win32::GetFullPathName('.'), $cwd);
ok((Win32::GetFullPathName('.'))[0], "$dir\\");
ok((Win32::GetFullPathName('.'))[1], $file);

ok((Win32::GetFullPathName('./'))[0], "$cwd\\");
ok((Win32::GetFullPathName('.\\'))[0], "$cwd\\");
ok((Win32::GetFullPathName('./'))[1], "");

ok(scalar Win32::GetFullPathName($cwd), $cwd);
ok((Win32::GetFullPathName($cwd))[0], "$dir\\");
ok((Win32::GetFullPathName($cwd))[1], $file);

ok(scalar Win32::GetFullPathName(substr($cwd,2)), $cwd);
ok((Win32::GetFullPathName(substr($cwd,2)))[0], "$dir\\");
ok((Win32::GetFullPathName(substr($cwd,2)))[1], $file);

ok(scalar Win32::GetFullPathName('/Foo Bar/'), substr($cwd,0,2)."\\Foo Bar\\");

chdir(my $dird = $dir !~ /^[A-Z]:$/ ? $dir : "$dir\\");
ok(scalar Win32::GetFullPathName('.'), $dird);

ok((Win32::GetFullPathName($file))[0], "$dir\\");
ok((Win32::GetFullPathName($file))[1], $file);
