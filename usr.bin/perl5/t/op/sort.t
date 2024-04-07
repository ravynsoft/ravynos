#!./perl
$|=1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use warnings;
plan(tests => 203);
use Tie::Array; # we need to test sorting tied arrays

# these shouldn't hang
{
    no warnings;
    sort { for ($_ = 0;; $_++) {} } @a;
    sort { while(1) {}            } @a;
    sort { while(1) { last; }     } @a;
    sort { while(0) { last; }     } @a;

    # Change 26011: Re: A surprising segfault
    map scalar(sort(+())), ('')x68;
}

sub Backwards { $a lt $b ? 1 : $a gt $b ? -1 : 0 }
sub Backwards_stacked($$) { my($a,$b) = @_; $a lt $b ? 1 : $a gt $b ? -1 : 0 }
sub Backwards_other { $a lt $b ? 1 : $a gt $b ? -1 : 0 }

my $upperfirst = 'A' lt 'a';

# Beware: in future this may become hairier because of possible
# collation complications: qw(A a B b) can be sorted at least as
# any of the following
#
#	A a B b
#	A B a b
#	a b A B
#	a A b B
#
# All the above orders make sense.
#
# That said, EBCDIC sorts all small letters first, as opposed
# to ASCII which sorts all big letters first.

@harry = ('dog','cat','x','Cain','Abel');
@george = ('gone','chased','yz','punished','Axed');

$x = join('', sort @harry);
$expected = $upperfirst ? 'AbelCaincatdogx' : 'catdogxAbelCain';

cmp_ok($x,'eq',$expected,'upper first 1');

$x = join('', sort( Backwards @harry));
$expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';

cmp_ok($x,'eq',$expected,'upper first 2');

$x = join('', sort( Backwards_stacked @harry));
$expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';

cmp_ok($x,'eq',$expected,'upper first 3');

$x = join('', sort @george, 'to', @harry);
$expected = $upperfirst ?
    'AbelAxedCaincatchaseddoggonepunishedtoxyz' :
    'catchaseddoggonepunishedtoxyzAbelAxedCain' ;

my @initially_sorted = ( 0 .. 260,
                         0x3FF, 0x400, 0x401,
                         0x7FF, 0x800, 0x801,
                         0x3FFF, 0x4000, 0x4001,
		         0xFFFF, 0x10000, 0x10001,
                       );
# It makes things easier below if there are an even number of elements in the
# array.
if (scalar(@initially_sorted) % 2 == 1) {
    push @initially_sorted, $initially_sorted[-1] + 1;
}

# We convert to a chr(), but prepend a constant string to make sure things can
# work on more than a single character.
my $prefix = "a\xb6";
my $prefix_len = length $prefix;

my @chr_initially_sorted = @initially_sorted;
$_ = $prefix . chr($_) for @chr_initially_sorted;

# Create a very unsorted version by reversing it, and then pushing the same
# code points again, but pair-wise reversed.
my @initially_unsorted = reverse @chr_initially_sorted;
for (my $i = 0; $i < @chr_initially_sorted - 1; $i += 2) {
    push @initially_unsorted, $chr_initially_sorted[$i+1],
                              $chr_initially_sorted[$i];
}

# And, an all-UTF-8 version
my @utf8_initialy_unsorted = @initially_unsorted;
utf8::upgrade($_) for @utf8_initialy_unsorted;

# Sort the non-UTF-8 version
my @non_utf8_result = sort @initially_unsorted;
my @wrongly_utf8;
my $ordered_correctly = 1;
for my $i (0 .. @chr_initially_sorted -1) {
    if (   $chr_initially_sorted[$i] ne $non_utf8_result[2*$i]
        || $chr_initially_sorted[$i] ne $non_utf8_result[2*$i+1])
    {
        $ordered_correctly = 0;
        last;
    }
    push @wrongly_utf8, $i if $i < 256 && utf8::is_utf8($non_utf8_result[$i]);
}
if (! ok($ordered_correctly, "sort of non-utf8 list worked")) {
    diag ("This should be in numeric order (with 2 instances of every code point):\n"
        . join " ", map { sprintf "%02x", ord substr $_, $prefix_len, 1 } @non_utf8_result);
}
if (! is(@wrongly_utf8, 0,
                      "No elements were wrongly converted to utf8 in sorting"))
{
    diag "For code points " . join " ", @wrongly_utf8;
}

# And then the UTF-8 one
my @wrongly_non_utf8;
$ordered_correctly = 1;
my @utf8_result = sort @utf8_initialy_unsorted;
for my $i (0 .. @chr_initially_sorted -1) {
    if (   $chr_initially_sorted[$i] ne $utf8_result[2*$i]
        || $chr_initially_sorted[$i] ne $utf8_result[2*$i+1])
    {
        $ordered_correctly = 0;
        last;
    }
    push @wrongly_non_utf8, $i unless utf8::is_utf8($utf8_result[$i]);
}
if (! ok($ordered_correctly, "sort of utf8 list worked")) {
    diag ("This should be in numeric order (with 2 instances of every code point):\n"
        . join " ", map { sprintf "%02x", ord substr $_, $prefix_len, 1 } @utf8_result);
}
if (! is(@wrongly_non_utf8, 0,
                      "No elements were wrongly converted from utf8 in sorting"))
{
    diag "For code points " . join " ", @wrongly_non_utf8;
}

