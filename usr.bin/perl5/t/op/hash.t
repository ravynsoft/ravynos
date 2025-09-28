#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

# This will crash perl if it fails

use constant PVBM => 'foo';

my $dummy = index 'foo', PVBM;
eval { my %h = (a => PVBM); 1 };

ok (!$@, 'fbm scalar can be inserted into a hash');


my $destroyed;
{ package Class; DESTROY { ++$destroyed; } }

$destroyed = 0;
{
    my %h;
    keys(%h) = 1;
    $h{key} = bless({}, 'Class');
}
is($destroyed, 1, 'Timely hash destruction with lvalue keys');


# [perl #79178] Hash keys must not be stringified during compilation
# Run perl -MO=Concise -e '$a{\"foo"}' on a non-threaded pre-5.13.8 version
# to see why.
{
    my $key;
    package bar;
    sub TIEHASH { bless {}, $_[0] }
    sub FETCH { $key = $_[1] }
    package main;
    tie my %h, "bar";
    () = $h{\'foo'};
    is ref $key, SCALAR =>
     'ref hash keys are not stringified during compilation';
    use constant u => undef;
    no warnings 'uninitialized'; # work around unfixed bug #105918
    () = $h{+u};
    is $key, undef,
      'undef hash keys are not stringified during compilation, either';
}

# Part of RT #85026: Deleting the current iterator in void context does not
# free it.
{
    my $gone;
    no warnings 'once';
    local *::DESTROY = sub { ++$gone };
    my %a=(a=>bless[]);
    each %a;   # make the entry with the obj the current iterator
    delete $a{a};
    ok $gone, 'deleting the current iterator in void context frees the val'
}

# [perl #99660] Deleted hash element visible to destructor
{
    my %h;
    $h{k} = bless [];
    my $normal_exit;
    local *::DESTROY = sub { my $x = $h{k}; ++$normal_exit };
    delete $h{k}; # must be in void context to trigger the bug
    ok $normal_exit, 'freed hash elems are not visible to DESTROY';
}

# [perl #100340] Similar bug: freeing a hash elem during a delete
sub guard::DESTROY {
   ${$_[0]}->();
};
*guard = sub (&) {
   my $callback = shift;
   return bless \$callback, "guard"
};
{
  my $ok;
  my %t; %t = (
    stash => {
        guard => guard(sub{
            $ok++;
            delete $t{stash};
        }),
        foo => "bar",
        bar => "baz",
    },
  );
  ok eval { delete $t{stash}{guard}; # must be in void context
            1 },
    'freeing a hash elem from destructor called by delete does not die';
  diag $@ if $@; # panic: free from wrong pool
  is $ok, 1, 'the destructor was called';
}

# Weak references to pad hashes
{
    my $ref;
    no warnings 'experimental::builtin';
    use builtin 'weaken';
    {
        my %hash;
        weaken($ref = \%hash);
        1;  # the previous statement must not be the last
    }
    is $ref, undef, 'weak refs to pad hashes go stale on scope exit';
}

# [perl #107440]
sub A::DESTROY { $::ra = 0 }
$::ra = {a=>bless [], 'A'};
undef %$::ra;
pass 'no crash when freeing hash that is being undeffed';
$::ra = {a=>bless [], 'A'};
%$::ra = ('a'..'z');
pass 'no crash when freeing hash that is being exonerated, ahem, cleared';

# If I have these correct then removing any part of the lazy hash fill handling
# code in hv.c will cause some of these tests to start failing.
sub validate_hash {
  my ($desc, $h) = @_;
  local $::Level = $::Level + 1;

  # test that scalar(%hash) works as expected, which as of perl 5.25 is
  # the same as 0+keys %hash;
  my $scalar= scalar %$h;
  my $count= 0+keys %$h;

  is($scalar, $count, "$desc scalar() should be the same as 0+keys() as of perl 5.25");

  require Hash::Util;
  sub Hash::Util::bucket_ratio (\%);

  # back compat tests, via Hash::Util::bucket_ratio();
  my $ratio = Hash::Util::bucket_ratio(%$h);
  my $expect = qr!\A(\d+)/(\d+)\z!;
  like($ratio, $expect, "$desc bucket_ratio matches pattern");
  my ($used, $total)= (0,0);
  ($used, $total)= ($1,$2) if $ratio =~ /$expect/;
  cmp_ok($total, '>', 0, "$desc has >0 array size ($total)");
  cmp_ok($used, '>', 0, "$desc uses >0 heads ($used)");
  cmp_ok($used, '<=', $total,
         "$desc doesn't use more heads than are available");
  return ($used, $total);
}

