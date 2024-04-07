#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

sub t1;
sub t2 : lvalue;
sub t3 ();
sub t4 ($);
sub t5 {1;}
{
    package P1;
    sub tmc {1;}
    package P2;
    @ISA = 'P1';
}

my $has_t1 = ok( exists &t1, 't1 sub declared' );
SKIP: {
    skip 't1 sub was not declared', 1 if ! $has_t1;
    ok( ! defined &t1, 't1 not defined' );
}

my $has_t2 = ok( exists &t2, 't2 sub declared' );
SKIP: {
    skip 't2 sub was not declared', 1 if ! $has_t2;
    ok( ! defined &t2, 't2 not defined' );
}

my $has_t3 = ok( exists &t3, 't3 sub declared' );
SKIP: {
    skip 't3 sub was not declared', 1 if ! $has_t3;
    ok( ! defined &t3, 't3 not defined' );
}

my $has_t4 = ok( exists &t4, 't4 sub declared' );
SKIP: {
    skip 't4 sub was not declared', 1 if ! $has_t4;
    ok( ! defined &t4, 't4 not defined' );
}

my $has_t5 = ok( exists &t5, 't5 sub declared' );
SKIP: {
    skip 't5 sub was not declared', 1 if ! $has_t5;
    ok( defined &t5, , 't5 defined' );
}

my $has_p2_tmc = ok(! exists &P2::tmc, 'P2::tmc not declared, it was inherited');
SKIP: {
    skip 'P2::tmc sub was not declared', 1 if ! $has_t5;
    ok( ! defined &P2::tmc, 'P2::tmc not defined' );
}

my $ref;
$ref->{A}[0] = \&t4;
my $ref_exists = ok( exists &{$ref->{A}[0]}, 'references to subroutines exist');
SKIP: {
    skip 1, 'Reference sub is not considered declared', 1 if ! $ref_exists;
    ok( ! defined &{$ref->{A}[0]}, 'Reference to a sub is not defined' );
}

my $p1_tmc_exists = ok( exists &P1::tmc, 'test setup check');
SKIP: {
    skip 'Setup P1::tmc sub is not considered declared', 1 if ! $p1_tmc_exists;
    ok( defined P1::tmc, 'Setup sub is defined' );
}

undef &P1::tmc;
$p1_tmc_exists = ok( exists &P1::tmc, 'P1::tmc was once defined, and continues to be after being undeffed');
SKIP: {
    skip( 'Sub P1::tmc still exists after having undef called on it', 1) if ! $p1_tmc_exists;
    ok( ! defined &P1::tmc, 'P1::tmc is not longer defined after undef was called on it' );
}

eval 'exists &t5()';
like( $@, qr/not a subroutine name/, 'exists takes subroutine names with no argument list');

done_testing();

exit 0;
