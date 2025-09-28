#!./perl

my $perl;

BEGIN {
    $perl = $^X;
}

use Config;

BEGIN {
    my $can_fork = $Config{d_fork} ||
		    (($^O eq 'MSWin32' || $^O eq 'NetWare') and
		     $Config{useithreads} and 
		     $Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/
		    );
    my $reason;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bIO\b/) {
	$reason = 'IO extension unavailable';
    }
    elsif (!$can_fork) {
        $reason = 'no fork';
    }
    elsif ($^O eq 'MSWin32' && !$ENV{TEST_IO_PIPE}) {
	$reason = 'Win32 testing environment not set';
    }
    if ($reason) {
	print "1..0 # Skip: $reason\n";
	exit 0;
    }
}

use IO::Pipe;

my $is_win32=$^O eq 'MSWin32' ? "MSWin32 has broken pipes" : "";

$| = 1;
print "1..10\n";

my $pipe;

if ($is_win32) {
    print "ok $_ # skipped: $is_win32\n" for 1..4;
} else {
    $pipe = IO::Pipe->new()->reader($perl, '-e', 'print qq(not ok 1\n)');
    while (<$pipe>) {
      s/^not //;
      print;
    }
    $pipe->close or print "# \$!=$!\nnot ";
    print "ok 2\n";
    my $cmd = 'BEGIN{$SIG{ALRM} = sub {print qq(not ok 4\n); exit}; alarm 10} s/not //';
    $pipe = IO::Pipe->new()->writer($perl, '-pe', $cmd);
    print $pipe "not ok 3\n" ;
    $pipe->close or print "# \$!=$!\nnot ";
    print "ok 4\n";
}

# Check if can fork with dynamic extensions (bug in CRT):
if ($^O eq 'os2' and
    system "$^X -I../lib -MOpcode -e 'defined fork or die'  > /dev/null 2>&1") {
    print "ok $_ # skipped: broken fork\n" for 5..10;
    exit 0;
}

$pipe = IO::Pipe->new();

my $pid = fork();

if($pid)
 {
  $pipe->writer;
  print $pipe "Xk 5\n";
  print $pipe "oY 6\n";
  $pipe->close;
  wait;
 }
elsif(defined $pid)
 {
  $pipe->reader;
  my $stdin = bless \*STDIN, "IO::Handle";
  $stdin->fdopen($pipe,"r");
  exec $^X, '-pne', 'tr/YX/ko/';
 }
else
 {
  die "# error = $!";
 }

if ($is_win32) {
    print "ok $_ # skipped: $is_win32\n" for 7..8;
} else {
    $pipe = IO::Pipe->new();
    my $pid = fork();

    if($pid)
 {
  $pipe->reader;
  while(<$pipe>) {
      s/^not //;
      print;
  }
  $pipe->close;
  wait;
 }
    elsif(defined $pid)
 {
  $pipe->writer;

  my $stdout = bless \*STDOUT, "IO::Handle";
  $stdout->fdopen($pipe,"w");
  print STDOUT "not ok 7\n";
  my @echo = 'echo';
  if ( $^O =~ /android/ ) {
     @echo = ('sh', '-c', q{echo $@}, '--');
  }
  exec @echo, 'not ok 8';
 }
    else
 {
  die;
 }
}
if ($is_win32) {
    print "ok $_ # skipped: $is_win32\n" for 9;
} else {
    $pipe = IO::Pipe->new;
    $pipe->writer;

    $SIG{'PIPE'} = 'broken_pipe';

    sub broken_pipe {
    print "ok 9\n";
    }

    print $pipe "not ok 9\n";
    $pipe->close;

    sleep 1;
}
print "ok 10\n";

