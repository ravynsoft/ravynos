use strict;
use warnings;

print "1..1\n";

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

# The :gzip tags are tested in external.t.

eval "use IO::Zlib qw(foo bar)";

ok(1, $@ =~ /^IO::Zlib::import: 'foo bar' is illegal /);
