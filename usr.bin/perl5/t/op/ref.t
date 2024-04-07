#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(. ../lib) );
}

use strict qw(refs subs);

plan(254);

# Test this first before we extend the stack with other operations.
# This caused an asan failure due to a bad write past the end of the stack.
eval { die  1..127, $_=\() };

# Test glob operations.

$bar = "one";
$foo = "two";
{
    local(*foo) = *bar;
    is($foo, 'one');
}
is ($foo, 'two');

$baz = "three";
$foo = "four";
{
    local(*foo) = 'baz';
    is ($foo, 'three');
}
is ($foo, 'four');

$foo = "global";
{
    local(*foo);
    is ($foo, undef);
    $foo = "local";
    is ($foo, 'local');
}
is ($foo, 'global');

{
    no strict 'refs';
# Test fake references.

    $baz = "valid";
    $bar = 'baz';
    $foo = 'bar';
    is ($$$foo, 'valid');
}

# Test real references.

$FOO = \$BAR;
$BAR = \$BAZ;
$BAZ = "hit";
is ($$$FOO, 'hit');

# Test references to real arrays.

my $test = curr_test();
@ary = ($test,$test+1,$test+2,$test+3);
$ref[0] = \@a;
$ref[1] = \@b;
$ref[2] = \@c;
$ref[3] = \@d;
for $i (3,1,2,0) {
    push(@{$ref[$i]}, "ok $ary[$i]\n");
}
print @a;
print ${$ref[1]}[0];
print @{$ref[2]}[0];
{
    no strict 'refs';
    print @{'d'};
}
curr_test($test+4);

# Test references to references.

$refref = \\$x;
$x = "Good";
is ($$$refref, 'Good');

# Test nested anonymous arrays.

$ref = [[],2,[3,4,5,]];
is (scalar @$ref, 3);
is ($$ref[1], 2);
is (${$$ref[2]}[2], 5);
is (scalar @{$$ref[0]}, 0);

is ($ref->[1], 2);
is ($ref->[2]->[0], 3);

# Test references to hashes of references.

$refref = \%whatever;
$refref->{"key"} = $ref;
is ($refref->{"key"}->[2]->[0], 3);

# Test to see if anonymous subarrays spring into existence.

$spring[5]->[0] = 123;
$spring[5]->[1] = 456;
push(@{$spring[5]}, 789);
is (join(':',@{$spring[5]}), "123:456:789");

# Test to see if anonymous subhashes spring into existence.

@{$spring2{"foo"}} = (1,2,3);
$spring2{"foo"}->[3] = 4;
is (join(':',@{$spring2{"foo"}}), "1:2:3:4");

# Test references to subroutines.

{
    my $called;
    sub mysub { $called++; }
    $subref = \&mysub;
    &$subref;
    is ($called, 1);
}
is ref eval {\&{""}}, "CODE", 'reference to &{""} [perl #94476]';
delete $My::{"Foo::"}; 
is ref \&My::Foo::foo, "CODE",
  'creating stub with \&deleted_stash::foo [perl #128532]';


# Test references to return values of operators (TARGs/PADTMPs)
{
    my @refs;
    for("a", "b") {
        push @refs, \"$_"
    }
    is join(" ", map $$_, @refs), "a b", 'refgen+PADTMP';
}

$subrefref = \\&mysub2;
is ($$subrefref->("GOOD"), "good");
sub mysub2 { lc shift }

# Test REGEXP assignment

SKIP: {
    skip_if_miniperl("no dynamic loading on miniperl, so can't load re", 5);
    require re;
    my $x = qr/x/;
    my $str = "$x"; # regex stringification may change

    my $y = $$x;
    is ($y, $str, "bare REGEXP stringifies correctly");
    ok (eval { "x" =~ $y }, "bare REGEXP matches correctly");
    
    my $z = \$y;
    ok (re::is_regexp($z), "new ref to REXEXP passes is_regexp");
    is ($z, $str, "new ref to REGEXP stringifies correctly");
    ok (eval { "x" =~ $z }, "new ref to REGEXP matches correctly");
}
{
    my ($x, $str);
    {
        my $y = qr/x/;
        $str = "$y";
        $x = $$y;
    }
    is ($x, $str, "REGEXP keeps a ref to its mother_re");
    ok (eval { "x" =~ $x }, "REGEXP with mother_re still matches");
}

