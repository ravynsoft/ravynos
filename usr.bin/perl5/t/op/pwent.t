#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

eval {my @n = getpwuid 0; setpwent()};
skip_all($1) if $@ && $@ =~ /(The \w+ function is unimplemented)/;

eval { require Config; };

sub try_prog {
    my ($where, $args, @pathnames) = @_;
    foreach my $prog (@pathnames) {
	next unless -x $prog;
	next unless open PW, '-|', "$prog $args 2>/dev/null";
	next unless defined <PW>;
	return $where;
    }
    return;
}

# Try NIS.
my $where = try_prog('NIS passwd', 'passwd',
		     qw(/usr/bin/ypcat /bin/ypcat /etc/ypcat));

# Try NetInfo.
$where //= try_prog('NetInfo passwd', 'passwd .', '/usr/bin/nidump');

# Try NIS+.
$where //= try_prog('NIS+', 'passwd.org_dir', '/bin/niscat');

# Try dscl
DSCL: {
my @dscl = qw(/usr/bin/dscl);
if (!defined $where && $Config::Config{useperlio} && grep { -x } @dscl) {
    eval { require PerlIO::scalar; }; # Beware miniperl.
    if ($@) {
        print "# No PerlIO::scalar, will not try dscl\n";
        last DSCL;
    }
    # Map dscl items to passwd fields, and provide support for
    # mucking with the dscl output if we need to (and we do).
    my %want = do {
	my $inx = 0;
	map {$_ => {inx => $inx++, mung => sub {$_[0]}}}
	    qw{RecordName Password UniqueID PrimaryGroupID
	       RealName NFSHomeDirectory UserShell};
    };

    # The RecordName for a /User record is the username. In some
    # cases there are synonyms (e.g. _www and www), in which case we
    # get a blank-delimited list. We prefer the first entry in the
    # list because getpwnam() does.
    $want{RecordName}{mung} = sub {(split '\s+', $_[0], 2)[0]};

    # The UniqueID and PrimaryGroupID for a /User record are the
    # user ID and the primary group ID respectively. In cases where
    # the high bit is set, 'dscl' returns a negative number, whereas
    # getpwnam() returns its twos complement. This mungs the dscl
    # output to agree with what getpwnam() produces. Interestingly
    # enough, getpwuid(-2) returns the right record ('nobody'), even
    # though it returns the uid as 4294967294. If you track uid_t
    # on an i386, you find it is an unsigned int, which makes the
    # unsigned version the right one; but both /etc/passwd and
    # /etc/master.passwd contain negative numbers.
    $want{UniqueID}{mung} = $want{PrimaryGroupID}{mung} = sub {
	unpack 'L', pack 'l', $_[0]};

    foreach my $dscl (@dscl) {
	next unless -x $dscl;
	next unless open my $fh, '-|', "$dscl . -readall /Users @{[keys %want]} 2>/dev/null";
	my @lines;
	my @rec;
	while (<$fh>) {
	    chomp;
	    if ($_ eq '-') {
		if (@rec) {
		    # Some records do not have all items. In particular,
		    # the macports user has no real name. Here it's an undef,
		    # in the password file it becomes an empty string.
		    no warnings 'uninitialized';
		    push @lines, join (':', @rec) . "\n";
		    @rec = ();
		}
		next;
	    }
	    my ($name, $value) = split ':\s+', $_, 2;
	    unless (defined $value) {
		s/:$//;
		$name = $_;
		$value = <$fh>;
		chomp $value;
		$value =~ s/^\s+//;
	    }
	    if (defined (my $info = $want{$name})) {
		$rec[$info->{inx}] = $info->{mung}->($value);
	    }
	}
	if (@rec) {
        # see above
        no warnings 'uninitialized';
	    push @lines, join (':', @rec) . "\n";
	}
	my $data = join '', @lines;
	if (open PW, '<', \$data) { # Needs PerlIO::scalar.
	    $where = "dscl . -readall /Users";
	    last;
	}
    }
}
} # DSCL:

