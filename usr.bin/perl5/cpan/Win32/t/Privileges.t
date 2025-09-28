use strict;
use warnings;

use Test;
use Win32;
use Config;
use File::Temp;

plan tests => 7;

ok(ref(Win32::GetProcessPrivileges()) eq 'HASH');
ok(ref(Win32::GetProcessPrivileges(Win32::GetCurrentProcessId())) eq 'HASH');

# All Windows PIDs are divisible by 4. It's an undocumented implementation
# detail, but it means it's extremely unlikely that the PID below is valid.
ok(!Win32::GetProcessPrivileges(3423237));

my $whoami = `whoami /priv 2>&1`;
my $skip = ($? == -1 || $? >> 8) ? '"whoami" command is missing' : 0;

skip($skip, sub{
    my $privs = Win32::GetProcessPrivileges();

    while ($whoami =~ /^(Se\w+)/mg) {
        return 0 unless exists $privs->{$1};
    }

    return 1;
});

# there isn't really anything to test, we just want to make sure that the
# function doesn't segfault
Win32::IsDeveloperModeEnabled();
ok(1);

Win32::IsSymlinkCreationAllowed();
ok(1);

$skip = $^O ne 'MSWin32' ? 'MSWin32-only test' : 0;
$skip ||= !$Config{d_symlink} ? 'this perl doesn\'t have symlink()' : 0;

skip($skip, sub {
    my $tmpdir = File::Temp->newdir;
    my $dirname = $tmpdir->dirname;

    if (Win32::IsSymlinkCreationAllowed()) {
        # we expect success
        return symlink("foo", $tmpdir->dirname . "/new_symlink") == 1;
    }
    else {
        # we expect failure
        return symlink("foo", $tmpdir->dirname . "/new_symlink") == 0;
    }
});

