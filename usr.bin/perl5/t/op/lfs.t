# NOTE: this file tests how large files (>2GB) work with perlio (or stdio).
# sysopen(), sysseek(), syswrite(), sysread() are tested in t/lib/syslfs.t.
# If you modify/add tests here, remember to update also ext/Fcntl/t/syslfs.t.

BEGIN {
	chdir 't' if -d 't';
	require './test.pl';
	set_up_inc('../lib');
    require Config;
	# Don't bother if there are no quad offsets.
	skip_all('no 64-bit file offsets')
		if $Config::Config{lseeksize} < 8;
}

use strict;

our @s;

my $big0 = tempfile();
my $big1 = tempfile();
my $big2 = tempfile();

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
	skip_all(@_);
    }
}

$| = 1;

print "# checking whether we have sparse files...\n";

# Known have-nots.
if ($^O eq 'MSWin32' || $^O eq 'VMS') {
    skip_all("no sparse files in $^O");
}

# Known haves that have problems running this test
# (for example because they do not support sparse files, like UNICOS)
if ($^O eq 'unicos') {
    skip_all("no sparse files in $^O, unable to test large files");
}

# Then try heuristically to deduce whether we have sparse files.

# Let's not depend on Fcntl or any other extension.

sub SEEK_SET () {0}
sub SEEK_CUR () {1}
sub SEEK_END () {2}

# We'll start off by creating a one megabyte file which has
# only three "true" bytes.  If we have sparseness, we should
# consume less blocks than one megabyte (assuming nobody has
# one megabyte blocks...)

open(BIG, ">$big1") or
    die "open $big1 failed: $!";
binmode(BIG) or
    die "binmode $big1 failed: $!";
seek(BIG, 1_000_000, SEEK_SET) or
    die "seek $big1 failed: $!";
print BIG "big" or
    die "print $big1 failed: $!";
close(BIG) or
    die "close $big1 failed: $!";

my @s1 = stat($big1);

print "# s1 = @s1\n";

open(BIG, ">$big2") or
    die "open $big2 failed: $!";
binmode(BIG) or
    die "binmode $big2 failed: $!";
seek(BIG, 2_000_000, SEEK_SET) or
    die "seek $big2 failed: $!";
print BIG "big" or
    die "print $big2 failed: $!";
close(BIG) or
    die "close $big2 failed: $!";

my @s2 = stat($big2);

print "# s2 = @s2\n";

unless ($s1[7] == 1_000_003 && $s2[7] == 2_000_003 &&
	$s1[11] == $s2[11] && $s1[12] == $s2[12] &&
	$s1[12] > 0) {
    skip_all("no sparse files?");
}

print "# we seem to have sparse files...\n";

# By now we better be sure that we do have sparse files:
# if we are not, the following will hog 5 gigabytes of disk.  Ooops.
# This may fail by producing some signal; run in a subprocess first for safety

$ENV{LC_ALL} = "C";

my $r = system '../perl', '-e', <<"EOF";
open my \$big, '>', q{$big0} or die qq{open $big0: $!};
seek \$big, 5_000_000_000, 0 or die qq{seek $big0: $!};
print \$big "big" or die qq{print $big0: $!};
close \$big or die qq{close $big0: $!};
exit 0;
EOF

open(BIG, ">$big0") or die "open failed: $!";
binmode BIG;
if ($r or not seek(BIG, 5_000_000_000, SEEK_SET)) {
    my $err = $r ? 'signal '.($r & 0x7f) : $!;
    explain("seeking past 2GB failed: $err");
}

# Either the print or (more likely, thanks to buffering) the close will
# fail if there are filesize limitations (process or fs).
my $print = print BIG "big";
print "# print failed: $!\n" unless $print;
my $close = close BIG;
print "# close failed: $!\n" unless $close;
unless ($print && $close) {
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
    local $::Level = $::Level + 1;
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

open(BIG, $big0) or die "open failed: $!";
binmode BIG;

isnt(seek(BIG, 4_500_000_000, SEEK_SET), undef);

offset('tell(BIG)', 4_500_000_000);

isnt(seek(BIG, 1, SEEK_CUR), undef);

# If you get 205_032_705 from here it means that
# your tell() is returning 32-bit values since (I32)4_500_000_001
# is exactly 205_032_705.
offset('tell(BIG)', 4_500_000_001);

isnt(seek(BIG, -1, SEEK_CUR), undef);

offset('tell(BIG)', 4_500_000_000);

isnt(seek(BIG, -3, SEEK_END), undef);

offset('tell(BIG)', 5_000_000_000);

my $big;

is(read(BIG, $big, 3), 3);

is($big, "big");

# 705_032_704 = (I32)5_000_000_000
# See that we don't have "big" in the 705_... spot:
# that would mean that we have a wraparound.
isnt(seek(BIG, 705_032_704, SEEK_SET), undef);

my $zero;

is(read(BIG, $zero, 3), 3);

is($zero, "\0\0\0");

explain() unless $::Tests_Are_Passing;

END {
    # unlink may fail if applied directly to a large file
    # be paranoid about leaving 5 gig files lying around
    open(BIG, ">$big0"); # truncate
    close(BIG);
}

# eof
