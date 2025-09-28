#!perl

use strict;
use warnings;

use Test::More tests => 31;

use_ok('XS::APItest');

my $method = 0;
my @types  = map { 'gv_autoload' . $_ } qw( 4 _sv _pv _pvn );

sub AUTOLOAD {
    our $AUTOLOAD;
    my ($subname, $message) = @_;
    is $subname, $AUTOLOAD, $message;
}

my $sub = "nothing";

ok my $glob = XS::APItest::gv_autoload_type(\%::, $sub, 1, $method);
*{$glob}{CODE}->( __PACKAGE__ . "::" . $sub, '$AUTOLOAD set correctly' );

$sub = "some_sub";
for my $type ( 0..3 ) {
    is $glob = XS::APItest::gv_autoload_type(\%::, $sub, $type, $method), "*main::AUTOLOAD", "*main::AUTOLOAD if autoload is true in $types[$type].";
    *{$glob}{CODE}->( __PACKAGE__ . "::" . $sub, '$AUTOLOAD set correctly' );
}

$sub = "method\0not quite!";

ok $glob = XS::APItest::gv_autoload_type(\%::, $sub, 0, $method);
*{$glob}{CODE}->( __PACKAGE__ . "::" . $sub, "gv_autoload4() is nul-clean");

ok $glob = XS::APItest::gv_autoload_type(\%::, $sub, 1, $method);
*{$glob}{CODE}->( __PACKAGE__ . "::" . $sub, "gv_autoload_sv() is nul-clean");

ok $glob = XS::APItest::gv_autoload_type(\%::, $sub, 2, $method);
*{$glob}{CODE}->( __PACKAGE__ . "::" . ($sub =~ s/\0.*//r), "gv_autoload_pv() is not nul-clean");

ok $glob = XS::APItest::gv_autoload_type(\%::, $sub, 3, $method);
*{$glob}{CODE}->( __PACKAGE__ . "::" . $sub, "gv_autoload_pvn() is nul-clean");

{
    use utf8;
    use open qw( :utf8 :std );

    package ｍａｉｎ;

    sub AUTOLOAD {
        our $AUTOLOAD;
        my ($subname, $message) = @_;
        ::is $subname, $AUTOLOAD, $message;
    }

    for my $type ( 1..3 ) {
        ::ok $glob = XS::APItest::gv_autoload_type(\%ｍａｉｎ::, $sub = "ｍｅｔｈｏｄ", $type, $method);
        *{$glob}{CODE}->( "ｍａｉｎ::" . $sub, "$types[$type]() is UTF8-clean when both the stash and the sub are in UTF-8");
        ::ok $glob = XS::APItest::gv_autoload_type(\%ｍａｉｎ::, $sub = "method", $type, $method);
        *{$glob}{CODE}->( "ｍａｉｎ::" . $sub, "$types[$type]() is UTF8-clean when only the stash is in UTF-8");
    }
}
