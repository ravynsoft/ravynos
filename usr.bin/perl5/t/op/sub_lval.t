#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
plan tests=>211;

sub a : lvalue { my $a = 34; ${\(bless \$a)} }  # Return a temporary
sub b : lvalue { ${\shift} }

my $out = a(b());		# Check that temporaries are allowed.
is(ref $out, 'main'); # Not reached if error.

my @out = grep /main/, a(b()); # Check that temporaries are allowed.
cmp_ok(scalar @out, '==', 1); # Not reached if error.

my $in;

# Check that we can return localized values from subroutines:

sub in : lvalue { $in = shift; }
sub neg : lvalue {  #(num_str) return num_str
    local $_ = shift;
    s/^\+/-/;
    $_;
}
in(neg("+2"));


is($in, '-2');

sub get_lex : lvalue { $in }
sub get_st : lvalue { $blah }
sub id : lvalue { ${\shift} }
sub id1 : lvalue { $_[0] }
sub inc : lvalue { ${\++$_[0]} }

$in = 5;
$blah = 3;

get_st = 7;

cmp_ok($blah, '==', 7);

get_lex = 7;

cmp_ok($in, '==', 7);

++get_st;

cmp_ok($blah, '==', 8);

++get_lex;

cmp_ok($in, '==', 8);

id(get_st) = 10;

cmp_ok($blah, '==', 10);

id(get_lex) = 10;

cmp_ok($in, '==', 10);

++id(get_st);

cmp_ok($blah, '==', 11);

++id(get_lex);

cmp_ok($in, '==', 11);

id1(get_st) = 20;

cmp_ok($blah, '==', 20);

id1(get_lex) = 20;

cmp_ok($in, '==', 20);

++id1(get_st);

cmp_ok($blah, '==', 21);

++id1(get_lex);

cmp_ok($in, '==', 21);

inc(get_st);

cmp_ok($blah, '==', 22);

inc(get_lex);

cmp_ok($in, '==', 22);

inc(id(get_st));

cmp_ok($blah, '==', 23);

inc(id(get_lex));

cmp_ok($in, '==', 23);

++inc(id1(id(get_st)));

cmp_ok($blah, '==', 25);

++inc(id1(id(get_lex)));

cmp_ok($in, '==', 25);

@a = (1) x 3;
@b = (undef) x 2;
$#c = 3;			# These slots are not fillable.

# Explanation: empty slots contain &sv_undef.

=for disabled constructs

sub a3 :lvalue {@a}
sub b2 : lvalue {@b}
sub c4: lvalue {@c}

$_ = '';

eval <<'EOE' or $_ = $@;
  ($x, a3, $y, b2, $z, c4, $t) = (34 .. 78);
  1;
EOE

#@out = ($x, a3, $y, b2, $z, c4, $t);
#@in = (34 .. 41, (undef) x 4, 46);
#print "# '@out' ne '@in'\nnot " unless "@out" eq "@in";

like($_, qr/Can\'t return an uninitialized value from lvalue subroutine/);
print "ok 22\n";

=cut


my $var;

sub a::var : lvalue { $var }

"a"->var = 45;

cmp_ok($var, '==', 45);

my $oo;
$o = bless \$oo, "a";

$o->var = 47;

cmp_ok($var, '==', 47);

sub o : lvalue { $o }

o->var = 49;

cmp_ok($var, '==', 49);

sub nolv () { $x0, $x1 } # Not lvalue

$_ = '';

eval <<'EOE' or $_ = $@;
  nolv = (2,3);
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call of &main::nolv in scalar assignment/);

$_ = '';

eval <<'EOE' or $_ = $@;
  nolv = (2,3) if $_;
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call of &main::nolv in scalar assignment/);

$_ = '';

eval <<'EOE' or $_ = $@;
  &nolv = (2,3) if $_;
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call of &main::nolv in scalar assignment/);

$x0 = $x1 = $_ = undef;
$nolv = \&nolv;

eval <<'EOE' or $_ = $@;
  $nolv->() = (2,3) if $_;
  1;
EOE

ok(!defined $_) or diag "'$_', '$x0', '$x1'";

$x0 = $x1 = $_ = undef;
$nolv = \&nolv;

