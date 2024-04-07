#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

watchdog(10);
plan(tests => 43);
use strict;

use Config;
my $have_alarm = $Config{d_alarm};

our $x = "abc\ndef\n";
study($x);

ok($x =~ /^abc/);
ok($x !~ /^def/);

# used to be a test for $*
ok($x =~ /^def/m);

$_ = '123';
study;
ok(/^([0-9][0-9]*)/);

ok(!($x =~ /^xxx/));
ok(!($x !~ /^abc/));

ok($x =~ /def/);
ok(!($x !~ /def/));

study($x);
ok($x !~ /.def/);
ok(!($x =~ /.def/));

ok($x =~ /\ndef/);
ok(!($x !~ /\ndef/));

$_ = 'aaabbbccc';
study;
ok(/(a*b*)(c*)/);
is($1, 'aaabbb');
is($2,'ccc');
ok(/(a+b+c+)/);
is($1, 'aaabbbccc');

ok(!/a+b?c+/);

$_ = 'aaabccc';
study;
ok(/a+b?c+/);
ok(/a*b+c*/);

$_ = 'aaaccc';
study;
ok(/a*b?c*/);
ok(!/a*b+c*/);

$_ = 'abcdef';
study;
ok(/bcd|xyz/);
ok(/xyz|bcd/);

ok(m|bc/*d|);

ok(/^$_$/);

# used to be a test for $*
ok("ab\ncd\n" =~ /^cd/m);

TODO: {
    # Even with the alarm() OS/390 and BS2000 can't manage these tests
    # (Perl just goes into a busy loop, luckily an interruptable one)
    todo_skip('busy loop - compiler bug?', 2)
	      if $^O eq 'os390' or $^O eq 'posix-bc';

    # [ID ] tests 25..26 may loop

    $_ = 'FGF';
    study;
    ok(!/G.F$/, 'bug 20010618.006 (#7126)');
    ok(!/[F]F$/, 'bug 20010618.006 (#7126)');
}

{
    my $a = 'QaaQaabQaabbQ';
    study $a;
    my @a = split /aab*/, $a;
    is("@a", 'Q Q Q Q', 'split with studied string passed to the regep engine');
}

{
    $_ = "AABBAABB";
    study;
    is(s/AB+/1/ge, 2, 'studied scalar passed to pp_substconst');
    is($_, 'A1A1');
}

{
    $_ = "AABBAABB";
    study;
    is(s/(A)B+/1/ge, 2,
       'studied scalar passed to pp_substconst with RX_MATCH_COPIED() true');
    is($1, 'A');
    is($2, undef);
    is($_, 'A1A1');
}

{
    my @got;
    $a = "ydydydyd";
    $b = "xdx";
    push @got, $_ foreach $a =~ /[^x]d(?{})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 control');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    study $a;
    push @got, $_ foreach $a =~ /[^x]d(?{})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 study $a');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    study $b;
    push @got, $_ foreach $a =~ /[^x]d(?{})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 study $b');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    push @got, $_ foreach $a =~ /[^x]d(?{study $b})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 study $b inside (?{}), nothing studied');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    my $c = 'zz';
    study $c;
    push @got, $_ foreach $a =~ /[^x]d(?{study $b})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 study $b inside (?{}), $c studied');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    study $a;
    push @got, $_ foreach $a =~ /[^x]d(?{study $b})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 study $b inside (?{}), $a studied');

    @got = ();
    $a = "ydydydyd";
    $b = "xdx";
    study $a;
    push @got, $_ foreach $a =~ /[^x]d(?{$a .= ''})[^x]d/g;
    is("@got", 'ydyd ydyd', '#92696 $a .= \'\' inside (?{}), $a studied');
}
