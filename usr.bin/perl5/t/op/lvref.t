#!perl
BEGIN {
    chdir 't';
    require './test.pl';
    set_up_inc("../lib");
}

plan 167;

eval '\$x = \$y';
like $@, qr/^Experimental aliasing via reference not enabled/,
    'error when feature is disabled';
eval '\($x) = \$y';
like $@, qr/^Experimental aliasing via reference not enabled/,
    'error when feature is disabled (aassign)';

use feature 'refaliasing', 'state';

{
    my($w,$c);
    local $SIG{__WARN__} = sub { $c++; $w = shift };
    eval '\$x = \$y';
    is $c, 1, 'one warning from lv ref assignment';
    like $w, qr/^Aliasing via reference is experimental/,
        'experimental warning';
    undef $c;
    eval '\($x) = \$y';
    is $c, 1, 'one warning from lv ref list assignment';
    like $w, qr/^Aliasing via reference is experimental/,
        'experimental warning';
}

no warnings 'experimental::refaliasing';

# Scalars

\$x = \$y;
is \$x, \$y, '\$pkg_scalar = ...';
my $m;
\$m = \$y;
is \$m, \$y, '\$lexical = ...';
\my $n = \$y;
is \$n, \$y, '\my $lexical = ...';
@_ = \$_;
\($x) = @_;
is \$x, \$_, '\($pkgvar) = ... gives list context';
undef *x;
(\$x) = @_;
is \$x, \$_, '(\$pkgvar) = ... gives list context';
my $o;
\($o) = @_;
is \$o, \$_, '\($lexical) = ... gives list cx';
my $q;
(\$q) = @_;
is \$q, \$_, '(\$lexical) = ... gives list cx';
\(my $p) = @_;
is \$p, \$_, '\(my $lexical) = ... gives list cx';
(\my $r) = @_;
is \$r, \$_, '(\my $lexical) = ... gives list cx';
\my($s) = @_;
is \$s, \$_, '\my($lexical) = ... gives list cx';
\($_a, my $a) = @{[\$b, \$c]};
is \$_a, \$b, 'package scalar in \(...)';
is \$a, \$c, 'lex scalar in \(...)';
(\$_b, \my $b) = @{[\$b, \$c]};
is \$_b, \$::b, 'package scalar in (\$foo, \$bar)';
is \$b, \$c, 'lex scalar in (\$foo, \$bar)';
is do { \local $l = \3; $l }, 3, '\local $scalar assignment';
is $l, undef, 'localisation unwound';
is do { \(local $l) = \4; $l }, 4, '\(local $scalar) assignment';
is $l, undef, 'localisation unwound';
\$foo = \*bar;
is *foo{SCALAR}, *bar{GLOB}, 'globref-to-scalarref assignment';
for (1,2) {
  \my $x = \3,
  \my($y) = \3,
  \state $a = \3,
  \state($b) = \3 if $_ == 1;
  if ($_ == 2) {
    is $x, undef, '\my $x = ... clears $x on scope exit';
    is $y, undef, '\my($x) = ... clears $x on scope exit';
    is $a, 3, '\state $x = ... does not clear $x on scope exit';
    is $b, 3, '\state($x) = ... does not clear $x on scope exit';
  }
}

# Array Elements

