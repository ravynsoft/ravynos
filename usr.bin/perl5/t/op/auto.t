#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib));
}

plan( tests => 47 );

$x = 10000;
cmp_ok(0 + ++$x - 1,'==',10000,'scalar ++x - 1');
cmp_ok(0 + $x-- - 1,'==',10000,'scalar x-- - 1');
cmp_ok(1 * $x,      '==',10000,'scalar 1 * x');
cmp_ok(0 + $x-- - 0,'==',10000,'scalar x-- - 0');
cmp_ok(1 + $x,      '==',10000,'scalar 1 + x');
cmp_ok(1 + $x++,    '==',10000,'scalar 1 + x++');
cmp_ok(0 + $x,      '==',10000,'scalar x');
cmp_ok(0 + --$x + 1,'==',10000,'scalar --x + 1');
cmp_ok(0 + ++$x + 0,'==',10000,'scalar ++x + 0');
cmp_ok($x,          '==',10000,'scalar x final');

$x[0] = 10000;
cmp_ok(0 + ++$x[0] - 1,'==',10000,'aelem ++x - 1');
cmp_ok(0 + $x[0]-- - 1,'==',10000,'aelem x-- - 1');
cmp_ok(1 * $x[0],      '==',10000,'aelem 1 * x');
cmp_ok(0 + $x[0]-- - 0,'==',10000,'aelem x-- - 0');
cmp_ok(1 + $x[0],      '==',10000,'aelem 1 + x');
cmp_ok(1 + $x[0]++,    '==',10000,'aelem 1 + x++');
cmp_ok(0 + $x[0],      '==',10000,'aelem x');
cmp_ok(0 + --$x[0] + 1,'==',10000,'aelem --x + 1');
cmp_ok(0 + ++$x[0] + 0,'==',10000,'aelem ++x + 0');
cmp_ok($x[0],          '==',10000,'aelem x final');

$x{0} = 10000;
cmp_ok(0 + ++$x{0} - 1,'==',10000,'helem ++x - 1');
cmp_ok(0 + $x{0}-- - 1,'==',10000,'helem x-- - 1');
cmp_ok(1 * $x{0},      '==',10000,'helem 1 * x');
cmp_ok(0 + $x{0}-- - 0,'==',10000,'helem x-- - 0');
cmp_ok(1 + $x{0},      '==',10000,'helem 1 + x');
cmp_ok(1 + $x{0}++,    '==',10000,'helem 1 + x++');
cmp_ok(0 + $x{0},      '==',10000,'helem x');
cmp_ok(0 + --$x{0} + 1,'==',10000,'helem --x + 1');
cmp_ok(0 + ++$x{0} + 0,'==',10000,'helem ++x + 0');
cmp_ok($x{0},          '==',10000,'helem x final');

# test magical autoincrement

cmp_ok(++($foo = '99'), 'eq','100','99 incr 100');
cmp_ok(++($foo = "99a"), 'eq','100','99a incr 100');
cmp_ok(++($foo = "99\0a"), 'eq','100','99\0a incr 100');
cmp_ok(++($foo = 'a0'), 'eq','a1','a0 incr a1');
cmp_ok(++($foo = 'Az'), 'eq','Ba','Az incr Ba');
cmp_ok(++($foo = 'zz'), 'eq','aaa','zzz incr aaa');
cmp_ok(++($foo = 'A99'),'eq','B00','A99 incr B00');
cmp_ok(++($foo = 'zi'), 'eq','zj','zi incr zj (EBCDIC i,j non-contiguous check)');
cmp_ok(++($foo = 'zr'), 'eq','zs','zr incr zs (EBCDIC r,s non-contiguous check)');

# test with glob copies

for(qw '$x++ ++$x $x-- --$x') {
  my $x = *foo;
  ok eval "$_; 1", "$_ does not die on a glob copy";
  is $x, /-/ ? -1 : 1, "result of $_ on a glob copy";
}
