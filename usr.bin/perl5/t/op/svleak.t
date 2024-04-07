#!./perl

# A place to put some simple leak tests. Uses XS::APItest to make
# PL_sv_count available, allowing us to run a bit of code multiple times and
# see if the count increases.

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';

    eval { require XS::APItest; XS::APItest->import('sv_count'); 1 }
	or skip_all("XS::APItest not available");
}

use Config;

plan tests => 151;

# run some code N times. If the number of SVs at the end of loop N is
# greater than (N-1)*delta at the end of loop 1, we've got a leak
#
sub leak {
    my ($n, $delta, $code, @rest) = @_;
    my $sv0 = 0;
    my $sv1 = 0;
    for my $i (1..$n) {
	&$code();
	$sv1 = sv_count();
	$sv0 = $sv1 if $i == 1;
    }
    cmp_ok($sv1-$sv0, '<=', ($n-1)*$delta, @rest);
}

# Like leak, but run a string eval instead.
# The code is used instead of the test name
# if the name is absent.
sub eleak {
    my ($n,$delta,$code,@rest) = @_;
    no warnings 'deprecated'; # Silence the literal control character warning
    leak $n, $delta, sub { eval $code },
         @rest ? @rest : $code
}

# run some expression N times. The expr is concatenated N times and then
# evaled, ensuring that there are no scope exits between executions.
# If the number of SVs at the end of expr N is greater than (N-1)*delta at
# the end of expr 1, we've got a leak
#
sub leak_expr {
    my ($n, $delta, $expr, @rest) = @_;
    my $sv0 = 0;
    my $sv1 = 0;
    my $true = 1; # avoid stuff being optimised away
    my $code1 = "($expr || \$true)";
    my $code = "$code1 && (\$sv0 = sv_count())" . ("&& $code1" x 4)
		. " && (\$sv1 = sv_count())";
    if (eval $code) {
	cmp_ok($sv1-$sv0, '<=', ($n-1)*$delta, @rest);
    }
    else {
	fail("eval @rest: $@");
    }
}


my @a;

leak(5, 0, sub {},                 "basic check 1 of leak test infrastructure");
leak(5, 0, sub {push @a,1;pop @a}, "basic check 2 of leak test infrastructure");
leak(5, 1, sub {push @a,1;},       "basic check 3 of leak test infrastructure");

# delete
{
    my $key = "foo";
    $key++ while exists $ENV{$key};
    leak(2, 0, sub { delete local $ENV{$key} },
	'delete local on nonexistent env var');
}

# defined
leak(2, 0, sub { defined *{"!"} }, 'defined *{"!"}');
leak(2, 0, sub { defined *{"["} }, 'defined *{"["}');
leak(2, 0, sub { defined *{"-"} }, 'defined *{"-"}');
sub def_bang { defined *{"!"}; delete $::{"!"} }
def_bang;
leak(2, 0, \&def_bang,'defined *{"!"} vivifying GV');
leak(2, 0, sub { defined *{"["}; delete $::{"["} },
    'defined *{"["} vivifying GV');
sub def_neg { defined *{"-"}; delete $::{"-"} }
def_neg;
leak(2, 0, \&def_neg, 'defined *{"-"} vivifying GV');

# Fatal warnings
my $f = "use warnings FATAL =>";
my $all = "$f 'all';";
eleak(2, 0, "$f 'deprecated'; qq|\\c\{|", 'qq|\c{| with fatal warnings');
eleak(2, 0, "$f 'syntax'; qq|\\c`|", 'qq|\c`| with fatal warnings');
eleak(2, 0, "$all /\$\\ /", '/$\ / with fatal warnings');
eleak(2, 0, "$all s//\\1/", 's//\1/ with fatal warnings');
eleak(2, 0, "$all qq|\\i|", 'qq|\i| with fatal warnings');
eleak(2, 0, "$f 'digit'; qq|\\o{9}|", 'qq|\o{9}| with fatal warnings');
eleak(3, 1, "$f 'misc'; sub foo{} sub foo:lvalue",
     'ignored :lvalue with fatal warnings');
eleak(2, 0, "no warnings; use feature ':all'; $f 'misc';
             my sub foo{} sub foo:lvalue",
     'ignored mysub :lvalue with fatal warnings');
