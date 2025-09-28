BEGIN { print "1..1\n"; }
use Carp;
my $x = "foo\x{666}"; $x =~ /foo\p{Alnum}/;
print "ok 1\n";
1;
