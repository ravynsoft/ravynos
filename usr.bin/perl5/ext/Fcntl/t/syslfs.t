# NOTE: this file tests how large files (>2GB) work with raw system IO.
# stdio: open(), tell(), seek(), print(), read() is tested in t/op/lfs.t.
# If you modify/add tests here, remember to update also t/op/lfs.t.

BEGIN {
	require Config; import Config;
	# Don't bother if there are no quad offsets.
	if ($Config{lseeksize} < 8) {
		print "1..0 # Skip: no 64-bit file offsets\n";
		exit(0);
	}
	require Fcntl; import Fcntl qw(/^O_/ /^SEEK_/);
}

use strict;
use File::Temp 'tempfile';
use Test::More;

our @s;

(undef, my $big0) = tempfile(UNLINK => 1);
(undef, my $big1) = tempfile(UNLINK => 1);
(undef, my $big2) = tempfile(UNLINK => 1);

my $explained;

sub explain {
    unless ($explained++) {
	print <<EOM;
#
# If the lfs (large file support: large meaning larger than two
# gigabytes) tests are skipped or fail, it may mean either that your
# process (or process group) is not allowed to write large files
# (resource limits) or that the file system (the network filesystem?)
# you are running the tests on doesn't let your user/group have large
# files (quota) or the filesystem simply doesn't support large files.
# You may even need to reconfigure your kernel.  (This is all very
# operating system and site-dependent.)
#
# Perl may still be able to support large files, once you have
# such a process, enough quota, and such a (file) system.
# It is just that the test failed now.
#
EOM
    }
    if (@_) {
	plan(skip_all => "@_");
    }
}

$| = 1;

print "# checking whether we have sparse files...\n";

# Known have-nots.
if ($^O eq 'MSWin32' || $^O eq 'VMS') {
    plan(skip_all => "no sparse files in $^O");
}

# Known haves that have problems running this test
# (for example because they do not support sparse files, like UNICOS)
if ($^O eq 'unicos') {
    plan(skip_all => "no sparse files in $^O, unable to test large files");
}

# Then try heuristically to deduce whether we have sparse files.

# We'll start off by creating a one megabyte file which has
# only three "true" bytes.  If we have sparseness, we should
# consume less blocks than one megabyte (assuming nobody has
# one megabyte blocks...)

sysopen(BIG, $big1, O_WRONLY|O_CREAT|O_TRUNC) or
    die "sysopen $big1 failed: $!";
binmode BIG;
sysseek(BIG, 1_000_000, SEEK_SET) or
    die "sysseek $big1 failed: $!";
syswrite(BIG, "big") or
    die "syswrite $big1 failed: $!";
close(BIG) or
    die "close $big1 failed: $!";

my @s1 = stat($big1);

print "# s1 = @s1\n";

sysopen(BIG, $big2, O_WRONLY|O_CREAT|O_TRUNC) or
    die "sysopen $big2 failed: $!";
binmode BIG;
sysseek(BIG, 2_000_000, SEEK_SET) or
    die "sysseek $big2 failed: $!";
syswrite(BIG, "big") or
    die "syswrite $big2 failed: $!";
close(BIG) or
    die "close $big2 failed: $!";

my @s2 = stat($big2);

print "# s2 = @s2\n";

unless ($s1[7] == 1_000_003 && $s2[7] == 2_000_003 &&
	$s1[11] == $s2[11] && $s1[12] == $s2[12] &&
	$s1[12] > 0) {
    plan(skip_all => "no sparse files?");
}

print "# we seem to have sparse files...\n";

# By now we better be sure that we do have sparse files:
# if we are not, the following will hog 5 gigabytes of disk.  Ooops.
# This may fail by producing some signal; run in a subprocess first for safety

$ENV{LC_ALL} = "C";

my $perl = '../../perl';
unless (-x $perl) {
    plan(tests => 1);
    fail("can't find perl: expected $perl");
}
my $r = system $perl, '-I../lib', '-e', <<"EOF";
use Fcntl qw(/^O_/ /^SEEK_/);
sysopen \$big, q{$big0}, O_WRONLY|O_CREAT|O_TRUNC or die qq{sysopen $big0 $!};
sysseek \$big, 5_000_000_000, SEEK_SET or die qq{sysseek $big0 $!};
syswrite \$big, "big" or die qq{syswrite $big0 $!};
close \$big or die qq{close $big0: $!};
exit 0;
EOF


