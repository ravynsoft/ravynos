#!./perl

# ** DO NOT ADD ANY MORE TESTS HERE **
# Instead, put the test in the appropriate test file and use the 
# fresh_perl_is()/fresh_perl_like() functions in t/test.pl.

# This is for tests that used to abnormally cause segfaults, and other nasty
# errors that might kill the interpreter and for some reason you can't
# use an eval().

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';	# for which_perl() etc
}

use strict;

my $Perl = which_perl();

$|=1;

my @prgs = ();
while(<DATA>) { 
    if(m/^#{8,}\s*(.*)/) { 
        push @prgs, ['', $1];
    }
    else { 
        $prgs[-1][0] .= $_;
    }
}
plan tests => scalar @prgs;

foreach my $prog (@prgs) {
    my($raw_prog, $name) = @$prog;

    my $switch;
    if ($raw_prog =~ s/^\s*(-\w.*)\n//){
	$switch = $1;
    }

    my($prog,$expected) = split(/\nEXPECT\n/, $raw_prog);
    $prog .= "\n";
    $expected = '' unless defined $expected;

    if ($prog =~ /^\# SKIP: (.+)/m) {
	if (eval $1) {
	    ok(1, "Skip: $1");
	    next;
	}
    }

    $expected =~ s/\n+$//;

    fresh_perl_is($prog, $expected, { switches => [$switch || ''] }, $name);
}

__END__
########
$a = ":="; @_ = split /($a)/o, "a:=b:=c"; print "@_"
EXPECT
a := b := c
########
$cusp = ~0 ^ (~0 >> 1);
use integer;
$, = " ";
print +($cusp - 1) % 8, $cusp % 8, -$cusp % 8, 8 | (($cusp + 1) % 8 + 7), "!\n";
EXPECT
7 0 0 8 !
########
$foo=undef; $foo->go;
EXPECT
Can't call method "go" on an undefined value at - line 1.
########
BEGIN
        {
	    "foo";
        }
########
$array[128]=1
########
$x=0x0eabcd; print $x->ref;
EXPECT
Can't locate object method "ref" via package "961485" (perhaps you forgot to load "961485"?) at - line 1.
########
chop ($str .= <DATA>);
########
close ($banana);
########
$x=2;$y=3;$x<$y ? $x : $y += 23;print $x;
EXPECT
25
########
eval 'sub bar {print "In bar"}';
########
system './perl -ne "print if eof" /dev/null'
########
chop($file = <DATA>);
########
package N;
sub new {my ($obj,$n)=@_; bless \$n}  
$aa=new N 1;
$aa=12345;
print $aa;
EXPECT
12345
########
$_="foo";
printf(STDOUT "%s\n", $_);
EXPECT
foo
########
push(@a, 1, 2, 3,)
########
quotemeta ""
########
for ("ABCDE") {
 &sub;
s/./&sub($&)/eg;
print;}
sub sub {local($_) = @_;
$_ x 4;}
EXPECT
Modification of a read-only value attempted at - line 3.
########
package FOO;sub new {bless {FOO => BAR}};
package main;
use strict vars;   
my $self = new FOO;
print $$self{FOO};
EXPECT
BAR
########
$_="foo";
s/.{1}//s;
print;
EXPECT
oo
########
print scalar ("foo","bar")
EXPECT
bar
########
sub by_number { $a <=> $b; };# inline function for sort below
$as_ary{0}="a0";
@ordered_array=sort by_number keys(%as_ary);
########
sub NewShell
{
  local($Host) = @_;
  my($m2) = $#Shells++;
  $Shells[$m2]{HOST} = $Host;
  return $m2;
}
 
sub ShowShell
{
  local($i) = @_;
}
 
&ShowShell(&NewShell(beach,Work,"+0+0"));
&ShowShell(&NewShell(beach,Work,"+0+0"));
&ShowShell(&NewShell(beach,Work,"+0+0"));
########
   {
       package FAKEARRAY;
   
       sub TIEARRAY
       { print "TIEARRAY @_\n"; 
         die "bomb out\n" unless $count ++ ;
         bless ['foo'] 
       }
       sub FETCH { print "fetch @_\n"; $_[0]->[$_[1]] }
       sub STORE { print "store @_\n"; $_[0]->[$_[1]] = $_[2] }
       sub DESTROY { print "DESTROY \n"; undef @{$_[0]}; }
   }
   
