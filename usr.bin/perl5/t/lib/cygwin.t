#!perl

BEGIN {
    chdir 't' if -d 't';
    @INC = ('../lib');
    require './test.pl';
    skip_all('cygwin specific test') unless $^O eq 'cygwin';
}

plan(tests => 16);

is(Cygwin::winpid_to_pid(Cygwin::pid_to_winpid($$)), $$,
   "perl pid translates to itself");

my $parent = getppid;
SKIP: {
    skip "test not run from cygwin process", 1 if $parent <= 1;
    is(Cygwin::winpid_to_pid(Cygwin::pid_to_winpid($parent)), $parent,
       "parent pid translates to itself");
}

my $catpid = open my $cat, "|cat" or die "Couldn't cat: $!";
open my $ps, "ps|" or die "Couldn't do ps: $!";
my ($catwinpid) = map /^.\s+$catpid\s+\d+\s+\d+\s+(\d+)/, <$ps>;
close($ps);

is(Cygwin::winpid_to_pid($catwinpid), $catpid, "winpid to pid");
is(Cygwin::pid_to_winpid($catpid), $catwinpid, "pid to winpid");
close($cat);

is(Cygwin::win_to_posix_path("t\\lib"), "t/lib", "win to posix path: t/lib");
is(Cygwin::posix_to_win_path("t/lib"), "t\\lib", "posix to win path: t\\lib");

use Win32;
use Cwd;
my $pwd = getcwd();
chdir("/");
my $winpath = Win32::GetCwd();
is(Cygwin::posix_to_win_path("/", 1), $winpath, "posix to absolute win path");
chdir($pwd);
is(Cygwin::win_to_posix_path($winpath, 1), "/", "win to absolute posix path");

my $mount = join '', `/usr/bin/mount`;
$mount =~ m|on /usr/bin type .+ \((\w+)[,\)]|m;
my $binmode = $1 =~ /binmode|binary/;
is(Cygwin::is_binmount("/"),  $binmode ? 1 : '', "check / for binmount");

my $rootmnt = Cygwin::mount_flags("/");
ok($binmode ? ($rootmnt =~ /,(binmode|binary)/) : ($rootmnt =~ /,textmode/), "check / mount_flags");
is(Cygwin::mount_flags("/cygdrive") =~ /,cygdrive/,  1, "check cygdrive mount_flags");

# Cygdrive mount prefix
my @flags = split(/,/, Cygwin::mount_flags('/cygdrive'));
my $prefix = pop(@flags);
ok($prefix, "cygdrive mount prefix  = " . (($prefix) ? $prefix : '<none>'));
my $prefix2 = readlink "/proc/cygdrive";
unless ($prefix2) {
    # fallback to old Cygwin, the drive need not actually exist, so
    # this will always work (but might return the wrong prefix if the
    # user re-mounted C:\
    chomp($prefix2 = `cygpath C:`);
    $prefix2 = substr($prefix2, 0, -1-(length($prefix2)>2));
}
is($prefix, $prefix2, 'cygdrive mount prefix2 = ' . $prefix2);

my @mnttbl = Cygwin::mount_table();
ok(@mnttbl > 0, "non empty mount_table");
for $i (@mnttbl) {
  if ($i->[0] eq '/') {
    is($i->[2].",".$i->[3], $rootmnt, "same root mount flags");
    last;
  }
}

ok(Cwd->cwd(), "bug#38628 legacy");
