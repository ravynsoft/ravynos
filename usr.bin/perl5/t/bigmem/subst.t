#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
    require './test.pl';
}

use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 4
    or skip_all("Need ~4Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(2);

# [perl #103260] [perl #123071]
my $x;
$x .=" "x4096 for 1..2**30/4096;
is eval { $x =~ s/ /_/g }, 2**30, "subst on long strings" ;
is eval { $x =~ s/_/${\" "}/g },2**30,"subst on long strings (substcont)" ;
