#!./perl

#
# various typeglob tests
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_unicode_tables();
}

use utf8;
use open qw( :utf8 :std );
use warnings;

plan( tests => 206 );

# type coersion on assignment
$ᕘ = 'ᕘ';
$ᴮᛅ = *main::ᕘ;
$ᴮᛅ = $ᕘ;
is(ref(\$ᴮᛅ), 'SCALAR');
$ᕘ = *main::ᴮᛅ;

# type coersion (not) on misc ops

ok($ᕘ);
is(ref(\$ᕘ), 'GLOB');

unlike ($ᕘ, qr/abcd/);
is(ref(\$ᕘ), 'GLOB');

is($ᕘ, '*main::ᴮᛅ');
is(ref(\$ᕘ), 'GLOB');

{
 no warnings;
 ${\*$ᕘ} = undef;
 is(ref(\$ᕘ), 'GLOB', 'no type coersion when assigning to *{} retval');
 $::{ఫｹ} = *ᴮᛅ;
 is(
   \$::{ఫｹ}, \*{"ఫｹ"},
   'symbolic *{} returns symtab entry when FAKE'
 );
 ${\*{"ఫｹ"}} = undef;
 is(
   ref(\$::{ఫｹ}), 'GLOB',
  'no type coersion when assigning to retval of symbolic *{}'
 );
 $::{pɥአＱuઍ} = *ᴮᛅ;
 eval '
   is(
     \$::{pɥአＱuઍ}, \*pɥአＱuઍ,
     "compile-time *{} returns symtab entry when FAKE"
   );
   ${\*pɥአＱuઍ} = undef;
 ';
 is(
   ref(\$::{pɥአＱuઍ}), 'GLOB',
  'no type coersion when assigning to retval of compile-time *{}'
 );
}

# type coersion on substitutions that match
$a = *main::ᕘ;
$b = $a;
$a =~ s/^X//;
is(ref(\$a), 'GLOB');
$a =~ s/^\*//;
is($a, 'main::ᕘ');
is(ref(\$b), 'GLOB');

# typeglobs as lvalues
substr($ᕘ, 0, 1) = "XXX";
is(ref(\$ᕘ), 'SCALAR');
is($ᕘ, 'XXXmain::ᴮᛅ');

# returning glob values
sub ᕘ {
  local($ᴮᛅ) = *main::ᕘ;
  $ᕘ = *main::ᴮᛅ;
  return ($ᕘ, $ᴮᛅ);
}

($ፉṶ, $ባ) = ᕘ();
ok(defined $ፉṶ);
is(ref(\$ፉṶ), 'GLOB');


ok(defined $ባ);
is(ref(\$ባ), 'GLOB');

# nested package globs
# NOTE:  It's probably OK if these semantics change, because the
#        fact that %X::Y:: is stored in %X:: isn't documented.
#        (I hope.)

{ package ฝ오::ʉ; no warnings 'once'; $test=1; }
ok(exists $ฝ오::{'ʉ::'});
is($ฝ오::{'ʉ::'}, '*ฝ오::ʉ::');


# test undef operator clearing out entire glob
$ᕘ = 'stuff';
@ᕘ = qw(more stuff);
%ᕘ = qw(even more random stuff);
undef *ᕘ;
is ($ᕘ, undef);
is (scalar @ᕘ, 0);
is (scalar %ᕘ, 0);

