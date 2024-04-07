#!./perl

# Test the core keywords.
#
# Initially this test file just checked that CORE::foo got correctly
# deparsed as CORE::foo, hence the name. It's since been expanded
# to fully test both CORE:: versus none, plus that any arguments
# are correctly deparsed. It also cross-checks against regen/keywords.pl
# to make sure we've tested all keywords, and with the correct strength.
#
# A keyword can be either weak or strong. Strong keywords can never be
# overridden, while weak ones can. So deparsing of weak keywords depends
# on whether a sub of that name has been created:
#
# for both:         keyword(..) deparsed as keyword(..)
# for weak:   CORE::keyword(..) deparsed as CORE::keyword(..)
# for strong: CORE::keyword(..) deparsed as keyword(..)
#
# Three permutations of lex/nonlex args are checked for:
#
#   foo($a,$b,$c,...)
#   foo(my $a,$b,$c,...)
#   my ($a,$b,$c,...); foo($a,$b,$c,...)
#
# Note that tests for prefixing feature.pm-enabled keywords with CORE:: when
# feature.pm is not enabled are in deparse.t, as they fit that format better.


BEGIN {
    require Config;
    if (($Config::Config{extensions} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use strict;
use Test::More;

use feature (sprintf(":%vd", $^V)); # to avoid relying on the feature
                                    # logic to add CORE::
use B::Deparse;
my $deparse = B::Deparse->new();

my %SEEN;
my %SEEN_STRENGTH;

# For a given keyword, create a sub of that name,
# then deparse 3 different assignment expressions
# using that keyword.  See if the $expr we get back
# matches $expected_expr.

sub testit {
    my ($keyword, $expr, $expected_expr, $lexsub) = @_;

    $expected_expr //= $expr;
    $SEEN{$keyword} = 1;

    # lex=0:   () = foo($a,$b,$c)
    # lex=1:   my ($a,$b); () = foo($a,$b,$c)
    # lex=2:   () = foo(my $a,$b,$c)
    for my $lex (0, 1, 2) {
        next if ($lex and $keyword =~ /local|our|state|my/);
        my $vars = $lex == 1 ? 'my($a, $b, $c, $d, $e);' . "\n    " : "";

        if ($lex == 2) {
            my $repl = 'my $a';
            if ($expr =~ 'CORE::do') {
                # do foo() is a syntax error, so B::Deparse emits
                # do (foo()), but does not distinguish between foo and my,
                # because it is too complicated.
                $repl = '(my $a)';
            }
            s/\$a/$repl/ for $expr, $expected_expr;
        }

        my $desc = "$keyword: lex=$lex $expr => $expected_expr";
        $desc .= " (lex sub)" if $lexsub;

        my $code;
        my $code_ref;
        if ($lexsub) {
            package lexsubtest;
            no warnings 'experimental::lexical_subs';
            use feature 'lexical_subs';
            no strict 'vars';
            $code = "sub { state sub $keyword; ${vars}() = $expr }";
            $code = "use feature 'isa';\n$code" if $keyword eq "isa";
            $code = "use feature 'switch';\n$code" if $keyword eq "break";
            $code_ref = eval $code or die "$@ in $expr";
        }
        else {
            package test;
            use subs ();
            import subs $keyword;
            $code = "no strict 'vars'; sub { ${vars}() = $expr }";
            $code = "use feature 'isa';\n$code" if $keyword eq "isa";
            $code = "use feature 'switch';\n$code" if $keyword eq "break";
            $code_ref = eval $code or die "$@ in $expr";
        }

        my $got_text = $deparse->coderef2text($code_ref);

        unless ($got_text =~ /
    package (?:lexsub)?test;
(?:    BEGIN \{\$\{\^WARNING_BITS\} = "[^"]+"\}
)?    use strict 'refs', 'subs';
    use feature [^\n]+
(?:    (?:CORE::)?state sub \w+;
)?    \Q$vars\E\(\) = (.*)
\}/s) {
            ::fail($desc);
            ::diag("couldn't extract line from boilerplate\n");
            ::diag($got_text);
            return;
        }

        my $got_expr = $1;
        is $got_expr, $expected_expr, $desc
            or ::diag("ORIGINAL CODE:\n$code");;
    }
}


# Deparse can't distinguish 'and' from '&&' etc
my %infix_map = qw(and && or ||);

# Test a keyword that is a binary infix operator, like 'cmp'.
# $parens - "$a op $b" is deparsed as "($a op $b)"
# $strong - keyword is strong

sub do_infix_keyword {
    my ($keyword, $parens, $strong) = @_;
    $SEEN_STRENGTH{$keyword} = $strong;
    my $expr = "(\$a $keyword \$b)";
    my $nkey = $infix_map{$keyword} // $keyword;
    my $expr = "(\$a $keyword \$b)";
    my $exp = "\$a $nkey \$b";
    $exp = "($exp)" if $parens;
    $exp .= ";";
    # with infix notation, a keyword is always interpreted as core,
    # so no need for Deparse to disambiguate with CORE::
    testit $keyword, "(\$a CORE::$keyword \$b)", $exp;
    testit $keyword, "(\$a $keyword \$b)", $exp;
    testit $keyword, "(\$a CORE::$keyword \$b)", $exp, 1;
    testit $keyword, "(\$a $keyword \$b)", $exp, 1;
    if (!$strong) {
        # B::Deparse fully qualifies any sub whose name is a keyword,
        # imported or not, since the importedness may not be reproduced by
        # the deparsed code.  x is special.
        my $pre = "test::" x ($keyword ne 'x');
        testit $keyword, "$keyword(\$a, \$b)", "$pre$keyword(\$a, \$b);";
    }
    testit $keyword, "$keyword(\$a, \$b)", "$keyword(\$a, \$b);", 1;
}

# Test a keyword that is a standard op/function, like 'index(...)'.
# $narg   - how many args to test it with
# $parens - "foo $a, $b" is deparsed as "foo($a, $b)"
# $dollar - an extra '$_' arg will appear in the deparsed output
# $strong - keyword is strong


sub do_std_keyword {
    my ($keyword, $narg, $parens, $dollar, $strong) = @_;

    $SEEN_STRENGTH{$keyword} = $strong;

    for my $core (0,1) { # if true, add CORE:: to keyword being deparsed
        for my $lexsub (0,1) { # if true, define lex sub
            my @code;
            for my $do_exp(0, 1) { # first create expr, then expected-expr
                my @args = map "\$$_", (undef,"a".."z")[1..$narg];
                push @args, '$_'
                    if $dollar && $do_exp && ($strong && !$lexsub or $core);
                my $args = join(', ', @args);
                # XXX $lex_parens is temporary, until lex subs are
                #     deparsed properly.
                my $lex_parens =
                    !$core && $do_exp && $lexsub && $keyword ne 'map';
                $args = ((!$core && !$strong) || $parens || $lex_parens)
                    ? "($args)"
                    :  @args
                        ? " $args"
                        : "";
                push @code, (
                    ($core && !($do_exp && $strong))
                    ? "CORE::"
                    : $lexsub && $do_exp
                        ? "CORE::" x $core
                        : $do_exp && !$core && !$strong
                            ? "test::"
                            : ""
                ) . "$keyword$args;";
            }
            # code[0]: to run; code[1]: expected
            testit $keyword, @code, $lexsub;
        }
    }
}


while (<DATA>) {
    chomp;
    s/#.*//;
    next unless /\S/;

    my @fields = split;
    die "not 3 fields" unless @fields == 3;
    my ($keyword, $args, $flags) = @fields;

    $args = '012' if $args eq '@';

    my $parens  = $flags =~ s/p//;
    my $invert1 = $flags =~ s/1//;
    my $dollar  = $flags =~ s/\$//;
    my $strong  = $flags =~ s/\+//;
    die "unrecognised flag(s): '$flags'" unless $flags =~ /^-?$/;

    if ($args eq 'B') { # binary infix
        die "$keyword: binary (B) op can't have '\$' flag\\n" if $dollar;
        die "$keyword: binary (B) op can't have '1' flag\\n" if $invert1;
        do_infix_keyword($keyword, $parens, $strong);
    }
    else {
        my @narg = split //, $args;
        for my $n (0..$#narg) {
            my $narg = $narg[$n];
            my $p = $parens;
            $p = !$p if ($n == 0 && $invert1);
            do_std_keyword($keyword, $narg, $p, (!$n && $dollar), $strong);
        }
    }
}


# Special cases

testit dbmopen  => 'CORE::dbmopen(%foo, $bar, $baz);';
testit dbmclose => 'CORE::dbmclose %foo;';

testit delete   => 'CORE::delete $h{\'foo\'};', 'delete $h{\'foo\'};';
testit delete   => 'CORE::delete $h{\'foo\'};', undef, 1;
testit delete   => 'CORE::delete @h{\'foo\'};', undef, 1;
testit delete   => 'CORE::delete $h[0];', undef, 1;
testit delete   => 'CORE::delete @h[0];', undef, 1;
testit delete   => 'delete $h{\'foo\'};',       'delete $h{\'foo\'};';

# do is listed as strong, but only do { block } is strong;
# do $file is weak,  so test it separately here
testit do       => 'CORE::do $a;';
testit do       => 'do $a;',                    'test::do($a);';
testit do       => 'CORE::do { 1 }',
		   "do {\n        1\n    };";
testit do       => 'CORE::do { 1 }',
		   "CORE::do {\n        1\n    };", 1;
testit do       => 'do { 1 };',
		   "do {\n        1\n    };";

testit each     => 'CORE::each %bar;';
testit each     => 'CORE::each @foo;';

testit eof      => 'CORE::eof();';

testit exists   => 'CORE::exists $h{\'foo\'};', 'exists $h{\'foo\'};';
testit exists   => 'CORE::exists $h{\'foo\'};', undef, 1;
testit exists   => 'CORE::exists &foo;', undef, 1;
testit exists   => 'CORE::exists $h[0];', undef, 1;
testit exists   => 'exists $h{\'foo\'};',       'exists $h{\'foo\'};';

testit exec     => 'CORE::exec($foo $bar);';

testit glob     => 'glob;',                       'glob($_);';
testit glob     => 'CORE::glob;',                 'CORE::glob($_);';
testit glob     => 'glob $a;',                    'glob($a);';
testit glob     => 'CORE::glob $a;',              'CORE::glob($a);';

testit grep     => 'CORE::grep { $a } $b, $c',    'grep({$a;} $b, $c);';

testit keys     => 'CORE::keys %bar;';
testit keys     => 'CORE::keys @bar;';

testit map      => 'CORE::map { $a } $b, $c',    'map({$a;} $b, $c);';

testit not      => '3 unless CORE::not $a && $b;';

testit pop      => 'CORE::pop @foo;';

testit push     => 'CORE::push @foo;',           'CORE::push(@foo);';
testit push     => 'CORE::push @foo, 1;',        'CORE::push(@foo, 1);';
testit push     => 'CORE::push @foo, 1, 2;',     'CORE::push(@foo, 1, 2);';

testit readline => 'CORE::readline $a . $b;';

testit readpipe => 'CORE::readpipe $a + $b;';

testit reverse  => 'CORE::reverse sort(@foo);';

testit shift    => 'CORE::shift @foo;';

testit splice   => q{CORE::splice @foo;},                 q{CORE::splice(@foo);};
testit splice   => q{CORE::splice @foo, 0;},              q{CORE::splice(@foo, 0);};
testit splice   => q{CORE::splice @foo, 0, 1;},           q{CORE::splice(@foo, 0, 1);};
testit splice   => q{CORE::splice @foo, 0, 1, 'a';},      q{CORE::splice(@foo, 0, 1, 'a');};
testit splice   => q{CORE::splice @foo, 0, 1, 'a', 'b';}, q{CORE::splice(@foo, 0, 1, 'a', 'b');};

# note that the test does '() = split...' which is why the
# limit is optimised to 1
testit split    => 'split;',                     q{split(' ', $_, 1);};
testit split    => 'CORE::split;',               q{split(' ', $_, 1);};
testit split    => 'split $a;',                  q{split(/$a/u, $_, 1);};
testit split    => 'CORE::split $a;',            q{split(/$a/u, $_, 1);};
testit split    => 'split $a, $b;',              q{split(/$a/u, $b, 1);};
testit split    => 'CORE::split $a, $b;',        q{split(/$a/u, $b, 1);};
testit split    => 'split $a, $b, $c;',          q{split(/$a/u, $b, $c);};
testit split    => 'CORE::split $a, $b, $c;',    q{split(/$a/u, $b, $c);};

testit sub      => 'CORE::sub { $a, $b }',
			"sub {\n        \$a, \$b;\n    }\n    ;";

testit system   => 'CORE::system($foo $bar);';

testit unshift  => 'CORE::unshift @foo;',        'CORE::unshift(@foo);';
testit unshift  => 'CORE::unshift @foo, 1;',     'CORE::unshift(@foo, 1);';
testit unshift  => 'CORE::unshift @foo, 1, 2;',  'CORE::unshift(@foo, 1, 2);';

testit values   => 'CORE::values %bar;';
testit values   => 'CORE::values @foo;';


# XXX These are deparsed wrapped in parens.
# whether they should be, I don't know!

testit dump     => '(CORE::dump);';
testit dump     => '(CORE::dump FOO);';
testit goto     => '(CORE::goto);',     '(goto);';
testit goto     => '(CORE::goto FOO);', '(goto FOO);';
testit last     => '(CORE::last);',     '(last);';
testit last     => '(CORE::last FOO);', '(last FOO);';
testit next     => '(CORE::next);',     '(next);';
testit next     => '(CORE::next FOO);', '(next FOO);';
testit redo     => '(CORE::redo);',     '(redo);';
testit redo     => '(CORE::redo FOO);', '(redo FOO);';
testit redo     => '(CORE::redo);',     '(redo);';
testit redo     => '(CORE::redo FOO);', '(redo FOO);';
testit return   => '(return);',         '(return);';
testit return   => '(CORE::return);',   '(return);';

# these are the keywords I couldn't think how to test within this framework

my %not_tested = map { $_ => 1} qw(
    __DATA__
    __END__
    __FILE__
    __LINE__
    __PACKAGE__
    ADJUST
    AUTOLOAD
    BEGIN
    CHECK
    CORE
    DESTROY
    END
    INIT
    UNITCHECK
    catch
    class
    default
    defer
    else
    elsif
    field
    finally
    for
    foreach
    format
    given
    if
    m
    method
    no
    package
    q
    qq
    qr
    qw
    qx
    require
    s
    tr
    try
    unless
    until
    use
    when
    while
    y
);

# Sanity check against keyword data:
# make sure we haven't missed any keywords,
# and that we got the strength right.

SKIP:
{
    skip "sanity checks when not PERL_CORE", 1 unless defined $ENV{PERL_CORE};
    my $count = 0;
    my $file = '../regen/keywords.pl';
    my $pass = 1;
    if (open my $fh, '<', $file) {
	while (<$fh>) {
	    last if /^__END__$/;
	}
	while (<$fh>) {
	    next unless /^([+\-])(\w+)$/;
	    my ($strength, $key) = ($1, $2);
	    $strength = ($strength eq '+') ? 1 : 0;
	    $count++;
	    if (!$SEEN{$key} && !$not_tested{$key}) {
		diag("keyword '$key' seen in $file, but not tested here!!");
		$pass = 0;
	    }
	    if (exists $SEEN_STRENGTH{$key} and $SEEN_STRENGTH{$key} != $strength) {
		diag("keyword '$key' strengh as seen in $file doen't match here!!");
		$pass = 0;
	    }
	}
    }
    else {
	diag("Can't open $file: $!");
	$pass = 0;
    }
    # insanity check
    if ($count < 200) {
	diag("Saw $count keywords: less than 200!");
	$pass = 0;
    }
    ok($pass, "sanity checks");
}

done_testing();

__DATA__
#
# format:
#   keyword args flags
#
# args consists of:
#  * one of more digits indictating which lengths of args the function accepts,
#  * or 'B' to indiate a binary infix operator,
#  * or '@' to indicate a list function.
#
# Flags consists of the following (or '-' if no flags):
#    + : strong keyword: can't be overrriden
#    p : the args are parenthesised on deparsing;
#    1 : parenthesising of 1st arg length is inverted
#        so '234 p1' means: foo a1,a2;  foo(a1,a2,a3); foo(a1,a2,a3,a4)
#    $ : on the first argument length, there is an implicit extra
#        '$_' arg which will appear on deparsing;
#        e.g. 12p$  will be tested as: foo(a1);     foo(a1,a2);
#                     and deparsed as: foo(a1, $_); foo(a1,a2);
#
# XXX Note that we really should get this data from regen/keywords.pl
# and regen/opcodes (augmented if necessary), rather than duplicating it
# here.

__SUB__          0     -
abs              01    $
accept           2     p
alarm            01    $
and              B     -
atan2            2     p
bind             2     p
binmode          12    p
bless            1     p
break            0     -
caller           0     -
chdir            01    -
chmod            @     p1
chomp            @     $
chop             @     $
chown            @     p1
chr              01    $
chroot           01    $
close            01    -
closedir         1     -
cmp              B     -
connect          2     p
continue         0     -
cos              01    $
crypt            2     p
# dbmopen  handled specially
# dbmclose handled specially
defined          01    $+
# delete handled specially
die              @     p1
# do handled specially
# dump handled specially
# each handled specially
endgrent         0     -
endhostent       0     -
endnetent        0     -
endprotoent      0     -
endpwent         0     -
endservent       0     -
eof              01    - # also tested specially
eq               B     -
eval             01    $+
evalbytes        01    $
exec             @     p1 # also tested specially
# exists handled specially
exit             01    -
exp              01    $
fc               01    $
fcntl            3     p
fileno           1     -
flock            2     p
fork             0     -
formline         2     p
ge               B     -
getc             01    -
getgrent         0     -
getgrgid         1     -
getgrnam         1     -
gethostbyaddr    2     p
gethostbyname    1     -
gethostent       0     -
getlogin         0     -
getnetbyaddr     2     p
getnetbyname     1     -
getnetent        0     -
getpeername      1     -
getpgrp          1     -
getppid          0     -
getpriority      2     p
getprotobyname   1     -
getprotobynumber 1     p
getprotoent      0     -
getpwent         0     -
getpwnam         1     -
getpwuid         1     -
getservbyname    2     p
getservbyport    2     p
getservent       0     -
getsockname      1     -
getsockopt       3     p
# given handled specially
grep             123   p+ # also tested specially
# glob handled specially
# goto handled specially
gmtime           01    -
gt               B     -
hex              01    $
index            23    p
int              01    $
ioctl            3     p
isa              B     -
join             13    p
# keys handled specially
kill             123   p
# last handled specially
lc               01    $
lcfirst          01    $
le               B     -
length           01    $
link             2     p
listen           2     p
local            1     p+
localtime        01    -
lock             1     -
log              01    $
lstat            01    $
lt               B     -
map              123   p+ # also tested specially
mkdir            @     p$
msgctl           3     p
msgget           2     p
msgrcv           5     p
msgsnd           3     p
my               123   p+ # skip with 0 args, as my() => ()
ne               B     -
# next handled specially
# not handled specially
oct              01    $
open             12345 p
opendir          2     p
or               B     -
ord              01    $
our              123   p+ # skip with 0 args, as our() => ()
pack             123   p
pipe             2     p
pop              0     1 # also tested specially
pos              01    $+
print            @     p$+
printf           @     p$+
prototype        1     +
# push handled specially
quotemeta        01    $
rand             01    -
read             34    p
readdir          1     -
# readline handled specially
readlink         01    $
# readpipe handled specially
recv             4     p
# redo handled specially
ref              01    $
rename           2     p
# XXX This code prints 'Undefined subroutine &main::require called':
#   use subs (); import subs 'require';
#   eval q[no strict 'vars'; sub { () = require; }]; print $@;
# so disable for now
#require          01    $+
reset            01    -
# return handled specially
reverse          @     p1 # also tested specially
rewinddir        1     -
rindex           23    p
rmdir            01    $
say              @     p$+
scalar           1     +
seek             3     p
seekdir          2     p
select           014   p1
semctl           4     p
semget           3     p
semop            2     p
send             34    p
setgrent         0     -
sethostent       1     -
setnetent        1     -
setpgrp          2     p
setpriority      3     p
setprotoent      1     -
setpwent         0     -
setservent       1     -
setsockopt       4     p
shift            0     1 # also tested specially
shmctl           3     p
shmget           3     p
shmread          4     p
shmwrite         4     p
shutdown         2     p
sin              01    $
sleep            01    -
socket           4     p
socketpair       5     p
sort             12    p+
# split handled specially
# splice handled specially
sprintf          123   p
sqrt             01    $
srand            01    -
stat             01    $
state            123   p1+ # skip with 0 args, as state() => ()
study            01    $+
# sub handled specially
substr           234   p
symlink          2     p
syscall          2     p
sysopen          34    p
sysread          34    p
sysseek          3     p
system           @     p1 # also tested specially
syswrite         234   p
tell             01    -
telldir          1     -
tie              234   p
tied             1     -
time             0     -
times            0     -
truncate         2     p
uc               01    $
ucfirst          01    $
umask            01    -
undef            01    +
unlink           @     p$
unpack           12    p$
# unshift handled specially
untie            1     -
utime            @     p1
# values handled specially
vec              3     p
wait             0     -
waitpid          2     p
wantarray        0     -
warn             @     p1
write            01    -
x                B     -
xor              B     p
