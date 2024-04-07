#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

# ok()/is() functions from other sources (e.g., t/test.pl) may use
# concatenation, but that is what is being tested in this file.  Hence, we
# place this file in the directory where do not use t/test.pl, and we
# write functions specially written to avoid any concatenation.

my $test = 1;

sub ok {
    my($ok, $name) = @_;

    printf "%sok %d - %s\n", ($ok ? "" : "not "), $test, $name;

    printf "# Failed test at line %d\n", (caller)[2] unless $ok;

    $test++;
    return $ok;
}

sub is {
    my($got, $expected, $name) = @_;

    my $ok = $got eq $expected;

    printf "%sok %d - %s\n", ($ok ? "" : "not "), $test, $name;

    if (!$ok) {
        printf "# Failed test at line %d\n", (caller)[2];
        printf "# got:      %s\n#expected: %s\n", $got, $expected;
    }

    $test++;
    return $ok;
}

print "1..254\n";

($a, $b, $c) = qw(foo bar);

ok("$a"     eq "foo",    "verifying assign");
ok("$a$b"   eq "foobar", "basic concatenation");
ok("$c$a$c" eq "foo",    "concatenate undef, fore and aft");

# Okay, so that wasn't very challenging.  Let's go Unicode.

{
    # bug id 20000819.004 (#3761) 

    $_ = $dx = "\x{10f2}";
    s/($dx)/$dx$1/;
    {
        ok($_ eq  "$dx$dx","bug id 20000819.004 (#3761), back");
    }

    $_ = $dx = "\x{10f2}";
    s/($dx)/$1$dx/;
    {
        ok($_ eq  "$dx$dx","bug id 20000819.004 (#3761), front");
    }

    $dx = "\x{10f2}";
    $_  = "\x{10f2}\x{10f2}";
    s/($dx)($dx)/$1$2/;
    {
        ok($_ eq  "$dx$dx","bug id 20000819.004 (#3761), front and back");
    }
}

{
    # bug id 20000901.092 (#4184)
    # test that undef left and right of utf8 results in a valid string

    my $a;
    $a .= "\x{1ff}";
    ok($a eq  "\x{1ff}", "bug id 20000901.092 (#4184), undef left");
    $a .= undef;
    ok($a eq  "\x{1ff}", "bug id 20000901.092 (#4184), undef right");
}

{
    # ID 20001020.006 (#4484)

    "x" =~ /(.)/; # unset $2

    # Without the fix this 5.7.0 would croak:
    # Modification of a read-only value attempted at ...
    eval {"$2\x{1234}"};
    ok(!$@, "bug id 20001020.006 (#4484), left");

    # For symmetry with the above.
    eval {"\x{1234}$2"};
    ok(!$@, "bug id 20001020.006 (#4484), right");

    *pi = \undef;
    # This bug existed earlier than the $2 bug, but is fixed with the same
    # patch. Without the fix this 5.7.0 would also croak:
    # Modification of a read-only value attempted at ...
    eval{"$pi\x{1234}"};
    ok(!$@, "bug id 20001020.006 (#4484), constant left");

    # For symmetry with the above.
    eval{"\x{1234}$pi"};
    ok(!$@, "bug id 20001020.006 (#4484), constant right");
}

sub beq { use bytes; $_[0] eq $_[1]; }

{
    # concat should not upgrade its arguments.
    my($l, $r, $c);

    ($l, $r, $c) = ("\x{101}", "\x{fe}", "\x{101}\x{fe}");
    ok(beq($l.$r, $c), "concat utf8 and byte");
    ok(beq($l, "\x{101}"), "right not changed after concat u+b");
    ok(beq($r, "\x{fe}"), "left not changed after concat u+b");

    ($l, $r, $c) = ("\x{fe}", "\x{101}", "\x{fe}\x{101}");
    ok(beq($l.$r, $c), "concat byte and utf8");
    ok(beq($l, "\x{fe}"), "right not changed after concat b+u");
    ok(beq($r, "\x{101}"), "left not changed after concat b+u");
}

{
    my $a; ($a .= 5) . 6;
    ok($a == 5, '($a .= 5) . 6 - present since 5.000');
}

{
    # [perl #24508] optree construction bug
    sub strfoo { "x" }
    my ($x, $y);
    $y = ($x = '' . strfoo()) . "y";
    ok( "$x,$y" eq "x,xy", 'figures out correct target' );
}

