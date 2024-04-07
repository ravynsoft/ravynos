require Test::Simple;

push @INC, 't/lib';
require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();

Test::Simple->import(tests => 5);

ok(1);
ok(5, 'yep');
ok(3, 'beer');
ok("wibble", "wibble");
ok(1);