if (not defined $where) {
    # Try local.
    my $no_i_pwd = !$Config::Config{i_pwd} && '$Config{i_pwd} undefined';

    my $PW = "/etc/passwd";
    if (!-f $PW) {
	skip_all($no_i_pwd) if $no_i_pwd;
	skip_all("no $PW file");
    } elsif (open PW, '<', $PW) {
	if(defined <PW>) {
	    $where = $PW;
	} else {
	    skip_all($no_i_pwd) if $no_i_pwd;
	    die "\$Config{i_pwd} is defined, $PW exists but has no entries, all other approaches failed, giving up";
	}
    } else {
	die "Can't open $PW: $!";
    }
}

# By now the PW filehandle should be open and full of juicy password entries.

plan(tests => 2);

# Go through at most this many users.
# (note that the first entry has been read away by now)
my $max = 25;

my $n = 0;
my %perfect;
my %seen;

print "# where $where\n";

setpwent();

while (<PW>) {
    chomp;
    # LIMIT -1 so that users with empty shells don't fall off
    my @s = split /:/, $_, -1;
    my ($name_s, $passwd_s, $uid_s, $gid_s, $gcos_s, $home_s, $shell_s);
    (my $v) = $Config::Config{osvers} =~ /^(\d+)/;
    if ($^O eq 'darwin' && $v < 9) {
       ($name_s, $passwd_s, $uid_s, $gid_s, $gcos_s, $home_s, $shell_s) = @s[0,1,2,3,7,8,9];
    } else {
       ($name_s, $passwd_s, $uid_s, $gid_s, $gcos_s, $home_s, $shell_s) = @s;
    }
    next if /^\+/; # ignore NIS includes
    if (@s) {
	push @{ $seen{$name_s} }, $.;
    } else {
	warn "# Your $where line $. is empty.\n";
	next;
    }
    if ($n == $max) {
	local $/;
	my $junk = <PW>;
	last;
    }
    # In principle we could whine if @s != 7 but do we know enough
    # of passwd file formats everywhere?
    if (@s == 7 || ($^O eq 'darwin' && @s == 10)) {
	my @n = getpwuid($uid_s);
	# 'nobody' et al.
	next unless @n;
	my ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$home,$shell) = @n;
	# Protect against one-to-many and many-to-one mappings.
	if ($name_s ne $name) {
	    @n = getpwnam($name_s);
	    ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$home,$shell) = @n;
	    next if $name_s ne $name;
	}
	$perfect{$name_s}++
	    if $name    eq $name_s    and
               $uid     eq $uid_s     and
# Do not compare passwords: think shadow passwords.
               $gid     eq $gid_s     and
               $gcos    eq $gcos_s    and
               $home    eq $home_s    and
               $shell   eq $shell_s;
    }
    $n++;
}

endpwent();

print "# max = $max, n = $n, perfect = ", scalar keys %perfect, "\n";

SKIP: {
    skip("Found no password entries", 1) unless $n;

    if (keys %perfect == 0) {
	$max++;
	print <<EOEX;
#
# The failure of op/pwent test is not necessarily serious.
# It may fail due to local password administration conventions.
# If you are for example using both NIS and local passwords,
# test failure is possible.  Any distributed password scheme
# can cause such failures.
#
# What the pwent test is doing is that it compares the $max first
# entries of $where
# with the results of getpwuid() and getpwnam() call.  If it finds no
# matches at all, it suspects something is wrong.
# 
EOEX
    }

    cmp_ok(keys %perfect, '>', 0, "pwent test satisfactory")
	or note("(not necessarily serious: run t/op/pwent.t by itself)");
}

# Test both the scalar and list contexts.

my @pw1;

setpwent();
for (1..$max) {
    my $pw = scalar getpwent();
    last unless defined $pw;
    push @pw1, $pw;
}
endpwent();

my @pw2;

setpwent();
for (1..$max) {
    my ($pw) = (getpwent());
    last unless defined $pw;
    push @pw2, $pw;
}
endpwent();

is("@pw1", "@pw2",
    "getpwent() produced identical results in list and scalar contexts");

close(PW);
