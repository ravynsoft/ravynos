# -*-perl-*-
use strict;
use Test;
BEGIN { plan tests => 11 }

ok(ok(1));
ok(ok('fixed', 'fixed'));
ok(skip("just testing skip()",0));
ok(undef, undef);
ok(ok 'the brown fox jumped over the lazy dog', '/lazy/');
ok(ok 'the brown fox jumped over the lazy dog', 'm,fox,');