sub expect_scalar_cx { wantarray ? 0 : \$_ }
sub expect_list_cx { wantarray ? (\$_,\$_) : 0 }
\$a[0] = expect_scalar_cx;
is \$a[0], \$_, '\$array[0]';
\($a[1]) = expect_list_cx;
is \$a[1], \$_, '\($array[0])';
{
  my @a;
  \$a[0] = expect_scalar_cx;
  is \$a[0], \$_, '\$lexical_array[0]';
  \($a[1]) = expect_list_cx;
  is \$a[1], \$_, '\($lexical_array[0])';
  my $tmp;
  {
    \local $a[0] = \$tmp;
    is \$a[0], \$tmp, '\local $a[0]';
  }
  is \$a[0], \$_, '\local $a[0] unwound';
  {
    \local ($a[1]) = \$tmp;
    is \$a[1], \$tmp, '\local ($a[0])';
  }
  is \$a[1], \$_, '\local $a[0] unwound';
}
{
  my @a;
  \@a[0,1] = expect_list_cx;
  is \$a[0].\$a[1], \$_.\$_, '\@array[indices]';
  \(@a[2,3]) = expect_list_cx;
  is \$a[0].\$a[1], \$_.\$_, '\(@array[indices])';
  my $tmp;
  {
    \local @a[0,1] = (\$tmp)x2;
    is \$a[0].\$a[1], \$tmp.\$tmp, '\local @a[indices]';
  }
  is \$a[0].\$a[1], \$_.\$_, '\local @a[indices] unwound';
}

# Hash Elements

\$h{a} = expect_scalar_cx;
is \$h{a}, \$_, '\$hash{a}';
\($h{b}) = expect_list_cx;
is \$h{b}, \$_, '\($hash{a})';
{
  my %h;
  \$h{a} = expect_scalar_cx;
  is \$h{a}, \$_, '\$lexical_array{a}';
  \($h{b}) = expect_list_cx;
  is \$h{b}, \$_, '\($lexical_array{a})';
  my $tmp;
  {
    \local $h{a} = \$tmp;
    is \$h{a}, \$tmp, '\local $h{a}';
  }
  is \$h{a}, \$_, '\local $h{a} unwound';
  {
    \local ($h{b}) = \$tmp;
    is \$h{b}, \$tmp, '\local ($h{a})';
  }
  is \$h{b}, \$_, '\local $h{a} unwound';
}
{
  my %h;
  \@h{"a","b"} = expect_list_cx;
  is \$h{a}.\$h{b}, \$_.\$_, '\@hash{indices}';
  \(@h{2,3}) = expect_list_cx;
  is \$h{a}.\$h{b}, \$_.\$_, '\(@hash{indices})';
  my $tmp;
  {
    \local @h{"a","b"} = (\$tmp)x2;
    is \$h{a}.\$h{b}, \$tmp.\$tmp, '\local @h{indices}';
  }
  is \$h{a}.\$h{b}, \$_.\$_, '\local @h{indices} unwound';
}

# Arrays

package ArrayTest {
  BEGIN { *is = *main::is }
  sub expect_scalar_cx { wantarray ? 0 : \@ThatArray }
  sub expect_list_cx   { wantarray ? (\$_,\$_) : 0 }
  sub expect_list_cx_a { wantarray ? (\@ThatArray)x2 : 0 }
  \@a = expect_scalar_cx;
  is \@a, \@ThatArray, '\@pkg';
  my @a;
  \@a = expect_scalar_cx;
  is \@a, \@ThatArray, '\@lexical';
  (\@b) = expect_list_cx_a;
  is \@b, \@ThatArray, '(\@pkg)';
  my @b;
  (\@b) = expect_list_cx_a;
  is \@b, \@ThatArray, '(\@lexical)';
  \my @c = expect_scalar_cx;
  is \@c, \@ThatArray, '\my @lexical';
  (\my @d) = expect_list_cx_a;
  is \@d, \@ThatArray, '(\my @lexical)';
  \(@e) = expect_list_cx;
  is \$e[0].\$e[1], \$_.\$_, '\(@pkg)';
  my @e;
  \(@e) = expect_list_cx;
  is \$e[0].\$e[1], \$_.\$_, '\(@lexical)';
  \(my @f) = expect_list_cx;
  is \$f[0].\$f[1], \$_.\$_, '\(my @lexical)';
  \my(@g) = expect_list_cx;
  is \$g[0].\$g[1], \$_.\$_, '\my(@lexical)';
  my $old = \@h;
  {
    \local @h = \@ThatArray;
    is \@h, \@ThatArray, '\local @a';
  }
  is \@h, $old, '\local @a unwound';
  $old = \@i;
  {
    (\local @i) = \@ThatArray;
    is \@i, \@ThatArray, '(\local @a)';
  }
  is \@i, $old, '(\local @a) unwound';
}
for (1,2) {
  \my @x = [1..3],
  \my(@y) = \3,
  \state @a = [1..3],
  \state(@b) = \3 if $_ == 1;
  if ($_ == 2) {
    is @x, 0, '\my @x = ... clears @x on scope exit';
    is @y, 0, '\my(@x) = ... clears @x on scope exit';
    is "@a", "1 2 3", '\state @x = ... does not clear @x on scope exit';
    is "@b", 3, '\state(@x) = ... does not clear @x on scope exit';
  }
}