{
    # [perl #26905] "use bytes" doesn't apply byte semantics to concatenation

    my $p = "\xB6"; # PILCROW SIGN (ASCII/EBCDIC), 2bytes in UTF-X
    my $u = "\x{100}";
    my $b = pack 'a*', "\x{100}";
    my $pu = "\xB6\x{100}";
    my $up = "\x{100}\xB6";
    my $x1 = $p;
    my $y1 = $u;
    my ($x2, $x3, $x4, $y2);

    use bytes;
    ok(beq($p.$u, $p.$b), "perl #26905, left eq bytes");
    ok(beq($u.$p, $b.$p), "perl #26905, right eq bytes");
    ok(!beq($p.$u, $pu),  "perl #26905, left ne unicode");
    ok(!beq($u.$p, $up),  "perl #26905, right ne unicode");

    $x1 .= $u;
    $x2 = $p . $u;
    $y1 .= $p;
    $y2 = $u . $p;

    $x3 = $p; $x3 .= $u . $u;
    $x4 = $p . $u . $u;

    no bytes;
    ok(beq($x1, $x2), "perl #26905, left,  .= vs = . in bytes");
    ok(beq($y1, $y2), "perl #26905, right, .= vs = . in bytes");
    ok(($x1 eq $x2),  "perl #26905, left,  .= vs = . in chars");
    ok(($y1 eq $y2),  "perl #26905, right, .= vs = . in chars");
    ok(($x3 eq $x4),  "perl #26905, twin,  .= vs = . in chars");
}

{
    # Concatenation needs to preserve UTF8ness of left oper.
    my $x = eval"qr/\x{fff}/";
    ok( ord chop($x .= "\303\277") == 191, "UTF8ness preserved" );
}

{
    my $x;
    $x = "a" . "b";
    $x .= "-append-";
    ok($x eq "ab-append-", "Appending to something initialized using constant folding");
}

# non-POK consts

{
    my $a = "a";
    my $b;
    $b = $a . $a . 1;
    ok($b eq "aa1", "aa1");
    $b = 2 . $a . $a;
    ok($b eq "2aa", "2aa");
}

# [perl #124160]
package o { use overload "." => sub { $_[0] }, fallback => 1 }
$o = bless [], "o";
ok(ref(CORE::state $y = "a $o b") eq 'o',
  'state $y = "foo $bar baz" does not stringify; only concats');


# multiconcat: utf8 dest with non-utf8 args should grow dest sufficiently.
# This is mainly for valgrind or ASAN to detect problems with.

{
    my $s = "\x{100}";
    my $t = "\x80" x 1024;
    $s .= "-$t-";
    ok length($s) == 1027, "utf8 dest with non-utf8 args";
}

# target on RHS

{
    my $a = "abc";
    $a .= $a;
    ok($a eq 'abcabc', 'append self');

    $a = "abc";
    $a = $a . $a;
    ok($a eq 'abcabc', 'double self');

    $a = "abc";
    $a .= $a . $a;
    ok($a eq 'abcabcabc', 'append double self');

    $a = "abc";
    $a = "$a-$a";
    ok($a eq 'abc-abc', 'double self with const');

    $a = "abc";
    $a .= "$a-$a";
    ok($a eq 'abcabc-abc', 'append double self with const');

    $a = "abc";
    $a .= $a . $a . $a;
    ok($a eq 'abcabcabcabc', 'append triple self');

    $a = "abc";
    $a = "$a-$a=$a";
    ok($a eq 'abc-abc=abc', 'triple self with const');

    $a = "abc";
    $a .= "$a-$a=$a";
    ok($a eq 'abcabc-abc=abc', 'append triple self with const');
}

# test the sorts of optree which may (or may not) get optimised into
# a single MULTICONCAT op. It's based on a loop in t/perf/opcount.t,
# but here the loop is unwound as we would need to use concat to
# generate the expected results to compare with the actual results,
# which would rather defeat the object.