eval <<'EOE' or $_ = $@;
  $nolv->() = (2,3);
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call/)
  or diag "'$_', '$x0', '$x1'";

sub lv0 : lvalue { }
sub rlv0 : lvalue { return }

$_ = undef;
eval <<'EOE' or $_ = $@;
  lv0 = (2,3);
  1;
EOE

like($_, qr/Can't return undef from lvalue subroutine/);

$_ = undef;
eval <<'EOE' or $_ = $@;
  rlv0 = (2,3);
  1;
EOE

like($_, qr/Can't return undef from lvalue subroutine/,
    'explicit return of nothing in scalar context');

$_ = undef;
eval <<'EOE' or $_ = $@;
  (lv0) = (2,3);
  1;
EOE

ok(!defined $_) or diag $_;

$_ = undef;
eval <<'EOE' or $_ = $@;
  (rlv0) = (2,3);
  1;
EOE

ok(!defined $_, 'explicit return of nothing in list context') or diag $_;

($a,$b)=();
(lv0($a,$b)) = (3,4);
is +($a//'undef') . ($b//'undef'), 'undefundef',
   'list assignment to empty lvalue sub';


sub lv1u :lvalue { undef }
sub rlv1u :lvalue { undef }

$_ = undef;
eval <<'EOE' or $_ = $@;
  lv1u = (2,3);
  1;
EOE

like($_, qr/Can't return undef from lvalue subroutine/);

$_ = undef;
eval <<'EOE' or $_ = $@;
  rlv1u = (2,3);
  1;
EOE

like($_, qr/Can't return undef from lvalue subroutine/,
     'explicitly returning undef in scalar context');

$_ = undef;
eval <<'EOE' or $_ = $@;
  (lv1u) = (2,3);
  1;
EOE

ok(!defined, 'implicitly returning undef in list context');

$_ = undef;
eval <<'EOE' or $_ = $@;
  (rlv1u) = (2,3);
  1;
EOE

ok(!defined, 'explicitly returning undef in list context');

$x = '1234567';

$_ = undef;
eval <<'EOE' or $_ = $@;
  sub lv1t : lvalue { index $x, 2 }
  lv1t = (2,3);
  1;
EOE

like($_, qr/Can\'t return a temporary from lvalue subroutine/);

$_ = undef;
eval <<'EOE' or $_ = $@;
  sub rlv1t : lvalue { index $x, 2 }
  rlv1t = (2,3);
  1;
EOE

like($_, qr/Can\'t return a temporary from lvalue subroutine/,
    'returning a PADTMP explicitly');

$_ = undef;
eval <<'EOE' or $_ = $@;
  (rlv1t) = (2,3);
  1;
EOE

like($_, qr/Can\'t return a temporary from lvalue subroutine/,
    'returning a PADTMP explicitly (list context)');

# These next two tests are not necessarily normative.  But this way we will
# know if this discrepancy changes.

$_ = undef;
eval <<'EOE' or $_ = $@;
  sub scalarray : lvalue { @a || $b }
  @a = 1;
  (scalarray) = (2,3);
  1;
EOE

like($_, qr/Can\'t return a temporary from lvalue subroutine/,
    'returning a scalar-context array via ||');

$_ = undef;
eval <<'EOE' or $_ = $@;
  use warnings "FATAL" => "all";
  sub myscalarray : lvalue { my @a = 1; @a || $b }
  (myscalarray) = (2,3);
  1;
EOE

like($_, qr/Useless assignment to a temporary/,
    'returning a scalar-context lexical array via ||');

$_ = undef;
sub lv2t : lvalue { shift }
(lv2t($_)) = (2,3);
is($_, 2);

$xxx = 'xxx';
sub xxx () { $xxx }  # Not lvalue

$_ = undef;
eval <<'EOE' or $_ = $@;
  sub lv1tmp : lvalue { xxx }			# is it a TEMP?
  lv1tmp = (2,3);
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call of &main::xxx at /);

$_ = undef;
eval <<'EOE' or $_ = $@;
  (lv1tmp) = (2,3);
  1;
EOE

like($_, qr/Can\'t modify non-lvalue subroutine call of &main::xxx at /);

sub yyy () { 'yyy' } # Const, not lvalue

$_ = undef;
eval <<'EOE' or $_ = $@;
  sub lv1tmpr : lvalue { yyy }			# is it read-only?
  lv1tmpr = (2,3);
  1;
EOE

like($_, qr/Can\'t return a readonly value from lvalue subroutine at/);

$_ = undef;
eval <<'EOE' or $_ = $@;
  (lv1tmpr) = (2,3);
  1;
EOE

like($_, qr/Can\'t return a readonly value from lvalue subroutine/);

eval <<'EOF';
  sub lv2tmpr : lvalue { my $x = *foo; Internals::SvREADONLY $x, 1; $x }
  lv2tmpr = (2,3);
EOF

like($@, qr/Can\'t return a readonly value from lvalue subroutine at/);

eval <<'EOG';
  (lv2tmpr) = (2,3);
EOG

like($@, qr/Can\'t return a readonly value from lvalue subroutine/);

sub lva : lvalue {@a}

$_ = undef;
@a = ();
$a[1] = 12;
eval <<'EOE' or $_ = $@;
  (lva) = (2,3);
  1;
EOE

is("'@a' $_", "'2 3' ");

$_ = undef;
@a = ();
$a[0] = undef;
$a[1] = 12;
eval <<'EOE' or $_ = $@;
  (lva) = (2,3);
  1;
EOE

is("'@a' $_", "'2 3' ");

is lva->${\sub { return $_[0] }}, 2,
  'lvalue->$thing when lvalue returns array';

my @my = qw/ a b c /;
sub lvmya : lvalue { @my }

is lvmya->${\sub { return $_[0] }}, 3,
  'lvalue->$thing when lvalue returns lexical array';

sub lv1n : lvalue { $newvar }

$_ = undef;
eval <<'EOE' or $_ = $@;
  lv1n = (3,4);
  1;
EOE

is("'$newvar' $_", "'4' ");

sub lv1nn : lvalue { $nnewvar }

$_ = undef;
eval <<'EOE' or $_ = $@;
  (lv1nn) = (3,4);
  1;
EOE

is("'$nnewvar' $_", "'3' ");

$a = \&lv1nn;
$a->() = 8;
is($nnewvar, '8');

eval 'sub AUTOLOAD : lvalue { $newvar }';
foobar() = 12;
is($newvar, "12");

# But autoloading should only be triggered by a call to an undefined
# subroutine.
&{"lv1nn"} = 14;
is $newvar, 12, 'AUTOLOAD does not take precedence over lvalue sub';
eval { &{"xxx"} = 14 };
is $newvar, 12, 'AUTOLOAD does not take precedence over non-lvalue sub';

{
my %hash; my @array;
sub alv : lvalue { $array[1] }
sub alv2 : lvalue { $array[$_[0]] }
sub hlv : lvalue { $hash{"foo"} }
sub hlv2 : lvalue { $hash{$_[0]} }
$array[1] = "not ok 51\n";
alv() = "ok 50\n";
is(alv(), "ok 50\n");

alv2(20) = "ok 51\n";
is($array[20], "ok 51\n");

$hash{"foo"} = "not ok 52\n";
hlv() = "ok 52\n";
is($hash{foo}, "ok 52\n");

$hash{bar} = "not ok 53\n";
hlv("bar") = "ok 53\n";
is(hlv("bar"), "ok 53\n");

sub array : lvalue  { @array  }
sub array2 : lvalue { @array2 } # This is a global.
sub hash : lvalue   { %hash   }
sub hash2 : lvalue  { %hash2  } # So's this.
@array2 = qw(foo bar);
%hash2 = qw(foo bar);

(array()) = qw(ok 54);
is("@array", "ok 54");

(array2()) = qw(ok 55);
is("@array2", "ok 55");

(hash()) = qw(ok 56);
cmp_ok($hash{ok}, '==', 56);

(hash2()) = qw(ok 57);
cmp_ok($hash2{ok}, '==', 57);

@array = qw(a b c d);
sub aslice1 : lvalue { @array[0,2] };
(aslice1()) = ("ok", "already");
is("@array", "ok b already d");

@array2 = qw(a B c d);
sub aslice2 : lvalue { @array2[0,2] };
(aslice2()) = ("ok", "already");
is("@array2", "ok B already d");

%hash = qw(a Alpha b Beta c Gamma);
sub hslice : lvalue { @hash{"c", "b"} }
(hslice()) = ("CISC", "BogoMIPS");
is(join("/",@hash{"c","a","b"}), "CISC/Alpha/BogoMIPS");
}

$str = "Hello, world!";
sub sstr : lvalue { substr($str, 1, 4) }
sstr() = "i";
is($str, "Hi, world!");

$str = "Made w/ JavaScript";
sub veclv : lvalue { vec($str, 2, 32) }
if ($::IS_ASCII) {
    veclv() = 0x5065726C;
}
else { # EBCDIC?
    veclv() = 0xD7859993;
}
is($str, "Made w/ PerlScript");

sub position : lvalue { pos }
@p = ();
$_ = "fee fi fo fum";
while (/f/g) {
    push @p, position;
    position() += 6;
}
is("@p", "1 8");

SKIP: {
    skip "no Hash::Util on miniperl", 3, if is_miniperl;
    require Hash::Util;
    sub Hash::Util::bucket_ratio (\%);

    sub keeze : lvalue { keys %__ }
    %__ = ("a","b");
    keeze = 64;
    like Hash::Util::bucket_ratio(%__), qr!1/(?:64|128)!, 'keys assignment through lvalue sub';
    eval { (keeze) = 64 };
    like $@, qr/^Can't modify keys in list assignment at /,
         'list assignment to keys through lv sub is forbidden';
    sub akeeze : lvalue { keys @_ }
    eval { (akeeze) = 64 };
    like $@, qr/^Can't modify keys on array in list assignment at /,
         'list assignment to keys @_ through lv sub is forbidden';
}

# Bug 20001223.002 (#5005): split thought that the list had only one element
@ary = qw(4 5 6);
sub lval1 : lvalue { $ary[0]; }
sub lval2 : lvalue { $ary[1]; }
(lval1(), lval2()) = split ' ', "1 2 3 4";

is(join(':', @ary), "1:2:6");

# check that an element of a tied hash/array can be assigned to via lvalueness

package Tie_Hash;

our ($key, $val);
sub TIEHASH { bless \my $v => __PACKAGE__ }
sub STORE   { ($key, $val) = @_[1,2] }

package main;
sub lval_tie_hash : lvalue {
    tie my %t => 'Tie_Hash';
    $t{key};
}

eval { lval_tie_hash() = "value"; };

is($@, "", "element of tied hash");

is("$Tie_Hash::key-$Tie_Hash::val", "key-value");


package Tie_Array;

our @val;
sub TIEARRAY { bless \my $v => __PACKAGE__ }
sub STORE   { $val[ $_[1] ] = $_[2] }

package main;
sub lval_tie_array : lvalue {
    tie my @t => 'Tie_Array';
    $t[0];
}

eval { lval_tie_array() = "value"; };


is($@, "", "element of tied array");

is ($Tie_Array::val[0], "value");


# Check that tied pad vars that are returned can be assigned to
sub TIESCALAR { bless [] }
sub STORE {$wheel = $_[1]}
sub FETCH {$wheel}
sub tied_pad_var  :lvalue { tie my $tyre, ''; $tyre }
sub tied_pad_varr :lvalue { tie my $tyre, ''; return $tyre }
tied_pad_var = 1;
is $wheel, 1, 'tied pad var returned in scalar lvalue context';
tied_pad_var->${\sub{ $_[0] = 2 }};
is $wheel, 2, 'tied pad var returned in scalar ref context';
(tied_pad_var) = 3;
is $wheel, 3, 'tied pad var returned in list lvalue context';
$_ = 4 for tied_pad_var;
is $wheel, 4, 'tied pad var returned in list ref context';
tied_pad_varr = 5;
is $wheel, 5, 'tied pad var explicitly returned in scalar lvalue context';
tied_pad_varr->${\sub{ $_[0] = 6 }};
is $wheel, 6, 'tied pad var explicitly returned in scalar ref context';
(tied_pad_varr) = 7;
is $wheel, 7, 'tied pad var explicitly returned in list lvalue context';
$_ = 8 for tied_pad_varr;
is $wheel, 8, 'tied pad var explicitly returned in list ref context';


# Test explicit return of lvalue expression
{
    # subs are copies from tests 1-~18 with an explicit return added.
    # They used not to work, which is why they are ‘badly’ named.
    sub bad_get_lex : lvalue { return $in };
    sub bad_get_st  : lvalue { return $blah }

    sub bad_id  : lvalue { return ${\shift} }
    sub bad_id1 : lvalue { return $_[0] }
    sub bad_inc : lvalue { return ${\++$_[0]} }

    $in = 5;
    $blah = 3;

    bad_get_st = 7;

    is( $blah, 7 );

    bad_get_lex = 7;

    is($in, 7, "yada");

    ++bad_get_st;

    is($blah, 8, "yada");

    ++bad_get_lex;
    cmp_ok($in, '==', 8);

    bad_id(bad_get_st) = 10;
    cmp_ok($blah, '==', 10);

    bad_id(bad_get_lex) = 10;
    cmp_ok($in, '==', 10);

    ++bad_id(bad_get_st);
    cmp_ok($blah, '==', 11);

    ++bad_id(bad_get_lex);
    cmp_ok($in, '==', 11);

    bad_id1(bad_get_st) = 20;
    cmp_ok($blah, '==', 20);

    bad_id1(bad_get_lex) = 20;
    cmp_ok($in, '==', 20);

    ++bad_id1(bad_get_st);
    cmp_ok($blah, '==', 21);

    ++bad_id1(bad_get_lex);
    cmp_ok($in, '==', 21);

    bad_inc(bad_get_st);
    cmp_ok($blah, '==', 22);

    bad_inc(bad_get_lex);
    cmp_ok($in, '==', 22);

    bad_inc(bad_id(bad_get_st));
    cmp_ok($blah, '==', 23);

    bad_inc(bad_id(bad_get_lex));
    cmp_ok($in, '==', 23);

    ++bad_inc(bad_id1(bad_id(bad_get_st)));
    cmp_ok($blah, '==', 25);

    ++bad_inc(bad_id1(bad_id(bad_get_lex)));
    cmp_ok($in, '==', 25);

    # Recursive
    my $r;
    my $to_modify;
    $r = sub :lvalue {
      my $depth = shift//0;
      if ($depth == 2) { return $to_modify }
      return &$r($depth+1);
    };
    &$r(0) = 7;
    is $to_modify, 7, 'recursive lvalue sub';

    # Recursive with substr [perl #72706]
    my $val = '';
    my $pie;
    $pie = sub :lvalue {
	my $depth = shift;
	return &$pie($depth) if $depth--;
	substr $val, 0;
    };
    for my $depth (0, 1, 2) {
	my $value = "Good $depth";
	eval {
	    &$pie($depth) = $value;
	};
	is($@, '', "recursive lvalue substr return depth $depth");
	is($val, $value,
	   "value assigned to recursive lvalue substr (depth $depth)");
    }
}

{ # bug #23790
    my @arr  = qw /one two three/;
    my $line = "zero";
    sub lval_array () : lvalue {@arr}

    for (lval_array) {
        $line .= $_;
    }

    is($line, "zeroonetwothree");

    sub trythislval { scalar(@_)."x".join "", @_ }
    is(trythislval(lval_array()), "3xonetwothree");

    sub changeme { $_[2] = "free" }
    changeme(lval_array);
    is("@arr", "one two free");

    # test again, with explicit return
    sub rlval_array() : lvalue {return @arr}
    @arr  = qw /one two three/;
    $line = "zero";
    for (rlval_array) {
        $line .= $_;
    }
    is($line, "zeroonetwothree");
    is(trythislval(rlval_array()), "3xonetwothree");
    changeme(rlval_array);
    is("@arr", "one two free");

    # Variations on the same theme, with multiple vars returned
    my $scalar = 'half';
    sub lval_scalar_array () : lvalue { $scalar, @arr }
    @arr  = qw /one two three/;
    $line = "zero";
    for (lval_scalar_array) {
        $line .= $_;
    }
    is($line, "zerohalfonetwothree");
    is(trythislval(lval_scalar_array()), "4xhalfonetwothree");
    changeme(lval_scalar_array);
    is("@arr", "one free three");

    sub lval_array_scalar () : lvalue { @arr, $scalar }
    @arr  = qw /one two three/;
    $line = "zero";
    $scalar = 'four';
    for (lval_array_scalar) {
        $line .= $_;
    }
    is($line, "zeroonetwothreefour");
    is(trythislval(lval_array_scalar()), "4xonetwothreefour");
    changeme(lval_array_scalar);
    is("@arr", "one two free");

    # Tests for specific ops not tested above
    # rv2av
    @array2 = qw 'one two free';
    is join(',', map $_, sub:lvalue{@array2}->()), 'one,two,free',
      'rv2av in reference context';
    is join(',', map $_, sub:lvalue{@{\@array2}}->()), 'one,two,free',
      'rv2av-with-ref in reference context';
    # padhv
    my %hash = qw[a b c d];
    like join(',', map $_, sub:lvalue{%hash}->()),
         qr/^(?:a,b,c,d|c,d,a,b)\z/, 'padhv in reference context';
    # rv2hv
    %hash2 = qw[a b c d];
    like join(',', map $_, sub:lvalue{%hash2}->()),
         qr/^(?:a,b,c,d|c,d,a,b)\z/, 'rv2hv in reference context';
    like join(',', map $_, sub:lvalue{%{\%hash2}}->()),
         qr/^(?:a,b,c,d|c,d,a,b)\z/, 'rv2hv-with-ref in reference context';
}

{
    package Foo;
    sub AUTOLOAD :lvalue { *{$AUTOLOAD} };
    package main;
    my $foo = bless {},"Foo";
    my $result;
    $foo->bar = sub { $result = "bar" };
    $foo->bar;
    is ($result, 'bar', "RT #41550");
}

SKIP: {
  skip 'no attributes.pm', 1 unless eval 'require attributes';
fresh_perl_is(<<'----', <<'====', {}, "lvalue can not be set after definition. [perl #68758]");
use warnings;
our $x;
sub foo { $x }
sub foo : lvalue;
sub MODIFY_CODE_ATTRIBUTES {}
sub foo : lvalue : fr0g;
foo = 3;
----
lvalue attribute ignored after the subroutine has been defined at - line 4.
lvalue attribute ignored after the subroutine has been defined at - line 6.
Can't modify non-lvalue subroutine call of &main::foo in scalar assignment at - line 7, near "3;"
Execution of - aborted due to compilation errors.
====
}

{
    my $x;
    sub lval_decl : lvalue;
    sub lval_decl { $x }
    lval_decl = 5;
    is($x, 5, "subroutine declared with lvalue before definition retains lvalue. [perl #68758]");
}

SKIP: { skip "no attributes.pm", 2 unless eval { require attributes };
  sub utf8::valid :lvalue;
  require attributes;
  is "@{[ &attributes::get(\&utf8::valid) ]}", 'lvalue',
   'sub declaration with :lvalue applies it to XSUBs';

  BEGIN { *wonky = \&marjibberous }
  sub wonky :lvalue;
  is "@{[ &attributes::get(\&wonky) ]}", 'lvalue',
   'sub declaration with :lvalue applies it to assigned stub';
}

sub fleen : lvalue { $pnare }
$pnare = __PACKAGE__;
ok eval { fleen = 1 }, "lvalues can return COWs (CATTLE?) [perl #75656]";\
is $pnare, 1, 'and returning CATTLE actually works';
$pnare = __PACKAGE__;
ok eval { (fleen) = 1 }, "lvalues can return COWs in list context";
is $pnare, 1, 'and returning COWs in list context actually works';
$pnare = __PACKAGE__;
ok eval { $_ = 1 for(fleen); 1 }, "lvalues can return COWs in ref cx";
is $pnare, 1, 'and returning COWs in reference context actually works';


# Returning an arbitrary expression, not necessarily lvalue
+sub :lvalue { return $ambaga || $ambaga }->() = 73;
is $ambaga, 73, 'explicit return of arbitrary expression (scalar context)';
(sub :lvalue { return $ambaga || $ambaga }->()) = 74;
is $ambaga, 74, 'explicit return of arbitrary expression (list context)';
+sub :lvalue { $ambaga || $ambaga }->() = 73;
is $ambaga, 73, 'implicit return of arbitrary expression (scalar context)';
(sub :lvalue { $ambaga || $ambaga }->()) = 74;
is $ambaga, 74, 'implicit return of arbitrary expression (list context)';
eval { +sub :lvalue { return 3 }->() = 4 };
like $@, qr/Can\'t return a readonly value from lvalue subroutine at/,
      'assignment to numeric constant explicitly returned from lv sub';
eval { (sub :lvalue { return 3 }->()) = 4 };
like $@, qr/Can\'t return a readonly value from lvalue subroutine at/,
      'assignment to num constant explicitly returned (list cx)';
eval { +sub :lvalue { 3 }->() = 4 };
like $@, qr/Can\'t return a readonly value from lvalue subroutine at/,
      'assignment to numeric constant implicitly returned from lv sub';
eval { (sub :lvalue { 3 }->()) = 4 };
like $@, qr/Can\'t return a readonly value from lvalue subroutine at/,
      'assignment to num constant implicitly returned (list cx)';

# reference (potential lvalue) context
$suffix = '';
for my $sub (sub :lvalue {$_}, sub :lvalue {return $_}) {
    &$sub()->${\sub { $_[0] = 37 }};
    is $_, '37', 'lvalue->method'.$suffix;
    ${\scalar &$sub()} = 38;
    is $_, '38', 'scalar(lvalue)'.$suffix;
    sub assign39_with_proto ($) { $_[0] = 39 }
    assign39_with_proto(&$sub());
    is $_, '39', 'func(lvalue) when func has $ proto'.$suffix;
    $_ = 1;
    ${\(&$sub()||undef)} = 40;
    is $_, '40', 'lvalue||...'.$suffix;
    ${\(${\undef}||&$sub())} = 41; # extra ${\...} to bypass const folding
    is $_, '41', '...||lvalue'.$suffix;
    $_ = 0;
    ${\(&$sub()&&undef)} = 42;
    is $_, '42', 'lvalue&&...'.$suffix;
    ${\(${\1}&&&$sub())} = 43;
    is $_, '43', '...&&lvalue'.$suffix;
    ${\(&$sub())[0]} = 44;
    is $_, '44', '(lvalue)[0]'.$suffix;
}
continue { $suffix = ' (explicit return)' }

# autovivification
$suffix = '';
for my $sub (sub :lvalue {$_}, sub :lvalue {return $_}) {
    undef $_;
    &$sub()->[3] = 4;
    is $_->[3], 4, 'func->[...] autovivification'.$suffix;
    undef $_;
    &$sub()->{3} = 4;
    is $_->{3}, 4, 'func->{...} autovivification'.$suffix;
    undef $_;
    ${&$sub()} = 4;
    is $$_, 4, '${func()} autovivification'      .$suffix;
    undef $_;
    @{&$sub()} = 4;
    is "@$_", 4, '@{func()} autovivification'    .$suffix;
    undef $_;
    %{&$sub()} = (4,5);
    is join('-',%$_), '4-5', '%{func()} autovivification'.$suffix;
    undef $_;
    ${ (), &$sub()} = 4;
    is $$_, 4, '${ (), func()} autovivification'      .$suffix;
}
continue { $suffix = ' (explicit return)' }

# [perl #92406] [perl #92290] Returning a pad var in rvalue context
$suffix = '';
for my $sub (
         sub :lvalue { my $x = 72; $x },
         sub :lvalue { my $x = 72; return $x }
) {
    is scalar(&$sub), 72, "sub returning pad var in scalar context$suffix";
    is +(&$sub)[0], 72, "sub returning pad var in list context$suffix";
}
continue { $suffix = ' (explicit return)' }

# Returning read-only values in reference context
$suffix = '';
for (
         sub :lvalue { $] }->(),
         sub :lvalue { return $] }->()
) {
    is \$_, \$], 'read-only values are returned in reference context'
	         .$suffix             # (they used to be copied)
}
continue { $suffix = ' (explicit return)' }

# Returning unwritables from nested lvalue sub call in rvalue context
# First, ensure we are testing what we think we are:
if (!Internals::SvREADONLY($])) { Internals::SvREADONLY($],1); }
sub squibble : lvalue { return $] }
sub squebble : lvalue {        squibble }
sub squabble : lvalue { return squibble }
is $x = squebble, $], 'returning ro from nested lv sub call in rv cx';
is $x = squabble, $], 'explct. returning ro from nested lv sub in rv cx';
is \squebble, \$], 'returning ro from nested lv sub call in ref cx';
is \squabble, \$], 'explct. returning ro from nested lv sub in ref cx';

