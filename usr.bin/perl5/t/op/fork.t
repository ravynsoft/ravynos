#!./perl

# tests for both real and emulated fork()

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
    skip_all('no fork')
	unless ($Config::Config{d_fork} or $Config::Config{d_pseudofork});
    skip_all('no fork')
        if $^O eq 'MSWin32' && is_miniperl();
}

$|=1;

run_multiple_progs('', \*DATA);

my $shell = $ENV{SHELL} || '';
SKIP: {
    skip "This test can only be run under bash or zsh"
        unless $shell =~ m{/(?:ba|z)sh$};
    skip "LSAN noise failing to create a thread due to limits"
        if $Config::Config{ccflags} =~ /sanitize=address/;
    my $probe = qx{
        $shell -c 'ulimit -u 1 2>/dev/null && echo good'
    };
    chomp $probe;
    skip "Can't set ulimit -u on this system: $probe"
	unless $probe eq 'good';

    my $out = qx{
        $shell -c 'ulimit -u 1; exec $^X -e "
            print((() = fork) == 1 ? q[ok] : q[not ok])
        "'
    };
    # perl #117141
    skip "fork() didn't fail, maybe you're running as root", 1
      if $out eq "okok";
    is($out, "ok", "bash/zsh-only test for 'fork' returning undef on failure");
}

done_testing();

__END__
$| = 1;
if ($cid = fork) {
    sleep 1;
    if ($result = (kill 9, $cid)) {
	print "ok 2\n";
    }
    else {
	print "not ok 2 $result\n";
    }
    sleep 1 if $^O eq 'MSWin32';	# avoid WinNT race bug
}
else {
    print "ok 1\n";
    sleep 10;
}
EXPECT
OPTION random
ok 1
ok 2
########
$| = 1;
if ($cid = fork) {
    sleep 1;
    print "not " unless kill 'INT', $cid;
    print "ok 2\n";
}
else {
    # XXX On Windows the default signal handler kills the
    # XXX whole process, not just the thread (pseudo-process)
    $SIG{INT} = sub { exit };
    print "ok 1\n";
    sleep 5;
    die;
}
EXPECT
OPTION random
ok 1
ok 2
########
$| = 1;
sub forkit {
    print "iteration $i start\n";
    my $x = fork;
    if (defined $x) {
	if ($x) {
	    print "iteration $i parent\n";
	}
	else {
	    print "iteration $i child\n";
	}
    }
    else {
	print "pid $$ failed to fork\n";
    }
}
while ($i++ < 3) { do { forkit(); }; }
EXPECT
OPTION random
iteration 1 start
iteration 1 parent
iteration 1 child
iteration 2 start
iteration 2 parent
iteration 2 child
iteration 2 start
iteration 2 parent
iteration 2 child
iteration 3 start
iteration 3 parent
iteration 3 child
iteration 3 start
iteration 3 parent
iteration 3 child
iteration 3 start
iteration 3 parent
iteration 3 child
iteration 3 start
iteration 3 parent
iteration 3 child
########
$| = 1;
fork()
 ? (print("parent\n"),sleep(1))
 : (print("child\n"),exit) ;
EXPECT
OPTION random
parent
child
########
$| = 1;
fork()
 ? (print("parent\n"),exit)
 : (print("child\n"),sleep(1)) ;
EXPECT
OPTION random
parent
child
########
$| = 1;
@a = (1..3);
for (@a) {
    if (fork) {
	print "parent $_\n";
	$_ = "[$_]";
    }
    else {
	print "child $_\n";
	$_ = "-$_-";
    }
}
print "@a\n";
EXPECT
OPTION random
parent 1
child 1
parent 2
child 2
parent 2
child 2
parent 3
child 3
parent 3
child 3
parent 3
child 3
parent 3
child 3
[1] [2] [3]
-1- [2] [3]
[1] -2- [3]
[1] [2] -3-
-1- -2- [3]
-1- [2] -3-
[1] -2- -3-
-1- -2- -3-
########
$| = 1;
foreach my $c (1,2,3) {
    if (fork) {
	print "parent $c\n";
    }
    else {
	print "child $c\n";
	exit;
    }
}
while (wait() != -1) { print "waited\n" }
EXPECT
OPTION random
child 1
child 2
child 3
parent 1
parent 2
parent 3
waited
waited
waited
########
use Config;
$| = 1;
$\ = "\n";
fork()
 ? print($Config{osname} eq $^O)
 : print($Config{osname} eq $^O) ;
