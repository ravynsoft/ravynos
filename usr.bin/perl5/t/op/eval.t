#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan(tests => 167);

eval 'pass();';

is($@, '');

eval "\$foo\n    = # this is a comment\n'ok 3';";
is($foo, 'ok 3');

eval "\$foo\n    = # this is a comment\n'ok 4\n';";
is($foo, "ok 4\n");

print eval '
$foo =;';		# this tests for a call through yyerror()
like($@, qr/line 2/);

print eval '$foo = /';	# this tests for a call through fatal()
like($@, qr/Search/);

is scalar(eval '++'), undef, 'eval syntax error in scalar context';
is scalar(eval 'die'), undef, 'eval run-time error in scalar context';
is +()=eval '++', 0, 'eval syntax error in list context';
is +()=eval 'die', 0, 'eval run-time error in list context';

is(eval '"ok 7\n";', "ok 7\n");

$foo = 5;
$fact = 'if ($foo <= 1) {1;} else {push(@x,$foo--); (eval $fact) * pop(@x);}';
$ans = eval $fact;
is($ans, 120, 'calculate a factorial with recursive evals');

$foo = 5;
$fact = 'local($foo)=$foo; $foo <= 1 ? 1 : $foo-- * (eval $fact);';
$ans = eval $fact;
is($ans, 120, 'calculate a factorial with recursive evals');

my $curr_test = curr_test();
my $tempfile = tempfile();
open(try,'>',$tempfile);
print try 'print "ok $curr_test\n";',"\n";
close try;

do "./$tempfile"; print $@;

# Test the singlequoted eval optimizer

$i = $curr_test + 1;
for (1..3) {
    eval 'print "ok ", $i++, "\n"';
}

$curr_test += 4;

eval {
    print "ok $curr_test\n";
    die sprintf "ok %d\n", $curr_test + 2;
    1;
} || printf "ok %d\n$@", $curr_test + 1;

curr_test($curr_test + 3);

# check whether eval EXPR determines value of EXPR correctly

{
  my @a = qw(a b c d);
  my @b = eval @a;
  is("@b", '4');
  is($@, '');

  my $a = q[defined(wantarray) ? (wantarray ? ($b='A') : ($b='S')) : ($b='V')];
  my $b;
  @a = eval $a;
  is("@a", 'A');
  is(  $b, 'A');
  $_ = eval $a;
  is(  $b, 'S');
  eval $a;
  is(  $b, 'V');

  $b = 'wrong';
  $x = sub {
     my $b = "right";
     is(eval('"$b"'), $b);
  };
  &$x();
}

{
  my $b = 'wrong';
  my $X = sub {
     my $b = "right";
     is(eval('"$b"'), $b);
  };
  &$X();
}

# check navigation of multiple eval boundaries to find lexicals

my $x = 'aa';
eval <<'EOT'; die if $@;
  print "# $x\n";	# clone into eval's pad
  sub do_eval1 {
     eval $_[0]; die if $@;
  }
EOT
do_eval1('is($x, "aa")');
$x++;
do_eval1('eval q[is($x, "ab")]');
$x++;
do_eval1('sub { print "# $x\n"; eval q[is($x, "ac")] }->()');
$x++;

# calls from within eval'' should clone outer lexicals

eval <<'EOT'; die if $@;
  sub do_eval2 {
     eval $_[0]; die if $@;
  }
do_eval2('is($x, "ad")');
$x++;
do_eval2('eval q[is($x, "ae")]');
$x++;
do_eval2('sub { print "# $x\n"; eval q[is($x, "af")] }->()');
EOT

# calls outside eval'' should NOT clone lexicals from called context

$main::ok = 'not ok';
my $ok = 'ok';
eval <<'EOT'; die if $@;
  # $x unbound here
  sub do_eval3 {
     eval $_[0]; die if $@;
  }
EOT
{
    my $ok = 'not ok';
    do_eval3('is($ok, q{ok})');
    do_eval3('eval q[is($ok, q{ok})]');
    do_eval3('sub { eval q[is($ok, q{ok})] }->()');
}

{
    my $x = curr_test();
    my $got;
    sub recurse {
	my $l = shift;
	if ($l < $x) {
	    ++$l;
	    eval 'print "# level $l\n"; recurse($l);';
	    die if $@;
	}
	else {
	    $got = "ok $l";
	}
    }
    local $SIG{__WARN__} = sub { fail() if $_[0] =~ /^Deep recurs/ };
    recurse(curr_test() - 5);

    is($got, "ok $x",
       "recursive subroutine-call inside eval'' see its own lexicals");
}


