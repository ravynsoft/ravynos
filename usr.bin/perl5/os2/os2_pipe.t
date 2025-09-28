#!/usr/bin/perl -w
BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Test::More tests => 80;
use strict;
use IO::Handle;
use Fcntl;

my $pname = "/pipe/perl_pipe_test$$";

ok !eval {OS2::pipe $pname, 'wait'}, 'wait for non-existing pipe fails';
is 0 + $^E, 3, 'correct error code';
ok my $server_pipe = OS2::pipe($pname, 'rw'), 'create pipe, no connect';
ok((my $fd = fileno $server_pipe) >= 0, 'has a fileno');
is +(OS2::pipeCntl($server_pipe, 'readstate'))[0], 2, 'is listening';
is OS2::pipeCntl($server_pipe, 'state') & 0xFF, 1, 'max count=1';

ok 0 > OS2::pipeCntl($server_pipe, 'connect', !'wait'), 'connect nowait';

ok open(my $fh, '+<', $pname), 'open client end';
#ok sysopen($fh, $pname, O_RDWR), 'sysopen client end' . $^E;
#my ($fd1, $action) = OS2::open $pname, 0x2042 or warn $^E; # ERROR,SHARE,RDWR
is +(OS2::pipeCntl($server_pipe, 'readstate'))[0], 3, 'is connected';
ok 0 < OS2::pipeCntl($server_pipe, 'connect', !'wait'), 'connect nowait';
ok OS2::pipeCntl($server_pipe, 'connect', 'wait'), 'connect wait';
is $server_pipe->autoflush, 0, 'autoflush server'; # Returns the old value
is $fh->autoflush, 0, 'autoflush';	# Returns the old value
ok syswrite($server_pipe, "some string\n"), 'server write';
is scalar <$fh>, "some string\n", 'client read';
ok syswrite($fh, "another string\n"), 'client write';

is OS2::pipeCntl($server_pipe, 'peek'), "another string\n", 'peeking is fine';
my ($st, $bytesAvail, $bytesInMess) = OS2::pipeCntl($server_pipe, 'readstate');
my ($name, $remoteID, $outBuffer, $inBuffer, $maxInstance, $countInstance)
  = OS2::pipeCntl($server_pipe, 'info');
is $bytesAvail, length("another string\n"), 'count bytes';
is $remoteID, 0, 'not remote';
is $maxInstance, 1, 'max count is 1';
is $countInstance, 1, 'count is 1';
#is $len, length($pname) + 1, 'length of name is 1 more than the actual';
(my $tmp = $pname) =~ s,/,\\,g;
is lc $name, lc $tmp, 'name is correct (up to case)';

# If do print() instead of syswrite(), this gets "some string\n" instead!!!
is scalar <$server_pipe>, "another string\n", 'server read';

ok !open(my $fh1, '+<', $pname), 'open client end fails';

# No new child present, return -1
ok 0 > OS2::pipeCntl($server_pipe, 'reset', !'wait'), 'server reset, no wait';
ok eof($fh), 'client EOF';
ok(($fh->clearerr, 1), 'client clear EOF');	# XXXX Returns void

$!=0; $^E = 0;
ok close $fh, 'close client';
#diag $!;
#diag $^E;
is fileno $fh, undef, 'was actually closed...';

ok open($fh, '+<', $pname), 'open client end';

is $fh->autoflush, 1, 'autoflush';	# Returns the old value
ok syswrite($server_pipe, "some string\n"), 'server write';
is scalar <$fh>, "some string\n", 'client read';
ok syswrite($fh, "another string\n"), 'client write';

# If do print() instead of syswrite(), this gets "some string\n" instead!!!
is scalar <$server_pipe>, "another string\n", 'server read';

ok syswrite($server_pipe, "some string\n"), 'server write';
ok syswrite($fh, "another string\n"), 'client write';
is scalar <$fh>, "some string\n", 'client read';

# If do print() instead of syswrite(), this gets "some string\n" instead!!!
is scalar <$server_pipe>, "another string\n", 'server read';

ok syswrite($server_pipe, "some string\n"), 'server write';
ok syswrite($fh, "another string\n"), 'client write';

ok((sysread $fh, my $in, 2000), 'client sysread');
is $in, "some string\n", 'client sysread correct';

# If do print() instead of syswrite(), this gets "some string\n" instead!!!
ok((sysread $server_pipe, $in, 2000), 'server sysread');
is $in, "another string\n", 'server sysread correct';

ok !open($fh1, '+<', $pname), 'open client end fails';

# XXXX Not needed???
#ok(($fh->clearerr, 1), 'client clear EOF');	# XXXX Returns void

ok close $fh, 'close client';
ok eof $server_pipe, 'server EOF';	# Creates an error condition

