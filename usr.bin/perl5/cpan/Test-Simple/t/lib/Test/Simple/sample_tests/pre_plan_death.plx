# ID 20020716.013, the exit code would become 0 if the test died
# before a plan.

require Test::Simple;

push @INC, 't/lib';
require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();

close STDERR;
die "Knife?";

Test::Simple->import(tests => 3);

ok(1);
ok(1);
ok(1);