# Hashes

package HashTest {
  BEGIN { *is = *main::is }
  sub expect_scalar_cx { wantarray ? 0 : \%ThatHash }
  sub expect_list_cx   { wantarray ? (\%ThatHash)x2 : 0 }
  \%a = expect_scalar_cx;
  is \%a, \%ThatHash, '\%pkg';
  my %a;
  \%a = expect_scalar_cx;
  is \%a, \%ThatHash, '\%lexical';
  (\%b) = expect_list_cx;
  is \%b, \%ThatHash, '(\%pkg)';
  my %b;
  (\%b) = expect_list_cx;
  is \%b, \%ThatHash, '(\%lexical)';
  \my %c = expect_scalar_cx;
  is \%c, \%ThatHash, '\my %lexical';
  (\my %d) = expect_list_cx;
  is \%d, \%ThatHash, '(\my %lexical)';
  my $old = \%h;
  {
    \local %h = \%ThatHash;
    is \%h, \%ThatHash, '\local %a';
  }
  is \%h, $old, '\local %a unwound';
  $old = \%i;
  {
    (\local %i) = \%ThatHash;
    is \%i, \%ThatHash, '(\local %a)';
  }
  is \%i, $old, '(\local %a) unwound';
}
for (1,2) {
  \state %y = {1,2},
  \my %x = {1,2} if $_ == 1;
  if ($_ == 2) {
    is %x, 0, '\my %x = ... clears %x on scope exit';
    is "@{[%y]}", "1 2", '\state %x = ... does not clear %x on scope exit';
  }
}

# Subroutines

package CodeTest {
  BEGIN { *is = *main::is; }
  use feature 'lexical_subs';
  no warnings 'experimental::lexical_subs';
  sub expect_scalar_cx { wantarray ? 0 : \&ThatSub }
  sub expect_list_cx   { wantarray ? (\&ThatSub)x2 : 0 }
  \&a = expect_scalar_cx;
  is \&a, \&ThatSub, '\&pkg';
  my sub a;
  \&a = expect_scalar_cx;
  is \&a, \&ThatSub, '\&mysub';
  state sub as;
  \&as = expect_scalar_cx;
  is \&as, \&ThatSub, '\&statesub';
  (\&b) = expect_list_cx;
  is \&b, \&ThatSub, '(\&pkg)';
  my sub b;
  (\&b) = expect_list_cx;
  is \&b, \&ThatSub, '(\&mysub)';
  my sub bs;
  (\&bs) = expect_list_cx;
  is \&bs, \&ThatSub, '(\&statesub)';
  \(&c) = expect_list_cx;
  is \&c, \&ThatSub, '\(&pkg)';
  my sub b;
  \(&c) = expect_list_cx;
  is \&c, \&ThatSub, '\(&mysub)';
  my sub bs;
  \(&cs) = expect_list_cx;
  is \&cs, \&ThatSub, '\(&statesub)';

  package main {
    # this is only a problem in main:: due to 1e2cfe157ca
    sub sx { "x" }
    sub sy { "y" }
    is sx(), "x", "check original";
    my $temp = \&sx;
    \&sx = \&sy;
    is sx(), "y", "aliased";
    \&sx = $temp;
    is sx(), "x", "and restored";
  }
}

# Mixed List Assignments

(\$tahi, $rua) = \(1,2);
is join(' ', $tahi, $$rua), '1 2',
  'mixed scalar ref and scalar list assignment';