# test dereferencing errors
{
    format STDERR =
.
    my $ref;
    foreach $ref (*STDOUT{IO}, *STDERR{FORMAT}) {
	eval q/ $$ref /;
	like($@, qr/Not a SCALAR reference/, "Scalar dereference");
	eval q/ @$ref /;
	like($@, qr/Not an ARRAY reference/, "Array dereference");
	eval q/ %$ref /;
	like($@, qr/Not a HASH reference/, "Hash dereference");
	eval q/ &$ref /;
	like($@, qr/Not a CODE reference/, "Code dereference");
    }

    $ref = *STDERR{FORMAT};
    eval q/ *$ref /;
    like($@, qr/Not a GLOB reference/, "Glob dereference");

    $ref = *STDOUT{IO};
    eval q/ *$ref /;
    is($@, '', "Glob dereference of PVIO is acceptable");

    is($ref, *{$ref}{IO}, "IO slot of the temporary glob is set correctly");
}

# Test the ref operator.

sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

my $pviv = 1; "$pviv";
my $pvnv = 1.0; "$pvnv";
my $x;

# we don't test
#   tied lvalue => SCALAR, as we haven't tested tie yet
#   BIND, 'cos we can't create them yet
#   REGEXP, 'cos that requires overload or Scalar::Util

for (
    [ 'undef',          SCALAR  => \undef               ],
    [ 'constant IV',    SCALAR  => \1                   ],
    [ 'constant NV',    SCALAR  => \1.0                 ],
    [ 'constant PV',    SCALAR  => \'f'                 ],
    [ 'scalar',         SCALAR  => \$x                  ],
    [ 'PVIV',           SCALAR  => \$pviv               ],
    [ 'PVNV',           SCALAR  => \$pvnv               ],
    [ 'PVMG',           SCALAR  => \$0                  ],
    [ 'PVBM',           SCALAR  => \PVBM                ],
    [ 'scalar @array',  SCALAR  => \scalar @array       ],
    [ 'scalar %hash',   SCALAR  => \scalar %hash        ],
    [ 'vstring',        VSTRING => \v1                  ],
    [ 'ref',            REF     => \\1                  ],
    [ 'substr lvalue',  LVALUE  => \substr($x, 0, 0)    ],
    [ 'pos lvalue',     LVALUE  => \pos                 ],
    [ 'vec lvalue',     LVALUE  => \vec($x,0,1)         ],     
    [ 'named array',    ARRAY   => \@ary                ],
    [ 'anon array',     ARRAY   => [ 1 ]                ],
    [ 'named hash',     HASH    => \%whatever           ],
    [ 'anon hash',      HASH    => { a => 1 }           ],
    [ 'named sub',      CODE    => \&mysub,             ],
    [ 'anon sub',       CODE    => sub { 1; }           ],
    [ 'glob',           GLOB    => \*foo                ],
    [ 'format',         FORMAT  => *STDERR{FORMAT}      ],
) {
    my ($desc, $type, $ref) = @$_;
    is (ref $ref, $type, "ref() for ref to $desc");
    like ("$ref", qr/^$type\(0x[0-9a-f]+\)$/, "stringify for ref to $desc");
}

is (ref *STDOUT{IO}, 'IO::File', 'IO refs are blessed into IO::File');
like (*STDOUT{IO}, qr/^IO::File=IO\(0x[0-9a-f]+\)$/,
    'stringify for IO refs');

{ # Test re-use of ref's TARG [perl #101738]
  my $obj = bless [], '____';
  my $uniobj = bless [], chr 256;
  my $get_ref = sub { ref shift };
  my $dummy = &$get_ref($uniobj);
     $dummy = &$get_ref($obj);
  ok exists { ____ => undef }->{$dummy}, 'ref sets UTF8 flag correctly';
}

