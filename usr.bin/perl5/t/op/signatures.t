#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use warnings;
use strict;

our $a = 123;
our $z;

{
    no warnings "illegalproto";
    sub t000 ($a) { $a || "z" }
    is prototype(\&t000), "\$a", "(\$a) interpreted as protoype when not enabled";
    is &t000(456), 123, "(\$a) not signature when not enabled";
    is $a, 123;
}

eval "#line 8 foo\nsub t004 :method (\$a) { }";
like $@, qr{syntax error at foo line 8}, "error when not enabled 1";

eval "#line 8 foo\nsub t005 (\$) (\$a) { }";
like $@, qr{syntax error at foo line 8}, "error when not enabled 2";


use feature "signatures";

sub t001 { $a || "z" }
is prototype(\&t001), undef;
is eval("t001()"), 123;
is eval("t001(456)"), 123;
is eval("t001(456, 789)"), 123;
is $a, 123;

sub _create_mismatch_regexp {
    my ($funcname, $got, $expected, $flexible_str) = @_;

    my $many_few_str = ($got > $expected) ? 'many' : 'few';

    $flexible_str //= q<>;

    return qr/\AToo $many_few_str arguments for subroutine '$funcname' \(got $got; expected $flexible_str$expected\) at \(eval \d+\) line 1\.\n\z/;
}

sub _create_flexible_mismatch_regexp {
    my ($funcname, $got, $expected) = @_;

    my $flexible_str = ($got > $expected) ? 'at most' : 'at least';
    $flexible_str .= q< >;

    return _create_mismatch_regexp($funcname, $got, $expected, $flexible_str);
}