$_ = 1;
\($bb, @cc, %dd, &ee, $_==1 ? $ff : @ff, $_==2 ? $gg : @gg, (@hh)) =
    (\$BB, \@CC, \%DD, \&EE, \$FF, \@GG, \1, \2, \3);
is \$bb, \$BB, '\$scalar in list assignment';
is \@cc, \@CC, '\@array in list assignment';
is \%dd, \%DD, '\%hash in list assignment';
is \&ee, \&EE, '\&code in list assignment';
is \$ff, \$FF, '$scalar in \ternary in list assignment';
is \@gg, \@GG, '@gg in \ternary in list assignment';
is "@hh", '1 2 3', '\(@array) in list assignment';

# Conditional expressions

$_ = 3;
$_ == 3 ? \$tahi : $rua = \3;
is $tahi, 3, 'cond assignment resolving to scalar ref';
$_ == 0 ? \$toru : $wha = \3;
is $$wha, 3, 'cond assignment resolving to scalar';
$_ == 3 ? \$rima : \$ono = \5;
is $rima, 5, 'cond assignment with refgens on both branches';
\($_ == 3 ? $whitu : $waru) = \5;
is $whitu, 5, '\( ?: ) assignment';
\($_ == 3 ? $_ < 4 ? $ii : $_ : $_) = \$_;
is \$ii, \$_, 'nested \ternary assignment';

# Foreach

for \my $topic (\$for1, \$for2) {
    push @for, \$topic;
}
is "@for", \$for1 . ' ' . \$for2, 'foreach \my $a';
is \$topic, \$::topic, 'for \my scoping';

@for = ();
for \$::a(\$for1, \$for2) {
    push @for, \$::a;
}
is "@for", \$for1 . ' ' . \$for2, 'foreach \$::a';

@for = ();
for \my @a([1,2], [3,4]) {
    push @for, @a;
}
is "@for", "1 2 3 4", 'foreach \my @a [perl #22335]';

@for = ();
for \@::a([1,2], [3,4]) {
    push @for, @::a;
}
is "@for", "1 2 3 4", 'foreach \@::a [perl #22335]';

@for = ();
for \my %a({5,6}, {7,8}) {
    push @for, %a;
}
is "@for", "5 6 7 8", 'foreach \my %a [perl #22335]';

@for = ();
for \%::a({5,6}, {7,8}) {
    push @for, %::a;
}
is "@for", "5 6 7 8", 'foreach \%::a [perl #22335]';

@for = ();
{
  use feature 'lexical_subs';
  no warnings 'experimental::lexical_subs';
  my sub a;
  for \&a(sub {9}, sub {10}) {
    push @for, &a;
  }
}
is "@for", "9 10", 'foreach \&padcv';

@for = ();
for \&::a(sub {9}, sub {10}) {
  push @for, &::a;
}
is "@for", "9 10", 'foreach \&rv2cv';

# Errors

eval { my $x; \$x = 3 };
like $@, qr/^Assigned value is not a reference at/, 'assigning non-ref';
eval { my $x; \$x = [] };
like $@, qr/^Assigned value is not a SCALAR reference at/,
    'assigning non-scalar ref to scalar ref';
eval { \$::x = [] };
like $@, qr/^Assigned value is not a SCALAR reference at/,
    'assigning non-scalar ref to package scalar ref';
eval { my @x; \@x = {} };
like $@, qr/^Assigned value is not an ARRAY reference at/,
    'assigning non-array ref to array ref';
eval { \@::x = {} };
like $@, qr/^Assigned value is not an ARRAY reference at/,
    'assigning non-array ref to package array ref';
eval { my %x; \%x = [] };
like $@, qr/^Assigned value is not a HASH reference at/,
    'assigning non-hash ref to hash ref';
eval { \%::x = [] };
like $@, qr/^Assigned value is not a HASH reference at/,
    'assigning non-hash ref to package hash ref';
eval { use feature 'lexical_subs';
       no warnings 'experimental::lexical_subs';
       my sub x; \&x = [] };
