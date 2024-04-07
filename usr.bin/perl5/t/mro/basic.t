#!./perl

BEGIN {
    chdir 't' if -d 't';
    require q(./test.pl);
    set_up_inc('../lib');
}

use strict;
use warnings;

plan(tests => 66);

require mro;

{
    package MRO_A;
    our @ISA = qw//;
    package MRO_B;
    our @ISA = qw//;
    package MRO_C;
    our @ISA = qw//;
    package MRO_D;
    our @ISA = qw/MRO_A MRO_B MRO_C/;
    package MRO_E;
    our @ISA = qw/MRO_A MRO_B MRO_C/;
    package MRO_F;
    our @ISA = qw/MRO_D MRO_E/;
}

my @MFO_F_DFS = qw/MRO_F MRO_D MRO_A MRO_B MRO_C MRO_E/;
my @MFO_F_C3 = qw/MRO_F MRO_D MRO_E MRO_A MRO_B MRO_C/;
is(mro::get_mro('MRO_F'), 'dfs');
ok(eq_array(
    mro::get_linear_isa('MRO_F'), \@MFO_F_DFS
));

ok(eq_array(mro::get_linear_isa('MRO_F', 'dfs'), \@MFO_F_DFS));
ok(eq_array(mro::get_linear_isa('MRO_F', 'c3'), \@MFO_F_C3));
eval{mro::get_linear_isa('MRO_F', 'C3')};
like($@, qr/^Invalid mro name: 'C3'/);

mro::set_mro('MRO_F', 'c3');
is(mro::get_mro('MRO_F'), 'c3');
ok(eq_array(
    mro::get_linear_isa('MRO_F'), \@MFO_F_C3
));

ok(eq_array(mro::get_linear_isa('MRO_F', 'dfs'), \@MFO_F_DFS));
ok(eq_array(mro::get_linear_isa('MRO_F', 'c3'), \@MFO_F_C3));
eval{mro::get_linear_isa('MRO_F', 'C3')};
like($@, qr/^Invalid mro name: 'C3'/);

my @isarev = sort { $a cmp $b } @{mro::get_isarev('MRO_B')};
ok(eq_array(
    \@isarev,
    [qw/MRO_D MRO_E MRO_F/]
));

ok(!mro::is_universal('MRO_B'));

@UNIVERSAL::ISA = qw/MRO_F/;
ok(mro::is_universal('MRO_B'));

@UNIVERSAL::ISA = ();
ok(!mro::is_universal('MRO_B'));

# is_universal, get_mro, and get_linear_isa should
# handle non-existent packages sanely
ok(!mro::is_universal('Does_Not_Exist'));
is(mro::get_mro('Also_Does_Not_Exist'), 'dfs');
ok(eq_array(
    mro::get_linear_isa('Does_Not_Exist_Three'),
    [qw/Does_Not_Exist_Three/]
));

# Assigning @ISA via globref
{
    package MRO_TestBase;
    sub testfunc { return 123 }
    package MRO_TestOtherBase;
    sub testfunctwo { return 321 }
    package MRO_M; our @ISA = qw/MRO_TestBase/;
}
*MRO_N::ISA = *MRO_M::ISA;
is(eval { MRO_N->testfunc() }, 123);

# XXX TODO (when there's a way to backtrack through a glob's aliases)
# push(@MRO_M::ISA, 'MRO_TestOtherBase');
# is(eval { MRO_N->testfunctwo() }, 321);

# Simple DESTROY Baseline
{
    my $x = 0;
    my $obj;

    {
        package DESTROY_MRO_Baseline;
        sub new { bless {} => shift }
        sub DESTROY { $x++ }

        package DESTROY_MRO_Baseline_Child;
        our @ISA = qw/DESTROY_MRO_Baseline/;
    }

    $obj = DESTROY_MRO_Baseline->new();
    undef $obj;
    is($x, 1);

    $obj = DESTROY_MRO_Baseline_Child->new();
    undef $obj;
    is($x, 2);
}

