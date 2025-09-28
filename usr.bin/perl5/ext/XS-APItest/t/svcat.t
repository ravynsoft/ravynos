#!perl

use Test::More tests => 4;
use XS::APItest;
use utf8;

my $e_acute = chr utf8::unicode_to_native(0xe9);
$_ = "καλοκαίρι";
sv_catpvn($_, " ${e_acute}t$e_acute"); # uses SV_CATBYTES
is $_, "καλοκαίρι été", 'sv_catpvn_flags(utfsv, ... SV_CATBYTES)';
$_ = "${e_acute}t$e_acute";
sv_catpvn($_, " καλοκαίρι"); # uses SV_CATUTF8
is $_, "été καλοκαίρι", 'sv_catpvn_flags(bytesv, ... SV_CATUTF8)';
$_ = "καλοκαίρι";
sv_catpvn($_, " été"); # uses SV_CATUTF8
is $_, "καλοκαίρι été", 'sv_catpvn_flags(utfsv, ... SV_CATUTF8)';
$_ = "${e_acute}t$e_acute";
sv_catpvn($_, " ${e_acute}t$e_acute"); # uses SV_CATBYTES
is $_, "été été", 'sv_catpvn_flags(bytesv, ... SV_CATBYTES)';