eval <<'EOT';
  sub create_closure {
    my $self = shift;
    return sub {
       return $self;
    };
  }
EOT
is(create_closure("good")->(), "good",
   'closures created within eval bind correctly');

$main::r = "good";
sub terminal { eval '$r . q{!}' }
is(do {
   my $r = "bad";
   eval 'terminal($r)';
}, 'good!', 'lexical search terminates correctly at subroutine boundary');

{
    # Have we cured panic which occurred with require/eval in die handler ?
    local $SIG{__DIE__} = sub { eval {1}; die shift };
    eval { die "wham_eth\n" };
    is($@, "wham_eth\n");
}

{
    my $c = eval "(1,2)x10";
    is($c, '2222222222', 'scalar eval"" pops stack correctly');
}

# return from eval {} should clear $@ correctly
{
    my $status = eval {
	eval { die };
	print "# eval { return } test\n";
	return; # removing this changes behavior
    };
    is($@, '', 'return from eval {} should clear $@ correctly');
}

# ditto for eval ""
{
    my $status = eval q{
	eval q{ die };
	print "# eval q{ return } test\n";
	return; # removing this changes behavior
    };
    is($@, '', 'return from eval "" should clear $@ correctly');
}

# Check that eval catches bad goto calls
#   (BUG ID 20010305.003 (#5963))
{
    eval {
	eval { goto foo; };
	like($@, qr/Can't "goto" into the middle of a foreach loop/,
	     'eval catches bad goto calls');
	last;
	foreach my $i (1) {
	    foo: fail('jumped into foreach');
	}
    };
    fail("Outer eval didn't execute the last");
    diag($@);
}

# Make sure that "my $$x" is forbidden
# 20011224 MJD
{
    foreach (qw($$x @$x %$x $$$x)) {
	eval 'my ' . $_;
	isnt($@, '', "my $_ is forbidden");
    }
}

{
    $@ = 5;
    eval q{};
    cmp_ok(length $@, '==', 0, '[ID 20020623.002 (#9721)] eval "" doesn\'t clear $@');
}

# DAPM Nov-2002. Perl should now capture the full lexical context during
# evals.

$::zzz = $::zzz = 0;
my $zzz = 1;

eval q{
    sub fred1 {
	eval q{ is(eval '$zzz', 1); }
    }
    fred1(47);
    { my $zzz = 2; fred1(48) }
};

eval q{
    sub fred2 {
	is(eval('$zzz'), 1);
    }
};
fred2(49);
{ my $zzz = 2; fred2(50) }

# sort() starts a new context stack. Make sure we can still find
# the lexically enclosing sub

sub do_sort {
    my $zzz = 2;
    my @a = sort
	    { is(eval('$zzz'), 2); $a <=> $b }
	    2, 1;
}
do_sort();

# more recursion and lexical scope leak tests

eval q{
    my $r = -1;
    my $yyy = 9;
    sub fred3 {
	my $l = shift;
	my $r = -2;
	return 1 if $l < 1;
	return 0 if eval '$zzz' != 1;
	return 0 if       $yyy  != 9;
	return 0 if eval '$yyy' != 9;
	return 0 if eval '$l' != $l;
	return $l * fred3($l-1);
    }
    my $r = fred3(5);
    is($r, 120);
    $r = eval'fred3(5)';
    is($r, 120);
    $r = 0;
    eval '$r = fred3(5)';
    is($r, 120);
    $r = 0;
    { my $yyy = 4; my $zzz = 5; my $l = 6; $r = eval 'fred3(5)' };
    is($r, 120);
};
my $r = fred3(5);
is($r, 120);
$r = eval'fred3(5)';
is($r, 120);
$r = 0;
eval'$r = fred3(5)';
is($r, 120);
$r = 0;
{ my $yyy = 4; my $zzz = 5; my $l = 6; $r = eval 'fred3(5)' };
is($r, 120);

# check that goto &sub within evals doesn't leak lexical scope

my $yyy = 2;

sub fred4 { 
    my $zzz = 3;
    is($zzz, 3);
    is(eval '$zzz', 3);
    is(eval '$yyy', 2);
}

eval q{
    fred4();
    sub fred5 {
	my $zzz = 4;
	is($zzz, 4);
	is(eval '$zzz', 4);
	is(eval '$yyy', 2);
	goto &fred4;
    }
    fred5();
};
fred5();
{ my $yyy = 88; my $zzz = 99; fred5(); }
eval q{ my $yyy = 888; my $zzz = 999; fred5(); };

{
   $eval = eval 'sub { eval "sub { %S }" }';
   $eval->({});
   pass('[perl #9728] used to dump core');
}