# Test anonymous hash syntax.

$anonhash = {};
is (ref $anonhash, 'HASH');
$anonhash2 = {FOO => 'BAR', ABC => 'XYZ',};
is (join('', sort values %$anonhash2), 'BARXYZ');

# Test bless operator.

package MYHASH;
{
    no warnings qw(syntax deprecated);
    $object = bless $main'anonhash2;
}
main::is (ref $object, 'MYHASH');
main::is ($object->{ABC}, 'XYZ');

$object2 = bless {};
main::is (ref $object2,	'MYHASH');

# Test ordinary call on object method.

&mymethod($object,"argument");

sub mymethod {
    local($THIS, @ARGS) = @_;
    die 'Got a "' . ref($THIS). '" instead of a MYHASH'
	unless ref $THIS eq 'MYHASH';
    main::is ($ARGS[0], "argument");
    main::is ($THIS->{FOO}, 'BAR');
}

# Test automatic destructor call.

$string = "bad";
$object = "foo";
$string = "good";
{
    no warnings qw(syntax deprecated);
    $main'anonhash2 = "foo";
}
$string = "";

DESTROY {
    return unless $string;
    main::is ($string, 'good');

    # Test that the object has not already been "cursed".
    main::isnt (ref shift, 'HASH');
}

# Now test inheritance of methods.

package OBJ;

@ISA = ('BASEOBJ');

{
    no warnings qw(syntax deprecated);
    $main'object = bless {FOO => 'foo', BAR => 'bar'};
}

package main;

# Test arrow-style method invocation.

is ($object->doit("BAR"), 'bar');

# Test indirect-object-style method invocation.

$foo = doit $object "FOO";
main::is ($foo, 'foo');

{
    no warnings qw(syntax deprecated);
    sub BASEOBJ'doit {
        local $ref = shift;
        die "Not an OBJ" unless ref $ref eq 'OBJ';
        $ref->{shift()};
    }
}

package UNIVERSAL;
@ISA = 'LASTCHANCE';

package LASTCHANCE;
sub foo { main::is ($_[1], 'works') }

package WHATEVER;
foo WHATEVER "works";

#
# test the \(@foo) construct
#
package main;
@foo = \(1..3);
@bar = \(@foo);
@baz = \(1,@foo,@bar);
is (scalar (@bar), 3);
is (scalar grep(ref($_), @bar), 3);
is (scalar (@baz), 3);

my(@fuu) = \(1..2,3);
my(@baa) = \(@fuu);
my(@bzz) = \(1,@fuu,@baa);
is (scalar (@baa), 3);
is (scalar grep(ref($_), @baa), 3);
is (scalar (@bzz), 3);

# also, it can't be an lvalue
# (That’s what *you* think!  --sprout)
eval '\\($x, $y) = (1, 2);';
like ($@, qr/Can\'t modify.*ref.*in.*assignment(?x:
           )|Experimental aliasing via reference not enabled/);

# test for proper destruction of lexical objects
$test = curr_test();
sub larry::DESTROY { print "# larry\nok $test\n"; }
sub curly::DESTROY { print "# curly\nok ", $test + 1, "\n"; }
sub moe::DESTROY   { print "# moe\nok ", $test + 2, "\n"; }

{
    my ($joe, @curly, %larry);
    my $moe = bless \$joe, 'moe';
    my $curly = bless \@curly, 'curly';
    my $larry = bless \%larry, 'larry';
    print "# leaving block\n";
}

print "# left block\n";
curr_test($test + 3);

# another glob test


$foo = "garbage";
{ local(*bar) = "foo" }
$bar = "glob 3";
local(*bar) = *bar;
is ($bar, "glob 3");

$var = "glob 4";
$_   = \$var;
is ($$_, 'glob 4');


