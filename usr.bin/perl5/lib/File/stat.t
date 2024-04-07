#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use strict;
use warnings;
use Test::More;
use Config qw( %Config );
use File::Temp qw( tempfile tempdir );

use File::stat;

my (undef, $file) = tempfile(UNLINK => 1);

{
    my @stat = CORE::stat $file;
    my $stat = File::stat::stat($file);
    isa_ok($stat, 'File::stat', 'should build a stat object');
    is_deeply($stat, \@stat, '... and matches the builtin');

    my $i = 0;
    foreach ([dev => 'device number'],
             [ino => 'inode number'],
             [mode => 'file mode'],
             [nlink => 'number of links'],
             [uid => 'owner uid'],
             [gid => 'group id'],
             [rdev => 'device identifier'],
             [size => 'file size'],
             [atime => 'last access time'],
             [mtime => 'last modify time'],
             [ctime => 'change time'],
             [blksize => 'IO block size'],
             [blocks => 'number of blocks']) {
        my ($meth, $desc) = @$_;
        # On OS/2 (fake) ino is not constant, it is incremented each time
    SKIP: {
            skip('inode number is not constant on OS/2', 1)
                if $i == 1 && $^O eq 'os2';
            is($stat->$meth, $stat[$i], "$desc in position $i");
        }
        ++$i;
    }

    my $stat2 = stat $file;
    isa_ok($stat2, 'File::stat',
           'File::stat exports stat, overriding the builtin');
    is_deeply($stat2, $stat, '... and matches the direct call');
}

sub test_X_ops {
    my ($file, $desc_tail, $skip) = @_;
    my @stat = CORE::stat $file;
    my $stat = File::stat::stat($file);
    my $lstat = File::stat::lstat($file);
    isa_ok($stat, 'File::stat', 'should build a stat object');

    for my $op (split //, "rwxoRWXOezsfdlpSbcugkMCA") {
        if ($skip && $op =~ $skip) {
            note("Not testing -A $desc_tail");
            next;
        }
        my $stat = $op eq 'l' ? $lstat : $stat;
        for my $access ('', 'use filetest "access";') {
            my ($warnings, $awarn, $vwarn, $rv);
            my $desc = $access
                ? "for -$op under use filetest 'access' $desc_tail"
                    : "for -$op $desc_tail";
            {
                local $SIG{__WARN__} = sub {
                    my $w = shift;
                    if ($w =~ /^File::stat ignores VMS ACLs/) {
                        ++$vwarn;
                    } elsif ($w =~ /^File::stat ignores use filetest 'access'/) {
                        ++$awarn;
                    } else {
                        $warnings .= $w;
                    }
                };
                $rv = eval "$access; -$op \$stat";
            }
            is($@, '', "Overload succeeds $desc");

            SKIP : {
                if ($^O eq "haiku" && $op =~ /A/) {
                    # atime is not stored on Haiku BFS
                    # and stat always returns local time instead
                    skip "testing -A $desc_tail on Haiku", 1;
                }

                if ($^O eq "VMS" && $op =~ /[rwxRWX]/) {
                    is($vwarn, 1, "warning about VMS ACLs $desc");
                } else {
                    is($rv, eval "-$op \$file", "correct overload $desc")
                        unless $access;
                    is($vwarn, undef, "no warnings about VMS ACLs $desc");
                }
            }

            # 111640 - File::stat bogus index check in overload
            if ($access && $op =~ /[rwxRXW]/) {
                # these should all warn with filetest access
                is($awarn, 1,
                   "produced the right warning $desc");
            } else {
                # -d and others shouldn't warn
                is($awarn, undef, "should be no warning $desc")
            }

            is($warnings, undef, "no other warnings seen $desc");
        }
    }
}

foreach ([file => $file],
         [dir => tempdir(CLEANUP => 1)]) {
    my ($what, $pathname) = @$_;
    test_X_ops($pathname, "for $what $pathname");

    my $orig_mode = (CORE::stat $pathname)[2];

    my $mode = 01000;
    while ($mode) {
        $mode >>= 1;
        my $mode_oct = sprintf "0%03o", $mode;
        chmod $mode, $pathname or die "Can't chmod $mode_oct $pathname: $!";
        test_X_ops($pathname, "for $what with mode=$mode_oct");
    }
    chmod $orig_mode, $pathname
        or die "Can't restore permissions on $pathname to ", sprintf("%#o", $orig_mode);
}

SKIP: {
    -e $^X && -x $^X or skip "$^X is not present and executable", 4;
    $^O eq "VMS" and skip "File::stat ignores VMS ACLs", 4;

    # Other tests running in parallel mean that $^X is read, updating its atime
    test_X_ops($^X, "for $^X", qr/A/);
}

# open early so atime is consistent with the name based call
local *STAT;
my $canopen = open(STAT, '<', $file);

my $stat = File::stat::stat($file);
isa_ok($stat, 'File::stat', 'should build a stat object');

for (split //, "tTB") {
    eval "-$_ \$stat";
    like( $@, qr/\Q-$_ is not implemented/, "-$_ overload fails" );
}

SKIP: {
	skip("Could not open file: $!", 2) unless $canopen;
	isa_ok(File::stat::stat('STAT'), 'File::stat',
	       '... should be able to find filehandle');

	package foo;
	local *STAT = *main::STAT;
	my $stat2 = File::stat::stat('STAT');
	main::isa_ok($stat2, 'File::stat',
		     '... and filehandle in another package');
	close STAT;

#	VOS open() updates atime; ignore this error (posix-975).
	my $stat3 = $stat2;
	if ($^O eq 'vos') {
		$$stat3[8] = $$stat[8];
	}

	main::skip("Win32: different stat-info on filehandle", 1) if $^O eq 'MSWin32';

	main::skip("OS/2: inode number is not constant on os/2", 1) if $^O eq 'os2';

	main::is_deeply($stat, $stat3, '... and must match normal stat');
}

SKIP:
{   # RT #111638
    skip "We can't check for FIFOs", 2 unless defined &Fcntl::S_ISFIFO;
    skip "No pipes", 2 unless defined $Config{d_pipe};
    pipe my ($rh, $wh)
      or skip "Couldn't create a pipe: $!", 2;
    skip "Built-in -p doesn't detect a pipe", 2 unless -p $rh;

    my $pstat = File::stat::stat($rh);
    ok(!-p($stat), "-p should be false on a file");
    ok(-p($pstat), "check -p detects a pipe");
}

# Testing pretty much anything else is unportable.

done_testing;

# ex: set ts=8 sts=4 sw=4 et:
