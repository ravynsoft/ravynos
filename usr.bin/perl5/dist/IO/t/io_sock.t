#!./perl -w

use Config;

BEGIN {
    my $can_fork = $Config{d_fork} ||
		    (($^O eq 'MSWin32' || $^O eq 'NetWare') and
		     $Config{useithreads} and 
		     $Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/
		    );
    my $reason;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bSocket\b/) {
	$reason = 'Socket extension unavailable';
    }
    elsif ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bIO\b/) {
	$reason = 'IO extension unavailable';
    }
    elsif (!$can_fork) {
        $reason = 'no fork';
    }
    $reason = q[Test out of sequence on windows] if $^O eq 'MSWin32' && $ENV{CONTINUOUS_INTEGRATION};
    if ($reason) {
	print "1..0 # Skip: $reason\n";
	exit 0;
    }
}

my $has_perlio = PerlIO::Layer->find( 'perlio' );

$| = 1;
print "1..26\n";

eval {
    $SIG{ALRM} = sub { die; };
    alarm 120;
};

use IO::Socket;

my $listen = IO::Socket::INET->new(LocalAddr => 'localhost',
				Listen => 2,
				Proto => 'tcp',
				# some systems seem to need as much as 10,
				# so be generous with the timeout
				Timeout => 15,
			       ) or die "$!";

print "ok 1\n";

# Check if can fork with dynamic extensions (bug in CRT):
if ($^O eq 'os2' and
    system "$^X -I../lib -MOpcode -e 'defined fork or die'  > /dev/null 2>&1") {
    print "ok $_ # skipped: broken fork\n" for 2..5;
    exit 0;
}

my $port = $listen->sockport;

if(my $pid = fork()) {

    my $sock = $listen->accept() or die "accept failed: $!";
    print "ok 2\n";

    $sock->autoflush(1);
    print $sock->getline();

    print $sock "ok 4\n";

    $sock->close;

    waitpid($pid,0);

    print "ok 5\n";

} elsif(defined $pid) {

    my $sock = IO::Socket::INET->new(PeerPort => $port,
				  Proto => 'tcp',
				  PeerAddr => 'localhost'
				 )
         || IO::Socket::INET->new(PeerPort => $port,
				  Proto => 'tcp',
				  PeerAddr => '127.0.0.1'
				 )
	or die "$! (maybe your system does not have a localhost at all, 'localhost' or 127.0.0.1)";

    $sock->autoflush(1);

    print $sock "ok 3\n";

    print $sock->getline();

    $sock->close;

    exit;
} else {
 die;
}

# Test various other ways to create INET sockets that should
# also work.
$listen = IO::Socket::INET->new(LocalAddr => 'localhost', Listen => '', Timeout => 15) or die "$!";
$port = $listen->sockport;

if(my $pid = fork()) {
  SERVER_LOOP:
    while (1) {
       last SERVER_LOOP unless my $sock = $listen->accept;
       while (<$sock>) {
           last SERVER_LOOP if /^quit/;
           last if /^done/;
           print;
       }
       $sock = undef;
    }
    $listen->close;
} elsif (defined $pid) {
    # child, try various ways to connect
    my $sock = IO::Socket::INET->new("localhost:$port")
         || IO::Socket::INET->new("127.0.0.1:$port");
    if ($sock) {
	print "not " unless $sock->connected;
	print "ok 6\n";
       $sock->print("ok 7\n");
       sleep(1);
       print "ok 8\n";
       $sock->print("ok 9\n");
       $sock->print("done\n");
       $sock->close;
    }
    else {
	print "# $IO::Socket::errstr\n";
	print "not ok 6\n";
	print "not ok 7\n";
	print "not ok 8\n";
	print "not ok 9\n";
    }

    # some machines seem to suffer from a race condition here
    sleep(2);

    $sock = IO::Socket::INET->new("127.0.0.1:$port");
    if ($sock) {
       $sock->print("ok 10\n");
       $sock->print("done\n");
       $sock->close;
    }
    else {
	print "# $IO::Socket::errstr\n";
	print "not ok 10\n";
    }

    # some machines seem to suffer from a race condition here
    sleep(1);

    $sock = IO::Socket->new(Domain => AF_INET,
                            PeerAddr => "localhost:$port")
         || IO::Socket->new(Domain => AF_INET,
                            PeerAddr => "127.0.0.1:$port");
    if ($sock) {
       $sock->print("ok 11\n");
       $sock->print("quit\n");
    } else {
       print "not ok 11\n";
    }
    $sock = undef;
    sleep(1);
    exit;
} else {
    die;
}

# Then test UDP sockets
my $server = IO::Socket->new(Domain => AF_INET,
                          Proto  => 'udp',
                          LocalAddr => 'localhost')
       || IO::Socket->new(Domain => AF_INET,
                          Proto  => 'udp',
                          LocalAddr => '127.0.0.1');
$port = $server->sockport;

if (my $pid = fork()) {
    my $buf;
    $server->recv($buf, 100);
    print $buf;
} elsif (defined($pid)) {
    #child
    my $sock = IO::Socket::INET->new(Proto => 'udp',
                                  PeerAddr => "localhost:$port")
         || IO::Socket::INET->new(Proto => 'udp',
                                  PeerAddr => "127.0.0.1:$port");
    $sock->send("ok 12\n");
    sleep(1);
    $sock->send("ok 12\n");  # send another one to be sure
    exit;
} else {
    die;
}

