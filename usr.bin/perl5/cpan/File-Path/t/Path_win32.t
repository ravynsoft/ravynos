use strict;
use Test::More;
use lib './t';
use FilePathTest qw(
    create_3_level_subdirs
    cleanup_3_level_subdirs
);
use File::Path;
use Cwd;
use File::Spec::Functions;
use Carp;

plan skip_all  => 'not win32' unless $^O eq 'MSWin32';
my ($ignore, $major, $minor, $build, $id) = Win32::GetOSVersion();
plan skip_all  => "WinXP or later"
     unless $id >= 2 && ($major > 5 || $major == 5 && $minor >= 1);
plan tests     => 9;

my $tmp_base = catdir(
    curdir(),
    sprintf( 'test-%x-%x-%x', time, $$, rand(99999) ),
);

my $UNC_path = catdir(getcwd(), $tmp_base, 'uncdir');
#dont compute a SMB path with $ENV{COMPUTERNAME}, since SMB may be turned off
#firewalled, disabled, blocked, or no NICs are on and there the PC has no
#working TCPIP stack, \\?\ will always work
$UNC_path = '\\\\?\\'.$UNC_path;

is(mkpath($UNC_path), 2, 'mkpath on Win32 UNC path returns made 2 dir - base and uncdir');

ok(-d $UNC_path, 'mkpath on Win32 UNC path made dir');

my $removed = rmtree($UNC_path);

cmp_ok($removed, '>', 0, "removed $removed entries from $UNC_path");

{
    my ($least_deep, $next_deepest, $deepest) =
        create_3_level_subdirs( qw| IsVFFJfJ03Rk jD7ToWQFmcjm hMZR6S1qNSf5 | );
    my (@created, $error);
    my $user = join('_' => 'foobar', $$);
    {
        my $warn;
        $SIG{__WARN__} = sub { $warn = shift };

        @created = mkpath($deepest, { mode => 0711, user => $user, error => \$error });
        like($warn,
            qr/Option\(s\) implausible on Win32 passed to mkpath\(\) or make_path\(\)/,
            'make_path with final hashref warned due to options implausible on Win32'
        );
        TODO: {
            local $TODO = "Notwithstanding the Win32-implausible 'user', mkpath will actually create subdirectories; should it?";
            is(scalar(@created), 0, "No subdirectories created");
        }
        is(scalar(@created), 3, "3 subdirectories created");
        is(scalar(@$error), 0, "no error condition" );
    }

    cleanup_3_level_subdirs($least_deep);
}

