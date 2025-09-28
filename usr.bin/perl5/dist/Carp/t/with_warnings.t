BEGIN { print "1..1\n"; }

use Carp ();
use warnings ();
$SIG{__WARN__} = sub {};
eval { warnings::warn("syntax", "foo") };
print $@ eq "" ? "" : "not ", "ok 1\n";

1;