like $@, qr/^Assigned value is not a CODE reference at/,
    'assigning non-code ref to lexical code ref';
eval { \&::x = [] };
like $@, qr/^Assigned value is not a CODE reference at/,
    'assigning non-code ref to package code ref';

eval { my $x; (\$x) = 3 };
like $@, qr/^Assigned value is not a reference at/,
    'list-assigning non-ref';
eval { my $x; (\$x) = [] };
like $@, qr/^Assigned value is not a SCALAR reference at/,
    'list-assigning non-scalar ref to scalar ref';
eval { (\$::x = []) };
like $@, qr/^Assigned value is not a SCALAR reference at/,
    'list-assigning non-scalar ref to package scalar ref';
eval { my @x; (\@x) = {} };
like $@, qr/^Assigned value is not an ARRAY reference at/,
    'list-assigning non-array ref to array ref';
eval { (\@::x) = {} };
like $@, qr/^Assigned value is not an ARRAY reference at/,
    'list-assigning non-array ref to package array ref';
eval { my %x; (\%x) = [] };
like $@, qr/^Assigned value is not a HASH reference at/,
    'list-assigning non-hash ref to hash ref';
eval { (\%::x) = [] };
like $@, qr/^Assigned value is not a HASH reference at/,
    'list-assigning non-hash ref to package hash ref';
eval { use feature 'lexical_subs';
       no warnings 'experimental::lexical_subs';
       my sub x; (\&x) = [] };
like $@, qr/^Assigned value is not a CODE reference at/,
    'list-assigning non-code ref to lexical code ref';
eval { (\&::x) = [] };
like $@, qr/^Assigned value is not a CODE reference at/,
    'list-assigning non-code ref to package code ref';

eval '(\do{}) = 42';
like $@, qr/^Can't modify reference to do block in list assignment at /,
    "Can't modify reference to do block in list assignment";
eval '(\pos) = 42';
like $@,
     qr/^Can't modify reference to match position in list assignment at /,
    "Can't modify ref to some scalar-returning op in list assignment";
eval '(\glob) = 42';
like $@,
     qr/^Can't modify reference to glob in list assignment at /,
    "Can't modify reference to some list-returning op in list assignment";
eval '\pos = 42';
like $@,
    qr/^Can't modify reference to match position in scalar assignment at /,
   "Can't modify ref to some scalar-returning op in scalar assignment";
eval '\(local @b) = 42';
like $@,
    qr/^Can't modify reference to localized parenthesized array in list(?x:
      ) assignment at /,
   q"Can't modify \(local @array) in list assignment";
eval '\local(@b) = 42';
like $@,
    qr/^Can't modify reference to localized parenthesized array in list(?x:
      ) assignment at /,
   q"Can't modify \local(@array) in list assignment";
eval '\local(@{foo()}) = 42';
like $@,
    qr/^Can't modify reference to array dereference in list assignment at/,
   q"'Array deref' error takes prec. over 'local paren' error";
eval '\(%b) = 42';
like $@,
    qr/^Can't modify reference to parenthesized hash in list assignment a/,
   "Can't modify ref to parenthesized package hash in scalar assignment";
eval '\(my %b) = 42';
like $@,
    qr/^Can't modify reference to parenthesized hash in list assignment a/,
   "Can't modify ref to parenthesized hash (\(my %b)) in list assignment";
eval '\my(%b) = 42';
like $@,
    qr/^Can't modify reference to parenthesized hash in list assignment a/,
   "Can't modify ref to parenthesized hash (\my(%b)) in list assignment";
eval '\%{"42"} = 42';
like $@,
    qr/^Can't modify reference to hash dereference in scalar assignment a/,
   "Can't modify reference to hash dereference in scalar assignment";
eval '$foo ? \%{"42"} : \%43 = 42';
like $@,
    qr/^Can't modify reference to hash dereference in scalar assignment a/,
   "Can't modify ref to whatever in scalar assignment via cond expr";