# Dynamic DESTROY
{
    my $x = 0;
    my $obj;

    {
        package DESTROY_MRO_Dynamic;
        sub new { bless {} => shift }

        package DESTROY_MRO_Dynamic_Child;
        our @ISA = qw/DESTROY_MRO_Dynamic/;
    }

    $obj = DESTROY_MRO_Dynamic->new();
    undef $obj;
    is($x, 0);

    $obj = DESTROY_MRO_Dynamic_Child->new();
    undef $obj;
    is($x, 0);

    no warnings 'once';
    *DESTROY_MRO_Dynamic::DESTROY = sub { $x++ };

    $obj = DESTROY_MRO_Dynamic->new();
    undef $obj;
    is($x, 1);

    $obj = DESTROY_MRO_Dynamic_Child->new();
    undef $obj;
    is($x, 2);
}

# clearing @ISA in different ways
#  some are destructive to the package, hence the new
#  package name each time
{
    no warnings 'uninitialized';
    {
        package ISACLEAR;
        our @ISA = qw/XX YY ZZ/;
    }
    # baseline
    ok(eq_array(mro::get_linear_isa('ISACLEAR'),[qw/ISACLEAR XX YY ZZ/]));

    # this looks dumb, but it preserves existing behavior for compatibility
    #  (undefined @ISA elements treated as "main")
    $ISACLEAR::ISA[1] = undef;
    ok(eq_array(mro::get_linear_isa('ISACLEAR'),[qw/ISACLEAR XX main ZZ/]));

    # undef the array itself
    undef @ISACLEAR::ISA;
    ok(eq_array(mro::get_linear_isa('ISACLEAR'),[qw/ISACLEAR/]));

    # Now, clear more than one package's @ISA at once
    {
        package ISACLEAR1;
        our @ISA = qw/WW XX/;

        package ISACLEAR2;
        our @ISA = qw/YY ZZ/;
    }
    # baseline
    ok(eq_array(mro::get_linear_isa('ISACLEAR1'),[qw/ISACLEAR1 WW XX/]));
    ok(eq_array(mro::get_linear_isa('ISACLEAR2'),[qw/ISACLEAR2 YY ZZ/]));
    (@ISACLEAR1::ISA, @ISACLEAR2::ISA) = ();

    ok(eq_array(mro::get_linear_isa('ISACLEAR1'),[qw/ISACLEAR1/]));
    ok(eq_array(mro::get_linear_isa('ISACLEAR2'),[qw/ISACLEAR2/]));

    # [perl #49564]  This is a pretty obscure way of clearing @ISA but
    # it tests a regression that affects XS code calling av_clear too.
    {
        package ISACLEAR3;
        our @ISA = qw/WW XX/;
    }
    ok(eq_array(mro::get_linear_isa('ISACLEAR3'),[qw/ISACLEAR3 WW XX/]));
    {
        package ISACLEAR3;
        reset 'I';
    }
    ok(eq_array(mro::get_linear_isa('ISACLEAR3'),[qw/ISACLEAR3/]));
}

# Check that recursion bails out "cleanly" in a variety of cases
# (as opposed to say, bombing the interpreter or something)
{
    my @recurse_codes = (
        '@MRO_R1::ISA = "MRO_R2"; @MRO_R2::ISA = "MRO_R1";',
        '@MRO_R3::ISA = "MRO_R4"; push(@MRO_R4::ISA, "MRO_R3");',
        '@MRO_R5::ISA = "MRO_R6"; @MRO_R6::ISA = qw/XX MRO_R5 YY/;',
        '@MRO_R7::ISA = "MRO_R8"; push(@MRO_R8::ISA, qw/XX MRO_R7 YY/)',
    );
    foreach my $code (@recurse_codes) {
        eval $code;
        ok($@ =~ /Recursive inheritance detected/);
    }
}

