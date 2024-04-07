#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan(tests => 65);

sub empty_sub {}

is(empty_sub,undef,"Is empty");
is(empty_sub(1,2,3),undef,"Is still empty");
@test = empty_sub();
is(scalar(@test), 0, 'Didnt return anything');
@test = empty_sub(1,2,3);
is(scalar(@test), 0, 'Didnt return anything');

# [perl #91844] return should always copy
{
    $foo{bar} = 7;
    for my $x ($foo{bar}) {
	# Pity test.pl doesnt have isn't.
	isnt \sub { delete $foo{bar} }->(), \$x,
	   'result of delete(helem) is copied when returned';
    }
    $foo{bar} = 7;
    for my $x ($foo{bar}) {
	isnt \sub { return delete $foo{bar} }->(), \$x,
	   'result of delete(helem) is copied when explicitly returned';
    }
    my $x;
    isnt \sub { delete $_[0] }->($x), \$x,
      'result of delete(aelem) is copied when returned';
    isnt \sub { return delete $_[0] }->($x), \$x,
      'result of delete(aelem) is copied when explicitly returned';
    isnt \sub { ()=\@_; shift }->($x), \$x,
      'result of shift is copied when returned';
    isnt \sub { ()=\@_; return shift }->($x), \$x,
      'result of shift is copied when explicitly returned';

    $foo{bar} = 7;
    my $r = \$foo{bar};
    sub {
        $$r++;
        isnt($_[0], $$r, "result of delete(helem) is copied: practical test");
    }->(sub { delete $foo{bar} }->());
}

fresh_perl_is
  <<'end', "main::foo\n", {}, 'sub redefinition sets CvGV';
*foo = \&baz;
*bar = *foo;
eval 'sub bar { print +(caller 0)[3], "\n" }';
bar();
end

fresh_perl_is
  <<'end', "main::foo\nok\n", {}, 'no double free redefining anon stub';
my $sub = sub { 4 };
*foo = $sub;
*bar = *foo;
undef &$sub;
eval 'sub bar { print +(caller 0)[3], "\n" }';
&$sub;
undef *foo;
undef *bar;
print "ok\n";
end

# The outer call sets the scalar returned by ${\""}.${\""} to the current
# package name.
# The inner call sets it to "road".
# Each call records the value twice, the outer call surrounding the inner
# call.  In 5.10-5.18 under ithreads, what gets pushed is
# qw(main road road road) because the inner call is clobbering the same
# scalar.  If __PACKAGE__ is changed to "main", it works, the last element
# becoming "main".
my @scratch;
sub a {
  for (${\""}.${\""}) {
    $_ = $_[0];
    push @scratch, $_;
    a("road",1) unless $_[1];
    push @scratch, $_;
  }
}
a(__PACKAGE__);
require Config;
is "@scratch", "main road road main",
   'recursive calls do not share shared-hash-key TARGs';

# Another test for the same bug, that does not rely on foreach.  It depends
# on ref returning a shared hash key TARG.
undef @scratch;
sub b {
    my ($pack, $depth) = @_;
    my $o = bless[], $pack;
    $pack++;
    push @scratch, (ref $o, $depth||b($pack,$depth+1))[0];
}
b('n',0);
is "@scratch", "o n", 
   'recursive calls do not share shared-hash-key TARGs (2)';

# [perl #78194] @_ aliasing op return values
sub { is \$_[0], \$_[0],
        '[perl #78194] \$_[0] == \$_[0] when @_ aliases "$x"' }
 ->("${\''}");

# The return statement should make no difference in this case:
sub not_constant () {        42 }
sub not_constantr() { return 42 }
use feature 'lexical_subs'; no warnings 'experimental::lexical_subs';
my sub not_constantm () {        42 }
my sub not_constantmr() { return 42 }
eval { ${\not_constant}++ };
is $@, "", 'sub (){42} returns a mutable value';
eval { ${\not_constantr}++ };
is $@, "", 'sub (){ return 42 } returns a mutable value';
eval { ${\not_constantm}++ };
is $@, "", 'my sub (){42} returns a mutable value';
eval { ${\not_constantmr}++ };
is $@, "", 'my sub (){ return 42 } returns a mutable value';
is eval {
    sub Crunchy () { 1 }
    sub Munchy { $_[0] = 2 }
    eval "Crunchy"; # test that freeing this op does not turn off PADTMP
    Munchy(Crunchy);
} || $@, 2, 'freeing ops does not make sub(){42} immutable';

# &xsub when @_ has nonexistent elements
{
    no warnings "uninitialized";
    local @_ = ();
    $#_++;
    &utf8::encode;
    is @_, 1, 'num of elems in @_ after &xsub with nonexistent $_[0]';
    is $_[0], "", 'content of nonexistent $_[0] is modified by &xsub';
}

# &xsub when @_ itself does not exist
undef *_;
eval { &utf8::encode };
# The main thing we are testing is that it did not crash.  But make sure 
# *_{ARRAY} was untouched, too.
is *_{ARRAY}, undef, 'goto &xsub when @_ does not exist';

