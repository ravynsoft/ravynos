BEGIN { print "1..1\n"; }

BEGIN { $^W = 1; }
BEGIN { $SIG{__WARN__} = sub { die "WARNING: $_[0]" }; }

use warnings;   # this may load Carp
use Carp ();

my $badstr = do { no warnings "utf8"; "\x{ffff}" };
sub dd { Carp::longmess() }
dd($badstr);

print "ok 1\n";

1;