{
    # test warnings from assignment of undef to glob
    my $msg = '';
    local $SIG{__WARN__} = sub { $msg = $_[0] };
    use warnings;
    *ᕘ = 'ᴮᛅ';
    is($msg, '');
    *ᕘ = undef;
    like($msg, qr/Undefined value assigned to typeglob/);

    my $O_grave = utf8::unicode_to_native(0xd2);
    my $E_grave = utf8::unicode_to_native(0xc8);
    my $pat = sprintf(
        # It took a lot of experimentation to get the backslashes right (khw)
        "Argument \"\\*main::(?:PW\\\\x\\{%x\\}MPF"
                            . "|SKR\\\\x\\{%x\\}\\\\x\\{%x\\}\\\\x\\{%x\\})\" "
                            . "isn't numeric in sprintf",
                              $O_grave, $E_grave, $E_grave, $E_grave);
    $pat = qr/$pat/;

    no warnings 'once';
    # test warnings for converting globs to other forms
    my $copy = *PWÒMPF;
    foreach ($copy, *SKRÈÈÈ) {
	$msg = '';
	my $victim = sprintf "%d", $_;
	like($msg, $pat, "Warning on conversion to IV");
	is($victim, 0);

	$msg = '';
	$victim = sprintf "%u", $_;
	like($msg, $pat, "Warning on conversion to UV");
	is($victim, 0);

	$msg = '';
	$victim = sprintf "%e", $_;
	like($msg, $pat, "Warning on conversion to NV");
	like($victim, qr/^0\.0+E\+?00/i, "Expect floating point zero");

	$msg = '';
	$victim = sprintf "%s", $_;
	is($msg, '', "No warning on stringification");
	is($victim, '' . $_);
    }
}

my $test = curr_test();
# test *glob{THING} syntax
$Ẋ = "ok $test\n";
++$test;
@Ẋ = ("ok $test\n");
++$test;
%Ẋ = ("ok $test" => "\n");
++$test;
sub Ẋ { "ok $test\n" }
print ${*Ẋ{SCALAR}}, @{*Ẋ{ARRAY}}, %{*Ẋ{HASH}}, &{*Ẋ{CODE}};
# This needs to go here, after the print, as sub Ẋ will return the current
# value of test
++$test;
format Ẋ =
XXX This text isn't used. Should it be?
.
curr_test($test);

is (ref *Ẋ{FORMAT}, "FORMAT");
*Ẋ = *STDOUT;
is (*{*Ẋ{GLOB}}, "*main::STDOUT");

{
    my $test = curr_test();

    print {*Ẋ{IO}} "ok $test\n";
    ++$test;

    my $warn;
    local $SIG{__WARN__} = sub {
	$warn .= $_[0];
    };
    my $val = *Ẋ{FILEHANDLE};

    # deprecation warning removed in v5.23 -- rjbs, 2015-12-31
    # https://github.com/Perl/perl5/issues/15105
    print {*Ẋ{IO}} (! defined $warn
		    ? "ok $test\n" : "not ok $test\n");
    curr_test(++$test);
}


{
    # test if defined() doesn't create any new symbols

    my $a = "Sʎｍ000";
    ok(!defined *{$a});

    ok(!defined ${$a});
    ok(!defined *{$a});

    ok(!defined &{$a});
    ok(!defined *{$a});

    my $state = "not";
    *{$a} = sub { $state = "ok" };
    ok(defined &{$a});
    ok(defined *{$a});
    &{$a};
    is ($state, 'ok');
}

# [ID 20010526.001 (#7038)] localized glob loses value when assigned to

$Ｊ=1; %Ｊ=(a=>1); @Ｊ=(1); local *Ｊ=*Ｊ; *Ｊ = sub{};

is($Ｊ, 1);
is($Ｊ{a}, 1);
is($Ｊ[0], 1);

{
    # does pp_readline() handle glob-ness correctly?
    my $g = *ᕘ;
    $g = <DATA>;
    is ($g, "Perl\n");
}

{
    my $w = '';
    local $SIG{__WARN__} = sub { $w = $_[0] };
    sub aʙȼ1 ();
    local *aʙȼ1 = sub { };
    is ($w, '');
    sub aʙȼ2 ();
    local *aʙȼ2;
    *aʙȼ2 = sub { };
    is ($w, '');
    sub aʙȼ3 ();
    *aʙȼ3 = sub { };
    like ($w, qr/Prototype mismatch/);
}

{
    # [17375] rcatline to formerly-defined undef was broken. Fixed in
    # do_readline by checking SvOK. AMS, 20020918
    my $x = "not ";
    $x  = undef;
    $x .= <DATA>;
    is ($x, "Rules\n");
}

{
    # test the assignment of a GLOB to an LVALUE
    my $e = '';
    local $SIG{__DIE__} = sub { $e = $_[0] };
    my %Ｖ;
    sub ƒ { $_[0] = 0; $_[0] = "a"; $_[0] = *DATA }
    ƒ($Ｖ{Ｖ});
    is ($Ｖ{Ｖ}, '*main::DATA');
    is (ref\$Ｖ{Ｖ}, 'GLOB', 'lvalue assignment preserves globs');
    my $x = readline $Ｖ{Ｖ};
    is ($x, "perl\n");
    is ($e, '', '__DIE__ handler never called');
}

