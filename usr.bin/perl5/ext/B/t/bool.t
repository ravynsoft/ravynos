#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use strict;
use warnings;

use B;
use Test::More;

$|  = 1;

{
    note "testing true";
    my $bool = ( 1 == 1 );
    my $sv = B::svref_2object(\$bool);
    ok $sv->IsBOOL, "got a boolean";
    ok $sv->TRUE_nomg, "TRUE_nomg is true";
    ok $sv->TRUE, "TRUE is true";
}

{
    note "testing false";
    my $bool = ( 1 == 0 );
    my $sv = B::svref_2object(\$bool);

    ok $sv->IsBOOL, "got a boolean";
    ok !$sv->TRUE_nomg, "TRUE_nomg is false";
    ok !$sv->TRUE, "TRUE is false";
}

{
    note "not a boolean";
    my $iv = 42;
    my $sv = B::svref_2object(\$iv);

    ok !$sv->IsBOOL, "not a boolean";
    ok $sv->TRUE_nomg, "TRUE_nomg is true";
    ok $sv->TRUE, "TRUE is true";
}

{
    note "not a boolean";
    my $iv = 0;
    my $sv = B::svref_2object(\$iv);

    ok !$sv->IsBOOL, "not a boolean";
    ok !$sv->TRUE_nomg, "TRUE_nomg is false";
    ok !$sv->TRUE, "TRUE is false";
}

done_testing();
