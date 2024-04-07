use strict;
use warnings;
use Test2::Tools::Tiny;
use Test2::Util qw/CAN_REALLY_FORK/;

BEGIN {
    skip_all "Set AUTHOR_TESTING to run this test" unless $ENV{AUTHOR_TESTING};
    skip_all "System cannot fork" unless CAN_REALLY_FORK;
    skip_all "known to fail on $]" if $] le "5.006002";
}

use IPC::Open3 qw/open3/;
use File::Temp qw/tempdir/;

my $tempdir = tempdir(CLEANUP => 1);

open(my $stdout, '>', "$tempdir/stdout") or die "Could not open: $!";
open(my $stderr, '>', "$tempdir/stderr") or die "Could not open: $!";

my $pid = open3(undef, ">&" . fileno($stdout), ">&" . fileno($stderr), $^X, '-Ilib', '-e', <<'EOT');
use Test2::IPC::Driver::Files;
use Test2::IPC;
use Test2::Tools::Tiny;
use Test2::API qw/test2_ipc/;
plan 1;
ok(1);

my $tmpdir = test2_ipc()->tempdir;
open(my $fh, '>', "$tmpdir/leftover") or die "Could not open file: $!";
print $fh "XXX\n";
close($fh) or die "Could not clone file";

print "TEMPDIR: $tmpdir\n";

exit 100;

EOT

waitpid($pid, 0);
my $exit = $?;

open($stdout, '<', "$tempdir/stdout") or die "Could not open: $!";
open($stderr, '<', "$tempdir/stderr") or die "Could not open: $!";

$stdout = join "" => <$stdout>;
$stderr = join "" => <$stderr>;

is(($exit >> 8), 255, "exited 255");
like($stderr, qr{^IPC Fatal Error: Leftover files in the directory \(.*/leftover\)!$}m, "Got expected error");
like($stdout, qr{^Bail out! IPC Fatal Error: Leftover files in the directory \(.*leftover\)!$}m, "Got a bail printed");

if(ok($stdout =~ m/^TEMPDIR: (.*)$/m, "Found temp dir")) {
    chomp(my $tmpdir = $1);
    if (-d $tmpdir) {
        note "Cleaning up temp dir\n";

        opendir(my $dh, $tmpdir) or diag "Could not open temp dir: $!";
        for my $file (readdir($dh)) {
            next if $file =~ m/^\./;
            unlink("$tmpdir/$file") or diag "Could not remove $tmpdir/$file: $!";
        }
        closedir($dh);
        rmdir($tmpdir) or diag "Could not remove temp dir: $!";
    }
}

done_testing;
