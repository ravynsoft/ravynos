#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

BEGIN {
    # Do this as the very first thing, in order to avoid problems with the
    # PADTMP flag on pre-5.19.3 threaded Perls.  On those Perls, compiling
    # code that contains a constant-folded canonical truth value breaks
    # the ability to take a reference to that canonical truth value later.
    $::false = 0;
    %::immortals = (
	'u' => \undef,
	'y' => \!$::false,
	'n' => \!!$::false,
    );
}

sub BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 'dist/Storable' if -d 'dist/Storable';
        @INC = ('../../lib', 't');
    } else {
        unshift @INC, 't';
        unshift @INC, 't/compat' if $] < 5.006002;
    }
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Test::More;

use Storable qw(freeze thaw store retrieve fd_retrieve);

%::weird_refs = 
  (REF            => \(my $aref    = []),
   VSTRING        => \(my $vstring = v1.2.3),
   'long VSTRING' => \(my $lvstring = eval "v" . 0 x 300),
   LVALUE         => \(my $substr  = substr((my $str = "foo"), 0, 3)));

my $test = 18;
my $tests = $test + 41 + (2 * 6 * keys %::immortals) + (3 * keys %::weird_refs);
plan(tests => $tests);

package SHORT_NAME;

sub make { bless [], shift }

package SHORT_NAME_WITH_HOOK;

sub make { bless [], shift }

sub STORABLE_freeze {
	my $self = shift;
	return ("", $self);
}

sub STORABLE_thaw {
	my $self = shift;
	my $cloning = shift;
	my ($x, $obj) = @_;
	die "STORABLE_thaw" unless $obj eq $self;
}

package main;

# Still less than 256 bytes, so long classname logic not fully exercised
#   Identifier too long - 5.004
#   parser.h: char	tokenbuf[256]: cperl5.24 => 1024
my $m = ($Config{usecperl} and $] >= 5.024) ? 56 : 14;
my $longname = "LONG_NAME_" . ('xxxxxxxxxxxxx::' x $m) . "final";

eval <<EOC;
package $longname;

\@ISA = ("SHORT_NAME");
EOC
is($@, '');

eval <<EOC;
package ${longname}_WITH_HOOK;

\@ISA = ("SHORT_NAME_WITH_HOOK");
EOC
is($@, '');

# Construct a pool of objects
my @pool;
for (my $i = 0; $i < 10; $i++) {
    push(@pool, SHORT_NAME->make);
    push(@pool, SHORT_NAME_WITH_HOOK->make);
    push(@pool, $longname->make);
    push(@pool, "${longname}_WITH_HOOK"->make);
}

my $x = freeze \@pool;
pass("Freeze didn't crash");

my $y = thaw $x;
is(ref $y, 'ARRAY');
is(scalar @{$y}, @pool);

is(ref $y->[0], 'SHORT_NAME');
is(ref $y->[1], 'SHORT_NAME_WITH_HOOK');
is(ref $y->[2], $longname);
is(ref $y->[3], "${longname}_WITH_HOOK");

my $good = 1;
for (my $i = 0; $i < 10; $i++) {
    do { $good = 0; last } unless ref $y->[4*$i]   eq 'SHORT_NAME';
    do { $good = 0; last } unless ref $y->[4*$i+1] eq 'SHORT_NAME_WITH_HOOK';
    do { $good = 0; last } unless ref $y->[4*$i+2] eq $longname;
    do { $good = 0; last } unless ref $y->[4*$i+3] eq "${longname}_WITH_HOOK";
}
is($good, 1);

{
    my $blessed_ref = bless \\[1,2,3], 'Foobar';
    my $x = freeze $blessed_ref;
    my $y = thaw $x;
    is(ref $y, 'Foobar');
    is($$$y->[0], 1);
}

package RETURNS_IMMORTALS;

sub make { my $self = shift; bless [@_], $self }

sub STORABLE_freeze {
    # Some reference some number of times.
    my $self = shift;
    my ($what, $times) = @$self;
    return ("$what$times", ($::immortals{$what}) x $times);
}

sub STORABLE_thaw {
    my $self = shift;
    my $cloning = shift;
    my ($x, @refs) = @_;
    my ($what, $times) = $x =~ /(.)(\d+)/;
    die "'$x' didn't match" unless defined $times;
    main::is(scalar @refs, $times);
    my $expect = $::immortals{$what};
    die "'$x' did not give a reference" unless ref $expect;
    my $fail;
    foreach (@refs) {
        $fail++ if $_ != $expect;
    }
    main::is($fail, undef);
}

package main;