cmp_ok($x,'eq',$expected,'upper first 4');
$" = ' ';
@a = ();
@b = reverse @a;
cmp_ok("@b",'eq',"",'reverse 1');

@a = (1);
@b = reverse @a;
cmp_ok("@b",'eq',"1",'reverse 2');

@a = (1,2);
@b = reverse @a;
cmp_ok("@b",'eq',"2 1",'reverse 3');

@a = (1,2,3);
@b = reverse @a;
cmp_ok("@b",'eq',"3 2 1",'reverse 4');

@a = (1,2,3,4);
@b = reverse @a;
cmp_ok("@b",'eq',"4 3 2 1",'reverse 5');

@a = (10,2,3,4);
@b = sort {$a <=> $b;} @a;
cmp_ok("@b",'eq',"2 3 4 10",'sort numeric');

$sub = 'Backwards';
$x = join('', sort $sub @harry);
$expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';

cmp_ok($x,'eq',$expected,'sorter sub name in var 1');

$sub = 'Backwards_stacked';
$x = join('', sort $sub @harry);
$expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';

cmp_ok($x,'eq',$expected,'sorter sub name in var 2');

# literals, combinations

@b = sort (4,1,3,2);
cmp_ok("@b",'eq','1 2 3 4','just sort');


@b = sort grep { $_ } (4,1,3,2);
cmp_ok("@b",'eq','1 2 3 4','grep then sort');


@b = sort map { $_ } (4,1,3,2);
cmp_ok("@b",'eq','1 2 3 4','map then sort');


@b = sort reverse (4,1,3,2);
cmp_ok("@b",'eq','1 2 3 4','reverse then sort');


@b = sort CORE::reverse (4,1,3,2);
cmp_ok("@b",'eq','1 2 3 4','CORE::reverse then sort');

eval  { @b = sort CORE::revers (4,1,3,2); };
like($@, qr/^Undefined sort subroutine "CORE::revers" called at /);


sub twoface { no warnings 'redefine'; *twoface = sub { $a <=> $b }; &twoface }
eval { @b = sort twoface 4,1,3,2 };
cmp_ok("@b",'eq','1 2 3 4','redefine sort sub inside the sort sub');


eval { no warnings 'redefine'; *twoface = sub { &Backwards } };
ok(!$@,"redefining sort subs outside the sort \$@=[$@]");

eval { @b = sort twoface 4,1,3,2 };
cmp_ok("@b",'eq','4 3 2 1','twoface redefinition');

{
  no warnings 'redefine';
  *twoface = sub { *twoface = *Backwards_other; $a <=> $b };
}

eval { @b = sort twoface 4,1,9,5 };
ok(($@ eq "" && "@b" eq "1 4 5 9"),'redefinition should not take effect during the sort');

{
  no warnings 'redefine';
  *twoface = sub {
                 eval 'sub twoface { $a <=> $b }';
		 die($@ eq "" ? "good\n" : "bad\n");
		 $a <=> $b;
	       };
}
eval { @b = sort twoface 4,1 };
cmp_ok(substr($@,0,4), 'eq', 'good', 'twoface eval');

eval <<'CODE';
    no warnings qw(deprecated syntax);
    my @result = sort main'Backwards 'one', 'two';
CODE
cmp_ok($@,'eq','',q(old skool package));

eval <<'CODE';
    # "sort 'one', 'two'" should not try to parse "'one" as a sort sub
    my @result = sort 'one', 'two';
CODE
cmp_ok($@,'eq','',q(one is not a sub));

{
  my $sortsub = \&Backwards;
  my $sortglob = *Backwards;
  my $sortglobr = \*Backwards;
  my $sortname = 'Backwards';
  @b = sort $sortsub 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 1');
  @b = sort $sortglob 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 2');
  @b = sort $sortname 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 3');
  @b = sort $sortglobr 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 4');
}

{
  my $sortsub = \&Backwards_stacked;
  my $sortglob = *Backwards_stacked;
  my $sortglobr = \*Backwards_stacked;
  my $sortname = 'Backwards_stacked';
  @b = sort $sortsub 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 5');
  @b = sort $sortglob 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 6');
  @b = sort $sortname 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 7');
  @b = sort $sortglobr 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname 8');
}

{
  local $sortsub = \&Backwards;
  local $sortglob = *Backwards;
  local $sortglobr = \*Backwards;
  local $sortname = 'Backwards';
  @b = sort $sortsub 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 1');
  @b = sort $sortglob 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 2');
  @b = sort $sortname 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 3');
  @b = sort $sortglobr 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 4');
}

{
  local $sortsub = \&Backwards_stacked;
  local $sortglob = *Backwards_stacked;
  local $sortglobr = \*Backwards_stacked;
  local $sortname = 'Backwards_stacked';
  @b = sort $sortsub 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 5');
  @b = sort $sortglob 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 6');
  @b = sort $sortname 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 7');
  @b = sort $sortglobr 4,1,3,2;
  cmp_ok("@b",'eq','4 3 2 1','sortname local 8');
}

## exercise sort builtins... ($a <=> $b already tested)
@a = ( 5, 19, 1996, 255, 90 );
@b = sort {
    my $dummy;		# force blockness
    return $b <=> $a
} @a;
cmp_ok("@b",'eq','1996 255 90 19 5','force blockness');

$x = join('', sort { $a cmp $b } @harry);
$expected = $upperfirst ? 'AbelCaincatdogx' : 'catdogxAbelCain';
cmp_ok($x,'eq',$expected,'a cmp b');

