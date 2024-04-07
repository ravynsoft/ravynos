#! perl
use lib 't';
use rt_101033;

print "1..1\n";
my $s = <DATA>;
print "not " if !$s or $s !~ /^test/;
print "ok 1 # TODO RT #101033 + Switch #97440 ignores __DATA__\n";

__DATA__
test