# test if reblessing during destruction results in more destruction
$test = curr_test();
{
    package A;
    sub new { bless {}, shift }
    DESTROY { print "# destroying 'A'\nok ", $test + 1, "\n" }
    package _B;
    sub new { bless {}, shift }
    DESTROY { print "# destroying '_B'\nok $test\n"; bless shift, 'A' }
    package main;
    my $b = _B->new;
}
curr_test($test + 2);

# test if $_[0] is properly protected in DESTROY()

{
    my $test = curr_test();
    my $i = 0;
    local $SIG{'__DIE__'} = sub {
	my $m = shift;
	if ($i++ > 4) {
	    print "# infinite recursion, bailing\nnot ok $test\n";
	    exit 1;
        }
	like ($m, qr/^Modification of a read-only/);
    };
    package C;
    sub new { bless {}, shift }
    DESTROY { $_[0] = 'foo' }
    {
	print "# should generate an error...\n";
	my $c = C->new;
    }
    print "# good, didn't recurse\n";
}

# test that DESTROY is called on all objects during global destruction,
# even those without hard references [perl #36347]

is(
  runperl(
   stderr => 1, prog => 'sub DESTROY { print qq-aaa\n- } bless \$a[0]'
  ),
 "aaa\n", 'DESTROY called on array elem'
);
is(
  runperl(
   stderr => 1,
   prog => '{ bless \my@x; *a=sub{@x}}sub DESTROY { print qq-aaa\n- }'
  ),
 "aaa\n",
 'DESTROY called on closure variable'
);

# But cursing objects must not result in double frees
# This caused "Attempt to free unreferenced scalar" in 5.16.
fresh_perl_is(
  'bless \%foo::, bar::; bless \%bar::, foo::; print "ok\n"', "ok\n",
   { stderr => 1 },
  'no double free when stashes are blessed into each other');


# test if refgen behaves with autoviv magic
{
    my @a;
    $a[1] = "good";
    my $got;
    for (@a) {
	$got .= ${\$_};
	$got .= ';';
    }
    is ($got, ";good;");
}

# This test is the reason for postponed destruction in sv_unref
$a = [1,2,3];
$a = $a->[1];
is ($a, 2);

# This test used to coredump. The BEGIN block is important as it causes the
# op that created the constant reference to be freed. Hence the only
# reference to the constant string "pass" is in $a. The hack that made
# sure $a = $a->[1] would work didn't work with references to constants.


foreach my $lexical ('', 'my $a; ') {
  my $expect = "pass\n";
  my $result = runperl (switches => ['-wl'], stderr => 1,
    prog => $lexical . 'BEGIN {$a = \q{pass}}; $a = $$a; print $a');

  is ($?, 0);
  is ($result, $expect);
}

$test = curr_test();
sub x::DESTROY {print "ok ", $test + shift->[0], "\n"}
{ my $a1 = bless [3],"x";
  my $a2 = bless [2],"x";
  { my $a3 = bless [1],"x";
    my $a4 = bless [0],"x";
    567;
  }
}
curr_test($test+4);

is (runperl (switches=>['-l'],
	     prog=> 'print 1; print qq-*$\*-;print 1;'),
    "1\n*\n*\n1\n");

# bug #21347

runperl(prog => 'sub UNIVERSAL::AUTOLOAD { qr// } a->p' );
is ($?, 0, 'UNIVERSAL::AUTOLOAD called when freeing qr//');

runperl(prog => 'sub UNIVERSAL::DESTROY { warn } bless \$a, A', stderr => 1);
is ($?, 0, 'warn called inside UNIVERSAL::DESTROY');


# bug #22719

runperl(prog => 'sub f { my $x = shift; *z = $x; } f({}); f();');
is ($?, 0, 'coredump on typeglob = (SvRV && !SvROK)');

# bug #27268: freeing self-referential typeglobs could trigger
# "Attempt to free unreferenced scalar" warnings

is (runperl(
    prog => 'use Symbol;my $x=bless \gensym,q{t}; print;*$$x=$x',
    stderr => 1
), '', 'freeing self-referential typeglob');

# using a regex in the destructor for STDOUT segfaulted because the
# REGEX pad had already been freed (ithreads build only). The
# object is required to trigger the early freeing of GV refs to STDOUT