eleak(2, 0, "no warnings; use feature ':all'; $all
             my sub foo{} sub foo:lvalue{}",
     'fatal mysub redef warning');
eleak(2, 0, "$all sub foo{} sub foo{}", 'fatal sub redef warning');
eleak(2, 0, "$all *x=sub {}",
     'fatal sub redef warning with sub-to-glob assignment');
eleak(2, 0, "$all *x=sub() {1}",
     'fatal const sub redef warning with sub-to-glob assignment');
eleak(2, 0, "$all XS::APItest::newCONSTSUB(\\%main::=>name=>0=>1)",
     'newCONSTSUB sub redefinition with fatal warnings');
eleak(2, 0, "$f 'misc'; my\$a,my\$a", 'double my with fatal warnings');
eleak(2, 0, "$f 'misc'; our\$a,our\$a", 'double our with fatal warnings');
eleak(2, 0, "$f 'closure';
             sub foo { my \$x; format=\n\@\n\$x\n.\n} write; ",
     'format closing over unavailable var with fatal warnings');
eleak(2, 0, "$all /(?{})?/ ", '(?{})? with fatal warnings');
eleak(2, 0, "$all /(?{})+/ ", '(?{})+ with fatal warnings');
eleak(2, 0, "$all /[\\i]/ ", 'invalid charclass escape with fatal warns');
eleak(2, 0, "$all /[:foo:]/ ", '/[:foo:]/ with fatal warnings');
eleak(2, 0, "$all /[a-\\d]/ ", '[a-\d] char class with fatal warnings');
eleak(2, 0, "$all v111111111111111111111111111111111111111111111111",
     'vstring num overflow with fatal warnings');

eleak(2, 0, 'sub{<*>}');
# Use a random number of ops, so that the glob op does not reuse the same
# address each time, giving us false passes.
leak(2, 0, sub { eval '$x+'x(1 + rand() * 100) . '<*>'; },
    'freeing partly iterated glob');

eleak(2, 0, 'goto sub {}', 'goto &sub in eval');
eleak(2, 0, '() = sort { goto sub {} } 1,2', 'goto &sub in sort');
eleak(2, 0, '/(?{ goto sub {} })/', 'goto &sub in regexp');

sub TIEARRAY	{ bless [], $_[0] }
sub FETCH	{ $_[0]->[$_[1]] }
sub STORE	{ $_[0]->[$_[1]] = $_[2] }

# local $tied_elem[..] leaks <20020502143736.N16831@dansat.data-plan.com>"
{
    tie my @a, 'main';
    leak(5, 0, sub {local $a[0]}, "local \$tied[0]");
}

# Overloading
require overload;
eleak(2, 0, "BEGIN{overload::constant integer=>sub{}} 1,1,1,1,1,1,1,1,1,1",
     '"too many errors" from constant overloading returning undef');
# getting this one to leak was complicated; we have to unset LOCALIZE_HH:
eleak(2, 0, 'BEGIN{overload::constant integer=>sub{}; $^H &= ~ 0x00020000}
             1,1,1,1,1,1,1,1,1,1',
     '"too many errors" from constant overloading with $^H sabotaged');
eleak(2, 0, "BEGIN{overload::constant integer=>sub{}; undef %^H}
             1,1,1,1,1,1,1,1,1,1",
     '"too many errors" from constant overloading with %^H undefined');


# [perl #74484]  repeated tries leaked SVs on the tmps stack

leak_expr(5, 0, q{"YYYYYa" =~ /.+?(a(.+?)|b)/ }, "trie leak");

# [perl #48004] map/grep didn't free tmps till the end