# [perl #102486] Sub calls as the last statement of an lvalue sub
package _102486 {
  my $called;
  my $x = 'nonlv';
  sub strictlv :lvalue { use strict 'refs'; &$x }
  sub lv :lvalue { &$x }
  sub nonlv { ++$called }
  eval { strictlv };
  ::like $@, qr/^Can't use string \("nonlv"\) as a subroutine ref while/,
        'strict mode applies to sub:lvalue{ &$string }';
  $called = 0;
  ::ok eval { lv },
      'sub:lvalue{&$x}->() does not die for non-lvalue inner sub call';
  ::is $called, 1, 'The &$x actually called the sub';
  eval { +sub :lvalue { &$x }->() = 3 };
  ::like $@, qr/^Can't modify non-lvalue subroutine call of &_102486::nonlv at /,
        'sub:lvalue{&$x}->() dies in true lvalue context';
}

# TARG should be copied in rvalue context
sub ucf :lvalue { ucfirst $_[0] }
is ucf("just another ") . ucf("perl hacker,\n"),
   "Just another Perl hacker,\n", 'TARG is copied in rvalue scalar cx';
is join('',ucf("just another "), ucf "perl hacker,\n"),
   "Just another Perl hacker,\n", 'TARG is copied in rvalue list cx';
sub ucfr : lvalue {
    @_ ? ucfirst $_[0] : do {
	is ucfr("just another ") . ucfr("perl hacker,\n"),
	   "Just another Perl hacker,\n",
	   'TARG is copied in recursive rvalue scalar cx';
	is join('',ucfr("just another "), ucfr("perl hacker,\n")),
	   "Just another Perl hacker,\n",
	   'TARG is copied in recursive rvalue list cx';
    }
}
ucfr();