sub t002 () { $a || "z" }
is prototype(\&t002), undef;
is eval("t002()"), 123;
is eval("t002(456)"), undef;
like $@, _create_mismatch_regexp('main::t002', 1, 0);
is eval("t002(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t002', 2, 0);
is $a, 123;

sub t003 ( ) { $a || "z" }
is prototype(\&t003), undef;
is eval("t003()"), 123;
is eval("t003(456)"), undef;
like $@, _create_mismatch_regexp('main::t003', 1, 0);
is eval("t003(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t003', 2, 0);
is $a, 123;

sub t006 ($a) { $a || "z" }
is prototype(\&t006), undef;
is eval("t006()"), undef;
like $@, _create_mismatch_regexp('main::t006', 0, 1);
is eval("t006(0)"), "z";
is eval("t006(456)"), 456;
is eval("t006(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t006', 2, 1);
is eval("t006(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t006', 3, 1);
is $a, 123;

sub t007 ($a, $b) { $a.$b }
is prototype(\&t007), undef;
is eval("t007()"), undef;
like $@, _create_mismatch_regexp('main::t007', 0, 2);
is eval("t007(456)"), undef;
like $@, _create_mismatch_regexp('main::t007', 1, 2);
is eval("t007(456, 789)"), "456789";
is eval("t007(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t007', 3, 2);
is eval("t007(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t007', 4, 2);
is $a, 123;

sub t008 ($a, $b, $c) { $a.$b.$c }
is prototype(\&t008), undef;
is eval("t008()"), undef;
like $@, _create_mismatch_regexp('main::t008', 0, 3);
is eval("t008(456)"), undef;
like $@, _create_mismatch_regexp('main::t008', 1, 3);
is eval("t008(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t008', 2, 3);
is eval("t008(456, 789, 987)"), "456789987";
is eval("t008(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t008', 4, 3);
is $a, 123;

sub t009 ($abc, $def) { $abc.$def }
is prototype(\&t009), undef;
is eval("t009()"), undef;
like $@, _create_mismatch_regexp('main::t009', 0, 2);
is eval("t009(456)"), undef;
like $@, _create_mismatch_regexp('main::t009', 1, 2);
is eval("t009(456, 789)"), "456789";
is eval("t009(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t009', 3, 2);
is eval("t009(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t009', 4, 2);
is $a, 123;

sub t010 ($a, $) { $a || "z" }
is prototype(\&t010), undef;
is eval("t010()"), undef;
like $@, _create_mismatch_regexp('main::t010', 0, 2);
is eval("t010(456)"), undef;
like $@, _create_mismatch_regexp('main::t010', 1, 2);
is eval("t010(0, 789)"), "z";
is eval("t010(456, 789)"), 456;
is eval("t010(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t010', 3, 2);
is eval("t010(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t010', 4, 2);
is $a, 123;

sub t011 ($, $a) { $a || "z" }
is prototype(\&t011), undef;
is eval("t011()"), undef;
like $@, _create_mismatch_regexp('main::t011', 0, 2);
is eval("t011(456)"), undef;
like $@, _create_mismatch_regexp('main::t011', 1, 2);
is eval("t011(456, 0)"), "z";
is eval("t011(456, 789)"), 789;
is eval("t011(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t011', 3, 2);
is eval("t011(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t011', 4, 2);
is $a, 123;

sub t012 ($, $) { $a || "z" }
is prototype(\&t012), undef;
is eval("t012()"), undef;
like $@, _create_mismatch_regexp('main::t012', 0, 2);
is eval("t012(456)"), undef;
like $@, _create_mismatch_regexp('main::t012', 1, 2);
is eval("t012(0, 789)"), 123;
is eval("t012(456, 789)"), 123;
is eval("t012(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t012', 3, 2);
is eval("t012(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t012', 4, 2);
is $a, 123;

sub t013 ($) { $a || "z" }
is prototype(\&t013), undef;
is eval("t013()"), undef;
like $@, _create_mismatch_regexp('main::t013', 0, 1);
is eval("t013(0)"), 123;
is eval("t013(456)"), 123;
is eval("t013(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t013', 2, 1);
is eval("t013(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t013', 3, 1);
is eval("t013(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t013', 4, 1);
is $a, 123;

sub t014 ($a = 222) { $a // "z" }
is prototype(\&t014), undef;
is eval("t014()"), 222;
is eval("t014(0)"), 0;
is eval("t014(undef)"), "z";
is eval("t014(456)"), 456;
is eval("t014(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t014', 2, 1);
is eval("t014(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t014', 3, 1);
is $a, 123;

sub t015 ($a = undef) { $a // "z" }
is prototype(\&t015), undef;
is eval("t015()"), "z";
is eval("t015(0)"), 0;
is eval("t015(undef)"), "z";
is eval("t015(456)"), 456;
is eval("t015(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t015', 2, 1);
is eval("t015(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t015', 3, 1);
is $a, 123;

sub t016 ($a = do { $z++; 222 }) { $a // "z" }
$z = 0;
is prototype(\&t016), undef;
is eval("t016()"), 222;
is $z, 1;
is eval("t016(0)"), 0;
is eval("t016(undef)"), "z";
is eval("t016(456)"), 456;
is eval("t016(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t016', 2, 1);
is eval("t016(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t016', 3, 1);
is $z, 1;
is eval("t016()"), 222;
is $z, 2;
is $a, 123;

sub t018 { join("/", @_) }
sub t017 ($p = t018 222, $a = 333) { $p // "z" }
is prototype(\&t017), undef;
is eval("t017()"), "222/333";
is $a, 333;
$a = 123;
is eval("t017(0)"), 0;
is eval("t017(undef)"), "z";
is eval("t017(456)"), 456;
is eval("t017(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t017', 2, 1);
is eval("t017(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t017', 3, 1);
is $a, 123;

sub t019 ($p = 222, $a = 333) { "$p/$a" }
is prototype(\&t019), undef;
is eval("t019()"), "222/333";
is eval("t019(0)"), "0/333";
is eval("t019(456)"), "456/333";
is eval("t019(456, 789)"), "456/789";
is eval("t019(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t019', 3, 2);
is $a, 123;

sub t020 :prototype($) { $_[0]."z" }
sub t021 ($p = t020 222, $a = 333) { "$p/$a" }
is prototype(\&t021), undef;
is eval("t021()"), "222z/333";
is eval("t021(0)"), "0/333";
is eval("t021(456)"), "456/333";
is eval("t021(456, 789)"), "456/789";
is eval("t021(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t021', 3, 2);
is $a, 123;

sub t022 ($p = do { $z += 10; 222 }, $a = do { $z++; 333 }) { "$p/$a" }
$z = 0;
is prototype(\&t022), undef;
is eval("t022()"), "222/333";
is $z, 11;
is eval("t022(0)"), "0/333";
is $z, 12;
is eval("t022(456)"), "456/333";
is $z, 13;
is eval("t022(456, 789)"), "456/789";
is eval("t022(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t022', 3, 2);
is $z, 13;
is $a, 123;

sub t023 ($a = sub { $_[0]."z" }) { $a->("a")."y" }
is prototype(\&t023), undef;
is eval("t023()"), "azy";
is eval("t023(sub { \"x\".\$_[0].\"x\" })"), "xaxy";
is eval("t023(sub { \"x\".\$_[0].\"x\" }, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t023', 2, 1);
is $a, 123;

sub t036 ($a = $a."x") { $a."y" }
is prototype(\&t036), undef;
is eval("t036()"), "123xy";
is eval("t036(0)"), "0y";
is eval("t036(456)"), "456y";
is eval("t036(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t036', 2, 1);
is $a, 123;

sub t120 ($a = $_) { $a // "z" }
is prototype(\&t120), undef;
$_ = "___";
is eval("t120()"), "___";
$_ = "___";
is eval("t120(undef)"), "z";
$_ = "___";
is eval("t120(0)"), 0;
$_ = "___";
is eval("t120(456)"), 456;
$_ = "___";
is eval("t120(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t120', 2, 1);
is $a, 123;

sub t121 ($a = caller) { $a // "z" }
is prototype(\&t121), undef;
is eval("t121()"), "main";
is eval("t121(undef)"), "z";
is eval("t121(0)"), 0;
is eval("t121(456)"), 456;
is eval("t121(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t121', 2, 1);
is eval("package T121::Z; ::t121()"), "T121::Z";
is eval("package T121::Z; ::t121(undef)"), "z";
is eval("package T121::Z; ::t121(0)"), 0;
is eval("package T121::Z; ::t121(456)"), 456;
is eval("package T121::Z; ::t121(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t121', 2, 1);
is $a, 123;

sub t129 ($a = return 222) { $a."x" }
is prototype(\&t129), undef;
is eval("t129()"), "222";
is eval("t129(0)"), "0x";
is eval("t129(456)"), "456x";
is eval("t129(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t129', 2, 1);
is $a, 123;

use feature "current_sub";
sub t122 ($c = 5, $r = $c > 0 ? __SUB__->($c - 1) : "") { $c.$r }
is prototype(\&t122), undef;
is eval("t122()"), "543210";
is eval("t122(0)"), "0";
is eval("t122(1)"), "10";
is eval("t122(5)"), "543210";
is eval("t122(5, 789)"), "5789";
is eval("t122(5, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t122', 3, 2);
is $a, 123;

sub t123 ($list = wantarray) { $list ? "list" : "scalar" }
is prototype(\&t123), undef;
is eval("scalar(t123())"), "scalar";
is eval("(t123())[0]"), "list";
is eval("scalar(t123(0))"), "scalar";
is eval("(t123(0))[0]"), "scalar";
is eval("scalar(t123(1))"), "list";
is eval("(t123(1))[0]"), "list";
is eval("t123(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t123', 2, 1);
is $a, 123;

sub t124 ($b = (local $a = $a + 1)) { "$a/$b" }
is prototype(\&t124), undef;
is eval("t124()"), "124/124";
is $a, 123;
is eval("t124(456)"), "123/456";
is $a, 123;
is eval("t124(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t124', 2, 1);
is $a, 123;

sub t125 ($c = (our $t125_counter)++) { $c }
is prototype(\&t125), undef;
is eval("t125()"), 0;
is eval("t125()"), 1;
is eval("t125()"), 2;
is eval("t125(456)"), 456;
is eval("t125(789)"), 789;
is eval("t125()"), 3;
is eval("t125()"), 4;
is eval("t125(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t125', 2, 1);
is $a, 123;

use feature "state";
sub t126 ($c = (state $s = $z++)) { $c }
is prototype(\&t126), undef;
$z = 222;
is eval("t126(456)"), 456;
is $z, 222;
is eval("t126()"), 222;
is $z, 223;
is eval("t126(456)"), 456;
is $z, 223;
is eval("t126()"), 222;
is $z, 223;
is eval("t126(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t126', 2, 1);
is $z, 223;
is $a, 123;

sub t127 ($c = do { state $s = $z++; $s++ }) { $c }
is prototype(\&t127), undef;
$z = 222;
is eval("t127(456)"), 456;
is $z, 222;
is eval("t127()"), 222;
is $z, 223;
is eval("t127()"), 223;
is eval("t127()"), 224;
is $z, 223;
is eval("t127(456)"), 456;
is eval("t127(789)"), 789;
is eval("t127()"), 225;
is eval("t127()"), 226;
is eval("t127(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t127', 2, 1);
is $z, 223;
is $a, 123;

sub t037 ($a = 222, $b = $a."x") { "$a/$b" }
is prototype(\&t037), undef;
is eval("t037()"), "222/222x";
is eval("t037(0)"), "0/0x";
is eval("t037(456)"), "456/456x";
is eval("t037(456, 789)"), "456/789";
is eval("t037(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t037', 3, 2);
is $a, 123;

sub t128 ($a = 222, $b = ($a = 333)) { "$a/$b" }
is prototype(\&t128), undef;
is eval("t128()"), "333/333";
is eval("t128(0)"), "333/333";
is eval("t128(456)"), "333/333";
is eval("t128(456, 789)"), "456/789";
is eval("t128(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t128', 3, 2);
is $a, 123;

sub t130 { join(",", @_).";".scalar(@_) }
{
    no warnings 'experimental::args_array_with_signatures';
    sub t131 ($a = 222, $b = goto &t130) { "$a/$b" }
}
is prototype(\&t131), undef;
is eval("t131()"), ";0";
is eval("t131(0)"), "0;1";
is eval("t131(456)"), "456;1";
is eval("t131(456, 789)"), "456/789";
is eval("t131(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t131', 3, 2);
is $a, 123;

eval "#line 8 foo\nsub t024 (\$a =) { }";
is $@,
    qq{Optional parameter lacks default expression at foo line 8, near "=) "\n};

sub t025 ($ = undef) { $a // "z" }
is prototype(\&t025), undef;
is eval("t025()"), 123;
is eval("t025(0)"), 123;
is eval("t025(456)"), 123;
is eval("t025(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t025', 2, 1);
is eval("t025(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t025', 3, 1);
is eval("t025(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t025', 4, 1);
is $a, 123;

sub t026 ($ = 222) { $a // "z" }
is prototype(\&t026), undef;
is eval("t026()"), 123;
is eval("t026(0)"), 123;
is eval("t026(456)"), 123;
is eval("t026(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t026', 2, 1);
is eval("t026(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t026', 3, 1);
is eval("t026(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t026', 4, 1);
is $a, 123;

sub t032 ($ = do { $z++; 222 }) { $a // "z" }
$z = 0;
is prototype(\&t032), undef;
is eval("t032()"), 123;
is $z, 1;
is eval("t032(0)"), 123;
is eval("t032(456)"), 123;
is eval("t032(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t032', 2, 1);
is eval("t032(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t032', 3, 1);
is eval("t032(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t032', 4, 1);
is $z, 1;
is $a, 123;

sub t027 ($ =) { $a // "z" }
is prototype(\&t027), undef;
is eval("t027()"), 123;
is eval("t027(0)"), 123;
is eval("t027(456)"), 123;
is eval("t027(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t027', 2, 1);
is eval("t027(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t027', 3, 1);
is eval("t027(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t027', 4, 1);
is $a, 123;

sub t119 ($ =, $a = 333) { $a // "z" }
is prototype(\&t119), undef;
is eval("t119()"), 333;
is eval("t119(0)"), 333;
is eval("t119(456)"), 333;
is eval("t119(456, 789)"), 789;
is eval("t119(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t119', 3, 2);
is eval("t119(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t119', 4, 2);
is $a, 123;

sub t028 ($a, $b = 333) { "$a/$b" }
is prototype(\&t028), undef;
is eval("t028()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t028', 0, 1);
is eval("t028(0)"), "0/333";
is eval("t028(456)"), "456/333";
is eval("t028(456, 789)"), "456/789";
is eval("t028(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t028', 3, 2);
is $a, 123;

sub t045 ($a, $ = 333) { "$a/" }
is prototype(\&t045), undef;
is eval("t045()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t045', 0, 1);
is eval("t045(0)"), "0/";
is eval("t045(456)"), "456/";
is eval("t045(456, 789)"), "456/";
is eval("t045(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t045', 3, 2);
is $a, 123;

sub t046 ($, $b = 333) { "$a/$b" }
is prototype(\&t046), undef;
is eval("t046()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t046', 0, 1);
is eval("t046(0)"), "123/333";
is eval("t046(456)"), "123/333";
is eval("t046(456, 789)"), "123/789";
is eval("t046(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t046', 3, 2);
is $a, 123;

sub t047 ($, $ = 333) { "$a/" }
is prototype(\&t047), undef;
is eval("t047()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t047', 0, 1);
is eval("t047(0)"), "123/";
is eval("t047(456)"), "123/";
is eval("t047(456, 789)"), "123/";
is eval("t047(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t047', 3, 2);
is $a, 123;

sub t029 ($a, $b, $c = 222, $d = 333) { "$a/$b/$c/$d" }
is prototype(\&t029), undef;
is eval("t029()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t029', 0, 2);
is eval("t029(0)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t029', 1, 2);
is eval("t029(456)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t029', 1, 2);
is eval("t029(456, 789)"), "456/789/222/333";
is eval("t029(456, 789, 987)"), "456/789/987/333";
is eval("t029(456, 789, 987, 654)"), "456/789/987/654";
is eval("t029(456, 789, 987, 654, 321)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t029', 5, 4);
is eval("t029(456, 789, 987, 654, 321, 111)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t029', 6, 4);
is $a, 123;

sub t038 ($a, $b = $a."x") { "$a/$b" }
is prototype(\&t038), undef;
is eval("t038()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t038', 0, 1);
is eval("t038(0)"), "0/0x";
is eval("t038(456)"), "456/456x";
is eval("t038(456, 789)"), "456/789";
is eval("t038(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t038', 3, 2);
is $a, 123;

eval "#line 8 foo\nsub t030 (\$a = 222, \$b) { }";
is $@, qq{Mandatory parameter follows optional parameter at foo line 8, near "\$b) "\n};

eval "#line 8 foo\nsub t031 (\$a = 222, \$b = 333, \$c, \$d) { }";
is $@, <<EOF;
Mandatory parameter follows optional parameter at foo line 8, near "\$c,"
Mandatory parameter follows optional parameter at foo line 8, near "\$d) "
EOF

sub t206 ($x, $y //= 3) { return $x + $y }
is eval("t206(5,4)"),     9, '//= present';
is eval("t206(5)"),       8, '//= absent';
is eval("t206(4,undef)"), 7, '//= undef';
is eval("t206(4,0)"),     4, '//= zero';

sub t207 ($x, $y ||= 3) { return $x + $y }
is eval("t207(5,4)"),     9, '||= present';
is eval("t207(5)"),       8, '||= absent';
is eval("t207(4,undef)"), 7, '||= undef';
is eval("t207(4,0)"),     7, '||= zero';

sub t034 (@abc) { join("/", @abc).";".scalar(@abc) }
is prototype(\&t034), undef;
is eval("t034()"), ";0";
is eval("t034(0)"), "0;1";
is eval("t034(456)"), "456;1";
is eval("t034(456, 789)"), "456/789;2";
is eval("t034(456, 789, 987)"), "456/789/987;3";
is eval("t034(456, 789, 987, 654)"), "456/789/987/654;4";
is eval("t034(456, 789, 987, 654, 321)"), "456/789/987/654/321;5";
is eval("t034(456, 789, 987, 654, 321, 111)"), "456/789/987/654/321/111;6";
is $a, 123;

eval "#line 8 foo\nsub t136 (\@abc = 222) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t137 (\@abc =) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "=) "\n};

sub t035 (@) { $a }
is prototype(\&t035), undef;
is eval("t035()"), 123;
is eval("t035(0)"), 123;
is eval("t035(456)"), 123;
is eval("t035(456, 789)"), 123;
is eval("t035(456, 789, 987)"), 123;
is eval("t035(456, 789, 987, 654)"), 123;
is eval("t035(456, 789, 987, 654, 321)"), 123;
is eval("t035(456, 789, 987, 654, 321, 111)"), 123;
is $a, 123;

eval "#line 8 foo\nsub t138 (\@ = 222) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t139 (\@ =) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "=) "\n};

sub t039 (%abc) { join("/", map { $_."=".$abc{$_} } sort keys %abc) }
is prototype(\&t039), undef;
is eval("t039()"), "";
is eval("t039(0)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t039' at \(eval \d+\) line 1\.\n\z#;
is eval("t039(456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t039' at \(eval \d+\) line 1\.\n\z#;
is eval("t039(456, 789)"), "456=789";
is eval("t039(456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t039' at \(eval \d+\) line 1\.\n\z#;
is eval("t039(456, 789, 987, 654)"), "456=789/987=654";
is eval("t039(456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t039' at \(eval \d+\) line 1\.\n\z#;
is eval("t039(456, 789, 987, 654, 321, 111)"), "321=111/456=789/987=654";
is $a, 123;

eval "#line 8 foo\nsub t140 (\%abc = 222) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t141 (\%abc =) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "=) "\n};

sub t040 (%) { $a }
is prototype(\&t040), undef;
is eval("t040()"), 123;
is eval("t040(0)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t040' at \(eval \d+\) line 1\.\n\z#;
is eval("t040(456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t040' at \(eval \d+\) line 1\.\n\z#;
is eval("t040(456, 789)"), 123;
is eval("t040(456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t040' at \(eval \d+\) line 1\.\n\z#;
is eval("t040(456, 789, 987, 654)"), 123;
is eval("t040(456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t040' at \(eval \d+\) line 1\.\n\z#;
is eval("t040(456, 789, 987, 654, 321, 111)"), 123;
is $a, 123;

eval "#line 8 foo\nsub t142 (\% = 222) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t143 (\% =) { }";
is $@, qq{A slurpy parameter may not have a default value at foo line 8, near "=) "\n};

sub t041 ($a, @b) { $a.";".join("/", @b) }
is prototype(\&t041), undef;
is eval("t041()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t041', 0, 1);
is eval("t041(0)"), "0;";
is eval("t041(456)"), "456;";
is eval("t041(456, 789)"), "456;789";
is eval("t041(456, 789, 987)"), "456;789/987";
is eval("t041(456, 789, 987, 654)"), "456;789/987/654";
is eval("t041(456, 789, 987, 654, 321)"), "456;789/987/654/321";
is eval("t041(456, 789, 987, 654, 321, 111)"), "456;789/987/654/321/111";
is $a, 123;

sub t042 ($a, @) { $a.";" }
is prototype(\&t042), undef;
is eval("t042()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t042', 0, 1);
is eval("t042(0)"), "0;";
is eval("t042(456)"), "456;";
is eval("t042(456, 789)"), "456;";
is eval("t042(456, 789, 987)"), "456;";
is eval("t042(456, 789, 987, 654)"), "456;";
is eval("t042(456, 789, 987, 654, 321)"), "456;";
is eval("t042(456, 789, 987, 654, 321, 111)"), "456;";
is $a, 123;

sub t043 ($, @b) { $a.";".join("/", @b) }
is prototype(\&t043), undef;
is eval("t043()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t043', 0, 1);
is eval("t043(0)"), "123;";
is eval("t043(456)"), "123;";
is eval("t043(456, 789)"), "123;789";
is eval("t043(456, 789, 987)"), "123;789/987";
is eval("t043(456, 789, 987, 654)"), "123;789/987/654";
is eval("t043(456, 789, 987, 654, 321)"), "123;789/987/654/321";
is eval("t043(456, 789, 987, 654, 321, 111)"), "123;789/987/654/321/111";
is $a, 123;

sub t044 ($, @) { $a.";" }
is prototype(\&t044), undef;
is eval("t044()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t044', 0, 1);
is eval("t044(0)"), "123;";
is eval("t044(456)"), "123;";
is eval("t044(456, 789)"), "123;";
is eval("t044(456, 789, 987)"), "123;";
is eval("t044(456, 789, 987, 654)"), "123;";
is eval("t044(456, 789, 987, 654, 321)"), "123;";
is eval("t044(456, 789, 987, 654, 321, 111)"), "123;";
is $a, 123;

sub t049 ($a, %b) { $a.";".join("/", map { $_."=".$b{$_} } sort keys %b) }
is prototype(\&t049), undef;
is eval("t049()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t049', 0, 1);
is eval("t049(222)"), "222;";
is eval("t049(222, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t049' at \(eval \d+\) line 1\.\n\z#;
is eval("t049(222, 456, 789)"), "222;456=789";
is eval("t049(222, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t049' at \(eval \d+\) line 1\.\n\z#;
is eval("t049(222, 456, 789, 987, 654)"), "222;456=789/987=654";
is eval("t049(222, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t049' at \(eval \d+\) line 1\.\n\z#;
is eval("t049(222, 456, 789, 987, 654, 321, 111)"),
    "222;321=111/456=789/987=654";
is $a, 123;

sub t051 ($a, $b, $c, @d) { "$a;$b;$c;".join("/", @d).";".scalar(@d) }
is prototype(\&t051), undef;
is eval("t051()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t051', 0, 3);
is eval("t051(456)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t051', 1, 3);
is eval("t051(456, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t051', 2, 3);
is eval("t051(456, 789, 987)"), "456;789;987;;0";
is eval("t051(456, 789, 987, 654)"), "456;789;987;654;1";
is eval("t051(456, 789, 987, 654, 321)"), "456;789;987;654/321;2";
is eval("t051(456, 789, 987, 654, 321, 111)"), "456;789;987;654/321/111;3";
is $a, 123;

sub t052 ($a, $b, %c) { "$a;$b;".join("/", map { $_."=".$c{$_} } sort keys %c) }
is prototype(\&t052), undef;
is eval("t052()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t052', 0, 2);
is eval("t052(222)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t052', 1, 2);
is eval("t052(222, 333)"), "222;333;";
is eval("t052(222, 333, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t052' at \(eval \d+\) line 1\.\n\z#;
is eval("t052(222, 333, 456, 789)"), "222;333;456=789";
is eval("t052(222, 333, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t052' at \(eval \d+\) line 1\.\n\z#;
is eval("t052(222, 333, 456, 789, 987, 654)"), "222;333;456=789/987=654";
is eval("t052(222, 333, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t052' at \(eval \d+\) line 1\.\n\z#;
is eval("t052(222, 333, 456, 789, 987, 654, 321, 111)"),
    "222;333;321=111/456=789/987=654";
is $a, 123;

sub t053 ($a, $b, $c, %d) {
    "$a;$b;$c;".join("/", map { $_."=".$d{$_} } sort keys %d)
}
is prototype(\&t053), undef;
is eval("t053()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t053', 0, 3);
is eval("t053(222)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t053', 1, 3);
is eval("t053(222, 333)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t053', 2, 3);
is eval("t053(222, 333, 444)"), "222;333;444;";
is eval("t053(222, 333, 444, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t053' at \(eval \d+\) line 1\.\n\z#;
is eval("t053(222, 333, 444, 456, 789)"), "222;333;444;456=789";
is eval("t053(222, 333, 444, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t053' at \(eval \d+\) line 1\.\n\z#;
is eval("t053(222, 333, 444, 456, 789, 987, 654)"),
    "222;333;444;456=789/987=654";
is eval("t053(222, 333, 444, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t053' at \(eval \d+\) line 1\.\n\z#;
is eval("t053(222, 333, 444, 456, 789, 987, 654, 321, 111)"),
    "222;333;444;321=111/456=789/987=654";
is $a, 123;

sub t048 ($a = 222, @b) { $a.";".join("/", @b).";".scalar(@b) }
is prototype(\&t048), undef;
is eval("t048()"), "222;;0";
is eval("t048(0)"), "0;;0";
is eval("t048(456)"), "456;;0";
is eval("t048(456, 789)"), "456;789;1";
is eval("t048(456, 789, 987)"), "456;789/987;2";
is eval("t048(456, 789, 987, 654)"), "456;789/987/654;3";
is eval("t048(456, 789, 987, 654, 321)"), "456;789/987/654/321;4";
is eval("t048(456, 789, 987, 654, 321, 111)"), "456;789/987/654/321/111;5";
is $a, 123;

sub t054 ($a = 222, $b = 333, @c) { "$a;$b;".join("/", @c).";".scalar(@c) }
is prototype(\&t054), undef;
is eval("t054()"), "222;333;;0";
is eval("t054(456)"), "456;333;;0";
is eval("t054(456, 789)"), "456;789;;0";
is eval("t054(456, 789, 987)"), "456;789;987;1";
is eval("t054(456, 789, 987, 654)"), "456;789;987/654;2";
is eval("t054(456, 789, 987, 654, 321)"), "456;789;987/654/321;3";
is eval("t054(456, 789, 987, 654, 321, 111)"), "456;789;987/654/321/111;4";
is $a, 123;

sub t055 ($a = 222, $b = 333, $c = 444, @d) {
    "$a;$b;$c;".join("/", @d).";".scalar(@d)
}
is prototype(\&t055), undef;
is eval("t055()"), "222;333;444;;0";
is eval("t055(456)"), "456;333;444;;0";
is eval("t055(456, 789)"), "456;789;444;;0";
is eval("t055(456, 789, 987)"), "456;789;987;;0";
is eval("t055(456, 789, 987, 654)"), "456;789;987;654;1";
is eval("t055(456, 789, 987, 654, 321)"), "456;789;987;654/321;2";
is eval("t055(456, 789, 987, 654, 321, 111)"), "456;789;987;654/321/111;3";
is $a, 123;

sub t050 ($a = 211, %b) { $a.";".join("/", map { $_."=".$b{$_} } sort keys %b) }
is prototype(\&t050), undef;
is eval("t050()"), "211;";
is eval("t050(222)"), "222;";
is eval("t050(222, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t050' at \(eval \d+\) line 1\.\n\z#;
is eval("t050(222, 456, 789)"), "222;456=789";
is eval("t050(222, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t050' at \(eval \d+\) line 1\.\n\z#;
is eval("t050(222, 456, 789, 987, 654)"), "222;456=789/987=654";
is eval("t050(222, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t050' at \(eval \d+\) line 1\.\n\z#;
is eval("t050(222, 456, 789, 987, 654, 321, 111)"),
    "222;321=111/456=789/987=654";
is $a, 123;

sub t056 ($a = 211, $b = 311, %c) {
    "$a;$b;".join("/", map { $_."=".$c{$_} } sort keys %c)
}
is prototype(\&t056), undef;
is eval("t056()"), "211;311;";
is eval("t056(222)"), "222;311;";
is eval("t056(222, 333)"), "222;333;";
is eval("t056(222, 333, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t056' at \(eval \d+\) line 1\.\n\z#;
is eval("t056(222, 333, 456, 789)"), "222;333;456=789";
is eval("t056(222, 333, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t056' at \(eval \d+\) line 1\.\n\z#;
is eval("t056(222, 333, 456, 789, 987, 654)"), "222;333;456=789/987=654";
is eval("t056(222, 333, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t056' at \(eval \d+\) line 1\.\n\z#;
is eval("t056(222, 333, 456, 789, 987, 654, 321, 111)"),
    "222;333;321=111/456=789/987=654";
is $a, 123;

sub t057 ($a = 211, $b = 311, $c = 411, %d) {
    "$a;$b;$c;".join("/", map { $_."=".$d{$_} } sort keys %d)
}
is prototype(\&t057), undef;
is eval("t057()"), "211;311;411;";
is eval("t057(222)"), "222;311;411;";
is eval("t057(222, 333)"), "222;333;411;";
is eval("t057(222, 333, 444)"), "222;333;444;";
is eval("t057(222, 333, 444, 456)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t057' at \(eval \d+\) line 1\.\n\z#;
is eval("t057(222, 333, 444, 456, 789)"), "222;333;444;456=789";
is eval("t057(222, 333, 444, 456, 789, 987)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t057' at \(eval \d+\) line 1\.\n\z#;
is eval("t057(222, 333, 444, 456, 789, 987, 654)"),
    "222;333;444;456=789/987=654";
is eval("t057(222, 333, 444, 456, 789, 987, 654, 321)"), undef;
like $@, qr#\AOdd name/value argument for subroutine 'main::t057' at \(eval \d+\) line 1\.\n\z#;
is eval("t057(222, 333, 444, 456, 789, 987, 654, 321, 111)"),
    "222;333;444;321=111/456=789/987=654";
is $a, 123;

sub t058 ($a, $b = 333, @c) { "$a;$b;".join("/", @c).";".scalar(@c) }
is prototype(\&t058), undef;
is eval("t058()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t058', 0, 1);
is eval("t058(456)"), "456;333;;0";
is eval("t058(456, 789)"), "456;789;;0";
is eval("t058(456, 789, 987)"), "456;789;987;1";
is eval("t058(456, 789, 987, 654)"), "456;789;987/654;2";
is eval("t058(456, 789, 987, 654, 321)"), "456;789;987/654/321;3";
is eval("t058(456, 789, 987, 654, 321, 111)"), "456;789;987/654/321/111;4";
is $a, 123;

eval "#line 8 foo\nsub t059 (\@a, \$b) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$b) "\n};

eval "#line 8 foo\nsub t060 (\@a, \$b = 222) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t061 (\@a, \@b) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\@b) "\n};

eval "#line 8 foo\nsub t062 (\@a, \%b) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "%b) "\n};

eval "#line 8 foo\nsub t063 (\@, \$b) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$b) "\n};

eval "#line 8 foo\nsub t064 (\@, \$b = 222) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t065 (\@, \@b) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\@b) "\n};

eval "#line 8 foo\nsub t066 (\@, \%b) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "%b) "\n};

eval "#line 8 foo\nsub t067 (\@a, \$) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$) "\n};

eval "#line 8 foo\nsub t068 (\@a, \$ = 222) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t069 (\@a, \@) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\@) "\n};

eval "#line 8 foo\nsub t070 (\@a, \%) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\%) "\n};

eval "#line 8 foo\nsub t071 (\@, \$) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$) "\n};

eval "#line 8 foo\nsub t072 (\@, \$ = 222) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "222) "\n};

eval "#line 8 foo\nsub t073 (\@, \@) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\@) "\n};

eval "#line 8 foo\nsub t074 (\@, \%) { }";
is $@, qq{Multiple slurpy parameters not allowed at foo line 8, near "\%) "\n};

eval "#line 8 foo\nsub t075 (\%a, \$b) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$b) "\n};

eval "#line 8 foo\nsub t076 (\%, \$b) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$b) "\n};

eval "#line 8 foo\nsub t077 (\$a, \@b, \$c) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$c) "\n};

eval "#line 8 foo\nsub t078 (\$a, \%b, \$c) { }";
is $@, qq{Slurpy parameter not last at foo line 8, near "\$c) "\n};

eval "#line 8 foo\nsub t079 (\$a, \@b, \$c, \$d) { }";
is $@, <<EOF;
Slurpy parameter not last at foo line 8, near "\$c,"
Slurpy parameter not last at foo line 8, near "\$d) "
EOF

sub t080 ($a,,, $b) { $a.$b }
is prototype(\&t080), undef;
is eval("t080()"), undef;
like $@, _create_mismatch_regexp('main::t080', 0, 2);
is eval("t080(456)"), undef;
like $@, _create_mismatch_regexp('main::t080', 1, 2);
is eval("t080(456, 789)"), "456789";
is eval("t080(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t080', 3, 2);
is eval("t080(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t080', 4, 2);
is $a, 123;

sub t081 ($a, $b,,) { $a.$b }
is prototype(\&t081), undef;
is eval("t081()"), undef;
like $@, _create_mismatch_regexp('main::t081', 0, 2);
is eval("t081(456)"), undef;
like $@, _create_mismatch_regexp('main::t081', 1, 2);
is eval("t081(456, 789)"), "456789";
is eval("t081(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t081', 3, 2);
is eval("t081(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t081', 4, 2);
is $a, 123;

eval "#line 8 foo\nsub t082 (, \$a) { }";
is $@, qq{syntax error at foo line 8, near "(,"\nExecution of foo aborted due to compilation errors.\n};

eval "#line 8 foo\nsub t083 (,) { }";
is $@, qq{syntax error at foo line 8, near "(,"\nExecution of foo aborted due to compilation errors.\n};

sub t084($a,$b){ $a.$b }
is prototype(\&t084), undef;
is eval("t084()"), undef;
like $@, _create_mismatch_regexp('main::t084', 0, 2);
is eval("t084(456)"), undef;
like $@, _create_mismatch_regexp('main::t084', 1, 2);
is eval("t084(456, 789)"), "456789";
is eval("t084(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t084', 3, 2);
is eval("t084(456, 789, 987, 654)"), undef;
like $@, _create_mismatch_regexp('main::t084', 4, 2);
is $a, 123;

sub t085
    (
    $
    a
    ,
    ,
    $
    b
    =
    333
    ,
    ,
    )
    { $a.$b }
is prototype(\&t085), undef;
is eval("t085()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t085', 0, 1);
is eval("t085(456)"), "456333";
is eval("t085(456, 789)"), "456789";
is eval("t085(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t085', 3, 2);
is eval("t085(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t085', 4, 2);
is $a, 123;

sub t086
    ( #foo)))
    $ #foo)))
    a #foo)))
    , #foo)))
    , #foo)))
    $ #foo)))
    b #foo)))
    = #foo)))
    333 #foo)))
    , #foo)))
    , #foo)))
    ) #foo)))
    { $a.$b }
is prototype(\&t086), undef;
is eval("t086()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t086', 0, 1);
is eval("t086(456)"), "456333";
is eval("t086(456, 789)"), "456789";
is eval("t086(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t086', 3, 2);
is eval("t086(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t086', 4, 2);
is $a, 123;

sub t087
    (#foo)))
    $ #foo)))
    a#foo)))
    ,#foo)))
    ,#foo)))
    $ #foo)))
    b#foo)))
    =#foo)))
    333#foo)))
    ,#foo)))
    ,#foo)))
    )#foo)))
    { $a.$b }
is prototype(\&t087), undef;
is eval("t087()"), undef;
like $@, _create_flexible_mismatch_regexp('main::t087', 0, 1);
is eval("t087(456)"), "456333";
is eval("t087(456, 789)"), "456789";
is eval("t087(456, 789, 987)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t087', 3, 2);
is eval("t087(456, 789, 987, 654)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t087', 4, 2);
is $a, 123;

eval "#line 8 foo\nsub t088 (\$ #foo\na) { }";
is $@, "";


eval "#line 8 foo\nsub t089 (\$#foo\na) { }";
like $@, qr{\A'#' not allowed immediately following a sigil in a subroutine signature at foo line 8, near "\(\$"\n};

eval "#line 8 foo\nsub t090 (\@ #foo\na) { }";
is $@, "";

eval "#line 8 foo\nsub t091 (\@#foo\na) { }";
like $@, qr{\A'#' not allowed immediately following a sigil in a subroutine signature at foo line 8, near "\(\@"\n};

eval "#line 8 foo\nsub t092 (\% #foo\na) { }";
is $@, "";

eval "#line 8 foo\nsub t093 (\%#foo\na) { }";
like $@, qr{\A'#' not allowed immediately following a sigil in a subroutine signature at foo line 8, near "\(%"\n};

eval "#line 8 foo\nsub t094 (123) { }";
like $@, qr{\AA signature parameter must start with '\$', '\@' or '%' at foo line 8, near "\(1"\n};

eval "#line 8 foo\nsub t095 (\$a, 123) { }";
is $@, <<EOF;
A signature parameter must start with '\$', '\@' or '%' at foo line 8, near ", 1"
syntax error at foo line 8, near ", 123"
Execution of foo aborted due to compilation errors.
EOF

eval "#line 8 foo\nno warnings; sub t096 (\$a 123) { }";
is $@, <<'EOF';
Illegal operator following parameter in a subroutine signature at foo line 8, near "($a 123"
syntax error at foo line 8, near "($a 123"
Execution of foo aborted due to compilation errors.
EOF

eval "#line 8 foo\nsub t097 (\$a { }) { }";
is $@, <<'EOF';
Illegal operator following parameter in a subroutine signature at foo line 8, near "($a { }"
syntax error at foo line 8, near "($a { }"
Execution of foo aborted due to compilation errors.
EOF

eval "#line 8 foo\nsub t098 (\$a; \$b) { }";
is $@, <<'EOF';
Illegal operator following parameter in a subroutine signature at foo line 8, near "($a; "
syntax error at foo line 8, near "($a; "
Execution of foo aborted due to compilation errors.
EOF

eval "#line 8 foo\nsub t099 (\$\$) { }";
is $@, <<EOF;
Illegal character following sigil in a subroutine signature at foo line 8, near "(\$"
syntax error at foo line 8, near "\$\$) "
Execution of foo aborted due to compilation errors.
EOF

eval "#line 8 foo\nsub t101 (\@_) { }";
like $@, qr/\ACan't use global \@_ in subroutine signature at foo line 8/;

eval "#line 8 foo\nsub t102 (\%_) { }";
like $@, qr/\ACan't use global \%_ in subroutine signature at foo line 8/;

my $t103 = sub ($a) { $a || "z" };
is prototype($t103), undef;
is eval("\$t103->()"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 0, 1);
is eval("\$t103->(0)"), "z";
is eval("\$t103->(456)"), 456;
is eval("\$t103->(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 2, 1);
is eval("\$t103->(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 3, 1);
is $a, 123;

my $t118 = sub :prototype($) ($a) { $a || "z" };
is prototype($t118), "\$";
is eval("\$t118->()"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 0, 1);
is eval("\$t118->(0)"), "z";
is eval("\$t118->(456)"), 456;
is eval("\$t118->(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 2, 1);
is eval("\$t118->(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::__ANON__', 3, 1);
is $a, 123;

sub t033 ($a = sub ($a) { $a."z" }) { $a->("a")."y" }
is prototype(\&t033), undef;
is eval("t033()"), "azy";
is eval("t033(sub { \"x\".\$_[0].\"x\" })"), "xaxy";
is eval("t033(sub { \"x\".\$_[0].\"x\" }, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t033', 2, 1);
is $a, 123;

sub t133 ($a = sub ($a = 222) { $a."z" }) { $a->()."/".$a->("a") }
is prototype(\&t133), undef;
is eval("t133()"), "222z/az";
is eval("t133(sub { \"x\".(\$_[0] // \"u\").\"x\" })"), "xux/xax";
is eval("t133(sub { \"x\".(\$_[0] // \"u\").\"x\" }, 789)"), undef;
like $@, _create_flexible_mismatch_regexp('main::t133', 2, 1);
is $a, 123;

sub t134 ($a = sub ($a, $t = sub { $_[0]."p" }) { $t->($a)."z" }) {
    $a->("a")."/".$a->("b", sub { $_[0]."q" } )
}
is prototype(\&t134), undef;
is eval("t134()"), "apz/bqz";
is eval("t134(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" })"),
    "xax/xbqx";
is eval("t134(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" }, 789)"),
    undef;
like $@, _create_flexible_mismatch_regexp('main::t134', 2, 1);
is $a, 123;

sub t135 ($a = sub ($a, $t = sub ($p) { $p."p" }) { $t->($a)."z" }) {
    $a->("a")."/".$a->("b", sub { $_[0]."q" } )
}
is prototype(\&t135), undef;
is eval("t135()"), "apz/bqz";
is eval("t135(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" })"),
    "xax/xbqx";
is eval("t135(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" }, 789)"),
    undef;
like $@, _create_flexible_mismatch_regexp('main::t135', 2, 1);
is $a, 123;

sub t132 (
    $a = sub ($a, $t = sub ($p = 222) { $p."p" }) { $t->($a)."z".$t->() },
) {
    $a->("a")."/".$a->("b", sub { ($_[0] // "u")."q" } )
}
is prototype(\&t132), undef;
is eval("t132()"), "apz222p/bqzuq";
is eval("t132(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" })"),
    "xax/xbqx";
is eval("t132(sub { \"x\".(\$_[1] // sub{\$_[0]})->(\$_[0]).\"x\" }, 789)"),
    undef;
like $@, _create_flexible_mismatch_regexp('main::t132', 2, 1);
is $a, 123;

sub t104 :method ($a) { $a || "z" }
is prototype(\&t104), undef;
is eval("t104()"), undef;
like $@, _create_mismatch_regexp('main::t104', 0, 1);
is eval("t104(0)"), "z";
is eval("t104(456)"), 456;
is eval("t104(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t104', 2, 1);
is eval("t104(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t104', 3, 1);
is $a, 123;

sub t105 :prototype($) ($a) { $a || "z" }
is prototype(\&t105), "\$";
is eval("t105()"), undef;
like $@, qr/\ANot enough arguments for main::t105 /;
is eval("t105(0)"), "z";
is eval("t105(456)"), 456;
is eval("t105(456, 789)"), undef;
like $@, qr/\AToo many arguments for main::t105 at \(eval \d+\) line 1, near/;
is eval("t105(456, 789, 987)"), undef;
like $@, qr/\AToo many arguments for main::t105 at \(eval \d+\) line 1, near/;
is $a, 123;

sub t106 :prototype(@) ($a) { $a || "z" }
is prototype(\&t106), "\@";
is eval("t106()"), undef;
like $@, _create_mismatch_regexp('main::t106', 0, 1);
is eval("t106(0)"), "z";
is eval("t106(456)"), 456;
is eval("t106(456, 789)"), undef;
like $@, _create_mismatch_regexp('main::t106', 2, 1);
is eval("t106(456, 789, 987)"), undef;
like $@, _create_mismatch_regexp('main::t106', 3, 1);
is $a, 123;

eval "#line 8 foo\nsub t107(\$a) :method { }";
isnt $@, "";

eval "#line 8 foo\nsub t108 (\$a) :prototype(\$) { }";
isnt $@, "";

sub t109 { }
is prototype(\&t109), undef;
is scalar(@{[ t109() ]}), 0;
is scalar(t109()), undef;

sub t110 () { }
is prototype(\&t110), undef;
is scalar(@{[ t110() ]}), 0;
is scalar(t110()), undef;

sub t111 ($a) { }
is prototype(\&t111), undef;
is scalar(@{[ t111(222) ]}), 0;
is scalar(t111(222)), undef;

sub t112 ($) { }
is prototype(\&t112), undef;
is scalar(@{[ t112(222) ]}), 0;
is scalar(t112(222)), undef;

sub t114 ($a = undef) { }
is prototype(\&t114), undef;
is scalar(@{[ t114() ]}), 0;
is scalar(t114()), undef;
is scalar(@{[ t114(333) ]}), 0;
is scalar(t114(333)), undef;

sub t113 ($a = 222) { }
is prototype(\&t113), undef;
is scalar(@{[ t113() ]}), 0;
is scalar(t113()), undef;
is scalar(@{[ t113(333) ]}), 0;
is scalar(t113(333)), undef;

sub t115 ($a = do { $z++; 222 }) { }
is prototype(\&t115), undef;
$z = 0;
is scalar(@{[ t115() ]}), 0;
is $z, 1;
is scalar(t115()), undef;
is $z, 2;
is scalar(@{[ t115(333) ]}), 0;
is scalar(t115(333)), undef;
is $z, 2;

sub t116 (@a) { }
is prototype(\&t116), undef;
is scalar(@{[ t116() ]}), 0;
is scalar(t116()), undef;
is scalar(@{[ t116(333) ]}), 0;
is scalar(t116(333)), undef;

sub t117 (%a) { }
is prototype(\&t117), undef;
is scalar(@{[ t117() ]}), 0;
is scalar(t117()), undef;
is scalar(@{[ t117(333, 444) ]}), 0;
is scalar(t117(333, 444)), undef;

sub t145 ($=3) { }
is scalar(t145()), undef;

{
    my $want;
    sub want { $want = wantarray ? "list"
                        : defined(wantarray) ? "scalar" : "void"; 1 }

    sub t144 ($a = want()) { $a }
    t144();
    is ($want, "scalar", "default expression is scalar in void context");
    my $x = t144();
    is ($want, "scalar", "default expression is scalar in scalar context");
    () = t144();
    is ($want, "scalar", "default expression is scalar in list context");
}


# check for default arg code doing nasty things (closures, gotos,
# modifying @_ etc).

{
    no warnings qw(closure);
    use Tie::Array;
    use Tie::Hash;

    sub t146 ($a = t146x()) {
        sub t146x { $a = "abc"; 1 }
        $a;
    }
    is t146(), 1, "t146: closure can make new lexical not undef";

    sub t147 ($a = t147x()) {
        sub t147x { $a = "abc"; pos($a)=1; 1 }
        is pos($a), undef, "t147: pos magic cleared";
        $a;
    }
    is t147(), 1, "t147: closure can make new lexical not undef and magical";

    sub t148 ($a = t148x()) {
        sub t148x { $a = [];  1 }
        $a;
    }
    is t148(), 1, "t148: closure can make new lexical a ref";

    sub t149 ($a = t149x()) {
        sub t149x { $a = 1;  [] }
        $a;
    }
    is ref(t149()), "ARRAY", "t149: closure can make new lexical a ref";

    # Quiet the 'use of @_ is experimental' warnings
    no warnings 'experimental::args_array_with_signatures';

    sub t150 ($a = do {@_ = qw(a b c); 1}, $b = 2) {
        is $a, 1,   "t150: a: growing \@_";
        is $b, "b", "t150: b: growing \@_";
    }
    t150();

    sub t151 ($a = do {tie @_, 'Tie::StdArray'; @_ = qw(a b c); 1}, $b = 2) {
        is $a, 1,   "t151: a: tied \@_";
        is $b, "b", "t151: b: tied \@_";
    }
    t151();

    sub t152 ($a = t152x(), @b) {
        sub t152x { @b = qw(a b c); 1 }
        $a . '-' . join(':', @b);
    }
    is t152(), "1-", "t152: closure can make new lexical array non-empty";

    sub t153 ($a = t153x(), %b) {
        sub t153x { %b = qw(a 10 b 20); 1 }
        $a . '-' . join(':', sort %b);
    }
    is t153(), "1-", "t153: closure can make new lexical hash non-empty";

    sub t154 ($a = t154x(), @b) {
        sub t154x { tie @b, 'Tie::StdArray'; @b = qw(a b c); 1 }
        $a . '-' . join(':', @b);
    }
    is t154(), "1-", "t154: closure can make new lexical array tied";

    sub t155 ($a = t155x(), %b) {
        sub t155x { tie %b, 'Tie::StdHash'; %b = qw(a 10 b 20); 1 }
        $a . '-' . join(':', sort %b);
    }
    is t155(), "1-", "t155: closure can make new lexical hash tied";

    sub t156 ($a = do {@_ = qw(a b c); 1}, @b) {
        is $a, 1,       "t156: a: growing \@_";
        is "@b", "b c", "t156: b: growing \@_";
    }
    t156();

    sub t157 ($a = do {@_ = qw(a b c); 1}, %b) {
        is $a, 1,                     "t157: a: growing \@_";
        is join(':', sort %b), "b:c", "t157: b: growing \@_";
    }
    t157();

    sub t158 ($a = do {tie @_, 'Tie::StdArray'; @_ = qw(a b c); 1}, @b) {
        is $a, 1,          "t158: a: tied \@_";
        is "@b", "b c",    "t158: b: tied \@_";
    }
    t158();

    sub t159 ($a = do {tie @_, 'Tie::StdArray'; @_ = qw(a b c); 1}, %b) {
        is  $a, 1,                     "t159: a: tied \@_";
        is  join(':', sort %b), "b:c", "t159: b: tied \@_";
    }
    t159();

    # see if we can handle the equivalent of @a = ($a[1], $a[0])

    sub t160 ($s, @a) {
        sub t160x {
            @a = qw(x y);
            t160(1, $a[1], $a[0]);
        }
        # encourage recently-freed SVPVs to be realloced with new values
        my @pad = qw(a b);
        join ':', $s, @a;
    }
    is t160x(), "1:y:x", 'handle commonality in slurpy array';

    # see if we can handle the equivalent of %h = ('foo', $h{foo})

    sub t161 ($s, %h) {
        sub t161x {
            %h = qw(k1 v1 k2 v2);
            t161(1, k1 => $h{k2}, k2 => $h{k1});
        }
        # encourage recently-freed SVPVs to be realloced with new values
        my @pad = qw(a b);
        join ' ', $s, map "($_,$h{$_})", sort keys %h;
    }
    is t161x(), "1 (k1,v2) (k2,v1)", 'handle commonality in slurpy hash';

    # see if we can handle the equivalent of ($a,$b) = ($b,$a)
    # Note that for non-signatured subs, my ($a,$b) = @_ already fails the
    # equivalent of this test too, since I skipped pessimising it
    # (90ce4d057857) as commonality in this case is rare and contrived,
    # as the example below shows. DAPM.
    sub t162 ($a, $b) {
        sub t162x {
            ($a, $b) = qw(x y);
            t162($b, $a);
        }
        "$a:$b";
    }
    {
        local $::TODO = q{can't handle commonaility};
        is t162x(), "y:x", 'handle commonality in scalar parms';
    }
}

{
    my $w;
    local $SIG{__WARN__} = sub { $w .= "@_" };
    is eval q{sub ($x,$x) { $x}->(1,2)}, 2, "duplicate sig var names";
    like $w, qr/^"my" variable \$x masks earlier declaration in same scope/,
            "masking warning";
}

# Reporting subroutine names

package T200 {
    sub foo ($x) {}
    *t201 = sub ($x) {}
}
*t202 = sub ($x) {};
my $t203 = sub ($x) {};
*t204 = *T200::foo;
*t205 = \&T200::foo;

eval { T200::foo() };
like($@, qr/^Too few arguments for subroutine 'T200::foo'/);
eval { T200::t201() };
like($@, qr/^Too few arguments for subroutine 'T200::__ANON__'/);
eval { t202() };
like($@, qr/^Too few arguments for subroutine 'main::__ANON__'/);
eval { $t203->() };
like($@, qr/^Too few arguments for subroutine 'main::__ANON__'/);
eval { t204() };
like($@, qr/^Too few arguments for subroutine 'T200::foo'/);
eval { t205() };
like($@, qr/^Too few arguments for subroutine 'T200::foo'/);


# RT #130661 a char >= 0x80 in a signature when a sigil was expected
# was triggering an assertion

eval "sub (\x80";
like $@, qr/A signature parameter must start with/, "RT #130661";



use File::Spec::Functions;
my $keywords_file = catfile(updir,'regen','keywords.pl');
open my $kh, $keywords_file
   or die "$0 cannot open $keywords_file: $!";
while(<$kh>) {
    if (m?__END__?..${\0} and /^[+-]/) {
        chomp(my $word = $');
        # $y should be an error after $x=foo.  The exact error we get may
        # differ if this is __END__ or s or some other special keyword.
        eval 'no warnings; sub ($x = ' . $word . ', $y) {}';
        isnt $@, "", "$word does not swallow trailing comma";
    }
}

# RT #132141
# Attributes such as lvalue have to come *before* the signature to
# ensure that they're applied to any code block within the signature

{
    my $x;
    sub f :lvalue ($a = do { $x = "abc"; return substr($x,0,1)}) {
        die; # notreached
    }

    f() = "X";
    is $x, "Xbc", "RT #132141";
}

# RT #132760
# attributes have been moved back before signatures for 5.28. Ensure that
# code doing it the old wrong way get a meaningful error message.

{
    my @errs;
    local $SIG{__WARN__} = sub { push @errs, @_};
    eval q{
        sub rt132760 ($a, $b) :prototype($$) { $a + $b }
    };

    @errs = split /\n/, $@;
    is +@errs, 1, "RT 132760 expect 1 error";
    like $errs[0],
        qr/^Subroutine attributes must come before the signature at/,
        "RT 132760 err 0";
}

# check that warnings come from the correct line

{
    my @warn;
    local $SIG{__WARN__} = sub { push @warn, @_};
    eval q{
        sub multiline1 (
            $a,
            $b = $a + 1,
            $c = $a + 1)
        {
            my $d = $a + 1;
            my $e = $a + 1;
        }
    };
    multiline1(undef);
    like $warn[0], qr/line 4,/, 'multiline1: $b';
    like $warn[1], qr/line 5,/, 'multiline1: $c';
    like $warn[2], qr/line 7,/, 'multiline1: $d';
    like $warn[3], qr/line 8,/, 'multiline1: $e';
}

# check errors for using global vars as params

{
    eval q{ sub ($_) {} };
    like $@, qr/Can't use global \$_ in subroutine signature/, 'f($_)';
    eval q{ sub (@_) {} };
    like $@, qr/Can't use global \@_ in subroutine signature/, 'f(@_)';
    eval q{ sub (%_) {} };
    like $@, qr/Can't use global \%_ in subroutine signature/, 'f(%_)';
    eval q{ sub ($1) {} };
    like $@, qr/Illegal operator following parameter in a subroutine signature/,
            'f($1)';
}

# check that various uses of @_ inside signatured subs causes "experimental"
# warnings at compiletime
{
    sub warnings_from {
        my ($code, $run) = @_;
        my $warnings = "";
        local $SIG{__WARN__} = sub { $warnings .= join "", @_ };
        my $cv = eval qq{ sub(\$x) { $code }} or die "Cannot eval() - $@";
        $run and $cv->(123);
        return $warnings;
    }

    sub snailwarns_ok {
        my ($opname, $code) = @_;
        my $warnings = warnings_from $code;
        ok($warnings =~ m/[Uu]se of \@_ in $opname with signatured subroutine is experimental at \(eval /,
            "`$code` warns of experimental \@_") or
            diag("Warnings were:\n$warnings");
    }

    sub snailwarns_runtime_ok {
        my ($opname, $code) = @_;
        my $warnings = warnings_from $code, 1;
        ok($warnings =~ m/[Uu]se of \@_ in $opname with signatured subroutine is experimental at \(eval /,
            "`$code` warns of experimental \@_") or
            diag("Warnings were:\n$warnings");
    }

    sub not_snailwarns_ok {
        my ($code) = @_;
        my $warnings = warnings_from $code;
        ok($warnings !~ m/[Uu]se of \@_ in .* with signatured subroutine is experimental at \(eval /,
            "`$code` warns of experimental \@_") or
            diag("Warnings were:\n$warnings");
    }

    # implicit @_
    snailwarns_ok 'shift',            'shift';
    snailwarns_ok 'pop',              'pop';
    snailwarns_ok 'goto',             'goto &SUB'; # tail-call
    snailwarns_ok 'subroutine entry', '&SUB'; # perl4-style

    # explicit @_
    snailwarns_ok 'shift',            'shift @_';
    snailwarns_ok 'pop',              'pop @_';
    snailwarns_ok 'array element',    '$_[0]';
    snailwarns_ok 'array element',    'my $one = 1; $_[$one]';
    snailwarns_ok 'push',             'push @_, 1';
    snailwarns_ok 'unshift',          'unshift @_, 9';
    snailwarns_ok 'splice',           'splice @_, 1, 2, 3';
    snailwarns_ok 'keys on array',    'keys @_';
    snailwarns_ok 'values on array',  'values @_';
    snailwarns_ok 'each on array',    'each @_';
    snailwarns_ok 'print',            'print "a", @_, "z"';
    snailwarns_ok 'subroutine entry', 'func("a", @_, "z")';

    # Also warns about @_ inside the signature params
    like(warnings_from('sub ($x = shift) { }'),
        qr/^Implicit use of \@_ in shift with signatured subroutine is experimental at \(eval /,
        'Warns of experimental @_ in param default');
    like(warnings_from('sub ($x = $_[0]) { }'),
        qr/^Use of \@_ in array element with signatured subroutine is experimental at \(eval /,
        'Warns of experimental @_ in param default');

    # Inside eval() still counts, at runtime
    snailwarns_runtime_ok 'array element', 'eval q( $_[0] )';

    # still permitted without warning
    not_snailwarns_ok 'my $f = sub { my $y = shift; }';
    not_snailwarns_ok 'my $f = sub { my $y = $_[0]; }';
    not_snailwarns_ok '\&SUB';
}

# Warnings can be disabled
{
    my $warnings = "";
    local $SIG{__WARN__} = sub { $warnings .= join "", @_ };
    eval q{
        no warnings 'experimental::snail_in_signatures';
        sub($x) { @_ = (1,2,3) }
    };
    is($warnings, "", 'No warnings emitted within scope of  no warnings "experimental"');
}

done_testing;

1;
