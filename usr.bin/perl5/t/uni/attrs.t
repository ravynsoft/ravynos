#!./perl

# Regression tests for attributes.pm and the C< : attrs> syntax.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load attributes");
}

use utf8;
use open qw( :utf8 :std );
use warnings;
use feature 'unicode_strings';

$SIG{__WARN__} = sub { die @_ };

sub eval_ok ($;$) {
    eval shift;
    is( $@, '', @_);
}

fresh_perl_is 'use attributes; print "ok"', 'ok', {},
   'attributes.pm can load without warnings.pm already loaded';

eval 'sub è1 ($) : plùgh ;';
like $@, qr/^Invalid CODE attributes?: ["']?plùgh["']? at/;

eval 'sub ɛ2 ($) : plǖgh(0,0) xyzzy ;';
like $@, qr/^Invalid CODE attributes: ["']?plǖgh\(0,0\)["']? /;

eval 'my ($x,$y) : plǖgh;';
like $@, qr/^Invalid SCALAR attribute: ["']?plǖgh["']? at/;

# bug #16080
eval '{my $x : plǖgh}';
like $@, qr/^Invalid SCALAR attribute: ["']?plǖgh["']? at/;
eval '{my ($x,$y) : plǖgh(})}';
like $@, qr/^Invalid SCALAR attribute: ["']?plǖgh\(\}\)["']? at/;

# More syntax tests from the attributes manpage
eval 'my $x : Şʨᚻ(10,ᕘ(7,3))  :  에ㄒ펜ሲ;';
like $@, qr/^Invalid SCALAR attributes: ["']?Şʨᚻ\(10,ᕘ\(7,3\)\) : 에ㄒ펜ሲ["']? at/;
eval q/my $x : Ugļᑈ('\(") :받;/;
like $@, qr/^Invalid SCALAR attributes: ["']?Ugļᑈ\('\\\("\) : 받["']? at/;
eval 'my $x : Şʨᚻ(10,ᕘ();';
like $@, qr/^Unterminated attribute parameter in attribute list at/;
eval q/my $x : Ugļᑈ('(');/;
like $@, qr/^Unterminated attribute parameter in attribute list at/;

sub A::MODIFY_SCALAR_ATTRIBUTES { return }
eval 'my A $x : plǖgh;';
like $@, qr/^SCALAR package attribute may clash with future reserved word: ["']?plǖgh["']? at/;

eval 'my A $x : plǖgh plover;';
like $@, qr/^SCALAR package attributes may clash with future reserved words: ["']?plǖgh["']? /;

no warnings 'reserved';
eval 'my A $x : plǖgh;';
is $@, '';

eval 'package Càt; my Càt @socks;';
is $@, '';

eval 'my Càt %nap;';
is $@, '';

sub X::MODIFY_CODE_ATTRIBUTES { die "$_[0]" }
sub X::ᕘ { 1 }
*Y::bar = \&X::ᕘ;
*Y::bar = \&X::ᕘ;	# second time for -w
eval 'package Z; sub Y::bar : ᕘ';
like $@, qr/^X at /;

# Begin testing attributes that tie

{
    package Ttìè;
    sub DESTROY {}
    sub TIESCALAR { my $x = $_[1]; bless \$x, $_[0]; }
    sub FETCH { ${$_[0]} }
    sub STORE {
	::pass;
	${$_[0]} = $_[1]*2;
    }
    package Tlòòp;
    sub MODIFY_SCALAR_ATTRIBUTES { tie ${$_[1]}, 'Ttìè', -1; (); }
}

eval_ok '
    package Tlòòp;
    for my $i (0..2) {
	my $x : TìèLòòp = $i;
	$x != $i*2 and ::is $x, $i*2;
    }
';

# bug #15898
eval 'our ${""} : ᕘ = 1';
like $@, qr/Can't declare scalar dereference in "our"/;
eval 'my $$ᕘ : bar = 1';
like $@, qr/Can't declare scalar dereference in "my"/;


# this will segfault if it fails
sub PVBM () { 'ᕘ' }
{ my $dummy = index 'ᕘ', PVBM }

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
	$c=undef; eval 'sub t0 :ᕘ';
	main::ok $c == \&{"t0"};
	$c=undef; eval 'sub t1 :ᕘ { }';
	main::ok $c == \&{"t1"};
	$c=undef; eval 'sub t2';
	our $t2a = \&{"t2"};
	$c=undef; eval 'sub t2 :ᕘ';
	main::ok $c == \&{"t2"} && $c == $t2a;
	$c=undef; eval 'sub t3';
	our $t3a = \&{"t3"};
	$c=undef; eval 'sub t3 :ᕘ { }';
	main::ok $c == \&{"t3"} && $c == $t3a;
	$c=undef; eval 'sub t4 :ᕘ';
	our $t4a = \&{"t4"};
	our $t4b = $c;
	$c=undef; eval 'sub t4 :ᕘ';
	main::ok $c == \&{"t4"} && $c == $t4b && $c == $t4a;
	$c=undef; eval 'sub t5 :ᕘ';
	our $t5a = \&{"t5"};
	our $t5b = $c;
	$c=undef; eval 'sub t5 :ᕘ { }';
	main::ok $c == \&{"t5"} && $c == $t5b && $c == $t5a;
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

# [perl #68658] Attributes on stately variables
{
  package thwext;
  sub MODIFY_SCALAR_ATTRIBUTES { () }
  my $i = 0;
  my $x_values = '';
  eval 'sub ᕘ { use 5.01; state $x :A0 = $i++; $x_values .= $x }';
  ᕘ(); ᕘ();
  package main;
  is $x_values, '00', 'state with attributes';
}

{
  package 닌g난ㄬ;
  sub MODIFY_SCALAR_ATTRIBUTES{}
  sub MODIFY_ARRAY_ATTRIBUTES{  }
  sub MODIFY_HASH_ATTRIBUTES{    }
  my ($cows, @go, %bong) : テa퐅Š = qw[ jibber jabber joo ];
  ::is $cows, 'jibber', 'list assignment to scalar with attrs';
  ::is "@go", 'jabber joo', 'list assignment to array with attrs';
}

done_testing();
