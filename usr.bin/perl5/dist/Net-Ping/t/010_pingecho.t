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

use Test::More tests => 2;
BEGIN {use_ok('Net::Ping')};

TODO: {
    local $TODO = "Not working on os390 smoker; may be a permissions problem"
      if $^O eq 'os390';
    my $result = pingecho("127.0.0.1");
    is($result, 1, "pingecho 127.0.0.1 works");
}