# Check that SUPER caches get invalidated correctly
{
    {
        package SUPERTEST;
        sub new { bless {} => shift }
        sub foo { $_[1]+1 }

        package SUPERTEST::MID;
        our @ISA = 'SUPERTEST';

        package SUPERTEST::KID;
        our @ISA = 'SUPERTEST::MID';
        sub foo { my $s = shift; $s->SUPER::foo(@_) }

        package SUPERTEST::REBASE;
        sub foo { $_[1]+3 }
    }

    my $stk_obj = SUPERTEST::KID->new();
    is($stk_obj->foo(1), 2);
    { no warnings 'redefine';
      *SUPERTEST::foo = sub { $_[1]+2 };
    }
    is($stk_obj->foo(2), 4);
    @SUPERTEST::MID::ISA = 'SUPERTEST::REBASE';
    is($stk_obj->foo(3), 6);
}

{ 
  {
    # assigning @ISA via arrayref to globref RT 60220
    package P1;
    sub new { bless {}, shift }
    
    package P2;
  }
  *{P2::ISA} = [ 'P1' ];
  my $foo = P2->new;
  ok(!eval { $foo->bark }, "no bark method");
  no warnings 'once';  # otherwise it'll bark about P1::bark used only once
  *{P1::bark} = sub { "[bark]" };
  is(scalar eval { $foo->bark }, "[bark]", "can bark now");
}

{
  # assigning @ISA via arrayref then modifying it RT 72866
  {
    package Q1;
    sub foo {  }

    package Q2;
    sub bar { }

    package Q3;
  }
  push @Q3::ISA, "Q1";
  can_ok("Q3", "foo");
  *Q3::ISA = [];
  push @Q3::ISA, "Q1";
  can_ok("Q3", "foo");
  *Q3::ISA = [];
  push @Q3::ISA, "Q2";
  can_ok("Q3", "bar");
  ok(!Q3->can("foo"), "can't call foo method any longer");
}

{
    # test mro::method_changed_in
    my $count = mro::get_pkg_gen("MRO_A");
    mro::method_changed_in("MRO_A");
    my $count_new = mro::get_pkg_gen("MRO_A");

    is($count_new, $count + 1);
}

{
    # test if we can call mro::invalidate_all_method_caches;
    eval {
        mro::invalidate_all_method_caches();
    };
    is($@, "");
}

{
    # @main::ISA
    no warnings 'once';
    @main::ISA = 'parent';
    my $output = '';
    *parent::do = sub { $output .= 'parent' };
    *parent2::do = sub { $output .= 'parent2' };
    main->do;
    @main::ISA = 'parent2';
    main->do;
    is $output, 'parentparent2', '@main::ISA is magical';
}

{
    # Undefining *ISA, then modifying @ISA
    # This broke Class::Trait. See [perl #79024].
    {package Class::Trait::Base}
    no strict 'refs';
    undef   *{"Extra::TSpouse::ISA"};
    'Extra::TSpouse'->isa('Class::Trait::Base'); # cache the mro
    unshift @{"Extra::TSpouse::ISA"}, 'Class::Trait::Base';
    ok 'Extra::TSpouse'->isa('Class::Trait::Base'),
     'a isa b after undef *a::ISA and @a::ISA modification';
}

{
    # Deleting $package::{ISA}
    # Broken in 5.10.0; fixed in 5.13.7
    @Blength::ISA = 'Bladd';
    delete $Blength::{ISA};
    ok !Blength->isa("Bladd"), 'delete $package::{ISA}';
}

{
    # Undefining stashes
    @Thrext::ISA = "Thwit";
    @Thwit::ISA = "Sile";
    undef %Thwit::;
    ok !Thrext->isa('Sile'), 'undef %package:: updates subclasses';
}

{
    # Obliterating @ISA via glob assignment
    # Broken in 5.14.0; fixed in 5.17.2
    @Gwythaint::ISA = "Fantastic::Creature";
    undef *This_glob_haD_better_not_exist; # paranoia; must have no array
    *Gwythaint::ISA = *This_glob_haD_better_not_exist;
    ok !Gwythaint->isa("Fantastic::Creature"),
       'obliterating @ISA via glob assignment';
}

