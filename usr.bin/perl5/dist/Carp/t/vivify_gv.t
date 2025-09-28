BEGIN { print "1..5\n"; }

our $has_is_utf8; BEGIN { $has_is_utf8 = exists($utf8::{"is_utf8"}); }
our $has_dgrade; BEGIN { $has_dgrade = exists($utf8::{"downgrade"}); }
our $has_swashnew; BEGIN { $has_swashnew = exists($utf8::{"SWASHNEW"}); }
our $has_strval; BEGIN { $has_strval = exists($overload::{"StrVal"}); }
our $has_sv2obj; BEGIN { $has_sv2obj = exists($B::{"svref_2object"}); }

use Carp;
sub { sub { Carp::longmess("x") }->() }->(\1, "\x{2603}", qr/\x{2603}/);

print !(exists($utf8::{"is_utf8"}) xor $has_is_utf8) ? "" : "not ", "ok 1\n";
print !(exists($utf8::{"downgrade"}) xor $has_dgrade) ? "" : "not ", "ok 2\n";
print !(exists($utf8::{"SWASHNEW"}) xor $has_swashnew) ? "" : "not ", "ok 3\n";
print !(exists($overload::{"StrVal"}) xor $has_sv2obj) ? "" : "not ", "ok 4\n";
print !(exists($B::{"svref_2object"}) xor $has_sv2obj) ? "" : "not ", "ok 5\n";

1;