# We do not want re.pm loaded at this point.  Move this test up or find
# another XSUB if this fails.
ok !exists $INC{"re.pm"}, 're.pm not loaded yet';
{
    sub re::regmust{}
    bless \&re::regmust;
    DESTROY {
        no warnings 'redefine', 'prototype';
        my $str1 = "$_[0]";
        *re::regmust = sub{}; # GvSV had no refcount, so this freed it
        my $str2 = "$_[0]";   # used to be UNKNOWN(0x7fdda29310e0)
        @str = ($str1, $str2);
    }
    local $^W; # Suppress redef warnings in XSLoader
    require re;
    is $str[1], $str[0],
      'XSUB clobbering sub whose DESTROY assigns to the glob';
}
{
    no warnings 'redefine';
    sub foo {}
    bless \&foo, 'newATTRSUBbug';
    sub newATTRSUBbug::DESTROY {
        my $str1 = "$_[0]";
        *foo = sub{}; # GvSV had no refcount, so this freed it
        my $str2 = "$_[0]";   # used to be UNKNOWN(0x7fdda29310e0)
        @str = ($str1, $str2);
    }
    splice @str;
    eval "sub foo{}";
    is $str[1], $str[0],
      'Pure-Perl sub clobbering sub whose DESTROY assigns to the glob';
}

# [perl #122107] previously this would return
#  Subroutine BEGIN redefined at (eval 2) line 2.
fresh_perl_is(<<'EOS', "", { stderr => 1 },
use strict; use warnings; eval q/use File::{Spec}/; eval q/use File::Spec/;
EOS
	       "check special blocks are cleared on error");

use constant { constant1 => 1, constant2 => 2 };
{
    my $w;
    local $SIG{__WARN__} = sub { $w++ };
    eval 'sub constant1; sub constant2($)';
    is eval '&constant1', '1',
      'stub re-declaration of constant with no prototype';
    is eval '&constant2', '2',
      'stub re-declaration of constant with wrong prototype';
    is $w, 2, 'two warnings from the above';
}

package _122845 {
    our $depth = 0;
    my $parent; # just to make the sub a closure

    sub {
	local $depth = $depth + 1;
	our $ok++, return if $depth == 2;

	()= $parent;  # just to make the sub a closure
	our $whatever; # this causes the crash

	CORE::__SUB__->();
    }->();
};
is $_122845::ok, 1,
  '[perl #122845] no crash in closure recursion with our-vars';

() = *predeclared; # vivify the glob at compile time
sub predeclared; # now we have a CV stub with no body (incorporeal? :-)
sub predeclared {
    CORE::state $x = 42;
    sub inside_predeclared {
	is eval '$x', 42, 'eval q/$var/ in named sub in predeclared sub';
    }
}
predeclared(); # set $x to 42
$main::x = $main::x = "You should not see this.";
inside_predeclared(); # run test

# RT #126845: this used to fail an assertion in Perl_newATTRSUB_x()
eval 'sub rt126845_1 (); sub rt126845_1 () :lvalue';
pass("RT #126845: stub with prototype, then with attribute");

eval 'sub rt126845_2 (); sub rt126845_2 () :lvalue {}';
pass("RT #126845: stub with prototype, then definition with attribute");

# RT #124156 death during unwinding causes crash
# the tie allows us to trigger another die while cleaning up the stack
# from an earlier die.

{
    package RT124156;

    sub TIEHASH { bless({}, $_[0]) }
    sub EXISTS { 0 }
    sub FETCH { undef }
    sub STORE { }
    sub DELETE { die "outer\n" }

    my @value;
    eval {
        @value = sub {
            @value = sub {
                my %a;
                tie %a, "RT124156";
                local $a{foo} = "bar";
                die "inner";
                ("dd2a", "dd2b");
            }->();
            ("cc3a", "cc3b");
        }->();
    };
    ::is($@, "outer\n", "RT124156 plain");

    my $destroyed = 0;
    sub DESTROY { $destroyed = 1 }

    sub f {
        my $x;
        my $f = sub {
            $x = 1; # force closure
            my %a;
            tie %a, "RT124156";
            local $a{foo} = "bar";
            die "inner";
        };
        bless $f, 'RT124156';
        $f->();
    }

    eval { f(); };
    # as opposed to $@ eq "Can't undef active subroutine"
    ::is($@, "outer\n", "RT124156 depth");
    ::is($destroyed, 1, "RT124156 freed cv");
}

# trapping dying while popping a scope needs to have the right pad at all
# times. Localising a tied array then dying in STORE raises an exception
# while leaving g(). Note that using an object and destructor wouldn't be
# sufficient since DESTROY is called with call_sv(...,G_EVAL).
# We make sure that the first item in every sub's pad is a lexical with
# different values per sub.