{

    my $e = '';
    # GLOB assignment to tied element
    local $SIG{__DIE__} = sub { $e = $_[0] };
    sub Ʈ::TIEARRAY  { bless [] => "Ʈ" }
    sub Ʈ::STORE     { $_[0]->[ $_[1] ] = $_[2] }
    sub Ʈ::FETCH     { $_[0]->[ $_[1] ] }
    sub Ʈ::FETCHSIZE { @{$_[0]} }
    tie my @ary => "Ʈ";
    $ary[0] = *DATA;
    is ($ary[0], '*main::DATA');
    is (
      ref\tied(@ary)->[0], 'GLOB',
     'tied elem assignment preserves globs'
    );
    is ($e, '', '__DIE__ handler not called');
    my $x = readline $ary[0];
    is($x, "rocks\n");
    is ($e, '', '__DIE__ handler never called');
}

{
    SKIP: {
        skip_if_miniperl('no dynamic loading on miniperl, no Encode', 2);
        # Need some sort of die or warn to get the global destruction text if the
        # bug is still present
        my $prog = <<'EOPROG';
            use utf8;
            use open qw( :utf8 :std );
            package ᴹ;
            $| = 1;
            sub DESTROY {eval {die qq{Farewell $_[0]}}; print $@}
            package main;
    
            bless \$Ⱥ::ㄅ, q{ᴹ};
            *Ⱥ:: = \*ㄅ::;
EOPROG
    
        utf8::decode($prog);
        my $output = runperl(prog => $prog);
        
        require Encode;
        $output = Encode::decode("UTF-8", $output);
        like($output, qr/^Farewell ᴹ=SCALAR/, "DESTROY was called");
        unlike($output, qr/global destruction/,
            "unreferenced symbol tables should be cleaned up immediately");
    }
}

{
    # Possibly not the correct test file for these tests.
    # There are certain space optimisations implemented via promotion rules to
    # GVs
    
    foreach (qw (оઓnḲ ga_ㄕƚo잎)) {
        ok(!exists $::{$_}, "no symbols of any sort to start with for $_");
    }
    
    # A string in place of the typeglob is promoted to the function prototype
    $::{оઓnḲ} = "pìè";
    my $proto = eval 'prototype \&оઓnḲ';
    die if $@;
    is ($proto, "pìè", "String is promoted to prototype");
    
    
    # A reference to a value is used to generate a constant subroutine
    foreach my $value (3, "Perl rules", \42, qr/whatever/, [1,2,3], {1=>2},
                    \*STDIN, \&ok, \undef, *STDOUT) {
        delete $::{оઓnḲ};
        $::{оઓnḲ} = \$value;
        $proto = eval 'prototype \&оઓnḲ';
        die if $@;
        is ($proto, '', "Prototype for a constant subroutine is empty");
    
        my $got = eval 'оઓnḲ';
        die if $@;
        is (ref $got, ref $value, "Correct type of value (" . ref($value) . ")");
        is ($got, $value, "Value is correctly set");
    }
}

delete $::{оઓnḲ};
$::{оઓnḲ} = \"Value";

*{"ga_ㄕƚo잎"} = \&{"оઓnḲ"};

is (ref $::{ga_ㄕƚo잎}, 'SCALAR', "Export of proxy constant as is");
is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'ga_ㄕƚo잎', "Value", "Constant has correct value");
is (ref $::{ga_ㄕƚo잎}, 'SCALAR',
    "Inlining of constant doesn't change representation");

delete $::{ga_ㄕƚo잎};

eval 'sub ga_ㄕƚo잎 (); 1' or die $@;
is ($::{ga_ㄕƚo잎}, '', "Prototype is stored as an empty string");

# Check that a prototype expands.
*{"ga_ㄕƚo잎"} = \&{"оઓnḲ"};

