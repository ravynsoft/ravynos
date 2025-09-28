#!./perl

use utf8;
use open qw( :utf8 :std );
use strict;
use warnings;

BEGIN { require q(./test.pl); } plan(tests => 53);

require mro;

{
    package MRO_அ;
    our @ISA = qw//;
    package MRO_ɓ;
    our @ISA = qw//;
    package MRO_ᶝ;
    our @ISA = qw//;
    package MRO_ｄ;
    our @ISA = qw/MRO_அ MRO_ɓ MRO_ᶝ/;
    package MRO_ɛ;
    our @ISA = qw/MRO_அ MRO_ɓ MRO_ᶝ/;
    package MRO_ᚠ;
    our @ISA = qw/MRO_ｄ MRO_ɛ/;
}

my @MFO_ᚠ_DFS = qw/MRO_ᚠ MRO_ｄ MRO_அ MRO_ɓ MRO_ᶝ MRO_ɛ/;
my @MFO_ᚠ_C3 = qw/MRO_ᚠ MRO_ｄ MRO_ɛ MRO_அ MRO_ɓ MRO_ᶝ/;
is(mro::get_mro('MRO_ᚠ'), 'dfs');
ok(eq_array(
    mro::get_linear_isa('MRO_ᚠ'), \@MFO_ᚠ_DFS
));

ok(eq_array(mro::get_linear_isa('MRO_ᚠ', 'dfs'), \@MFO_ᚠ_DFS));
ok(eq_array(mro::get_linear_isa('MRO_ᚠ', 'c3'), \@MFO_ᚠ_C3));
eval{mro::get_linear_isa('MRO_ᚠ', 'C3')};
like($@, qr/^Invalid mro name: 'C3'/);

mro::set_mro('MRO_ᚠ', 'c3');
is(mro::get_mro('MRO_ᚠ'), 'c3');
ok(eq_array(
    mro::get_linear_isa('MRO_ᚠ'), \@MFO_ᚠ_C3
));

ok(eq_array(mro::get_linear_isa('MRO_ᚠ', 'dfs'), \@MFO_ᚠ_DFS));
ok(eq_array(mro::get_linear_isa('MRO_ᚠ', 'c3'), \@MFO_ᚠ_C3));
eval{mro::get_linear_isa('MRO_ᚠ', 'C3')};
like($@, qr/^Invalid mro name: 'C3'/);

ok(!mro::is_universal('MRO_ɓ'));

@UNIVERSAL::ISA = qw/MRO_ᚠ/;
ok(mro::is_universal('MRO_ɓ'));

@UNIVERSAL::ISA = ();
ok(!mro::is_universal('MRO_ᚠ'));
ok(!mro::is_universal('MRO_ɓ'));

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
    package MRO_ҭṣṱबꗻ;
    sub 텟tf운ꜿ { return 123 }
    package MRO_Teﬆ옽ḦРꤷsӭ;
    sub 텟ₜꖢᶯcƧ { return 321 }
    package MRO_Ɯ; our @ISA = qw/MRO_ҭṣṱबꗻ/;
}
*MRO_ᕡ::ISA = *MRO_Ɯ::ISA;
is(eval { MRO_ᕡ->텟tf운ꜿ() }, 123);

# XXX TODO (when there's a way to backtrack through a glob's aliases)
# push(@MRO_M::ISA, 'MRO_TestOtherBase');
# is(eval { MRO_N->testfunctwo() }, 321);

# Simple DESTROY Baseline
{
    my $x = 0;
    my $obj;

    {
        package DESTROY_MRO_Bӓeᓕne;
        sub new { bless {} => shift }
        sub DESTROY { $x++ }

        package DESTROY_MRO_Bӓeᓕne_χḻɖ;
        our @ISA = qw/DESTROY_MRO_Bӓeᓕne/;
    }

    $obj = DESTROY_MRO_Bӓeᓕne->new();
    undef $obj;
    is($x, 1);

    $obj = DESTROY_MRO_Bӓeᓕne_χḻɖ->new();
    undef $obj;
    is($x, 2);
}

