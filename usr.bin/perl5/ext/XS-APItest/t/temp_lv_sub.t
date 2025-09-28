#!perl -w

use strict;
use utf8;
use Test::More tests => 5;

require_ok('XS::APItest');

sub make_temp_mg_lv :lvalue {  XS::APItest::TempLv::make_temp_mg_lv($_[0]); }

{
    my $x = "[]";
    eval { XS::APItest::TempLv::make_temp_mg_lv($x) = "a"; };
    is($@, '',    'temp mg lv from xs exception check');
    is($x, '[a]', 'temp mg lv from xs success');
}

{
    my $x = "{}";
    eval { make_temp_mg_lv($x) = "b"; };
    is($@, '',    'temp mg lv from pp exception check');
    is($x, '{b}', 'temp mg lv from pp success');
}

1;
