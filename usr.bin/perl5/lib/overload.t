#!./perl -T

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bList/Util\b!) ){
	print "1..0 # Skip -- Perl configured without List::Util module\n";
	exit 0;
    }
}

my $no_taint_support = exists($Config::Config{taint_support})
                     && !$Config::Config{taint_support};

my %skip_fetch_count_when_no_taint = (
    '<${$ts}> RT57012_OV' => 1,
    '<use integer; ${$ts}> RT57012_OV' => 1,
    '<do {&{$ts} for 1,2}> RT57012_OV' => 1,
    '<use integer; do {&{$ts} for 1,2}> RT57012_OV' => 1,
    '<*RT57012B = *{$ts}; our $RT57012B> RT57012_OV' => 1,
    '<use integer; *RT57012B = *{$ts}; our $RT57012B> RT57012_OV' => 1,
);

sub is_if_taint_supported {
    my ($got, $expected, $name, @mess) = @_;
    if ($expected && $no_taint_support) {
        return skip("your perl was built without taint support");
    }
    else {
        return is($got, $expected, $name, @mess);
    }
}


package Oscalar;
use overload ( 
				# Anonymous subroutines:
'+'	=>	sub {new Oscalar $ {$_[0]}+$_[1]},
'-'	=>	sub {new Oscalar
		       $_[2]? $_[1]-${$_[0]} : ${$_[0]}-$_[1]},
'<=>'	=>	sub {new Oscalar
		       $_[2]? $_[1]-${$_[0]} : ${$_[0]}-$_[1]},
'cmp'	=>	sub {new Oscalar
		       $_[2]? ($_[1] cmp ${$_[0]}) : (${$_[0]} cmp $_[1])},
'*'	=>	sub {new Oscalar ${$_[0]}*$_[1]},
'/'	=>	sub {new Oscalar 
		       $_[2]? $_[1]/${$_[0]} :
			 ${$_[0]}/$_[1]},
'%'	=>	sub {new Oscalar
		       $_[2]? $_[1]%${$_[0]} : ${$_[0]}%$_[1]},
'**'	=>	sub {new Oscalar
		       $_[2]? $_[1]**${$_[0]} : ${$_[0]}-$_[1]},

qw(
""	stringify
0+	numify)			# Order of arguments insignificant
);

sub new {
  my $foo = $_[1];
  bless \$foo, $_[0];
}

sub stringify { "${$_[0]}" }
sub numify { 0 + "${$_[0]}" }	# Not needed, additional overhead
				# comparing to direct compilation based on
				# stringify

package main;

$| = 1;
BEGIN { require './test.pl'; require './charset_tools.pl' }
plan tests => 5363;

use Scalar::Util qw(tainted);

$a = new Oscalar "087";
$b= "$a";

is($b, $a);
is($b, "087");
is(ref $a, "Oscalar");
is($a, $a);
is($a, "087");

$c = $a + 7;

is(ref $c, "Oscalar");
isnt($c, $a);
is($c, "94");

$b=$a;

is(ref $a, "Oscalar");

$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "88");
is(ref $a, "Oscalar");

$c=$b;
$c-=$a;

is(ref $c, "Oscalar");
is($a, "087");
is($c, "1");
is(ref $a, "Oscalar");

$b=1;
$b+=$a;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "88");
is(ref $a, "Oscalar");

eval q[ package Oscalar; use overload ('++' => sub { $ {$_[0]}++;$_[0] } ) ];

$b=$a;

is(ref $a, "Oscalar");

$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "88");
is(ref $a, "Oscalar");

package Oscalar;
$dummy=bless \$dummy;		# Now cache of method should be reloaded
package main;

$b=$a;
$b++;				

is(ref $b, "Oscalar");
is($a, "087");
is($b, "88");
is(ref $a, "Oscalar");

undef $b;			# Destroying updates tables too...

eval q[package Oscalar; use overload ('++' => sub { $ {$_[0]} += 2; $_[0] } ) ];

$b=$a;

is(ref $a, "Oscalar");

$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "89");
is(ref $a, "Oscalar");

package Oscalar;
$dummy=bless \$dummy;		# Now cache of method should be reloaded
package main;

$b++;				

is(ref $b, "Oscalar");
is($a, "087");
is($b, "91");
is(ref $a, "Oscalar");

$b=$a;
$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "89");
is(ref $a, "Oscalar");


ok($b? 1:0);

eval q[ package Oscalar; use overload ('=' => sub {$main::copies++; 
						   package Oscalar;
						   local $new=$ {$_[0]};
						   bless \$new } ) ];

$b=new Oscalar "$a";

is(ref $b, "Oscalar");
is($a, "087");
is($b, "087");
is(ref $a, "Oscalar");

$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "89");
is(ref $a, "Oscalar");
is($copies, undef);

$b+=1;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "90");
is(ref $a, "Oscalar");
is($copies, undef);

$b=$a;
$b+=1;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "88");
is(ref $a, "Oscalar");
is($copies, undef);

$b=$a;
$b++;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "89");
is(ref $a, "Oscalar");
is($copies, 1);

eval q[package Oscalar; use overload ('+=' => sub {$ {$_[0]} += 3*$_[1];
						   $_[0] } ) ];
$c=new Oscalar;			# Cause rehash

$b=$a;
$b+=1;

is(ref $b, "Oscalar");
is($a, "087");
is($b, "90");
is(ref $a, "Oscalar");
is($copies, 2);

$b+=$b;

is(ref $b, "Oscalar");
is($b, "360");
is($copies, 2);
$b=-$b;

is(ref $b, "Oscalar");
is($b, "-360");
is($copies, 2);

$b=abs($b);

is(ref $b, "Oscalar");
is($b, "360");
is($copies, 2);

$b=abs($b);

is(ref $b, "Oscalar");
is($b, "360");
is($copies, 2);

eval q[package Oscalar; 
       use overload ('x' => sub {new Oscalar ( $_[2] ? "_.$_[1]._" x $ {$_[0]}
					      : "_.${$_[0]}._" x $_[1])}) ];

$a=new Oscalar "yy";
$a x= 3;
is($a, "_.yy.__.yy.__.yy._");

eval q[package Oscalar; 
       use overload ('.' => sub {new Oscalar ( $_[2] ? 
					      "_.$_[1].__.$ {$_[0]}._"
					      : "_.$ {$_[0]}.__.$_[1]._")}) ];

$a=new Oscalar "xx";

is("b${a}c", "_._.b.__.xx._.__.c._");

# Check inheritance of overloading;
{
  package OscalarI;
  @ISA = 'Oscalar';
}

$aI = new OscalarI "$a";
is(ref $aI, "OscalarI");
is("$aI", "xx");
is($aI, "xx");
is("b${aI}c", "_._.b.__.xx._.__.c._");

# Here we test that both "no overload" and
# blessing to a package update hash

eval "package Oscalar; no overload '.'";

is("b${a}", "bxx");
$x="1";
bless \$x, Oscalar;
is("b${a}c", "bxxc");
new Oscalar 1;
is("b${a}c", "bxxc");

# Negative overloading:

$na = eval { ~$a };
like($@, qr/no method found/);

# Check AUTOLOADING:

*Oscalar::AUTOLOAD = 
  sub { *{"Oscalar::$AUTOLOAD"} = sub {"_!_" . shift() . "_!_"} ;
	goto &{"Oscalar::$AUTOLOAD"}};

eval "package Oscalar; sub comple; use overload '~' => 'comple'";

$na = eval { ~$a };
is($@, '');

bless \$x, Oscalar;

$na = eval { ~$a };		# Hash updated
warn "'$na', $@" if $@;
ok !$@;
is($na, '_!_xx_!_');

$na = 0;

$na = eval { ~$aI };
is($@, '');

bless \$x, OscalarI;

$na = eval { ~$aI };
print $@;

ok(!$@);
is($na, '_!_xx_!_');

eval "package Oscalar; sub rshft; use overload '>>' => 'rshft'";

$na = eval { $aI >> 1 };
is($@, '');

bless \$x, OscalarI;

$na = 0;

$na = eval { $aI >> 1 };
print $@;

ok(!$@);
is($na, '_!_xx_!_');

# warn overload::Method($a, '0+'), "\n";
is(overload::Method($a, '0+'), \&Oscalar::numify);
is(overload::Method($aI,'0+'), \&Oscalar::numify);
ok(overload::Overloaded($aI));
ok(!overload::Overloaded('overload'));

ok(! defined overload::Method($aI, '<<'));
ok(! defined overload::Method($a, '<'));

like (overload::StrVal($aI), qr/^OscalarI=SCALAR\(0x[\da-fA-F]+\)$/);
is(overload::StrVal(\$aI), "@{[\$aI]}");

# Check overloading by methods (specified deep in the ISA tree).
{
  package OscalarII;
  @ISA = 'OscalarI';
  sub Oscalar::lshft {"_<<_" . shift() . "_<<_"}
  eval "package OscalarI; use overload '<<' => 'lshft', '|' => 'lshft'";
}

$aaII = "087";
$aII = \$aaII;
bless $aII, 'OscalarII';
bless \$fake, 'OscalarI';		# update the hash
is(($aI | 3), '_<<_xx_<<_');
# warn $aII << 3;
is(($aII << 3), '_<<_087_<<_');

{
  BEGIN { $int = 7; overload::constant 'integer' => sub {$int++; shift}; }
  $out = 2**10;
}
is($int, 9);
is($out, 1024);
is($int, 9);
{
  BEGIN { overload::constant 'integer' => sub {$int++; shift()+1}; }
  eval q{$out = 42};
}
is($int, 10);
is($out, 43);

$foo = 'foo';
$foo1 = 'f\'o\\o';
{
  BEGIN { $q = $qr = 7; 
	  overload::constant 'q' => sub {$q++; push @q, shift, ($_[1] || 'none'); shift},
			     'qr' => sub {$qr++; push @qr, shift, ($_[1] || 'none'); shift}; }
  $out = 'foo';
  $out1 = 'f\'o\\o';
  $out2 = "a\a$foo,\,";
  /b\b$foo.\./;
}

is($out, 'foo');
is($out, $foo);
is($out1, 'f\'o\\o');
is($out1, $foo1);
is($out2, "a\afoo,\,");
is("@q", "foo q f'o\\\\o q a\\a qq ,\\, qq");
is($q, 11);
is("@qr", "b\\b qq .\\. qq");
is($qr, 9);

{
  $_ = '!<b>!foo!<-.>!';
  BEGIN { overload::constant 'q' => sub {push @q1, shift, ($_[1] || 'none'); "_<" . (shift) . ">_"},
			     'qr' => sub {push @qr1, shift, ($_[1] || 'none'); "!<" . (shift) . ">!"}; }
  $out = 'foo';
  $out1 = 'f\'o\\o';
  $out2 = "a\a$foo,\,";
  $res = /b\b$foo.\./;
  $a = <<EOF;
oups
EOF
  $b = <<'EOF';
oups1
EOF
  $c = bareword;
  m'try it';
  s'first part'second part';
  s/yet another/tail here/;
  tr/A-Z/a-z/;
}