# XXX Failed tests:  15, 27, 39 with 5.12 and 5.10 threaded.
# 15: 1 fail (y x 1), 27: 2 fail (y x 2), 39: 3 fail (y x 3)
# $Storable::DEBUGME = 1;
my $count;
foreach $count (1..3) {
  my $immortal;
  foreach $immortal (keys %::immortals) {
    print "# $immortal x $count\n";
    my $i =  RETURNS_IMMORTALS->make ($immortal, $count);

    my $f = freeze ($i);
  TODO: {
      # ref sv_true is not always sv_true, at least in older threaded perls.
      local $TODO = "Some 5.10/12 do not preserve ref identity with freeze \\(1 == 1)"
        if !defined($f) and $] < 5.013 and $] > 5.009 and $immortal eq 'y';
      isnt($f, undef);
    }
    my $t = thaw $f;
    pass("thaw didn't crash");
  }
}

# Test automatic require of packages to find thaw hook.

package HAS_HOOK;

$loaded_count = 0;
$thawed_count = 0;

sub make {
  bless [];
}

sub STORABLE_freeze {
  my $self = shift;
  return '';
}

package main;

my $f = freeze (HAS_HOOK->make);

is($HAS_HOOK::loaded_count, 0);
is($HAS_HOOK::thawed_count, 0);

my $t = thaw $f;
is($HAS_HOOK::loaded_count, 1);
is($HAS_HOOK::thawed_count, 1);
isnt($t, undef);
is(ref $t, 'HAS_HOOK');

delete $INC{"HAS_HOOK.pm"};
delete $HAS_HOOK::{STORABLE_thaw};

$t = thaw $f;
is($HAS_HOOK::loaded_count, 2);
is($HAS_HOOK::thawed_count, 2);
isnt($t, undef);
is(ref $t, 'HAS_HOOK');

{
    package STRESS_THE_STACK;

    my $stress;
    sub make {
	bless [];
    }

    sub no_op {
	0;
    }

    sub STORABLE_freeze {
	my $self = shift;
	++$freeze_count;
	return no_op(1..(++$stress * 2000)) ? die "can't happen" : '';
    }

    sub STORABLE_thaw {
	my $self = shift;
	++$thaw_count;
	no_op(1..(++$stress * 2000)) && die "can't happen";
	return;
    }
}

$STRESS_THE_STACK::freeze_count = 0;
$STRESS_THE_STACK::thaw_count = 0;

$f = freeze (STRESS_THE_STACK->make);

is($STRESS_THE_STACK::freeze_count, 1);
is($STRESS_THE_STACK::thaw_count, 0);

$t = thaw $f;
is($STRESS_THE_STACK::freeze_count, 1);
is($STRESS_THE_STACK::thaw_count, 1);
isnt($t, undef);
is(ref $t, 'STRESS_THE_STACK');

my $file = "storable-testfile.$$";
die "Temporary file '$file' already exists" if -e $file;

END { while (-f $file) {unlink $file or die "Can't unlink '$file': $!" }}

$STRESS_THE_STACK::freeze_count = 0;
$STRESS_THE_STACK::thaw_count = 0;

store (STRESS_THE_STACK->make, $file);

is($STRESS_THE_STACK::freeze_count, 1);
is($STRESS_THE_STACK::thaw_count, 0);

$t = retrieve ($file);
is($STRESS_THE_STACK::freeze_count, 1);
is($STRESS_THE_STACK::thaw_count, 1);
isnt($t, undef);
is(ref $t, 'STRESS_THE_STACK');

{
    package ModifyARG112358;
    sub STORABLE_freeze { $_[0] = "foo"; }
    my $o= {str=>bless {}};
    my $f= ::freeze($o);
    ::is ref $o->{str}, __PACKAGE__,
	'assignment to $_[0] in STORABLE_freeze does not corrupt things';
}

# [perl #113880]
{
    {
        package WeirdRefHook;
        sub STORABLE_freeze { () }
        $INC{'WeirdRefHook.pm'} = __FILE__;
    }

    for my $weird (keys %weird_refs) {
        my $obj = $weird_refs{$weird};
        bless $obj, 'WeirdRefHook';
        my $frozen;
        my $success = eval { $frozen = freeze($obj); 1 };
        ok($success, "can freeze $weird objects")
            || diag("freezing failed: $@");
        my $thawn = thaw($frozen);
        # is_deeply ignores blessings
        is ref $thawn, ref $obj, "get the right blessing back for $weird";
        if ($weird =~ 'VSTRING') {
            # It is not just Storable that did not support vstrings. :-)
            # See https://rt.cpan.org/Ticket/Display.html?id=78678
            my $newver = "version"->can("new")
                           ? sub { "version"->new(shift) }
                           : sub { "" };
            if (!ok
                  $$thawn eq $$obj && &$newver($$thawn) eq &$newver($$obj),
                 "get the right value back"
            ) {
                diag "$$thawn vs $$obj";
                diag &$newver($$thawn) eq &$newver($$obj) if &$newver(1);
             }
        }
        else {
            is_deeply($thawn, $obj, "get the right value back");
        }
    }
}

