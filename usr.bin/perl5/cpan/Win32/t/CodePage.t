use strict;
use Test;
use Win32;

plan tests => 8;

my $ansicp = Win32::GetACP();
ok($ansicp > 0 && $ansicp <= 65001);

my $inputcp = Win32::GetConsoleCP();
ok($inputcp > 0 && $inputcp <= 65001);

my $outputcp = Win32::GetConsoleOutputCP();
ok($outputcp > 0 && $outputcp <= 65001);

my $oemcp = Win32::GetOEMCP();
ok($oemcp > 0 && $oemcp <= 65001);

ok(Win32::SetConsoleCP($ansicp));
ok(Win32::GetConsoleCP() == $ansicp);

ok(Win32::SetConsoleOutputCP($ansicp));
ok(Win32::GetConsoleOutputCP() == $ansicp);

# Reset things when we're done.
Win32::SetConsoleCP($inputcp);
Win32::SetConsoleOutputCP($outputcp);