# evals that appear in the DB package should see the lexical scope of the
# thing outside DB that called them (usually the debugged code), rather
# than the usual surrounding scope

our $x = 1;
{
    my $x=2;
    sub db1	{ $x; eval '$x' }
    sub DB::db2	{ $x; eval '$x' }
    package DB;
    sub db3	{ eval '$x' }
    sub DB::db4	{ eval '$x' }
    sub db5	{ my $x=4; eval '$x' }
    package main;
    sub db6	{ my $x=4; eval '$x' }
}
{
    my $x = 3;
    is(db1(),      2);
    is(DB::db2(),  2);
    is(DB::db3(),  3);
    is(DB::db4(),  3);
    is(DB::db5(),  3);
    is(db6(),      4);
}

# [perl #19022] used to end up with shared hash warnings
# The program should generate no output, so anything we see is on stderr
my $got = runperl (prog => '$h{a}=1; foreach my $k (keys %h) {eval qq{\$k}}',
		   stderr => 1);
is ($got, '');

# And a buggy way of fixing #19022 made this fail - $k became undef after the
# eval for a build with copy on write
{
  my %h;
  $h{a}=1;
  foreach my $k (keys %h) {
    is($k, 'a');

    eval "\$k";

    is($k, 'a');
  }
}

sub Foo {} print Foo(eval {});
pass('#20798 (used to dump core)');

# check for context in string eval
{
  my(@r,$r,$c);
  sub context { defined(wantarray) ? (wantarray ? ($c='A') : ($c='S')) : ($c='V') }

  my $code = q{ context() };
  @r = qw( a b );
  $r = 'ab';
  @r = eval $code;
  is("@r$c", 'AA', 'string eval list context');
  $r = eval $code;
  is("$r$c", 'SS', 'string eval scalar context');
  eval $code;
  is("$c", 'V', 'string eval void context');
}

# [perl #34682] escaping an eval with last could coredump or dup output

$got = runperl (
    prog => 
    'sub A::TIEARRAY { L: { eval { last L } } } tie @a, A; warn qq(ok\n)',
stderr => 1);

is($got, "ok\n", 'eval and last');

# eval undef should be the same as eval "" barring any warnings

{
    local $@ = "foo";
    eval undef;
    is($@, "", 'eval undef');
}

{
    no warnings;
    eval "&& $b;";
    like($@, qr/^syntax error/, 'eval syntax error, no warnings');
}

# a syntax error in an eval called magically (eg via tie or overload)
# resulted in an assertion failure in S_docatch, since doeval_compile had
# already popped the EVAL context due to the failure, but S_docatch
# expected the context to still be there.

{
    my $ok  = 0;
    package Eval1;
    sub STORE { eval '('; $ok = 1 }
    sub TIESCALAR { bless [] }

    my $x;
    tie $x, bless [];
    $x = 1;
    ::is($ok, 1, 'eval docatch');
}

# [perl #51370] eval { die "\x{a10d}" } followed by eval { 1 } did not reset
# length $@ 
$@ = "";
eval { die "\x{a10d}"; };
$_ = length $@;
eval { 1 };

cmp_ok($@, 'eq', "", 'length of $@ after eval');
cmp_ok(length $@, '==', 0, 'length of $@ after eval');

# Check if eval { 1 }; completely resets $@
SKIP: {
    skip_if_miniperl('no dynamic loading on miniperl, no Devel::Peek', 2);
    require Config;
    skip('Devel::Peek was not built', 2)
	unless $Config::Config{extensions} =~ /\bDevel\/Peek\b/;

    my $tempfile = tempfile();
    open $prog, ">", $tempfile or die "Can't create test file";
    print $prog <<'END_EVAL_TEST';
    use Devel::Peek;
    $! = 0;
    $@ = $!;
    Dump($@);
    print STDERR "******\n";
    eval { die "\x{a10d}"; };
    $_ = length $@;
    eval { 1 };
    Dump($@);
    print STDERR "******\n";
    print STDERR "Done\n";
END_EVAL_TEST
    close $prog or die "Can't close $tempfile: $!";
    my $got = runperl(progfile => $tempfile, stderr => 1);
    my ($first, $second, $tombstone) = split (/\*\*\*\*\*\*\n/, $got);

    is($tombstone, "Done\n", 'Program completed successfully');

    $first =~ s/p?[NI]OK,//g;
    s/ PV = 0x[0-9a-f]+/ PV = 0x/ foreach $first, $second;
    s/ LEN = [0-9]+/ LEN = / foreach $first, $second;
    # Dump may double newlines through pipes, though not files
    # which is what this test used to use.
    $second =~ s/ IV = 0\n\n/ IV = 0\n/ if $^O eq 'VMS';

    is($second, $first, 'eval { 1 } completely resets $@');
}