TODO: {
    local $TODO = "works but output through pipe is mangled" if $^O eq 'VMS';
    like (runperl(
        prog => '$x=bless[]; sub IO::Handle::DESTROY{$_=q{bad};s/bad/ok/;print}',
        stderr => 1
          ), qr/^(ok)+$/, 'STDOUT destructor');
}

{
    no strict 'refs';
    $name8 = chr 163;
    $name_utf8 = $name8 . chr 256;
    chop $name_utf8;

    is ($$name8, undef, 'Nothing before we start');
    is ($$name_utf8, undef, 'Nothing before we start');
    $$name8 = "Pound";
    is ($$name8, "Pound", 'Accessing via 8 bit symref works');
    is ($$name_utf8, "Pound", 'Accessing via UTF8 symref works');
}

{
    no strict 'refs';
    $name_utf8 = $name = chr 9787;
    utf8::encode $name_utf8;

    is (length $name, 1, "Name is 1 char");
    is (length $name_utf8, 3, "UTF8 representation is 3 chars");

    is ($$name, undef, 'Nothing before we start');
    is ($$name_utf8, undef, 'Nothing before we start');
    $$name = "Face";
    is ($$name, "Face", 'Accessing via Unicode symref works');
    is ($$name_utf8, undef,
	'Accessing via the UTF8 byte sequence gives nothing');
}

{
    no strict 'refs';
    $name1 = "\0Chalk";
    $name2 = "\0Cheese";

    isnt ($name1, $name2, "They differ");

    is ($$name1, undef, 'Nothing before we start (scalars)');
    is ($$name2, undef, 'Nothing before we start');
    $$name1 = "Yummy";
    is ($$name1, "Yummy", 'Accessing via the correct name works');
    is ($$name2, undef,
	'Accessing via a different NUL-containing name gives nothing');
    # defined uses a different code path
    ok (defined $$name1, 'defined via the correct name works');
    ok (!defined $$name2,
	'defined via a different NUL-containing name gives nothing');

    is ($name1->[0], undef, 'Nothing before we start (arrays)');
    is ($name2->[0], undef, 'Nothing before we start');
    $name1->[0] = "Yummy";
    is ($name1->[0], "Yummy", 'Accessing via the correct name works');
    is ($name2->[0], undef,
	'Accessing via a different NUL-containing name gives nothing');
    ok (defined $name1->[0], 'defined via the correct name works');
    ok (!defined$name2->[0],
	'defined via a different NUL-containing name gives nothing');

    my (undef, $one) = @{$name1}[2,3];
    my (undef, $two) = @{$name2}[2,3];
    is ($one, undef, 'Nothing before we start (array slices)');
    is ($two, undef, 'Nothing before we start');
    @{$name1}[2,3] = ("Very", "Yummy");
    (undef, $one) = @{$name1}[2,3];
    (undef, $two) = @{$name2}[2,3];
    is ($one, "Yummy", 'Accessing via the correct name works');
    is ($two, undef,
	'Accessing via a different NUL-containing name gives nothing');
    ok (defined $one, 'defined via the correct name works');
    ok (!defined $two,
	'defined via a different NUL-containing name gives nothing');

    is ($name1->{PWOF}, undef, 'Nothing before we start (hashes)');
    is ($name2->{PWOF}, undef, 'Nothing before we start');
    $name1->{PWOF} = "Yummy";
    is ($name1->{PWOF}, "Yummy", 'Accessing via the correct name works');
    is ($name2->{PWOF}, undef,
	'Accessing via a different NUL-containing name gives nothing');
    ok (defined $name1->{PWOF}, 'defined via the correct name works');
    ok (!defined $name2->{PWOF},
	'defined via a different NUL-containing name gives nothing');

    my (undef, $one) = @{$name1}{'SNIF', 'BEEYOOP'};
    my (undef, $two) = @{$name2}{'SNIF', 'BEEYOOP'};
    is ($one, undef, 'Nothing before we start (hash slices)');
    is ($two, undef, 'Nothing before we start');
    @{$name1}{'SNIF', 'BEEYOOP'} = ("Very", "Yummy");
    (undef, $one) = @{$name1}{'SNIF', 'BEEYOOP'};
    (undef, $two) = @{$name2}{'SNIF', 'BEEYOOP'};
    is ($one, "Yummy", 'Accessing via the correct name works');
    is ($two, undef,
	'Accessing via a different NUL-containing name gives nothing');
    ok (defined $one, 'defined via the correct name works');
    ok (!defined $two,
	'defined via a different NUL-containing name gives nothing');

    $name1 = "Left"; $name2 = "Left\0Right";
    my $glob2 = *{$name2};

    is ($glob1, undef, "We get different typeglobs. In fact, undef");

    *{$name1} = sub {"One"};
    *{$name2} = sub {"Two"};

    is (&{$name1}, "One");
    is (&{$name2}, "Two");
}

