#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

eval {my @n = getgrgid 0};
if ($@ =~ /(The \w+ function is unimplemented)/) {
    skip_all "getgrgid unimplemented";
}

eval { require Config; import Config; };
my $reason;
if ($Config{'i_grp'} ne 'define') {
	$reason = '$Config{i_grp} not defined';
}
elsif (not -f "/etc/group" ) { # Play safe.
	$reason = 'no /etc/group file';
}

if (not defined $where) {	# Try NIS.
    foreach my $ypcat (qw(/usr/bin/ypcat /bin/ypcat /etc/ypcat)) {
        if (-x $ypcat &&
            open(GR, "$ypcat group 2>/dev/null |") &&
            defined(<GR>)) 
        {
            print "# `ypcat group` worked\n";

            # Check to make sure we are really using NIS.
            if( open(NSSW, "/etc/nsswitch.conf" ) ) {
                my($group) = grep /^\s*group:/, <NSSW>;

                # If there is no group line, assume it default to compat.
                if( !$group || $group !~ /(nis|compat)/ ) {
                    print "# Doesn't look like you're using NIS in ".
                          "/etc/nsswitch.conf\n";
                    last;
                }
            }
            $where = "NIS group - $ypcat";
            undef $reason;
            last;
        }
    }
}

if (not defined $where) {	# Try NetInfo.
    foreach my $nidump (qw(/usr/bin/nidump)) {
        if (-x $nidump &&
            open(GR, "$nidump group . 2>/dev/null |") &&
            defined(<GR>)) 
        {
            $where = "NetInfo group - $nidump";
            undef $reason;
            last;
        }
    }
}

if (not defined $where) {	# Try local.
    my $GR = "/etc/group";
    if (-f $GR && open(GR, $GR) && defined(<GR>)) {
        undef $reason;
        $where = "local $GR";
    }
}

if ($reason) {
    skip_all $reason;
}


# By now the GR filehandle should be open and full of juicy group entries.

plan tests => 3;

# Go through at most this many groups.
# (note that the first entry has been read away by now)
my $max = 25;

my $n   = 0;
my $tst = 1;
my %perfect;
my %seen;

print "# where $where\n";

ok( setgrent(), 'setgrent' ) || print "# $!\n";

while (<GR>) {
    chomp;
    # LIMIT -1 so that groups with no users do not fall off
    my @s = split /:/, $_, -1;
    my ($name_s,$passwd_s,$gid_s,$members_s) = @s;
    if (@s) {
	push @{ $seen{$name_s} }, $.;
    } else {
	warn "# Your $where line $. is empty.\n";
	next;
    }
    if ($n == $max) {
	local $/;
	my $junk = <GR>;
	last;
    }
    # In principle we could whine if @s != 4 but do we know enough
    # of group file formats everywhere?
    if (@s == 4) {
	$members_s =~ s/\s*,\s*/,/g;
	$members_s =~ s/\s+$//;
	$members_s =~ s/^\s+//;
	@n = getgrgid($gid_s);
	# 'nogroup' et al.
	next unless @n;
	my ($name,$passwd,$gid,$members) = @n;
	# Protect against one-to-many and many-to-one mappings.
	if ($name_s ne $name) {
	    @n = getgrnam($name_s);
	    ($name,$passwd,$gid,$members) = @n;
	    next if $name_s ne $name;
	}
	# NOTE: group names *CAN* contain whitespace.
	$members =~ s/\s+/,/g;
	# what about different orders of members?
	$perfect{$name_s}++
	    if $name    eq $name_s    and
# Do not compare passwords: think shadow passwords.
# Not that group passwords are used much but better not assume anything.
               $gid     eq $gid_s     and
               $members eq $members_s;
    }
    $n++;
}

endgrent();

print "# max = $max, n = $n, perfect = ", scalar keys %perfect, "\n";

if (keys %perfect == 0 && $n) {
    $max++;
    print <<EOEX;
#
# The failure of op/grent test is not necessarily serious.
# It may fail due to local group administration conventions.
# If you are for example using both NIS and local groups,
# test failure is possible.  Any distributed group scheme
# can cause such failures.
#
# What the grent test is doing is that it compares the $max first
# entries of $where
# with the results of getgrgid() and getgrnam() call.  If it finds no
# matches at all, it suspects something is wrong.
# 
EOEX

    fail();
    print "#\t (not necessarily serious: run t/op/grent.t by itself)\n";
} else {
    pass("getgrgid and getgrnam performed as expected");
}

# Test both the scalar and list contexts.

my @gr1;

setgrent();
for (1..$max) {
    my $gr = scalar getgrent();
    last unless defined $gr;
    push @gr1, $gr;
}
endgrent();

my @gr2;

setgrent();
for (1..$max) {
    my ($gr) = (getgrent());
    last unless defined $gr;
    push @gr2, $gr;
}
endgrent();

is("@gr1", "@gr2", "getgrent gave same results in scalar and list contexts");

close(GR);
