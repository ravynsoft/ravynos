#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

BEGIN {
    our $hasne;
    eval { my @n = getnetbyname "loopback" };
    $hasne = 1 unless $@ && $@ =~ /unimplemented|unsupported/i;
    unless ($hasne) { print "1..0 # Skip: no getnetbyname\n"; exit 0 }
    use Config;
    $hasne = 0 unless $Config{'i_netdb'} eq 'define';
    unless ($hasne) { print "1..0 # Skip: no netdb.h\n"; exit 0 }
}

BEGIN {
    our @netent = getnetbyname "loopback"; # This is the function getnetbyname.
    unless (@netent) { print "1..0 # Skip: no loopback net\n"; exit 0 }
}

print "1..2\n";

use Net::netent;

print "ok 1\n";

my $netent = getnetbyname "loopback"; # This is the OO getnetbyname.

print "not " unless $netent->name   eq $netent[0];
print "ok 2\n";

# Testing pretty much anything else is unportable;
# e.g. the canonical name of the "loopback" net may be "loop".