# Test that "use feature" and other hint transmission in evals and s///ee
# don't leak memory
{
    use feature qw(:5.10);
    my $count_expected = ($^H & 0x20000) ? 2 : 1;
    my $t;
    my $s = "a";
    $s =~ s/a/$t = \%^H;  qq( qq() );/ee;
    refcount_is $t, $count_expected, 'RT 63110';
}

# make sure default arg eval only adds a hints hash once to entereval
#
{
    local $_ = "21+12";
    is(eval, 33, 'argless eval without hints');
    use feature qw(:5.10);
    local $_ = "42+24";
    is(eval, 66, 'argless eval with hints');
}

{
    # test that the CV compiled for the eval is freed by checking that no additional 
    # reference to outside lexicals are made.
    my $x;
    refcount_is \$x, 1+1, "originally only 1 reference"; # + 1 to account for the ref here
    eval '$x';
    refcount_is \$x, 1+1, "execution eval doesn't create new references"; # + 1 the same
}

fresh_perl_is(<<'EOP', "ok\n", undef, 'RT #70862');
$::{'@'}='';
eval {};
print "ok\n";
EOP

fresh_perl_is(<<'EOP', "ok\n", undef, 'variant of RT #70862');
eval {
    $::{'@'}='';
};
print "ok\n";
EOP

fresh_perl_is(<<'EOP', "ok\n", undef, 'related to RT #70862');
$::{'@'}=\3;
eval {};
print "ok\n";
EOP

fresh_perl_is(<<'EOP', "ok\n", undef, 'related to RT #70862');
eval {
    $::{'@'}=\3;
};
print "ok\n";
EOP

    fresh_perl_is(<<'EOP', "ok\n", undef, 'segfault on syntax errors in block evals');
# localize the hits hash so the eval ends up with the pad offset of a copy of it in its targ
BEGIN { $^H |= 0x00020000 }
eval q{ eval { + } };
print "ok\n";
EOP

fresh_perl_is(<<'EOP', "ok\n", undef, 'assert fail on non-string in Perl_lex_start');
use overload '""'  => sub { '1;' };
my $ov = bless [];
eval $ov;
print "ok\n";
EOP

for my $k (!0) {
  eval 'my $do_something_with = $k';
  eval { $k = 'mon' };
  is "a" =~ /a/, "1",
    "string eval leaves readonly lexicals readonly [perl #19135]";
}

# [perl #68750]
fresh_perl_is(<<'EOP', "ok\nok\nok\n", undef, 'eval clears %^H');
  BEGIN {
    require re; re->import('/x'); # should only affect surrounding scope
    eval '
      print "a b" =~ /a b/ ? "ok\n" : "nokay\n";
      use re "/m";
      print "a b" =~ /a b/ ? "ok\n" : "nokay\n";
   ';
  }
  print "ab" =~ /a b/ ? "ok\n" : "nokay\n";
EOP

# [perl #70151]
{
    BEGIN { eval 'require re; import re "/x"' }
    ok "ab" =~ /a b/, 'eval does not localise %^H at run time';
}

# The fix for perl #70151 caused an assertion failure that broke
# SNMP::Trapinfo, when toke.c finds no syntax errors but perly.y fails.
eval(q|""!=!~//|);
pass("phew! dodged the assertion after a parsing (not lexing) error");

# [perl #111462]
{
   local $ENV{PERL_DESTRUCT_LEVEL} = 1;
   unlike
     runperl(
      prog => 'BEGIN { $^H{foo} = bar }'
             .'our %FIELDS; my main $x; eval q[$x->{foo}]',
      stderr => 1,
     ),
     qr/Unbalanced string table/,
    'Errors in finalize_optree do not leak string eval op tree';
}

# [perl #114658] Line numbers at end of string eval
for("{;", "{") {
    eval $_; is $@ =~ s/eval \d+/eval 1/rag, <<'EOE',
Missing right curly or square bracket at (eval 1) line 1, at end of line
syntax error at (eval 1) line 1, at EOF
Execution of (eval 1) aborted due to compilation errors.
EOE
	qq'Right line number for eval "$_"';
}

{
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };

    eval "\${\nfoobar\n} = 10; warn q{should be line 3}";
    is(
        $w =~ s/eval \d+/eval 1/ra,
        "should be line 3 at (eval 1) line 3.\n",
        'eval qq{\${\nfoo\n}; warn} updates the line number correctly'
    );
}

sub _117941 { package _117941; eval '$a' }
delete $::{"_117941::"};
_117941();
pass("eval in freed package does not crash");

