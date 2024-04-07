require Test::Simple;

push @INC, 't/lib';
require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();

require Dev::Null;

Test::Simple->import(tests => 5);
tie *STDERR, 'Dev::Null';

ok(1);
ok(1);
ok(1);
$! = 0;
die "This is a test";
