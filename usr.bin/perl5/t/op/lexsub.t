#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    *bar::is = *is;
    *bar::like = *like;
}
plan 151;

# -------------------- our -------------------- #

{
  our sub foo { 42 }
  is foo, 42, 'calling our sub from same package';
  is &foo, 42, 'calling our sub from same package (amper)';
  package bar;
  sub bar::foo { 43 }
  is foo, 42, 'calling our sub from another package';
  is &foo, 42, 'calling our sub from another package (amper)';
}
package bar;
is foo, 43, 'our sub falling out of scope';
is &foo, 43, 'our sub falling out of scope (called via amper)';
package main;
{
  sub bar::a { 43 }
  our sub a {
    if (shift) {
      package bar;
      is a, 43, 'our sub invisible inside itself';
      is &a, 43, 'our sub invisible inside itself (called via amper)';
    }
    42
  }
  a(1);
  sub bar::b { 43 }
  our sub b;
  our sub b {
    if (shift) {
      package bar;
      is b, 42, 'our sub visible inside itself after decl';
      is &b, 42, 'our sub visible inside itself after decl (amper)';
    }
    42
  }
  b(1)
}
sub c { 42 }
sub bar::c { 43 }
{
  our sub c;
  package bar;
  is c, 42, 'our sub foo; makes lex alias for existing sub';
  is &c, 42, 'our sub foo; makes lex alias for existing sub (amper)';
}
{
  our sub d;
  sub bar::d { 'd43' }
  package bar;
  sub d { 'd42' }
  is eval ::d, 'd42', 'our sub foo; applies to subsequent sub foo {}';
}
{
  our sub e ($);
  is prototype "::e", '$', 'our sub with proto';
}
{
  our sub if() { 42 }
  my $x = if if if;
  is $x, 42, 'lexical subs (even our) override all keywords';
  package bar;
  my $y = if if if;
  is $y, 42, 'our subs from other packages override all keywords';
}
# Interaction with ‘use constant’
{
  our sub const; # symtab now has an undefined CV
  BEGIN { delete $::{const} } # delete symtab entry; pad entry still exists
  use constant const => 3; # symtab now has a scalar ref
  # inlining this used to fail an assertion (parentheses necessary):
  is(const, 3, 'our sub pointing to "use constant" constant');
}
# our sub and method confusion
sub F::h { 4242 }
{
  my $called;
  our sub h { ++$called; 4343 };
  is((h F),4242, 'our sub symbol translation does not affect meth names');
  undef $called;
  print "#";
  print h F; # follows a different path through yylex to intuit_method
  print "\n";
  is $called, undef, 'our sub symbol translation & meth names after print'
}
our sub j;
is j
  =>, 'j', 'name_of_our_sub <newline> =>  is parsed properly';
