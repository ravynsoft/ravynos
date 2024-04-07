use strict;

BEGIN {
    require Time::HiRes;
    unless(&Time::HiRes::d_hires_stat) {
        require Test::More;
        Test::More::plan(skip_all => "no hi-res stat");
    }
    if($^O =~ /\A(?:cygwin|MSWin)/) {
        require Test::More;
        Test::More::plan(skip_all =>
                "$^O file timestamps not reliable enough for stat test");
    }
}

use Test::More tests => 43;
BEGIN { push @INC, '.' }
use t::Watchdog;

my @atime;
my @mtime;
for (1..5) {
    note "cycle $_";
    Time::HiRes::sleep(rand(0.1) + 0.1);
    open(X, '>', $$);
    print X $$;
    close(X);
    my($a, $stat, $b) = ("a", [Time::HiRes::stat($$)], "b");
    is $a, "a", "stat stack discipline";
    is $b, "b", "stat stack discipline";
    is ref($stat), "ARRAY", "stat returned array";
    push @mtime, $stat->[9];
    ($a, my $lstat, $b) = ("a", [Time::HiRes::lstat($$)], "b");
    is $a, "a", "lstat stack discipline";
    is $b, "b", "lstat stack discipline";
    SKIP: {
        if($^O eq "haiku") {
            skip "testing stat access time on Haiku", 2;
        }
        if ($ENV{PERL_FILE_ATIME_CHANGES}) {
            # something else might access the file, changing atime
            $lstat->[8] = $stat->[8];
        }
        is_deeply $lstat, $stat, "write: stat and lstat returned same values";
        Time::HiRes::sleep(rand(0.1) + 0.1);
        open(X, '<', $$);
        <X>;
        close(X);
        $stat = [Time::HiRes::stat($$)];
        push @atime, $stat->[8];
        $lstat = [Time::HiRes::lstat($$)];
        is_deeply $lstat, $stat, "read:  stat and lstat returned same values";
    }
}
1 while unlink $$;
note ("mtime = @mtime");
note ("atime = @atime");
my $ai = 0;
my $mi = 0;
my $ss = 0;
for (my $i = 1; $i < @atime; $i++) {
    if ($atime[$i] >= $atime[$i-1]) {
        $ai++;
    }
    if ($atime[$i] > int($atime[$i])) {
        $ss++;
    }
}
for (my $i = 1; $i < @mtime; $i++) {
    if ($mtime[$i] >= $mtime[$i-1]) {
        $mi++;
    }
    if ($mtime[$i] > int($mtime[$i])) {
        $ss++;
    }
}
note ("ai = $ai, mi = $mi, ss = $ss");
# Need at least 75% of monotonical increase and
# 20% of subsecond results. Yes, this is guessing.
SKIP: {
    skip "no subsecond timestamps detected", 1 if $ss == 0;
    skip "testing stat access on Haiku", 1 if $^O eq "haiku";
    ok $mi/(@mtime-1) >= 0.75 && $ai/(@atime-1) >= 0.75 &&
             $ss/(@mtime+@atime) >= 0.2,
        "monotonical increase and subsecond results within expected parameters";
}

my $targetname = "tgt$$";
my $linkname = "link$$";
SKIP: {
    open(X, '>', $targetname);
    print X $$;
    close(X);
    eval { symlink $targetname, $linkname or die "can't symlink: $!"; };
    skip "can't symlink", 7 if $@ ne "";
    note "compare Time::HiRes::stat with ::lstat";
    my @tgt_stat = Time::HiRes::stat($targetname);
    my @tgt_lstat = Time::HiRes::lstat($targetname);
    my @lnk_stat = Time::HiRes::stat($linkname);
    my @lnk_lstat = Time::HiRes::lstat($linkname);
    my $exp = 13;
    is scalar(@tgt_stat), $exp,  "stat on target";
    is scalar(@tgt_lstat), $exp, "lstat on target";
    is scalar(@lnk_stat), $exp,  "stat on link";
    is scalar(@lnk_lstat), $exp, "lstat on link";
    skip "testing stat access on Haiku", 3 if $^O eq "haiku";
    is_deeply \@tgt_stat, \@tgt_lstat, "stat and lstat return same values on target";
    is_deeply \@tgt_stat, \@lnk_stat,  "stat and lstat return same values on link";
    isnt $lnk_lstat[2], $tgt_stat[2],
        "target stat mode value differs from link lstat mode value";
}
1 while unlink $linkname;
1 while unlink $targetname;

1;
