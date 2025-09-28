#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

tie my $c => 'Tie::Monitor';

sub expected_tie_calls {
    my ($obj, $rexp, $wexp, $tn) = @_;
    local $::Level = $::Level + 1;
    my ($rgot, $wgot) = $obj->init();
    is ($rgot, $rexp, $tn ? "number of fetches when $tn" : ());
    is ($wgot, $wexp, $tn ? "number of stores when $tn" : ());
}

# Use ok() instead of is(), cmp_ok() etc, to strictly control number of accesses
my($r, $s);
ok($r = $c + 0 == 0, 'the thing itself');
expected_tie_calls(tied $c, 1, 0);
ok($r = "$c" eq '0', 'the thing itself');
expected_tie_calls(tied $c, 1, 0);

ok($c . 'x' eq '0x', 'concat');
expected_tie_calls(tied $c, 1, 0);
ok('x' . $c eq 'x0', 'concat');
expected_tie_calls(tied $c, 1, 0);
$s = $c . $c;
ok($s eq '00', 'concat');
expected_tie_calls(tied $c, 2, 0);
$r = 'x';
$s = $c = $r . 'y';
ok($s eq 'xy', 'concat');
expected_tie_calls(tied $c, 1, 1);
$s = $c = $c . 'x';
ok($s eq '0x', 'concat');
expected_tie_calls(tied $c, 2, 1);
$s = $c = 'x' . $c;
ok($s eq 'x0', 'concat');
expected_tie_calls(tied $c, 2, 1);
$s = $c = $c . $c;
ok($s eq '00', 'concat');
expected_tie_calls(tied $c, 3, 1);

$s = chop($c);
ok($s eq '0', 'multiple magic in core functions');
expected_tie_calls(tied $c, 1, 1);

$c = *strat;
$s = $c;
ok($s eq *strat,
   'Assignment should not ignore magic when the last thing assigned was a glob');
expected_tie_calls(tied $c, 1, 1);

package o { use overload '""' => sub { "foo\n" } }
$c = bless [], o::;
chomp $c;
expected_tie_calls(tied $c, 1, 2, 'chomping a ref');

{
    no warnings 'once'; # main::foo
    my $outfile = tempfile();
    open my $h, ">$outfile" or die  "$0 cannot close $outfile: $!";
    binmode $h;
    print $h "bar\n";
    close $h or die "$0 cannot close $outfile: $!";    

    $c = *foo;                                         # 1 write
    open $h, $outfile;
    binmode $h;
    sysread $h, $c, 3, 7;                              # 1 read; 1 write
    is $c, "*main::bar", 'what sysread wrote';         # 1 read
    expected_tie_calls(tied $c, 2, 2, 'calling sysread with tied buf');
    close $h or die "$0 cannot close $outfile: $!";

    unlink_all $outfile;
}

# autovivication of aelem, helem, of rv2sv combined with get-magic
{
    my $true = 1;
    my $s;
    tie $$s, "Tie::Monitor";
    $$s = undef;
    $$s->[0] = 73;
    is($$s->[0], 73);
    expected_tie_calls(tied $$s, 3, 2);

    my @a;
    tie $a[0], "Tie::Monitor";
    $a[0] = undef;
    $a[0][0] = 73;
    is($a[0][0], 73);
    expected_tie_calls(tied $a[0], 3, 2);

    my %h;
    tie $h{foo}, "Tie::Monitor";
    $h{foo} = undef;
    $h{foo}{bar} = 73;
    is($h{foo}{bar}, 73);
    expected_tie_calls(tied $h{foo}, 3, 2);

    # Similar tests, but with obscured autovivication by using dummy list or "?:" operator
    $$s = undef;
    ${ (), $$s }[0] = 73;
    is( $$s->[0], 73);
    expected_tie_calls(tied $$s, 3, 2);

    $$s = undef;
    ( ! $true ? undef : $$s )->[0] = 73;
    is( $$s->[0], 73);
    expected_tie_calls(tied $$s, 3, 2);

    $$s = undef;
    ( $true ? $$s : undef )->[0] = 73;
    is( $$s->[0], 73);
    expected_tie_calls(tied $$s, 3, 2);
}