eval 'tie @h, FAKEARRAY, fred' ;
tie @h, FAKEARRAY, fred ;
EXPECT
TIEARRAY FAKEARRAY fred
TIEARRAY FAKEARRAY fred
DESTROY 
########
BEGIN { die "phooey\n" }
EXPECT
phooey
BEGIN failed--compilation aborted at - line 1.
########
BEGIN { 1/$zero }
EXPECT
Illegal division by zero at - line 1.
BEGIN failed--compilation aborted at - line 1.
########
BEGIN { undef = 0 }
EXPECT
Can't modify undef operator in scalar assignment at - line 1, near "0 }"
BEGIN not safe after errors--compilation aborted at - line 1.
########
{
    package foo;
    sub PRINT {
        shift;
        print join(' ', reverse @_)."\n";
    }
    sub PRINTF {
        shift;
	  my $fmt = shift;
        print sprintf($fmt, @_)."\n";
    }
    sub TIEHANDLE {
        bless {}, shift;
    }
    sub READLINE {
	"Out of inspiration";
    }
    sub DESTROY {
	print "and destroyed as well\n";
  }
  sub READ {
      shift;
      print STDOUT "foo->can(READ)(@_)\n";
      return 100; 
  }
  sub GETC {
      shift;
      print STDOUT "Don't GETC, Get Perl\n";
      return "a"; 
  }    
}
{
    local(*FOO);
    tie(*FOO,'foo');
    print FOO "sentence.", "reversed", "a", "is", "This";
    print "-- ", <FOO>, " --\n";
    my($buf,$len,$offset);
    $buf = "string";
    $len = 10; $offset = 1;
    read(FOO, $buf, $len, $offset) == 100 or die "foo->READ failed";
    getc(FOO) eq "a" or die "foo->GETC failed";
    printf "%s is number %d\n", "Perl", 1;
}
EXPECT
This is a reversed sentence.
-- Out of inspiration --
foo->can(READ)(string 10 1)
Don't GETC, Get Perl
Perl is number 1
and destroyed as well
########
my @a; $a[2] = 1; for (@a) { $_ = 2 } print "@a\n"
EXPECT
2 2 2
########
# used to attach defelem magic to all immortal values,
# which made restore of local $_ fail.
foo(2>1);
sub foo { bar() for @_;  }
sub bar { local $_; }
print "ok\n";
EXPECT
ok
########
@a = ($a, $b, $c, $d) = (5, 6);
print "ok\n"
  if ($a[0] == 5 and $a[1] == 6 and !defined $a[2] and !defined $a[3]);
EXPECT
ok
########
print "ok\n" if (1E2<<1 == 200 and 3E4<<3 == 240000);
EXPECT
ok
########
print "ok\n" if ("\0" lt "\xFF");
EXPECT
ok
########
open(H,'run/fresh_perl.t'); # must be in the 't' directory
stat(H);
print "ok\n" if (-e _ and -f _ and -r _);
EXPECT
ok
########
sub thing { 0 || return qw(now is the time) }
print thing(), "\n";
EXPECT
nowisthetime
########
$ren = 'joy';
$stimpy = 'happy';
{ local $main::{ren} = *stimpy; print $ren, ' ' }
print $ren, "\n";
EXPECT
happy joy
########
$stimpy = 'happy';
{ local $main::{ren} = *stimpy; print ${'ren'}, ' ' }
print +(defined(${'ren'}) ? 'oops' : 'joy'), "\n";
EXPECT
happy joy
########
package p;
sub func { print 'really ' unless wantarray; 'p' }
sub groovy { 'groovy' }
package main;
print p::func()->groovy(), "\n"
EXPECT
really groovy
########
@list = ([ 'one', 1 ], [ 'two', 2 ]);
sub func { $num = shift; (grep $_->[1] == $num, @list)[0] }
print scalar(map &func($_), 1 .. 3), " ",
      scalar(map scalar &func($_), 1 .. 3), "\n";