{
    package tie_exception;
    sub TIEARRAY { my $x = 4; bless [0] }
    sub FETCH    { my $x = 5; 1 }
    sub STORE    { my $x = 6; die if $_[0][0]; $_[0][0] = 1 }

    my $y;
    sub f { my $x = 7; eval { g() }; $y = $x }
    sub g {
        my $x = 8;
        my @a;
        tie @a, "tie_exception";
        local $a[0];
    }

    f();
    ::is($y, 7, "tie_exception");
}


# check that return pops extraneous stuff from the stack

sub check_ret {
    # the extra scopes push contexts and extra SVs on the stack
    {
        my @a = map $_ + 20, @_;
        for ('x') {
            return if defined $_[0] && $_[0] < 0;
        }
        for ('y') {
            check_ret(1, do { (2,3,4, return @a ? @a[0..$#a] : ()) }, 4.5);
        }
    }
}

is(scalar check_ret(),          undef, "check_ret() scalar");
is(scalar check_ret(5),         25,    "check_ret(5) scalar");
is(scalar check_ret(5,6),       26,    "check_ret(5,6) scalar");
is(scalar check_ret(5,6,7),     27,    "check_ret(5,6,7) scalar");
is(scalar check_ret(5,6,7,8),   28,    "check_ret(5,6,7,8) scalar");
is(scalar check_ret(5,6,7,8,9), 29,    "check_ret(5,6,7,8,9) scalar");

is(scalar check_ret(-1),        undef, "check_ret(-1) scalar");
is(scalar check_ret(-1,5),      undef, "check_ret(-1,5) scalar");

is(join('-', 10, check_ret()),          "10",                "check_ret() list");
is(join('-', 10, check_ret(5)),         "10-25",             "check_ret(5) list");
is(join('-', 10, check_ret(5,6)),       "10-25-26",          "check_ret(5,6) list");
is(join('-', 10, check_ret(5,6,7)),     "10-25-26-27",       "check_ret(5,6,7) list");
is(join('-', 10, check_ret(5,6,7,8)),   "10-25-26-27-28",    "check_ret(5,6,7,8) list");
is(join('-', 10, check_ret(5,6,7,8,9)), "10-25-26-27-28-29", "check_ret(5,6,7,8,9) list");

is(join('-', 10, check_ret(-1)),        "10",  "check_ret(-1) list");
is(join('-', 10, check_ret(-1,5)),      "10",  "check_ret(-1,5) list");

# a sub without nested scopes that still leaves rubbish on the stack
# which needs popping
{
    my @res = sub {
        my $false;
        # conditional leaves rubbish on stack
        return @_ unless $false and $false;
        1;
    }->('a','b');
    is(join('-', @res), "a-b", "unnested rubbish");
}

# a sub should copy returned PADTMPs

{
    sub f99 { $_[0] . "x" };
    my $a = [ f99(1), f99(2) ];
    is("@$a", "1x 2x", "PADTMPs copied on return");
}

# A sub should FREETMPS on exit
# RT #124248

{
    package p124248;
    my $d = 0;
    sub DESTROY { $d++ }
    sub f { ::is($d, 1, "RT 124248"); }
    sub g { !!(my $x = bless []); }
    f(g());
}

# return should have the right PL_curpm while copying its return args

sub curpm {
    "b" =~ /(.)/;
    {
        "c" =~ /(.)/;
        return $1;
    }
}
"a" =~ /(.)/;
is(curpm(), 'c', 'return and PL_curpm');

sub rt_129916 { 42 }
is ref($main::{rt_129916}), 'CODE', 'simple sub stored as CV in stash (main::)';
{
    package RT129916;
    sub foo { 42 }
}
{
    local $::TODO = "disabled for now";
    is ref($RT129916::{foo}), 'CODE', 'simple sub stored as CV in stash (non-main::)';
}

# Calling xsub via ampersand syntax when @_ has holes
SKIP: {
    skip "no XS::APItest on miniperl" if is_miniperl;
    skip "XS::APItest not available", 1 if ! eval { require XS::APItest };
    local *_;
    $_[1] = 1;
    &XS::APItest::unshift_and_set_defav;
    is "@_", "42 43 1"
}

# [perl #129090] Crashes and hangs
watchdog 10;
{ no warnings;
  eval '$a=qq|a$a|;my sub b;%c;sub c{sub b;sub c}';
}
eval '
   ()= %d;
   {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u);}
   {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u);}
   {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u);}
   CORE::state sub b; sub d { sub b {} sub d }
 ';
eval '()=%e; sub e { sub e; eval q|$x| } e;';

fresh_perl_like(
    q#<s,,$0[sub{m]]]],}>0,shift#,
    qr/^syntax error/,
    {},
    "GH Issue #16944 - Syntax error with sub and shift causes segfault"
);

# Bug 20010515.004 (#6998)
# freeing array used as args to sub

fresh_perl_like(
    q{my @h = 1 .. 10; bad(@h); sub bad { undef @h; warn "O\n"; print for @_; warn "K\n";}},
    qr/Use of freed value in iteration/,
    {},
    "#6998 freeing array used as args to sub",
);

# github #21044
ok( eval { $_->{x} = 1 for sub { undef }->(); 1 }, "check sub return values are modifiable")
  or diag $@;