print "not " unless $server->blocking;
print "ok 13\n";

if ( $^O eq 'qnx' ) {
  # QNX4 library bug: Can set non-blocking on socket, but
  # cannot return that status.
  print "ok 14 # skipped on QNX4\n";
} else {
  $server->blocking(0);
  print "not " if $server->blocking;
  print "ok 14\n";
}

### TEST 15
### Set up some data to be transferred between the server and
### the client. We'll use own source code ...
#
my @data;
if( !open( SRC, '<', $0)) {
    print "not ok 15 - $!\n";
} else {
    @data = <SRC>;
    close(SRC);
    print "ok 15\n";
}

### TEST 16
### Start the server
#
$listen = IO::Socket::INET->new(LocalAddr => 'localhost', Listen => 2, Proto => 'tcp', Timeout => 15) ||
    print "not ";
print "ok 16\n";
die if( !defined( $listen));
my $serverport = $listen->sockport;
my $server_pid = fork();
if( $server_pid) {

    ### TEST 17 Client/Server establishment
    #
    print "ok 17\n";

    ### TEST 18
    ### Get data from the server using a single stream
    #
    my $sock = IO::Socket::INET->new("localhost:$serverport")
         || IO::Socket::INET->new("127.0.0.1:$serverport");

    if ($sock) {
	$sock->print("send\n");

	my @array = ();
	while( <$sock>) {
	    push( @array, $_);
	}

	$sock->print("done\n");
	$sock->close;

	print "not " if( @array != @data);
    } else {
	print "not ";
    }
    print "ok 18\n";

    ### TEST 21
    ### Get data from the server using a stream, which is
    ### interrupted by eof calls.
    ### On perl-5.7.0@7673 this failed in a SOCKS environment, because eof
    ### did an getc followed by an ungetc in order to check for the streams
    ### end. getc(3) got replaced by the SOCKS function, which ended up in
    ### a recv(2) call on the socket, while ungetc(3) put back a character
    ### to an IO buffer, which never again was read.
    #
    ### TESTS 19,20,21,22
    ### Try to ping-pong some Unicode.
    #
    $sock = IO::Socket::INET->new("localhost:$serverport")
         || IO::Socket::INET->new("127.0.0.1:$serverport");

    if ($has_perlio) {
	print binmode($sock, ":utf8") ? "ok 19\n" : "not ok 19\n";
    } else {
	print "ok 19 - Skip: no perlio\n";
    }

    if ($sock) {

	if ($has_perlio) {
	    $sock->print("ping \x{100}\n");
	    chomp(my $pong = scalar <$sock>);
	    print $pong =~ /^pong (.+)$/ && $1 eq "\x{100}" ?
		"ok 20\n" : "not ok 20\n";

	    $sock->print("ord \x{100}\n");
	    chomp(my $ord = scalar <$sock>);
	    print $ord == 0x100 ?
		"ok 21\n" : "not ok 21\n";

	    $sock->print("chr 0x100\n");
	    chomp(my $chr = scalar <$sock>);
	    print $chr eq "\x{100}" ?
		"ok 22\n" : "not ok 22\n";
	} else {
	    print "ok $_ - Skip: no perlio\n" for 20..22;
	}

	$sock->print("send\n");

	my @array = ();
	while( !eof( $sock ) ){
	    while( <$sock>) {
		push( @array, $_);
		last;
	    }
	}

	$sock->print("done\n");
	$sock->close;

	print "not " if( @array != @data);
    } else {
	print "not ";
    }
    print "ok 23\n";

    ### TEST 24
    ### Stop the server
    #
    $sock = IO::Socket::INET->new("localhost:$serverport")
         || IO::Socket::INET->new("127.0.0.1:$serverport");

    if ($sock) {
	$sock->print("done\n");
	$sock->close;

	print "not " if( 1 != kill 0, $server_pid);
    } else {
	print "not ";
    }
    print "ok 24\n";

} elsif (defined($server_pid)) {
   
    ### Child
    #
    SERVER_LOOP: while (1) {
	last SERVER_LOOP unless my $sock = $listen->accept;
	# Do not print ok/not ok for this binmode() since there's
	# a race condition with our client, just die if we fail.
	if ($has_perlio) { binmode($sock, ":utf8") or die }
	while (<$sock>) {
	    last SERVER_LOOP if /^quit/;
	    last if /^done/;
	    if (/^ping (.+)/) {
		print $sock "pong $1\n";
		next;
	    }
	    if (/^ord (.+)/) {
		print $sock ord($1), "\n";
		next;
	    }
	    if (/^chr (.+)/) {
		print $sock chr(hex($1)), "\n";
		next;
	    }
	    if (/^send/) {
		print $sock @data;
		last;
	    }
	    print;
	}
	$sock = undef;
    }
    $listen->close;
    exit 0;

} else {

    ### Fork failed
    #
    print "not ok 17\n";
    die;
}

# test Blocking option in constructor

my $sock = IO::Socket::INET->new(Blocking => 0)
    or print "not ";
print "ok 25\n";

if ( $^O eq 'qnx' ) {
  print "ok 26 # skipped on QNX4\n";
  # QNX4 library bug: Can set non-blocking on socket, but
  # cannot return that status.
} else {
  my $status = $sock->blocking;
  print "not " unless defined $status && !$status;
  print "ok 26\n";
}
