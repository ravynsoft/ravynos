#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

BEGIN {
    our $hasse;
    eval { my @n = getservbyname "echo", "tcp" };
    $hasse = 1 unless $@ && $@ =~ /unimplemented|unsupported/i;
    unless ($hasse) { print "1..0 # Skip: no getservbyname\n"; exit 0 }
    use Config;
    $hasse = 0 unless $Config{'i_netdb'} eq 'define';
    unless ($hasse) { print "1..0 # Skip: no netdb.h\n"; exit 0 }
}

BEGIN {
    our @servent = getservbyname "echo", "tcp"; # This is the function getservbyname.
    unless (@servent) { print "1..0 # Skip: no echo service\n"; exit 0 }
}

print "1..3\n";

use Net::servent;

print "ok 1\n";

my $servent = getservbyname "echo", "tcp"; # This is the OO getservbyname.

print "not " unless $servent->name   eq $servent[0];
print "ok 2\n";

print "not " unless $servent->port  == $servent[2];
print "ok 3\n";

# Testing pretty much anything else is unportable.