{
    # Autovivifying @ISA via @{*ISA}
    no warnings;
    undef *fednu::ISA;
    @{*fednu::ISA} = "pyfg";
    ok +fednu->isa("pyfg"), 'autovivifying @ISA via *{@ISA}';
}

{
    sub Detached::method;
    my $h = delete $::{"Detached::"};
    eval { local *Detached::method };
    is $@, "", 'localising gv-with-cv belonging to detached package';
}

{
    # *ISA localisation
    @il::ISA = "ilsuper";
    sub ilsuper::can { "puree" }
    sub il::tomatoes;
    {
        local *il::ISA;
        is +il->can("tomatoes"), \&il::tomatoes, 'local *ISA';
    }
    is "il"->can("tomatoes"), "puree", 'local *ISA unwinding';
    {
        local *il::ISA = [];
        is +il->can("tomatoes"), \&il::tomatoes, 'local *ISA = []';
    }
    is "il"->can("tomatoes"), "puree", 'local *ISA=[] unwinding';
}

# Changes to UNIVERSAL::DESTROY should not leave stale DESTROY caches
# (part of #114864)
our $destroy_output;
sub UNIVERSAL::DESTROY { $destroy_output = "old" }
my $x = bless[];
undef $x; # cache the DESTROY method
undef *UNIVERSAL::DESTROY;
*UNIVERSAL::DESTROY = sub { $destroy_output = "new" };
$x = bless[];
undef $x; # should use the new DESTROY
is $destroy_output, "new",
    'Changes to UNIVERSAL::DESTROY invalidate DESTROY caches';
undef *UNIVERSAL::DESTROY;

{
    no warnings 'uninitialized';
    $#_119433::ISA++;
    pass "no crash when ISA contains nonexistent elements";
}

{ # 123788
    fresh_perl_is(<<'PROG', "ok", {}, "don't crash when deleting ISA");
$x = \@{q(Foo::ISA)};
delete $Foo::{ISA};
@$x = "Bar";
print "ok\n";
PROG

    # when there are multiple references to an ISA array, the mg_obj
    # turns into an AV of globs, which is a different code path
    # this test only crashes on -DDEBUGGING builds
    fresh_perl_is(<<'PROG', "ok", {}, "a case with multiple refs to ISA");
@Foo::ISA = qw(Abc Def);
$x = \@{q(Foo::ISA)};
*Bar::ISA = $x;
delete $Bar::{ISA};
delete $Foo::{ISA};
++$y;
$x->[1] = "Ghi";
@$x = "Bar";
print "ok\n";
PROG

    # reverse order of delete to exercise removing from the other end
    # of the array
    # again, may only crash on -DDEBUGGING builds
    fresh_perl_is(<<'PROG', "ok", {}, "a case with multiple refs to ISA");
$x = \@{q(Foo::ISA)};
*Bar::ISA = $x;
delete $Foo::{ISA};
delete $Bar::{ISA};
++$y;
@$x = "Bar";
print "ok\n";
PROG
}

{
    # [perl #127351]
    # *Foo::ISA = \@some_array
    # didn't magicalize the elements of @some_array, causing two
    # problems:

    #  a) assignment to those elements didn't update the cache

    fresh_perl_is(<<'PROG', "foo\nother", {}, "magical *ISA = arrayref elements");
*My::Parent::foo = sub { "foo" };
*My::OtherParent::foo = sub { "other" };
my $x = [ "My::Parent" ];
*Fake::ISA = $x;
print Fake->foo, "\n";
$x->[0] = "My::OtherParent";
print Fake->foo, "\n";
PROG

    #  b) code that attempted to remove the magic when @some_array
    #     was no longer an @ISA asserted/crashed

    fresh_perl_is(<<'PROG', "foo", {}, "unmagicalize *ISA elements");
{
    local *My::Parent::foo = sub { "foo" };
    my $x = [ "My::Parent" ];
    *Fake::ISA = $x;
    print Fake->foo, "\n";
    my $s = \%Fake::;
    delete $s->{ISA};
}
PROG
}
