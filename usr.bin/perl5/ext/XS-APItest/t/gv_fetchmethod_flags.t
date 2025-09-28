#!perl

use strict;
use warnings;

use Test::More tests => 24;

use_ok('XS::APItest');

sub method { 1 }

ok !XS::APItest::gv_fetchmethod_flags_type(\%::, "nothing", 1, 0);

for my $type ( 1..3 ) {
    is XS::APItest::gv_fetchmethod_flags_type(\%::, "method", $type, 0), "*main::method", "Sanity check";
}

ok !XS::APItest::gv_fetchmethod_flags_type(\%::, "method\0not quite!", 1, 0), "gv_fetchmethod_flags_sv() is nul-clean";
ok !XS::APItest::gv_fetchmethod_flags_type(\%::, "method\0not quite!", 3, 0), "gv_fetchmethod_flags_pvn() is nul-clean";

ok XS::APItest::gv_fetchmethod_flags_type(\%::, "method\0not quite!", 0, 0), "gv_fetchmethod_flags() is not nul-clean";
is XS::APItest::gv_fetchmethod_flags_type(\%::, "method\0not quite!", 2, 0), "*main::method", "gv_fetchmethod_flags_pv() is not nul-clean";

{
    use utf8;
    use open qw( :utf8 :std );

    package ｍａｉｎ;
    
    sub ｍｅｔｈｏｄ { 1 }
    sub method { 1 }

    my $meth_as_octets =
            "\357\275\215\357\275\205\357\275\224\357\275\210\357\275\217\357\275\204";

    for my $type ( 1..3 ) {
        ::is XS::APItest::gv_fetchmethod_flags_type(\%ｍａｉｎ::, "ｍｅｔｈｏｄ", $type, 0), "*ｍａｉｎ::ｍｅｔｈｏｄ";
        ::ok !XS::APItest::gv_fetchmethod_flags_type(\%ｍａｉｎ::, $meth_as_octets, $type, 0);
        ::is XS::APItest::gv_fetchmethod_flags_type(\%ｍａｉｎ::, "method", $type, 0), "*ｍａｉｎ::method";
        
        {
            no strict 'refs';
            ::ok !XS::APItest::gv_fetchmethod_flags_type(
                            \%{"\357\275\215\357\275\201\357\275\211\357\275\216::"},
                            "ｍｅｔｈｏｄ", $type, 0);
            ::ok !XS::APItest::gv_fetchmethod_flags_type(
                            \%{"\357\275\215\357\275\201\357\275\211\357\275\216::"},
                            "method", $type, 0);
        }
    }
}

# [perl #129267] Buffer overrun when argument name ends with colon and
#                there is a colon past the end.  This used to segv.
XS::APItest::gv_fetchmethod_flags_type(\%::, "method:::::", 4, 7);
                                             # With type 4, 7 is the length
