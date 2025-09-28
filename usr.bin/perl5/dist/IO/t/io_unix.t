#!./perl

use Config;
use IO::Socket;

BEGIN {
    my $reason;
    my $can_fork = $Config{d_fork} ||
		    (($^O eq 'MSWin32' || $^O eq 'NetWare') and
		     $Config{useithreads} and
		     $Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/
		    );

    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bSocket\b/) {
	$reason = 'Socket extension unavailable';
    }
    elsif ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bIO\b/) {
	$reason = 'IO extension unavailable';
    }
    elsif ($^O eq 'os2') {
	eval {IO::Socket::pack_sockaddr_un('/foo/bar') || 1}
	  or $@ !~ /not implemented/ or
	    $reason = 'compiled without TCP/IP stack v4';
    }
    elsif ($^O =~ m/^(?:qnx|nto|vos)$/ ) {
	$reason = "UNIX domain sockets not implemented on $^O";
    }
    elsif (! $can_fork) {
	$reason = 'no fork';
    }
    elsif ($^O eq 'MSWin32') {
      if ($ENV{CONTINUOUS_INTEGRATION}) {
         $reason = 'Skipping on Windows CI, see gh17575 and gh17429';
      } else {
       $reason = "AF_UNIX unavailable or disabled on this platform"
         unless eval { socket(my $sock, PF_UNIX, SOCK_STREAM, 0) };
      }
    }

    if ($reason) {
	print "1..0 # Skip: $reason\n";
	exit 0;
    }
}

my $PATH = "sock-$$";

if ($^O eq 'os2') {	# Can't create sockets with relative path...
  require Cwd;
  my $d = Cwd::cwd();
  $d =~ s/^[a-z]://i;
  $PATH = "$d/$PATH";
}

# Test if we can create the file within the tmp directory
if (-e $PATH or not open(TEST, '>', $PATH) and $^O ne 'os2') {
    print "1..0 # Skip: cannot open '$PATH' for write\n";
    exit 0;
}
close(TEST);
unlink($PATH) or $^O eq 'os2' or die "Can't unlink $PATH: $!";

# Start testing
$| = 1;
print "1..5\n";

my $listen = IO::Socket::UNIX->new(Local => $PATH, Listen => 0);

# Sometimes UNIX filesystems are mounted for security reasons
# with "nodev" option which spells out "no" for creating UNIX
# local sockets.  Therefore we will retry with a File::Temp
# generated filename from a temp directory.
unless (defined $listen) {
    eval { require File::Temp };
    unless ($@) {
	File::Temp->import( 'mktemp' );
	for my $TMPDIR ($ENV{TMPDIR}, "/tmp") {
	    if (defined $TMPDIR && -d $TMPDIR && -w $TMPDIR) {
		$PATH = mktemp("$TMPDIR/sXXXXXXXX");
		last if $listen = IO::Socket::UNIX->new(Local => $PATH,
							Listen => 0);
	    }
	}
    }
    defined $listen or die "$PATH: $!";
}
print "ok 1\n";

if (my $pid = fork()) {

    my $sock = $listen->accept();

    if (defined $sock) {
	print "ok 2\n";

	print $sock->getline();

	print $sock "ok 4\n";

	$sock->close;

	waitpid($pid,0);
	unlink($PATH) || $^O eq 'os2' || warn "Can't unlink $PATH: $!";

	print "ok 5\n";
    } else {
	print "# accept() failed: $!\n";
	for (2..5) {
	    print "not ok $_ # accept failed\n";
	}
    }
} elsif(defined $pid) {

    my $sock = IO::Socket::UNIX->new(Peer => $PATH) or die "$!";

    print $sock "ok 3\n";

    print $sock->getline();

    $sock->close;

    exit;
} else {
 die;
}
