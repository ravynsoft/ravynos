require Test::Simple;

use lib 't/lib';
require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();

Test::Simple->import(tests => 5);

ok(0);
ok(0);
ok('');
ok(0);
ok(0);