{
    # qr/1/ just creates tmps that are hopefully freed per iteration

    my $s;
    my @a;
    my @count = (0) x 4; # pre-allocate
    # Using 0..3 with constant endpoints will cause an erroneous test fail-
    # ure, as the SV in the op tree needs to be copied (to protect it),
    # but copying happens *during iteration*, causing the number of SVs to
    # go up.  Using a variable (0..$_3) will cause evaluation of the range
    # operator at run time, not compile time, so the values will already be
    # on the stack before grep starts.
    my $_3 = 3;

    grep qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 0, "void   grep expr:  no new tmps per iter");
    grep { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 0, "void   grep block: no new tmps per iter");

    $s = grep qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 0, "scalar grep expr:  no new tmps per iter");
    $s = grep { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 0, "scalar grep block: no new tmps per iter");

    @a = grep qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 0, "list   grep expr:  no new tmps per iter");
    @a = grep { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 0, "list   grep block: no new tmps per iter");


    map qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 0, "void   map expr:  no new tmps per iter");
    map { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 0, "void   map block: no new tmps per iter");

    $s = map qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 0, "scalar map expr:  no new tmps per iter");
    $s = map { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 0, "scalar map block: no new tmps per iter");

    @a = map qr/1/ && ($count[$_] = sv_count()) && 99,  0..$_3;
    is(@count[3] - @count[0], 3, "list   map expr:  one new tmp per iter");
    @a = map { qr/1/ && ($count[$_] = sv_count()) && 99 }  0..$_3;
    is(@count[3] - @count[0], 3, "list   map block: one new tmp per iter");

}

# Map plus sparse array
{
    my @a;
    $a[10] = 10;
    leak(3, 0, sub { my @b = map 1, @a },
     'map reading from sparse array');
}

{ # broken by 304474c3, fixed by cefd5c7c, but didn't seem to cause
  # any other test failures
  # base test case from ribasushi (Peter Rabbitson)
  no warnings 'experimental::builtin';
  use builtin 'weaken';
  my $weak;
  {
    $weak = my $in = {};
    weaken($weak);
    my $out = { in => $in, in => undef }
  }
  ok(!$weak, "hash referenced weakened SV released");
}

# prototype() errors
leak(2,0, sub { eval { prototype "CORE::fu" } }, 'prototype errors');

# RT #72246: rcatline memory leak on bad $/

leak(2, 0,
    sub {
	my $f;
	open CATLINE, '<', \$f;
	local $/ = "\x{262E}";
	my $str = "\x{2622}";
	eval { $str .= <CATLINE> };
    },
    "rcatline leak"
);

{
    my $RE = qr/
      (?:
        <(?<tag>
          \s*
          [^>\s]+
        )>
      )??
    /xis;

    "<html><body></body></html>" =~ m/$RE/gcs;

    leak(5, 0, sub {
        my $tag = $+{tag};
    }, "named regexp captures");
}

eleak(2,0,'/[:]/');
eleak(2,0,'/[\xdf]/i');
eleak(2,0,'s![^/]!!');
eleak(2,0,'/[pp]/');
eleak(2,0,'/[[:ascii:]]/');
eleak(2,0,'/[[.zog.]]/');
eleak(2,0,'/[.zog.]/');
eleak(2,0,'/|\W/', '/|\W/ [perl #123198]');
eleak(2,0,'/a\sb/', '/a\sb/ [GH #18604]');
eleak(2,0,'no warnings; /(?[])/');
eleak(2,0,'no warnings; /(?[[a]+[b]])/');
eleak(2,0,'no warnings; /(?[[a]-[b]])/');
eleak(2,0,'no warnings; /(?[[a]&[b]])/');
eleak(2,0,'no warnings; /(?[[a]|[b]])/');
eleak(2,0,'no warnings; /(?[[a]^[b]])/');
eleak(2,0,'no warnings; /(?[![a]])/');
eleak(2,0,'no warnings; /(?[\p{Word}])/');
eleak(2,0,'no warnings; /(?[[a]+)])/');
eleak(2,0,'no warnings; /(?[\d\d)])/');

# These can generate one ref count, but just  once.
eleak(4,1,'chr(0x100) =~ /[[:punct:]]/');
eleak(4,1,'chr(0x100) =~ /[[:^punct:]]/');
eleak(4,1,'chr(0x100) =~ /[[:word:]]/');
eleak(4,1,'chr(0x100) =~ /[[:^word:]]/');

eleak(2,0,'chr(0x100) =~ /\P{Assigned}/');
leak(2,0,sub { /(??{})/ }, '/(??{})/');

leak(2,0,sub { !$^V }, '[perl #109762] version object in boolean context');


# [perl #114356] run-time rexexp with unchanging pattern got
# inflated refcounts
eleak(2, 0, q{ my $x = "x"; "abc" =~ /$x/ for 1..5 }, '#114356');