sysopen(BIG, $big0, O_WRONLY|O_CREAT|O_TRUNC) or
    die "sysopen $big0 failed: $!";
binmode BIG;
my $sysseek = sysseek(BIG, 5_000_000_000, SEEK_SET);
unless (! $r && defined $sysseek && $sysseek == 5_000_000_000) {
    $sysseek = 'undef' unless defined $sysseek;
    explain("seeking past 2GB failed: ",
	    $r ? 'signal '.($r & 0x7f) : "$! (sysseek returned $sysseek)");
}

# The syswrite will fail if there are are filesize limitations (process or fs).
my $syswrite = syswrite(BIG, "big");
print "# syswrite failed: $! (syswrite returned ",
      defined $syswrite ? $syswrite : 'undef', ")\n"
    unless defined $syswrite && $syswrite == 3;
my $close     = close BIG;
print "# close failed: $!\n" unless $close;
unless($syswrite && $close) {
    if ($! =~/too large/i) {
	explain("writing past 2GB failed: process limits?");
    } elsif ($! =~ /quota/i) {
	explain("filesystem quota limits?");
    } else {
	explain("error: $!");
    }
}

@s = stat($big0);

print "# @s\n";

unless ($s[7] == 5_000_000_003) {
    explain("kernel/fs not configured to use large files?");
}

sub offset ($$) {
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    my ($offset_will_be, $offset_want) = @_;
    my $offset_is = eval $offset_will_be;
    unless ($offset_is == $offset_want) {
        print "# bad offset $offset_is, want $offset_want\n";
	my ($offset_func) = ($offset_will_be =~ /^(\w+)/);
	if (unpack("L", pack("L", $offset_want)) == $offset_is) {
	    print "# 32-bit wraparound suspected in $offset_func() since\n";
	    print "# $offset_want cast into 32 bits equals $offset_is.\n";
	} elsif ($offset_want - unpack("L", pack("L", $offset_want)) - 1
	         == $offset_is) {
	    print "# 32-bit wraparound suspected in $offset_func() since\n";
	    printf "# %s - unpack('L', pack('L', %s)) - 1 equals %s.\n",
	        $offset_want,
	        $offset_want,
	        $offset_is;
        }
        fail($offset_will_be);
    } else {
	pass($offset_will_be);
    }
}

plan(tests => 17);

is($s[7], 5_000_000_003, 'exercises pp_stat');
is(-s $big0, 5_000_000_003, 'exercises pp_ftsize');

is(-e $big0, 1);
is(-f $big0, 1);

sysopen(BIG, $big0, O_RDONLY) or die "sysopen failed: $!";
binmode BIG;
offset('sysseek(BIG, 4_500_000_000, SEEK_SET)', 4_500_000_000);

offset('sysseek(BIG, 0, SEEK_CUR)', 4_500_000_000);

# If you get 205_032_705 from here it means that
# your tell() is returning 32-bit values since (I32)4_500_000_001
# is exactly 205_032_705.
offset('sysseek(BIG, 1, SEEK_CUR)', 4_500_000_001);

offset('sysseek(BIG, 0, SEEK_CUR)', 4_500_000_001);

offset('sysseek(BIG, -1, SEEK_CUR)', 4_500_000_000);

offset('sysseek(BIG, 0, SEEK_CUR)', 4_500_000_000);

offset('sysseek(BIG, -3, SEEK_END)', 5_000_000_000);

offset('sysseek(BIG, 0, SEEK_CUR)', 5_000_000_000);

my $big;

is(sysread(BIG, $big, 3), 3);

is($big, "big");

# 705_032_704 = (I32)5_000_000_000
# See that we don't have "big" in the 705_... spot:
# that would mean that we have a wraparound.
isnt(sysseek(BIG, 705_032_704, SEEK_SET), undef);

my $zero;

is(read(BIG, $zero, 3), 3);

is($zero, "\0\0\0");

explain() unless Test::Builder->new()->is_passing();

END {
    # unlink may fail if applied directly to a large file
    # be paranoid about leaving 5 gig files lying around
    open(BIG, '>', $big0); # truncate
    close(BIG);
}

# eof
