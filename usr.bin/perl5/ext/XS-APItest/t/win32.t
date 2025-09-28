#!perl -w
use strict;
use Test::More;
use XS::APItest;
use Config;

plan skip_all => "Tests only apply on MSWin32"
  unless $^O eq "MSWin32";

SKIP:
{
    # [perl #126755] previous the bad drive tests would crash
    $Config{ccflags} =~ /(?:\A|\s)-DPERL_IMPLICIT_SYS\b/
      or skip "need implicit_sys for this test", 1;
    eval "use Encode; 1"
      or skip "Can't load Encode", 1;
    for my $letter ("A" .. "Z", "a" .. "z") {
        my $good_drive = $letter . ":";
        my $result = PerlDir_mapA($good_drive);
        like($result, qr/^$letter:\\/i, "check good drive $letter");

        my $wgood_drive = encode("UTF-16LE", $good_drive . "\0");
        $result = PerlDir_mapW($wgood_drive);
        like(decode("UTF16-LE", $result), qr/^$letter:\\/i,
             "check a good drive (wide)");
    }
    for my $bad ('@', '[', '!', '~', '`', '{') {
        my $bad_drive = "$bad:";
        my $result = PerlDir_mapA($bad_drive);
        is($result, $bad_drive, "check bad drive $bad:");

        my $wbad_drive = encode("UTF-16LE", $bad_drive . "\0");
        $result = PerlDir_mapW($wbad_drive);
        is(decode("UTF16-LE", $result), "$bad_drive\0",
           "check bad drive $bad: (wide)");
    }
    require Win32;
    my (undef, $major, $minor)=  Win32::GetOSVersion();
    if ($major >= 5 && $minor >= 1) { #atleast XP, 2K only has V5
    #this is testing the current state of things, specifically manifest stuff
    #this test can be changed if perls relationship to comctl32.dll changes
        my @ccver = Comctl32Version();
        cmp_ok($ccver[0], '>=', 6, "comctl32.dll is atleast version 6")
          or diag "comctl32 version is (@ccver)";
    }
}

done_testing();