# Dynamic DESTROY
{
    my $x = 0;
    my $obj;

    {
        package DESTROY_MRO_Ｄჷ및;
        sub new { bless {} => shift }

        package DESTROY_MRO_Ｄჷ및_χḻɖ;
        our @ISA = qw/DESTROY_MRO_Ｄჷ및/;
    }

    $obj = DESTROY_MRO_Ｄჷ및->new();
    undef $obj;
    is($x, 0);

    $obj = DESTROY_MRO_Ｄჷ및_χḻɖ->new();
    undef $obj;
    is($x, 0);

    no warnings 'once';
    *DESTROY_MRO_Ｄჷ및::DESTROY = sub { $x++ };

    $obj = DESTROY_MRO_Ｄჷ및->new();
    undef $obj;
    is($x, 1);

    $obj = DESTROY_MRO_Ｄჷ및_χḻɖ->new();
    undef $obj;
    is($x, 2);
}

# clearing @ISA in different ways
#  some are destructive to the package, hence the new
#  package name each time
{
    no warnings 'uninitialized';
    {
        package ᛁ앛ଌᛠ;
        our @ISA = qw/ｘｘ ƳƳ ƶƶ/;
    }
    # baseline
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ'),[qw/ᛁ앛ଌᛠ ｘｘ ƳƳ ƶƶ/]));

    # this looks dumb, but it preserves existing behavior for compatibility
    #  (undefined @ISA elements treated as "main")
    $ᛁ앛ଌᛠ::ISA[1] = undef;
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ'),[qw/ᛁ앛ଌᛠ ｘｘ main ƶƶ/]));

    # undef the array itself
    undef @ᛁ앛ଌᛠ::ISA;
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ'),[qw/ᛁ앛ଌᛠ/]));

    # Now, clear more than one package's @ISA at once
    {
        package ᛁ앛ଌᛠ1;
        our @ISA = qw/ＷẆ ｘｘ/;

        package ᛁ앛ଌᛠ2;
        our @ISA = qw/ƳƳ ƶƶ/;
    }
    # baseline
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ1'),[qw/ᛁ앛ଌᛠ1 ＷẆ ｘｘ/]));
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ2'),[qw/ᛁ앛ଌᛠ2 ƳƳ ƶƶ/]));
    (@ᛁ앛ଌᛠ1::ISA, @ᛁ앛ଌᛠ2::ISA) = ();

    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ1'),[qw/ᛁ앛ଌᛠ1/]));
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ2'),[qw/ᛁ앛ଌᛠ2/]));

    # [perl #49564]  This is a pretty obscure way of clearing @ISA but
    # it tests a regression that affects XS code calling av_clear too.
    {
        package ᛁ앛ଌᛠ3;
        our @ISA = qw/ＷẆ ｘｘ/;
    }
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ3'),[qw/ᛁ앛ଌᛠ3 ＷẆ ｘｘ/]));
    {
        package ᛁ앛ଌᛠ3;
        reset 'I';
    }
    ok(eq_array(mro::get_linear_isa('ᛁ앛ଌᛠ3'),[qw/ᛁ앛ଌᛠ3/]));
}

# Check that recursion bails out "cleanly" in a variety of cases
# (as opposed to say, bombing the interpreter or something)
{
    my @recurse_codes = (
        '@MRO_ഋ1::ISA = "MRO_ഋ2"; @MRO_ഋ2::ISA = "MRO_ഋ1";',
        '@MRO_ഋ3::ISA = "MRO_ഋ4"; push(@MRO_ഋ4::ISA, "MRO_ഋ3");',
        '@MRO_ഋ5::ISA = "MRO_ഋ6"; @MRO_ഋ6::ISA = qw/ｘｘ MRO_ഋ5 ƳƳ/;',
        '@MRO_ഋ7::ISA = "MRO_ഋ8"; push(@MRO_ഋ8::ISA, qw/ｘｘ MRO_ഋ7 ƳƳ/)',
    );
    foreach my $code (@recurse_codes) {
        eval $code;
        ok($@ =~ /Recursive inheritance detected/);
    }
}