{
    # [perl #118551]
    {
        package RT118551;

        sub new {
            my $class = shift;
            my $string = shift;
            die 'Bad data' unless defined $string;
            my $self = { string => $string };
            return bless $self, $class;
        }

        sub STORABLE_freeze {
            my $self = shift;
            my $cloning = shift;
            return if $cloning;
            return ($self->{string});
        }

        sub STORABLE_attach {
            my $class = shift;
            my $cloning = shift;
            my $string = shift;
            return $class->new($string);
        }
    }

    my $x = [ RT118551->new('a'), RT118551->new('') ];

    $y = freeze($x);

    ok(eval {thaw($y)}, "empty serialized") or diag $@; # <-- dies here with "Bad data"
}

{
    {
        package FreezeHookDies;
        sub STORABLE_freeze {
            die ${$_[0]}
        }

	package ThawHookDies;
	sub STORABLE_freeze {
	    my ($self, $cloning) = @_;
	    my $tmp = $$self;
	    return "a", \$tmp;
	}
	sub STORABLE_thaw {
	    my ($self, $cloning, $str, $obj) = @_;
	    die $$obj;
	}
    }
    my $x = bless \(my $tmpx = "Foo"), "FreezeHookDies";
    my $y = bless \(my $tmpy = []), "FreezeHookDies";

    ok(!eval { store($x, "store$$"); 1 }, "store of hook which throws no NL died");
    ok(!eval { store($y, "store$$"); 1 }, "store of hook which throws ref died");

    ok(!eval { freeze($x); 1 }, "freeze of hook which throws no NL died");
    ok(!eval { freeze($y); 1 }, "freeze of hook which throws ref died");

    ok(!eval { dclone($x); 1 }, "dclone of hook which throws no NL died");
    ok(!eval { dclone($y); 1 }, "dclone of hook which throws ref died");

    my $ostr = bless \(my $tmpstr = "Foo"), "ThawHookDies";
    my $oref = bless \(my $tmpref = []), "ThawHookDies";
    ok(store($ostr, "store$$"), "save throw Foo on thaw");
    ok(!eval { retrieve("store$$"); 1 }, "retrieve of throw Foo on thaw died");
    open FH, "<", "store$$" or die;
    binmode FH;
    ok(!eval { fd_retrieve(*FH); 1 }, "fd_retrieve of throw Foo on thaw died");
    ok(!ref $@, "right thing thrown");
    close FH;
    ok(store($oref, "store$$"), "save throw ref on thaw");
    ok(!eval { retrieve("store$$"); 1 }, "retrieve of throw ref on thaw died");
    open FH, "<", "store$$" or die;
    binmode FH;
    ok(!eval { fd_retrieve(*FH); 1 }, "fd_retrieve of throw [] on thaw died");
    ok(ref $@, "right thing thrown");
    close FH;

    my $strdata = freeze($ostr);
    ok(!eval { thaw($strdata); 1 }, "thaw of throw Foo on thaw died");
    ok(!ref $@, "and a string thrown");
    my $refdata = freeze($oref);
    ok(!eval { thaw($refdata); 1 }, "thaw of throw [] on thaw died");
    ok(ref $@, "and a ref thrown");

    unlink("store$$");
}

{
    # trying to freeze a glob via STORABLE_freeze
    {
        package GlobHookedBase;

        sub STORABLE_freeze {
            return \1;
        }

        package GlobHooked;
        our @ISA = "GlobHookedBase";
    }
    use Symbol ();
    my $glob = bless Symbol::gensym(), "GlobHooked";
    eval {
        my $data = freeze($glob);
    };
    my $msg = $@;
    like($msg, qr/Unexpected object type \(GLOB\) of class 'GlobHooked' in store_hook\(\) calling GlobHookedBase::STORABLE_freeze/,
         "check we get the verbose message");
}

SKIP:
{
    $] < 5.012
      and skip "Can't assign regexps directly before 5.12", 4;
    my $hook_called;
    # store regexp via hook
    {
        package RegexpHooked;
        sub STORABLE_freeze {
            ++$hook_called;
            "$_[0]";
        }
        sub STORABLE_thaw {
            my ($obj, $cloning, $serialized) = @_;
            ++$hook_called;
            $$obj = ${ qr/$serialized/ };
        }
    }

    my $obj = bless qr/abc/, "RegexpHooked";
    my $data = freeze($obj);
    ok($data, "froze regexp blessed into hooked class");
    ok($hook_called, "and the hook was actually called");
    $hook_called = 0;
    my $obj_thawed = thaw($data);
    ok($hook_called, "hook called for thaw");
    like("abc", $obj_thawed, "check the regexp");
}