sub torture_hash {
  my $desc = shift;
  # Intentionally use an anon hash rather than a lexical, as lexicals default
  # to getting reused on subsequent calls
  my $h = {};
  ++$h->{$_} foreach @_;

  my ($used0, $total0) = validate_hash($desc, $h);
  # Remove half the keys each time round, until there are only 1 or 2 left
  my @groups;
  my ($h2, $h3, $h4);
  while (keys %$h > 2) {
    my $take = (keys %$h) / 2 - 1;
    my @keys = (sort keys %$h)[0..$take];

    my $scalar = %$h;
    delete @$h{@keys};
    push @groups, $scalar, \@keys;

    my $count = keys %$h;
    my ($used, $total) = validate_hash("$desc (-$count)", $h);
    is($total, $total0, "$desc ($count) has same array size");
    cmp_ok($used, '<=', $used0, "$desc ($count) has same or fewer heads");
    ++$h2->{$_} foreach @keys;
    my (undef, $total2) = validate_hash("$desc (+$count)", $h2);
    cmp_ok($total2, '<=', $total0, "$desc ($count) array size no larger");

    # Each time this will get emptied then repopulated. If the fill isn't reset
    # when the hash is emptied, the used count will likely exceed the array
    %$h3 = %$h2;
    is(join(",", sort keys %$h3),join(",",sort keys %$h2),"$desc (+$count copy) has same keys");
    my (undef, $total3) = validate_hash("$desc (+$count copy)", $h3);
    # We now only split when we collide on insert AND exceed the load factor
    # when we did so. Building a hash via %x=%y means a pseudo-random key
    # order inserting into %x, and we may end up encountering a collision
    # at a different point in the load order, resulting in a possible power of
    # two difference under the current load factor expectations. If this test
    # fails then it is probably because DO_HSPLIT was changed, and this test
    # needs to be adjusted accordingly.
    ok( $total2 == $total3 || $total2*2==$total3 || $total2==$total3*2,
        "$desc (+$count copy) array size within a power of 2 of each other");

    # This might use fewer buckets than the original
    %$h4 = %$h;
    my (undef, $total4) = validate_hash("$desc ($count copy)", $h4);
    cmp_ok($total4, '<=', $total0, "$desc ($count copy) array size no larger");
  }

  my $scalar = %$h;
  my @keys = sort keys %$h;
  delete @$h{@keys};
  is(scalar %$h, 0, "scalar keys for empty $desc");

  # Rebuild the original hash, and build a copy
  # These will fail if hash key addition and deletion aren't handled correctly
  my $h1;
  foreach (@keys) {
    ++$h->{$_};
    ++$h1->{$_};
  }
  is(scalar %$h, $scalar, "scalar keys restored when rebuilding");

  while (@groups) {
    my $keys = pop @groups;
    ++$h->{$_} foreach @$keys;
    my (undef, $total) = validate_hash($desc, $h);
    ok($total == $total0 || $total == ($total0*2), "bucket count is expected size when rebuilding");
    is(scalar %$h, pop @groups, "scalar keys is identical when rebuilding");
    ++$h1->{$_} foreach @$keys;
    validate_hash("$desc copy", $h1);
  }
  # This will fail if the fill count isn't handled correctly on hash split
  is(scalar %$h1, scalar %$h, "scalar keys is identical on copy and original");
}

if (is_miniperl) {
    print "# skipping torture_hash tests on miniperl because no Hash::Util\n";
} else {
    torture_hash('a .. zz', 'a' .. 'zz');
    torture_hash('0 .. 9', 0 .. 9);
    torture_hash("'Perl'", 'Rules');
}

{
    my %h = qw(a x b y c z);
    no warnings qw(misc uninitialized);
    %h = $h{a};
    is(join(':', %h), 'x:', 'hash self-assign');
}

# magic keys and values should be evaluated before the hash on the LHS is
# cleared

package Magic {
    my %inner;
    sub TIEHASH { bless [] }
    sub FETCH { $inner{$_[1]} }
    sub STORE { $inner{$_[1]} = $_[2]; }
    sub CLEAR { %inner = () }

    my (%t1, %t2);
    tie %t1, 'Magic';
    tie %t2, 'Magic';

    %inner = qw(a x b y);
    %t1 = (@t2{'a','b'});
    ::is(join( ':', %inner), "x:y", "magic keys");
}



done_testing();
