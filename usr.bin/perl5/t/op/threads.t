#!perl

BEGIN {
     chdir 't' if -d 't';
     require './test.pl';
     set_up_inc( '../lib' );
     $| = 1;

     skip_all_without_config('useithreads');
     skip_all_if_miniperl("no dynamic loading on miniperl, no threads");

     plan(30);
}

use strict;
use warnings;
use threads;

# test that we don't get:
# Attempt to free unreferenced scalar: SV 0x40173f3c
fresh_perl_is(<<'EOI', 'ok', { }, 'delete() under threads');
use threads;
threads->create(sub { my %h=(1,2); delete $h{1}})->join for 1..2;
print "ok";
EOI

#PR24660
# test that we don't get:
# Attempt to free unreferenced scalar: SV 0x814e0dc.
fresh_perl_is(<<'EOI', 'ok', { }, 'weaken ref under threads');
use threads;
no warnings 'experimental::builtin';
use builtin 'weaken';
my $data = "a";
my $obj = \$data;
my $copy = $obj;
weaken($copy);
threads->create(sub { 1 })->join for (1..1);
print "ok";
EOI

#PR24663
# test that we don't get:
# panic: magic_killbackrefs.
# Scalars leaked: 3
fresh_perl_is(<<'EOI', 'ok', { }, 'weaken ref #2 under threads');
package Foo;
sub new { bless {},shift }
package main;
use threads;
no warnings 'experimental::builtin';
use builtin 'weaken';
my $object = Foo->new;
my $ref = $object;
weaken $ref;
threads->create(sub { $ref = $object } )->join; # $ref = $object causes problems
print "ok";
EOI

#PR30333 - sort() crash with threads
sub mycmp { length($b) <=> length($a) }

sub do_sort_one_thread {
   my $kid = shift;
   print "# kid $kid before sort\n";
   my @list = ( 'x', 'yy', 'zzz', 'a', 'bb', 'ccc', 'aaaaa', 'z',
                'hello', 's', 'thisisalongname', '1', '2', '3',
                'abc', 'xyz', '1234567890', 'm', 'n', 'p' );

   for my $j (1..99999) {
      for my $k (sort mycmp @list) {}
   }
   print "# kid $kid after sort, sleeping 1\n";
   sleep(1);
   print "# kid $kid exit\n";
}

sub do_sort_threads {
   my $nthreads = shift;
   my @kids = ();
   for my $i (1..$nthreads) {
      my $t = threads->create(\&do_sort_one_thread, $i);
      print "# parent $$: continue\n";
      push(@kids, $t);
   }
   for my $t (@kids) {
      print "# parent $$: waiting for join\n";
      $t->join();
      print "# parent $$: thread exited\n";
   }
}

do_sort_threads(2);        # crashes
ok(1);

# Change 24643 made the mistake of assuming that CvCONST can only be true on
# XSUBs. Somehow it can also end up on perl subs.
fresh_perl_is(<<'EOI', 'ok', { }, 'cloning constant subs');
use constant x=>1;
use threads;
$SIG{__WARN__} = sub{};
async sub {};
print "ok";
EOI

# From a test case by Tim Bunce in
# http://www.nntp.perl.org/group/perl.perl5.porters/63123
fresh_perl_is(<<'EOI', 'ok', { }, 'Ensure PL_linestr can be cloned');
use threads;
print do 'op/threads_create.pl' || die $@;
EOI


# Scalars leaked: 1
foreach my $BLOCK (qw(CHECK INIT)) {
    fresh_perl_is(<<EOI, 'ok', { }, "threads in $BLOCK block");
        use threads;
        $BLOCK { threads->create(sub {})->join; }
        print 'ok';
EOI
}

# Scalars leaked: 1
fresh_perl_is(<<'EOI', 'ok', { }, 'Bug #41138');
    use threads;
    leak($x);
    sub leak
    {
        local $x;
        threads->create(sub {})->join();
    }
    print 'ok';
EOI