{
    my ($a1, $a2, $a3) = qw(1 2 3);
    our $pkg;
    my $lex;

    is("-", '-', '"-"');
    is("-", '-', '"-"');
    is("-", '-', '"-"');
    is("-", '-', '"-"');
    is($a1, '1', '$a1');
    is("-".$a1, '-1', '"-".$a1');
    is($a1."-", '1-', '$a1."-"');
    is("-".$a1."-", '-1-', '"-".$a1."-"');
    is("$a1", '1', '"$a1"');
    is("-$a1", '-1', '"-$a1"');
    is("$a1-", '1-', '"$a1-"');
    is("-$a1-", '-1-', '"-$a1-"');
    is($a1.$a2, '12', '$a1.$a2');
    is($a1."-".$a2, '1-2', '$a1."-".$a2');
    is("-".$a1."-".$a2, '-1-2', '"-".$a1."-".$a2');
    is($a1."-".$a2."-", '1-2-', '$a1."-".$a2."-"');
    is("-".$a1."-".$a2."-", '-1-2-', '"-".$a1."-".$a2."-"');
    is("$a1$a2", '12', '"$a1$a2"');
    is("$a1-$a2", '1-2', '"$a1-$a2"');
    is("-$a1-$a2", '-1-2', '"-$a1-$a2"');
    is("$a1-$a2-", '1-2-', '"$a1-$a2-"');
    is("-$a1-$a2-", '-1-2-', '"-$a1-$a2-"');
    is($a1.$a2.$a3, '123', '$a1.$a2.$a3');
    is($a1."-".$a2."-".$a3, '1-2-3', '$a1."-".$a2."-".$a3');
    is("-".$a1."-".$a2."-".$a3, '-1-2-3', '"-".$a1."-".$a2."-".$a3');
    is($a1."-".$a2."-".$a3."-", '1-2-3-', '$a1."-".$a2."-".$a3."-"');
    is("-".$a1."-".$a2."-".$a3."-", '-1-2-3-', '"-".$a1."-".$a2."-".$a3."-"');
    is("$a1$a2$a3", '123', '"$a1$a2$a3"');
    is("$a1-$a2-$a3", '1-2-3', '"$a1-$a2-$a3"');
    is("-$a1-$a2-$a3", '-1-2-3', '"-$a1-$a2-$a3"');
    is("$a1-$a2-$a3-", '1-2-3-', '"$a1-$a2-$a3-"');
    is("-$a1-$a2-$a3-", '-1-2-3-', '"-$a1-$a2-$a3-"');
    $pkg  = "-";
    is($pkg, '-', '$pkg  = "-"');
    $pkg  = "-";
    is($pkg, '-', '$pkg  = "-"');
    $pkg  = "-";
    is($pkg, '-', '$pkg  = "-"');
    $pkg  = "-";
    is($pkg, '-', '$pkg  = "-"');
    $pkg  = $a1;
    is($pkg, '1', '$pkg  = $a1');
    $pkg  = "-".$a1;
    is($pkg, '-1', '$pkg  = "-".$a1');
    $pkg  = $a1."-";
    is($pkg, '1-', '$pkg  = $a1."-"');
    $pkg  = "-".$a1."-";
    is($pkg, '-1-', '$pkg  = "-".$a1."-"');
    $pkg  = "$a1";
    is($pkg, '1', '$pkg  = "$a1"');
    $pkg  = "-$a1";
    is($pkg, '-1', '$pkg  = "-$a1"');
    $pkg  = "$a1-";
    is($pkg, '1-', '$pkg  = "$a1-"');
    $pkg  = "-$a1-";
    is($pkg, '-1-', '$pkg  = "-$a1-"');
    $pkg  = $a1.$a2;
    is($pkg, '12', '$pkg  = $a1.$a2');
    $pkg  = $a1."-".$a2;
    is($pkg, '1-2', '$pkg  = $a1."-".$a2');
    $pkg  = "-".$a1."-".$a2;
    is($pkg, '-1-2', '$pkg  = "-".$a1."-".$a2');
    $pkg  = $a1."-".$a2."-";
    is($pkg, '1-2-', '$pkg  = $a1."-".$a2."-"');
    $pkg  = "-".$a1."-".$a2."-";
    is($pkg, '-1-2-', '$pkg  = "-".$a1."-".$a2."-"');
    $pkg  = "$a1$a2";
    is($pkg, '12', '$pkg  = "$a1$a2"');
    $pkg  = "$a1-$a2";
    is($pkg, '1-2', '$pkg  = "$a1-$a2"');
    $pkg  = "-$a1-$a2";
    is($pkg, '-1-2', '$pkg  = "-$a1-$a2"');
    $pkg  = "$a1-$a2-";
    is($pkg, '1-2-', '$pkg  = "$a1-$a2-"');
    $pkg  = "-$a1-$a2-";
    is($pkg, '-1-2-', '$pkg  = "-$a1-$a2-"');
    $pkg  = $a1.$a2.$a3;
    is($pkg, '123', '$pkg  = $a1.$a2.$a3');
    $pkg  = $a1."-".$a2."-".$a3;
    is($pkg, '1-2-3', '$pkg  = $a1."-".$a2."-".$a3');
    $pkg  = "-".$a1."-".$a2."-".$a3;
    is($pkg, '-1-2-3', '$pkg  = "-".$a1."-".$a2."-".$a3');
    $pkg  = $a1."-".$a2."-".$a3."-";
    is($pkg, '1-2-3-', '$pkg  = $a1."-".$a2."-".$a3."-"');
    $pkg  = "-".$a1."-".$a2."-".$a3."-";
    is($pkg, '-1-2-3-', '$pkg  = "-".$a1."-".$a2."-".$a3."-"');
    $pkg  = "$a1$a2$a3";
    is($pkg, '123', '$pkg  = "$a1$a2$a3"');
    $pkg  = "$a1-$a2-$a3";
    is($pkg, '1-2-3', '$pkg  = "$a1-$a2-$a3"');
    $pkg  = "-$a1-$a2-$a3";
    is($pkg, '-1-2-3', '$pkg  = "-$a1-$a2-$a3"');
    $pkg  = "$a1-$a2-$a3-";
    is($pkg, '1-2-3-', '$pkg  = "$a1-$a2-$a3-"');
    $pkg  = "-$a1-$a2-$a3-";
    is($pkg, '-1-2-3-', '$pkg  = "-$a1-$a2-$a3-"');
    $pkg = 'P';
    $pkg .= "-";
    is($pkg, 'P-', '$pkg .= "-"');
    $pkg = 'P';
    $pkg .= "-";
    is($pkg, 'P-', '$pkg .= "-"');
    $pkg = 'P';
    $pkg .= "-";
    is($pkg, 'P-', '$pkg .= "-"');
    $pkg = 'P';
    $pkg .= "-";
    is($pkg, 'P-', '$pkg .= "-"');
    $pkg = 'P';
    $pkg .= $a1;
    is($pkg, 'P1', '$pkg .= $a1');
    $pkg = 'P';
    $pkg .= "-".$a1;
    is($pkg, 'P-1', '$pkg .= "-".$a1');
    $pkg = 'P';
    $pkg .= $a1."-";
    is($pkg, 'P1-', '$pkg .= $a1."-"');
    $pkg = 'P';
    $pkg .= "-".$a1."-";
    is($pkg, 'P-1-', '$pkg .= "-".$a1."-"');
    $pkg = 'P';
    $pkg .= "$a1";
    is($pkg, 'P1', '$pkg .= "$a1"');
    $pkg = 'P';
    $pkg .= "-$a1";
    is($pkg, 'P-1', '$pkg .= "-$a1"');
    $pkg = 'P';
    $pkg .= "$a1-";
    is($pkg, 'P1-', '$pkg .= "$a1-"');
    $pkg = 'P';
    $pkg .= "-$a1-";
    is($pkg, 'P-1-', '$pkg .= "-$a1-"');
    $pkg = 'P';
    $pkg .= $a1.$a2;
    is($pkg, 'P12', '$pkg .= $a1.$a2');
    $pkg = 'P';
    $pkg .= $a1."-".$a2;
    is($pkg, 'P1-2', '$pkg .= $a1."-".$a2');
    $pkg = 'P';
    $pkg .= "-".$a1."-".$a2;
    is($pkg, 'P-1-2', '$pkg .= "-".$a1."-".$a2');
    $pkg = 'P';
    $pkg .= $a1."-".$a2."-";
    is($pkg, 'P1-2-', '$pkg .= $a1."-".$a2."-"');
    $pkg = 'P';
    $pkg .= "-".$a1."-".$a2."-";
    is($pkg, 'P-1-2-', '$pkg .= "-".$a1."-".$a2."-"');
    $pkg = 'P';
    $pkg .= "$a1$a2";
    is($pkg, 'P12', '$pkg .= "$a1$a2"');
    $pkg = 'P';
    $pkg .= "$a1-$a2";
    is($pkg, 'P1-2', '$pkg .= "$a1-$a2"');
    $pkg = 'P';
    $pkg .= "-$a1-$a2";
    is($pkg, 'P-1-2', '$pkg .= "-$a1-$a2"');
    $pkg = 'P';
    $pkg .= "$a1-$a2-";
    is($pkg, 'P1-2-', '$pkg .= "$a1-$a2-"');
    $pkg = 'P';
    $pkg .= "-$a1-$a2-";
    is($pkg, 'P-1-2-', '$pkg .= "-$a1-$a2-"');
    $pkg = 'P';
    $pkg .= $a1.$a2.$a3;
    is($pkg, 'P123', '$pkg .= $a1.$a2.$a3');
    $pkg = 'P';
    $pkg .= $a1."-".$a2."-".$a3;
    is($pkg, 'P1-2-3', '$pkg .= $a1."-".$a2."-".$a3');
    $pkg = 'P';
    $pkg .= "-".$a1."-".$a2."-".$a3;
    is($pkg, 'P-1-2-3', '$pkg .= "-".$a1."-".$a2."-".$a3');
    $pkg = 'P';
    $pkg .= $a1."-".$a2."-".$a3."-";
    is($pkg, 'P1-2-3-', '$pkg .= $a1."-".$a2."-".$a3."-"');
    $pkg = 'P';
    $pkg .= "-".$a1."-".$a2."-".$a3."-";
    is($pkg, 'P-1-2-3-', '$pkg .= "-".$a1."-".$a2."-".$a3."-"');
    $pkg = 'P';
    $pkg .= "$a1$a2$a3";
    is($pkg, 'P123', '$pkg .= "$a1$a2$a3"');
    $pkg = 'P';
    $pkg .= "$a1-$a2-$a3";
    is($pkg, 'P1-2-3', '$pkg .= "$a1-$a2-$a3"');
    $pkg = 'P';
    $pkg .= "-$a1-$a2-$a3";
    is($pkg, 'P-1-2-3', '$pkg .= "-$a1-$a2-$a3"');
    $pkg = 'P';
    $pkg .= "$a1-$a2-$a3-";
    is($pkg, 'P1-2-3-', '$pkg .= "$a1-$a2-$a3-"');
    $pkg = 'P';
    $pkg .= "-$a1-$a2-$a3-";
    is($pkg, 'P-1-2-3-', '$pkg .= "-$a1-$a2-$a3-"');
    $lex  = "-";
    is($lex, '-', '$lex  = "-"');
    $lex  = "-";
    is($lex, '-', '$lex  = "-"');
    $lex  = "-";
    is($lex, '-', '$lex  = "-"');
    $lex  = "-";
    is($lex, '-', '$lex  = "-"');
    $lex  = $a1;
    is($lex, '1', '$lex  = $a1');
    $lex  = "-".$a1;
    is($lex, '-1', '$lex  = "-".$a1');
    $lex  = $a1."-";
    is($lex, '1-', '$lex  = $a1."-"');
    $lex  = "-".$a1."-";
    is($lex, '-1-', '$lex  = "-".$a1."-"');
    $lex  = "$a1";
    is($lex, '1', '$lex  = "$a1"');
    $lex  = "-$a1";
    is($lex, '-1', '$lex  = "-$a1"');
    $lex  = "$a1-";
    is($lex, '1-', '$lex  = "$a1-"');
    $lex  = "-$a1-";
    is($lex, '-1-', '$lex  = "-$a1-"');
    $lex  = $a1.$a2;
    is($lex, '12', '$lex  = $a1.$a2');
    $lex  = $a1."-".$a2;
    is($lex, '1-2', '$lex  = $a1."-".$a2');
    $lex  = "-".$a1."-".$a2;
    is($lex, '-1-2', '$lex  = "-".$a1."-".$a2');
    $lex  = $a1."-".$a2."-";
    is($lex, '1-2-', '$lex  = $a1."-".$a2."-"');
    $lex  = "-".$a1."-".$a2."-";
    is($lex, '-1-2-', '$lex  = "-".$a1."-".$a2."-"');
    $lex  = "$a1$a2";
    is($lex, '12', '$lex  = "$a1$a2"');
    $lex  = "$a1-$a2";
    is($lex, '1-2', '$lex  = "$a1-$a2"');
    $lex  = "-$a1-$a2";
    is($lex, '-1-2', '$lex  = "-$a1-$a2"');
    $lex  = "$a1-$a2-";
    is($lex, '1-2-', '$lex  = "$a1-$a2-"');
    $lex  = "-$a1-$a2-";
    is($lex, '-1-2-', '$lex  = "-$a1-$a2-"');
    $lex  = $a1.$a2.$a3;
    is($lex, '123', '$lex  = $a1.$a2.$a3');
    $lex  = $a1."-".$a2."-".$a3;
    is($lex, '1-2-3', '$lex  = $a1."-".$a2."-".$a3');
    $lex  = "-".$a1."-".$a2."-".$a3;
    is($lex, '-1-2-3', '$lex  = "-".$a1."-".$a2."-".$a3');
    $lex  = $a1."-".$a2."-".$a3."-";
    is($lex, '1-2-3-', '$lex  = $a1."-".$a2."-".$a3."-"');
    $lex  = "-".$a1."-".$a2."-".$a3."-";
    is($lex, '-1-2-3-', '$lex  = "-".$a1."-".$a2."-".$a3."-"');
    $lex  = "$a1$a2$a3";
    is($lex, '123', '$lex  = "$a1$a2$a3"');
    $lex  = "$a1-$a2-$a3";
    is($lex, '1-2-3', '$lex  = "$a1-$a2-$a3"');
    $lex  = "-$a1-$a2-$a3";
    is($lex, '-1-2-3', '$lex  = "-$a1-$a2-$a3"');
    $lex  = "$a1-$a2-$a3-";
    is($lex, '1-2-3-', '$lex  = "$a1-$a2-$a3-"');
    $lex  = "-$a1-$a2-$a3-";
    is($lex, '-1-2-3-', '$lex  = "-$a1-$a2-$a3-"');
    $lex = 'L';
    $lex .= "-";
    is($lex, 'L-', '$lex .= "-"');
    $lex = 'L';
    $lex .= "-";
    is($lex, 'L-', '$lex .= "-"');
    $lex = 'L';
    $lex .= "-";
    is($lex, 'L-', '$lex .= "-"');
    $lex = 'L';
    $lex .= "-";
    is($lex, 'L-', '$lex .= "-"');
    $lex = 'L';
    $lex .= $a1;
    is($lex, 'L1', '$lex .= $a1');
    $lex = 'L';
    $lex .= "-".$a1;
    is($lex, 'L-1', '$lex .= "-".$a1');
    $lex = 'L';
    $lex .= $a1."-";
    is($lex, 'L1-', '$lex .= $a1."-"');
    $lex = 'L';
    $lex .= "-".$a1."-";
    is($lex, 'L-1-', '$lex .= "-".$a1."-"');
    $lex = 'L';
    $lex .= "$a1";
    is($lex, 'L1', '$lex .= "$a1"');
    $lex = 'L';
    $lex .= "-$a1";
    is($lex, 'L-1', '$lex .= "-$a1"');
    $lex = 'L';
    $lex .= "$a1-";
    is($lex, 'L1-', '$lex .= "$a1-"');
    $lex = 'L';
    $lex .= "-$a1-";
    is($lex, 'L-1-', '$lex .= "-$a1-"');
    $lex = 'L';
    $lex .= $a1.$a2;
    is($lex, 'L12', '$lex .= $a1.$a2');
    $lex = 'L';
    $lex .= $a1."-".$a2;
    is($lex, 'L1-2', '$lex .= $a1."-".$a2');
    $lex = 'L';
    $lex .= "-".$a1."-".$a2;
    is($lex, 'L-1-2', '$lex .= "-".$a1."-".$a2');
    $lex = 'L';
    $lex .= $a1."-".$a2."-";
    is($lex, 'L1-2-', '$lex .= $a1."-".$a2."-"');
    $lex = 'L';
    $lex .= "-".$a1."-".$a2."-";
    is($lex, 'L-1-2-', '$lex .= "-".$a1."-".$a2."-"');
    $lex = 'L';
    $lex .= "$a1$a2";
    is($lex, 'L12', '$lex .= "$a1$a2"');
    $lex = 'L';
    $lex .= "$a1-$a2";
    is($lex, 'L1-2', '$lex .= "$a1-$a2"');
    $lex = 'L';
    $lex .= "-$a1-$a2";
    is($lex, 'L-1-2', '$lex .= "-$a1-$a2"');
    $lex = 'L';
    $lex .= "$a1-$a2-";
    is($lex, 'L1-2-', '$lex .= "$a1-$a2-"');
    $lex = 'L';
    $lex .= "-$a1-$a2-";
    is($lex, 'L-1-2-', '$lex .= "-$a1-$a2-"');
    $lex = 'L';
    $lex .= $a1.$a2.$a3;
    is($lex, 'L123', '$lex .= $a1.$a2.$a3');
    $lex = 'L';
    $lex .= $a1."-".$a2."-".$a3;
    is($lex, 'L1-2-3', '$lex .= $a1."-".$a2."-".$a3');
    $lex = 'L';
    $lex .= "-".$a1."-".$a2."-".$a3;
    is($lex, 'L-1-2-3', '$lex .= "-".$a1."-".$a2."-".$a3');
    $lex = 'L';
    $lex .= $a1."-".$a2."-".$a3."-";
    is($lex, 'L1-2-3-', '$lex .= $a1."-".$a2."-".$a3."-"');
    $lex = 'L';
    $lex .= "-".$a1."-".$a2."-".$a3."-";
    is($lex, 'L-1-2-3-', '$lex .= "-".$a1."-".$a2."-".$a3."-"');
    $lex = 'L';
    $lex .= "$a1$a2$a3";
    is($lex, 'L123', '$lex .= "$a1$a2$a3"');
    $lex = 'L';
    $lex .= "$a1-$a2-$a3";
    is($lex, 'L1-2-3', '$lex .= "$a1-$a2-$a3"');
    $lex = 'L';
    $lex .= "-$a1-$a2-$a3";
    is($lex, 'L-1-2-3', '$lex .= "-$a1-$a2-$a3"');
    $lex = 'L';
    $lex .= "$a1-$a2-$a3-";
    is($lex, 'L1-2-3-', '$lex .= "$a1-$a2-$a3-"');
    $lex = 'L';
    $lex .= "-$a1-$a2-$a3-";
    is($lex, 'L-1-2-3-', '$lex .= "-$a1-$a2-$a3-"');
    {
        my $l = "-";
        is($l, '-', 'my $l = "-"');
    }
    {
        my $l = "-";
        is($l, '-', 'my $l = "-"');
    }
    {
        my $l = "-";
        is($l, '-', 'my $l = "-"');
    }
    {
        my $l = "-";
        is($l, '-', 'my $l = "-"');
    }
    {
        my $l = $a1;
        is($l, '1', 'my $l = $a1');
    }
    {
        my $l = "-".$a1;
        is($l, '-1', 'my $l = "-".$a1');
    }
    {
        my $l = $a1."-";
        is($l, '1-', 'my $l = $a1."-"');
    }
    {
        my $l = "-".$a1."-";
        is($l, '-1-', 'my $l = "-".$a1."-"');
    }
    {
        my $l = "$a1";
        is($l, '1', 'my $l = "$a1"');
    }
    {
        my $l = "-$a1";
        is($l, '-1', 'my $l = "-$a1"');
    }
    {
        my $l = "$a1-";
        is($l, '1-', 'my $l = "$a1-"');
    }
    {
        my $l = "-$a1-";
        is($l, '-1-', 'my $l = "-$a1-"');
    }
    {
        my $l = $a1.$a2;
        is($l, '12', 'my $l = $a1.$a2');
    }
    {
        my $l = $a1."-".$a2;
        is($l, '1-2', 'my $l = $a1."-".$a2');
    }
    {
        my $l = "-".$a1."-".$a2;
        is($l, '-1-2', 'my $l = "-".$a1."-".$a2');
    }
    {
        my $l = $a1."-".$a2."-";
        is($l, '1-2-', 'my $l = $a1."-".$a2."-"');
    }
    {
        my $l = "-".$a1."-".$a2."-";
        is($l, '-1-2-', 'my $l = "-".$a1."-".$a2."-"');
    }
    {
        my $l = "$a1$a2";
        is($l, '12', 'my $l = "$a1$a2"');
    }
    {
        my $l = "$a1-$a2";
        is($l, '1-2', 'my $l = "$a1-$a2"');
    }
    {
        my $l = "-$a1-$a2";
        is($l, '-1-2', 'my $l = "-$a1-$a2"');
    }
    {
        my $l = "$a1-$a2-";
        is($l, '1-2-', 'my $l = "$a1-$a2-"');
    }
    {
        my $l = "-$a1-$a2-";
        is($l, '-1-2-', 'my $l = "-$a1-$a2-"');
    }
    {
        my $l = $a1.$a2.$a3;
        is($l, '123', 'my $l = $a1.$a2.$a3');
    }
    {
        my $l = $a1."-".$a2."-".$a3;
        is($l, '1-2-3', 'my $l = $a1."-".$a2."-".$a3');
    }
    {
        my $l = "-".$a1."-".$a2."-".$a3;
        is($l, '-1-2-3', 'my $l = "-".$a1."-".$a2."-".$a3');
    }
    {
        my $l = $a1."-".$a2."-".$a3."-";
        is($l, '1-2-3-', 'my $l = $a1."-".$a2."-".$a3."-"');
    }
    {
        my $l = "-".$a1."-".$a2."-".$a3."-";
        is($l, '-1-2-3-', 'my $l = "-".$a1."-".$a2."-".$a3."-"');
    }
    {
        my $l = "$a1$a2$a3";
        is($l, '123', 'my $l = "$a1$a2$a3"');
    }
    {
        my $l = "$a1-$a2-$a3";
        is($l, '1-2-3', 'my $l = "$a1-$a2-$a3"');
    }
    {
        my $l = "-$a1-$a2-$a3";
        is($l, '-1-2-3', 'my $l = "-$a1-$a2-$a3"');
    }
    {
        my $l = "$a1-$a2-$a3-";
        is($l, '1-2-3-', 'my $l = "$a1-$a2-$a3-"');
    }
    {
        my $l = "-$a1-$a2-$a3-";
        is($l, '-1-2-3-', 'my $l = "-$a1-$a2-$a3-"');
    }
}

