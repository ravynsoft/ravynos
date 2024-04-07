use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        print("1..0 # SKIP threads::shared not available\n");
        exit(0);
    }

    $| = 1;
    print("1..20\n");   ### Number of tests that will be run ###
};

my $TEST;
BEGIN {
    share($TEST);
    $TEST = 1;
}

ok(1, 'Loaded');

sub ok {
    my ($ok, $name) = @_;

    lock($TEST);
    my $id = $TEST++;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}

sub skip {
    ok(1, '# SKIP ' . $_[0]);
}


### Start of Testing ###

{
    my $retval = threads->create(sub { return ("hi") })->join();
    ok($retval eq 'hi', "Check basic returnvalue");
}
{
    my ($thread) = threads->create(sub { return (1,2,3) });
    my @retval = $thread->join();
    ok($retval[0] == 1 && $retval[1] == 2 && $retval[2] == 3,'');
}
{
    my $retval = threads->create(sub { return [1] })->join();
    ok($retval->[0] == 1,"Check that a array ref works",);
}
{
    my $retval = threads->create(sub { return { foo => "bar" }})->join();
    ok($retval->{foo} eq 'bar',"Check that hash refs work");
}
{
    my $retval = threads->create( sub {
        open(my $fh, "+>threadtest") || die $!;
        print $fh "test\n";
        return $fh;
    })->join();
    ok(ref($retval) eq 'GLOB', "Check that we can return FH $retval");
    print $retval "test2\n";
    close($retval);
    unlink("threadtest");
}
{
    my $test = "hi";
    my $retval = threads->create(sub { return $_[0]}, \$test)->join();
    ok($$retval eq 'hi','');
}
{
    my $test = "hi";
    share($test);
    my $retval = threads->create(sub { return $_[0]}, \$test)->join();
    ok($$retval eq 'hi','');
    $test = "foo";
    ok($$retval eq 'foo','');
}
{
    my %foo;
    share(%foo);
    threads->create(sub { 
        my $foo;
        share($foo);
        $foo = "thread1";
        return $foo{bar} = \$foo;
    })->join();
    ok(1,"");
}

# We parse ps output so this is OS-dependent.
if ($^O eq 'linux') {
    # First modify $0 in a subthread.
    #print "# mainthread: \$0 = $0\n";
    threads->create(sub{ #print "# subthread: \$0 = $0\n";
                        $0 = "foobar";
                        #print "# subthread: \$0 = $0\n"
                 })->join;
    #print "# mainthread: \$0 = $0\n";
    #print "# pid = $$\n";
    if (open PS, "ps -f |") { # Note: must work in (all) systems.
        my ($sawpid, $sawexe);
        while (<PS>) {
            chomp;
            #print "# [$_]\n";
            if (/^\s*\S+\s+$$\s/) {
                $sawpid++;
                if (/\sfoobar\s*$/) { # Linux 2.2 leaves extra trailing spaces.
                    $sawexe++;
                }
                last;
            }
        }
        close PS or die;
        if ($sawpid) {
            ok($sawpid && $sawexe, 'altering $0 is effective');
        } else {
            skip("\$0 check: did not see pid $$ in 'ps -f |'");
        }
    } else {
        skip("\$0 check: opening 'ps -f |' failed: $!");
    }
} else {
    skip("\$0 check: only on Linux");
}

{
    my $t = threads->create(sub {});
    $t->join();
    threads->create(sub {})->join();
    eval { $t->join(); };
    ok(($@ =~ /Thread already joined/), "Double join works");
    eval { $t->detach(); };
    ok(($@ =~ /Cannot detach a joined thread/), "Detach joined thread");
}

{
    my $t = threads->create(sub {});
    $t->detach();
    threads->create(sub {})->join();
    eval { $t->detach(); };
    ok(($@ =~ /Thread already detached/), "Double detach works");
    eval { $t->join(); };
    ok(($@ =~ /Cannot join a detached thread/), "Join detached thread");
}

{
    # The "use IO::File" is not actually used for anything; its only purpose
    # is incite a lot of calls to newCONSTSUB.  See the p5p archives for
    # the thread "maint@20974 or before broke mp2 ithreads test".
    use IO::File;
    # This coredumped between #20930 and #21000
    $_->join for map threads->create(sub{ok($_, "stress newCONSTSUB")}), 1..2;
}

{
    my $go : shared = 0;

    my $t = threads->create( sub {
        lock($go);
        cond_wait($go) until $go;
    }); 

    my $joiner = threads->create(sub { $_[0]->join }, $t);

    threads->yield();
    sleep 1;
    eval { $t->join; };
    ok(($@ =~ /^Thread already joined at/)?1:0, "Join pending join");

    { lock($go); $go = 1; cond_signal($go); }
    $joiner->join;
}

{
    my $go : shared = 0;
    my $t = threads->create( sub {
        eval { threads->self->join; };
        ok(($@ =~ /^Cannot join self/), "Join self");
        lock($go); $go = 1; cond_signal($go);
    });

    { lock ($go); cond_wait($go) until $go; }
    $t->join;
}

{
    my $go : shared = 0;
    my $t = threads->create( sub {
        lock($go);  cond_wait($go) until $go;
    });
    my $joiner = threads->create(sub { $_[0]->join; }, $t);

    threads->yield();
    sleep 1;
    eval { $t->detach };
    ok(($@ =~ /^Cannot detach a joined thread at/)?1:0, "Detach pending join");

    { lock($go); $go = 1; cond_signal($go); }
    $joiner->join;
}

exit(0);

# EOF
