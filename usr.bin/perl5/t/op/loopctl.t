#!./perl

# We have the following types of loop:
#
# 1a)  while(A) {B}
# 1b)  B while A;
#
# 2a)  until(A) {B}
# 2b)  B until A;
#
# 3a)  for(@A)  {B}
# 3b)  B for A;
#
# 4a)  for (A;B;C) {D}
#
# 5a)  { A }        # a bare block is a loop which runs once
#
# Loops of type (b) don't allow for next/last/redo style
#  control, so we ignore them here. Type (a) loops can
#  all be labelled, so there are ten possibilities (each
#  of 5 types, labelled/unlabelled). We therefore need
#  thirty tests to try the three control statements against
#  the ten types of loop. For the first four types it's useful
#  to distinguish the case where next re-iterates from the case
#  where it leaves the loop. That makes 38.
# All these tests rely on "last LABEL"
#  so if they've *all* failed, maybe you broke that...
#
# These tests are followed by an extra test of nested loops.
# Feel free to add more here.
#
#  -- .robin. <robin@kitsite.com>  2001-03-13
BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib));
}

plan( tests => 67 );

my $ok;

TEST1: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  while($x--) {
    if (!$first_time) {
      $ok = 1;
      last TEST1;
    }
    $ok = 0;
    $first_time = 0;
    redo;
    last TEST1;
  }
  continue {
    $ok = 0;
    last TEST1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on while()');

TEST2: {

  $ok = 0;

  my $x = 2;
  my $first_time = 1;
  my $been_in_continue = 0;
  while($x--) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST2;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST2;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on while() successful next');

TEST3: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  while($x--) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST3;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST3;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'no label on while() unsuccessful next');

TEST4: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  while($x++) {
    if (!$first_time) {
      $ok = 0;
      last TEST4;
    }
    $ok = 0;
    $first_time = 0;
    last;
    last TEST4;
  }
  continue {
    $ok = 0;
    last TEST4;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'no label on while() last');

TEST5: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  until($x++) {
    if (!$first_time) {
      $ok = 1;
      last TEST5;
    }
    $ok = 0;
    $first_time = 0;
    redo;
    last TEST5;
  }
  continue {
    $ok = 0;
    last TEST5;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on until()');

TEST6: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  my $been_in_continue = 0;
  until($x++ >= 2) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST6;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST6;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on until() successful next');

TEST7: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  until($x++) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST7;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST7;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'no label on until() unsuccessful next');

TEST8: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  until($x++ == 10) {
    if (!$first_time) {
      $ok = 0;
      last TEST8;
    }
    $ok = 0;
    $first_time = 0;
    last;
    last TEST8;
  }
  continue {
    $ok = 0;
    last TEST8;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'no label on until() last');

TEST9: {

  $ok = 0;

  my $first_time = 1;
  for(1) {
    if (!$first_time) {
      $ok = 1;
      last TEST9;
    }
    $ok = 0;
    $first_time = 0;
    redo;
    last TEST9;
  }
  continue {
    $ok = 0;
    last TEST9;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on for(@array)');

TEST10: {

  $ok = 0;

  my $first_time = 1;
  my $been_in_continue = 0;
  for(1,2) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST10;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST10;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on for(@array) successful next');

TEST11: {

  $ok = 0;

  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  for(1) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST11;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST11;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'no label on for(@array) unsuccessful next');

TEST12: {

  $ok = 0;

  my $first_time = 1;
  for(1..10) {
    if (!$first_time) {
      $ok = 0;
      last TEST12;
    }
    $ok = 0;
    $first_time = 0;
    last;
    last TEST12;
  }
  continue {
    $ok=0;
    last TEST12;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'no label on for(@array) last');

TEST13: {

  $ok = 0;

  for(my $first_time = 1; 1;) {
    if (!$first_time) {
      $ok = 1;
      last TEST13;
    }
    $ok = 0;
    $first_time=0;

    redo;
    last TEST13;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on for(;;)');

TEST14: {

  $ok = 0;

  for(my $first_time = 1; 1; $first_time=0) {
    if (!$first_time) {
      $ok = 1;
      last TEST14;
    }
    $ok = 0;
    next;
    last TEST14;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on for(;;) successful next');

TEST15: {

  $ok = 0;

  my $x=1;
  my $been_in_loop = 0;
  for(my $first_time = 1; $x--;) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST15;
    }
    $ok = 0;
    $first_time = 0;
    next;
    last TEST15;
  }
  $ok = $been_in_loop;
}
cmp_ok($ok,'==',1,'no label on for(;;) unsuccessful next');

TEST16: {

  $ok = 0;

  for(my $first_time = 1; 1; last TEST16) {
    if (!$first_time) {
      $ok = 0;
      last TEST16;
    }
    $ok = 0;
    $first_time = 0;
    last;
    last TEST16;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'no label on for(;;) last');

TEST17: {

  $ok = 0;
  my $first_time = 1;

  {
    if (!$first_time) {
      $ok = 1;
      last TEST17;
    }
    $ok = 0;
    $first_time=0;

    redo;
    last TEST17;
  }
  continue {
    $ok = 0;
    last TEST17;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on bare block');

TEST18: {

  $ok = 0;
  {
    next;
    last TEST18;
  }
  continue {
    $ok = 1;
    last TEST18;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'no label on bare block next');

TEST19: {

  $ok = 0;
  {
    last;
    last TEST19;
  }
  continue {
    $ok = 0;
    last TEST19;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'no label on bare block last');

### Now do it all again with labels

TEST20: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  LABEL20: while($x--) {
    if (!$first_time) {
      $ok = 1;
      last TEST20;
    }
    $ok = 0;
    $first_time = 0;
    redo LABEL20;
    last TEST20;
  }
  continue {
    $ok = 0;
    last TEST20;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on while()');

TEST21: {

  $ok = 0;

  my $x = 2;
  my $first_time = 1;
  my $been_in_continue = 0;
  LABEL21: while($x--) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST21;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL21;
    last TEST21;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on while() successful next');

TEST22: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  LABEL22: while($x--) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST22;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL22;
    last TEST22;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'label on while() unsuccessful next');

TEST23: {

  $ok = 0;

  my $x = 1;
  my $first_time = 1;
  LABEL23: while($x++) {
    if (!$first_time) {
      $ok = 0;
      last TEST23;
    }
    $ok = 0;
    $first_time = 0;
    last LABEL23;
    last TEST23;
  }
  continue {
    $ok = 0;
    last TEST23;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'label on while() last');

TEST24: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  LABEL24: until($x++) {
    if (!$first_time) {
      $ok = 1;
      last TEST24;
    }
    $ok = 0;
    $first_time = 0;
    redo LABEL24;
    last TEST24;
  }
  continue {
    $ok = 0;
    last TEST24;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on until()');

TEST25: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  my $been_in_continue = 0;
  LABEL25: until($x++ >= 2) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST25;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL25;
    last TEST25;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on until() successful next');

TEST26: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  LABEL26: until($x++) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST26;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL26;
    last TEST26;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'label on until() unsuccessful next');

TEST27: {

  $ok = 0;

  my $x = 0;
  my $first_time = 1;
  LABEL27: until($x++ == 10) {
    if (!$first_time) {
      $ok = 0;
      last TEST27;
    }
    $ok = 0;
    $first_time = 0;
    last LABEL27;
    last TEST27;
  }
  continue {
    $ok = 0;
    last TEST8;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'label on until() last');

TEST28: {

  $ok = 0;

  my $first_time = 1;
  LABEL28: for(1) {
    if (!$first_time) {
      $ok = 1;
      last TEST28;
    }
    $ok = 0;
    $first_time = 0;
    redo LABEL28;
    last TEST28;
  }
  continue {
    $ok = 0;
    last TEST28;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on for(@array)');

TEST29: {

  $ok = 0;

  my $first_time = 1;
  my $been_in_continue = 0;
  LABEL29: for(1,2) {
    if (!$first_time) {
      $ok = $been_in_continue;
      last TEST29;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL29;
    last TEST29;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on for(@array) successful next');

TEST30: {

  $ok = 0;

  my $first_time = 1;
  my $been_in_loop = 0;
  my $been_in_continue = 0;
  LABEL30: for(1) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST30;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL30;
    last TEST30;
  }
  continue {
    $been_in_continue = 1;
  }
  $ok = $been_in_loop && $been_in_continue;
}
cmp_ok($ok,'==',1,'label on for(@array) unsuccessful next');

TEST31: {

  $ok = 0;

  my $first_time = 1;
  LABEL31: for(1..10) {
    if (!$first_time) {
      $ok = 0;
      last TEST31;
    }
    $ok = 0;
    $first_time = 0;
    last LABEL31;
    last TEST31;
  }
  continue {
    $ok=0;
    last TEST31;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'label on for(@array) last');

TEST32: {

  $ok = 0;

  LABEL32: for(my $first_time = 1; 1;) {
    if (!$first_time) {
      $ok = 1;
      last TEST32;
    }
    $ok = 0;
    $first_time=0;

    redo LABEL32;
    last TEST32;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on for(;;)');

TEST33: {

  $ok = 0;

  LABEL33: for(my $first_time = 1; 1; $first_time=0) {
    if (!$first_time) {
      $ok = 1;
      last TEST33;
    }
    $ok = 0;
    next LABEL33;
    last TEST33;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on for(;;) successful next');

TEST34: {

  $ok = 0;

  my $x=1;
  my $been_in_loop = 0;
  LABEL34: for(my $first_time = 1; $x--;) {
    $been_in_loop = 1;
    if (!$first_time) {
      $ok = 0;
      last TEST34;
    }
    $ok = 0;
    $first_time = 0;
    next LABEL34;
    last TEST34;
  }
  $ok = $been_in_loop;
}
cmp_ok($ok,'==',1,'label on for(;;) unsuccessful next');

TEST35: {

  $ok = 0;

  LABEL35: for(my $first_time = 1; 1; last TEST16) {
    if (!$first_time) {
      $ok = 0;
      last TEST35;
    }
    $ok = 0;
    $first_time = 0;
    last LABEL35;
    last TEST35;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'label on for(;;) last');

TEST36: {

  $ok = 0;
  my $first_time = 1;

  LABEL36: {
    if (!$first_time) {
      $ok = 1;
      last TEST36;
    }
    $ok = 0;
    $first_time=0;

    redo LABEL36;
    last TEST36;
  }
  continue {
    $ok = 0;
    last TEST36;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on bare block');

TEST37: {

  $ok = 0;
  LABEL37: {
    next LABEL37;
    last TEST37;
  }
  continue {
    $ok = 1;
    last TEST37;
  }
  $ok = 0;
}
cmp_ok($ok,'==',1,'label on bare block next');

TEST38: {

  $ok = 0;
  LABEL38: {
    last LABEL38;
    last TEST38;
  }
  continue {
    $ok = 0;
    last TEST38;
  }
  $ok = 1;
}
cmp_ok($ok,'==',1,'label on bare block last');

TEST39: {
    $ok = 0;
    my ($x, $y, $z) = (1,1,1);
    one39: while ($x--) {
      $ok = 0;
      two39: while ($y--) {
        $ok = 0;
        three39: while ($z--) {
           next two39;
        }
        continue {
          $ok = 0;
          last TEST39;
        }
      }
      continue {
        $ok = 1;
        last TEST39;
      }
      $ok = 0;
    }
}
cmp_ok($ok,'==',1,'nested constructs');

sub test_last_label { last TEST40 }

TEST40: {
    $ok = 1;
    test_last_label();
    $ok = 0;
}
cmp_ok($ok,'==',1,'dynamically scoped label');

sub test_last { last }

TEST41: {
    $ok = 1;
    test_last();
    $ok = 0;
}
cmp_ok($ok,'==',1,'dynamically scoped');


# [perl #27206] Memory leak in continue loop
# Ensure that the temporary object is freed each time round the loop,
# rather then all 10 of them all being freed right at the end

{
    my $n=10; my $late_free = 0;
    sub X::DESTROY { $late_free++ if $n < 0 };
    {
	($n-- && bless {}, 'X') && redo;
    }
    cmp_ok($late_free,'==',0,"bug 27206: redo memory leak");

    $n = 10; $late_free = 0;
    {
	($n-- && bless {}, 'X') && redo;
    }
    continue { }
    cmp_ok($late_free,'==',0,"bug 27206: redo with continue memory leak");
}

# ensure that redo doesn't clear a lexical declared in the condition

{
    my $i = 1;
    while (my $x = $i) {
	$i++;
	redo if $i == 2;
	cmp_ok($x,'==',1,"while/redo lexical life");
	last;
    }
    $i = 1;
    until (! (my $x = $i)) {
	$i++;
	redo if $i == 2;
	cmp_ok($x,'==',1,"until/redo lexical life");
	last;
    }
    for ($i = 1; my $x = $i; ) {
	$i++;
	redo if $i == 2;
	cmp_ok($x,'==',1,"for/redo lexical life");
	last;
    }

}

{
    $a37725[3] = 1; # use package var
    $i = 2;
    for my $x (reverse @a37725) {
	$x = $i++;
    }
    cmp_ok("@a37725",'eq',"5 4 3 2",'bug 37725: reverse with empty slots bug');
}

# [perl #21469] bad things happened with for $x (...) { *x = *y }

{
    my $i = 1;
    $x_21469  = 'X';
    $y1_21469 = 'Y1';
    $y2_21469 = 'Y2';
    $y3_21469 = 'Y3';
    for $x_21469 (1,2,3) {
	is($x_21469, $i, "bug 21469: correct at start of loop $i");
	*x_21469 = (*y1_21469, *y2_21469, *y3_21469)[$i-1];
	is($x_21469, "Y$i", "bug 21469: correct at tail of loop $i");
	$i++;
    }
    is($x_21469, 'X', "bug 21469: X okay at end of loop");
}

# [perl #112316] Wrong behavior regarding labels with same prefix
{
    my $fail;
    CATCH: {
    CATCHLOOP: {
            last CATCH;
        }
        $fail = 1;
    }
    ok(!$fail, "perl 112316: Labels with the same prefix don't get mixed up.");
}

# [perl #73618]
{
    sub foo_73618_0 {
        while (0) { }
    }
    sub bar_73618_0 {
        my $i = 0;
        while ($i) { }
    }
    sub foo_73618_undef {
        while (undef) { }
    }
    sub bar_73618_undef {
        my $i = undef;
        while ($i) { }
    }
    sub foo_73618_emptystring {
        while ("") { }
    }
    sub bar_73618_emptystring {
        my $i = "";
        while ($i) { }
    }
    sub foo_73618_0float {
        while (0.0) { }
    }
    sub bar_73618_0float {
        my $i = 0.0;
        while ($i) { }
    }
    sub foo_73618_0string {
        while ("0") { }
    }
    sub bar_73618_0string {
        my $i = "0";
        while ($i) { }
    }
    sub foo_73618_until {
        until (1) { }
    }
    sub bar_73618_until {
        my $i = 1;
        until ($i) { }
    }

    is(scalar(foo_73618_0()), scalar(bar_73618_0()),
       "constant optimization doesn't change return value");
    is(scalar(foo_73618_undef()), scalar(bar_73618_undef()),
       "constant optimization doesn't change return value");
    is(scalar(foo_73618_emptystring()), scalar(bar_73618_emptystring()),
       "constant optimization doesn't change return value");
    is(scalar(foo_73618_0float()), scalar(bar_73618_0float()),
       "constant optimization doesn't change return value");
    is(scalar(foo_73618_0string()), scalar(bar_73618_0string()),
       "constant optimization doesn't change return value");
    { local $TODO = "until is still wrongly optimized";
    is(scalar(foo_73618_until()), scalar(bar_73618_until()),
       "constant optimization doesn't change return value");
    }
}

# [perl #113684]
last_113684:
{
    label1:
    {
        my $label = "label1";
        eval { last $label };
        fail("last with non-constant label");
        last last_113684;
    }
    pass("last with non-constant label");
}
next_113684:
{
    label2:
    {
        my $label = "label2";
        eval { next $label };
        fail("next with non-constant label");
        next next_113684;
    }
    pass("next with non-constant label");
}
redo_113684:
{
    my $count;
    label3:
    {
        if ($count++) {
            pass("redo with non-constant label"); last redo_113684
        }
        my $label = "label3";
        eval { redo $label };
        fail("redo with non-constant label");
    }
}

# [perl #3112]
# The original report, which produced a Bizarre copy
@a  = ();
eval {
    for (1) {
        push @a, last;
    }
};
is @a, 0, 'push @a, last;  does not push';
is $@, "", 'no error, either';
# And my japh, which relied on the misbehaviour
is do{{&{sub{"Just another Perl hacker,\n"}},last}}, undef,
  'last returns nothing';