# A plain *foo should not call get-magic on *foo.
# This method of scalar-tying an immutable glob relies on details of the
# current implementation that are subject to change. This test may need to
# be rewritten if they do change.
my $tyre = tie $::{gelp} => 'Tie::Monitor';
# Compilation of this eval autovivifies the *gelp glob.
eval '$tyre->init(0); () = \*gelp';
my($rgot, $wgot) = $tyre->init(0);
ok($rgot == 0, 'a plain *foo causes no get-magic');
ok($wgot == 0, 'a plain *foo causes no set-magic');

# get-magic when exiting a non-lvalue sub in potentially autovivify-
# ing context
{
  no strict;

  my $tied_to = tie $_{elem}, "Tie::Monitor";
  () = sub { delete $_{elem} }->()->[3];
  expected_tie_calls $tied_to, 1, 0,
     'mortal magic var is implicitly returned in autoviv context';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  () = sub { return delete $_{elem} }->()->[3];
  expected_tie_calls $tied_to, 1, 0,
      'mortal magic var is explicitly returned in autoviv context';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  my $rsub;
  $rsub = sub { if ($_[0]) { delete $_{elem} } else { &$rsub(1)->[3] } };
  &$rsub;
  expected_tie_calls $tied_to, 1, 0,
    'mortal magic var is implicitly returned in recursive autoviv context';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  $rsub = sub {
    if ($_[0]) { return delete $_{elem} } else { &$rsub(1)->[3] }
  };
  &$rsub;
  expected_tie_calls $tied_to, 1, 0,
    'mortal magic var is explicitly returned in recursive autoviv context';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  my $x = \sub { delete $_{elem} }->();
  expected_tie_calls $tied_to, 1, 0,
     'mortal magic var is implicitly returned to refgen';
  is tied $$x, undef,
     'mortal magic var is copied when implicitly returned';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  $x = \sub { return delete $_{elem} }->();
  expected_tie_calls $tied_to, 1, 0,
     'mortal magic var is explicitly returned to refgen';
  is tied $$x, undef,
     'mortal magic var is copied when explicitly returned';

  $tied_to = tie $_{elem}, "Tie::Monitor";
  $x = \do { 1; delete $_{elem} };
  expected_tie_calls $tied_to, 1, 0,
     'mortal magic var from do passed to refgen';
  is tied $$x, undef,
     'mortal magic var from do is copied';
}

# For better or worse, the order in which concat args are fetched varies
# depending on their number. In A .= B.C.D, they are fetched in the order
# BCDA, while for A .= B, the order is AB (so for a single concat, the LHS
# tied arg is FETCH()ed first). Make sure multiconcat preserves current
# behaviour.

package Increment {
    sub TIESCALAR {  bless [0, 0] }
    # returns a new value for each FETCH, until the first STORE
    sub FETCH { my $x = $_[0][0]; $_[0][0]++ unless $_[0][1]; $x }
    sub STORE { @{$_[0]} = ($_[1],1) }

    my $t;
    tie $t, 'Increment';
    my $r;
    $r = $t . $t;
    ::is $r, '01', 'Increment 01';
    $r = "-$t-$t-$t-";
    ::is $r, '-2-3-4-', 'Increment 234';
    $t .= "-$t-$t-$t-";
    ::is $t, '8-5-6-7-', 'Increment 8567';
}

done_testing();

# adapted from Tie::Counter by Abigail
package Tie::Monitor;

sub TIESCALAR {
    my($class, $value) = @_;
    bless {
	read => 0,
	write => 0,
	values => [ 0 ],
    };
}

sub FETCH {
    my $self = shift;
    ++$self->{read};
    $self->{values}[$#{ $self->{values} }];
}

sub STORE {
    my($self, $value) = @_;
    ++$self->{write};
    push @{ $self->{values} }, $value;
}

sub init {
    my $self = shift;
    my @results = ($self->{read}, $self->{write});
    $self->{read} = $self->{write} = 0;
    $self->{values} = [ 0 ];
    @results;
}