my $pid = system 4|0x40000, $^X, '-wle', <<'EOS', $pname; # SESSION|INDEPENDENT
  my $success;
  END {sleep($success ? 1 : 10);}
  my $mess = '';
  $SIG{TERM} = sub {die "kid1 error: Got SIGTERM\nmess=`$mess'"};
  my $pn = shift;
  my $fh;
  eval {
    $mess .= "Pipe open fails\n" unless open $fh, '+<', $pn;
    my $t = time;		### TIMESTAMP0
    warn "kid1: Wait for pipe...\n";
    $mess .= "Pipe became available\n" if OS2::pipe $pn, 'wait';
    my $t1 = time() - $t;	### TIMESTAMP1
    $mess .= "Unexpected delay $t1\n" unless $t1 >= 1 and $t1 <= 3;
    warn "kid1: sleep 4...\n";
    sleep 4;
    $mess .= "Pipe open\n" if open $fh, '+<', $pn;
    binmode $fh;
    1;				### TIMESTAMP2
  } or warn $@;
  warn "kid1: pipe opened...\n";
  select $fh; $| = 1;
  my $c = syswrite $fh, $mess or warn "print: $!";
  warn "kid1: Wrote $c bytes\n";
  warn $mess;
  close $fh or die "kid1 error: close: $!";
  $success = 1;
EOS

ok $pid > 0, 'kid pid';

### TIMESTAMP0
sleep 2;
my $t = time;
### TIMESTAMP1
# New child present; will clear error condition...
ok 0 < OS2::pipeCntl($server_pipe, 'reset', 'wait'), 'server reset, wait';
### TIMESTAMP2
my $t1 = time() - $t;
ok $t1 <= 6 && $t1 >= 2, 'correct delay';

sleep 2;

ok binmode($server_pipe), 'binmode';
ok !eof $server_pipe, 'server: no EOF';
my @in = <$server_pipe>;
my @exp = ( "Pipe open fails\n", "Pipe became available\n", "Pipe open\n");

is "@in", "@exp", 'expected data';

# Can't switch to message mode if created in byte mode...
ok close $server_pipe, 'server close';
ok $server_pipe = OS2::pipe($pname, 'RW'), 'create pipe in message mode';
ok OS2::pipeCntl($server_pipe, 'byte'),    'can switch to byte mode';
ok OS2::pipeCntl($server_pipe, 'message'), 'can switch to message mode';

$pid = system 4|0x40000, $^X, '-wle', <<'EOS', $pname, $$; # SESSION|INDEPENDENT
  END {sleep 2}
  my ($name, $ppid) = (shift, shift);
  $name =~ s,/,\\,g;
  $name = uc $name;
  warn "kid2: OS2::pipe $name, 'call', ...\n";
  my $got = OS2::pipe $name, 'call', "Is your pid $ppid?\n";
  my $ok = $got eq 'Yes';
  warn "kid2: got `$got'\n";
  OS2::pipe $name, 'call', $ok ? "fine\n" : "bad\n";
EOS

ok $pid, 'kid started';
sleep 2;			# XXX How to synchronize with kid???
$in = scalar <$server_pipe>;
my $ok1 = ($in || '') eq "Is your pid $$?\n";
is $in, "Is your pid $$?\n", 'call in';
ok syswrite($server_pipe, $ok1 ? 'Yes' : 'No' ), 'server write';

ok 0 < OS2::pipeCntl($server_pipe, 'reset', 'wait'), 'server reset, wait';
$in = scalar <$server_pipe>;
is $in, "fine\n", 'call in';
ok syswrite($server_pipe, 'ending' ), 'server write';

ok close $server_pipe, 'server close';

ok $server_pipe = OS2::pipe($pname, 'W'), 'create pipe in message write mode';
ok !eval {OS2::pipeCntl($server_pipe, 'readstate'); 1}, 'readstate fails, as expected';
ok close $server_pipe, 'server close';

ok $server_pipe = OS2::pipe($pname, 'w'), 'create pipe in byte write mode';
ok !eval {OS2::pipeCntl($server_pipe, 'readstate'); 1}, 'readstate fails, as expected';
ok close $server_pipe, 'server close';

ok $server_pipe = OS2::pipe($pname, 'r'), 'create pipe in byte read mode';
is +(OS2::pipeCntl($server_pipe, 'readstate'))[0], 2, 'is listening';
ok close $server_pipe, 'server close';

ok $server_pipe = OS2::pipe($pname, 'r', 0), 'create-no-connect pipe in byte read mode';
is +(OS2::pipeCntl($server_pipe, 'readstate'))[0], 1, 'is disconnected';
ok close $server_pipe, 'server close';

ok $server_pipe = OS2::pipe($pname, 'R'), 'create pipe in message read mode';
is +(OS2::pipeCntl($server_pipe, 'readstate'))[0], 2, 'is listening';
ok close $server_pipe, 'server close';

#is waitpid($pid, 0), $pid, 'kid ended';
#is $?, 0, 'kid exitcode';