# test derefs after list slice

is ( ({foo => "bar"})[0]{foo}, "bar", 'hash deref from list slice w/o ->' );
is ( ({foo => "bar"})[0]->{foo}, "bar", 'hash deref from list slice w/ ->' );
is ( ([qw/foo bar/])[0][1], "bar", 'array deref from list slice w/o ->' );
is ( ([qw/foo bar/])[0]->[1], "bar", 'array deref from list slice w/ ->' );
is ( (sub {"bar"})[0](), "bar", 'code deref from list slice w/o ->' );
is ( (sub {"bar"})[0]->(), "bar", 'code deref from list slice w/ ->' );

# deref on empty list shouldn't autovivify
{
    local $@;
    eval { ()[0]{foo} };
    like ( "$@", qr/Can't use an undefined value as a HASH reference/,
           "deref of undef from list slice fails" );
}

# these will segfault if they fail

my $pvbm = PVBM;
my $rpvbm = \$pvbm;

ok (!eval { *$rpvbm }, 'PVBM ref is not a GLOB ref');
ok (!eval { *$pvbm }, 'PVBM is not a GLOB ref');
ok (!eval { $$pvbm }, 'PVBM is not a SCALAR ref');
ok (!eval { @$pvbm }, 'PVBM is not an ARRAY ref');
ok (!eval { %$pvbm }, 'PVBM is not a HASH ref');
ok (!eval { $pvbm->() }, 'PVBM is not a CODE ref');
ok (!eval { $rpvbm->foo }, 'PVBM is not an object');

# bug 24254
is( runperl(stderr => 1, prog => 'map eval qq(exit),1 for 1'), "");
is( runperl(stderr => 1, prog => 'eval { for (1) { map { die } 2 } };'), "");
is( runperl(stderr => 1, prog => 'for (125) { map { exit } (213)}'), "");
my $hushed = $^O eq 'VMS' ? 'use vmsish qw(hushed);' : '';
is( runperl(stderr => 1, prog => $hushed . 'map die,4 for 3'), "Died at -e line 1.\n");
is( runperl(stderr => 1, prog => $hushed . 'grep die,4 for 3'), "Died at -e line 1.\n");
is( runperl(stderr => 1, prog => $hushed . 'for $a (3) {@b=sort {die} 4,5}'), "Died at -e line 1.\n");

# bug 57564
is( runperl(stderr => 1, prog => 'my $i;for $i (1) { for $i (2) { } }'), "");

# The mechanism for freeing objects in globs used to leave dangling
# pointers to freed SVs. To test this, we construct this nested structure:
#    GV => blessed(AV) => RV => GV => blessed(SV)
# all with a refcnt of 1, and hope that the second GV gets processed first
# by do_clean_named_objs.  Then when the first GV is processed, it mustn't
# find anything nasty left by the previous GV processing.
# The eval is stop things in the main body of the code holding a reference
# to a GV, and the print at the end seems to bee necessary to ensure
# the correct freeing order of *x and *y (no, I don't know why - DAPM).

