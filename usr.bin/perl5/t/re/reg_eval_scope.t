#!perl

# Test scoping issues with embedded code in regexps.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw(lib ../lib));
    if (is_miniperl()) {
        eval 'require re';
        if ($@) { skip_all("miniperl, no 're'") }
    }
}

plan 49;

fresh_perl_is <<'CODE', '781745', {}, '(?{}) has its own lexical scope';
 my $x = 7; my $a = 4; my $b = 5;
 print "a" =~ /(?{ print $x; my $x = 8; print $x; my $y })a/;
 print $x,$a,$b;
CODE

fresh_perl_is <<'CODE',
 for my $x("a".."c") {
  $y = 1;
  print scalar
   "abcabc" =~
       /
        (
         a (?{ print $y; local $y = $y+1; print $x; my $x = 8; print $x })
         b (?{ print $y; local $y = $y+1; print $x; my $x = 9; print $x })
         c (?{ print $y; local $y = $y+1; print $x; my $x = 10; print $x })
        ){2}
       /x;
  print "$x ";
 }
CODE
 '1a82a93a104a85a96a101a 1b82b93b104b85b96b101b 1c82c93c104c85c96c101c ',
  {},
 'multiple (?{})s in loop with lexicals';

fresh_perl_is <<'CODE', '781745', {}, 'run-time re-eval has its own scope';
 use re qw(eval);
 my $x = 7;  my $a = 4; my $b = 5;
 my $rest = 'a';
 print "a" =~ /(?{ print $x; my $x = 8; print $x; my $y })$rest/;
 print $x,$a,$b;
CODE