# [perl #45053] Memory corruption with heavy module loading in threads
#
# run-time usage of newCONSTSUB (as done by the IO boot code) wasn't
# thread-safe - got occasional coredumps or malloc corruption
watchdog(180, "process");
{
    local $SIG{__WARN__} = sub {};   # Ignore any thread creation failure warnings
    my @t;
    for (1..10) {
        my $thr = threads->create( sub { require IO });
        last if !defined($thr);      # Probably ran out of memory
        push(@t, $thr);
    }
    $_->join for @t;
    ok(1, '[perl #45053]');
}

sub matchit {
    is (ref $_[1], "Regexp");
    like ($_[0], $_[1]);
}

threads->new(\&matchit, "Pie", qr/pie/i)->join();

# tests in threads don't get counted, so
curr_test(curr_test() + 2);


# the seen_evals field of a regexp was getting zeroed on clone, so
# within a thread it didn't  know that a regex object contained a 'safe'
# code expression, so it later died with 'Eval-group not allowed' when
# you tried to interpolate the object

sub safe_re {
    my $re = qr/(?{1})/;	# this is literal, so safe
    eval { "a" =~ /$re$re/ };	# interpolating safe values, so safe
    ok($@ eq "", 'clone seen-evals');
}
threads->new(\&safe_re)->join();

# tests in threads don't get counted, so
curr_test(curr_test() + 1);

# This used to crash in 5.10.0 [perl #64954]

undef *a;
threads->new(sub {})->join;
pass("undefing a typeglob doesn't cause a crash during cloning");


# Test we don't get:
# panic: del_backref during global destruction.
# when returning a non-closure sub from a thread and subsequently starting
# a new thread.
fresh_perl_is(<<'EOI', 'ok', { }, 'No del_backref panic [perl #70748]');
use threads;
sub foo { return (sub { }); }
my $bar = threads->create(\&foo)->join();
threads->create(sub { })->join();
print "ok";
EOI

# Another, more reliable test for the same del_backref bug:
fresh_perl_is(
 <<'   EOJ', 'ok', {}, 'No del_backref panic [perl #70748] (2)'
   use threads;
   push @bar, threads->create(sub{sub{}})->join() for 1...10;
   print "ok";
   EOJ
);

# Simple closure-returning test: At least this case works (though it
# leaks), and we don't want to break it.
fresh_perl_is(<<'EOJ', 'foo', {}, 'returning a closure');
use threads;
print create threads sub {
 my $x = 'foo';
 sub{sub{$x}}
}=>->join->()()
 //"undef"
EOJ

# At the point of thread creation, $h{1} is on the temps stack.
# The weak reference $a, however, is visible from the symbol table.
fresh_perl_is(<<'EOI', 'ok', { }, 'Test for 34394ecd06e704e9');
    use threads;
    no warnings 'experimental::builtin';
    use builtin 'weaken';
    %h = (1, 2);
    $a = \$h{1};
    weaken($a);
    delete $h{1} && threads->create(sub {}, shift)->join();
    print 'ok';
EOI

# This will fail in "interesting" ways if stashes in clone_params is not
# initialised correctly.
fresh_perl_like(<<'EOI', qr/\AThread 1 terminated abnormally: Not a CODE reference/, { }, 'RT #73046');
    use strict;
    use threads;

    sub foo::bar;

    my %h = (1, *{$::{'foo::'}}{HASH});
    *{$::{'foo::'}} = {};

    threads->create({}, delete $h{1})->join();

    print "end";
EOI

fresh_perl_is(<<'EOI', 'ok', { }, '0 refcnt neither on tmps stack nor in @_');
    use threads;
    no warnings 'experimental::builtin';
    use builtin 'weaken';
    my %h = (1, []);
    my $a = $h{1};
    weaken($a);
    delete $h{1} && threads->create(sub {}, shift)->join();
    print 'ok';
EOI

{
    my $got;
    sub stuff {
	my $a;
	if (@_) {
	    $a = "Leakage";
	    threads->create(\&stuff)->join();
	} else {
	    is ($a, undef, 'RT #73086 - clone used to clone active pads');
	}
    }

    stuff(1);

    curr_test(curr_test() + 1);
}

