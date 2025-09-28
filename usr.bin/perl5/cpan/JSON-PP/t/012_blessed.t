# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 16 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;


my $o1 = bless { a => 3 }, "XX";
my $o2 = bless \(my $dummy = 1), "YY";

sub XX::TO_JSON {
   {'__',""}
}

my $js = JSON::PP->new;

eval { $js->encode ($o1) }; ok ($@ =~ /allow_blessed/);
eval { $js->encode ($o2) }; ok ($@ =~ /allow_blessed/);
$js->allow_blessed;
ok ($js->encode ($o1) eq "null");
ok ($js->encode ($o2) eq "null");
$js->convert_blessed;
ok ($js->encode ($o1) eq '{"__":""}');
ok ($js->encode ($o2) eq "null");

$js->filter_json_object (sub { 5 });
$js->filter_json_single_key_object (a => sub { shift });
$js->filter_json_single_key_object (b => sub { 7 });

ok ("ARRAY" eq ref $js->decode ("[]"));
ok (5 eq join ":", @{ $js->decode ('[{}]') });
ok (6 eq join ":", @{ $js->decode ('[{"a":6}]') });
ok (5 eq join ":", @{ $js->decode ('[{"a":4,"b":7}]') });

$js->filter_json_object;
ok (7 == $js->decode ('[{"a":4,"b":7}]')->[0]{b});
ok (3 eq join ":", @{ $js->decode ('[{"a":3}]') });

$js->filter_json_object (sub { });
ok (7 == $js->decode ('[{"a":4,"b":7}]')->[0]{b});
ok (9 eq join ":", @{ $js->decode ('[{"a":9}]') });

$js->filter_json_single_key_object ("a");
ok (4 == $js->decode ('[{"a":4}]')->[0]{a});

$js->filter_json_single_key_object (a => sub { return; }); # sub {} is not suitable for Perl 5.6
ok (4 == $js->decode ('[{"a":4}]')->[0]{a});

