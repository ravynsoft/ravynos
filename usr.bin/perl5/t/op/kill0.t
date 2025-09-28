#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

BEGIN {
    if ($^O eq 'riscos') {
	skip_all("kill() not implemented on this platform");
    }
}

use strict;
use Config;

plan tests => 9;

ok( kill(0, $$), 'kill(0, $pid) returns true if $pid exists' );

# It's not easy to come up with an individual PID that is known not to exist,
# so just check that at least some PIDs in a large range are reported not to
# exist.
my $count = 0;
my $total = 30_000;
for my $pid (1 .. $total) {
  ++$count if kill(0, $pid);
}
# It is highly unlikely that all of the above PIDs are genuinely in use,
# so $count should be less than $total.
ok( $count < $total, 'kill(0, $pid) returns false if $pid does not exist' );

# Verify that trying to kill a non-numeric PID is fatal
my @bad_pids = (
    [ undef , 'undef'         ],
    [ ''    , 'empty string'  ],
    [ 'abcd', 'alphabetic'    ],
);

for my $case ( @bad_pids ) {
  my ($pid, $name) = @$case;
  eval { kill 0, $pid };
  like( $@, qr/^Can't kill a non-numeric process ID/, "dies killing $name pid");
}

# Verify that killing a magic variable containing a number doesn't
# trigger the above
{
  my $x = $$ . " ";
  $x =~ /(\d+)/;
  ok(eval { kill 0, $1 }, "can kill a number string in a magic variable");
}


# RT #121230: test process group kill on Win32

SKIP: {
  skip 'custom process group kill() only on Win32', 3 if ($^O ne 'MSWin32');

  # Create 2 child processes: an outer one created by kill0.t that runs
  # the "op/kill0_child" script, and an inner one created by outer that
  # just does 'sleep 5'. We then try to kill both of them as a single
  # process group. If only the outer one is killed, the inner will stay
  # around and eventually print "not ok 9999", presenting out of sequence
  # TAP to harness. The outer child creates a temporary file when it is
  # ready.

  my $killfile = 'tmp-killchildstarted';
  unlink($killfile);
  die "can't unlink $killfile: $!" if -e $killfile;
  eval q{END {unlink($killfile);}};

  my $pid = system(1, $^X, 'op/kill0_child', $killfile);
  die 'PID is 0' if !$pid;
  while( ! -e $killfile) {
    sleep 1; # a sleep 0 with $i++ would take ~160 iterations here
  }
  # (some ways to manually make this test fail:
  #   change '-KILL' to 'KILL';
  #   change $pid to a bogus number)
  is(kill('-KILL', $pid), 1, 'process group kill, named signal');

  # create a mapping of signal names to numbers

  my ($i, %signo, @signame, $sig_name) = 0;
  ($sig_name = $Config{sig_name}) || die "No signals?";
  foreach my $name (split(' ', $sig_name)) {
    $signo{$name} = $i;
    $signame[$i] = $name;
    $i++;
  }
  ok(scalar keys %signo > 1 && exists $signo{KILL},
        '$Config{sig_name} parsed correctly');
  die "a child proc wasn't killed and did cleanup on its own" if ! -e $killfile;
  unlink $killfile;

  # Now repeat the test with a numeric kill sigbal

  die "can't unlink" if -e $killfile;
  # no need to create another END block: already done earlier
  $pid = system(1, $^X, 'op/kill0_child', $killfile);
  die 'PID is 0' if !$pid;
  while( ! -e $killfile) {
    sleep 1; # a sleep 0 with $i++ would take ~160 iterations here
  }
  is(kill(-$signo{KILL}, $pid), 1, 'process group kill, numeric signal');
}