is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'ga_ㄕƚo잎', "Value", "Constant has correct value");
is (ref \$::{ga_ㄕƚo잎}, 'GLOB', "Symbol table has full typeglob");


@::zᐓｔ = ('Zᐓｔ!');

# Check that assignment to an existing typeglob works
{
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  *{"zᐓｔ"} = \&{"оઓnḲ"};
  is($w, '', "Should be no warning");
}

is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'zᐓｔ', "Value", "Constant has correct value");
is (ref \$::{zᐓｔ}, 'GLOB', "Symbol table has full typeglob");
is (join ('!', @::zᐓｔ), 'Zᐓｔ!', "Existing array still in typeglob");

sub Ṩp맅싵Ş () {
    "Traditional";
}

# Check that assignment to an existing subroutine works
{
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  *{"Ṩp맅싵Ş"} = \&{"оઓnḲ"};
  like($w, qr/^Constant subroutine main::Ṩp맅싵Ş redefined/,
       "Redefining a constant sub should warn");
}

is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'Ṩp맅싵Ş', "Value", "Constant has correct value");
is (ref \$::{Ṩp맅싵Ş}, 'GLOB', "Symbol table has full typeglob");

# Check that assignment to an existing typeglob works
{
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  *{"plუᒃ"} = [];
  *{"plუᒃ"} = \&{"оઓnḲ"};
  is($w, '', "Should be no warning");
}

is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'plუᒃ', "Value", "Constant has correct value");
is (ref \$::{plუᒃ}, 'GLOB', "Symbol table has full typeglob");

my $gr = eval '\*plუᒃ' or die;

{
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  *{$gr} = \&{"оઓnḲ"};
  is($w, '', "Redefining a constant sub to another constant sub with the same underlying value should not warn (It's just re-exporting, and that was always legal)");
}

is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'plუᒃ', "Value", "Constant has correct value");
is (ref \$::{plუᒃ}, 'GLOB', "Symbol table has full typeglob");

# Non-void context should defeat the optimisation, and will cause the original
# to be promoted (what change 26482 intended)
my $result;
{
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  $result = *{"aẈʞƙʞƙʞƙ"} = \&{"оઓnḲ"};
  is($w, '', "Should be no warning");
}

is (ref \$result, 'GLOB',
    "Non void assignment should still return a typeglob");

is (ref \$::{оઓnḲ}, 'GLOB', "This export does affect original");
is (eval 'plუᒃ', "Value", "Constant has correct value");
is (ref \$::{plუᒃ}, 'GLOB', "Symbol table has full typeglob");

delete $::{оઓnḲ};
$::{оઓnḲ} = \"Value";

sub non_dangling {
  my $w = '';
  local $SIG{__WARN__} = sub { $w = $_[0] };
  *{"z앞"} = \&{"оઓnḲ"};
  is($w, '', "Should be no warning");
}

non_dangling();
is (ref $::{оઓnḲ}, 'SCALAR', "Export doesn't affect original");
is (eval 'z앞', "Value", "Constant has correct value");
is (ref $::{z앞}, 'SCALAR', "Exported target is also a PCS");

sub dangling {
  local $SIG{__WARN__} = sub { die $_[0] };
  *{"ビfᶠ"} = \&{"оઓnḲ"};
}

dangling();
is (ref \$::{оઓnḲ}, 'GLOB', "This export does affect original");
is (eval 'ビfᶠ', "Value", "Constant has correct value");
is (ref \$::{ビfᶠ}, 'GLOB', "Symbol table has full typeglob");

{
    use vars qw($gᓙʞ $sምḲ $ᕘf);
    # Check reference assignment isn't affected by the SV type (bug #38439)
    $gᓙʞ = 3;
    $sምḲ = 4;
    $ᕘf = "halt and cool down";

    my $rv = \*sምḲ;
    is($gᓙʞ, 3);
    *gᓙʞ = $rv;
    is($gᓙʞ, 4);

    my $pv = "";
    $pv = \*sምḲ;
    is($ᕘf, "halt and cool down");
    *ᕘf = $pv;
    is($ᕘf, 4);
}