eval '\$0=~y///=0';
like $@,
    qr#^Can't modify transliteration \(tr///\) in scalar assignment a#,
   "Can't modify transliteration (tr///) in scalar assignment";

# Miscellaneous

{
  local $::TODO = ' ';
  my($x,$y);
  sub {
    sub {
      \$x = \$y;
    }->();
    is \$x, \$y, 'lexical alias affects outer closure';
  }->();
  is \$x, \$y, 'lexical alias affects outer sub where vars are declared';
}

{ # PADSTALE has a double meaning
  use feature 'lexical_subs', 'signatures';
  no warnings 'experimental';
  my $c;
  my sub s ($arg) {
    state $x = ++$c;
    if ($arg == 3) { return $c }
    goto skip if $arg == 2;
    my $y;
   skip:
    # $y is PADSTALE the 2nd time
    \$x = \$y if $arg == 2;
  }
  s(1);
  s(2);
  is s(3), 1, 'padstale alias should not reset state'
}

{
    my $a;
    no warnings 'experimental::builtin';
    builtin::weaken($r = \$a);
    \$a = $r;
    pass 'no crash when assigning \$lex = $weakref_to_lex'
}

{
    \my $x = \my $y;
    $x = 3;
    ($x, my $z) = (1, $y);
    is $z, 3, 'list assignment after aliasing lexical scalars';
}
{
    (\my $x) = \my $y;
    $x = 3;
    ($x, my $z) = (1, $y);
    is $z, 3,
      'regular list assignment after aliasing via list assignment';
}
{
    my $y;
    goto do_aliasing;

   do_test:
    $y = 3;
    my($x,$z) = (1, $y);
    is $z, 3, 'list assignment "before" aliasing lexical scalars';
    last;

   do_aliasing:
    \$x = \$y;
    goto do_test;
}
{
    my $y;
    goto do_aliasing2;

   do_test2:
    $y = 3;
    my($x,$z) = (1, $y);
    is $z, 3,
     'list assignment "before" aliasing lex scalars via list assignment';
    last;

   do_aliasing2:
    \($x) = \$y;
    goto do_test2;
}
{
    my @a;
    goto do_aliasing3;

   do_test3:
    @a[0,1] = qw<a b>;
    my($y,$x) = ($a[0],$a[1]);
    is "@a", 'b a',
       'aelemfast_lex-to-scalar list assignment "before" aliasing';
    last;

   do_aliasing3:
    \(@a) = \($x,$y);
    goto do_test3;
}

# Used to fail an assertion [perl #123821]
eval '\(&$0)=0';
pass("RT #123821");

# Used to fail an assertion [perl #128252]
{
    no feature 'refaliasing';
    use warnings;
    eval q{sub{\@0[0]=0};};
    pass("RT #128252");
}

# RT #133538 slices were inadvertently always localising

{
    use feature 'refaliasing';
    no warnings 'experimental';

    my @src = (100,200,300);

    my @a = (1,2,3);
    my %h = qw(one 10 two 20 three 30);

    {
        use feature 'declared_refs';
        local \(@a[0,1,2]) = \(@src);
        local \(@h{qw(one two three)}) = \(@src);
        $src[0]++;
        is("@a", "101 200 300", "rt #133538 \@a aliased");
        is("$h{one} $h{two} $h{three}", "101 200 300", "rt #133538 %h aliased");
    }
    is("@a", "1 2 3", "rt #133538 \@a restored");
    is("$h{one} $h{two} $h{three}", "10 20 30", "rt #133538 %h restored");

    {
        \(@a[0,1,2]) = \(@src);
        \(@h{qw(one two three)}) = \(@src);
        $src[0]++;
        is("@a", "102 200 300", "rt #133538 \@a aliased try 2");
        is("$h{one} $h{two} $h{three}", "102 200 300",
                "rt #133538 %h aliased try 2");
    }
    $src[2]++;
    is("@a", "102 200 301", "rt #133538 \@a still aliased");
    is("$h{one} $h{two} $h{three}", "102 200 301", "rt #133538 %h still aliased");

}