{
    my $got;
    sub more_stuff {
	my $a;
	$::b = \$a;
	if (@_) {
	    $a = "More leakage";
	    threads->create(\&more_stuff)->join();
	} else {
	    is ($a, undef, 'Just special casing lexicals in ?{ ... }');
	}
    }

    more_stuff(1);

    curr_test(curr_test() + 1);
}

# Test from Jerry Hedden, reduced by him from Object::InsideOut's tests.
fresh_perl_is(<<'EOI', 'ok', { }, '0 refcnt during CLONE');
use strict;
use warnings;

use threads;

{
    package My::Obj;
    no warnings 'experimental::builtin';
    use builtin 'weaken';

    my %reg;

    sub new
    {
        # Create object with ID = 1
        my $class = shift;
        my $id = 1;
        my $obj = bless(\do{ my $scalar = $id; }, $class);

        # Save weak copy of object for reference during cloning
        weaken($reg{$id} = $obj);

        # Return object
        return $obj;
    }

    # Return the internal ID of the object
    sub id
    {
        my $obj = shift;
        return $$obj;
    }

    # During cloning 'look' at the object
    sub CLONE {
        foreach my $id (keys(%reg)) {
            # This triggers SvREFCNT_inc() then SvREFCNT_dec() on the referent.
            my $obj = $reg{$id};
        }
    }
}

# Create object in 'main' thread
my $obj = My::Obj->new();
my $id = $obj->id();
die "\$id is '$id'" unless $id == 1;

# Access object in thread
threads->create(
    sub {
        print $obj->id() == 1 ? "ok\n" : "not ok '" . $obj->id() . "'\n";
    }
)->join();

EOI

# make sure peephole optimiser doesn't recurse heavily.
# (We run this inside a thread to get a small stack)

{
    # lots of constructs that have o->op_other etc
    my $code = <<'EOF';
	$r = $x || $y;
	$x ||= $y;
	$r = $x // $y;
	$x //= $y;
	$r = $x && $y;
	$x &&= $y;
	$r = $x ? $y : $z;
	@a = map $x+1, @a;
	@a = grep $x+1, @a;
	$r = /$x/../$y/;

	# this one will fail since we removed tail recursion optimisation
	# with f11ca51e41e8
	#while (1) { $x = 0 };

	while (0) { $x = 0 };
	for ($x=0; $y; $z=0) { $r = 0 };
	for (1) { $x = 0 };
	{ $x = 0 };
	$x =~ s/a/$x + 1/e;
EOF
    $code = 'my ($r, $x,$y,$z,@a); return 5; ' . ($code x 1000);
    my $res = threads->create(sub { eval $code})->join;
    is($res, 5, "avoid peephole recursion");
}


# [perl #78494] Pipes shared between threads block when closed
{
  my $perl = which_perl;
  $perl = qq'"$perl"' if $perl =~ /\s/;
  open(my $OUT, "|$perl") || die("ERROR: $!");
  threads->create(sub { })->join;
  ok(1, "Pipes shared between threads do not block when closed");
}

# [perl #105208] Typeglob clones should not be cloned again during a join
{
  threads->create(sub { sub { $::hypogamma = 3 } })->join->();
  is $::hypogamma, 3, 'globs cloned and joined are not recloned';
}

fresh_perl_is(
  'use threads;' .
  'async { delete $::{INC}; eval q"my $foo : bar" } ->join; print "ok\n";',
  "ok",
   {},
  'no crash when deleting $::{INC} in thread'
);

fresh_perl_is(<<'CODE', 'ok', {}, 'no crash modifying extended array element');
use threads;
my @a = 1;
threads->create(sub { $#a = 1; $a[1] = 2; print qq/ok\n/ })->join;
CODE

fresh_perl_is(<<'CODE', '3.5,3.5', {}, 'RT #36664: Strange behavior of shared array');
use threads;
use threads::shared;

our @List : shared = (1..5);
my $v = 3.5;
$v > 0;
$List[3] = $v;
printf "%s,%s", @List[(3)], $List[3];
CODE

fresh_perl_like(<<'CODE', qr/ok/, {}, 'RT #41121 binmode(STDOUT,":encoding(utf8) does not crash');
use threads;
binmode(STDOUT,":encoding(utf8)");
threads->create(sub{});
print "ok\n";
CODE

# EOF
