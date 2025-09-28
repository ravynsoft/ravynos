#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

is( 1 ? 1 : 0, 1, 'compile time, true' );
is( 0 ? 0 : 1, 1, 'compile time, false' );

$x = 1;
is(  $x ? 1 : 0, 1, 'run time, true');
is( !$x ? 0 : 1, 1, 'run time, false');

# This used to SEGV due to deep recursion in Perl_scalar().
# (Actually it only SEGVed with the depth being about 100_000; but
# compiling the nested condition goes quadratic in some way, so I've
# reduced to the count to a manageable size. So this is not so much a
# proper test, as it is a comment on the sort of thing that used to break)

{
    my $e = "1";
    $e = "(\$x ? 1 : $e)" for 1..20_000;
    $e = "\$x = $e";
    eval $e;
    is $@, "", "SEGV in Perl_scalar";
}


done_testing();
