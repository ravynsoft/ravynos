#!./perl -w

#
# test method calls and autoloading.
#

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(. ../lib lib ../dist/base/lib) );
}

use strict;
no warnings 'once';

plan(tests => 163);

{
    # RT #126042 &{1==1} * &{1==1} would crash
    # There are two issues here.  Method lookup yields a fake method for
    # ->import or ->unimport if there's no actual method, for historical
    # reasons so that "use" doesn't barf if there's no import method.
    # The first bug, the one which caused the crash, is that the fake
    # method was broken in scalar context, messing up the stack.  We test
    # for that on its own.
    foreach my $meth (qw(import unimport)) {
	is join(",", map { $_ // "u" } "a", "b", "Unknown"->$meth, "c", "d"), "a,b,c,d", "Unknown->$meth in list context";
	is join(",", map { $_ // "u" } "a", "b", scalar("Unknown"->$meth), "c", "d"), "a,b,u,c,d", "Unknown->$meth in scalar context";
    }
    # The second issue is that the fake method wasn't actually a CV or
    # anything referencing a CV, but was &PL_sv_yes being used as a magic
    # placeholder.  That's inconsistent with &PL_sv_yes being a string,
    # which we'd expect to serve as a symbolic CV ref.  This test must
    # come before AUTOLOAD gets set up below.
    foreach my $one (1, !!1) {
	my @res = eval { no strict "refs"; &$one() };
	like $@, qr/\AUndefined subroutine \&main::1 called at /;
	@res = eval { no strict "refs"; local *1 = sub { 123 }; &$one() };
	is $@, "";
	is "@res", "123";
	@res = eval { &$one() };
	like $@, qr/\ACan't use string \("1"\) as a subroutine ref while "strict refs" in use at /;
    }
}

@A::ISA = 'BB';
@BB::ISA = 'C';

sub C::d {"C::d"}
sub D::d {"D::d"}

# First, some basic checks of method-calling syntax:
my $obj = bless [], "Pack";
sub Pack::method { shift; join(",", "method", @_) }
my $mname = "method";

is(Pack->method("a","b","c"), "method,a,b,c");
is(Pack->$mname("a","b","c"), "method,a,b,c");
is(method Pack ("a","b","c"), "method,a,b,c");
is((method Pack "a","b","c"), "method,a,b,c");

is(Pack->method(), "method");
is(Pack->$mname(), "method");
is(method Pack (), "method");
is(Pack->method, "method");
is(Pack->$mname, "method");
is(method Pack, "method");

is($obj->method("a","b","c"), "method,a,b,c");
is($obj->$mname("a","b","c"), "method,a,b,c");
is((method $obj ("a","b","c")), "method,a,b,c");
is((method $obj "a","b","c"), "method,a,b,c");

is($obj->method(0), "method,0");
is($obj->method(1), "method,1");

is($obj->method(), "method");
is($obj->$mname(), "method");
is((method $obj ()), "method");
is($obj->method, "method");
is($obj->$mname, "method");
is(method $obj, "method");

is( A->d, "C::d");		# Update hash table;

*BB::d = \&D::d;			# Import now.
is(A->d, "D::d");		# Update hash table;

{
    local @A::ISA = qw(C);	# Update hash table with split() assignment
    is(A->d, "C::d");
    $#A::ISA = -1;
    is(eval { A->d } || "fail", "fail");
}
is(A->d, "D::d");

{
    local *BB::d;
    eval 'sub BB::d {"BB::d1"}';	# Import now.
    is(A->d, "BB::d1");	# Update hash table;
    undef &BB::d;
    is((eval { A->d }, ($@ =~ /Undefined subroutine/)), 1);
}

is(A->d, "D::d");		# Back to previous state

eval 'no warnings "redefine"; sub BB::d {"BB::d2"}';	# Import now.
is(A->d, "BB::d2");		# Update hash table;

# What follows is hardly guarantied to work, since the names in scripts
# are already linked to "pruned" globs. Say, 'undef &BB::d' if it were
# after 'delete $BB::{d}; sub BB::d {}' would reach an old subroutine.

undef &BB::d;
delete $BB::{d};
is(A->d, "C::d");

eval 'sub BB::d {"BB::d2.5"}';
A->d;				# Update hash table;
my $glob = \delete $BB::{d};	# non-void context; hang on to the glob
is(A->d, "C::d");		# Update hash table;

eval 'sub BB::d {"BB::d3"}';	# Import now.
is(A->d, "BB::d3");		# Update hash table;

delete $BB::{d};
*dummy::dummy = sub {};		# Mark as updated
is(A->d, "C::d");

eval 'sub BB::d {"BB::d4"}';	# Import now.
is(A->d, "BB::d4");		# Update hash table;

delete $BB::{d};			# Should work without any help too
is(A->d, "C::d");

{
    local *C::d;
    is(eval { A->d } || "nope", "nope");
}
is(A->d, "C::d");

*A::x = *A::d;
A->d;
is(eval { A->x } || "nope", "nope", 'cache should not follow synonyms');

my $counter;

eval <<'EOF';
sub C::e;
BEGIN { *BB::e = \&C::e }	# Shouldn't prevent AUTOLOAD in original pkg
sub Y::f;
$counter = 0;

@X::ISA = 'Y';
@Y::ISA = 'BB';

sub BB::AUTOLOAD {
  my $c = ++$counter;
  my $method = $BB::AUTOLOAD;
  my $msg = "B: In $method, $c";
  eval "sub $method { \$msg }";
  goto &$method;
}
sub C::AUTOLOAD {
  my $c = ++$counter;
  my $method = $C::AUTOLOAD;
  my $msg = "C: In $method, $c";
  eval "sub $method { \$msg }";
  goto &$method;
}
EOF

is(A->e(), "C: In C::e, 1");	# We get a correct autoload
is(A->e(), "C: In C::e, 1");	# Which sticks

is(A->ee(), "B: In A::ee, 2"); # We get a generic autoload, method in top
is(A->ee(), "B: In A::ee, 2"); # Which sticks

is(Y->f(), "B: In Y::f, 3");	# We vivify a correct method
is(Y->f(), "B: In Y::f, 3");	# Which sticks

# This test is not intended to be reasonable. It is here just to let you
# know that you broke some old construction. Feel free to rewrite the test
# if your patch breaks it.

{
no warnings 'redefine';
*BB::AUTOLOAD = sub {
  use warnings;
  my $c = ++$counter;
  my $method = $::AUTOLOAD;
  no strict 'refs';
  *$::AUTOLOAD = sub { "new B: In $method, $c" };
  goto &$::AUTOLOAD;
};
}

is(A->eee(), "new B: In A::eee, 4");	# We get a correct $autoload
is(A->eee(), "new B: In A::eee, 4");	# Which sticks

# test that failed subroutine calls don't affect method calls
{
    package A1;
    sub foo { "foo" }
    package A2;
    @A2::ISA = 'A1';
    package main;
    is(A2->foo(), "foo");
    is(do { eval 'A2::foo()'; $@ ? 1 : 0}, 1);
    is(A2->foo(), "foo");
}

## This test was totally misguided.  It passed before only because the
## code to determine if a package was loaded used to look for the hash
## %Foo::Bar instead of the package Foo::Bar:: -- and Config.pm just
## happens to export %Config.
#  {
#      is(do { use Config; eval 'Config->foo()';
#  	      $@ =~ /^\QCan't locate object method "foo" via package "Config" at/ ? 1 : $@}, 1);
#      is(do { use Config; eval '$d = bless {}, "Config"; $d->foo()';
#  	      $@ =~ /^\QCan't locate object method "foo" via package "Config" at/ ? 1 : $@}, 1);
#  }

# test error messages if method loading fails
my $e;

eval '$e = bless {}, "E::A"; E::A->foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::A" at/);
eval '$e = bless {}, "E::B"; $e->foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::B" at/);
eval 'E::C->foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::C" (perhaps /);

eval 'UNIVERSAL->E::D::foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::D" (perhaps /);
eval 'my $e = bless {}, "UNIVERSAL"; $e->E::E::foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::E" (perhaps /);

$e = bless {}, "E::F";  # force package to exist
eval 'UNIVERSAL->E::F::foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::F" at/);
eval '$e = bless {}, "UNIVERSAL"; $e->E::F::foo()';
like ($@, qr/^\QCan't locate object method "foo" via package "E::F" at/);

# SUPER:: pseudoclass
@Saab::ISA = "Souper";
sub Souper::method { @_ }
@OtherSaab::ISA = "OtherSouper";
sub OtherSouper::method { "Isidore Ropen, Draft Manager" }
{
   my $o = bless [], "Saab";
   package Saab;
   my @ret = $o->SUPER::method('whatever');
   ::is $ret[0], $o, 'object passed to SUPER::method';
   ::is $ret[1], 'whatever', 'argument passed to SUPER::method';
   {
       no warnings qw(syntax deprecated);
       @ret = $o->SUPER'method('whatever');
   }
   ::is $ret[0], $o, "object passed to SUPER'method";
   ::is $ret[1], 'whatever', "argument passed to SUPER'method";
   @ret = Saab->SUPER::method;
   ::is $ret[0], 'Saab', "package name passed to SUPER::method";
   @ret = OtherSaab->SUPER::method;
   ::is $ret[0], 'OtherSaab',
      "->SUPER::method uses current package, not invocant";
}
() = *SUPER::;
{
   local our @ISA = "Souper";
   is eval { (main->SUPER::method)[0] }, 'main',
      'Mentioning *SUPER:: does not stop ->SUPER from working in main';
}
{
    BEGIN {
        *Mover:: = *Mover2::;
        *Mover2:: = *foo;
    }
    package Mover;
    no strict;
    # Not our(@ISA), because the bug we are testing for interacts with an
    # our() bug that cancels this bug out.
    @ISA = 'door';
    sub door::dohtem { 'dohtem' }
    ::is eval { Mover->SUPER::dohtem; }, 'dohtem',
        'SUPER inside moved package';
    undef *door::dohtem;
    *door::dohtem = sub { 'method' };
    ::is eval { Mover->SUPER::dohtem; }, 'method',
        'SUPER inside moved package respects method changes';
}

package foo120694 {
    BEGIN { our @ISA = qw(bar120694) }

    sub AUTOLOAD {
        my $self = shift;
        local our $recursive = $recursive;
        return "recursive" if $recursive++;
        return if our $AUTOLOAD eq 'DESTROY';
        $AUTOLOAD = "SUPER:" . substr $AUTOLOAD, rindex($AUTOLOAD, ':');
        return $self->$AUTOLOAD(@_);
    }
}
package bar120694 {
    sub AUTOLOAD {
        return "xyzzy";
    }
}
is bless( [] => "foo120694" )->plugh, 'xyzzy',
    '->SUPER::method autoloading uses parent of current pkg';


# failed method call or UNIVERSAL::can() should not autovivify packages
is( $::{"Foo::"} || "none", "none");  # sanity check 1
is( $::{"Foo::"} || "none", "none");  # sanity check 2

is( UNIVERSAL::can("Foo", "boogie") ? "yes":"no", "no" );
is( $::{"Foo::"} || "none", "none");  # still missing?

is( Foo->UNIVERSAL::can("boogie")   ? "yes":"no", "no" );
is( $::{"Foo::"} || "none", "none");  # still missing?

is( Foo->can("boogie")              ? "yes":"no", "no" );
is( $::{"Foo::"} || "none", "none");  # still missing?

is( eval 'Foo->boogie(); 1'         ? "yes":"no", "no" );
is( $::{"Foo::"} || "none", "none");  # still missing?

is(do { eval 'Foo->boogie()';
	  $@ =~ /^\QCan't locate object method "boogie" via package "Foo" (perhaps / ? 1 : $@}, 1);

eval 'sub Foo::boogie { "yes, sir!" }';
is( $::{"Foo::"} ? "ok" : "none", "ok");  # should exist now
is( Foo->boogie(), "yes, sir!");

# TODO: universal.t should test NoSuchPackage->isa()/can()

# This is actually testing parsing of indirect objects and undefined subs
#   print foo("bar") where foo does not exist is not an indirect object.
#   print foo "bar"  where foo does not exist is an indirect object.
eval 'sub AUTOLOAD { "ok ", shift, "\n"; }';
ok(1);

# Bug ID 20010902.002 (#7609)
is(
    eval q[
	my $x = 'x'; # Lexical or package variable, 5.6.1 panics.
	sub Foo::x : lvalue { $x }
	Foo->$x = 'ok';
    ] || $@, 'ok'
);

# An autoloaded, inherited DESTROY may be invoked differently than normal
# methods, and has been known to give rise to spurious warnings
# eg <200203121600.QAA11064@gizmo.fdgroup.co.uk>

{
    use warnings;
    my $w = '';
    local $SIG{__WARN__} = sub { $w = $_[0] };

    sub AutoDest::Base::AUTOLOAD {}
    @AutoDest::ISA = qw(AutoDest::Base);
    { my $x = bless {}, 'AutoDest'; }
    $w =~ s/\n//g;
    is($w, '');
}

# [ID 20020305.025 (#8788)] PACKAGE::SUPER doesn't work anymore

package main;
our @X;
package Amajor;
sub test {
    push @main::X, 'Amajor', @_;
}
package Bminor;
use base qw(Amajor);
package main;
sub Bminor::test {
    $_[0]->Bminor::SUPER::test('x', 'y');
    push @main::X, 'Bminor', @_;
}
Bminor->test('y', 'z');
is("@X", "Amajor Bminor x y Bminor Bminor y z");

package main;
for my $meth (['Bar', 'Foo::Bar'],
	      ['SUPER::Bar', 'main::SUPER::Bar'],
	      ['Xyz::SUPER::Bar', 'Xyz::SUPER::Bar'])
{
    fresh_perl_is(<<EOT,
package UNIVERSAL; sub AUTOLOAD { my \$c = shift; print "\$c \$AUTOLOAD\\n" }
sub DESTROY {} # prevent AUTOLOAD being called on DESTROY
package Xyz;
package main; Foo->$meth->[0]();
EOT
	"Foo $meth->[1]",
	{ switches => [ '-w' ] },
	"check if UNIVERSAL::AUTOLOAD works",
    );
}

# Test for #71952: crash when looking for a nonexistent destructor
# Regression introduced by fbb3ee5af3d4
{
    fresh_perl_is(<<'EOT',
sub M::DESTROY; bless {}, "M" ; print "survived\n";
EOT
    "survived",
    {},
	"no crash with a declared but missing DESTROY method"
    );
}

# Test for calling a method on a packag name return by a magic variable
sub TIESCALAR{bless[]}
sub FETCH{"main"}
my $kalled;
sub bolgy { ++$kalled; }
tie my $a, "";
$a->bolgy;
is $kalled, 1, 'calling a class method via a magic variable';

{
    package NulTest;
    sub method { 1 }

    package main;
    eval {
        NulTest->${ \"method\0Whoops" };
    };
    like $@, qr/Can't locate object method "method\\0Whoops" via package "NulTest" at/,
            "method lookup is nul-clean";

    *NulTest::AUTOLOAD = sub { our $AUTOLOAD; return $AUTOLOAD };

    like(NulTest->${ \"nul\0test" }, qr/nul\0test/, "AUTOLOAD is nul-clean");
}


{
    fresh_perl_is(
    q! sub T::DESTROY { $x = $_[0]; } bless [], "T";!,
    "DESTROY created new reference to dead object 'T' during global destruction.",
    {},
	"DESTROY creating a new reference to the object generates a warning."
    );
}

# [perl #43663]
{
    $::{"Just"} = \1;
    sub Just::a_japh { return "$_[0] another Perl hacker," }
    is eval { "Just"->a_japh }, "Just another Perl hacker,",
	'constants do not interfere with class methods';
}

# [perl #109264]
{
    no strict 'vars';
    sub bliggles { 1 }
    sub lbiggles :lvalue { index "foo", "f" }
    ok eval { main->bliggles(my($foo,$bar)) },
      'foo->bar(my($foo,$bar)) is not called in lvalue context';
    ok eval { main->bliggles(our($foo,$bar)) },
      'foo->bar(our($foo,$bar)) is not called in lvalue context';
    ok eval { main->bliggles(local($foo,$bar)) },
      'foo->bar(local($foo,$bar)) is not called in lvalue context';
    ok eval { () = main->lbiggles(my($foo,$bar)); 1 },
      'foo->lv(my($foo,$bar)) is not called in lvalue context';
    ok eval { () = main->lbiggles(our($foo,$bar)); 1 },
      'foo->lv(our($foo,$bar)) is not called in lvalue context';
    ok eval { () = main->lbiggles(local($foo,$bar)); 1 },
      'foo->lv(local($foo,$bar)) is not called in lvalue context';
}

{
   # AUTOLOAD and DESTROY can be declared without a leading sub,
   # like BEGIN and friends.
   package NoSub;

   eval 'AUTOLOAD { our $AUTOLOAD; return $AUTOLOAD }';
   ::ok( !$@, "AUTOLOAD without a leading sub is legal" );

   eval "DESTROY { ::pass( q!DESTROY without a leading sub is legal and gets called! ) }";
   {
      ::ok( NoSub->can("AUTOLOAD"), "...and sets up an AUTOLOAD normally" );
      ::is( eval { NoSub->bluh }, "NoSub::bluh", "...which works as expected" );
   }
   { bless {}, "NoSub"; }
}

{
    # [perl #124387]
    my $autoloaded;
    package AutoloadDestroy;
    sub AUTOLOAD { $autoloaded = 1 }
    package main;
    bless {}, "AutoloadDestroy";
    ok($autoloaded, "AUTOLOAD called for DESTROY");

    # 127494 - AUTOLOAD for DESTROY was called without setting $AUTOLOAD
    my %methods;
    package AutoloadDestroy2;
    sub AUTOLOAD {
        our $AUTOLOAD;
        (my $method = $AUTOLOAD) =~ s/.*:://;
        ++$methods{$method};
    }
    package main;
    # this cached AUTOLOAD as the DESTROY method
    bless {}, "AutoloadDestroy2";
    %methods = ();
    my $o = bless {}, "AutoloadDestroy2";
    # this sets $AUTOLOAD to "AutoloadDestroy2::foo"
    $o->foo;
    # this would call AUTOLOAD without setting $AUTOLOAD
    undef $o;
    ok($methods{DESTROY}, "\$AUTOLOAD set correctly for DESTROY");
}

eval { () = 3; new {} };
like $@,
     qr/^Can't call method "new" without a package or object reference/,
    'Err msg from new{} when stack contains a number';
eval { () = "foo"; new {} };
like $@,
     qr/^Can't call method "new" without a package or object reference/,
    'Err msg from new{} when stack contains a word';
eval { () = undef; new {} };
like $@,
     qr/^Can't call method "new" without a package or object reference/,
    'Err msg from new{} when stack contains undef';

package egakacp {
  our @ISA = 'ASI';
  sub ASI::m { shift; "@_" };
  my @a = (bless([]), 'arg');
  my $r = SUPER::m{@a};
  ::is $r, 'arg', 'method{@array}';
  $r = SUPER::m{}@a;
  ::is $r, 'arg', 'method{}@array';
  $r = SUPER::m{@a}"b";
  ::is $r, 'arg b', 'method{@array}$more_args';
}

# [perl #114924] SUPER->method
@SUPER::ISA = "SUPPER";
sub SUPPER::foo { "supper" }
is "SUPER"->foo, 'supper', 'SUPER->method';

sub flomp { "flimp" }
sub main::::flomp { "flump" }
is "::"->flomp, 'flump', 'method call on ::';
is "::main"->flomp, 'flimp', 'method call on ::main';
eval { ""->flomp };
like $@,
     qr/^Can't call method "flomp" without a package or object reference/,
    'method call on empty string';
is "3foo"->CORE::uc, '3FOO', '"3foo"->CORE::uc';
{ no strict; @{"3foo::ISA"} = "CORE"; }
is "3foo"->uc, '3FOO', '"3foo"->uc (autobox style!)';

# *foo vs (\*foo)
sub myclass::squeak { 'eek' }
eval { *myclass->squeak };
like $@,
     qr/^Can't call method "squeak" without a package or object reference/,
    'method call on typeglob ignores package';
eval { (\*myclass)->squeak };
like $@,
     qr/^Can't call method "squeak" on unblessed reference/,
    'method call on \*typeglob';
*stdout2 = *STDOUT;  # stdout2 now stringifies as *main::STDOUT
 sub IO::Handle::self { $_[0] }
# This used to stringify the glob:
is *stdout2->self, (\*stdout2)->self,
  '*glob->method is equiv to (\*glob)->method';
sub { $_[0] = *STDOUT; is $_[0]->self, \$::h{k}, '$pvlv_glob->method' }
 ->($::h{k});

# Test that PL_stashcache doesn't change the resolution behaviour for file
# handles and package names.
SKIP: {
    skip_if_miniperl('file handles as methods requires loading IO::File', 26);
    require Fcntl;

    foreach (qw (Count::DATA Count Colour::H1 Color::H1 C3::H1)) {
	eval qq{
            package $_;

            sub getline {
                return "method in $_";
            }

            1;
        } or die $@;
    }

    BEGIN {
	*The::Count:: = \*Count::;
    }

    is(Count::DATA->getline(), 'method in Count::DATA',
       'initial resolution is a method');
    is(The::Count::DATA->getline(), 'method in Count::DATA',
       'initial resolution is a method in aliased classes');

    require Count;

    is(Count::DATA->getline(), "one! ha ha ha\n", 'file handles take priority');
    is(The::Count::DATA->getline(), "two! ha ha ha\n",
       'file handles take priority in aliased classes');

    eval q{close Count::DATA} or die $!;

    {
	no warnings 'io';
	is(Count::DATA->getline(), undef,
	   "closing a file handle doesn't change object resolution");
	is(The::Count::DATA->getline(), undef,
	   "closing a file handle doesn't change object resolution in aliased classes");
}

    undef *Count::DATA;
    is(Count::DATA->getline(), 'method in Count::DATA',
       'undefining the typeglob does change object resolution');
    is(The::Count::DATA->getline(), 'method in Count::DATA',
       'undefining the typeglob does change object resolution in aliased classes');

    is(Count->getline(), 'method in Count',
       'initial resolution is a method');
    is(The::Count->getline(), 'method in Count',
       'initial resolution is a method in aliased classes');

    eval q{
        open Count, '<', $INC{'Count.pm'}
            or die "Can't open $INC{'Count.pm'}: $!";
1;
    } or die $@;

    is(Count->getline(), "# zero! ha ha ha\n", 'file handles take priority');
    is(The::Count->getline(), 'method in Count', 'but not in an aliased class');

    eval q{close Count} or die $!;

    {
	no warnings 'io';
	is(Count->getline(), undef,
	   "closing a file handle doesn't change object resolution");
    }

    undef *Count;
    is(Count->getline(), 'method in Count',
       'undefining the typeglob does change object resolution');

    open Colour::H1, 'op/method.t' or die $!;
    while (<Colour::H1>) {
	last if /^__END__/;
    }
    open CLOSED, 'TEST' or die $!;
    close CLOSED or die $!;

    my $fh_start = tell Colour::H1;
    my $data_start = tell DATA;
    is(Colour::H1->getline(), <DATA>, 'read from a file');
    is(Color::H1->getline(), 'method in Color::H1',
       'initial resolution is a method');

    *Color::H1 = *Colour::H1{IO};

    is(Colour::H1->getline(), <DATA>, 'read from a file');
    is(Color::H1->getline(), <DATA>,
       'file handles take priority after io-to-typeglob assignment');

    *Color::H1 = *CLOSED{IO};
    {
	no warnings 'io';
	is(Color::H1->getline(), undef,
	   "assigning a closed a file handle doesn't change object resolution");
    }

    undef *Color::H1;
    is(Color::H1->getline(), 'method in Color::H1',
       'undefining the typeglob does change object resolution');

    *Color::H1 = *Colour::H1;

    is(Color::H1->getline(), <DATA>,
       'file handles take priority after typeglob-to-typeglob assignment');

    seek Colour::H1, $fh_start, Fcntl::SEEK_SET() or die $!;
    seek DATA, $data_start, Fcntl::SEEK_SET() or die $!;

    is(Colour::H1->getline(), <DATA>, 'read from a file');
    is(C3::H1->getline(), 'method in C3::H1', 'initial resolution is a method');

    *Copy:: = \*C3::;
    *C3:: = \*Colour::;

    is(Colour::H1->getline(), <DATA>, 'read from a file');
    is(C3::H1->getline(), <DATA>,
       'file handles take priority after stash aliasing');

    *C3:: = \*Copy::;

    is(C3::H1->getline(), 'method in C3::H1',
       'restoring the stash returns to a method');
}

# RT #123619 constant class name should be read-only

{
    sub RT123619::f { chop $_[0] }
    eval { 'RT123619'->f(); };
    like ($@, qr/Modification of a read-only value attempted/, 'RT #123619');
}

{
    fresh_perl_is(<<'PROG', <<'EXPECT', {}, "don't negative cache NOUNIVERSAL lookups");
use v5.36;

my $foo;

BEGIN {
    $foo = bless {}, 'Foo';
    $foo->isa('Foo') and say "->isa works!";
 }

# bump PL_sub_generation
local *Foo::DESTROY = sub {};
undef &Foo::DESTROY;
local *Foo::DESTROY = sub {};

$foo isa 'Foo' and say " and isa works!";
$foo->isa('Foo') and say "->isa works!";

PROG
->isa works!
 and isa works!
->isa works!
EXPECT
}

# RT#130496: assertion failure when looking for a method of undefined name
# on an unblessed reference
fresh_perl_is('eval { {}->$x }; print $@;',
              "Can't call method \"\" on unblessed reference at - line 1.",
              {},
              "no crash with undef method name on unblessed ref");

__END__
#FF9900
#F78C08
#FFA500
#FF4D00
#FC5100
#FF5D00