is($out, '_<foo>_');
is($out1, '_<f\'o\\o>_');
is($out2, "_<a\a>_foo_<,\,>_");
is("@q1", "foo q f'o\\\\o q a\\a qq ,\\, qq oups
 qq oups1
 q second part q tail here s A-Z tr a-z tr");
is("@qr1", "b\\b qq .\\. qq try it q first part q yet another qq");
is($res, 1);
is($a, "_<oups
>_");
is($b, "_<oups1
>_");
is($c, "bareword");

{
  package symbolic;		# Primitive symbolic calculator
  use overload nomethod => \&wrap, '""' => \&str, '0+' => \&num,
      '=' => \&cpy, '++' => \&inc, '--' => \&dec;

  sub new { shift; bless ['n', @_] }
  sub cpy {
    my $self = shift;
    bless [@$self], ref $self;
  }
  sub inc { $_[0] = bless ['++', $_[0], 1]; }
  sub dec { $_[0] = bless ['--', $_[0], 1]; }
  sub wrap {
    my ($obj, $other, $inv, $meth) = @_;
    if ($meth eq '++' or $meth eq '--') {
      @$obj = ($meth, (bless [@$obj]), 1); # Avoid circular reference
      return $obj;
    }
    ($obj, $other) = ($other, $obj) if $inv;
    bless [$meth, $obj, $other];
  }
  sub str {
    my ($meth, $a, $b) = @{+shift};
    $a = 'u' unless defined $a;
    if (defined $b) {
      "[$meth $a $b]";
    } else {
      "[$meth $a]";
    }
  } 
  my %subr = ( 'n' => sub {$_[0]} );
  foreach my $op (split " ", $overload::ops{with_assign}) {
    $subr{$op} = $subr{"$op="} = eval "sub {shift() $op shift()}";
  }
  my @bins = qw(binary 3way_comparison num_comparison str_comparison);
  foreach my $op (split " ", "@overload::ops{ @bins }") {
    $subr{$op} = eval "sub {shift() $op shift()}";
  }
  foreach my $op (split " ", "@overload::ops{qw(unary func)}") {
    $subr{$op} = eval "sub {$op shift()}";
  }
  $subr{'++'} = $subr{'+'};
  $subr{'--'} = $subr{'-'};
  
  sub num {
    my ($meth, $a, $b) = @{+shift};
    my $subr = $subr{$meth} 
      or die "Do not know how to ($meth) in symbolic";
    $a = $a->num if ref $a eq __PACKAGE__;
    $b = $b->num if ref $b eq __PACKAGE__;
    $subr->($a,$b);
  }
  sub TIESCALAR { my $pack = shift; $pack->new(@_) }
  sub FETCH { shift }
  sub nop {  }		# Around a bug
  sub vars { my $p = shift; tie($_, $p), $_->nop foreach @_; }
  sub STORE { 
    my $obj = shift; 
    $#$obj = 1; 
    $obj->[1] = shift;
  }
}

{
  my $foo = new symbolic 11;
  my $baz = $foo++;
  is((sprintf "%d", $foo), '12');
  is((sprintf "%d", $baz), '11');
  my $bar = $foo;
  $baz = ++$foo;
  is((sprintf "%d", $foo), '13');
  is((sprintf "%d", $bar), '12');
  is((sprintf "%d", $baz), '13');
  my $ban = $foo;
  $baz = ($foo += 1);
  is((sprintf "%d", $foo), '14');
  is((sprintf "%d", $bar), '12');
  is((sprintf "%d", $baz), '14');
  is((sprintf "%d", $ban), '13');
  $baz = 0;
  $baz = $foo++;
  is((sprintf "%d", $foo), '15');
  is((sprintf "%d", $baz), '14');
  is("$foo", '[++ [+= [++ [++ [n 11] 1] 1] 1] 1]');
}

{
  my $iter = new symbolic 2;
  my $side = new symbolic 1;
  my $cnt = $iter;
  
  while ($cnt) {
    $cnt = $cnt - 1;		# The "simple" way
    $side = (sqrt(1 + $side**2) - 1)/$side;
  }
  my $pi = $side*(2**($iter+2));
  is("$side", '[/ [- [sqrt [+ 1 [** [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]] 2]]] 1] [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]]]');
  is((sprintf "%f", $pi), '3.182598');
}

{
  my $iter = new symbolic 2;
  my $side = new symbolic 1;
  my $cnt = $iter;
  
  while ($cnt--) {
    $side = (sqrt(1 + $side**2) - 1)/$side;
  }
  my $pi = $side*(2**($iter+2));
  is("$side", '[/ [- [sqrt [+ 1 [** [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]] 2]]] 1] [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]]]');
  is((sprintf "%f", $pi), '3.182598');
}

{
  my ($a, $b);
  symbolic->vars($a, $b);
  my $c = sqrt($a**2 + $b**2);
  $a = 3; $b = 4;
  is((sprintf "%d", $c), '5');
  $a = 12; $b = 5;
  is((sprintf "%d", $c), '13');
}

{
  package symbolic1;		# Primitive symbolic calculator
  # Mutator inc/dec
  use overload nomethod => \&wrap, '""' => \&str, '0+' => \&num, '=' => \&cpy;

  sub new { shift; bless ['n', @_] }
  sub cpy {
    my $self = shift;
    bless [@$self], ref $self;
  }
  sub wrap {
    my ($obj, $other, $inv, $meth) = @_;
    if ($meth eq '++' or $meth eq '--') {
      @$obj = ($meth, (bless [@$obj]), 1); # Avoid circular reference
      return $obj;
    }
    ($obj, $other) = ($other, $obj) if $inv;
    bless [$meth, $obj, $other];
  }
  sub str {
    my ($meth, $a, $b) = @{+shift};
    $a = 'u' unless defined $a;
    if (defined $b) {
      "[$meth $a $b]";
    } else {
      "[$meth $a]";
    }
  } 
  my %subr = ( 'n' => sub {$_[0]} );
  foreach my $op (split " ", $overload::ops{with_assign}) {
    $subr{$op} = $subr{"$op="} = eval "sub {shift() $op shift()}";
  }
  my @bins = qw(binary 3way_comparison num_comparison str_comparison);
  foreach my $op (split " ", "@overload::ops{ @bins }") {
    $subr{$op} = eval "sub {shift() $op shift()}";
  }
  foreach my $op (split " ", "@overload::ops{qw(unary func)}") {
    $subr{$op} = eval "sub {$op shift()}";
  }
  $subr{'++'} = $subr{'+'};
  $subr{'--'} = $subr{'-'};
  
  sub num {
    my ($meth, $a, $b) = @{+shift};
    my $subr = $subr{$meth} 
      or die "Do not know how to ($meth) in symbolic";
    $a = $a->num if ref $a eq __PACKAGE__;
    $b = $b->num if ref $b eq __PACKAGE__;
    $subr->($a,$b);
  }
  sub TIESCALAR { my $pack = shift; $pack->new(@_) }
  sub FETCH { shift }
  sub vars { my $p = shift; tie($_, $p) foreach @_; }
  sub STORE { 
    my $obj = shift; 
    $#$obj = 1; 
    $obj->[1] = shift;
  }
}

{
  my $foo = new symbolic1 11;
  my $baz = $foo++;
  is((sprintf "%d", $foo), '12');
  is((sprintf "%d", $baz), '11');
  my $bar = $foo;
  $baz = ++$foo;
  is((sprintf "%d", $foo), '13');
  is((sprintf "%d", $bar), '12');
  is((sprintf "%d", $baz), '13');
  my $ban = $foo;
  $baz = ($foo += 1);
  is((sprintf "%d", $foo), '14');
  is((sprintf "%d", $bar), '12');
  is((sprintf "%d", $baz), '14');
  is((sprintf "%d", $ban), '13');
  $baz = 0;
  $baz = $foo++;
  is((sprintf "%d", $foo), '15');
  is((sprintf "%d", $baz), '14');
  is("$foo", '[++ [+= [++ [++ [n 11] 1] 1] 1] 1]');
}

{
  my $iter = new symbolic1 2;
  my $side = new symbolic1 1;
  my $cnt = $iter;
  
  while ($cnt) {
    $cnt = $cnt - 1;		# The "simple" way
    $side = (sqrt(1 + $side**2) - 1)/$side;
  }
  my $pi = $side*(2**($iter+2));
  is("$side", '[/ [- [sqrt [+ 1 [** [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]] 2]]] 1] [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]]]');
  is((sprintf "%f", $pi), '3.182598');
}

{
  my $iter = new symbolic1 2;
  my $side = new symbolic1 1;
  my $cnt = $iter;
  
  while ($cnt--) {
    $side = (sqrt(1 + $side**2) - 1)/$side;
  }
  my $pi = $side*(2**($iter+2));
  is("$side", '[/ [- [sqrt [+ 1 [** [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]] 2]]] 1] [/ [- [sqrt [+ 1 [** [n 1] 2]]] 1] [n 1]]]');
  is((sprintf "%f", $pi), '3.182598');
}

{
  my ($a, $b);
  symbolic1->vars($a, $b);
  my $c = sqrt($a**2 + $b**2);
  $a = 3; $b = 4;
  is((sprintf "%d", $c), '5');
  $a = 12; $b = 5;
  is((sprintf "%d", $c), '13');
}

{
  package two_face;		# Scalars with separate string and
                                # numeric values.
  sub new { my $p = shift; bless [@_], $p }
  use overload '""' => \&str, '0+' => \&num, fallback => 1;
  sub num {shift->[1]}
  sub str {shift->[0]}
}

{
  my $seven = new two_face ("vii", 7);
  is((sprintf "seven=$seven, seven=%d, eight=%d", $seven, $seven+1),
	'seven=vii, seven=7, eight=8');
  is(scalar ($seven =~ /i/), '1');
}

{
  package sorting;
  use overload 'cmp' => \&comp;
  sub new { my ($p, $v) = @_; bless \$v, $p }
  sub comp { my ($x,$y) = @_; ($$x * 3 % 10) <=> ($$y * 3 % 10) or $$x cmp $$y }
}
{
  my @arr = map sorting->new($_), 0..12;
  my @sorted1 = sort @arr;
  my @sorted2 = map $$_, @sorted1;
  is("@sorted2", '0 10 7 4 1 11 8 5 12 2 9 6 3');
}
{
  package iterator;
  use overload '<>' => \&iter;
  sub new { my ($p, $v) = @_; bless \$v, $p }
  sub iter { my ($x) = @_; return undef if $$x < 0; return $$x--; }
}

{
  my $iter = iterator->new(5);
  my $acc = '';
  my $out;
  $acc .= " $out" while $out = <${iter}>;
  is($acc, ' 5 4 3 2 1 0');
  $iter = iterator->new(5);
  is(scalar <${iter}>, '5');
  $acc = '';
  $acc .= " $out" while $out = <$iter>;
  is($acc, ' 4 3 2 1 0');
}
{
  package deref;
  use overload '%{}' => \&hderef, '&{}' => \&cderef, 
    '*{}' => \&gderef, '${}' => \&sderef, '@{}' => \&aderef;
  sub new { my ($p, $v) = @_; bless \$v, $p }
  sub deref {
    my ($self, $key) = (shift, shift);
    my $class = ref $self;
    bless $self, 'deref::dummy'; # Disable overloading of %{} 
    my $out = $self->{$key};
    bless $self, $class;	# Restore overloading
    $out;
  }
  sub hderef {shift->deref('h')}
  sub aderef {shift->deref('a')}
  sub cderef {shift->deref('c')}
  sub gderef {shift->deref('g')}
  sub sderef {shift->deref('s')}
}
{
  my $deref = bless { h => { foo => 5 , fake => 23 },
		      c => sub {return shift() + 34},
		      's' => \123,
		      a => [11..13],
		      g => \*srt,
		    }, 'deref';
  # Hash:
  my @cont = sort %$deref;
  if ("\t" eq "\011") { # ASCII
      is("@cont", '23 5 fake foo');
  } 
  else {                # EBCDIC alpha-numeric sort order
      is("@cont", 'fake foo 23 5');
  }
  my @keys = sort keys %$deref;
  is("@keys", 'fake foo');
  my @val = sort values %$deref;
  is("@val", '23 5');
  is($deref->{foo}, 5);
  is(defined $deref->{bar}, '');
  my $key;
  @keys = ();
  push @keys, $key while $key = each %$deref;
  @keys = sort @keys;
  is("@keys", 'fake foo');
  is(exists $deref->{bar}, '');
  is(exists $deref->{foo}, 1);
  # Code:
  is($deref->(5), 39);
  is(&$deref(6), 40);
  sub xxx_goto { goto &$deref }
  is(xxx_goto(7), 41);
  my $srt = bless { c => sub {$b <=> $a}
		  }, 'deref';
  *srt = \&$srt;
  my @sorted = sort srt 11, 2, 5, 1, 22;
  is("@sorted", '22 11 5 2 1');
  # Scalar
  is($$deref, 123);
  # Code
  @sorted = sort $srt 11, 2, 5, 1, 22;
  is("@sorted", '22 11 5 2 1');
  # Array
  is("@$deref", '11 12 13');
  is($#$deref, '2');
  my $l = @$deref;
  is($l, 3);
  is($deref->[2], '13');
  $l = pop @$deref;
  is($l, 13);
  $l = 1;
  is($deref->[$l], '12');
  # Repeated dereference
  my $double = bless { h => $deref,
		     }, 'deref';
  is($double->{foo}, 5);
}

{
  package two_refs;
  use overload '%{}' => \&gethash, '@{}' => sub { ${shift()} };
  sub new { 
    my $p = shift; 
    bless \ [@_], $p;
  }
  sub gethash {
    my %h;
    my $self = shift;
    tie %h, ref $self, $self;
    \%h;
  }

  sub TIEHASH { my $p = shift; bless \ shift, $p }
  my %fields;
  my $i = 0;
  $fields{$_} = $i++ foreach qw{zero one two three};
  sub STORE { 
    my $self = ${shift()};
    my $key = $fields{shift()};
    defined $key or die "Out of band access";
    $$self->[$key] = shift;
  }
  sub FETCH { 
    my $self = ${shift()};
    my $key = $fields{shift()};
    defined $key or die "Out of band access";
    $$self->[$key];
  }
}

my $bar = new two_refs 3,4,5,6;
$bar->[2] = 11;
is($bar->{two}, 11);
$bar->{three} = 13;
is($bar->[3], 13);

{
  package two_refs_o;
  @ISA = ('two_refs');
}

$bar = new two_refs_o 3,4,5,6;
$bar->[2] = 11;
is($bar->{two}, 11);
$bar->{three} = 13;
is($bar->[3], 13);

{
  package two_refs1;
  use overload '%{}' => sub { ${shift()}->[1] },
               '@{}' => sub { ${shift()}->[0] };
  sub new { 
    my $p = shift; 
    my $a = [@_];
    my %h;
    tie %h, $p, $a;
    bless \ [$a, \%h], $p;
  }
  sub gethash {
    my %h;
    my $self = shift;
    tie %h, ref $self, $self;
    \%h;
  }

  sub TIEHASH { my $p = shift; bless \ shift, $p }
  my %fields;
  my $i = 0;
  $fields{$_} = $i++ foreach qw{zero one two three};
  sub STORE { 
    my $a = ${shift()};
    my $key = $fields{shift()};
    defined $key or die "Out of band access";
    $a->[$key] = shift;
  }
  sub FETCH { 
    my $a = ${shift()};
    my $key = $fields{shift()};
    defined $key or die "Out of band access";
    $a->[$key];
  }
}

$bar = new two_refs_o 3,4,5,6;
$bar->[2] = 11;
is($bar->{two}, 11);
$bar->{three} = 13;
is($bar->[3], 13);

{
  package two_refs1_o;
  @ISA = ('two_refs1');
}

$bar = new two_refs1_o 3,4,5,6;
$bar->[2] = 11;
is($bar->{two}, 11);
$bar->{three} = 13;
is($bar->[3], 13);

{
  package B;
  use overload bool => sub { ${+shift} };
}

my $aaa;
{ my $bbbb = 0; $aaa = bless \$bbbb, B }

is !$aaa, 1;

unless ($aaa) {
  pass();
} else {
  fail();
}

# check that overload isn't done twice by join
{ my $c = 0;
  package Join;
  use overload '""' => sub { $c++ };
  my $x = join '', bless([]), 'pq', bless([]);
  main::is $x, '0pq1';
};

# Test module-specific warning
{
    # check the Odd number of arguments for overload::constant warning
    my $a = "" ;
    local $SIG{__WARN__} = sub {$a = $_[0]} ;
    $x = eval ' overload::constant "integer" ; ' ;
    is($a, "");
    use warnings 'overload' ;
    $x = eval ' overload::constant "integer" ; ' ;
    like($a, qr/^Odd number of arguments for overload::constant at/);
}

{
    # check the '$_[0]' is not an overloadable type warning
    my $a = "" ;
    local $SIG{__WARN__} = sub {$a = $_[0]} ;
    $x = eval ' overload::constant "fred" => sub {} ; ' ;
    is($a, "");
    use warnings 'overload' ;
    $x = eval ' overload::constant "fred" => sub {} ; ' ;
    like($a, qr/^'fred' is not an overloadable type at/);
}

{
    # check the '$_[1]' is not a code reference warning
    my $a = "" ;
    local $SIG{__WARN__} = sub {$a = $_[0]} ;
    $x = eval ' overload::constant "integer" => 1; ' ;
    is($a, "");
    use warnings 'overload' ;
    $x = eval ' overload::constant "integer" => 1; ' ;
    like($a, qr/^'1' is not a code reference at/);
}

{
    # check the invalid argument warning [perl #74098]
    my $a = "" ;
    local $SIG{__WARN__} = sub {$a = $_[0]} ;
    $x = eval ' use overload "~|_|~" => sub{} ' ;
    eval ' no overload "~|_|~" ' ;
    is($a, "");
    use warnings 'overload' ;
    $x = eval ' use overload "~|_|~" => sub{} ' ;
    like($a, qr/^overload arg '~\|_\|~' is invalid at \(eval \d+\) line /,
	'invalid arg warning');
    undef $a;
    eval ' no overload "~|_|~" ' ;
    like($a, qr/^overload arg '~\|_\|~' is invalid at \(eval \d+\) line /,
	'invalid arg warning');
}

{
  my $c = 0;
  package ov_int1;
  use overload '""'    => sub { 3+shift->[0] },
               '0+'    => sub { 10+shift->[0] },
               'int'   => sub { 100+shift->[0] };
  sub new {my $p = shift; bless [shift], $p}

  package ov_int2;
  use overload '""'    => sub { 5+shift->[0] },
               '0+'    => sub { 30+shift->[0] },
               'int'   => sub { 'ov_int1'->new(1000+shift->[0]) };
  sub new {my $p = shift; bless [shift], $p}

  package noov_int;
  use overload '""'    => sub { 2+shift->[0] },
               '0+'    => sub { 9+shift->[0] };
  sub new {my $p = shift; bless [shift], $p}

  package main;

  my $x = new noov_int 11;
  my $int_x = int $x;
  main::is("$int_x", 20);
  $x = new ov_int1 31;
  $int_x = int $x;
  main::is("$int_x", 131);
  $x = new ov_int2 51;
  $int_x = int $x;
  main::is("$int_x", 1054);
}

# make sure that we don't infinitely recurse
{
  my $c = 0;
  package Recurse;
  use overload '""'    => sub { shift },
               '0+'    => sub { shift },
               'bool'  => sub { shift },
               fallback => 1;
  my $x = bless([]);
  # For some reason beyond me these have to be oks rather than likes.
  main::ok("$x" =~ /Recurse=ARRAY/);
  main::ok($x);
  main::ok($x+0 =~ qr/Recurse=ARRAY/);
}

# BugID 20010422.003 (#6872)
package Foo;

use overload
  'bool' => sub { return !$_[0]->is_zero() || undef; }
;
 
sub is_zero
  {
  my $self = shift;
  return $self->{var} == 0;
  }

sub new
  {
  my $class = shift;
  my $self =  {};
  $self->{var} = shift;
  bless $self,$class;
  }

package main;

use strict;

my $r = Foo->new(8);
$r = Foo->new(0);

is(($r || 0), 0);

package utf8_o;

use overload 
  '""'  =>  sub { return $_[0]->{var}; }
  ;
  
sub new
  {
    my $class = shift;
    my $self =  {};
    $self->{var} = shift;
    bless $self,$class;
  }

package main;


my $utfvar = new utf8_o 200.2.1;
is("$utfvar", 200.2.1); # 223 - stringify
is("a$utfvar", "a".200.2.1); # 224 - overload via sv_2pv_flags

# 225..227 -- more %{} tests.  Hangs in 5.6.0, okay in later releases.
# Basically this example implements strong encapsulation: if Hderef::import()
# were to eval the overload code in the caller's namespace, the privatisation
# would be quite transparent.
package Hderef;
use overload '%{}' => sub { caller(0) eq 'Foo' ? $_[0] : die "zap" };
package Foo;
@Foo::ISA = 'Hderef';
sub new { bless {}, shift }
sub xet { @_ == 2 ? $_[0]->{$_[1]} :
	  @_ == 3 ? ($_[0]->{$_[1]} = $_[2]) : undef }
package main;
my $a = Foo->new;
$a->xet('b', 42);
is ($a->xet('b'), 42);
ok (!defined eval { $a->{b} });
like ($@, qr/zap/);

{
   package t229;
   use overload '='  => sub { 42 },
                '++' => sub { my $x = ${$_[0]}; $_[0] };
   sub new { my $x = 42; bless \$x }

   my $warn;
   {  
     local $SIG{__WARN__} = sub { $warn++ };
      my $x = t229->new;
      my $y = $x;
      eval { $y++ };
   }
   main::ok (!$warn);
}

{
    my ($int, $out1, $out2);
    {
        BEGIN { $int = 0; overload::constant 'integer' => sub {$int++; 17}; }
        $out1 = 0;
        $out2 = 1;
    }
    is($int,  2,  "#24313");	# 230
    is($out1, 17, "#24313");	# 231
    is($out2, 17, "#24313");	# 232
}

{
    package perl31793;
    use overload cmp => sub { 0 };
    package perl31793_fb;
    use overload cmp => sub { 0 }, fallback => 1;
    package main;
    my $o  = bless [], 'perl31793';
    my $of = bless [], 'perl31793_fb';
    my $no = bless [], 'no_overload';
    like(overload::StrVal(\"scalar"), qr/^SCALAR\(0x[0-9a-f]+\)$/);
    like(overload::StrVal([]),        qr/^ARRAY\(0x[0-9a-f]+\)$/);
    like(overload::StrVal({}),        qr/^HASH\(0x[0-9a-f]+\)$/);
    like(overload::StrVal(sub{1}),    qr/^CODE\(0x[0-9a-f]+\)$/);
    like(overload::StrVal(\*GLOB),    qr/^GLOB\(0x[0-9a-f]+\)$/);
    like(overload::StrVal(\$o),       qr/^REF\(0x[0-9a-f]+\)$/);
    like(overload::StrVal(qr/a/),     qr/^Regexp=REGEXP\(0x[0-9a-f]+\)$/);
    like(overload::StrVal($o),        qr/^perl31793=ARRAY\(0x[0-9a-f]+\)$/);
    like(overload::StrVal($of),       qr/^perl31793_fb=ARRAY\(0x[0-9a-f]+\)$/);
    like(overload::StrVal($no),       qr/^no_overload=ARRAY\(0x[0-9a-f]+\)$/);
}

{
    package Numify;
    use overload (qw(0+ numify fallback 1));

    sub new {
	my $val = $_[1];
	bless \$val, $_[0];
    }

    sub numify { ${$_[0]} }
}

# These all check that overloaded values, rather than reference addresses,
# are what are getting tested.
my ($two, $one, $un, $deux) = map {new Numify $_} 2, 1, 1, 2;
my ($ein, $zwei) = (1, 2);

my %map = (one => 1, un => 1, ein => 1, deux => 2, two => 2, zwei => 2);
foreach my $op (qw(<=> == != < <= > >=)) {
    foreach my $l (keys %map) {
	foreach my $r (keys %map) {
	    my $ocode = "\$$l $op \$$r";
	    my $rcode = "$map{$l} $op $map{$r}";

	    my $got = eval $ocode;
	    die if $@;
	    my $expect = eval $rcode;
	    die if $@;
	    is ($got, $expect, $ocode) or print "# $rcode\n";
	}
    }
}
{
    # check that overloading works in regexes
    {
	package Foo493;
	use overload
	    '""' => sub { "^$_[0][0]\$" },
	    '.'  => sub { 
		    bless [
			     $_[2]
			    ? (ref $_[1] ? $_[1][0] : $_[1]) . ':' .$_[0][0] 
			    : $_[0][0] . ':' . (ref $_[1] ? $_[1][0] : $_[1])
		    ], 'Foo493'
			};
    }

    my $a = bless [ "a" ], 'Foo493';
    like('a', qr/$a/);
    like('x:a', qr/x$a/);
    like('x:a:=', qr/x$a=$/);
    like('x:a:a:=', qr/x$a$a=$/);

}

{
    {
        package QRonly;
        use overload qr => sub { qr/x/ }, fallback => 1;
    }
    {
        my $x = bless [], "QRonly";

        # like tries to be too clever, and decides that $x-stringified
        # doesn't look like a regex
        ok("x" =~ $x, "qr-only matches");
        ok("y" !~ $x, "qr-only doesn't match what it shouldn't");
        ok("x" =~ /^(??{$x})$/, "qr-only with ?? matches");
        ok("y" !~ /^(??{$x})$/, "qr-only with ?? doesn't match what it shouldn't");
        ok("xx" =~ /x$x/, "qr-only matches with concat");
        like("$x", qr/^QRonly=ARRAY/, "qr-only doesn't have string overload");

        my $qr = bless qr/y/, "QRonly";
        ok("x" =~ $qr, "qr with qr-overload uses overload");
        ok("y" !~ $qr, "qr with qr-overload uses overload");
	ok("x" =~ /^(??{$qr})$/, "qr with qr-overload with ?? uses overload");
	ok("y" !~ /^(??{$qr})$/, "qr with qr-overload with ?? uses overload");
        is("$qr", "".qr/y/, "qr with qr-overload stringify");

        my $rx = $$qr;
        ok("y" =~ $rx, "bare rx with qr-overload doesn't overload match");
        ok("x" !~ $rx, "bare rx with qr-overload doesn't overload match");
        ok("y" =~ /^(??{$rx})$/, "bare rx with qr-overload with ?? doesn't overload match");
        ok("x" !~ /^(??{$rx})$/, "bare rx with qr-overload with ?? doesn't overload match");
        is("$rx", "".qr/y/, "bare rx with qr-overload stringify");
    }
    {
        package QRandSTR;
        use overload qr => sub { qr/x/ }, q/""/ => sub { "y" };
    }
    {
        my $x = bless [], "QRandSTR";
        ok("x" =~ $x, "qr+str uses qr for match");
        ok("y" !~ $x, "qr+str uses qr for match");
        ok("xx" =~ /x$x/, "qr+str uses qr for match with concat");
        is("$x", "y", "qr+str uses str for stringify");

        my $qr = bless qr/z/, "QRandSTR";
        is("$qr", "y", "qr with qr+str uses str for stringify");
        ok("xx" =~ /x$x/, "qr with qr+str uses qr for match");

        my $rx = $$qr;
        ok("z" =~ $rx, "bare rx with qr+str doesn't overload match");
        is("$rx", "".qr/z/, "bare rx with qr+str doesn't overload stringify");
    }
    {
        package QRany;
        use overload qr => sub { $_[0]->(@_) };

        package QRself;
        use overload qr => sub { $_[0] };
    }
    {
        my $rx = bless sub { ${ qr/x/ } }, "QRany";
        ok("x" =~ $rx, "qr overload accepts a bare rx");
        ok("y" !~ $rx, "qr overload accepts a bare rx");

        my $str = bless sub { "x" }, "QRany";
        ok(!eval { "x" =~ $str }, "qr overload doesn't accept a string");
        like($@, qr/^Overloaded qr did not return a REGEXP/, "correct error");

        my $oqr = bless qr/z/, "QRandSTR";
        my $oqro = bless sub { $oqr }, "QRany";
        ok("z" =~ $oqro, "qr overload doesn't recurse");

        my $qrs = bless qr/z/, "QRself";
        ok("z" =~ $qrs, "qr overload can return self");
    }
    {
        package STRonly;
        use overload q/""/ => sub { "x" };

        package STRonlyFB;
        use overload q/""/ => sub { "x" }, fallback => 1;
    }
    {
        my $fb = bless [], "STRonlyFB";
        ok("x" =~ $fb, "qr falls back to \"\"");
        ok("y" !~ $fb, "qr falls back to \"\"");

        my $nofb = bless [], "STRonly";
        ok("x" =~ $nofb, "qr falls back even without fallback");
        ok("y" !~ $nofb, "qr falls back even without fallback");
    }
}

{
    my $twenty_three = 23;
    # Check that constant overloading propagates into evals
    BEGIN { overload::constant integer => sub { 23 } }
    is(eval "17", $twenty_three);
}

{
    # Check readonliness of constants, whether shared hash key
    # scalars or no (brought up in bug #109744)
    BEGIN { overload::constant integer => sub { "main" }; }
    eval { ${\5} = 'whatever' };
    like $@, qr/^Modification of a read-only value attempted at /,
	'constant overloading makes read-only constants';
    BEGIN { overload::constant integer => sub { __PACKAGE__ }; }
    eval { ${\5} = 'whatever' };
    like $@, qr/^Modification of a read-only value attempted at /,
	'... even with shared hash key scalars';
}

{
    package Sklorsh;
    use overload
	bool     => sub { shift->is_cool };

    sub is_cool {
	$_[0]->{name} eq 'cool';
    }

    sub delete {
	undef %{$_[0]};
	bless $_[0], 'Brap';
	return 1;
    }

    sub delete_with_self {
	my $self = shift;
	undef %$self;
	bless $self, 'Brap';
	return 1;
    }

    package Brap;

    1;

    package main;

    my $obj;
    $obj = bless {name => 'cool'}, 'Sklorsh';
    $obj->delete;
    ok(eval {if ($obj) {1}; 1}, $@ || 'reblessed into nonexistent namespace');

    $obj = bless {name => 'cool'}, 'Sklorsh';
    $obj->delete_with_self;
    ok (eval {if ($obj) {1}; 1}, $@);
    
    my $a = $b = {name => 'hot'};
    bless $b, 'Sklorsh';
    is(ref $a, 'Sklorsh');
    is(ref $b, 'Sklorsh');
    ok(!$b, "Expect overloaded boolean");
    ok(!$a, "Expect overloaded boolean");
}

{
    package Flrbbbbb;
    use overload
	bool	 => sub { shift->{truth} eq 'yes' },
	'0+'	 => sub { shift->{truth} eq 'yes' ? '1' : '0' },
	'!'	 => sub { shift->{truth} eq 'no' },
	fallback => 1;

    sub new { my $class = shift; bless { truth => shift }, $class }

    package main;

    my $yes = Flrbbbbb->new('yes');
    my $x;
    $x = 1 if $yes;			is($x, 1);
    $x = 2 unless $yes;			is($x, 1);
    $x = 3 if !$yes;			is($x, 1);
    $x = 4 unless !$yes;		is($x, 4);

    my $no = Flrbbbbb->new('no');
    $x = 0;
    $x = 1 if $no;			is($x, 0);
    $x = 2 unless $no;			is($x, 2);
    $x = 3 if !$no;			is($x, 3);
    $x = 4 unless !$no;			is($x, 3);

    $x = 0;
    $x = 1 if !$no && $yes;		is($x, 1);
    $x = 2 unless !$no && $yes;		is($x, 1);
    $x = 3 if $no || !$yes;		is($x, 1);
    $x = 4 unless $no || !$yes;		is($x, 4);

    $x = 0;
    $x = 1 if !$no || !$yes;		is($x, 1);
    $x = 2 unless !$no || !$yes;	is($x, 1);
    $x = 3 if !$no && !$yes;		is($x, 1);
    $x = 4 unless !$no && !$yes;	is($x, 4);
}

{
    no warnings 'experimental::builtin';
    use builtin 'weaken';

    package Shklitza;
    use overload '""' => sub {"CLiK KLAK"};

    package Ksshfwoom;

    package main;

    my ($obj, $ref);
    $obj = bless do {my $a; \$a}, 'Shklitza';
    $ref = $obj;

    is ("$obj", "CLiK KLAK");
    is ("$ref", "CLiK KLAK");

    weaken $ref;
    is ("$ref", "CLiK KLAK");

    bless $obj, 'Ksshfwoom';

    like ($obj, qr/^Ksshfwoom=/);
    like ($ref, qr/^Ksshfwoom=/);

    undef $obj;
    is ($ref, undef);
}

{
    package bit;
    # bit operations have overloadable assignment variants too

    sub new { bless \$_[1], $_[0] }

    use overload
          "&=" => sub { bit->new($_[0]->val . ' & ' . $_[1]->val) }, 
          "^=" => sub { bit->new($_[0]->val . ' ^ ' . $_[1]->val) },
          "|"  => sub { bit->new($_[0]->val . ' | ' . $_[1]->val) }, # |= by fallback
          ;

    sub val { ${$_[0]} }

    package main;

    my $a = bit->new(my $va = 'a');
    my $b = bit->new(my $vb = 'b');

    $a &= $b;
    is($a->val, 'a & b', "overloaded &= works");

    my $c = bit->new(my $vc = 'c');

    $b ^= $c;
    is($b->val, 'b ^ c', "overloaded ^= works");

    my $d = bit->new(my $vd = 'd');

    $c |= $d;
    is($c->val, 'c | d', "overloaded |= (by fallback) works");
}

{
    # comparison operators with nomethod (bug 41546)
    my $warning = "";
    my $method;

    package nomethod_false;
    use overload nomethod => sub { $method = 'nomethod'; 0 };

    package nomethod_true;
    use overload nomethod => sub { $method= 'nomethod'; 'true' };

    package main;
    local $^W = 1;
    local $SIG{__WARN__} = sub { $warning = $_[0] };

    my $f = bless [], 'nomethod_false';
    ($warning, $method) = ("", "");
    is($f eq 'whatever', 0, 'nomethod makes eq return 0');
    is($method, 'nomethod');

    my $t = bless [], 'nomethod_true';
    ($warning, $method) = ("", "");
    is($t eq 'whatever', 'true', 'nomethod makes eq return "true"');
    is($method, 'nomethod');
    is($warning, "", 'nomethod eq need not return number');

    eval q{ 
        package nomethod_false;
        use overload cmp => sub { $method = 'cmp'; 0 };
    };
    $f = bless [], 'nomethod_false';
    ($warning, $method) = ("", "");
    ok($f eq 'whatever', 'eq falls back to cmp (nomethod not called)');
    is($method, 'cmp');

    eval q{
        package nomethod_true;
        use overload cmp => sub { $method = 'cmp'; 'true' };
    };
    $t = bless [], 'nomethod_true';
    ($warning, $method) = ("", "");
    ok($t eq 'whatever', 'eq falls back to cmp (nomethod not called)');
    is($method, 'cmp');
    like($warning, qr/isn't numeric/, 'cmp should return number');

}

{
    # nomethod called for '!' after attempted fallback
    my $nomethod_called = 0;

    package nomethod_not;
    use overload nomethod => sub { $nomethod_called = 'yes'; };

    package main;
    my $o = bless [], 'nomethod_not';
    my $res = ! $o;

    is($nomethod_called, 'yes', "nomethod() is called for '!'");
    is($res, 'yes', "nomethod(..., '!') return value propagates");
}

{
    # Subtle bug pre 5.10, as a side effect of the overloading flag being
    # stored on the reference rather than the referent. Despite the fact that
    # objects can only be accessed via references (even internally), the
    # referent actually knows that it's blessed, not the references. So taking
    # a new, unrelated, reference to it gives an object. However, the
    # overloading-or-not flag was on the reference prior to 5.10, and taking
    # a new reference didn't (use to) copy it.

    package kayo;

    use overload '""' => sub {${$_[0]}};

    sub Pie {
	return "$_[0], $_[1]";
    }

    package main;

    my $class = 'kayo';
    my $string = 'bam';
    my $crunch_eth = bless \$string, $class;

    is("$crunch_eth", $string);
    is ($crunch_eth->Pie("Meat"), "$string, Meat");

    my $wham_eth = \$string;

    is("$wham_eth", $string,
       'This reference did not have overloading in 5.8.8 and earlier');
    is ($crunch_eth->Pie("Apple"), "$string, Apple");

    my $class = ref $wham_eth;
    $class =~ s/=.*//;

    # Bless it back into its own class!
    bless $wham_eth, $class;

    is("$wham_eth", $string);
    is ($crunch_eth->Pie("Blackbird"), "$string, Blackbird");
}

{
    package numify_int;
    use overload "0+" => sub { $_[0][0] += 1; 42 };
    package numify_self;
    use overload "0+" => sub { $_[0][0]++; $_[0] };
    package numify_other;
    use overload "0+" => sub { $_[0][0]++; $_[0][1] = bless [], 'numify_int' };
    package numify_by_fallback;
    use overload fallback => 1;

    package main;
    my $o = bless [], 'numify_int';
    is(int($o), 42, 'numifies to integer');
    is($o->[0], 1, 'int() numifies only once');

    my $aref = [];
    my $num_val = int($aref);
    my $r = bless $aref, 'numify_self';
    is(int($r), $num_val, 'numifies to self');
    is($r->[0], 1, 'int() numifies once when returning self');

    my $s = bless [], 'numify_other';
    is(int($s), 42, 'numifies to numification of other object');
    is($s->[0], 1, 'int() numifies once when returning other object');
    is($s->[1][0], 1, 'returned object numifies too');

    my $m = bless $aref, 'numify_by_fallback';
    is(int($m), $num_val, 'numifies to usual reference value');
    is(abs($m), $num_val, 'numifies to usual reference value');
    is(-$m, -$num_val, 'numifies to usual reference value');
    is(0+$m, $num_val, 'numifies to usual reference value');
    is($m+0, $num_val, 'numifies to usual reference value');
    is($m+$m, 2*$num_val, 'numifies to usual reference value');
    is(0-$m, -$num_val, 'numifies to usual reference value');
    is(1*$m, $num_val, 'numifies to usual reference value');
    is(int($m/1), $num_val, 'numifies to usual reference value');
    is($m%100, $num_val%100, 'numifies to usual reference value');
    is($m**1, $num_val, 'numifies to usual reference value');

    is(abs($aref), $num_val, 'abs() of ref');
    is(-$aref, -$num_val, 'negative of ref');
    is(0+$aref, $num_val, 'ref addition');
    is($aref+0, $num_val, 'ref addition');
    is($aref+$aref, 2*$num_val, 'ref addition');
    is(0-$aref, -$num_val, 'subtraction of ref');
    is(1*$aref, $num_val, 'multiplicaton of ref');
    is(int($aref/1), $num_val, 'division of ref');
    is($aref%100, $num_val%100, 'modulo of ref');
    is($aref**1, $num_val, 'exponentiation of ref');
}

{
    package CopyConstructorFallback;
    use overload
        '++'        => sub { "$_[0]"; $_[0] },
        fallback    => 1;
    sub new { bless {} => shift }

    package main;

    my $o = CopyConstructorFallback->new;
    my $x = $o++; # would segfault
    my $y = ++$o;
    is($x, $o, "copy constructor falls back to assignment (postinc)");
    is($y, $o, "copy constructor falls back to assignment (preinc)");
}

# only scalar 'x' should currently overload

{
    package REPEAT;

    my ($x,$n, $nm);

    use overload
	'x'        => sub { $x++; 1 },
	'0+'       => sub { $n++; 1 },
	'nomethod' => sub { $nm++; 1 },
	'fallback' => 0,
    ;

    my $s = bless {};

    package main;

    my @a;
    my $count = 3;

    ($x,$n,$nm) = (0,0,0);
    @a = ((1,2,$s) x $count);
    is("$x-$n-$nm", "0-0-0", 'repeat 1');

    ($x,$n,$nm) = (0,0,0);
    @a = ((1,$s,3) x $count);
    is("$x-$n-$nm", "0-0-0", 'repeat 2');

    ($x,$n,$nm) = (0,0,0);
    @a = ((1,2,3) x $s);
    is("$x-$n-$nm", "0-1-0", 'repeat 3');
}



# RT #57012: magic items need to have mg_get() called before testing for
# overload. Lack of this means that overloaded values returned by eg a
# tied array didn't call overload methods.
# We test here both a tied array and scalar, since the implementation of
# tied  arrays (and hashes) is such that in rvalue context, mg_get is
# called prior to executing the op, while it isn't for a tied scalar.
# We also check that return values are correctly tainted.
# We try against two overload packages; one has all expected methods, the
# other uses only fallback methods.

{

    # @tests holds a list of test cases. Each elem is an array ref with
    # the following entries:
    #
    #  * the value that the overload method should return
    #
    #  * the expression to be evaled. %s is replaced with the
    #       variable being tested ($ta[0], $ts, or $plain)
    #
    #  * a string listing what functions we expect to be called.
    #       Each method appends its name in parentheses, so "(=)(+)" means
    #       we expect the copy constructor and then the add method to be
    #       called.
    #
    #  * like above, but what should be called for the fallback-only test
    #      (in this case, nomethod() identifies itself as "(NM:*)" where *
    #      is the op).  If this value is undef, fallback tests are skipped.
    #
    #  * An array ref of expected counts of calls to FETCH/STORE.
    #      The first three values are:
    #         1. the expected number of FETCHs for a tied array
    #         2. the expected number of FETCHs for a tied scalar
    #         3. the expected number of STOREs
    #       If there are a further three elements present, then
    #       these represent the expected counts for the fallback
    #       version of the tests. If absent, they are assumed to
    #       be the same as for the full method test
    #
    #  * Under the taint version of the tests,  whether we expect
    #       the result to be tainted (for example comparison ops
    #       like '==' don't return a tainted value, even if their
    #       args are.
    my @tests;

    my %subs;
    my $funcs;
    my $use_int;

    BEGIN {
	# A note on what methods to expect to be called, and
	# how many times FETCH/STORE is called:
	#
	# Mutating ops (+=, ++ etc) trigger a copy ('='), since
	# the code can't distinguish between something that's been copied:
	#    $a = foo->new(0); $b = $a; refcnt($$b) == 2
	# and overloaded objects stored in ties which will have extra
	# refcounts due to the tied_obj magic and entries on the tmps
	# stack when returning from FETCH etc. So we always copy.

	# This accounts for a '=', and an extra STORE.
	# We also have a FETCH returning the final value from the eval,
	# plus a FETCH in the overload subs themselves: ($_[0][0])
	# triggers one. However, tied aggregates have a mechanism to prevent
	# multiple fetches between STOREs, which means that the tied
	# hash skips doing a FETCH during '='.

	for (qw(+ - * / % ** << >> & | ^)) {
	    my $op = $_;
	    $op = '%%' if $op eq '%';
	    my $e = "%s $op= 3";
	    $subs{"$_="} = $e;
	    # ARRAY  FETCH: initial,        sub+=, eval-return,
	    # SCALAR FETCH: initial, sub=,  sub+=, eval-return,
	    # STORE:        copy, mutator
	    push @tests, [ 18, $e, "(=)($_=)", "(=)(NM:$_=)", [ 3, 4, 2 ], 1 ];

	    $subs{$_} =
		"do { my \$arg = %s; \$_[2] ? (3 $op \$arg) : (\$arg $op 3) }";
	    # ARRAY  FETCH: initial
	    # SCALAR FETCH: initial eval-return,
	    push @tests, [ 18, "%s $op 3", "($_)", "(NM:$_)", [ 1, 2, 0 ], 1 ];
	    push @tests, [ 18, "3 $op %s", "($_)", "(NM:$_)", [ 1, 2, 0 ], 1 ];
	}

	# these use string fallback rather than nomethod
	for (qw(x .)) {
	    my $op = $_;
	    my $e = "%s $op= 3";
	    $subs{"$_="} = $e;
	    # For normal case:
	    #   ARRAY  FETCH: initial,        sub+=, eval-return,
	    #   SCALAR FETCH: initial, sub=,  sub+=, eval-return,
	    #          STORE: copy, mutator
	    # for fallback, we just stringify, so eval-return and copy skipped

	    push @tests, [ 18, $e, "(=)($_=)", '("")',
			    [ 3, 4, 2,     2, 3, 1 ], 1 ];

	    $subs{$_} =
		"do { my \$arg = %s; \$_[2] ? (3 $op \$arg) : (\$arg $op 3) }";
	    # ARRAY  FETCH: initial
	    # SCALAR FETCH: initial eval-return,
	    # with fallback, we just stringify, so eval-return skipped,
	    #    but an extra FETCH happens in sub"", except for 'x',
	    #    which passes a copy of the RV to sub"", avoiding the
	    #    second FETCH

	    push @tests, [ 18, "%s $op 3", "($_)", '("")',
			    [ 1, 2, 0,     1, ($_ eq '.' ? 2 : 1), 0 ], 1 ];
	    next if $_ eq 'x'; # repeat only overloads on LHS
	    push @tests, [ 18, "3 $op %s", "($_)", '("")',
			    [ 1, 2, 0,     1, 2, 0 ], 1 ];
	}

	for (qw(++ --)) {
	    my $pre  = "$_%s";
	    my $post = "%s$_";
	    $subs{$_} = $pre;
	    push @tests,
		# ARRAY  FETCH: initial,        sub+=, eval-return,
		# SCALAR FETCH: initial, sub=,  sub+=, eval-return,
		# STORE:        copy, mutator
		[ 18, $pre, "(=)($_)(\"\")", "(=)(NM:$_)(\"\")", [ 3, 4, 2 ], 1 ],
		# ARRAY  FETCH: initial,        sub+=
		# SCALAR FETCH: initial, sub=,  sub+=
		# STORE:        copy, mutator
		[ 18, $post, "(=)($_)(\"\")", "(=)(NM:$_)(\"\")", [ 2, 3, 2 ], 1 ];
	}

	# For the non-mutator ops, we have a initial FETCH,
	# an extra FETCH within the sub itself for the scalar option,
	# and no STOREs

	for (qw(< <= >  >= == != lt le gt ge eq ne)) {
	    my $e = "%s $_ 3";
	    $subs{$_} = $e;
	    push @tests, [ 3, $e, "($_)", "(NM:$_)", [ 1, 2, 0 ], 0 ];
	}
	for (qw(<=> cmp)) {
	    my $e = "%s $_ 3";
	    $subs{$_} = $e;
	    push @tests, [ 3, $e, "($_)", "(NM:$_)", [ 1, 2, 0 ], 1 ];
	}
	for (qw(atan2)) {
	    my $e = "$_ %s, 3";
	    $subs{$_} = $e;
	    push @tests, [ 18, $e, "($_)", "(NM:$_)", [ 1, 2, 0 ], 1 ];
	}
	for (qw(cos sin exp abs log sqrt int ~)) {
	    my $e = "$_(%s)";
	    $subs{$_} = $e;
	    push @tests, [ 1.23, $e, "($_)",
		    ($_ eq 'int' ? '(0+)' : "(NM:$_)") , [ 1, 2, 0 ], 1 ];
	}
	for (qw(!)) {
	    my $e = "$_(%s)";
	    $subs{$_} = $e;
	    push @tests, [ 1.23, $e, "($_)", '(0+)', [ 1, 2, 0 ], 0 ];
	}
	for (qw(-)) {
	    my $e = "$_(%s)";
	    $subs{neg} = $e;
	    push @tests, [ 18, $e, '(neg)', '(NM:neg)', [ 1, 2, 0 ], 1 ];
	}
	my $e = '(%s) ? 1 : 0';
	$subs{bool} = $e;
	push @tests, [ 18, $e, '(bool)', '(0+)', [ 1, 2, 0 ], 0 ];

	# note: this is testing unary qr, not binary =~
	$subs{qr} = '(qr/%s/)';
	push @tests, [ "abc", '"abc" =~ (%s)', '(qr)', '("")', [ 1, 2, 0 ], 0 ];
	push @tests, [ chr 256, 'chr(256) =~ (%s)', '(qr)', '("")',
	                                                  [ 1, 2, 0 ], 0 ];

	$e = '"abc" ~~ (%s)';
	$subs{'~~'} = $e;
	push @tests, [ "abc", $e, '(~~)', '(NM:~~)', [ 1, 1, 0 ], 0 ];

	$subs{'-X'} = 'do { my $f = (%s);'
		    . '$_[1] eq "r" ? (-r ($f)) :'
		    . '$_[1] eq "e" ? (-e ($f)) :'
		    . '$_[1] eq "f" ? (-f ($f)) :'
		    . '$_[1] eq "l" ? (-l ($f)) :'
		    . '$_[1] eq "t" ? (-t ($f)) :'
		    . '$_[1] eq "T" ? (-T ($f)) : 0;}';
	# Note - we don't care what these file tests return, as
	# long as the tied and untied versions return the same value.
	# The flags below are chosen to test all uses of tryAMAGICftest_MG
	for (qw(r e f l t T)) {
	    push @tests, [ 'TEST', "-$_ (%s)", '(-X)', '("")', [ 1, 2, 0 ], 0 ];
	}

	$subs{'${}'} = '%s';
	push @tests, [ do {my $s=99; \$s}, '${%s}', '(${})', undef, [ 1, 1, 0 ], 0 ];

	# we skip testing '@{}' here because too much of this test
	# framework involves array dereferences!

	$subs{'%{}'} = '%s';
	push @tests, [ {qw(a 1 b 2 c 3)}, 'join "", sort keys %%{%s}',
			'(%{})', undef, [ 1, 1, 0 ], 0 ];

	$subs{'&{}'} = '%s';
	push @tests, [ sub {99}, 'do {&{%s} for 1,2}',
			    '(&{})(&{})', undef, [ 2, 2, 0 ], 0 ];

	our $RT57012A = 88;
	our $RT57012B;
	$subs{'*{}'} = '%s';
	push @tests, [ \*RT57012A, '*RT57012B = *{%s}; our $RT57012B',
		'(*{})', undef, [ 1, 1, 0 ], 0 ];

	my $iter_text = ("some random text\n" x 100) . $^X;
	open my $iter_fh, '<', \$iter_text
	    or die "open of \$iter_text gave ($!)\n";
	$subs{'<>'} = '<$iter_fh>';
	push @tests, [ $iter_fh, '<%s>', '(<>)', undef, [ 1, 1, 0 ], 1 ];
	push @tests, [ $iter_fh,
		      'local *CORE::GLOBAL::glob = sub {}; eval q|<%s>|',
		      '(<>)', undef, [ 1, 1, 0 ], 1 ];

	# eval should do tie, overload on its arg before checking taint */
	push @tests, [ '1;', 'eval q(eval %s); $@ =~ /Insecure/',
		'("")', '("")', [ 1, 1, 0 ], 0 ];


	for my $sub (keys %subs) {
	    no warnings 'deprecated';
	    my $term = $subs{$sub};
	    my $t = sprintf $term, '$_[0][0]';
	    my $e ="sub { \$funcs .= '($sub)'; my \$r; if (\$use_int) {"
		. "use integer; \$r = ($t) } else { \$r = ($t) } \$r }";
	    $subs{$sub} = eval $e;
	    die "Compiling sub gave error:\n<$e>\n<$@>\n" if $@;
	}
    }

    my $fetches;
    my $stores;

    package RT57012_OV;

    use overload
	%subs,
	"="   => sub { $funcs .= '(=)';  bless [ $_[0][0] ] },
	'0+'  => sub { $funcs .= '(0+)'; 0 + $_[0][0] },
	'""'  => sub { $funcs .= '("")'; "$_[0][0]"   },
	;

    package RT57012_OV_FB; # only contains fallback conversion functions

    use overload
	"="   => sub { $funcs .= '(=)';  bless [ $_[0][0] ] },
	'0+'  => sub { $funcs .= '(0+)'; 0 + $_[0][0] },
	'""'  => sub { $funcs .= '("")'; "$_[0][0]"   },
	"nomethod" => sub {
			$funcs .= "(NM:$_[3])";
			my $e = defined($_[1])
				? $_[3] eq 'atan2'
				    ? $_[2]
				       ? "atan2(\$_[1],\$_[0][0])"
				       : "atan2(\$_[0][0],\$_[1])"
				    : $_[2]
					? "\$_[1] $_[3] \$_[0][0]"
					: "\$_[0][0] $_[3] \$_[1]"
				: $_[3] eq 'neg'
				    ? "-\$_[0][0]"
				    : "$_[3](\$_[0][0])";
			my $r;
			no warnings 'deprecated';
			if ($use_int) {
			    use integer; $r = eval $e;
			}
			else {
			    $r = eval $e;
			}
			::diag("eval of nomethod <$e> gave <$@>") if $@;
			$r;
		    }

	;

    package RT57012_TIE_S;

    my $tie_val;
    sub TIESCALAR { bless [ bless [ $tie_val ], $_[1] ] }
    sub FETCH     { $fetches++; $_[0][0] }
    sub STORE     { $stores++;  $_[0][0] = $_[1] }

    package RT57012_TIE_A;

    sub TIEARRAY  { bless [] }
    sub FETCH     { $fetches++; $_[0][0] }
    sub STORE     { $stores++;  $_[0][$_[1]] = $_[2] }

    package main;

    for my $test (@tests) {
	my ($val, $sub_term, $exp_funcs, $exp_fb_funcs,
	    $exp_counts, $exp_taint) = @$test;

	my $tainted_val;
	{
	    # create tainted version of $val (unless its a ref)
	    my $t = substr($^X,0,0);
	    my $t0 = $t."0";
	    my $val1 = $val; # use a copy to avoid stringifying original
	    $tainted_val = ref($val1) ? $val :
			($val1 =~ /^[\d\.]+$/) ? $val+$t0 : $val.$t;
	}
	$tie_val = $tainted_val;

	for my $int ('', 'use integer; ') {
	    $use_int = ($int ne '');
	    my $plain = $tainted_val;
	    my $plain_term = $int . sprintf $sub_term, '$plain';
	    my $exp = do {no warnings 'deprecated'; eval $plain_term };
	    diag("eval of plain_term <$plain_term> gave <$@>") if $@;
	    SKIP: {
		is_if_taint_supported(tainted($exp), $exp_taint,
		    "<$plain_term> taint of expected return");
	    }

	    for my $ov_pkg (qw(RT57012_OV RT57012_OV_FB)) {
		next if $ov_pkg eq 'RT57012_OV_FB'
			and  not defined $exp_fb_funcs;
		my ($exp_fetch_a, $exp_fetch_s, $exp_store) =
		    ($ov_pkg eq 'RT57012_OV' || @$exp_counts < 4)
			? @$exp_counts[0,1,2]
			: @$exp_counts[3,4,5];

		tie my $ts, 'RT57012_TIE_S', $ov_pkg;
		tie my @ta, 'RT57012_TIE_A';
		$ta[0]    = bless [ $tainted_val ], $ov_pkg;
		my $oload = bless [ $tainted_val ], $ov_pkg;

		for my $var ('$ta[0]', '$ts', '$oload',
			    ($sub_term eq '<%s>' ? '${ts}' : ())
		) {

		    $funcs = '';
		    $fetches = 0;
		    $stores = 0;

		    my $res_term  = $int . sprintf $sub_term, $var;
		    my $desc =  "<$res_term> $ov_pkg" ;
		    my $res = do { no warnings 'deprecated'; eval $res_term };
		    diag("eval of res_term $desc gave <$@>") if $@;
		    # uniquely, the inc/dec ops return the original
		    # ref rather than a copy, so stringify it to
		    # find out if its tainted
		    $res = "$res" if $res_term =~ /\+\+|--/;
		    SKIP: {
			is_if_taint_supported(tainted($res), $exp_taint,
			    "$desc taint of result return");
		    }
		    is($res, $exp, "$desc return value");
		    my $fns =($ov_pkg eq 'RT57012_OV_FB')
				? $exp_fb_funcs : $exp_funcs;
		    if ($var eq '$oload' && $res_term !~ /oload(\+\+|--)/) {
			# non-tied overloading doesn't trigger a copy
			# except for post inc/dec
			$fns =~ s/^\(=\)//;
		    }
		    is($funcs, $fns, "$desc methods called");
		    next if $var eq '$oload';
		    my $exp_fetch = ($var eq '$ts') ?
			    $exp_fetch_s : $exp_fetch_a;
		    SKIP: {
			if ($skip_fetch_count_when_no_taint{$desc} && $no_taint_support) {
			    skip("your perl was built without taint support");
			}
			else {
			    is($fetches, $exp_fetch, "$desc FETCH count");
			}
		    }
		    is($stores, $exp_store, "$desc STORE count");

		}

	    }
	}
    }
}

# Test overload from the main package
fresh_perl_is
 '$^W = 1; use overload q\""\ => sub {"ning"}; print bless []',
 'ning',
  { switches => ['-wl'], stderr => 1 },
 'use overload from the main package'
;

{
    package blessed_methods;
    use overload '+' => sub {};
    bless overload::Method __PACKAGE__,'+';
    eval { overload::Method __PACKAGE__,'+' };
    ::is($@, '', 'overload::Method and blessed overload methods');
}

{
    # fallback to 'cmp' and '<=>' with heterogeneous operands
    # [perl #71286]
    my $not_found = 'no method found';
    my $used = 0;
    package CmpBase;
    sub new {
        my $n = $_[1] || 0;
        bless \$n, ref $_[0] || $_[0];
    }
    sub cmp {
        $used = \$_[0];
        (${$_[0]} <=> ${$_[1]}) * ($_[2] ? -1 : 1);
    }

    package NCmp;
    use parent '-norequire', 'CmpBase';
    use overload '<=>' => 'cmp';

    package SCmp;
    use parent '-norequire', 'CmpBase';
    use overload 'cmp' => 'cmp';

    package main;
    my $n = NCmp->new(5);
    my $s = SCmp->new(3);
    my $res;

    eval { $res = $n > $s; };
    $res = $not_found if $@ =~ /$not_found/;
    is($res, 1, 'A>B using A<=> when B overloaded, no B<=>');

    eval { $res = $s < $n; };
    $res = $not_found if $@ =~ /$not_found/;
    is($res, 1, 'A<B using B<=> when A overloaded, no A<=>');

    eval { $res = $s lt $n; };
    $res = $not_found if $@ =~ /$not_found/;
    is($res, 1, 'A lt B using A:cmp when B overloaded, no B:cmp');

    eval { $res = $n gt $s; };
    $res = $not_found if $@ =~ /$not_found/;
    is($res, 1, 'A gt B using B:cmp when A overloaded, no A:cmp');

    my $o = NCmp->new(9);
    $res = $n < $o;
    is($used, \$n, 'A < B uses <=> from A in preference to B');

    my $t = SCmp->new(7);
    $res = $s lt $t;
    is($used, \$s, 'A lt B uses cmp from A in preference to B');
}

{
    # Combinatorial testing of 'fallback' and 'nomethod'
    # [perl #71286]
    package NuMB;
    use overload '0+' => sub { ${$_[0]}; },
        '""' => 'str';
    sub new {
        my $self = shift;
        my $n = @_ ? shift : 0;
        bless my $obj = \$n, ref $self || $self;
    }
    sub str {
        no strict qw/refs/;
        my $s = "(${$_[0]} ";
        $s .= "nomethod, " if defined ${ref($_[0]).'::(nomethod'};
        my $fb = ${ref($_[0]).'::()'};
        $s .= "fb=" . (defined $fb ? 0 + $fb : 'undef') . ")";
    }
    sub nomethod { "${$_[0]}.nomethod"; }

    # create classes for tests
    package main;
    my @falls = (0, 'undef', 1);
    my @nomethods = ('', 'nomethod');
    my $not_found = 'no method found';
    for my $fall (@falls) {
        for my $nomethod (@nomethods) {
            my $nomethod_decl = $nomethod
                ? $nomethod . "=>'nomethod'," : '';
            eval qq{
                    package NuMB$fall$nomethod;
                    use parent '-norequire', qw/NuMB/;
                    use overload $nomethod_decl
                    fallback => $fall;
                };
        }
    }

    # operation and precedence of 'fallback' and 'nomethod'
    # for all combinations with 2 overloaded operands
    for my $nomethod2 (@nomethods) {
        for my $nomethod1 (@nomethods) {
            for my $fall2 (@falls) {
                my $pack2 = "NuMB$fall2$nomethod2";
                for my $fall1 (@falls) {
                    my $pack1 = "NuMB$fall1$nomethod1";
                    my ($test, $out, $exp);
                    eval qq{
                            my \$x = $pack1->new(2);
                            my \$y = $pack2->new(3);
                            \$test = "\$x" . ' * ' . "\$y";
                            \$out = \$x * \$y;
                        };
                    $out = $not_found if $@ =~ /$not_found/;
                    $exp = $nomethod1 ? '2.nomethod' :
                         $nomethod2 ? '3.nomethod' :
                         $fall1 eq '1' && $fall2 eq '1' ? 6
                         : $not_found;
                    is($out, $exp, "$test --> $exp");
                }
            }
        }
    }

    # operation of 'fallback' and 'nomethod'
    # where the other operand is not overloaded
    for my $nomethod (@nomethods) {
        for my $fall (@falls) {
            my ($test, $out, $exp);
            eval qq{
                    my \$x = NuMB$fall$nomethod->new(2);
                    \$test = "\$x" . ' * 3';
                    \$out = \$x * 3;
                };
            $out = $not_found if $@ =~ /$not_found/;
            $exp = $nomethod ? '2.nomethod' :
                $fall eq '1' ? 6
                : $not_found;
            is($out, $exp, "$test --> $exp");

            eval qq{
                    my \$x = NuMB$fall$nomethod->new(2);
                    \$test = '3 * ' . "\$x";
                    \$out = 3 * \$x;
                };
            $out = $not_found if $@ =~ /$not_found/;
            is($out, $exp, "$test --> $exp");
        }
    }
}

# since 5.6 overloaded <> was leaving an extra arg on the stack!

{
    package Iter1;
    use overload '<>' => sub { 11 };
    package main;
    my $a = bless [], 'Iter1';
    my $x;
    my @a = (10, ($x = <$a>), 12);
    is ($a[0], 10, 'Iter1: a[0]');
    is ($a[1], 11, 'Iter1: a[1]');
    is ($a[2], 12, 'Iter1: a[2]');
    @a = (10, ($x .= <$a>), 12);
    is ($a[0],   10, 'Iter1: a[0] concat');
    is ($a[1], 1111, 'Iter1: a[1] concat');
    is ($a[2],   12, 'Iter1: a[2] concat');
}

# Some tests for error messages
{
    package Justus;
    use overload '+' => 'justice';
    eval {"".bless[]};
    ::like $@, qr/^Can't resolve method "justice" overloading "\+" in p(?x:
                  )ackage "Justus" at /,
      'Error message when explicitly named overload method does not exist';

    package JustUs;
    our @ISA = 'JustYou';
    package JustYou { use overload '+' => 'injustice'; }
    "JustUs"->${\"(+"};
    eval {"".bless []};
    ::like $@, qr/^Stub found while resolving method "\?{3}" overloadin(?x:
                  )g "\+" in package "JustUs" at /,
      'Error message when sub stub is encountered';
}

{
    # check that the right number of stringifications
    # and the correct un-utf8-ifying happen on regex compile
    package utf8_match;
    my $c;
    use overload '""' => sub { $c++; $_[0][0] ? "^\x{100}\$" : "^A\$"; };
    my $o = bless [0], 'utf8_match';

    $o->[0] = 0;
    $c = 0;
    ::ok("A" =~  "^A\$",	"regex stringify utf8=0 ol=0 bytes=0");
    ::ok("A" =~ $o,		"regex stringify utf8=0 ol=1 bytes=0");
    ::is($c, 1,			"regex stringify utf8=0 ol=1 bytes=0 count");

    $o->[0] = 1;
    $c = 0;
    ::ok("\x{100}" =~ "^\x{100}\$",
				"regex stringify utf8=1 ol=0 bytes=0");
    ::ok("\x{100}" =~ $o,	"regex stringify utf8=1 ol=1 bytes=0");
    ::is($c, 1,			"regex stringify utf8=1 ol=1 bytes=0 count");

    use bytes;

    $o->[0] = 0;
    $c = 0;
    ::ok("A" =~  "^A\$",	"regex stringify utf8=0 ol=0 bytes=1");
    ::ok("A" =~ $o,		"regex stringify utf8=0 ol=1 bytes=1");
    ::is($c, 1,			"regex stringify utf8=0 ol=1 bytes=1 count");

    $o->[0] = 1;
    $c = 0;
    ::ok(main::byte_utf8a_to_utf8n("\xc4\x80") =~ "^\x{100}\$",
				"regex stringify utf8=1 ol=0 bytes=1");
    ::ok(main::byte_utf8a_to_utf8n("\xc4\x80") =~ $o,	"regex stringify utf8=1 ol=1 bytes=1");
    ::is($c, 1,			"regex stringify utf8=1 ol=1 bytes=1 count");


}

# [perl #40333]
# overload::Overloaded should not use a ->can designed for autoloading.
# This example attempts to be as realistic as possible.  The o class has a
# default singleton object, but can have instances, too.  The proxy class
# represents proxies for o objects, but class methods delegate to the
# singleton.
# overload::Overloaded used to return incorrect results for proxy objects.
package proxy {
    sub new { bless [$_[1]], $_[0] }
    sub AUTOLOAD {
       our $AUTOLOAD =~ s/.*:://;
       &_self->$AUTOLOAD;
    }
    sub can      { SUPER::can{@_} || &_self->can($_[1]) }
    sub _self { ref $_[0] ? $_[0][0] : $o::singleton }
}
package o     { use overload '""' => sub { 'keck' };
                sub new { bless[], $_[0] }
                our $singleton = o->new; }
ok !overload::Overloaded(new proxy new o),
 'overload::Overloaded does not incorrectly return true for proxy classes';

# Another test, based on the type of explosive test class for which
# perl #40333 was filed.
{
    package broken_can;
    sub can {}
    use overload '""' => sub {"Ahoy!"};

    package main;
    my $obj = bless [], 'broken_can';
    ok(overload::Overloaded($obj));
}

sub eleventative::cos { 'eleven' }
sub twelvetative::abs { 'twelve' }
sub thirteentative::abs { 'thirteen' }
sub fourteentative::abs { 'fourteen' }
@eleventative::ISA = twelvetative::;
{
    my $o = bless [], 'eleventative';
    eval 'package eleventative; use overload map +($_)x2, cos=>abs=>';
    is cos $o, 'eleven', 'overloading applies to object blessed before';
    bless [], 'eleventative';
    is cos $o, 'eleven',
      'ovrld applies to previously-blessed obj after other obj is blessed';
    $o = bless [], 'eleventative';
    *eleventative::cos = sub { 'ten' };
    is cos $o, 'ten', 'method changes affect overloading';
    @eleventative::ISA = thirteentative::;
    is abs $o, 'thirteen', 'isa changes affect overloading';
    bless $o, 'fourteentative';
    @fourteentative::ISA = 'eleventative';
    is abs $o, 'fourteen', 'isa changes can turn overloading on';
}

# no overload "fallback";
{ package phake;
  use overload fallback => 1, '""' => sub { 'arakas' };
  no overload 'fallback';
}
$a = bless [], 'phake';
is "$a", "arakas",
    'no overload "fallback" does not stop overload from working';
ok !eval { () = $a eq 'mpizeli'; 1 },
    'no overload "fallback" resets fallback to undef on overloaded class';
{ package ent; use overload fallback => 0, abs => sub{};
  our@ISA = 'huorn';
  package huorn;
  use overload fallback => 1;
  package ent;
  no overload "fallback"; # disable previous declaration
}
$a = bless [], ent::;
is eval {"$a"}, overload::StrVal($a),
    'no overload undoes fallback declaration completetly'
 or diag $@;

# inherited fallback
{
 package pervyy;
 our @ISA = 'vtoryy';
 use overload "abs" =>=> sub {};
 package vtoryy;
 use overload fallback => 1, 'sin' =>=> sub{}
}
$a = bless [], pervyy::;
is eval {"$a"}, overload::StrVal($a),
 'fallback is inherited by classes that have their own overloading'
 or diag $@;

# package separators in method names
{
 package mane;
 use overload q\""\ => "bear::strength";
 use overload bool  => "bear'bouillon";
}
@bear::ISA = 'food';
sub food::strength { 'twine' }
sub food::bouillon { 0 }
$a = bless[], mane::;
is eval { "$a" }, 'twine', ':: in method name' or diag $@;
is eval { !$a  },   1,      "' in method name" or diag $@;

# [perl #113050] Half of CPAN assumes fallback is under "()"
{
  package dodo;
  use overload '+' => sub {};
  no strict;
  *{"dodo::()"} = sub{};
  ${"dodo::()"} = 1;
}
$a = bless [],'dodo';
is eval {"$a"}, overload::StrVal($a), 'fallback is stored under "()"';

# [perl #47119]
{
    my $context;

    {
        package Splitter;
        use overload '<>' => \&chars;

        sub new {
            my $class = shift;
            my ($string) = @_;
            bless \$string, $class;
        }

        sub chars {
            my $self = shift;
            my @chars = split //, $$self;
            $context = wantarray;
            return @chars;
        }
    }

    my $obj = Splitter->new('bar');

    $context = 42; # not 1, '', or undef

    my @foo = <$obj>;
    is($context, 1, "list context (readline list)");
    is(scalar(@foo), 3, "correct result (readline list)");
    is($foo[0], 'b', "correct result (readline list)");
    is($foo[1], 'a', "correct result (readline list)");
    is($foo[2], 'r', "correct result (readline list)");

    $context = 42;

    my $foo = <$obj>;
    ok(defined($context), "scalar context (readline scalar)");
    is($context, '', "scalar context (readline scalar)");
    is($foo, 3, "correct result (readline scalar)");

    $context = 42;

    <$obj>;
    ok(!defined($context), "void context (readline void)");

    $context = 42;

    my @bar = <${obj}>;
    is($context, 1, "list context (glob list)");
    is(scalar(@bar), 3, "correct result (glob list)");
    is($bar[0], 'b', "correct result (glob list)");
    is($bar[1], 'a', "correct result (glob list)");
    is($bar[2], 'r', "correct result (glob list)");

    $context = 42;

    my $bar = <${obj}>;
    ok(defined($context), "scalar context (glob scalar)");
    is($context, '', "scalar context (glob scalar)");
    is($bar, 3, "correct result (glob scalar)");

    $context = 42;

    <${obj}>;
    ok(!defined($context), "void context (glob void)");
}
{
    my $context;

    {
        package StringWithContext;
        use overload '""' => \&stringify;

        sub new {
            my $class = shift;
            my ($string) = @_;
            bless \$string, $class;
        }

        sub stringify {
            my $self = shift;
            $context = wantarray;
            return $$self;
        }
    }

    my $obj = StringWithContext->new('bar');

    $context = 42;

    my @foo = "".$obj;
    ok(defined($context), "scalar context (stringify list)");
    is($context, '', "scalar context (stringify list)");
    is(scalar(@foo), 1, "correct result (stringify list)");
    is($foo[0], 'bar', "correct result (stringify list)");

    $context = 42;

    my $foo = "".$obj;
    ok(defined($context), "scalar context (stringify scalar)");
    is($context, '', "scalar context (stringify scalar)");
    is($foo, 'bar', "correct result (stringify scalar)");

    $context = 42;

    "".$obj;

    is($context, '', "scalar context (stringify void)");
}
{
    my ($context, $swap);

    {
        package AddWithContext;
        use overload '+' => \&add;

        sub new {
            my $class = shift;
            my ($num) = @_;
            bless \$num, $class;
        }

        sub add {
            my $self = shift;
            my ($other, $swapped) = @_;
            $context = wantarray;
            $swap = $swapped;
            return ref($self)->new($$self + $other);
        }

        sub val { ${ $_[0] } }
    }

    my $obj = AddWithContext->new(6);

    $context = $swap = 42;

    my @foo = $obj + 7;
    ok(defined($context), "scalar context (add list)");
    is($context, '', "scalar context (add list)");
    ok(defined($swap), "not swapped (add list)");
    is($swap, '', "not swapped (add list)");
    is(scalar(@foo), 1, "correct result (add list)");
    is($foo[0]->val, 13, "correct result (add list)");

    $context = $swap = 42;

    @foo = 7 + $obj;
    ok(defined($context), "scalar context (add list swap)");
    is($context, '', "scalar context (add list swap)");
    ok(defined($swap), "swapped (add list swap)");
    is($swap, 1, "swapped (add list swap)");
    is(scalar(@foo), 1, "correct result (add list swap)");
    is($foo[0]->val, 13, "correct result (add list swap)");

    $context = $swap = 42;

    my $foo = $obj + 7;
    ok(defined($context), "scalar context (add scalar)");
    is($context, '', "scalar context (add scalar)");
    ok(defined($swap), "not swapped (add scalar)");
    is($swap, '', "not swapped (add scalar)");
    is($foo->val, 13, "correct result (add scalar)");

    $context = $swap = 42;

    my $foo = 7 + $obj;
    ok(defined($context), "scalar context (add scalar swap)");
    is($context, '', "scalar context (add scalar swap)");
    ok(defined($swap), "swapped (add scalar swap)");
    is($swap, 1, "swapped (add scalar swap)");
    is($foo->val, 13, "correct result (add scalar swap)");

    $context = $swap = 42;

    $obj + 7;

    ok(!defined($context), "void context (add void)");
    ok(defined($swap), "not swapped (add void)");
    is($swap, '', "not swapped (add void)");

    $context = $swap = 42;

    7 + $obj;

    ok(!defined($context), "void context (add void swap)");
    ok(defined($swap), "swapped (add void swap)");
    is($swap, 1, "swapped (add void swap)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    my @foo = $obj += 7;
    ok(defined($context), "scalar context (add assign list)");
    is($context, '', "scalar context (add assign list)");
    ok(!defined($swap), "not swapped and autogenerated (add assign list)");
    is(scalar(@foo), 1, "correct result (add assign list)");
    is($foo[0]->val, 13, "correct result (add assign list)");
    is($obj->val, 13, "correct result (add assign list)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    my $foo = $obj += 7;
    ok(defined($context), "scalar context (add assign scalar)");
    is($context, '', "scalar context (add assign scalar)");
    ok(!defined($swap), "not swapped and autogenerated (add assign scalar)");
    is($foo->val, 13, "correct result (add assign scalar)");
    is($obj->val, 13, "correct result (add assign scalar)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    $obj += 7;

    ok(defined($context), "scalar context (add assign void)");
    is($context, '', "scalar context (add assign void)");
    ok(!defined($swap), "not swapped and autogenerated (add assign void)");
    is($obj->val, 13, "correct result (add assign void)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    my @foo = ++$obj;
    ok(defined($context), "scalar context (add incr list)");
    is($context, '', "scalar context (add incr list)");
    ok(!defined($swap), "not swapped and autogenerated (add incr list)");
    is(scalar(@foo), 1, "correct result (add incr list)");
    is($foo[0]->val, 7, "correct result (add incr list)");
    is($obj->val, 7, "correct result (add incr list)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    my $foo = ++$obj;
    ok(defined($context), "scalar context (add incr scalar)");
    is($context, '', "scalar context (add incr scalar)");
    ok(!defined($swap), "not swapped and autogenerated (add incr scalar)");
    is($foo->val, 7, "correct result (add incr scalar)");
    is($obj->val, 7, "correct result (add incr scalar)");

    $obj = AddWithContext->new(6);

    $context = $swap = 42;

    ++$obj;

    ok(defined($context), "scalar context (add incr void)");
    is($context, '', "scalar context (add incr void)");
    ok(!defined($swap), "not swapped and autogenerated (add incr void)");
    is($obj->val, 7, "correct result (add incr void)");
}

# [perl #113010]
{
    {
        package OnlyFallback;
        use overload fallback => 0;
    }
    {
        my $obj = bless {}, 'OnlyFallback';
        my $died = !eval { "".$obj; 1 };
        my $err = $@;
        ok($died, "fallback of 0 causes error");
        like($err, qr/"\.": no method found/, "correct error");
    }

    {
        package OnlyFallbackUndef;
        use overload fallback => undef;
    }
    {
        my $obj = bless {}, 'OnlyFallbackUndef';
        my $died = !eval { "".$obj; 1 };
        my $err = $@;
        ok($died, "fallback of undef causes error");
        # this one tries falling back to stringify before dying
        like($err, qr/"""": no method found/, "correct error");
    }

    {
        package OnlyFallbackTrue;
        use overload fallback => 1;
    }
    {
        my $obj = bless {}, 'OnlyFallbackTrue';
        my $val;
        my $died = !eval { $val = "".$obj; 1 };
        my $err = $@;
        ok(!$died, "fallback of 1 doesn't cause error")
            || diag("got error of $err");
        like($val, qr/^OnlyFallbackTrue=HASH\(/, "stringified correctly");
    }
}

{
    # Making Regexp class overloaded: avoid infinite recursion.
    # Do this in a separate process since it, well, overloads Regexp!
    fresh_perl_is(
	<<'EOF',
package Regexp;
use overload q{""} => sub {$_[0] };
package main;
my $r1 = qr/1/;
my $r2 = qr/ABC$r1/;
print $r2,"\n";
EOF
	'(?^:ABC(?^:1))',
	{ stderr => 1 },
	'overloaded REGEXP'
    );
}

{
    # RT #121362
    # splitting the stash HV while rebuilding the overload cache gave
    # valgrind errors. This test code triggers such a split. It doesn't
    # actually test anything; its just there for valgrind to spot
    # problems.

    package A_121362;

    sub stringify { }
    use overload '""' => 'stringify';

    package B_121362;
    our @ISA = qw(A_121362);

    package main;

    my $x = bless { }, 'B_121362';

    for ('a'..'z') {
        delete $B_121362::{stringify}; # delete cache entry
        no strict 'refs';
        *{"B_121362::$_"}  = sub { };  # increase size of %B_121362
        my $y = $x->{value};       # trigger cache add to %B_121362
    }
    pass("RT 121362");
}

package refsgalore {
    use overload
	'${}' => sub { \42  },
	'@{}' => sub { [43] },
	'%{}' => sub { { 44 => 45 } },
	'&{}' => sub { sub { 46 } };
}
{
    use feature 'postderef';
    tell myio; # vivifies *myio{IO} at compile time
    use constant ioref => bless *myio{IO}, refsgalore::;
    is ioref->$*, 42, '(overloaded constant that is not a scalar ref)->$*';
    is ioref->[0], 43, '(ovrld constant that is not an array ref)->[0]';
    is ioref->{44}, 45, "(ovrld const that is not a hash ref)->{key}";
    is ioref->(), 46, '(overloaded constant that is not a sub ref)->()';
}

package xstack { use overload 'x' => sub { shift . " x " . shift },
                              '""'=> sub { "xstack" } }
is join(",", 1..3, scalar((bless([], 'xstack')) x 3, 1), 4..6),
  "1,2,3,1,4,5,6",
  '(...)x... in void cx with x overloaded [perl #121827]';

package bitops {
    our @o;
    use overload do {
	my %o;
	for my $o (qw(& | ^ ~ &. |. ^. ~. &= |= ^= &.= |.= ^.=)) {
	    $o{$o} = sub {
		::ok !defined $_[3], "undef (or nonexistent) arg 3 for $o";
		push @o, $o, scalar @_, $_[4]//'u';
		$_[0]
	    }
	}
	%o, '=' => sub { bless [] };
    }
}
{
    use experimental 'bitwise';
    my $o = bless [], bitops::;
    $_ = $o & 0;
    $_ = $o | 0;
    $_ = $o ^ 0;
    $_ = ~$o;
    $_ = $o &. 0;
    $_ = $o |. 0;
    $_ = $o ^. 0;
    $_ = ~.$o;
    $o &= 0;
    $o |= 0;
    $o ^= 0;
    $o &.= 0;
    $o |.= 0;
    $o ^.= 0;
    # elems are in triplets: op, length of @_, numeric? (1/u for y/n)
    is "@bitops::o", '& 5 1 | 5 1 ^ 5 1 ~ 5 1 &. 3 u |. 3 u ^. 3 u ~. 3 u ' 		   . '&= 5 1 |= 5 1 ^= 5 1 &.= 3 u |.= 3 u ^.= 3 u',
       'experimental "bitwise" ops'
}
package bitops2 {
    our @o;
    use overload
	 nomethod => sub { push @o, $_[3], scalar @_, $_[4]//'u'; $_[0] },
	'=' => sub { bless [] };
}
{
    use experimental 'bitwise';
    my $o = bless [], bitops2::;
    $_ = $o & 0;
    $_ = $o | 0;
    $_ = $o ^ 0;
    $_ = ~$o;
    $_ = $o &. 0;
    $_ = $o |. 0;
    $_ = $o ^. 0;
    $_ = ~.$o;
    $o &= 0;
    $o |= 0;
    $o ^= 0;
    $o &.= 0;
    $o |.= 0;
    $o ^.= 0;
    # elems are in triplets: op, length of @_, numeric? (1/u for y/n)
    is "@bitops2::o", '& 5 1 | 5 1 ^ 5 1 ~ 5 1 &. 4 u |. 4 u ^. 4 u ~. 4 u ' 		    . '&= 5 1 |= 5 1 ^= 5 1 &.= 4 u |.= 4 u ^.= 4 u',
       'experimental "bitwise" ops with nomethod'
}

package length_utf8 {
    use overload '""' => sub { "\x{100}" };
    my $o = bless [];
print length $o, "\n";

    ::is length($o), 1, "overloaded utf8 length";
    ::is "$o", "\x{100}", "overloaded utf8 value";
}


{ # undefining the overload stash -- KEEP THIS TEST LAST
    package ant;
    use overload '+' => 'onion';
    $_ = \&overload::nil;
    undef %overload::;
    ()=0+bless[];
    ::ok(1, 'no crash when undefining %overload::');
}


# test various aspects of string concat overloading, especially where
# multiple concats etc are optimised into a single multiconcat op

package Concat {

    my $id;

    # append a brief description of @_ to $id
    sub id {
        my @a = map ref $_      ?  "[" . $_->[0] . "]" :
                    !defined $_ ? "u"                  :
                    $_,
                @_;
        $id .= '(' . join (',', @a) . ')';
    }

    use overload
        '.'  => sub {
                    id('.', @_);
                    my ($l, $r, $rev) = @_;
                    ($l, $r) = map ref $_ ? $_->[0] : $_, $l, $r;
                    ($l,$r) = ($r, $l) if $rev;
                    bless [ $l . $r ];
                },

        '.=' => sub {
                    id('.=', @_);
                    my ($l, $r, $rev) = @_;
                    my ($ll, $rr) = map ref $_ ? $_->[0] : $_, $l, $r;
                    die "Unexpected reverse in .=" if $rev;
                    $l->[0] .= ref $r ? $r->[0] : $r;
                    $l;
                },

        '=' => sub {
                    id('=', @_);
                    bless [ $_[0][0] ];
                },

        '""' => sub {
                    id('""', @_);
                    $_[0][0];
                },
    ;

    my $a = 'a';
    my $b = 'b';
    my $c = 'c';
    my $A = bless [ 'A' ];
    my $B = bless [ 'B' ];
    my $C = bless [ 'C' ];

    my ($r, $R);


    # like cc, but with $is_ref set to 1
    sub c {
        my ($expr, $expect, $exp_id) = @_;
        cc($expr, $expect, 1, $exp_id);
    }

    # eval $expr, and see if it returns $expect, and whether
    # the returned value is a ref ($is_ref). Finally, check that
    # $id, which has accumulated info from all overload method calls,
    # matches $exp_id.

    sub cc {
        my ($expr, $expect, $is_ref, $exp_id) = @_;

        $id = '';
        $r = 'r';
        $R = bless ['R'];

        my $got = eval $expr;
        die "eval failed: $@" if $@;
        ::is "$got", $expect,   "expect: $expr";
        ::is $id, $exp_id,      "id:     $expr";
        ::is ref($got), ($is_ref ? 'Concat' : ''), "is_ref: $expr";
    }

    # single concats

    c '$r=$A.$b',       'Ab',   '(.,[A],b,)("",[Ab],u,)';
    c '$r=$a.$B',       'aB',   '(.,[B],a,1)("",[aB],u,)';
    c '$r=$A.$B',       'AB',   '(.,[A],[B],)("",[AB],u,)';
    c '$R.=$a',         'Ra',   '(.=,[R],a,u)("",[Ra],u,)';
    c '$R.=$A',         'RA',   '(.=,[R],[A],u)("",[RA],u,)';

   # two concats

    c '$r=$A.$b.$c',    'Abc',   '(.,[A],b,)(.=,[Ab],c,u)("",[Abc],u,)';
    c '$r=$A.($b.$c)',  'Abc',   '(.,[A],bc,)("",[Abc],u,)';
    c '$r=$a.$B.$c',    'aBc',   '(.,[B],a,1)(.=,[aB],c,u)("",[aBc],u,)';
    c '$r=$a.($B.$c)',  'aBc',   '(.,[B],c,)(.,[Bc],a,1)("",[aBc],u,)';
    c '$r=$a.$b.$C',    'abC',   '(.,[C],ab,1)("",[abC],u,)';
    c '$r=$a.($b.$C)',  'abC',   '(.,[C],b,1)(.,[bC],a,1)("",[abC],u,)';

   # two concats plus mutator

    c '$r.=$A.$b.$c',   'rAbc',  '(.,[A],b,)(.=,[Ab],c,u)(.,[Abc],r,1)'
                                .'("",[rAbc],u,)';
    c '$r.=$A.($b.$c)', 'rAbc',  '(.,[A],bc,)(.,[Abc],r,1)("",[rAbc],u,)';
    c '$r.=$a.$B.$c',   'raBc',  '(.,[B],a,1)(.=,[aB],c,u)(.,[aBc],r,1)'
                                .'("",[raBc],u,)';
    c '$r.=$a.($B.$c)', 'raBc',  '(.,[B],c,)(.,[Bc],a,1)(.,[aBc],r,1)'
                                .'("",[raBc],u,)';
    c '$r.=$a.$b.$C',   'rabC',  '(.,[C],ab,1)(.,[abC],r,1)("",[rabC],u,)';
    c '$r.=$a.($b.$C)', 'rabC',  '(.,[C],b,1)(.,[bC],a,1)(.,[abC],r,1)'
                                .'("",[rabC],u,)';

    c '$R.=$A.$b.$c',   'RAbc',  '(.,[A],b,)(.=,[Ab],c,u)(.=,[R],[Abc],u)'
                                .'("",[RAbc],u,)';
    c '$R.=$A.($b.$c)', 'RAbc',  '(.,[A],bc,)(.=,[R],[Abc],u)("",[RAbc],u,)';
    c '$R.=$a.$B.$c',   'RaBc',  '(.,[B],a,1)(.=,[aB],c,u)(.=,[R],[aBc],u)'
                                .'("",[RaBc],u,)';
    c '$R.=$a.($B.$c)', 'RaBc',  '(.,[B],c,)(.,[Bc],a,1)(.=,[R],[aBc],u)'
                                .'("",[RaBc],u,)';
    c '$R.=$a.$b.$C',   'RabC',  '(.,[C],ab,1)(.=,[R],[abC],u)("",[RabC],u,)';
    c '$R.=$a.($b.$C)', 'RabC',  '(.,[C],b,1)(.,[bC],a,1)(.=,[R],[abC],u)'
                                .'("",[RabC],u,)';

    # concat over assign

    c '($R.=$a).$B.$c', 'RaBc',  '(.=,[R],a,u)(.,[Ra],[B],)(.=,[RaB],c,u)'
                                  .'("",[RaBc],u,)';
    ::is "$R", "Ra", 'R in concat over assign';


    # nested mutators

    c '(($R.=$a).=$b).=$c', 'Rabc',  '(.=,[R],a,u)(=,[Ra],u,)(.=,[Ra],b,u)'
                                   . '(=,[Rab],u,)(.=,[Rab],c,u)("",[Rabc],u,)';
    c '(($R.=$a).=$B).=$c', 'RaBc',  '(.=,[R],a,u)(=,[Ra],u,)(.=,[Ra],[B],u)'
                                   . '(=,[RaB],u,)(.=,[RaB],c,u)("",[RaBc],u,)';

    # plain SV on both LHS and RHS with RHS object

    c '$r=$r.$A.$r',   'rAr',  '(.,[A],r,1)(.=,[rA],r,u)("",[rAr],u,)';
    c '$r.=$r.$A.$r',  'rrAr', '(.,[A],r,1)(.=,[rA],r,u)(.,[rAr],r,1)'
                              .'("",[rrAr],u,)';

    # object on both LHS and RHS

    c '$R.=$R',        'RR',    '(.=,[R],[R],u)("",[RR],u,)';
    c '$R.=$R.$b.$c',  'RRbc',  '(.,[R],b,)(.=,[Rb],c,u)(.=,[R],[Rbc],u)'
                               .'("",[RRbc],u,)';
    c '$R.=$a.$R.$c',  'RaRc',  '(.,[R],a,1)(.=,[aR],c,u)(.=,[R],[aRc],u)'
                               .'("",[RaRc],u,)'; 
    c '$R.=$a.$b.$R',  'RabR',  '(.,[R],ab,1)(.=,[R],[abR],u)("",[RabR],u,)';


    # sprintf shouldn't do concat overloading

    cc '$r=sprintf("%s%s%s",$a,$B,$c)',  'aBc',  0, '("",[B],u,)';
    cc '$R=sprintf("%s%s%s",$a,$B,$c)',  'aBc',  0, '("",[B],u,)';
    cc '$r.=sprintf("%s%s%s",$a,$B,$c)', 'raBc', 0, '("",[B],u,)';
    cc '$R.=sprintf("%s%s%s",$a,$B,$c)', 'RaBc', 1, '("",[B],u,)(.=,[R],aBc,u)'
                                                   .'("",[RaBc],u,)';

    # multiple constants should individually overload (RT #132385)

    c '$r=$A."b"."c"', 'Abc',  '(.,[A],b,)(.=,[Ab],c,u)("",[Abc],u,)';

    # ... except for this
    c '$R.="a"."b"',   'Rab',  '(.=,[R],ab,u)("",[Rab],u,)';
}

# RT #132385
# The first arg of a reversed concat shouldn't be stringified:
#      $left . $right
#  where $right is overloaded, should invoke
#      concat($right, $left, 1)
#  rather than
#      concat($right, "$left", 1)
# There's a similar issue with
#      $left .= $right
# when left is overloaded

package RT132385 {

    use constant C => [ "constref" ];

    use overload '.' => sub {
                            my ($l, $r, $rev) = @_;
                            ($l,$r) = ($r,$l) if $rev;
                            $l = ref $l ? $l->[0] : "$l";
                            $r = ref $r ? $r->[0] : "$r";
                            "$l-$r";
                        }
    ;

    my $r1 = [ "ref1" ];
    my $r2 = [ "ref2" ];
    my $s1 =   "str1";

    my $o = bless [ "obj" ];

    # try variations that will call either pp_concat or pp_multiconcat,
    # with the ref as the first or a later arg

    ::is($r1.$o,        "ref1-obj",             "RT #132385 r1.o");
    ::is($r1.$o.$s1 ,   "ref1-objstr1",         "RT #132385 r1.o.s1");
    ::is("const".$o.$s1 ,"const-objstr1",       "RT #132385 const.o.s1");
    ::is(C.$o.$s1       ,"constref-objstr1",    "RT #132385 C.o.s1");

    ::like($r1.$r2.$o,   qr/^ARRAY\(0x\w+\)ARRAY\(0x\w+\)-obj/,
                                                "RT #132385 r1.r2.o");

    # ditto with a mutator
    ::is($o .= $r1,     "obj-ref1",             "RT #132385 o.=r1");
}

# the RHS of an overloaded .= should be passed as-is to the overload
# method, rather than being stringified or otherwise being processed in
# such a way that it triggers an undef warning
package RT132783 {
    use warnings;
    use overload '.=' => sub { return "foo" };
    my $w = 0;
    local $SIG{__WARN__} = sub { $w++ };
    my $undef;
    my $ov = bless [];
    $ov .= $undef;
    ::is($w, 0, "RT #132783 - should be no warnings");
}

# changing the overloaded object to a plain string within an overload
# method should be permanent.
package RT132827 {
    use overload '""' => sub { $_[0] = "a" };
    my $ov = bless [];
    my $b = $ov . "b";
    ::is(ref \$ov, "SCALAR", "RT #132827");
}

# RT #132793
# An arg like "$b" in $overloaded .= "$b" should be stringified
# before being passed to the method

package RT132793 {
    my $type;
    my $str = 0;
    use overload
        '.=' => sub { $type = ref(\$_[1]); "foo"; },
        '""' => sub { $str++;              "bar" };

    my $a = bless {};
    my $b = bless {};
    $a .= "$b";
    ::is($type, "SCALAR", "RT #132793 type");
    ::is($str,  1,        "RT #132793 stringify count");
}

# RT #132801
# A second RHS-not-stringified bug

package RT132801 {
    my $type;
    my $str    = 0;
    my $concat = 0;
    use overload
        '.'  => sub { $concat++; bless []; },
        '""' => sub { $str++;    "bar" };

    my $a = "A";
    my $b = bless [];
    my $c;
    $c = "$a-$b";
    ::is($concat, 1, "RT #132801 concat count");
    ::is($str,    1, "RT #132801 stringify count");
}

# General testing of optimising away OP_STRINGIFY, and whether
# OP_MULTICONCAT emulates existing behaviour.
#
# It could well be argued that the existing behaviour is buggy, but
# for now emulate the old behaviour.
#
# In more detail:
#
# Since 5.000, any OP_STRINGIFY immediately following an OP_CONCAT
# is optimised away, on the assumption that since concat will always
# return a valid string anyway, it doesn't need stringifying.
# So in "$x", the stringify is needed, but on "$x$y" it isn't.
# This assumption is flawed once overloading has been introduced, since
# concat might return an overloaded object which still needs stringifying.
# However, this flawed behaviour is apparently needed by at least one
# module, and is tested for in opbasic/concat.t: see RT #124160.
#
# There is also a wart with the OPpTARGET_MY optimisation: specifically,
# in $lex = "...", if $lex is a lexical var, then a chain of 2 or more
# concats *doesn't* optimise away OP_STRINGIFY:
#
# $lex = "$x";        # stringifies
# $lex = "$x$y";      # doesn't stringify
# $lex = "$x$y$z..."; # stringifies

package Stringify {
    my $count;
    use overload
        '.'  => sub {
                        my ($a, $b, $rev) = @_;
                        bless [ $rev ? "$b" . $a->[0] : $a->[0] . "$b" ];
            },
        '""' => sub {  $count++; $_[0][0] },
    ;

    for my $test(
        [ 1, '$pkg   =  "$ov"' ],
        [ 1, '$lex   =  "$ov"' ],
        [ 1, 'my $a  =  "$ov"' ],
        [ 1, '$pkg  .=  "$ov"' ],
        [ 1, '$lex  .=  "$ov"' ],
        [ 1, 'my $a .=  "$ov"' ],

        [ 0, '$pkg   =  "$ov$x"' ],
        [ 0, '$lex   =  "$ov$x"' ],
        [ 0, 'my $a  =  "$ov$x"' ],
        [ 0, '$pkg  .=  "$ov$x"' ],
        [ 0, '$lex  .=  "$ov$x"' ],
        [ 0, 'my $a .=  "$ov$x"' ],

        [ 0, '$pkg   =  "$ov$x$y"' ],
        [ 1, '$lex   =  "$ov$x$y"' ],  # XXX note the anomaly
        [ 0, 'my $a  =  "$ov$x$y"' ],
        [ 0, '$pkg  .=  "$ov$x$y"' ],
        [ 0, '$lex  .=  "$ov$x$y"' ],
        [ 0, 'my $a .=  "$ov$x$y"' ],
    )
    {
        my ($stringify, $code) = @$test;
        our $pkg = 'P';
        my ($ov, $x, $y, $lex) = (bless(['OV']), qw(X Y L));
        $count = 0;
        eval "$code; 1" or die $@;
        ::is $count, $stringify, $code;
    }
}

# RT #133789: in multiconcat with overload, the overloaded ref returned
# from the overload method was being assigned to the pad targ, causing
# a delay to the freeing of the object

package RT33789 {
    use overload
        '.'  => sub { $_[0] }
    ;

    my $destroy = 0;
    sub DESTROY { $destroy++ }

    {
        my $o = bless [];
        my $result = '1' . ( '2' . ( '3' . ( '4' . ( '5' . $o ) ) ) );
    }
    ::is($destroy, 1, "RT #133789: delayed destroy");
}
