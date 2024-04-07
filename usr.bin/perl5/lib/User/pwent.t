#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

BEGIN {
    our $haspw;
    eval { my @n = getpwuid 0 };
    $haspw = 1 unless $@ && $@ =~ /unimplemented/;
    unless ($haspw) { print "1..0 # Skip: no getpwuid\n"; exit 0 }
    use Config;
    # VMS's pwd.h struct passwd conflicts with the one in vmsish.h
    $haspw = 0 unless ( $Config{'i_pwd'} eq 'define' || $^O eq 'VMS' );
    unless ($haspw) { print "1..0 # Skip: no pwd.h\n"; exit 0 }
}

BEGIN {
    our $uid = 0;
    # On VMS getpwuid(0) may return [$gid,0] UIC info (which may not exist).
    # It is better to use the $< uid for testing on VMS instead.
    if ( $^O eq 'VMS' ) { $uid = $< ; }
    if ( $^O eq 'cygwin' ) { $uid = 500 ; }
    our @pwent = getpwuid $uid; # This is the function getpwuid.
    unless (@pwent) { print "1..0 # Skip: no uid $uid\n"; exit 0 }
}

print "1..9\n";

use User::pwent;

print "ok 1\n";

my $pwent = getpwuid $uid; # This is the OO getpwuid.

my $uid_expect = $uid;
if ( $^O eq 'cygwin' ) {
    print "not " unless (   $pwent->uid == $uid_expect
                         || $pwent->uid == 500         );  # go figure
}
else {
    print "not " unless $pwent->uid    == $uid_expect ;
}
print "ok 2\n";

print "not " unless $pwent->name   eq $pwent[0];
print "ok 3\n";

if ($^O eq 'os390') {
    print "not "
	unless not defined $pwent->passwd &&
	       $pwent[1] eq '0'; # go figure
} else {
    print "not " unless $pwent->passwd eq $pwent[1];
}
print "ok 4\n";

print "not " unless $pwent->uid    == $pwent[2];
print "ok 5\n";

print "not " unless $pwent->gid    == $pwent[3];
print "ok 6\n";

# The quota and comment fields are unportable.

print "not " unless $pwent->gecos  eq $pwent[6];
print "ok 7\n";

print "not " unless $pwent->dir    eq $pwent[7];
print "ok 8\n";

print "not " unless $pwent->shell  eq $pwent[8];
print "ok 9\n";

# The expire field is unportable.

# Testing pretty much anything else is unportable:
# there maybe more than one username with uid 0;
# uid 0's home directory may be "/" or "/root' or something else,
# and so on.