fresh_perl_is <<'CODE', '178279371047857967101745', {},
 use re "eval";
 my $x = 7; $y = 1;
 my $a = 4; my $b = 5;
 print scalar
  "abcabc"
    =~ ${\'(?x)
        (
         a (?{ print $y; local $y = $y+1; print $x; my $x = 8; print $x })
         b (?{ print $y; local $y = $y+1; print $x; my $x = 9; print $x })
         c (?{ print $y; local $y = $y+1; print $x; my $x = 10; print $x })
        ){2}
       '};
 print $x,$a,$b
CODE
 'multiple (?{})s in "foo" =~ $string';

fresh_perl_is <<'CODE', '178279371047857967101745', {},
 use re "eval";
 my $x = 7; $y = 1;
 my $a = 4; my $b = 5;
 print scalar
  "abcabc" =~
      /${\'
        (
         a (?{ print $y; local $y = $y+1; print $x; my $x = 8; print $x })
         b (?{ print $y; local $y = $y+1; print $x; my $x = 9; print $x })
         c (?{ print $y; local $y = $y+1; print $x; my $x = 10; print $x })
        ){2}
      '}/x;
 print $x,$a,$b
CODE
 'multiple (?{})s in "foo" =~ /$string/x';

fresh_perl_is <<'CODE', '123123', {},
  for my $x(1..3) {
   push @regexps, qr/(?{ print $x })a/;
  }
 "a" =~ $_ for @regexps;
 "ba" =~ /b$_/ for @regexps;
CODE
 'qr/(?{})/ is a closure';

"a" =~ do { package foo; qr/(?{ $::pack = __PACKAGE__ })a/ };
is $pack, 'foo', 'qr// inherits package';
"a" =~ do { use re "/x"; qr/(?{ $::re = qr-- })a/ };
is $re, '(?^x:)', 'qr// inherits pragmata';

$::pack = '';
"ba" =~ /b${\do { package baz; qr|(?{ $::pack = __PACKAGE__ })a| }}/;
is $pack, 'baz', '/text$qr/ inherits package';
"ba" =~ m+b${\do { use re "/i"; qr|(?{ $::re = qr-- })a| }}+;
is $re, '(?^i:)', '/text$qr/ inherits pragmata';

{
  use re 'eval';
  package bar;
  "ba" =~ /${\'(?{ $::pack = __PACKAGE__ })a'}/;
}
is $pack, 'bar', '/$text/ containing (?{}) inherits package';
{
  use re 'eval', "/m";
  "ba" =~ /${\'(?{ $::re = qr -- })a'}/;
}
is $re, '(?^m:)', '/$text/ containing (?{}) inherits pragmata';

fresh_perl_is <<'CODE', '45', { stderr => 1 }, '(?{die})';
my $a=4; my $b=5;  eval { "a" =~ /(?{die})a/ }; print $a,$b;
CODE

fresh_perl_is <<'CODE', 'Y45', { stderr => 1 }, '(?{eval{die}})';
my $a=4; my $b=5;
"a" =~ /(?{eval { die; print "X" }; print "Y"; })a/; print $a,$b;
CODE

fresh_perl_is <<'CODE',
    my $a=4; my $b=5;
    sub f { "a" =~ /(?{print((caller(0))[3], "\n");})a/ };
    f();
    print $a,$b;
CODE
    "main::f\n45",
    { stderr => 1 }, 'sub f {(?{caller})}';


fresh_perl_is <<'CODE',
    my $a=4; my $b=5;
    sub f { print ((caller(0))[3], "-", (caller(1))[3], "-\n") };
    "a" =~ /(?{f()})a/;
    print $a,$b;
CODE
    "main::f--\n45",
    { stderr => 1 }, 'sub f {caller} /(?{f()})/';


fresh_perl_is <<'CODE',
    my $a=4; my $b=5;
    sub f {
	"a" =~ /(?{print "X"; return; print "Y"; })a/;
	print "Z";
    };
    f();
    print $a,$b;
CODE
    "XZ45",
    { stderr => 1 }, 'sub f {(?{return})}';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; "a" =~ /(?{last})a/; print $a,$b
CODE
    q{Can't "last" outside a loop block at - line 1.},
    { stderr => 1 }, '(?{last})';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; "a" =~ /(?{for (1..4) {last}})a/; print $a,$b
CODE
    '45',
    { stderr => 1 }, '(?{for {last}})';


fresh_perl_is <<'CODE',
for (1) {  my $a=4; my $b=5; "a" =~ /(?{last})a/ }; print $a,$b
CODE
    q{Can't "last" outside a loop block at - line 1.},
    { stderr => 1 }, 'for (1) {(?{last})}';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; eval { "a" =~ /(?{last})a/ }; print $a,$b
CODE
    '45',
    { stderr => 1 }, 'eval {(?{last})}';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; "a" =~ /(?{next})a/; print $a,$b
CODE
    q{Can't "next" outside a loop block at - line 1.},
    { stderr => 1 }, '(?{next})';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; "a" =~ /(?{for (1,2,3) { next} })a/; print $a,$b
CODE
    '45',
    { stderr => 1 }, '(?{for {next}})';


fresh_perl_is <<'CODE',
for (1) {  my $a=4; my $b=5; "a" =~ /(?{next})a/ }; print $a,$b
CODE
    q{Can't "next" outside a loop block at - line 1.},
    { stderr => 1 }, 'for (1) {(?{next})}';


fresh_perl_is <<'CODE',
my $a=4; my $b=5; eval { "a" =~ /(?{next})a/ }; print $a,$b
CODE
    '45',
    { stderr => 1 }, 'eval {(?{next})}';


fresh_perl_is <<'CODE',
my $a=4; my $b=5;
"a" =~ /(?{ goto FOO; print "X"; })a/;
print "Y";
FOO:
print $a,$b
CODE
    q{Can't "goto" out of a pseudo block at - line 2.},
    { stderr => 1 }, '{(?{goto})}';


{
    local $::TODO = "goto doesn't yet work in pseudo blocks";
fresh_perl_is <<'CODE',
my $a=4; my $b=5;
"a" =~ /(?{ goto FOO; print "X"; FOO: print "Y"; })a/;
print "Z";
FOO;
print $a,$b
CODE
    "YZ45",
    { stderr => 1 }, '{(?{goto FOO; FOO:})}';
}

# [perl #3590]
fresh_perl_is <<'CODE', '', { stderr => 1 }, '(?{eval{die}})';
"$_$_$_"; my $foo; # these consume pad entries and ensure a SEGV on opd perls
"" =~ m{(?{exit(0)})};
CODE


# [perl #92256]
{ my $y = "a"; $y =~ /a(?{ undef *_ })/ }
pass "undef *_ in a re-eval does not cause a double free";

# make sure regexp warnings are reported on the right line
# (we don't care what warning */
SKIP: {
    skip("no \\p{Unassigned} under miniperl", 1) if is_miniperl;
    use warnings;
    my $w;
    local $SIG{__WARN__} = sub { $w = "@_" };
    my $qr = qr/(??{'a'})/;
    my $filler = 1;
    my $a = "\x{110000}" =~ /\p{Unassigned}/; my $line = __LINE__;
    like($w, qr/Matched non-Unicode code point .* line $line\b/, "warning on right line");
}

# on immediate exit from pattern with code blocks, make sure PL_curcop is
# restored

{
    use re 'eval';

    my $c = '(?{"1"})';
    my $w = '';
    my $l;

    local $SIG{__WARN__} = sub { $w .= "@_" };
    $l = __LINE__; "1" =~ /^1$c/x and warn "foo";
    like($w, qr/foo.+line $l/, 'curcop 1');

    $w = '';
    $l = __LINE__; "4" =~ /^1$c/x or warn "foo";
    like($w, qr/foo.+line $l/, 'curcop 2');

    $c = '(??{"1"})';
    $l = __LINE__; "1" =~ /^$c/x and warn "foo";
    like($w, qr/foo.+line $l/, 'curcop 3');

    $w = '';
    $l = __LINE__; "4" =~ /^$c/x or warn "foo";
    like($w, qr/foo.+line $l/, 'curcop 4');
}

# [perl #113928] caller behaving unexpectedly in re-evals
#
#   /(?{...})/ should be in the same caller scope as the surrounding code;
# qr/(?{...})/ should be in an anon sub

{

    my $l;

    sub callers {
	my @c;
	my $stack = '';
	my $i = 1;
	while (@c = caller($i++)) {
	    $stack .= "($c[3]:" . ($c[2] - $l) . ')';
	}
	$stack;
    }

    $l = __LINE__;
    my $c;
    is (callers(), '', 'callers() null');
    "" =~ /(?{ $c = callers() })/;
    is ($c, '', 'callers() //');

    $l = __LINE__;
    sub m1 { "" =~ /(?{ $c = callers() })/; }
    m1();
    is ($c, '(main::m1:2)', 'callers() m1');

    $l = __LINE__;
    my $r1 = qr/(?{ $c = callers() })/;
    "" =~ /$r1/;
    is ($c, '(main::__ANON__:2)', 'callers() r1');

    $l = __LINE__;
    sub r1 { "" =~ /$r1/; }
    r1();
    is ($c, '(main::__ANON__:1)(main::r1:2)', 'callers() r1/r1');

    $l = __LINE__;
    sub c2 { $c = callers() }
    my $r2 = qr/(?{ c2 })/;
    "" =~ /$r2/;
    is ($c, '(main::c2:2)(main::__ANON__:3)', 'callers() r2/c2');
    sub r2 { "" =~ /$r2/; }
    r2();
    is ($c, '(main::c2:2)(main::__ANON__:5)(main::r2:6)', 'callers() r2/r2/c2');

    $l = __LINE__;
    sub c3 { $c = callers() }
    my $r3 = qr/(?{ c3 })/;
    my $c1;
    "ABC" =~ /A(?{ $c1 = callers() })B${r3}C/;
    is ($c, '(main::c3:2)(main::__ANON__:4)', 'callers() r3/c3');
    is ($c1,'', 'callers() r3/c3 part 2');
    sub r3 { "ABC" =~ /A(?{ $c1 = callers() })B${r3}C/; }
    r3();
    is ($c, '(main::c3:2)(main::__ANON__:7)(main::r3:8)', 'callers() r3/r3/c3');
    is ($c1,'(main::r3:8)', 'callers() r3/r3/c3 part 2');

}

# [perl #113928] caller behaving unexpectedly in re-evals
#
# make sure __SUB__ within a code block returns something safe.
# NB waht it actually returns is subject to change

{

    my $s;

    sub f1 { /(?{ $s = CORE::__SUB__; })/ }
    f1();
    is ($s, \&f1, '__SUB__ direct');

    my $r = qr/(?{ $s = CORE::__SUB__; })/;
    sub f2 { "" =~ $r }
    f2();
    is ($s, \&f2, '__SUB__ qr');

    sub f3 { "AB" =~ /A${r}B/ }
    f3();
    is ($s, \&f3, '__SUB__ qr multi');
}

# RT #133879
# ensure scope is properly restored when there's an error compiling a
# "looks a bit like it has (?{}) but doesn't" qr//

fresh_perl_like <<'CODE',
    BEGIN {$^H = 0x10000 }; # HINT_NEW_RE
    qr/\(?{/
CODE
    qr/Constant\(qq\) unknown/,
    { stderr => 1 },
    'qr/\(?{';
