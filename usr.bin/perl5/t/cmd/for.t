#!./perl

print "1..16\n";

for ($i = 0; $i <= 10; $i++) {
    $x[$i] = $i;
}
$y = $x[10];
print "#1	:$y: eq :10:\n";
$y = join(' ', @x);
print "#1	:$y: eq :0 1 2 3 4 5 6 7 8 9 10:\n";
if (join(' ', @x) eq '0 1 2 3 4 5 6 7 8 9 10') {
	print "ok 1\n";
} else {
	print "not ok 1\n";
}

$i = $c = 0;
for (;;) {
	$c++;
	last if $i++ > 10;
}
if ($c == 12) {print "ok 2\n";} else {print "not ok 2\n";}

$foo = 3210;
@ary = (1,2,3,4,5);
foreach $foo (@ary) {
	$foo *= 2;
}
if (join('',@ary) eq '246810') {print "ok 3\n";} else {print "not ok 3\n";}

for (@ary) {
    s/(.*)/ok $1\n/;
}

print $ary[1];

# test for internal scratch array generation
# this also tests that $foo was restored to 3210 after test 3
for (split(' ','a b c d e')) {
	$foo .= $_;
}
if ($foo eq '3210abcde') {print "ok 5\n";} else {print "not ok 5 $foo\n";}

foreach $foo (("ok 6\n","ok 7\n")) {
	print $foo;
}

sub foo {
    for $i (1..5) {
	return $i if $_[0] == $i;
    }
}

print foo(1) == 1 ? "ok" : "not ok", " 8\n";
print foo(2) == 2 ? "ok" : "not ok", " 9\n";
print foo(5) == 5 ? "ok" : "not ok", " 10\n";

sub bar {
    return (1, 2, 4);
}

$a = 0;
foreach $b (bar()) {
    $a += $b;
}
print $a == 7 ? "ok" : "not ok", " 11\n";

$loop_count = 0;
for ("-3" .. "0") {
    $loop_count++;
}
print $loop_count == 4 ? "ok" : "not ok", " 12\n";

# modifying arrays in loops is a no-no
@a = (3,4);
eval { @a = () for (1,2,@a) };
print $@ =~ /Use of freed value in iteration/ ? "ok" : "not ok", " 13\n";

# [perl #30061] double destory when same iterator variable (eg $_) used in
# DESTROY as used in for loop that triggered the destroy

{

    my $x = 0;
    sub X::DESTROY {
	my $o = shift;
	$x++;
	1 for (1);
    }

    my %h;
    $h{foo} = bless [], 'X';
    delete $h{foo} for $h{foo}, 1;
    print $x == 1 ? "ok" : "not ok", " 14 - double destroy, x=$x\n";
}

# [perl #78194] foreach() aliasing op return values
for ("${\''}") {
    print "not " unless \$_ == \$_;
    print 'ok 15 - [perl \#78194] \$_ == \$_ inside for("$x"){...}',
          "\n";
}

# [perl #123286] Lone C-style for in a block messes up the stack
@_ = (1..3, do {for(0;0;){}}, 4..6);
print "not " unless "@_" eq '1 2 3 0 4 5 6';
print "ok 16 - [perl #78194] Lone C-style for in a block\n";