EXPECT
2 3
########
($k, $s)  = qw(x 0);
@{$h{$k}} = qw(1 2 4);
for (@{$h{$k}}) { $s += $_; delete $h{$k} if ($_ == 2) }
print "bogus\n" unless $s == 7;
########
my $a = 'outer';
eval q[ my $a = 'inner'; eval q[ print "$a " ] ];
eval { my $x = 'peace'; eval q[ print "$x\n" ] }
EXPECT
inner peace
########
-w
$| = 1;
sub foo {
    print "In foo1\n";
    eval 'sub foo { print "In foo2\n" }';
    print "Exiting foo1\n";
}
foo;
foo;
EXPECT
In foo1
Subroutine foo redefined at (eval 1) line 1.
Exiting foo1
In foo2
########
$s = 0;
map {#this newline here tickles the bug
$s += $_} (1,2,4);
print "eat flaming death\n" unless ($s == 7);
########
sub foo { local $_ = shift; @_ = split; @_ }
@x = foo(' x  y  z ');
print "you die joe!\n" unless "@x" eq 'x y z';
########
"A" =~ /(?{"{"})/	# Check it outside of eval too
EXPECT
########
/(?{"{"}})/	# Check it outside of eval too
EXPECT
Sequence (?{...}) not terminated with ')' at - line 1.
########
BEGIN { @ARGV = qw(a b c d e) }
BEGIN { print "argv <@ARGV>\nbegin <",shift,">\n" }
END { print "end <",shift,">\nargv <@ARGV>\n" }
INIT { print "init <",shift,">\n" }
CHECK { print "check <",shift,">\n" }
EXPECT
argv <a b c d e>
begin <a>
check <b>
init <c>
end <d>
argv <e>
########
-l
# fdopen from a system descriptor to a system descriptor used to close
# the former.
open STDERR, '>&=STDOUT' or die $!;
select STDOUT; $| = 1; print fileno STDOUT or die $!;
select STDERR; $| = 1; print fileno STDERR or die $!;
EXPECT
1
2
########
-w
sub testme { my $a = "test"; { local $a = "new test"; print $a }}
EXPECT
Can't localize lexical variable $a at - line 1.
########
package X;
sub ascalar { my $r; bless \$r }
sub DESTROY { print "destroyed\n" };
package main;
*s = ascalar X;
EXPECT
destroyed
########
package X;
sub anarray { bless [] }
sub DESTROY { print "destroyed\n" };
package main;
*a = anarray X;
EXPECT
destroyed
########
package X;
sub ahash { bless {} }
sub DESTROY { print "destroyed\n" };
package main;
*h = ahash X;
EXPECT
destroyed
########
package X;
sub aclosure { my $x; bless sub { ++$x } }
sub DESTROY { print "destroyed\n" };
package main;
*c = aclosure X;
EXPECT
destroyed
########
package X;
sub any { bless {} }
my $f = "FH000"; # just to thwart any future optimisations
sub afh { select select ++$f; my $r = *{$f}{IO}; delete $X::{$f}; bless $r }
sub DESTROY { print "destroyed\n" }
package main;
$x = any X; # to bump sv_objcount. IO objs aren't counted??
*f = afh X;
EXPECT
destroyed
destroyed
########
BEGIN {
  $| = 1;
  $SIG{__WARN__} = sub {
    eval { print $_[0] };
    die "bar\n";
  };
  warn "foo\n";
}
EXPECT
foo
bar
BEGIN failed--compilation aborted at - line 8.
########
package X;
@ISA='Y';
sub new {
    my $class = shift;
    my $self = { };
    bless $self, $class;
    my $init = shift;
    $self->foo($init);
    print "new", $init;
    return $self;
}
sub DESTROY {
    my $self = shift;
    print "DESTROY", $self->foo;
}
package Y;
sub attribute {
    my $self = shift;
    my $var = shift;
    if (@_ == 0) {
	return $self->{$var};
    } elsif (@_ == 1) {
	$self->{$var} = shift;
    }
}
sub AUTOLOAD {
    $AUTOLOAD =~ /::([^:]+)$/;
    my $method = $1;
    splice @_, 1, 0, $method;
    goto &attribute;
}
package main;
my $x = X->new(1);
for (2..3) {
    my $y = X->new($_);
    print $y->foo;
}
print $x->foo;
EXPECT
new1new22DESTROY2new33DESTROY31DESTROY1
########
re();
sub re {
    my $re = join '', eval 'qr/(??{ $obj->method })/';
    $re;
}
EXPECT
########
use strict;
my $foo = "ZZZ\n";
END { print $foo }
EXPECT
ZZZ
########
eval '
use strict;
my $foo = "ZZZ\n";
END { print $foo }
';
EXPECT
ZZZ
########
-w
if (@ARGV) { print "" }
else {
  if ($x == 0) { print "" } else { print $x }
}
EXPECT
Use of uninitialized value $x in numeric eq (==) at - line 3.
########
$x = sub {};
foo();
sub foo { eval { return }; }
print "ok\n";
EXPECT
ok
########
# moved to op/lc.t
EXPECT
########
sub f { my $a = 1; my $b = 2; my $c = 3; my $d = 4; next }
my $x = "foo";
{ f } continue { print $x, "\n" }
EXPECT
foo
########
# [perl #3066]
sub C () { 1 }
sub M { print "$_[0]\n" }
eval "C";
M(C);
EXPECT
1
########
print qw(ab a\b a\\b);
EXPECT
aba\ba\b
########
# lexicals declared after the myeval() definition should not be visible
# within it
sub myeval { eval $_[0] }
my $foo = "ok 2\n";
myeval('sub foo { local $foo = "ok 1\n"; print $foo; }');
die $@ if $@;
foo();
print $foo;
EXPECT
ok 1
ok 2
########
# lexicals outside an eval"" should be visible inside subroutine definitions
# within it
eval <<'EOT'; die $@ if $@;
{
    my $X = "ok\n";
    eval 'sub Y { print $X }'; die $@ if $@;
    Y();
}
EOT
EXPECT
ok
########
# [ID 20001202.002 (#4821)] and change #8066 added 'at -e line 1';
# reversed again as a result of [perl #17763]
die qr(x)
EXPECT
(?^:x)
########
# 20001210.003 (#4893) mjd@plover.com
format REMITOUT_TOP =
FOO
.

