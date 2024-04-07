# Test to perform udp protocol testing.

use strict;
use Config;

sub isWindowsVista {
   return unless $^O eq 'MSWin32' or $^O eq "cygwin";
   return unless eval { require Win32 };
   return unless defined &Win32::GetOSVersion();

   #is this Vista or later?
   my ($string, $major, $minor, $build, $id) = Win32::GetOSVersion();
   return $build >= 6;
}

use Test::More tests => 3;
BEGIN {use_ok('Net::Ping')};

SKIP: {
    skip "No udp echo port", 2 unless getservbyname('echo', 'udp');
    skip "udp ping blocked by Window's default settings", 2 if isWindowsVista();
    skip "No getprotobyname", 2 unless $Config{d_getpbyname};
    skip "Not allowed on $^O", 2 if $^O =~ /^(hpux|irix|aix|freebsd)$/;
    my $p = new Net::Ping "udp";
    # message_type can't be used
    eval {
        $p->message_type();
    };
    like($@, qr/message type only supported on 'icmp' protocol/, "message_type() API only concern 'icmp' protocol");
    is($p->ping("127.0.0.1"), 1);
}