EXPECT
OPTION random
1
1
########
$| = 1;
$\ = "\n";
fork()
 ? do { require Config; print($Config::Config{osname} eq $^O); }
 : do { require Config; print($Config::Config{osname} eq $^O); }
EXPECT
OPTION random
1
1
########
$| = 1;
use Cwd;
my $cwd = cwd(); # Make sure we load Win32.pm while "../lib" still works.
$\ = "\n";
my $dir;
if (fork) {
    $dir = "f$$.tst";
    mkdir $dir, 0755;
    chdir $dir;
    print cwd() =~ /\Q$dir/i ? "ok 1 parent" : "not ok 1 parent";
    chdir "..";
    rmdir $dir;
}
else {
    sleep 2;
    $dir = "f$$.tst";
    mkdir $dir, 0755;
    chdir $dir;
    print cwd() =~ /\Q$dir/i ? "ok 1 child" : "not ok 1 child";
    chdir "..";
    rmdir $dir;
}
EXPECT
OPTION random
ok 1 parent
ok 1 child
########
$| = 1;
$\ = "\n";
my $getenv;
if ($^O eq 'MSWin32') {
    $getenv = qq[$^X -e "print \$ENV{TST}"];
}
else {
    $getenv = qq[$^X -e 'print \$ENV{TST}'];
}
$ENV{TST} = 'foo';
if (fork) {
    sleep 1;
    print "parent before: " . `$getenv`;
    $ENV{TST} = 'bar';
    print "parent after: " . `$getenv`;
}
else {
    print "child before: " . `$getenv`;
    $ENV{TST} = 'baz';
    print "child after: " . `$getenv`;
}
EXPECT
OPTION random
child before: foo
child after: baz
parent before: foo
parent after: bar
########
$| = 1;
$\ = "\n";
if ($pid = fork) {
    waitpid($pid,0);
    print "parent got $?"
}
else {
    exit(42);
}
EXPECT
OPTION random
parent got 10752
########
$| = 1;
$\ = "\n";
my $echo = 'echo';
if ($^O =~ /android/) {
    $echo = q{sh -c 'echo $@' -- };
}
if ($pid = fork) {
    waitpid($pid,0);
    print "parent got $?"
}
else {
    exec("$echo foo");
}
EXPECT
OPTION random
foo
parent got 0
########
if (fork) {
    die "parent died";
}
else {
    die "child died";
}
EXPECT
OPTION random
parent died at - line 2.
child died at - line 5.
########
if ($pid = fork) {
    eval { die "parent died" };
    print $@;
}
else {
    eval { die "child died" };
    print $@;
}
EXPECT
OPTION random
parent died at - line 2.
child died at - line 6.
########
if (eval q{$pid = fork}) {
    eval q{ die "parent died" };
    print $@;
}
else {
    eval q{ die "child died" };
    print $@;
}
EXPECT
OPTION random
parent died at (eval 2) line 1.
child died at (eval 2) line 1.
########
BEGIN {
    $| = 1;
    fork and exit;
    print "inner\n";
}
# XXX In emulated fork(), the child will not execute anything after
# the BEGIN block, due to difficulties in recreating the parse stacks
# and restarting yyparse() midstream in the child.  This can potentially
# be overcome by treating what's after the BEGIN{} as a brand new parse.
#print "outer\n"
EXPECT
OPTION random
inner
########
sub pipe_to_fork ($$) {
    my $parent = shift;
    my $child = shift;
    pipe($child, $parent) or die;
    my $pid = fork();
    die "fork() failed: $!" unless defined $pid;
    close($pid ? $child : $parent);
    $pid;
}