is (runperl(
	prog => 'eval q[bless \@y; bless \$x; $y[0] = \*x; $z = \*y; ]; '
		. 'delete $::{x}; delete $::{y}; print qq{ok\n};',
	stderr => 1),
    "ok\n", 'freeing freed glob in global destruction');


# Test undefined hash references as arguments to %{} in boolean context
# [perl #81750]
{
 no strict 'refs';
 eval { my $foo; %$foo;             }; ok !$@, '%$undef';
 eval { my $foo; scalar %$foo;      }; ok !$@, 'scalar %$undef';
 eval { my $foo; !%$foo;            }; ok !$@, '!%$undef';
 eval { my $foo; if ( %$foo) {}     }; ok !$@, 'if ( %$undef) {}';
 eval { my $foo; if (!%$foo) {}     }; ok !$@, 'if (!%$undef) {}';
 eval { my $foo; unless ( %$foo) {} }; ok !$@, 'unless ( %$undef) {}';
 eval { my $foo; unless (!%$foo) {} }; ok !$@, 'unless (!%$undef) {}';
 eval { my $foo; 1 if %$foo;        }; ok !$@, '1 if %$undef';
 eval { my $foo; 1 if !%$foo;       }; ok !$@, '1 if !%$undef';
 eval { my $foo; 1 unless %$foo;    }; ok !$@, '1 unless %$undef;';
 eval { my $foo; 1 unless ! %$foo;  }; ok !$@, '1 unless ! %$undef';
 eval { my $foo;  %$foo ? 1 : 0;    }; ok !$@, ' %$undef ? 1 : 0';
 eval { my $foo; !%$foo ? 1 : 0;    }; ok !$@, '!%$undef ? 1 : 0';
}

# RT #88330
# Make sure that a leaked thinggy with multiple weak references to
# it doesn't trigger a panic with multiple rounds of global cleanup
# (Perl_sv_clean_all).

{
    local $ENV{PERL_DESTRUCT_LEVEL} = 2;

    # we do all permutations of array/hash, 1ref/2ref, to account
    # for the different way backref magic is stored

    fresh_perl_is(<<'EOF', 'ok', { stderr => 1 }, 'array with 1 weak ref');
no warnings 'experimental::builtin';
use builtin qw(weaken);
my $r = [];
Internals::SvREFCNT(@$r, 9);
my $r1 = $r;
weaken($r1);
print "ok";
EOF

    fresh_perl_is(<<'EOF', 'ok', { stderr => 1 }, 'array with 2 weak refs');
no warnings 'experimental::builtin';
use builtin qw(weaken);
my $r = [];
Internals::SvREFCNT(@$r, 9);
my $r1 = $r;
weaken($r1);
my $r2 = $r;
weaken($r2);
print "ok";
EOF

    fresh_perl_is(<<'EOF', 'ok', { stderr => 1 }, 'hash with 1 weak ref');
no warnings 'experimental::builtin';
use builtin qw(weaken);
my $r = {};
Internals::SvREFCNT(%$r, 9);
my $r1 = $r;
weaken($r1);
print "ok";
EOF

    fresh_perl_is(<<'EOF', 'ok', { stderr => 1 }, 'hash with 2 weak refs');
no warnings 'experimental::builtin';
use builtin qw(weaken);
my $r = {};
Internals::SvREFCNT(%$r, 9);
my $r1 = $r;
weaken($r1);
my $r2 = $r;
weaken($r2);
print "ok";
EOF

}

{
    my $error;
    *hassgropper::DESTROY = sub {
        no warnings 'experimental::builtin';
        use builtin qw(weaken);
        eval { weaken($_[0]) };
        $error = $@;
        # This line caused a crash before weaken refused to weaken a
        # read-only reference:
        $do::not::overwrite::this = $_[0];
    };
    my $xs = bless [], "hassgropper";
    undef $xs;
    like $error, qr/^Modification of a read-only/,
       'weaken refuses to weaken a read-only ref';
    # Now that the test has passed, avoid sabotaging global destruction:
    undef *hassgropper::DESTROY;
    undef $do::not::overwrite::this;
}