format REMITOUT =
BAR
.

# This loop causes a segv in 5.6.0
for $lineno (1..61) {
   write REMITOUT;
}

print "It's OK!";
EXPECT
It's OK!
########
# Inaba Hiroto
reset;
if (0) {
  if ("" =~ //) {
  }
}
########
# Nicholas Clark
$ENV{TERM} = 0;
reset;
// if 0;
########
# Vadim Konovalov
use strict;
sub new_pmop($) {
    my $pm = shift;
    return eval "sub {shift=~/$pm/}";
}
new_pmop "abcdef"; reset;
new_pmop "abcdef"; reset;
new_pmop "abcdef"; reset;
new_pmop "abcdef"; reset;
########
# David Dyck
# coredump in 5.7.1
close STDERR; die;
EXPECT
########
# core dump in 20000716.007 (#3516)
-w
"x" =~ /(\G?x)?/;
########
# Bug 20010506.041 (#6952)
"abcd\x{1234}" =~ /(a)(b[c])(d+)?/i and print "ok\n";
EXPECT
ok
########
my $foo = Bar->new();
my @dst;
END {
    ($_ = "@dst") =~ s/\(0x.+?\)/(0x...)/;
    print $_, "\n";
}
package Bar;
sub new {
    my Bar $self = bless [], Bar;
    eval '$self';
    return $self;
}
sub DESTROY { 
    push @dst, "$_[0]";
}
EXPECT
Bar=ARRAY(0x...)
######## (?{...}) compilation bounces on PL_rs
-0
{
  /(?{ $x })/;
  # {
}
BEGIN { print "ok\n" }
EXPECT
ok
######## scalar ref to file test operator segfaults on 5.6.1 [ID 20011127.155 (#7947)]
# This only happens if the filename is 11 characters or less.
$foo = \-f "blah";
print "ok" if ref $foo && !$$foo;
EXPECT
ok
######## [ID 20011128.159 (#7951)] 'X' =~ /\X/ segfault in 5.6.1
print "ok" if 'X' =~ /\X/;
EXPECT
ok
######## segfault in 5.6.1 within peep()
@a = (1..9);
@b = sort { @c = sort { @d = sort { 0 } @a; @d; } @a; } @a;
print join '', @a, "\n";
EXPECT
123456789
######## example from Camel 5, ch. 15, pp.406 (with my)
# SKIP: ord "A" == 193 # EBCDIC
use strict;
use utf8;
my $人 = 2; # 0xe4 0xba 0xba: U+4eba, "human" in CJK ideograph
$人++; # a child is born
print $人, "\n";
EXPECT
3
######## example from Camel 5, ch. 15, pp.406 (with our)
# SKIP: ord "A" == 193 # EBCDIC
use strict;
use utf8;
our $人 = 2; # 0xe4 0xba 0xba: U+4eba, "human" in CJK ideograph
$人++; # a child is born
print $人, "\n";
EXPECT
3
######## example from Camel 5, ch. 15, pp.406 (with package vars)
# SKIP: ord "A" == 193 # EBCDIC
use utf8;
$人 = 2; # 0xe4 0xba 0xba: U+4eba, "human" in CJK ideograph
$人++; # a child is born
print $人, "\n";
EXPECT
3
######## example from Camel 5, ch. 15, pp.406 (with use vars)
# SKIP: ord "A" == 193 # EBCDIC
use strict;
use utf8;
use vars qw($人);
$人 = 2; # 0xe4 0xba 0xba: U+4eba, "human" in CJK ideograph
$人++; # a child is born
print $人, "\n";
EXPECT
3
########
# test that closures generated by eval"" hold on to the CV of the eval""
# for their entire lifetime
$code = eval q[
  sub { eval '$x = "ok 1\n"'; }
];
&{$code}();
print $x;
EXPECT
ok 1
######## [ID 20020623.009 (#9728)] nested eval/sub segfaults
$eval = eval 'sub { eval "sub { %S }" }';
$eval->({});
######## [perl #17951] Strange UTF error
-W
# From: "John Kodis" <kodis@mail630.gsfc.nasa.gov>
# Newsgroups: comp.lang.perl.moderated
# Subject: Strange UTF error
# Date: Fri, 11 Oct 2002 16:19:58 -0400
# Message-ID: <pan.2002.10.11.20.19.48.407190@mail630.gsfc.nasa.gov>
$_ = "foobar\n";
utf8::upgrade($_); # the original code used a UTF-8 locale (affects STDIN)
# matching is actually irrelevant: avoiding several dozen of these
# Illegal hexadecimal digit '	' ignored at /usr/lib/perl5/5.8.0/utf8_heavy.pl line 152
# is what matters.
/^([[:digit:]]+)/;
EXPECT
######## [perl #20667] unicode regex vs non-unicode regex
# SKIP: !defined &DynaLoader::boot_DynaLoader && !eval 'require "unicore/UCD.pl"'
# (skip under miniperl if Unicode tables are not built yet)
$toto = 'Hello';
$toto =~ /\w/; # this line provokes the problem!
$name = 'A B';
# utf8::upgrade($name) if @ARGV;
if ($name =~ /(\p{IsUpper}) (\p{IsUpper})/){
    print "It's good! >$1< >$2<\n";
} else {
    print "It's not good...\n";
}
EXPECT
It's good! >A< >B<
######## [perl #8760] strangeness with utf8 and warn
$_="foo";utf8::upgrade($_);/bar/i,warn$_;
EXPECT
foo at - line 1.
######## "#75146: 27e904532594b7fb (fix for #23810) introduces a #regression"
use strict;