if (pipe_to_fork('PARENT','CHILD')) {
    # parent
    print PARENT "pipe_to_fork\n";
    close PARENT;
}
else {
    # child
    while (<CHILD>) { print; }
    close CHILD;
    exit;
}

sub pipe_from_fork ($$) {
    my $parent = shift;
    my $child = shift;
    pipe($parent, $child) or die;
    my $pid = fork();
    die "fork() failed: $!" unless defined $pid;
    close($pid ? $child : $parent);
    $pid;
}

if (pipe_from_fork('PARENT','CHILD')) {
    # parent
    while (<PARENT>) { print; }
    close PARENT;
}
else {
    # child
    print CHILD "pipe_from_fork\n";
    close CHILD;
    exit;
}
EXPECT
OPTION random
pipe_from_fork
pipe_to_fork
########
$|=1;
if ($pid = fork()) {
    print "forked first kid\n";
    print "waitpid() returned ok\n" if waitpid($pid,0) == $pid;
}
else {
    print "first child\n";
    exit(0);
}
if ($pid = fork()) {
    print "forked second kid\n";
    print "wait() returned ok\n" if wait() == $pid;
}
else {
    print "second child\n";
    exit(0);
}
EXPECT
OPTION random
forked first kid
first child
waitpid() returned ok
forked second kid
second child
wait() returned ok
########
pipe(RDR,WTR) or die $!;
my $pid = fork;
die "fork: $!" if !defined $pid;
if ($pid == 0) {
    close RDR;
    print WTR "STRING_FROM_CHILD\n";
    close WTR;
} else {
    close WTR;
    chomp(my $string_from_child  = <RDR>);
    close RDR;
    print $string_from_child eq "STRING_FROM_CHILD", "\n";
}
EXPECT
OPTION random
1
########
# [perl #39145] Perl_dounwind() crashing with Win32's fork() emulation
sub { @_ = 3; fork ? die "1\n" : die "1\n" }->(2);
EXPECT
OPTION random
1
1
########
# [perl #72604] @DB::args stops working across Win32 fork
$|=1;
sub f {
    if ($pid = fork()) {
	print "waitpid() returned ok\n" if waitpid($pid,0) == $pid;
    }
    else {
	package DB;
	my @c = caller(0);
	print "child: called as [$c[3](", join(',',@DB::args), ")]\n";
	exit(0);
    }
}
f("foo", "bar");
EXPECT
OPTION random
child: called as [main::f(foo,bar)]
waitpid() returned ok
########
# Windows 2000: https://rt.cpan.org/Ticket/Display.html?id=66016#txn-908976
system $^X,  "-e", "if (\$pid=fork){sleep 1;kill(9, \$pid)} else {sleep 5}";
print $?>>8, "\n";
EXPECT
0
########
# Windows 7: https://rt.cpan.org/Ticket/Display.html?id=66016#txn-908976
system $^X,  "-e", "if (\$pid=fork){kill(9, \$pid)} else {sleep 5}";
print $?>>8, "\n";
EXPECT
0
########
# Windows fork() emulation: can we still waitpid() after signalling SIGTERM?
$|=1;
if (my $pid = fork) {
    sleep 1;
    print "1\n";
    kill 'TERM', $pid;
    waitpid($pid, 0);
    print "4\n";
}
else {
    $SIG{TERM} = sub { print "2\n" };
    sleep 10;
    print "3\n";
}
EXPECT
1
2
3
4
########
# this used to SEGV. RT # 121721
$|=1;
&main;
sub main {
    if (my $pid = fork) {
	waitpid($pid, 0);
    }
    else {
        print "foo\n";
    }
}
EXPECT
foo
########
# ${^GLOBAL_PHASE} at the end of a pseudo-fork
if (my $pid = fork) {
    waitpid $pid, 0;
} else {
    eval 'END { print "${^GLOBAL_PHASE}\n" }';
    exit;
}
EXPECT
END