# multiconcat optimises away scalar assign, and is responsible
# for handling the assign itself. If the LHS is something weird,
# make sure it's handled ok

{
    my $a = 'a';
    my $b = 'b';
    my $o = 'o';

    my $re = qr/abc/;
    $$re = $a . $b;
    is($$re, "ab", '$$re = $a . $b');

    #passing a hash elem to a sub creates a PVLV
    my $s = sub { $_[0] = $a . $b; };
    my %h;
    $s->($h{foo});
    is($h{foo}, "ab", "PVLV");

    # assigning a string to a typeglob creates an alias
    $Foo = 'myfoo';
    *Bar = ("F" . $o . $o);
    is($Bar, "myfoo", '*Bar = "Foo"');

    # while that same typeglob also appearing on the RHS returns
    # a stringified value

    package QPR {
        ${'*QPR::Bar*QPR::BarBaz'} = 'myfoobarbaz';
        *Bar = (*Bar  . *Bar . "Baz");
        ::is($Bar, "myfoobarbaz", '*Bar =  (*Bar  . *Bar . "Baz")');
    }
}

# distinguish between '=' and  '.=' where the LHS has the OPf_MOD flag

{
    my $foo = "foo";
    my $a . $foo; # weird but legal
    is($a, '', 'my $a . $foo');
    my $b; $b .= $foo;
    is($b, 'foo', 'my $b; $b .= $foo');
}