# Check that SUPER caches get invalidated correctly
{
    {
        package ｽṔઍR텟ʇ;
        sub new { bless {} => shift }
        sub ຟઓ { $_[1]+1 }

        package ｽṔઍR텟ʇ::MᶤƉ;
        our @ISA = 'ｽṔઍR텟ʇ';

        package ｽṔઍR텟ʇ::킫;
        our @ISA = 'ｽṔઍR텟ʇ::MᶤƉ';
        sub ຟઓ { my $s = shift; $s->SUPER::ຟઓ(@_) }

        package ｽṔઍR텟ʇ::렙ﷰए;
        sub ຟઓ { $_[1]+3 }
    }

    my $stk_obj = ｽṔઍR텟ʇ::킫->new();
    is($stk_obj->ຟઓ(1), 2);
    { no warnings 'redefine';
      *ｽṔઍR텟ʇ::ຟઓ = sub { $_[1]+2 };
    }
    is($stk_obj->ຟઓ(2), 4);
    @ｽṔઍR텟ʇ::MᶤƉ::ISA = 'ｽṔઍR텟ʇ::렙ﷰए';
    is($stk_obj->ຟઓ(3), 6);
}

{ 
  {
    # assigning @ISA via arrayref to globref RT 60220
    package ᛔ1;
    sub new { bless {}, shift }
    
    package ᛔ2;
  }
  *{ᛔ2::ISA} = [ 'ᛔ1' ];
  my $foo = ᛔ2->new;
  ok(!eval { $foo->ɓᛅƘ }, "no ɓᛅƘ method");
  no warnings 'once';  # otherwise it'll bark about ᛔ1::ɓᛅƘ used only once
  *{ᛔ1::ɓᛅƘ} = sub { "[ɓᛅƘ]" };
  is(scalar eval { $foo->ɓᛅƘ }, "[ɓᛅƘ]", "can ɓᛅƘ now");
  is $@, '';
}

{
  # assigning @ISA via arrayref then modifying it RT 72866
  {
    package ㄑ1;
    sub Ｆஓ {  }

    package ㄑ2;
    sub ƚ { }

    package ㄑ3;
  }
  push @ㄑ3::ISA, "ㄑ1";
  can_ok("ㄑ3", "Ｆஓ");
  *ㄑ3::ISA = [];
  push @ㄑ3::ISA, "ㄑ1";
  can_ok("ㄑ3", "Ｆஓ");
  *ㄑ3::ISA = [];
  push @ㄑ3::ISA, "ㄑ2";
  can_ok("ㄑ3", "ƚ");
  ok(!ㄑ3->can("Ｆஓ"), "can't call Ｆஓ method any longer");
}

{
    # test mro::method_changed_in
    my $count = mro::get_pkg_gen("MRO_அ");
    mro::method_changed_in("MRO_அ");
    my $count_new = mro::get_pkg_gen("MRO_அ");

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
    @main::ISA = 'პᛅeȵᛏ';
    my $output = '';
    *პᛅeȵᛏ::ど = sub { $output .= 'პᛅeȵᛏ' };
    *პᛅeȵᛏ2::ど = sub { $output .= 'პᛅeȵᛏ2' };
    main->ど;
    @main::ISA = 'პᛅeȵᛏ2';
    main->ど;
    is $output, 'პᛅeȵᛏპᛅeȵᛏ2', '@main::ISA is magical';
}

{
    # Undefining *ISA, then modifying @ISA
    # This broke Class::Trait. See [perl #79024].
    {package Class::Trait::Base}
    no strict 'refs';
    undef   *{"एxṰர::ʦፖㄡsȨ::ISA"};
    'एxṰர::ʦፖㄡsȨ'->isa('Class::Trait::Base'); # cache the mro
    unshift @{"एxṰர::ʦፖㄡsȨ::ISA"}, 'Class::Trait::Base';
    ok 'एxṰர::ʦፖㄡsȨ'->isa('Class::Trait::Base'),
     'a isa b after undef *a::ISA and @a::ISA modification';
}

{
    # Deleting $package::{ISA}
    # Broken in 5.10.0; fixed in 5.13.7
    @BḼᵑtｈ::ISA = 'Bલdḏ';
    delete $BḼᵑtｈ::{ISA};
    ok !BḼᵑtｈ->isa("Bલdḏ"), 'delete $package::{ISA}';
}

{
    # Undefining stashes
    @ᖫᕃㄒṭ::ISA = "ᖮw잍";
    @ᖮw잍::ISA = "ሲঌએ";
    undef %ᖮw잍::;
    ok !ᖫᕃㄒṭ->isa('ሲঌએ'), 'undef %package:: updates subclasses';
}