# eval is supposed normally to clear $@ on success

{
    $@ = 1;
    eval q{$@ = 2};
    ok(!$@, 'eval clearing $@');
}

# RT #127786
# this used to give an assertion failure

{
    package DB {
        sub f127786 { eval q/\$s/ }
    }
    my $s;
    sub { $s; DB::f127786}->();
    pass("RT #127786");
}

# Late calling of destructors overwriting $@.
# When leaving an eval scope (either by falling off the end or dying),
# we must ensure that any temps are freed before the end of the eval
# leave: in particular before $@ is set (to either "" or the error),
# because otherwise the tmps freeing may call a destructor which
# will change $@ (e.g. due to a successful eval) *after* its been set.
# Some extra nested scopes are included in the tests to ensure they don't
# affect the tmps freeing.

{
    package TMPS;
    sub DESTROY { eval { die "died in DESTROY"; } } # alters $@

    eval { { 1; { 1; bless []; } } };
    ::is ($@, "", "FREETMPS: normal try exit");

    eval q{ { 1; { 1; bless []; } } };
    ::is ($@, "", "FREETMPS: normal string eval exit");

    eval { { 1; { 1; return bless []; } } };
    ::is ($@, "", "FREETMPS: return try exit");

    eval q{ { 1; { 1; return bless []; } } };
    ::is ($@, "", "FREETMPS: return string eval exit");

    eval { { 1; { 1; my $x = bless []; die $x = 0, "die in eval"; } } };
    ::like ($@, qr/die in eval/, "FREETMPS: die try exit");

    eval q{ { 1; { 1; my $x = bless []; die $x = 0, "die in eval"; } } };
    ::like ($@, qr/die in eval/, "FREETMPS: die eval string exit");
}

{
    local ${^MAX_NESTED_EVAL_BEGIN_BLOCKS}= 0;
    my ($x, $ok);
    $x = 0;
    $ok= eval 'BEGIN { $x++ } 1';
    ::ok(!$ok,'${^MAX_NESTED_EVAL_BEGIN_BLOCKS} = 0 blocks BEGIN blocks entirely');
    ::like($@,qr/Too many nested BEGIN blocks, maximum of 0 allowed/,
        'Blocked BEGIN results in expected error');
    ::is($x,0,'BEGIN really did nothing');

    ${^MAX_NESTED_EVAL_BEGIN_BLOCKS}= 2;
    $ok= eval 'sub f { my $n= shift; eval q[BEGIN { $x++; f($n-1) if $n>0 } 1] or die $@ } f(3); 1';
    ::ok(!$ok,'${^MAX_NESTED_EVAL_BEGIN_BLOCKS} = 2 blocked three nested BEGIN blocks');
    ::like($@,qr/Too many nested BEGIN blocks, maximum of 2 allowed/,
        'Blocked BEGIN results in expected error');
    ::is($x,2,'BEGIN really did nothing');

}

{
    # make sure that none of these segfault.
    foreach my $line (
        'eval "UNITCHECK { eval q(UNITCHECK { die; }); print q(A-) }";',
        'eval "UNITCHECK { eval q(BEGIN     { die; }); print q(A-) }";',
        'eval "BEGIN     { eval q(UNITCHECK { die; }); print q(A-) }";',
        'CHECK     { eval "]" } print q"A-";',
        'INIT      { eval "]" } print q"A-";',
        'UNITCHECK { eval "]" } print q"A-";',
        'BEGIN     { eval "]" } print q"A-";',
        'INIT      { eval q(UNITCHECK { die; } print 0;); print q(A-); }',
    ) {
        fresh_perl_is($line . ' print "ok";', "A-ok", {}, "No segfault: $line");

        # sort blocks are somewhat special and things that work in normal blocks
        # can blow up in sort blocks, so test these constructs specially.
        my $sort_line= 'my @x= sort { ' . $line . ' } 1,2;';
        fresh_perl_is($sort_line . ' print "ok";', "A-ok", {},
            "No segfault inside sort: $sort_line");
    }
}
{
    # test that all of these cases behave the same
    for my $fragment ('bar', '1+;', '1+;' x 11, 's/', ']') {
        fresh_perl_is(
            # code:
            'use strict; use warnings; $SIG{__DIE__} = sub { die "X" }; ' .
            'eval { eval "'.$fragment.'"; print "after eval $@"; };' .
            'if ($@) { print "outer eval $@" }',
            # wanted:
            "after eval X at - line 1.",
            # opts:
            {},
            # name:
            "test that nested eval '$fragment' calls sig die as expected"
        );
    }
}