# Test TARG with potential lvalue context, too
for (sub : lvalue { "$x" }->()) {
    is \$_, \$_, '\$_ == \$_ in for(sub :lvalue{"$x"}->()){...}'
}

# [perl #117947] XSUBs should not be treated as lvalues at run time
eval { &{\&utf8::is_utf8}("") = 3 };
like $@, qr/^Can't modify non-lvalue subroutine call of &utf8::is_utf8 at /,
        'XSUB not seen at compile time dies in lvalue context';

# [perl #119797] else implicitly returning value
# This used to cause Bizarre copy of ARRAY in pp_leave
sub else119797 : lvalue {
    if ($_[0]) {
	1; # two statements force a leave op
	@119797
    }
    else {
	@119797
    }
}
eval { (else119797(0)) = 1..3 };
is $@, "", '$@ after writing to array returned by else';
is "@119797", "1 2 3", 'writing to array returned by else';
eval { (else119797(1)) = 4..6 };
is $@, "", '$@ after writing to array returned by if (with else)';
is "@119797", "4 5 6", 'writing to array returned by if (with else)';
sub if119797 : lvalue {
    if ($_[0]) {
	@119797
    }
}
@119797 = ();
eval { (if119797(1)) = 4..6 };
is $@, "", '$@ after writing to array returned by if';
is "@119797", "4 5 6", 'writing to array returned by if';
sub unless119797 : lvalue {
    unless ($_[0]) {
	@119797
    }
}
@119797 = ();
eval { (unless119797(0)) = 4..6 };
is $@, "", '$@ after writing to array returned by unless';
is "@119797", "4 5 6", 'writing to array returned by unless';
sub bare119797 : lvalue {
    {;
	@119797
    }
}
@119797 = ();
eval { (bare119797(0)) = 4..6 };
is $@, "", '$@ after writing to array returned by bare block';
is "@119797", "4 5 6", 'writing to array returned by bare block';

# a sub with nested scopes must pop rubbish on the stack
{
    my $x = "a";
    sub loopreturn : lvalue {
        for (1,2) {
            return $x
        }
    }
    loopreturn = "b";
    is($x, "b", "loopreturn");
}

# a sub without nested scopes that still leaves rubbish on the stack
# which needs popping
{
    my $x = "a";
    sub junkreturn : lvalue {
        my $false;
        return $x unless $false and $false;
        1;
    }
    junkreturn = "b";
    is($x, "b", "junkreturn");
}
