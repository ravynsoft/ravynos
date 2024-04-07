# copied over from JSON::XS and modified to use JSON::PP

package JSON::PP::freeze;

1;

package JSON::PP::tojson;

1;

package main;

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 20 };
BEGIN { $^W = 0 } # hate

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $json = JSON::PP->new->convert_blessed->allow_tags->allow_nonref;

ok (1);

sub JSON::PP::tojson::TO_JSON {
   ok (@_ == 1);
   ok (JSON::PP::tojson:: eq ref $_[0]);
   ok ($_[0]{k} == 1);
   7
}

my $obj = bless { k => 1 }, JSON::PP::tojson::;

ok (1);

my $enc = $json->encode ($obj);
ok ($enc eq 7);

ok (1);

sub JSON::PP::freeze::FREEZE {
   ok (@_ == 2);
   ok ($_[1] eq "JSON");
   ok (JSON::PP::freeze:: eq ref $_[0]);
   ok ($_[0]{k} == 1);
   (3, 1, 2)
}

sub JSON::PP::freeze::THAW {
   ok (@_ == 5);
   ok (JSON::PP::freeze:: eq $_[0]);
   ok ($_[1] eq "JSON");
   ok ($_[2] == 3);
   ok ($_[3] == 1);
   ok ($_[4] == 2);
   777
}

$obj = bless { k => 1 }, JSON::PP::freeze::;
$enc = $json->encode ($obj);
ok ($enc eq '("JSON::PP::freeze")[3,1,2]');

my $dec = $json->decode ($enc);
ok ($dec eq 777);

ok (1);

