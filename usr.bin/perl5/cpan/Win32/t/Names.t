use strict;
BEGIN {
    eval "use Test::More";
    return unless $@;
    print "1..0 # Skip: Test requires Test::More module\n";
    exit 0;
}
use Win32;

my $tests = 16;
$tests += 2 if Win32::IsWinNT();

plan tests => $tests;

# test Win32::DomainName()
if (Win32::IsWinNT()) {
    my $domain = eval { Win32::DomainName() };
    SKIP: {
	skip('The Workstation service has not been started', 2) if (Win32::GetLastError() == 2138);
	is( $@, '', "Win32::DomainName()" );
	like( $domain, '/^[a-zA-Z0-9!@#$%^&()_\'{}.~-]+$/', "  - checking returned domain" );
    }
}

# test Win32::GetArchName()
$ENV{PROCESSOR_ARCHITECTURE} ||= "unknown";
my $archname = eval { Win32::GetArchName() };
is( $@, '', "Win32::GetArchName()" );
cmp_ok( length($archname), '>=', 3, "  - checking returned architecture name" );

# test Win32::GetChipArch()
my $chiparch = eval { Win32::GetChipArch() };
is( $@, '', "Win32::GetChipArch()" );
like( $chiparch, '/^(0|5|6|9|12)$/', " - checking returned chip arch" );

# test Win32::GetChipName()
my $chipname = eval { Win32::GetChipName() };
is( $@, '', "Win32::GetChipName()" );
like( $chipname, '/^(0|386|486|586|2200|8664)$/', " - checking returned chip name");

# test Win32::GetOSName()
#  - scalar context
my $osname = eval { Win32::GetOSName() };
is( $@, '', "Win32::GetOSName() in scalar context" );
cmp_ok( length($osname), '>', 3, "  - checking returned OS name" );

#  - list context
my ($osname2, $desc) = eval { Win32::GetOSName() };
is( $@, '', "Win32::GetOSName() in list context" );
cmp_ok( length($osname2), '>', 3, "  - checking returned OS name" );
ok( defined($desc), "  - checking returned description" );
is( $osname2, $osname, "  - checking that OS name is the same in both calls" );

# test Win32::LoginName()
my $login = eval { Win32::LoginName() };
is( $@, '', "Win32::LoginName()" );
cmp_ok( length($login), '>', 0, "  - checking returned login name" );

# test Win32::NodeName()
my $nodename = eval { Win32::NodeName() };
is( $@, '', "Win32::NodeName()" );
cmp_ok( length($nodename), '>', 0, "  - checking returned node name" );
