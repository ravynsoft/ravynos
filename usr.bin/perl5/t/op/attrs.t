#!./perl

# Regression tests for attributes.pm and the C< : attrs> syntax.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load attributes");
}

use warnings;

$SIG{__WARN__} = sub { die @_ };

sub eval_ok ($;$) {
    eval shift;
    is( $@, '', @_);
}

fresh_perl_is 'use attributes; print "ok"', 'ok', {},
   'attributes.pm can load without warnings.pm already loaded';

our $anon1; eval_ok '$anon1 = sub : method { $_[0]++ }';

eval 'sub e1 ($) : plugh ;';
like $@, qr/^Invalid CODE attributes?: ["']?plugh["']? at/;

eval 'sub e2 ($) : plugh(0,0) xyzzy ;';
like $@, qr/^Invalid CODE attributes: ["']?plugh\(0,0\)["']? /;

eval 'sub e3 ($) : plugh(0,0 xyzzy ;';
like $@, qr/^Unterminated attribute parameter in attribute list at \(eval \d+\) line 1\.$/;

eval 'sub e4 ($) : plugh + XYZZY ;';
like $@, qr/Invalid separator character '[+]' in attribute list at/;

eval_ok 'my main $x : = 0;';
eval_ok 'my $x : = 0;';
eval_ok 'my $x ;';
eval_ok 'my ($x) : = 0;';
eval_ok 'my ($x) ;';
eval_ok 'my ($x) : ;';
eval_ok 'my ($x,$y) : = 0;';
eval_ok 'my ($x,$y) ;';
eval_ok 'my ($x,$y) : ;';

eval 'my ($x,$y) : plugh;';
like $@, qr/^Invalid SCALAR attribute: ["']?plugh["']? at/;

# bug #16080
eval '{my $x : plugh}';
like $@, qr/^Invalid SCALAR attribute: ["']?plugh["']? at/;
eval '{my ($x,$y) : plugh(})}';
like $@, qr/^Invalid SCALAR attribute: ["']?plugh\(\}\)["']? at/;

# More syntax tests from the attributes manpage
eval 'my $x : switch(10,foo(7,3))  :  expensive;';
like $@, qr/^Invalid SCALAR attributes: ["']?switch\(10,foo\(7,3\)\) : expensive["']? at/;
eval q/my $x : Ugly('\(") :Bad;/;
like $@, qr/^Invalid SCALAR attributes: ["']?Ugly\('\\\("\) : Bad["']? at/;
eval 'my $x : _5x5;';
like $@, qr/^Invalid SCALAR attribute: ["']?_5x5["']? at/;
eval 'my $x : locked method;';
like $@, qr/^Invalid SCALAR attributes: ["']?locked : method["']? at/;
eval 'my $x : switch(10,foo();';
like $@, qr/^Unterminated attribute parameter in attribute list at \(eval \d+\) line 1\.$/;
eval q/my $x : Ugly('(');/;
like $@, qr/^Unterminated attribute parameter in attribute list at \(eval \d+\) line 1\.$/;
eval 'my $x : 5x5;';
like $@, qr/error/;
eval 'my $x : Y2::north;';
like $@, qr/Invalid separator character ':' in attribute list at/;

sub A::MODIFY_SCALAR_ATTRIBUTES { return }
eval 'my A $x : plugh;';
like $@, qr/^SCALAR package attribute may clash with future reserved word: ["']?plugh["']? at/;

eval 'my A $x : plugh plover;';
like $@, qr/^SCALAR package attributes may clash with future reserved words: ["']?plugh["']? /;

no warnings 'reserved';
eval 'my A $x : plugh;';
is $@, '';

eval 'package Cat; my Cat @socks;';
is $@, '';

eval 'my Cat %nap;';
is $@, '';

sub X::MODIFY_CODE_ATTRIBUTES { die "$_[0]" }
sub X::foo { 1 }
*Y::bar = \&X::foo;
*Y::bar = \&X::foo;	# second time for -w
eval 'package Z; sub Y::bar : foo';
like $@, qr/^X at /;

@attrs = eval 'attributes::get $anon1';
is "@attrs", "method";

sub Z::DESTROY { }
sub Z::FETCH_CODE_ATTRIBUTES { return 'Z' }
my $thunk = eval 'bless +sub : method { 1 }, "Z"';
is ref($thunk), "Z";

@attrs = eval 'attributes::get $thunk';
is "@attrs", "method Z";

# Test attributes on predeclared subroutines:
eval 'package A; sub PS : lvalue';
@attrs = eval 'attributes::get \&A::PS';
is "@attrs", "lvalue";

# Multiple attributes at once
eval 'package A; sub PS2 : lvalue method';
@attrs = eval 'attributes::get \&A::PS2';
is "@attrs", "lvalue method", 'Multiple builtin attributes can be set at once';

# Test attributes on predeclared subroutines, after definition
eval 'package A; sub PS : lvalue; sub PS { }';
@attrs = eval 'attributes::get \&A::PS';
is "@attrs", "lvalue";

# Test ability to modify existing sub's (or XSUB's) attributes.
eval 'package A; sub X { $_[0] } sub X : method';
@attrs = eval 'attributes::get \&A::X';
is "@attrs", "method";

# Above not with just 'pure' built-in attributes.
sub Z::MODIFY_CODE_ATTRIBUTES { (); }
eval 'package Z; sub L { $_[0] } sub L : Z method';
@attrs = eval 'attributes::get \&Z::L';
is "@attrs", "method Z";

# Begin testing attributes that tie

{
    package Ttie;
    sub DESTROY {}
    sub TIESCALAR { my $x = $_[1]; bless \$x, $_[0]; }
    sub FETCH { ${$_[0]} }
    sub STORE {
	::pass;
	${$_[0]} = $_[1]*2;
    }
    package Tloop;
    sub MODIFY_SCALAR_ATTRIBUTES { tie ${$_[1]}, 'Ttie', -1; (); }
}

eval_ok '
    package Tloop;
    for my $i (0..2) {
	my $x : TieLoop = $i;
	$x != $i*2 and ::is $x, $i*2;
    }
';

# bug #15898
eval 'our ${""} : foo = 1';
like $@, qr/Can't declare scalar dereference in "our"/;
eval 'my $$foo : bar = 1';
like $@, qr/Can't declare scalar dereference in "my"/;


my @code = qw(lvalue method);
my @other = qw(shared);
my @deprecated = qw();
my @invalid = qw(unique locked);
my %valid;
$valid{CODE} = {map {$_ => 1} @code};
$valid{SCALAR} = {map {$_ => 1} @other};
$valid{ARRAY} = $valid{HASH} = $valid{SCALAR};
my %deprecated;

our ($scalar, @array, %hash);
foreach my $value (\&foo, \$scalar, \@array, \%hash) {
    my $type = ref $value;
    foreach my $negate ('', '-') {
	foreach my $attr (@code, @other, @deprecated, @invalid) {
	    my $attribute = $negate . $attr;
	    eval "use attributes __PACKAGE__, \$value, '$attribute'";
	    if ($deprecated{$type}{$attr}) {
		like $@, qr/^Attribute "$attr" is deprecated, (?#:
                            )and will disappear in Perl 5.28 at \(eval \d+\)/,
		    "$type attribute $attribute deprecated";
	    } elsif ($valid{$type}{$attr}) {
		if ($attribute eq '-shared') {
		    like $@, qr/^A variable may not be unshared/;
		} else {
		    is( $@, '', "$type attribute $attribute");
		}
	    } else {
		like $@, qr/^Invalid $type attribute: $attribute/,
		    "Bogus $type attribute $attribute should fail";
	    }
	}
    }
}

# this will segfault if it fails
sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

ok !defined(eval 'attributes::get(\PVBM)'), 
    'PVBMs don\'t segfault attributes::get';

{
    #  [perl #49472] Attributes + Unknown Error
    eval '
	use strict;
	sub MODIFY_CODE_ATTRIBUTE{}
	sub f:Blah {$nosuchvar};
    ';

    my $err = $@;
    like ($err, qr/Global symbol "\$nosuchvar" requires /, 'perl #49472');
}

# Test that code attributes always get applied to the same CV that
# we're left with at the end (bug#66970).
{
	package bug66970;
	our $c;
	sub MODIFY_CODE_ATTRIBUTES { $c = $_[1]; () }
	$c=undef; eval 'sub t0 :Foo';
	main::ok $c == \&{"t0"};
	$c=undef; eval 'sub t1 :Foo { }';
	main::ok $c == \&{"t1"};
	$c=undef; eval 'sub t2';
	our $t2a = \&{"t2"};
	$c=undef; eval 'sub t2 :Foo';
	main::ok $c == \&{"t2"} && $c == $t2a;
	$c=undef; eval 'sub t3';
	our $t3a = \&{"t3"};
	$c=undef; eval 'sub t3 :Foo { }';
	main::ok $c == \&{"t3"} && $c == $t3a;
	$c=undef; eval 'sub t4 :Foo';
	our $t4a = \&{"t4"};
	our $t4b = $c;
	$c=undef; eval 'sub t4 :Foo';
	main::ok $c == \&{"t4"} && $c == $t4b && $c == $t4a;
	$c=undef; eval 'sub t5 :Foo';
	our $t5a = \&{"t5"};
	our $t5b = $c;
	$c=undef; eval 'sub t5 :Foo { }';
	main::ok $c == \&{"t5"} && $c == $t5b && $c == $t5a;
}

my @tests = grep {/^[^#]/} split /\n/, <<'EOT';
# This one is fine as an empty attribute list
my $holy_Einstein : = '';
# This one is deprecated
my $krunch := 4;
our $FWISK_FWISK_FWIZZACH_FWACH_ZACHITTY_ZICH_SHAZZATZ_FWISK := '';
state $thump := 'Trumpets';
# Lather rinse repeat in my usual obsessive style
my @holy_perfect_pitch : = ();
my @zok := ();
our @GUKGUK := ();
# state @widget_mark := ();
my %holy_seditives : = ();
my %bang := ();
our %GIGAZING := ();
# state %hex := ();
my $holy_giveaways : = '';
my $eee_yow := [];
our $TWOYYOYYOING_THUK_UGH := 1 == 1;
state $octothorn := 'Tinky Winky';
my @holy_Taj_Mahal : = ();
my @touche := ();
our @PLAK_DAK_THUK_FRIT := ();
# state @hash_mark := ();
my %holy_priceless_collection_of_Etruscan_snoods : = ();
my %wham_eth := ();
our %THWUK := ();
# state %octalthorpe := ();
my $holy_sewer_pipe : = '';
my $thunk := undef;
our $BLIT := time;
state $crunch := 'Laa Laa';
my @glurpp := ();
my @holy_harem : = ();
our @FABADAP := ();
# state @square := ();
my %holy_pin_cushions : = ();
my %swoosh := ();
our %RRRRR := ();
# state %scratchmark := ();
EOT

foreach my $test (@tests) {
    use feature 'state';
    eval $test;
    if ($test =~ /:=/) {
	like $@, qr/Use of := for an empty attribute list is not allowed/,
	    "Parse error for q{$test}";
    } else {
	is $@, '', "No error for q{$test}";
    }
}

# [perl #68560] Calling closure prototypes (only accessible via :attr)
{
  package brength;
  my $proto;
  sub MODIFY_CODE_ATTRIBUTES { $proto = $_[1]; _: }
  eval q{
     my $x;
     () = sub :a0 { $x };
  };
  package main;
  eval { $proto->() };               # used to crash in pp_entersub
  like $@, qr/^Closure prototype called/,
     "Calling closure proto with (no) args";
  eval { () = &$proto };             # used to crash in pp_leavesub
  like $@, qr/^Closure prototype called/,
     'Calling closure proto with no @_ that returns a lexical';
}

# Referencing closure prototypes
{
  package buckbuck;
  my @proto;
  sub MODIFY_CODE_ATTRIBUTES { push @proto, $_[1], \&{$_[1]}; _: }
  my $id;
  () = sub :buck {$id};
  &::is(@proto, 'referencing closure prototype');
}

# [perl #68658] Attributes on stately variables
{
  package thwext;
  sub MODIFY_SCALAR_ATTRIBUTES { () }
  my $i = 0;
  my $x_values = '';
  eval 'sub foo { use 5.01; state $x :A0 = $i++; $x_values .= $x }';
  foo(); foo();
  package main;
  is $x_values, '00', 'state with attributes';
}

{
  package ningnangnong;
  sub MODIFY_SCALAR_ATTRIBUTES{}
  sub MODIFY_ARRAY_ATTRIBUTES{  }
  sub MODIFY_HASH_ATTRIBUTES{    }
  my ($cows, @go, %bong) : teapots = qw[ jibber jabber joo ];
  ::is $cows, 'jibber', 'list assignment to scalar with attrs';
  ::is "@go", 'jabber joo', 'list assignment to array with attrs';
}

{
  my $w;
  local $SIG{__WARN__} = sub { $w = shift };
  sub  ent         {}
  sub lent :lvalue {}
  my $posmsg =
      'lvalue attribute applied to already-defined subroutine at '
     .'\(eval';
  my $negmsg =
      'lvalue attribute removed from already-defined subroutine at '
     .'\(eval';
  eval 'use attributes __PACKAGE__, \&ent, "lvalue"';
  like $w, qr/^$posmsg/, 'lvalue attr warning on def sub';
  is join("",&attributes::get(\&ent)), "lvalue",':lvalue applied anyway';
  $w = '';
  eval 'use attributes __PACKAGE__, \&lent, "lvalue"; 1' or die;
  is $w, "", 'no lvalue warning on def lvalue sub';
  eval 'use attributes __PACKAGE__, \&lent, "-lvalue"';
  like $w, qr/^$negmsg/, '-lvalue attr warning on def sub';
  is join("",&attributes::get(\&lent)), "",
       'lvalue attribute removed anyway';
  $w = '';
  eval 'use attributes __PACKAGE__, \&lent, "-lvalue"; 1' or die;
  is $w, "", 'no -lvalue warning on def non-lvalue sub';
  no warnings 'misc';
  eval 'use attributes __PACKAGE__, \&lent, "lvalue"';
  is $w, "", 'no lvalue warnings under no warnings misc';
  eval 'use attributes __PACKAGE__, \&ent, "-lvalue"';
  is $w, "", 'no -lvalue warnings under no warnings misc';
}

unlike runperl(
         prog => 'BEGIN {$^H{a}=b} sub foo:bar{1}',
         stderr => 1,
       ),
       qr/Unbalanced/,
      'attribute errors do not cause op trees to leak';

package ProtoTest {
    sub MODIFY_CODE_ATTRIBUTES { $Proto = prototype $_[1]; () }
    sub foo ($) : gelastic {}
}
is $ProtoTest::Proto, '$', 'prototypes are visible in attr handlers';

{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    attributes ->import(__PACKAGE__, \&foo, "const");
    like $w, qr/^Useless use of attribute "const" at /,
            'Warning for useless const via attributes.pm';
    $w = '';
    attributes ->import(__PACKAGE__, \&foo, "const");
    is $w, '', 'no warning for const if already applied';
    attributes ->import(__PACKAGE__, \&foo, "-const");
    is $w, '', 'no warning for -const with attr already applied';
    attributes ->import(__PACKAGE__, \&bar, "-const");
    is $w, '', 'no warning for -const with attr not already applied';
    package ConstTest;
    sub MODIFY_CODE_ATTRIBUTES {
        attributes->import(shift, shift, lc shift) if $_[2]; ()
    }
    $_ = 32487;
    my $sub = eval '+sub : Const { $_ }';
    ::is $w, '',
     'no warning for :const applied to closure protosub via attributes.pm';
    undef $_;
    ::is &$sub, 32487,
        'applying const attr via attributes.pm';
}

# [perl #123817] Attributes in list-type operators
# These tests used to fail an assertion because the list op generated by
# the lexical attribute declaration was converted to another op type with
# the OPpLVAL_INTRO flag still set.  These op types were not expecting that
# flag to be set, though it was harmless for non-debugging builds.
package _123817 {
    sub MODIFY_SCALAR_ATTRIBUTES {()}
    eval '{my $x : m}';
    eval '[(my $x : m)]';
    eval 'formline my $x : m';
    eval 'return my $x : m';
}

# [perl #126257]
# attributed lex var as function arg caused assertion failure

package P126257 {
    sub MODIFY_SCALAR_ATTRIBUTES {}
    sub MODIFY_ARRAY_ATTRIBUTES  {}
    sub MODIFY_HASH_ATTRIBUTES   {}
    sub MODIFY_CODE_ATTRIBUTES   {}
    sub foo {}
    eval { foo(my $x : bar); };
    ::is $@, "", "RT 126257 scalar";
    eval { foo(my @x : bar); };
    ::is $@, "", "RT 126257 array";
    eval { foo(my %x : bar); };
    ::is $@, "", "RT 126257 hash";
    eval { foo(sub : bar {}); };
    ::is $@, "", "RT 126257 sub";
}

# RT #129099
# Setting an attribute on a BEGIN prototype causes
#       BEGIN { require "attributes"; ... }
# to be compiled, which caused problems with ops being prematurely
# freed when CvSTART was transferred from the old BEGIN to the new BEGIN

is runperl(
       prog => 'package Foo; sub MODIFY_CODE_ATTRIBUTES {()} '
             . 'sub BEGIN :Foo; print qq{OK\n}',
       stderr => 1,
   ),
   "OK\n",
  'RT #129099 BEGIN';
is runperl(
       prog => 'package Foo; sub MODIFY_CODE_ATTRIBUTES {()} '
             . 'no warnings q{prototype}; sub BEGIN() :Foo; print qq{OK\n}',
       stderr => 1,
   ),
   "OK\n",
  'RT #129099 BEGIN()';


#129086
# When printing error message for an attribute arg without closing ')',
# if the buffer got reallocated during the scan of the arg, the error
# message would try to use the old buffer
fresh_perl_like(
   'my $abc: abcdefg(' . 'x' x 195 . "\n" . 'x' x 8200 ."\n",
    qr/^Unterminated attribute parameter in attribute list at - line 1\.$/,
    { stderr => 1 },
    'RT #129086 attr(00000'
);

TODO: {
    local $TODO = 'RT #3605: Attribute syntax causes parsing errors near my $var :';
    my $out = runperl(prog => <<'EOP', stderr => 1);
    $ref = \($1 ? my $var : my $othervar);
EOP
    unlike($out, qr/Invalid separator character/, 'RT #3605: Errors near attribute colon need a better error message');
    is($out, '', 'RT #3605: $a ? my $var : my $othervar is perfectly valid syntax');
}

fresh_perl_is('sub dummy {} our $dummy : Dummy', <<EOS, {},
Invalid SCALAR attribute: Dummy at - line 1.
BEGIN failed--compilation aborted at - line 1.
EOS
              "attribute on our scalar with sub of same name");

fresh_perl_is('sub dummy {} our @dummy : Dummy', <<EOS, {},
Invalid ARRAY attribute: Dummy at - line 1.
BEGIN failed--compilation aborted at - line 1.
EOS
              "attribute on our array with sub of same name");

fresh_perl_is('sub dummy {} our %dummy : Dummy', <<EOS, {},
Invalid HASH attribute: Dummy at - line 1.
BEGIN failed--compilation aborted at - line 1.
EOS
              "attribute on our hash with sub of same name");

fresh_perl_is('$_ = ""; s/^/ { my $x : shared = 1; } /e;', "", {},
              "attributes in sub-parse");

done_testing();