unshift @INC, sub {
    my ($self, $fn) = @_;

    (my $pkg = $fn) =~ s{/}{::}g;
    $pkg =~ s{.pm$}{};

    if ($pkg eq 'Credit') {
        my $code = <<'EOC';
package Credit;

use NonsenseAndBalderdash;

1;
EOC
        eval $code;
        die "\$@ is $@";
    }

    #print STDERR "Generator: not one of mine, ignoring\n";
    return undef;
};

# create load-on-demand new() constructors
{
    package Credit;
    sub new {
        eval "use Credit";
    }
};

eval {
    my $credit = new Credit;
};

print "If you get here, you didn't crash\n";
EXPECT
If you get here, you didn't crash
######## [perl #112312] crash on syntax error
# SKIP: !defined &DynaLoader::boot_DynaLoader # miniperl
#!/usr/bin/perl
use strict;
use warnings;
sub meow (&);
my %h;
my $k;
meow {
	my $t : need_this;
	$t = {
		size =>  $h{$k}{size};
		used =>  $h{$k}(used}
	};
};
EXPECT
syntax error at - line 12, near "used"
Execution of - aborted due to compilation errors.
######## [perl #112312] crash on syntax error - another test
# SKIP: !defined &DynaLoader::boot_DynaLoader # miniperl
#!/usr/bin/perl
use strict;
use warnings;

sub meow (&);

my %h;
my $k;

meow {
        my $t : need_this;
        $t = {
                size => $h{$k}{size};
                used => $h{$k}(used}
        };
};

sub testo {
        my $value = shift;
        print;
        print;
        print;
        1;
}

EXPECT
syntax error at - line 15, near "used"
Execution of - aborted due to compilation errors.