# distinguish between nested appends and concats; the former is
# affected by the change of value of the target on each concat.
# This is why multiconcat shouldn't be used in that case

{
    my $a = "a";
    (($a .= $a) .= $a) .= $a;
    is($a, "aaaaaaaa", '(($a .= $a) .= $a) .= $a;');
}

# check everything works ok near the max arg size of a multiconcat

{
    my @a = map "<$_>", 0..99;
    for my $i (60..68) { # check each side of 64 threshold
        my $c = join '.', map "\$a[$_]", 0..$i;
        my $got = eval $c or die $@;
        my $empty = ''; # don't use a const string in case join'' ever
                        # gets optimised into a multiconcat
        my $expected = join $empty, @a[0..$i];
        is($got, $expected, "long concat chain $i");
    }
}

# RT #132646
# with adjacent consts, the second const is treated as an arg rather than a
# consts. Make sure this doesn't exceeed the maximum allowed number of
# args
{
    my $x = 'X';
    my $got =
          'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        . 'A' . $x . 'B' . 'C' . $x . 'D'
        ;
    is ($got,
        "AXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXDAXBCXD",
        "RT #132646");
}

# RT #132595
# multiconcat shouldn't affect the order of arg evaluation
package RT132595 {
    my $a = "a";
    my $i = 0;
    sub TIESCALAR { bless({}, $_[0]) }
    sub FETCH { ++$i; $a = "b".$i; "c".$i }
    my $t;
    tie $t, "RT132595";
    my $res = $a.$t.$a.$t;
    ::is($res, "b1c1b1c2", "RT #132595");
}

# RT #133441
# multiconcat wasn't seeing a mutator as a mutator
{
    my ($a, $b)  = qw(a b);
    ($a = 'A'.$b) .= 'c';
    is($a, "Abc", "RT #133441");
}
