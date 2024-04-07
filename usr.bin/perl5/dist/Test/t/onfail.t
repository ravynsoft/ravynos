# -*-perl-*-

use strict;
use Test qw($ntest plan ok $TESTOUT $TESTERR);

BEGIN { plan test => 6, onfail => \&myfail }

our $mycnt = 0;

my $why = "zero != one";
# sneak in a test that Test::Harness wont see
open J, ">", "junk";
$TESTOUT = *J{IO};
$TESTERR = *J{IO};
ok(0, 1, $why);
$TESTOUT = *STDOUT{IO};
$TESTERR = *STDERR{IO};
close J;
unlink "junk";
$ntest = 1;

sub myfail {
    my ($f) = @_;
    ok(@$f, 1);

    my $t = $$f[0];
    ok($$t{diagnostic}, $why);
    ok($$t{'package'}, 'main');
    ok($$t{repetition}, 1);
    ok($$t{result}, 0);
    ok($$t{expected}, 1);
}
