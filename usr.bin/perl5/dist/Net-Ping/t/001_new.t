use warnings;
use strict;
use Config;

BEGIN {
  unless (my $port = getservbyname('echo', 'tcp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
  unless ($Config{d_getpbyname}) {
    print "1..0 \# Skip: no getprotobyname\n";
    exit;
  }
}

use Test::More qw(no_plan);
BEGIN {use_ok('Net::Ping')};

# plain ol' constuctor call
my $p = Net::Ping->new();
isa_ok($p, "Net::Ping");

# call new from an instantiated object
my $p2 = $p->new();
isa_ok($p2, "Net::Ping");

# named args
my $p3 = Net::Ping->new({proto => 'tcp', ttl => 5});
isa_ok($p3, "Net::Ping");

# check for invalid proto
eval {
    $p = Net::Ping->new("thwackkk");
};
like($@, qr/Protocol for ping must be "icmp", "icmpv6", "udp", "tcp", "syn", "stream" or "external"/, "new() errors for invalid protocol");

# check for invalid timeout
eval {
    $p = Net::Ping->new("tcp", -1);
};
like($@, qr/Default timeout for ping must be greater than 0 seconds/, "new() errors for invalid timeout");

# check for invalid data sizes
eval {
    $p = Net::Ping->new("udp", 10, -1);
};
like($@, qr/Data for ping must be from/, "new() errors for invalid data size");

eval {
    $p = Net::Ping->new("udp", 10, 70000);
};
like($@, qr/Data for ping must be from/, "new() errors for invalid data size");

# force failures for udp


# force failures for tcp
SKIP: {
    # diag "Checking icmp";
    eval { $p = Net::Ping->new('icmp'); };
    skip "icmp ping requires root privileges.", 3
      if !Net::Ping::_isroot() or $^O eq 'MSWin32';
    if($> and $^O ne 'VMS' and $^O ne 'cygwin') {
        like($@, qr/icmp ping requires root privilege/, "Need root for icmp");
        skip "icmp tests require root", 2;
    } else {
        isa_ok($p, "Net::Ping");
    }

    # set IP TOS to "Minimum Delay"
    $p = Net::Ping->new("icmp", undef, undef, undef, 8);
    isa_ok($p, "Net::Ping");

    # This really shouldn't work.  Not sure who to blame.
    $p = Net::Ping->new("icmp", undef, undef, undef, "does this fail");
    isa_ok($p, "Net::Ping");
}