eleak(2, 0, 'sub', '"sub" with nothing following');
eleak(2, 0, '+sub:a{}', 'anon subs with invalid attributes');
eleak(2, 0, 'no warnings; sub a{1 1}', 'sub with syntax error');
eleak(2, 0, 'no warnings; sub {1 1}', 'anon sub with syntax error');
eleak(2, 0, 'no warnings; use feature ":all"; my sub a{1 1}',
     'my sub with syntax error');

# Reification (or lack thereof)
leak(2, 0, sub { sub { local $_[0]; shift }->(1) },
    'local $_[0] on surreal @_, followed by shift');
leak(2, 0, sub { sub { local $_[0]; \@_ }->(1) },
    'local $_[0] on surreal @_, followed by reification');

sub recredef {}
sub Recursive::Redefinition::DESTROY {
    *recredef = sub { CORE::state $x } # state makes it cloneable
}
leak(2, 0, sub {
    bless \&recredef, "Recursive::Redefinition"; eval "sub recredef{}"
}, 'recursive sub redefinition');

# Sub calls
leak(2, 0, sub { local *_; $_[1]=1; &re::regname },
    'passing sparse array to xsub via ampersand call');

# Syntax errors
eleak(2, 0, '"${<<END}"
                 ', 'unterminated here-doc in quotes in multiline eval');
eleak(2, 0, '"${<<END
               }"', 'unterminated here-doc in multiline quotes in eval');
leak(2, 0, sub { eval { do './op/svleak.pl' } },
        'unterminated here-doc in file');
eleak(2, 0, 'tr/9-0//');
eleak(2, 0, 'tr/a-z-0//');
eleak(2, 0, 'no warnings; nonexistent_function 33838',
        'bareword followed by number');
eleak(2, 0, '//dd;'x20, '"too many errors" when parsing m// flags');
eleak(2, 0, 's///dd;'x20, '"too many errors" when parsing s/// flags');
eleak(2, 0, 'no warnings; 2 2;BEGIN{}',
      'BEGIN block after syntax error');
{
    local %INC; # in case Errno is already loaded
    eleak(2, 0, 'no warnings; 2@!{',
                'implicit "use Errno" after syntax error');
}
eleak(2, 0, "\"\$\0\356\"", 'qq containing $ <null> something');
eleak(2, 0, 'END OF TERMS AND CONDITIONS', 'END followed by words');
eleak(2, 0, "+ + +;qq|\\N{a}|"x10,'qq"\N{a}" after errors');
eleak(2, 0, "qq|\\N{%}|",      'qq"\N{%}" (invalid charname)');
eleak(2, 0, "qq|\\N{au}|;",    'qq"\N{invalid}"');
eleak(2, 0, "qq|\\c|;"x10,     '"too many errors" from qq"\c"');
eleak(2, 0, "qq|\\o|;"x10,     '"too many errors" from qq"\o"');
eleak(2, 0, "qq|\\x{|;"x10,    '"too many errors" from qq"\x{"');
eleak(2, 0, "qq|\\N|;"x10,     '"too many errors" from qq"\N"');
eleak(2, 0, "qq|\\N{|;"x10,    '"too many errors" from qq"\N{"');
eleak(2, 0, "qq|\\N{U+GETG}|;"x10,'"too many errors" from qq"\N{U+JUNK}"');


# [perl #114764] Attributes leak scalars
leak(2, 0, sub { eval 'my $x : shared' }, 'my $x :shared used to leak');

eleak(2, 0, 'ref: 1', 'labels');

# Tied hash iteration was leaking if the hash was freed before itera-
# tion was over.
package t {
    sub TIEHASH { bless [] }
    sub FIRSTKEY { 0 }
}
leak(2, 0, sub {
    my $h = {};
    tie %$h, t;
    each %$h;
    undef $h;
}, 'tied hash iteration does not leak');

package explosive_scalar {
    sub TIESCALAR { my $self = shift; bless [undef, {@_}], $self  }
    sub FETCH     { die 'FETCH' if $_[0][1]{FETCH}; $_[0][0] }
    sub STORE     { die 'STORE' if $_[0][1]{STORE}; $_[0][0] = $_[1] }
}
tie my $die_on_fetch, 'explosive_scalar', FETCH => 1;

# List assignment was leaking when assigning explosive scalars to
# aggregates.
leak(2, 0, sub {
    eval {%a = ($die_on_fetch, 0)}; # key
    eval {%a = (0, $die_on_fetch)}; # value
    eval {%a = ($die_on_fetch, $die_on_fetch)}; # both
    eval {%a = ($die_on_fetch)}; # key, odd elements
}, 'hash assignment does not leak');
leak(2, 0, sub {
    eval {@a = ($die_on_fetch)};
    eval {($die_on_fetch, $b) = ($b, $die_on_fetch)};
    # restore
    tie $die_on_fetch, 'explosive_scalar', FETCH => 1;
}, 'array assignment does not leak');

# [perl #107000]
package hhtie {
    sub TIEHASH { bless [] }
    sub STORE    { $_[0][0]{$_[1]} = $_[2] }
    sub FETCH    { die if $explosive; $_[0][0]{$_[1]} }
    sub FIRSTKEY { keys %{$_[0][0]}; each %{$_[0][0]} }
    sub NEXTKEY  { each %{$_[0][0]} }
}
leak(2, 0, sub {
    eval q`
    	BEGIN {
	    $hhtie::explosive = 0;
	    tie %^H, hhtie;
	    $^H{foo} = bar;
	    $hhtie::explosive = 1;
    	}
	{ 1; }
    `;
}, 'hint-hash copying does not leak');

package explosive_array {
    sub TIEARRAY  { bless [[], {}], $_[0]  }
    sub FETCH     { die if $_[0]->[1]{FETCH}; $_[0]->[0][$_[1]]  }
    sub FETCHSIZE { die if $_[0]->[1]{FETCHSIZE}; scalar @{ $_[0]->[0]  }  }
    sub STORE     { die if $_[0]->[1]{STORE}; $_[0]->[0][$_[1]] = $_[2]  }
    sub CLEAR     { die if $_[0]->[1]{CLEAR}; @{$_[0]->[0]} = ()  }
    sub EXTEND    { die if $_[0]->[1]{EXTEND}; return  }
    sub explode   { my $self = shift; $self->[1] = {@_} }
}

leak(2, 0, sub {
    tie my @a, 'explosive_array';
    tied(@a)->explode( STORE => 1 );
    my $x = 0;
    eval { @a = ($x)  };
}, 'explosive array assignment does not leak');

leak(2, 0, sub {
    my ($a, $b);
    eval { warn $die_on_fetch };
}, 'explosive warn argument');

leak(2, 0, sub {
    my $foo = sub { return $die_on_fetch };
    my $res = eval { $foo->() };
    my @res = eval { $foo->() };
}, 'function returning explosive does not leak');

leak(2, 0, sub {
    my $res = eval { {$die_on_fetch, 0} };
    $res = eval { {0, $die_on_fetch} };
}, 'building anon hash with explosives does not leak');

leak(2, 0, sub {
    my $res = eval { [$die_on_fetch] };
}, 'building anon array with explosives does not leak');

leak(2, 0, sub {
    my @a;
    eval { push @a, $die_on_fetch };
}, 'pushing exploding scalar does not leak');

leak(2, 0, sub {
    eval { push @-, '' };
}, 'pushing onto read-only array does not leak');


# Run-time regexp code blocks
{
    use re 'eval';
    my @tests = ('[(?{})]','(?{})');
    for my $t (@tests) {
	leak(2, 0, sub {
	    / $t/;
	}, "/ \$x/ where \$x is $t does not leak");
	leak(2, 0, sub {
	    /(?{})$t/;
	}, "/(?{})\$x/ where \$x is $t does not leak");
    }
}


{
    use warnings FATAL => 'all';
    leak(2, 0, sub {
	no warnings 'once';
	eval { printf uNopened 42 };
    }, 'printfing to bad handle under fatal warnings does not leak');
    open my $fh, ">", \my $buf;
    leak(2, 0, sub {
	eval { printf $fh chr 2455 };
    }, 'wide fatal warning does not make printf leak');
    close $fh or die $!;
}


leak(2,0,sub{eval{require untohunothu}}, 'requiring nonexistent module');

# [perl #120939]
use constant const_av_xsub_leaked => 1 .. 3;
leak(5, 0, sub { scalar &const_av_xsub_leaked }, "const_av_sub in scalar context");

# check that OP_MULTIDEREF doesn't leak when compiled and then freed

eleak(2, 0, <<'EOF', 'OP_MULTIDEREF');
no strict;
no warnings;
my ($x, @a, %h, $r, $k, $i);
$x = $a[0]{foo}{$k}{$i};
$x = $h[0]{foo}{$k}{$i};
$x = $r->[0]{foo}{$k}{$i};
$x = $mdr::a[0]{foo}{$mdr::k}{$mdr::i};
$x = $mdr::h[0]{foo}{$mdr::k}{$mdr::i};
$x = $mdr::r->[0]{foo}{$mdr::k}{$mdr::i};
EOF

# un-localizing a tied (or generally magic) item could leak if the things
# called by mg_set() died

{
    package MG_SET;

    sub TIESCALAR {  bless [] }
    sub FETCH { 1; }
    my $do_die = 0;
    sub STORE { die if $do_die; }

    sub f {
        local $s;
        tie $s, 'MG_SET';
        local $s;
        $do_die = 1;
    }
    sub g {
        eval { my $x = f(); };
    }

    ::leak(5,0, \&g, "MG_SET");
}

# check that @_ isn't leaked when dieing while goto'ing a new sub

{
    package my_goto;
    sub TIEARRAY { bless [] }
    sub FETCH { 1 }
    sub STORE { die if $_[0][0]; $_[0][0] = 1 }

    sub f { eval { g() } }
    sub g {
        my @a;
        tie @a, "my_goto";
        local $a[0];
        goto &h;
    }
    sub h {}

    ::leak(5, 0, \&f, q{goto shouldn't leak @_});
}

# [perl #128313] POSIX warnings shouldn't leak
{
    no warnings 'experimental';
    use re 'strict';
    my $a = 'aaa';
    my $b = 'aa';
    sub f { $a =~ /[^.]+$b/; }
    ::leak(2, 0, \&f, q{use re 'strict' shouldn't leak warning strings});
}

# check that B::RHE->HASH does not leak
{
    package BHINT;
    sub foo {}
    require B;
    my $op = B::svref_2object(\&foo)->ROOT->first;
    sub lk { { my $d = $op->hints_hash->HASH } }
    ::leak(3, 0, \&lk, q!B::RHE->HASH shoudln't leak!);
}


# dying while compiling a regex with codeblocks imported from an embedded
# qr// could leak

{
    my sub codeblocks {
        my $r = qr/(?{ 1; })/;
        my $c = '(?{ 2; })';
        eval { /$r$c/ }
    }
    ::leak(2, 0, \&codeblocks, q{leaking embedded qr codeblocks});
}

{
    # Perl_reg_named_buff_fetch() leaks an AV when called with an RE
    # with no named captures
    sub named {
        "x" =~ /x/;
        re::regname("foo", 1);
    }
    ::leak(2, 0, \&named, "Perl_reg_named_buff_fetch() on no-name RE");
}

{
    sub N_leak { eval 'tr//\N{}-0/' }
    ::leak(2, 0, \&N_leak, "a bad \\N{} in a range leaks");
}

leak 2,0,\&XS::APItest::PerlIO_stderr,'T_INOUT in default typemap';
leak 2,0,\&XS::APItest::PerlIO_stdin, 'T_IN in default typemap';
leak 2,0,\&XS::APItest::PerlIO_stdout,'T_OUT in default typemap';
SKIP: {
 skip "for now; crashes";
 leak 2,1,sub{XS::APItest::PerlIO_exportFILE(*STDIN,"");0},
                                      'T_STDIO in default typemap';
}

{
    my %rh= ( qr/^foo/ => 1);
    sub Regex_Key_Leak { my ($r)= keys %rh; "foo"=~$r; }
    leak 2, 0, \&Regex_Key_Leak,"RT #132892 - regex patterns should not leak";
}

{
    # perl #133660
    fresh_perl_is(<<'PERL', "ok", {}, "check goto core sub doesn't leak");
# done this way to avoid overloads for all of svleak.t
use B;
BEGIN {
    *CORE::GLOBAL::open = sub (*;$@) {
        goto \&CORE::open;
    };
}

my $refcount;
{
    open(my $fh, '<', 'TEST');
    my $sv = B::svref_2object($fh);
    print $sv->REFCNT == 1 ? "ok" : "not ok";
}
PERL
}
