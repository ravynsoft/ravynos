#!./perl

# On platforms that don't support all of the filetest operators the code
# that faked the results of missing tests used to leave the test's
# argument on the stack.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

my @ops = split //, 'rwxoRWXOezsfdlpSbctugkTMBAC';

plan( tests => @ops * 5 + 1 );

package o { use overload '-X' => sub { 1 } }
my $o = bless [], 'o';

for my $op (@ops) {
    ok( 1 == @{ [ eval "-$op 'TEST'" ] }, "-$op returns single value" );
    ok( 1 == @{ [ eval "-$op *TEST" ] }, "-$op *gv returns single value" );

    my $count = 0;
    my $t;
    for my $m ("a", "b") {
	if ($count == 0) {
	    $t = eval "-$op _" ? 0 : "foo";
	}
	elsif ($count == 1) {
	    is($m, "b", "-$op did not remove too many values from the stack");
	}
	$count++;
    }

    $count = 0;
    for my $m ("c", "d") {
	if ($count == 0) {
	    $t = eval "-$op -e \$^X" ? 0 : "bar";
	}
	elsif ($count == 1) {
	    is($m, "d", "-$op -e \$^X did not remove too many values from the stack");
	}
	$count++;
    }

    my @foo = eval "-$op \$o";
    is @foo, 1, "-$op \$overld did not leave \$overld on the stack";
}

{
    # [perl #129347] cope with stacked filetests where PL_op->op_next is null
    () = sort { -d -d } \*TEST0, \*TEST1;
    ok 1, "survived stacked filetests with null op_next";
}