is ref( bless {}, "nul\0clean" ), "nul\0clean", "ref() is nul-clean";

# Test constants and references thereto.
for (3) {
    eval { $_ = 4 };
    like $@, qr/^Modification of a read-only/,
       'assignment to value aliased to literal number';
    eval { ${\$_} = 4 };
    like $@, qr/^Modification of a read-only/,
       'refgen does not allow assignment to value aliased to literal number';
}
for ("4eounthouonth") {
    eval { $_ = 4 };
    like $@, qr/^Modification of a read-only/,
       'assignment to value aliased to literal string';
    eval { ${\$_} = 4 };
    like $@, qr/^Modification of a read-only/,
       'refgen does not allow assignment to value aliased to literal string';
}
{
    my $aref = \123;
    is \$$aref, $aref,
	'[perl #109746] referential identity of \literal under threads+mad'
}

# ref in boolean context
{
    my $false = 0;
    my $true  = 1;
    my $plain = [];
    my $obj     = bless {}, "Foo";
    my $objnull = bless [], "";
    my $obj0    = bless [], "0";
    my $obj00   = bless [], "00";
    my $obj1    = bless [], "1";

    is !ref $false,   1, '!ref $false';
    is !ref $true,    1, '!ref $true';
    is !ref $plain,   "", '!ref $plain';
    is !ref $obj,     "", '!ref $obj';
    is !ref $objnull, "", '!ref $objnull';
    is !ref $obj0   , 1, '!ref $obj0';
    is !ref $obj00,   "", '!ref $obj00';
    is !ref $obj1,    "", '!ref $obj1';

    is ref $obj || 0,               "Foo",   'ref $obj || 0';
    is ref $obj // 0,               "Foo",   'ref $obj // 0';
    is $true && ref $obj,           "Foo",   '$true && ref $obj';
    is ref $obj ? "true" : "false", "true",  'ref $obj ? "true" : "false"';

    my $r = 2;
    if (ref $obj) { $r = 1 };
    is $r, 1, 'if (ref $obj)';

    $r = 2;
    if (ref $obj0) { $r = 1 };
    is $r, 2, 'if (ref $obj0)';

    $r = 2;
    if (ref $obj) { $r = 1 } else { $r = 0 };
    is $r, 1, 'if (ref $obj) else';

    $r = 2;
    if (ref $obj0) { $r = 1 } else { $r = 0 };
    is $r, 0, 'if (ref $obj0) else';
}

{
    # RT #78288
    # if an op returns &PL_sv_zero rather than newSViv(0), the
    # value should be mutable. So ref (via the PADTMP flag) should
    # make a mutable copy

    my @a = ();
    my $r = \ scalar grep $_ == 1, @a;
    $$r += 10;
    is $$r, 10, "RT #78288 - mutable PL_sv_zero copy";
}


# RT#130861: heap-use-after-free in pp_rv2sv, from asan fuzzing
SKIP: {
    skip_if_miniperl("no dynamic loading on miniperl, so can't load arybase", 1);
    # this value is critical - its just enough so that the stack gets
    # grown which loading/calling arybase
    my $n = 125;

    my $code = <<'EOF';
$ary = '[';
my @a = map $$ary, 1..NNN;
print "@a\n";
EOF
    $code =~ s/NNN/$n/g;
    my @exp = ("0") x $n;
    fresh_perl_is($code, "@exp", { stderr => 1 },
                    'rt#130861: heap uaf in pp_rv2sv');
}

# Bit of a hack to make test.pl happy. There are 3 more tests after it leaves.
$test = curr_test();
curr_test($test + 3);
# test global destruction

my $test1 = $test + 1;
my $test2 = $test + 2;

package FINALE;

{
    $ref3 = bless ["ok $test2\n"];	# package destruction
    my $ref2 = bless ["ok $test1\n"];	# lexical destruction
    local $ref1 = bless ["ok $test\n"];	# dynamic destruction
    1;					# flush any temp values on stack
}

DESTROY {
    print $_[0][0];
}