{
no warnings 'once';
format =
.
    
    foreach my $value ({1=>2}, *STDOUT{IO}, *STDOUT{FORMAT}) {
        # *STDOUT{IO} returns a reference to a PVIO. As it's blessed, ref returns
        # IO::Handle, which isn't what we want.
        my $type = $value;
        $type =~ s/.*=//;
        $type =~ s/\(.*//;
        delete $::{оઓnḲ};
        $::{оઓnḲ} = $value;
        $proto = eval 'prototype \&оઓnḲ';
        like ($@, qr/^Cannot convert a reference to $type to typeglob/,
            "Cannot upgrade ref-to-$type to typeglob");
    }
}

{
    no warnings qw(once uninitialized);
    my $g = \*ȼલᑧɹ;
    my $r = eval {no strict; ${*{$g}{SCALAR}}};
    is ($@, '', "PERL_DONT_CREATE_GVSV shouldn't affect thingy syntax");

    $g = \*vȍwɯ;
    $r = eval {use strict; ${*{$g}{SCALAR}}};
    is ($@, '',
	"PERL_DONT_CREATE_GVSV shouldn't affect thingy syntax under strict");
}

{
    # Bug reported by broquaint on IRC
    *ᔅᓗsḨ::{HASH}->{ISA}=[];
    ᔅᓗsḨ->import;
    pass("gv_fetchmeth coped with the unexpected");

    # An audit found these:
    {
	package ᔅᓗsḨ;
	sub 맆 {
	    my $s = shift;
	    $s->SUPER::맆;
	}
    }
    {
        eval {ᔅᓗsḨ->맆;};
        like ($@, qr/^Can't locate object method "맆"/, "Even with SUPER");
    }
    is(ᔅᓗsḨ->isa('swoosh'), '');
}

{
    die if exists $::{본ㄎ};
    $::{본ㄎ} = \"포ヰｅ";
    *{"본ㄎ"} = \&{"본ㄎ"};
    eval 'is(본ㄎ(), "포ヰｅ",
             "Assignment works when glob created midway (bug 45607)"); 1'
	or die $@;
}


# [perl #72740] - indirect object syntax, heuristically imputed due to
# the non-existence of a function, should not cause a stash entry to be
# created for the non-existent function.
{
    {
            package RƬ72740a;
            my $f = bless({}, RƬ72740b);
            sub s1 { s2 $f; }
            our $s4;
            sub s3 { s4 $f; }
    }
    {
            package RƬ72740b;
            sub s2 { "RƬ72740b::s2" }
            sub s4 { "RƬ72740b::s4" }
    }
    ok(exists($RƬ72740a::{s1}), "RƬ72740a::s1 exists");
    ok(!exists($RƬ72740a::{s2}), "RƬ72740a::s2 does not exist");
    ok(exists($RƬ72740a::{s3}), "RƬ72740a::s3 exists");
    ok(exists($RƬ72740a::{s4}), "RƬ72740a::s4 exists");
    is(RƬ72740a::s1(), "RƬ72740b::s2", "RƬ72740::s1 parsed correctly");
    is(RƬ72740a::s3(), "RƬ72740b::s4", "RƬ72740::s3 parsed correctly");
}

# [perl #71686] Globs that are in symbol table can be un-globbed
$ŚyṀ = undef;
$::{Ḟ앜ɞ} = *ŚyṀ;
is (eval 'local *::Ḟ앜ɞ = \"chuck"; $Ḟ앜ɞ', 'chuck',
	"Localized glob didn't coerce into a RV");
is ($@, '', "Can localize FAKE glob that's present in stash");
{
    is (scalar $::{Ḟ앜ɞ}, "*main::ŚyṀ",
            "Localized FAKE glob's value was correctly restored");
}

# [perl #1804] *$x assignment when $x is a copy of another glob
# And [perl #77508] (same thing with list assignment)
 {
    no warnings 'once';
    my $x = *_ràndom::glob_that_is_not_used_elsewhere;
    *$x = sub{};
    is(
      "$x", '*_ràndom::glob_that_is_not_used_elsewhere',
      '[perl #1804] *$x assignment when $x is FAKE',
    );
    $x = *_ràndom::glob_that_is_not_used_elsewhere;
    (my $dummy, *$x) = (undef,[]);
    is(
      "$x", '*_ràndom::glob_that_is_not_used_elsewhere',
      '[perl #77508] *$x list assignment when $x is FAKE',
    ) or require Devel::Peek, Devel::Peek::Dump($x);
}

# [perl #76540]
# this caused panics or 'Attempt to free unreferenced scalar'
# (its a compile-time issue, so the die lets us skip the prints)
{
    my @warnings;
    local $SIG{__WARN__} = sub { push @warnings, @_ };

    eval <<'EOF';
BEGIN { $::{FÒÒ} = \'ᴮᛅ' }
die "made it";
print FÒÒ, "\n";
print FÒÒ, "\n";
EOF

    like($@, qr/made it/, "#76540 - no panic");
    ok(!@warnings, "#76540 - no 'Attempt to free unreferenced scalar'");
}

# [perl #77362] various bugs related to globs as PVLVs
{
 no warnings qw 'once void';
 my %h; # We pass a key of this hash to the subroutine to get a PVLV.
 sub { for(shift) {
  # Set up our glob-as-PVLV
  $_ = *hòn;
  is $_, "*main::hòn";

  # Bad symbol for array
  ok eval{ @$_; 1 }, 'PVLV glob slots can be autovivified' or diag $@;

    {
        # This should call TIEHANDLE, not TIESCALAR
        *thèxt::TIEHANDLE = sub{};
        ok eval{ tie *$_, 'thèxt'; 1 }, 'PVLV globs can be tied as handles'
            or diag $@;
    }
  # Assigning undef to the glob should not overwrite it...
  {
   my $w;
   local $SIG{__WARN__} = sub { $w = shift };
   *$_ = undef;
   is $_, "*main::hòn", 'PVLV: assigning undef to the glob does nothing';
   like $w, qr\Undefined value assigned to typeglob\,
    'PVLV: assigning undef to the glob warns';
  }

  # Neither should reference assignment.
  *$_ = [];
  is $_, "*main::hòn", "PVLV: arrayref assignment assigns to the AV slot";

  # Concatenation should still work.
  ok eval { $_ .= 'thlèw' }, 'PVLV concatenation does not die' or diag $@;
  is $_, '*main::hònthlèw', 'PVLV concatenation works';

  # And we should be able to overwrite it with a string, number, or refer-
  # ence, too, if we omit the *.
  $_ = *hòn; $_ = 'tzòr';
  is $_, 'tzòr', 'PVLV: assigning a string over a glob';
  $_ = *hòn; $_ = 23;
  is $_, 23, 'PVLV: assigning an integer over a glob';
  $_ = *hòn; $_ = 23.23;
  is $_, 23.23, 'PVLV: assigning a float over a glob';
  $_ = *hòn; $_ = \my $sthat;
  is $_, \$sthat, 'PVLV: assigning a reference over a glob';

  # This bug was found by code inspection. Could this ever happen in
  # real life? :-)
  # This duplicates a file handle, accessing it through a PVLV glob, the
  # glob having been removed from the symbol table, so a stringified form
  # of it does not work. This checks that sv_2io does not stringify a PVLV.
  $_ = *quìn;
  open *quìn, "test.pl"; # test.pl is as good a file as any
  delete $::{quìn};
  ok eval { open my $zow, "<&", $_ }, 'PVLV: sv_2io stringifieth not'
   or diag $@;

  # Similar tests to make sure sv_2cv etc. do not stringify.
  *$_ = sub { 1 };
  ok eval { &$_ }, "PVLV glob can be called as a sub" or diag $@;
  *flèlp = sub { 2 };
  $_ = 'flèlp';
  is eval { &$_ }, 2, 'PVLV holding a string can be called as a sub'
   or diag $@;

  # Coderef-to-glob assignment when the glob is no longer accessible
  # under its name: These tests are to make sure the OPpASSIGN_CV_TO_GV
  # optimisation takes PVLVs into account, which is why the RHSs have to be
  # named subs.
  use constant ghèèn => 'quàrè';
  $_ = *mìng;
  delete $::{mìng};
  *$_ = \&ghèèn;
  is eval { &$_ }, 'quàrè',
   'PVLV: constant assignment when the glob is detached from the symtab'
    or diag $@;
  $_ = *bèngth;
  delete $::{bèngth};
  *ghèck = sub { 'lon' };
  *$_ = \&ghèck;
  is eval { &$_ }, 'lon',
   'PVLV: coderef assignment when the glob is detached from the symtab'
    or diag $@;

SKIP: {
    skip_if_miniperl("no dynamic loading on miniperl, so can't load PerlIO::scalar", 1);
    # open should accept a PVLV as its first argument
    $_ = *hòn;
    ok eval { open $_,'<', \my $thlext }, 'PVLV can be the first arg to open'
	or diag $@;
  }

  # -t should not stringify
  $_ = *thlìt; delete $::{thlìt};
  *$_ = *STDOUT{IO};
  ok defined -t $_, 'PVLV: -t does not stringify';

  # neither should -T
  # but some systems donât support this on file handles
  my $pass;
  ok
    eval {
     open my $quìle, "<", 'test.pl';
     $_ = *$quìle;
     $pass = -T $_;
     1
    } ? $pass : $@ =~ /not implemented on filehandles/,
   "PVLV: -T does not stringify";
  # Unopened file handle
  {
   my $w;
   local $SIG{__WARN__} = sub { $w .= shift };
   $_ = *vòr;
   close $_;
   like $w, qr\unopened filehandle vòr\,
    'PVLV globs get their names reported in unopened error messages';
  }

 }}->($h{k});
}

*àieee = 4;
pass('Can assign integers to typeglobs');
*àieee = 3.14;
pass('Can assign floats to typeglobs');
*àieee = 'pi';
pass('Can assign strings to typeglobs');


{
  package thrèxt;
  sub TIESCALAR{bless[]}
  sub STORE{ die "No!"}
  sub FETCH{ no warnings 'once'; *thrìt }
  tie my $a, "thrèxt";
  () = "$a"; # do a fetch; now $a holds a glob
  eval { *$a = sub{} };
  untie $a;
  eval { $a = "ᴮᛅ" };
  ::is $a, "ᴮᛅ",
    "[perl #77812] Globs in tied scalars can be reified if STORE dies"
}

# These two crashed prior to 5.13.6. In 5.13.6 they were fatal errors. They
# were fixed in 5.13.7.
ok eval {
  my $glob = \*hèèn::ISA;
  delete $::{"hèèn::"};
  *$glob = *ᴮᛅ; 
}, "glob-to-*ISA assignment works when *ISA has lost its stash";
ok eval {
  my $glob = \*slàre::ISA;
  delete $::{"slàre::"};
  *$glob = []; 
}, "array-to-*ISA assignment works when *ISA has lost its stash";
# These two crashed in 5.13.6. They were likewise fixed in 5.13.7.
ok eval {
  sub grèck;
  my $glob = do { no warnings "once"; \*phìng::ᕘ};
  delete $::{"phìng::"};
  *$glob = *grèck; 
}, "Assigning a glob-with-sub to a glob that has lost its stash warks";
ok eval {
  sub pòn::ᕘ;
  my $glob = \*pòn::ᕘ;
  delete $::{"pòn::"};
  *$glob = *ᕘ; 
}, "Assigning a glob to a glob-with-sub that has lost its stash warks";

{
  package Tie::Alias;
  sub TIESCALAR{ bless \\pop }
  sub FETCH { $${$_[0]} }
  sub STORE { $${$_[0]} = $_[1] }
  package main;
  tie my $alias, 'Tie::Alias', my $var;
  no warnings 'once';
  $var = *gàlobbe;
  {
    local *$alias = [];
    $var = 3;
    is $alias, 3, "[perl #77926] Glob reification during localisation";
  }
}

# This code causes gp_free to call a destructor when a glob is being
# restored on scope exit. The destructor used to see SVs with a refcount of
# zero inside the glob, which could result in crashes (though not in this
# test case, which just panics).
{
 no warnings 'once';
 my $survived;
 *Trìt::DESTROY = sub {
   $thwèxt = 42;  # panic
   $survived = 1;
 };
 {
  local *thwèxt = bless [],'Trìt';
  ();
 }
 ok $survived,
  'no error when gp_free calls a destructor that assigns to the gv';
}

__END__
Perl
Rules
perl
rocks