$x = join('', sort { $b cmp $a } @harry);
$expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';
cmp_ok($x,'eq',$expected,'b cmp a');

{
    use integer;
    @b = sort { $a <=> $b } @a;
    cmp_ok("@b",'eq','5 19 90 255 1996','integer a <=> b');

    @b = sort { $b <=> $a } @a;
    cmp_ok("@b",'eq','1996 255 90 19 5','integer b <=> a');

    $x = join('', sort { $a cmp $b } @harry);
    $expected = $upperfirst ? 'AbelCaincatdogx' : 'catdogxAbelCain';
    cmp_ok($x,'eq',$expected,'integer a cmp b');

    $x = join('', sort { $b cmp $a } @harry);
    $expected = $upperfirst ? 'xdogcatCainAbel' : 'CainAbelxdogcat';
    cmp_ok($x,'eq',$expected,'integer b cmp a');

}



$x = join('', sort { $a <=> $b } 3, 1, 2);
cmp_ok($x,'eq','123',q(optimized-away comparison block doesn't take any other arguments away with it));

# test sorting in non-main package
{
    package Foo;
    @a = ( 5, 19, 1996, 255, 90 );
    @b = sort { $b <=> $a } @a;
    ::cmp_ok("@b",'eq','1996 255 90 19 5','not in main:: 1');

    @b = sort ::Backwards_stacked @a;
    ::cmp_ok("@b",'eq','90 5 255 1996 19','not in main:: 2');

    # check if context for sort arguments is handled right
    sub test_if_list {
        my $gimme = wantarray;
        ::is($gimme,1,'wantarray 1');
    }
    my $m = sub { $a <=> $b };

    sub cxt_one { sort $m test_if_list() }
    cxt_one();
    sub cxt_two { sort { $a <=> $b } test_if_list() }
    cxt_two();
    sub cxt_three { sort &test_if_list() }
    cxt_three();
    sub cxt_three_anna_half { sort 0, test_if_list() }
    cxt_three_anna_half();

    sub test_if_scalar {
        my $gimme = wantarray;
        ::is(!($gimme or !defined($gimme)),1,'wantarray 2');
    }

    $m = \&test_if_scalar;
    sub cxt_four { sort $m 1,2 }
    @x = cxt_four();
    sub cxt_five { sort { test_if_scalar($a,$b); } 1,2 }
    @x = cxt_five();
    sub cxt_six { sort test_if_scalar 1,2 }
    @x = cxt_six();
}


# test against a reentrancy bug
{
    package Bar;
    sub compare { $a cmp $b }
    sub reenter { my @force = sort compare qw/a b/ }
}
{
    my($def, $init) = (0, 0);
    @b = sort {
	$def = 1 if defined $Bar::a;
	Bar::reenter() unless $init++;
	$a <=> $b
    } qw/4 3 1 2/;
    cmp_ok("@b",'eq','1 2 3 4','reenter 1');

    ok(!$def,'reenter 2');
}


{
    sub routine { "one", "two" };
    @a = sort(routine(1));
    cmp_ok("@a",'eq',"one two",'bug id 19991001.003 (#1549)');
}


# check for in-place optimisation of @a = sort @a
{
    my ($r1,$r2,@a);
    our @g;
    @g = (3,2,1); $r1 = \$g[2]; @g = sort @g; $r2 = \$g[0];
    is "$$r1-$$r2-@g", "1-1-1 2 3", "inplace sort of global";

    @a = qw(b a c); $r1 = \$a[1]; @a = sort @a; $r2 = \$a[0];
    is "$$r1-$$r2-@a", "a-a-a b c", "inplace sort of lexical";

    @g = (2,3,1); $r1 = \$g[1]; @g = sort { $b <=> $a } @g; $r2 = \$g[0];
    is "$$r1-$$r2-@g", "3-3-3 2 1", "inplace reversed sort of global";

    @g = (2,3,1);
    $r1 = \$g[1]; @g = sort { $a<$b?1:$a>$b?-1:0 } @g; $r2 = \$g[0];
    is "$$r1-$$r2-@g", "3-3-3 2 1", "inplace custom sort of global";

    sub mysort { $b cmp $a };
    @a = qw(b c a); $r1 = \$a[1]; @a = sort mysort @a; $r2 = \$a[0];
    is "$$r1-$$r2-@a", "c-c-c b a", "inplace sort with function of lexical";

    my @t;
    tie @t, 'Tie::StdArray';

    @t = qw(b c a); @t = sort @t;
    is "@t", "a b c", "inplace sort of tied array";

    @t = qw(b c a); @t = sort mysort @t;
    is "@t", "c b a", "inplace sort of tied array with function";

    #  [perl #29790] don't optimise @a = ('a', sort @a) !

    @g = (3,2,1); @g = ('0', sort @g);
    is "@g", "0 1 2 3", "un-inplace sort of global";
    @g = (3,2,1); @g = (sort(@g),'4');
    is "@g", "1 2 3 4", "un-inplace sort of global 2";

    @a = qw(b a c); @a = ('x', sort @a);
    is "@a", "x a b c", "un-inplace sort of lexical";
    @a = qw(b a c); @a = ((sort @a), 'x');
    is "@a", "a b c x", "un-inplace sort of lexical 2";

    @g = (2,3,1); @g = ('0', sort { $b <=> $a } @g);
    is "@g", "0 3 2 1", "un-inplace reversed sort of global";
    @g = (2,3,1); @g = ((sort { $b <=> $a } @g),'4');
    is "@g", "3 2 1 4", "un-inplace reversed sort of global 2";

    @g = (2,3,1); @g = ('0', sort { $a<$b?1:$a>$b?-1:0 } @g);
    is "@g", "0 3 2 1", "un-inplace custom sort of global";
    @g = (2,3,1); @g = ((sort { $a<$b?1:$a>$b?-1:0 } @g),'4');
    is "@g", "3 2 1 4", "un-inplace custom sort of global 2";

    @a = qw(b c a); @a = ('x', sort mysort @a);
    is "@a", "x c b a", "un-inplace sort with function of lexical";
    @a = qw(b c a); @a = ((sort mysort @a),'x');
    is "@a", "c b a x", "un-inplace sort with function of lexical 2";

    # RT#54758. Git 62b40d2474e7487e6909e1872b6bccdf812c6818
    no warnings 'void';
    my @m; push @m, 0 for 1 .. 1024; $#m; @m = sort @m;
    ::pass("in-place sorting segfault");

    # RT #39358 - array should be preserved during sort

    {
        my @aa = qw(b c a);
        my @copy;
        @aa = sort { @copy = @aa; $a cmp $b } @aa;
        is "@aa",   "a b c", "RT 39358 - aa";
        is "@copy", "b c a", "RT 39358 - copy";
    }

    # RT #128340: in-place sort incorrectly preserves element lvalue identity

    @a = (5, 4, 3);
    my $r = \$a[2];
    @a = sort { $a <=> $b } @a;
    $$r = "z";
    is ("@a", "3 4 5", "RT #128340");

}
{
    @Tied_Array_EXTEND_Test::ISA= 'Tie::StdArray';
    my $extend_count;
    sub Tied_Array_EXTEND_Test::EXTEND {
        $extend_count= $_[1];
        return;
    }
    my @t;
    tie @t, "Tied_Array_EXTEND_Test";
    is($extend_count, undef, "test that EXTEND has not been called prior to initialization");
    $t[0]=3;
    $t[1]=1;
    $t[2]=2;
    is($extend_count, undef, "test that EXTEND has not been called during initialization");
    @t= sort @t;
    is($extend_count, 3, "test that EXTEND was called with an argument of 3 by pp_sort()");
    is("@t","1 2 3","test that sorting the tied array worked even though EXTEND is a no-op");
}


# Test optimisations of reversed sorts. As we now guarantee stability by
# default, # optimisations which do not provide this are bogus.

{
    package Oscalar;
    use overload (qw("" stringify 0+ numify fallback 1));

    sub new {
	bless [$_[1], $_[2]], $_[0];
    }

    sub stringify { $_[0]->[0] }

    sub numify { $_[0]->[1] }
}

sub generate {
    my $count = 0;
    map {new Oscalar $_, $count++} qw(A A A B B B C C C);
}

my @input = &generate;
my @output = sort @input;
is join(" ", map {0+$_} @output), "0 1 2 3 4 5 6 7 8", "Simple stable sort";

@input = &generate;
@input = sort @input;
is join(" ", map {0+$_} @input), "0 1 2 3 4 5 6 7 8",
    "Simple stable in place sort";

# This won't be very interesting
@input = &generate;
@output = sort {$a <=> $b} @input;
is "@output", "A A A B B B C C C", 'stable $a <=> $b sort';

@input = &generate;
@output = sort {$a cmp $b} @input;
is join(" ", map {0+$_} @output), "0 1 2 3 4 5 6 7 8", 'stable $a cmp $b sort';

@input = &generate;
@input = sort {$a cmp $b} @input;
is join(" ", map {0+$_} @input), "0 1 2 3 4 5 6 7 8",
    'stable $a cmp $b in place sort';

@input = &generate;
@output = sort {$b cmp $a} @input;
is join(" ", map {0+$_} @output), "6 7 8 3 4 5 0 1 2", 'stable $b cmp $a sort';

@input = &generate;
@input = sort {$b cmp $a} @input;
is join(" ", map {0+$_} @input), "6 7 8 3 4 5 0 1 2",
    'stable $b cmp $a in place sort';

@input = &generate;
@output = reverse sort @input;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0", "Reversed stable sort";

@input = &generate;
@input = reverse sort @input;
is join(" ", map {0+$_} @input), "8 7 6 5 4 3 2 1 0",
    "Reversed stable in place sort";

@input = &generate;
my $output = reverse sort @input;
is $output, "CCCBBBAAA", "Reversed stable sort in scalar context";


@input = &generate;
@output = reverse sort {$a cmp $b} @input;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0",
    'reversed stable $a cmp $b sort';

@input = &generate;
@input = reverse sort {$a cmp $b} @input;
is join(" ", map {0+$_} @input), "8 7 6 5 4 3 2 1 0",
    'revesed stable $a cmp $b in place sort';

@input = &generate;
$output = reverse sort {$a cmp $b} @input;
is $output, "CCCBBBAAA", 'Reversed stable $a cmp $b sort in scalar context';

@input = &generate;
@output = reverse sort {$b cmp $a} @input;
is join(" ", map {0+$_} @output), "2 1 0 5 4 3 8 7 6",
    'reversed stable $b cmp $a sort';

@input = &generate;
@input = reverse sort {$b cmp $a} @input;
is join(" ", map {0+$_} @input), "2 1 0 5 4 3 8 7 6",
    'revesed stable $b cmp $a in place sort';

@input = &generate;
$output = reverse sort {$b cmp $a} @input;
is $output, "AAABBBCCC", 'Reversed stable $b cmp $a sort in scalar context';

sub stuff {
    # Something complex enough to defeat any constant folding optimiser
    $$ - $$;
}

@input = &generate;
@output = reverse sort {stuff || $a cmp $b} @input;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0",
    'reversed stable complex sort';

@input = &generate;
@input = reverse sort {stuff || $a cmp $b} @input;
is join(" ", map {0+$_} @input), "8 7 6 5 4 3 2 1 0",
    'revesed stable complex in place sort';

@input = &generate;
$output = reverse sort {stuff || $a cmp $b } @input;
is $output, "CCCBBBAAA", 'Reversed stable complex sort in scalar context';

sub sortr {
    reverse sort @_;
}

@output = sortr &generate;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0",
    'reversed stable sort return list context';
$output = sortr &generate;
is $output, "CCCBBBAAA",
    'reversed stable sort return scalar context';

sub sortcmpr {
    reverse sort {$a cmp $b} @_;
}

@output = sortcmpr &generate;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0",
    'reversed stable $a cmp $b sort return list context';
$output = sortcmpr &generate;
is $output, "CCCBBBAAA",
    'reversed stable $a cmp $b sort return scalar context';

sub sortcmprba {
    reverse sort {$b cmp $a} @_;
}

@output = sortcmprba &generate;
is join(" ", map {0+$_} @output), "2 1 0 5 4 3 8 7 6",
    'reversed stable $b cmp $a sort return list context';
$output = sortcmprba &generate;
is $output, "AAABBBCCC",
'reversed stable $b cmp $a sort return scalar context';

sub sortcmprq {
    reverse sort {stuff || $a cmp $b} @_;
}

@output = sortcmpr &generate;
is join(" ", map {0+$_} @output), "8 7 6 5 4 3 2 1 0",
    'reversed stable complex sort return list context';
$output = sortcmpr &generate;
is $output, "CCCBBBAAA",
    'reversed stable complex sort return scalar context';

# And now with numbers

sub generate1 {
    my $count = 'A';
    map {new Oscalar $count++, $_} 0, 0, 0, 1, 1, 1, 2, 2, 2;
}

# This won't be very interesting
@input = &generate1;
@output = sort {$a cmp $b} @input;
is "@output", "A B C D E F G H I", 'stable $a cmp $b sort';

@input = &generate1;
@output = sort {$a <=> $b} @input;
is "@output", "A B C D E F G H I", 'stable $a <=> $b sort';

@input = &generate1;
@input = sort {$a <=> $b} @input;
is "@input", "A B C D E F G H I", 'stable $a <=> $b in place sort';

@input = &generate1;
@output = sort {$b <=> $a} @input;
is "@output", "G H I D E F A B C", 'stable $b <=> $a sort';

@input = &generate1;
@input = sort {$b <=> $a} @input;
is "@input", "G H I D E F A B C", 'stable $b <=> $a in place sort';

# test that optimized {$b cmp $a} and {$b <=> $a} remain stable
# (new in 5.9) without overloading
{ no warnings;
@b = sort { $b <=> $a } @input = qw/5first 6first 5second 6second/;
is "@b" , "6first 6second 5first 5second", "optimized {$b <=> $a} without overloading" ;
@input = sort {$b <=> $a} @input;
is "@input" , "6first 6second 5first 5second","inline optimized {$b <=> $a} without overloading" ;
};

# These two are actually doing string cmp on 0 1 and 2
@input = &generate1;
@output = reverse sort @input;
is "@output", "I H G F E D C B A", "Reversed stable sort";

@input = &generate1;
@input = reverse sort @input;
is "@input", "I H G F E D C B A", "Reversed stable in place sort";

@input = &generate1;
$output = reverse sort @input;
is $output, "IHGFEDCBA", "Reversed stable sort in scalar context";

@input = &generate1;
@output = reverse sort {$a <=> $b} @input;
is "@output", "I H G F E D C B A", 'reversed stable $a <=> $b sort';

@input = &generate1;
@input = reverse sort {$a <=> $b} @input;
is "@input", "I H G F E D C B A", 'revesed stable $a <=> $b in place sort';

@input = &generate1;
$output = reverse sort {$a <=> $b} @input;
is $output, "IHGFEDCBA", 'reversed stable $a <=> $b sort in scalar context';

@input = &generate1;
@output = reverse sort {$b <=> $a} @input;
is "@output", "C B A F E D I H G", 'reversed stable $b <=> $a sort';

@input = &generate1;
@input = reverse sort {$b <=> $a} @input;
is "@input", "C B A F E D I H G", 'revesed stable $b <=> $a in place sort';

@input = &generate1;
$output = reverse sort {$b <=> $a} @input;
is $output, "CBAFEDIHG", 'reversed stable $b <=> $a sort in scalar context';

@input = &generate1;
@output = reverse sort {stuff || $a <=> $b} @input;
is "@output", "I H G F E D C B A", 'reversed stable complex sort';

@input = &generate1;
@input = reverse sort {stuff || $a <=> $b} @input;
is "@input", "I H G F E D C B A", 'revesed stable complex in place sort';

@input = &generate1;
$output = reverse sort {stuff || $a <=> $b} @input;
is $output, "IHGFEDCBA", 'reversed stable complex sort in scalar context';

sub sortnumr {
    reverse sort {$a <=> $b} @_;
}

@output = sortnumr &generate1;
is "@output", "I H G F E D C B A",
    'reversed stable $a <=> $b sort return list context';
$output = sortnumr &generate1;
is $output, "IHGFEDCBA", 'reversed stable $a <=> $b sort return scalar context';

sub sortnumrba {
    reverse sort {$b <=> $a} @_;
}

@output = sortnumrba &generate1;
is "@output", "C B A F E D I H G",
    'reversed stable $b <=> $a sort return list context';
$output = sortnumrba &generate1;
is $output, "CBAFEDIHG", 'reversed stable $b <=> $a sort return scalar context';

sub sortnumrq {
    reverse sort {stuff || $a <=> $b} @_;
}

@output = sortnumrq &generate1;
is "@output", "I H G F E D C B A",
    'reversed stable complex sort return list context';
$output = sortnumrq &generate1;
is $output, "IHGFEDCBA", 'reversed stable complex sort return scalar context';

@output = reverse (sort(qw(C A B)), 0);
is "@output", "0 C B A", 'reversed sort with trailing argument';

@output = reverse (0, sort(qw(C A B)));
is "@output", "C B A 0", 'reversed sort with leading argument';

eval { @output = sort {goto sub {}} 1,2; };
$fail_msg = q(Can't goto subroutine outside a subroutine);
cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'goto subr outside subr');



sub goto_sub {goto sub{}}
eval { @output = sort goto_sub 1,2; };
$fail_msg = q(Can't goto subroutine from a sort sub);
cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'goto subr from a sort sub');



eval { @output = sort {goto label} 1,2; };
$fail_msg = q(Can't "goto" out of a pseudo block);
cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'goto out of a pseudo block 1');



sub goto_label {goto label}
label: eval { @output = sort goto_label 1,2; };
$fail_msg = q(Can't "goto" out of a pseudo block);
cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'goto out of a pseudo block 2');



sub self_immolate {undef &self_immolate; $a<=>$b}
eval { @output = sort self_immolate 1,2,3 };
$fail_msg = q(Can't undef active subroutine);
cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'undef active subr');


for(1,2) # We run this twice, to make sure sort does not lower the ref
{        # count. See bug 71076.
    my $failed = 0;

    sub rec {
	my $n = shift;
	if (!defined($n)) {  # No arg means we're being called by sort()
	    return 1;
	}
	if ($n<5) { rec($n+1); }
	else { () = sort rec 1,2; }

	$failed = 1 if !defined $n;
    }

    rec(1);
    ok(!$failed, "sort from active sub");
}

# $a and $b are set in the package the sort() is called from,
# *not* the package the sort sub is in. This is longstanding
# de facto behaviour that shouldn't be broken.
my $answer = "good";
() = sort OtherPack::foo 1,2,3,4;

{
    package OtherPack;
    no warnings 'once';
    sub foo {
	$answer = "something was unexpectedly defined or undefined" if
	defined($a) || defined($b) || !defined($main::a) || !defined($main::b);
	$main::a <=> $main::b;
    }
}

cmp_ok($answer,'eq','good','sort subr called from other package');


# Bug 36430 - sort called in package2 while a
# sort in package1 is active should set $package2::a/b.
{
    my $answer = "good";
    my @list = sort { A::min(@$a) <=> A::min(@$b) }
      [3, 1, 5], [2, 4], [0];

    cmp_ok($answer,'eq','good','bug 36430');

    package A;
    sub min {
        my @list = sort {
            $answer = '$a and/or $b are not defined ' if !defined($a) || !defined($b);
            $a <=> $b;
        } @_;
        $list[0];
    }
}



# I commented out this TODO test because messing with FREEd scalars on the
# stack can have all sorts of strange side-effects, not made safe by eval
# - DAPM.
#
#{
#    local $TODO = "sort should make sure elements are not freed in the sort block";
#    eval { @nomodify_x=(1..8);
#	   our @copy = sort { undef @nomodify_x; 1 } (@nomodify_x, 3); };
#    is($@, "");
#}


# Sorting shouldn't increase the refcount of a sub
{
    sub sportello {(1+$a) <=> (1+$b)}
    # + 1 to account for prototype-defeating &... calling convention
    my $refcnt = &Internals::SvREFCNT(\&sportello) + 1;
    @output = sort sportello 3,7,9;

    {
        package Doc;
        ::refcount_is \&::sportello, $refcnt, "sort sub refcnt";
        $fail_msg = q(Modification of a read-only value attempted);
        # Sorting a read-only array in-place shouldn't be allowed
        my @readonly = (1..10);
        Internals::SvREADONLY(@readonly, 1);
        eval { @readonly = sort @readonly; };
        ::cmp_ok(substr($@,0,length($fail_msg)),'eq',$fail_msg,'in-place sort of read-only array');
    }
}


# Using return() should be okay even in a deeper context
@b = sort {while (1) {return ($a <=> $b)} } 1..10;
is("@b", "1 2 3 4 5 6 7 8 9 10", "return within loop");

# Using return() should be okay even if there are other items
# on the stack at the time.
@b = sort {$_ = ($a<=>$b) + do{return $b<=> $a}} 1..10;
is("@b", "10 9 8 7 6 5 4 3 2 1", "return with SVs on stack");

# As above, but with a sort sub rather than a sort block.
sub ret_with_stacked { $_ = ($a<=>$b) + do {return $b <=> $a} }
@b = sort ret_with_stacked 1..10;
is("@b", "10 9 8 7 6 5 4 3 2 1", "return with SVs on stack");

# Comparison code should be able to give result in non-integer representation.
sub cmp_as_string($$) { $_[0] < $_[1] ? "-1" : $_[0] == $_[1] ? "0" : "+1" }
@b = sort { cmp_as_string($a, $b) } (1,5,4,7,3,2,3);
is("@b", "1 2 3 3 4 5 7", "comparison result as string");
@b = sort cmp_as_string (1,5,4,7,3,2,3);
is("@b", "1 2 3 3 4 5 7", "comparison result as string");

# RT #34604: sort didn't honour overloading if the overloaded elements
# were retrieved via tie

{
    package RT34604;

    sub TIEHASH { bless {
			p => bless({ val => 2 }),
			q => bless({ val => 1 }),
		    }
		}
    sub FETCH { $_[0]{$_[1] } }

    my $cc = 0;
    sub compare { $cc++; $_[0]{val} cmp $_[1]{val} }
    my $cs = 0;
    sub str { $cs++; $_[0]{val} }

    use overload 'cmp' => \&compare, '""' => \&str;

    package main;

    tie my %h, 'RT34604';
    my @sorted = sort @h{qw(p q)};
    is($cc, 1, 'overload compare called once');
    is("@sorted","1 2", 'overload sort result');
    is($cs, 2, 'overload string called twice');
}

fresh_perl_is('sub w ($$) {my ($l, $r) = @_; my $v = \@_; undef @_; $l <=> $r}; print join q{ }, sort w 3, 1, 2, 0',
             '0 1 2 3',
             {stderr => 1, switches => ['-w']},
             'RT #72334');

fresh_perl_is('sub w ($$) {my ($l, $r) = @_; my $v = \@_; undef @_; @_ = 0..2; $l <=> $r}; print join q{ }, sort w 3, 1, 2, 0',
             '0 1 2 3',
             {stderr => 1, switches => ['-w']},
             'RT #72334');

{
    my $count = 0;
    {
	package Counter;

	sub new {
	    ++$count;
	    bless [];
	}

	sub DESTROY {
	    --$count;
	}
    }

    sub sorter ($$) {
	my ($l, $r) = @_;
	my $q = \@_;
	$l <=> $r;
    }

    is($count, 0, 'None before we start');
    my @a = map { Counter->new() } 0..1;
    is($count, 2, '2 here');

    my @b = sort sorter @a;

    is(scalar @b, 2);
    cmp_ok($b[0], '<', $b[1], 'sorted!');

    is($count, 2, 'still the same 2 here');

    @a = (); @b = ();

    is($count, 0, 'all gone');
}

# [perl #77930] The context stack may be reallocated during a sort, as a
#               result of deeply-nested (or not-so-deeply-nested) calls
#               from a custom sort subroutine.
fresh_perl_is
 '
   $sub = sub {
    local $count = $count+1;
    ()->$sub if $count < 1000;
    $a cmp $b
   };
   () = sort $sub qw<a b c d e f g>;
   print "ok"
 ',
 'ok',
  {},
 '[perl #77930] cx_stack reallocation during sort'
;

# [perl #76026]
# Match vars should not leak from one sort sub call to the next
{
  my $output = '';
  sub soarter {
    $output .= $1;
    "Leakage" =~ /(.*)/;
    1
  }
  sub soarterdd($$) {
    $output .= $1;
    "Leakage" =~ /(.*)/;
    1
  }

  "Win" =~ /(.*)/;
  my @b = sort soarter 0..2;

  like $output, qr/^(?:Win)+\z/,
   "Match vars do not leak from one plain sort sub to the next";

  $output = '';

  "Win" =~ /(.*)/;
  @b = sort soarterdd 0..2;

  like $output, qr/^(?:Win)+\z/,
   'Match vars do not leak from one $$ sort sub to the next';
}

# [perl #30661] autoloading
AUTOLOAD { $b <=> $a }
sub stubbedsub;
is join("", sort stubbedsub split//, '04381091'), '98431100',
    'stubborn AUTOLOAD';
is join("", sort hopefullynonexistent split//, '04381091'), '98431100',
    'AUTOLOAD without stub';
my $stubref = \&givemeastub;
is join("", sort $stubref split//, '04381091'), '98431100',
    'AUTOLOAD with stubref';


# this happened while the padrange op was being added. Sort blocks
# are executed in void context, and the padrange op was skipping pushing
# the item in void cx. The net result was that the return value was
# whatever was on the stack last.

{
    my @a = sort {
	my $r = $a <=> $b;
	if ($r) {
	    undef; # this got returned by mistake
	    return $r
	}
	return 0;
    } 5,1,3,6,0;
    is "@a", "0 1 3 5 6", "padrange and void context";
}

# Fatal warnings an sort sub returning a non-number
# We need two evals, because the panic used to happen on scope exit.
eval { eval { use warnings FATAL => 'all'; () = sort { undef } 1,2 } };
is $@, "",
  'no panic/crash with fatal warnings when sort sub returns undef';
eval { eval { use warnings FATAL => 'all'; () = sort { "no thin" } 1,2 } };
is $@, "",
  'no panic/crash with fatal warnings when sort sub returns string';
sub notdef($$) { undef }
eval { eval { use warnings FATAL => 'all'; () = sort notdef 1,2 } };
is $@, "",
  'no panic/crash with fatal warnings when sort sub($$) returns undef';
sub yarn($$) { "no thinking aloud" }
eval { eval { use warnings FATAL => 'all'; () = sort yarn 1,2 } };
is $@, "",
  'no panic/crash with fatal warnings when sort sub($$) returns string';

$#a = -1;
() = [sort { $a = 10; $b = 10; 0 } $#a, $#a];
is $#a, 10, 'sort block modifying $a and $b';

() = sort {
    is \$a, \$a, '[perl #78194] op return values passed to sort'; 0
} "${\''}", "${\''}";

package deletions {
    @_=sort { delete $deletions::{a}; delete $deletions::{b}; 3 } 1..3;
}
pass "no crash when sort block deletes *a and *b";

# make sure return args are always evaluated in scalar context

{
    package Ret;
    no warnings 'void';
    sub f0 { }
    sub f1 { $b <=> $a, $a <=> $b }
    sub f2 { return ($b <=> $a, $a <=> $b) }
    sub f3 { for ($b <=> $a) { return ($b <=> $a, $a <=> $b) } }

    {
        no warnings 'uninitialized';
        ::is (join('-', sort { () } 3,1,2,4), '3-1-2-4', "Ret: null blk");
    }
    ::is (join('-', sort { $b <=> $a, $a <=> $b } 3,1,2,4), '1-2-3-4', "Ret: blk");
    ::is (join('-', sort { for($b <=> $a) { return ($b <=> $a, $a <=> $b) } }
                            3,1,2,4), '1-2-3-4', "Ret: blk ret");
    {
        no warnings 'uninitialized';
        ::is (join('-', sort f0 3,1,2,4), '3-1-2-4', "Ret: f0");
    }
    ::is (join('-', sort f1 3,1,2,4), '1-2-3-4', "Ret: f1");
    ::is (join('-', sort f2 3,1,2,4), '1-2-3-4', "Ret: f2");
    ::is (join('-', sort f3 3,1,2,4), '1-2-3-4', "Ret: f3");
}

{
    @a = sort{ *a=0; 1} 0..1;
    pass "No crash when GP deleted out from under us [perl 124097]";

    no warnings 'redefine';
    # some alternative non-solutions localized modifications to *a and *b
    sub a { 0 };
    @a = sort { *a = sub { 1 }; $a <=> $b } 0 .. 1;
    ok(a(), "*a wasn't localized inadvertantly");
}

SKIP:
{
    eval { require Config; 1 }
      or skip "Cannot load Config", 1;
    $Config::Config{ivsize} == 8
      or skip "this test can only fail with 64-bit integers", 1;
    # sort's built-in numeric comparison wasn't careful enough in a world
    # of integers with more significant digits than NVs
    my @in = ( "0", "20000000000000001", "20000000000000000" );
    my @out = sort { $a <=> $b } @in;
    is($out[1], "20000000000000000", "check sort order");
}

# [perl #92264] refcounting of GvSV slot of *a and *b
{
    my $act;
    package ReportDestruction {
	sub new { bless({ p => $_[1] }, $_[0]) }
	sub DESTROY { $act .= $_[0]->{p}; }
    }
    $act = "";
    my $filla = \(ReportDestruction->new("[filla]"));
    () = sort { my $r = $a cmp $b; $act .= "0"; *a = \$$filla; $act .= "1"; $r }
	    ReportDestruction->new("[sorta]"), "foo";
    $act .= "2";
    $filla = undef;
    is $act, "01[sorta]2[filla]";
    $act = "";
    my $fillb = \(ReportDestruction->new("[fillb]"));
    () = sort { my $r = $a cmp $b; $act .= "0"; *b = \$$fillb; $act .= "1"; $r }
	    "foo", ReportDestruction->new("[sortb]");
    $act .= "2";
    $fillb = undef;
    is $act, "01[sortb]2[fillb]";
}

# GH #18081
# sub call via return in sort block was called in void rather than scalar
# context

{
    sub sort18081 { $a + 1 <=> $b + 1 }
    my @a = sort { return &sort18081 } 6,1,2;
    is "@a", "1 2 6", "GH #18081";
}

# make a physically empty sort a compile-time error
# Note that it was a wierd compile time error until
# [perl #90030], v5.15.6-390-ga46b39a853
# which made it a NOOP.
# Then in Jan 2022 it was made an error again, to allow future
# use of attribuute-like syntax, e.g.
#    @a = $cond ? sort :num 1,2,3 : ....;
# See http://nntp.perl.org/group/perl.perl5.porters/262425

{
    my @empty = ();
    my @sorted = sort @empty;
    is "@sorted", "", 'sort @empty';

    eval 'my @s = sort';
    like($@, qr/Not enough arguments for sort/, 'empty sort not allowed');

    eval '{my @s = sort}';
    like($@, qr/Not enough arguments for sort/, 'empty {sort} not allowed');

    eval 'my @s = sort; 1';
    like($@, qr/Not enough arguments for sort/, 'empty sort; not allowed');

    eval 'my @s = (sort); 1';
    like($@, qr/Not enough arguments for sort/, 'empty (sort); not allowed');
}
