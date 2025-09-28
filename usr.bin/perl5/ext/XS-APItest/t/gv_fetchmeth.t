#!perl

use strict;
use warnings;

use Test::More tests => 44;

use_ok('XS::APItest');

my $level = -1;
my @types = map { 'gv_fetchmeth' . $_ } '', qw( _sv _pv _pvn );

sub test { "Sanity check" }

for my $type ( 0..3 ) {
    is *{
         XS::APItest::gv_fetchmeth_type(\%::, "test", $type, $level, 0)
        }{CODE}->(), "Sanity check";
}

for my $type ( 0..3 ) {
    my $meth = "gen$type";
    ok !XS::APItest::gv_fetchmeth_type(\%::, $meth, $type, -1, 0), "With level = -1, $types[$type] returns false.";
    ok !$::{$meth}, "...and doesn't vivify the glob.";

    ok !XS::APItest::gv_fetchmeth_type(\%::, $meth, $type, 0, 0), "With level = 0, $types[$type] still returns false.";
    ok $::{$meth}, "...but does vivify the glob.";
}

{
    no warnings 'once';
    *method = sub { 1 };
}

ok !XS::APItest::gv_fetchmeth_type(\%::, "method\0not quite!", 0, $level, 0), "gv_fetchmeth() is nul-clean";
ok !XS::APItest::gv_fetchmeth_type(\%::, "method\0not quite!", 1, $level, 0), "gv_fetchmeth_sv() is nul-clean";
is XS::APItest::gv_fetchmeth_type(\%::, "method\0not quite!", 2, $level, 0), "*main::method", "gv_fetchmeth_pv() is not nul-clean";
ok !XS::APItest::gv_fetchmeth_type(\%::, "method\0not quite!", 3, $level, 0), "gv_fetchmeth_pvn() is nul-clean";

{
    use utf8;
    use open qw( :utf8 :std );

    package ｍａｉｎ;

    sub ｍｅｔｈｏｄ { 1 }

    use constant { φου1 => 1,
                   φου2 => 2,
                   φου3 => 3, };

    my $meth_as_octets =
            "\357\275\215\357\275\205\357\275\224\357\275\210\357\275\217\357\275\204";

    $level = 1;
    for my $type ( 1..3 ) {
        ::is XS::APItest::gv_fetchmeth_type(\%ｍａｉｎ::, "ｍｅｔｈｏｄ", $type, $level, 0), "*ｍａｉｎ::ｍｅｔｈｏｄ", "$types[$type] is UTF-8 clean";
        ::ok !XS::APItest::gv_fetchmeth_type(\%ｍａｉｎ::, $meth_as_octets, $type, $level, 0);
        ::ok !XS::APItest::gv_fetchmeth_type(\%ｍａｉｎ::, "method", $type, $level, 0);
        ::is XS::APItest::gv_fetchmeth_type(\%ｍａｉｎ::, "φου$type", $type, $level, 0), "*ｍａｉｎ::φου$type", "$types[$type] can fetch UTF-8 constant";
        
        {
            no strict 'refs';
            ::ok !XS::APItest::gv_fetchmeth_type(
                            \%{"\357\275\215\357\275\201\357\275\211\357\275\216::"},
                            "ｍｅｔｈｏｄ", $type, $level, 0);
            ::ok !XS::APItest::gv_fetchmeth_type(
                            \%{"\357\275\215\357\275\201\357\275\211\357\275\216::"},
                            "method", $type, $level, 0);
        }
    }
}

{
    @Foo::ISA = qw/Bar/;
    @Bar::ISA = qw//;

    is(XS::APItest::gv_fetchmeth_type(\%Foo::, "nomethod", 1, -1, 0), undef, 'gv_fetchmeth_sv survives @ISA traversal');
}
