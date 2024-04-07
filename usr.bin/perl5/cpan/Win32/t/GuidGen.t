use strict;
use Test;
use Win32;

plan tests => 3;

my $guid1 = Win32::GuidGen();
my $guid2 = Win32::GuidGen();

# {FB9586CD-273B-43BE-A20C-485A6BD4FCD6}
ok($guid1, qr/^{\w{8}(-\w{4}){3}-\w{12}}$/);
ok($guid2, qr/^{\w{8}(-\w{4}){3}-\w{12}}$/);

# Every GUID is unique
ok($guid1 ne $guid2);