sub _cmp { $a cmp $b }
sub bar::_cmp { $b cmp $a }
{
  package bar;
  our sub _cmp;
  package main;
  is join(" ", sort _cmp split //, 'oursub'), 'u u s r o b', 'sort our_sub'
}

# -------------------- state -------------------- #

use feature 'state'; # state
{
  state sub foo { 44 }
  isnt \&::foo, \&foo, 'state sub is not stored in the package';
  is foo, 44, 'calling state sub from same package';
  is &foo, 44, 'calling state sub from same package (amper)';
  package bar;
  is foo, 44, 'calling state sub from another package';
  is &foo, 44, 'calling state sub from another package (amper)';
}
package bar;
is foo, 43, 'state sub falling out of scope';
is &foo, 43, 'state sub falling out of scope (called via amper)';
{
  sub sa { 43 }
  state sub sa {
    if (shift) {
      is sa, 43, 'state sub invisible inside itself';
      is &sa, 43, 'state sub invisible inside itself (called via amper)';
    }
    44
  }
  sa(1);
  sub sb { 43 }
  state sub sb;
  state sub sb {
    if (shift) {
      # ‘state sub foo{}’ creates a new pad entry, not reusing the forward
      #  declaration.  Being invisible inside itself, it sees the stub.
      eval{sb};
      like $@, qr/^Undefined subroutine &sb called at /,
        'state sub foo {} after forward declaration';
      eval{&sb};
      like $@, qr/^Undefined subroutine &sb called at /,
        'state sub foo {} after forward declaration (amper)';
    }
    44
  }
  sb(1);
  sub sb2 { 43 }
  state sub sb2;
  sub sb2 {
    if (shift) {
      package bar;
      is sb2, 44, 'state sub visible inside itself after decl';
      is &sb2, 44, 'state sub visible inside itself after decl (amper)';
    }
    44
  }
  sb2(1);
  state sub sb3;
  {
    state sub sb3 { # new pad entry
      # The sub containing this comment is invisible inside itself.
      # So this one here will assign to the outer pad entry:
      sub sb3 { 47 }
    }
  }
  is eval{sb3}, 47,
    'sub foo{} applying to "state sub foo;" even inside state sub foo{}';
  # Same test again, but inside an anonymous sub
  sub {
    state sub sb4;
    {
      state sub sb4 {
        sub sb4 { 47 }
      }
    }
    is sb4, 47,
      'sub foo{} applying to "state sub foo;" even inside state sub foo{}';
  }->();
}
sub sc { 43 }
{
  state sub sc;
  eval{sc};
  like $@, qr/^Undefined subroutine &sc called at /,
     'state sub foo; makes no lex alias for existing sub';
  eval{&sc};
  like $@, qr/^Undefined subroutine &sc called at /,
     'state sub foo; makes no lex alias for existing sub (amper)';
}
package main;
{
  state sub se ($);
  is prototype eval{\&se}, '$', 'state sub with proto';
  is prototype "se", undef, 'prototype "..." ignores state subs';
}
{
  state sub if() { 44 }
  my $x = if if if;
  is $x, 44, 'state subs override all keywords';
  package bar;
  my $y = if if if;
  is $y, 44, 'state subs from other packages override all keywords';
}
{
  use warnings; no warnings "experimental::lexical_subs";
  state $w ;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval '#line 87 squidges
    state sub foo;
    state sub foo {};
  ';
  is $w,
     '"state" subroutine &foo masks earlier declaration in same scope at '
   . "squidges line 88.\n",
     'warning for state sub masking earlier declaration';
}
# Since state vars inside anonymous subs are cloned at the same time as the
# anonymous subs containing them, the same should happen for state subs.
sub make_closure {
  my $x = shift;
  sub {
    state sub foo { $x }
    foo
  }
}
$sub1 = make_closure 48;
$sub2 = make_closure 49;
is &$sub1, 48, 'state sub in closure (1)';
is &$sub2, 49, 'state sub in closure (2)';
# But we need to test that state subs actually do persist from one invoca-
# tion of a named sub to another (i.e., that they are not my subs).
{
  use warnings; no warnings "experimental::lexical_subs";
  state $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval '#line 65 teetet
    sub foom {
      my $x = shift;
      state sub poom { $x }
      eval{\&poom}
    }
  ';
  is $w, "Variable \"\$x\" will not stay shared at teetet line 67.\n",
         'state subs get "Variable will not stay shared" messages';
  my $poom = foom(27);
  my $poom2 = foom(678);
  is eval{$poom->()}, eval {$poom2->()},
    'state subs close over the first outer my var, like pkg subs';
  my $x = 43;
  for $x (765) {
    state sub etetetet { $x }
    is eval{etetetet}, 43, 'state sub ignores for() localisation';
  }
}
# And we also need to test that multiple state subs can close over each
# other’s entries in the parent subs pad, and that cv_clone is not con-
# fused by that.
sub make_anon_with_state_sub{
  sub {
    state sub s1;
    state sub s2 { \&s1 }
    sub s1 { \&s2 }
    if (@_) { return \&s1 }
    is s1,\&s2, 'state sub in anon closure closing over sibling state sub';
    is s2,\&s1, 'state sub in anon closure closing over sibling state sub';
  }
}
{
  my $s = make_anon_with_state_sub;
  &$s;

  # And make sure the state subs were actually cloned.
  isnt make_anon_with_state_sub->(0), &$s(0),
    'state subs in anon subs are cloned';
  is &$s(0), &$s(0), 'but only when the anon sub is cloned';
}
# Check that nested state subs close over variables properly
{
  is sub {
    state sub a;
    state sub b {
      state sub c {
        state $x = 42;
        sub a { $x }
      }
      c();
    }
    b();
    a();
  }->(), 42, 'state sub with body defined in doubly-nested state subs';
  is sub {
    state sub a;
    state sub b;
    state sub c {
      sub b {
        state $x = 42;
        sub a { $x }
      }
    }
    b();
    a();
  }->(), 42, 'nested state subs declared in same scope';
  state $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  use warnings 'closure';
  my $sub = sub {
    state sub a;
    sub {
      my $x;
      sub a { $x }
    }
  };
  like $w, qr/Variable \"\$x\" is not available at /,
      "unavailability warning when state closure is defined in anon sub";
}
{
  state sub BEGIN { exit };
  pass 'state subs are never special blocks';
  state sub END { shift }
  is eval{END('jkqeudth')}, jkqeudth,
    'state sub END {shift} implies @_, not @ARGV';
  state sub CORE { scalar reverse shift }
  is CORE::uc("hello"), "HELLO",
    'lexical CORE does not interfere with CORE::...';
}
{
  state sub redef {}
  use warnings; no warnings "experimental::lexical_subs";
  state $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval "#line 56 pygpyf\nsub redef {}";
  is $w, "Subroutine redef redefined at pygpyf line 56.\n",
         "sub redefinition warnings from state subs";
}
{
  state sub p (\@) {
    is ref $_[0], 'ARRAY', 'state sub with proto';
  }
  p(my @a);
  p my @b;
  state sub q () { 45 }
  is q(), 45, 'state constant called with parens';
}
{
  state sub x;
  eval 'sub x {3}';
  is x, 3, 'state sub defined inside eval';

  sub r {
    state sub foo { 3 };
    if (@_) { # outer call
      r();
      is foo(), 42,
         'state sub run-time redefinition applies to all recursion levels';
    }
    else { # inner call
      eval 'sub foo { 42 }';
    }
  }
  r(1);
}
like runperl(
      switches => [ '-Mfeature=lexical_subs,state' ],
      prog     => 'state sub a { foo ref } a()',
      stderr   => 1
     ),
     qr/syntax error/,
    'referencing a state sub after a syntax error does not crash';
{
  state $stuff;
  package A {
    state sub foo{ $stuff .= our $AUTOLOAD }
    *A::AUTOLOAD = \&foo;
  }
  A::bar();
  is $stuff, 'A::bar', 'state sub assigned to *AUTOLOAD can autoload';
}
{
  state sub quire{qr "quires"}
  package o { use overload qr => \&quire }
  ok "quires" =~ bless([], o::), 'state sub used as overload method';
}
{
  state sub foo;
  *cvgv = \&foo;
  local *cvgv2 = *cvgv;
  eval 'sub cvgv2 {42}'; # uses the stub already present
  is foo, 42, 'defining state sub body via package sub declaration';
}
{
  local $ENV{PERL5DB} = 'sub DB::DB{}';
  is(
    runperl(
     switches => [ '-d' ],
     progs => [ split "\n",
      'use feature qw - lexical_subs state -;
       no warnings q-experimental::lexical_subs-;
       sub DB::sub{
         print qq|4\n| unless $DB::sub =~ DESTROY;
         goto $DB::sub
       }
       state sub foo {print qq|2\n|}
       foo();
      '
     ],
     stderr => 1
    ),
    "4\n2\n",
    'state subs and DB::sub under -d'
  );
  is(
    runperl(
     switches => [ '-d' ],
     progs => [ split "\n",
      'use feature qw - lexical_subs state -;
       no warnings q-experimental::lexical_subs-;
       sub DB::goto{ print qq|4\n|; $_ = $DB::sub }
       state sub foo {print qq|2\n|}
       $^P|=0x80;
       sub { goto &foo }->();
       print $_ == \&foo ? qq|ok\n| : qq|$_\n|;
      '
     ],
     stderr => 1
    ),
    "4\n2\nok\n",
    'state subs and DB::goto under -d'
  );
}
# This used to fail an assertion, but only as a standalone script
is runperl(switches => ['-lXMfeature=:all'],
           prog     => 'state sub x {}; undef &x; print defined &x',
           stderr   => 1), "\n", 'undefining state sub';
{
  state sub x { is +(caller 0)[3], 'x', 'state sub name in caller' }
  x
}
{
  state sub _cmp { $b cmp $a }
  is join(" ", sort _cmp split //, 'lexsub'), 'x u s l e b',
    'sort state_sub LIST'
}
{
  state sub handel { "" }
  print handel, "ok ", curr_test(),
       " - no 'No comma allowed' after state sub\n";
  curr_test(curr_test()+1);
}
{
  use utf8;
  state sub φου;
  eval { φου };
  like $@, qr/^Undefined subroutine &φου called at /,
    'state sub with utf8 name';
}
# This used to crash, but only as a standalone script
is runperl(switches => ['-lXMfeature=:all'],
           prog     => '$::x = global=>;
                        sub x;
                        sub x {
                          state $x = 42;
                          state sub x { print eval q|$x| }
                          x()
                        }
                        x()',
           stderr   => 1), "42\n",
  'closure behaviour of state sub in predeclared package sub';

# -------------------- my -------------------- #

{
  my sub foo { 44 }
  isnt \&::foo, \&foo, 'my sub is not stored in the package';
  is foo, 44, 'calling my sub from same package';
  is &foo, 44, 'calling my sub from same package (amper)';
  package bar;
  is foo, 44, 'calling my sub from another package';
  is &foo, 44, 'calling my sub from another package (amper)';
}
package bar;
is foo, 43, 'my sub falling out of scope';
is &foo, 43, 'my sub falling out of scope (called via amper)';
{
  sub ma { 43 }
  my sub ma {
    if (shift) {
      is ma, 43, 'my sub invisible inside itself';
      is &ma, 43, 'my sub invisible inside itself (called via amper)';
    }
    44
  }
  ma(1);
  sub mb { 43 }
  my sub mb;
  my sub mb {
    if (shift) {
      # ‘my sub foo{}’ creates a new pad entry, not reusing the forward
      #  declaration.  Being invisible inside itself, it sees the stub.
      eval{mb};
      like $@, qr/^Undefined subroutine &mb called at /,
        'my sub foo {} after forward declaration';
      eval{&mb};
      like $@, qr/^Undefined subroutine &mb called at /,
        'my sub foo {} after forward declaration (amper)';
    }
    44
  }
  mb(1);
  sub mb2 { 43 }
  my sub sb2;
  sub mb2 {
    if (shift) {
      package bar;
      is mb2, 44, 'my sub visible inside itself after decl';
      is &mb2, 44, 'my sub visible inside itself after decl (amper)';
    }
    44
  }
  mb2(1);
  my sub mb3;
  {
    my sub mb3 { # new pad entry
      # The sub containing this comment is invisible inside itself.
      # So this one here will assign to the outer pad entry:
      sub mb3 { 47 }
    }
  }
  is eval{mb3}, 47,
    'sub foo{} applying to "my sub foo;" even inside my sub foo{}';
  # Same test again, but inside an anonymous sub
  sub {
    my sub mb4;
    {
      my sub mb4 {
        sub mb4 { 47 }
      }
    }
    is mb4, 47,
      'sub foo{} applying to "my sub foo;" even inside my sub foo{}';
  }->();
}
sub mc { 43 }
{
  my sub mc;
  eval{mc};
  like $@, qr/^Undefined subroutine &mc called at /,
     'my sub foo; makes no lex alias for existing sub';
  eval{&mc};
  like $@, qr/^Undefined subroutine &mc called at /,
     'my sub foo; makes no lex alias for existing sub (amper)';
}
package main;
{
  my sub me ($);
  is prototype eval{\&me}, '$', 'my sub with proto';
  is prototype "me", undef, 'prototype "..." ignores my subs';

  my $coderef = eval "my sub foo (\$\x{30cd}) {1}; \\&foo";
  my $proto = prototype $coderef;
  ok(utf8::is_utf8($proto), "my sub with UTF8 proto maintains the UTF8ness");
  is($proto, "\$\x{30cd}", "check the prototypes actually match");
}
{
  my sub if() { 44 }
  my $x = if if if;
  is $x, 44, 'my subs override all keywords';
  package bar;
  my $y = if if if;
  is $y, 44, 'my subs from other packages override all keywords';
}
{
  use warnings; no warnings "experimental::lexical_subs";
  my $w ;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval '#line 87 squidges
    my sub foo;
    my sub foo {};
  ';
  is $w,
     '"my" subroutine &foo masks earlier declaration in same scope at '
   . "squidges line 88.\n",
     'warning for my sub masking earlier declaration';
}
# Test that my subs are cloned inside anonymous subs.
sub mmake_closure {
  my $x = shift;
  sub {
    my sub foo { $x }
    foo
  }
}
$sub1 = mmake_closure 48;
$sub2 = mmake_closure 49;
is &$sub1, 48, 'my sub in closure (1)';
is &$sub2, 49, 'my sub in closure (2)';
# Test that they are cloned in named subs.
{
  use warnings; no warnings "experimental::lexical_subs";
  my $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval '#line 65 teetet
    sub mfoom {
      my $x = shift;
      my sub poom { $x }
      \&poom
    }
  ';
  is $w, undef, 'my subs get no "Variable will not stay shared" messages';
  my $poom = mfoom(27);
  my $poom2 = mfoom(678);
  is $poom->(), 27, 'my subs closing over outer my var (1)';
  is $poom2->(), 678, 'my subs closing over outer my var (2)';
  my $x = 43;
  my sub aoeu;
  for $x (765) {
    my sub etetetet { $x }
    sub aoeu { $x }
    is etetetet, 765, 'my sub respects for() localisation';
    is aoeu, 43, 'unless it is declared outside the for loop';
  }
}
# And we also need to test that multiple my subs can close over each
# other’s entries in the parent subs pad, and that cv_clone is not con-
# fused by that.
sub make_anon_with_my_sub{
  sub {
    my sub s1;
    my sub s2 { \&s1 }
    sub s1 { \&s2 }
    if (@_) { return eval { \&s1 } }
    is eval{s1},eval{\&s2}, 'my sub in anon closure closing over sibling my sub';
    is eval{s2},eval{\&s1}, 'my sub in anon closure closing over sibling my sub';
  }
}

# Test my subs inside predeclared my subs
{
  my sub s2;
  sub s2 {
    my $x = 3;
    my sub s3 { eval '$x' }
    s3;
  }
  is s2, 3, 'my sub inside predeclared my sub';
}

{
  my $s = make_anon_with_my_sub;
  &$s;

  # And make sure the my subs were actually cloned.
  isnt make_anon_with_my_sub->(0), &$s(0),
    'my subs in anon subs are cloned';
  isnt &$s(0), &$s(0), 'at each invocation of the enclosing sub';
}
{
  my sub BEGIN { exit };
  pass 'my subs are never special blocks';
  my sub END { shift }
  is END('jkqeudth'), jkqeudth,
    'my sub END {shift} implies @_, not @ARGV';
}
{
  my sub redef {}
  use warnings; no warnings "experimental::lexical_subs";
  my $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval "#line 56 pygpyf\nsub redef {}";
  is $w, "Subroutine redef redefined at pygpyf line 56.\n",
         "sub redefinition warnings from my subs";

  undef $w;
  sub {
    my sub x {};
    sub { eval "#line 87 khaki\n\\&x" }
  }->()();
  is $w, "Subroutine \"&x\" is not available at khaki line 87.\n",
         "unavailability warning during compilation of eval in closure";

  undef $w;
  no warnings 'void';
  eval <<'->()();';
#line 87 khaki
    sub {
      my sub x{}
      sub not_lexical8 {
        \&x
      }
    }
->()();
  is $w, "Subroutine \"&x\" is not available at khaki line 90.\n",
         "unavailability warning during compilation of named sub in anon";

  undef $w;
  sub not_lexical9 {
    my sub x {};
    format =
@
&x
.
  }
  eval { write };
  my($f,$l) = (__FILE__,__LINE__ - 1);
  is $w, "Subroutine \"&x\" is not available at $f line $l.\n",
         'unavailability warning during cloning';
  $l -= 3;
  is $@, "Undefined subroutine &x called at $f line $l.\n",
         'Vivified sub is correctly named';
}
sub not_lexical10 {
  my sub foo;
  foo();
  sub not_lexical11 {
    my sub bar {
      my $x = 'khaki car keys for the khaki car';
      not_lexical10();
      sub foo {
       is $x, 'khaki car keys for the khaki car',
       'mysubs in inner clonables use the running clone of their CvOUTSIDE'
      }
    }
    bar()
  }
}
not_lexical11();
{
  my sub p (\@) {
    is ref $_[0], 'ARRAY', 'my sub with proto';
  }
  p(my @a);
  p @a;
  my sub q () { 46 }
  is q(), 46, 'my constant called with parens';
}
{
  my sub x;
  my $count;
  sub x { x() if $count++ < 10 }
  x();
  is $count, 11, 'my recursive subs';
}
{
  my sub x;
  eval 'sub x {3}';
  is x, 3, 'my sub defined inside eval';

  my sub z;
  BEGIN { eval 'sub z {4}' }
  is z, 4, 'my sub defined in BEGIN { eval "..." }';
}

{
  state $w;
  local $SIG{__WARN__} = sub { $w .= shift };
  eval q{ my sub george () { 2 } };
  is $w, undef, 'no double free from constant my subs';
}
like runperl(
      switches => [ '-Mfeature=lexical_subs,state' ],
      prog     => 'my sub a { foo ref } a()',
      stderr   => 1
     ),
     qr/syntax error/,
    'referencing a my sub after a syntax error does not crash';
{
  state $stuff;
  package A {
    my sub foo{ $stuff .= our $AUTOLOAD }
    *A::AUTOLOAD = \&foo;
  }
  A::bar();
  is $stuff, 'A::bar', 'my sub assigned to *AUTOLOAD can autoload';
}
{
  my sub quire{qr "quires"}
  package mo { use overload qr => \&quire }
  ok "quires" =~ bless([], mo::), 'my sub used as overload method';
}
{
  my sub foo;
  *mcvgv = \&foo;
  local *mcvgv2 = *mcvgv;
  eval 'sub mcvgv2 {42}'; # uses the stub already present
  is foo, 42, 'defining my sub body via package sub declaration';
}
{
  my sub foo;
  *mcvgv3 = \&foo;
  local *mcvgv4 = *mcvgv3;
  eval 'sub mcvgv4 {42}'; # uses the stub already present
  undef *mcvgv3; undef *mcvgv4; # leaves the pad with the only reference
}
# We would have crashed by now if it weren’t fixed.
pass "pad taking ownership once more of packagified my-sub";

{
  local $ENV{PERL5DB} = 'sub DB::DB{}';
  is(
    runperl(
     switches => [ '-d' ],
     progs => [ split "\n",
      'use feature qw - lexical_subs state -;
       no warnings q-experimental::lexical_subs-;
       sub DB::sub{
         print qq|4\n| unless $DB::sub =~ DESTROY;
         goto $DB::sub
       }
       my sub foo {print qq|2\n|}
       foo();
      '
     ],
     stderr => 1
    ),
    "4\n2\n",
    'my subs and DB::sub under -d'
  );
}
# This used to fail an assertion, but only as a standalone script
is runperl(switches => ['-lXMfeature=:all'],
           prog     => 'my sub x {}; undef &x; print defined &x',
           stderr   => 1), "\n", 'undefining my sub';
{
  my sub x { is +(caller 0)[3], 'x', 'my sub name in caller' }
  x
}
{
  my sub _cmp { $b cmp $a }
  is join(" ", sort _cmp split //, 'lexsub'), 'x u s l e b',
    'sort my_sub LIST'
}
{
  my sub handel { "" }
  print handel,"ok ",curr_test()," - no 'No comma allowed' after my sub\n";
  curr_test(curr_test()+1);
}
{
  my $x = 43;
  my sub y :prototype() {$x};
  is y, 43, 'my sub that looks like constant closure';
}
{
  use utf8;
  my sub φου;
  eval { φου };
  like $@, qr/^Undefined subroutine &φου called at /,
    'my sub with utf8 name';
}
{
  my $w;
  local $SIG{__WARN__} = sub { $w = shift };
  use warnings 'closure';
  eval 'sub stayshared { my sub x; sub notstayshared { x } } 1' or die;
  like $w, qr/^Subroutine "&x" will not stay shared at /,
          'Subroutine will not stay shared';
}

# -------------------- Interactions (and misc tests) -------------------- #

is sub {
    my sub s1;
    my sub s2 { 3 };
    sub s1 { state sub foo { \&s2 } foo }
    s1
  }->()(), 3, 'state sub inside my sub closing over my sub uncle';

{
  my sub s2 { 3 };
  sub not_lexical { state sub foo { \&s2 } foo }
  is not_lexical->(), 3, 'state subs that reference my sub from outside';
}

# Test my subs inside predeclared package subs
# This test also checks that CvOUTSIDE pointers are not mangled when the
# inner sub’s CvOUTSIDE points to another sub.
sub not_lexical2;
sub not_lexical2 {
  my $x = 23;
  my sub bar;
  sub not_lexical3 {
    not_lexical2();
    sub bar { $x }
  };
  bar
}
is not_lexical3, 23, 'my subs inside predeclared package subs';

# Test my subs inside predeclared package sub, where the lexical sub is
# declared outside the package sub.
# This checks that CvOUTSIDE pointers are fixed up even when the sub is
# not declared inside the sub that its CvOUTSIDE points to.
sub not_lexical5 {
  my sub foo;
  sub not_lexical4;
  sub not_lexical4 {
    my $x = 234;
    not_lexical5();
    sub foo { $x }
  }
  foo
}
is not_lexical4, 234,
    'my sub defined in predeclared pkg sub but declared outside';

undef *not_lexical6;
{
  my sub foo;
  sub not_lexical6 { sub foo { } }
  pass 'no crash when cloning a mysub declared inside an undef pack sub';
}

undef &not_lexical7;
eval 'sub not_lexical7 { my @x }';
{
  my sub foo;
  foo();
  sub not_lexical7 {
    state $x;
    sub foo {
      is ref \$x, 'SCALAR',
        "redeffing a mysub's outside does not make it use the wrong pad"
    }
  }
}

like runperl(
      switches => [ '-Mfeature=lexical_subs,state', '-Mwarnings=FATAL,all', '-M-warnings=experimental::lexical_subs' ],
      prog     => 'my sub foo; sub foo { foo } foo',
      stderr   => 1
     ),
     qr/Deep recursion on subroutine "foo"/,
    'deep recursion warnings for lexical subs do not crash';

like runperl(
      switches => [ '-Mfeature=lexical_subs,state', '-Mwarnings=FATAL,all', '-M-warnings=experimental::lexical_subs' ],
      prog     => 'my sub foo() { 42 } undef &foo',
      stderr   => 1
     ),
     qr/Constant subroutine foo undefined at /,
    'constant undefinition warnings for lexical subs do not crash';

{
  my sub foo;
  *AutoloadTestSuper::blah = \&foo;
  sub AutoloadTestSuper::AUTOLOAD {
    is $AutoloadTestSuper::AUTOLOAD, "AutoloadTestSuper::blah",
      "Autoloading via inherited lex stub";
  }
  @AutoloadTest::ISA = AutoloadTestSuper::;
  AutoloadTest->blah;
}

# This used to crash because op.c:find_lexical_cv was looking at the wrong
# CV’s OUTSIDE pointer.  [perl #124099]
{
  my sub h; sub{my $x; sub{h}}
}

is join("-", qw(aa bb), do { my sub lleexx; 123 }, qw(cc dd)),
  "aa-bb-123-cc-dd", 'do { my sub...} in a list [perl #132442]';

{
    # this would crash because find_lexical_cv() couldn't handle an
    # intermediate scope which didn't include the sub
    no warnings 'experimental::builtin';
    use builtin 'ceil';
    sub nested {
        ok(eval 'ceil(1.5)', "no assertion failure calling a lexical sub from nested eval");
    }
    nested();
}
