#!./perl
#
# tests for default output handle

# DAPM 30/4/10 this area seems to have been undertested. For now, the only
# tests are ensuring things don't crash when PL_defoutgv isn't a GV;
# it probably needs expanding at some point to cover other stuff.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

$|=0;   # test.pl makes it 1, and that conflicts with the below.

plan tests => 22;


my $stdout = *STDOUT;
select($stdout);
$stdout = 1; # whoops, PL_defoutgv no longer a GV!
# XXX It is a GV as of 5.13.7. Is this test file needed any more?

# note that in the tests below, the return values aren't as important
# as the fact that they don't crash

ok print(""), 'print';
ok select(), 'select';

$a = 'fooo';
format STDOUT =
@ @<<
"#", $a
.
ok((write())[0], 'write');

ok($^, '$^');
ok($~, '$~');
ok($=, '$=');
ok($-, '$-');
is($%, 0,      '$%');
is($|, 0,      '$|');
$^ = 1; pass '$^ = 1';
$~ = 1; pass '$~ = 1';
$= = 1; pass '$= = 1';
$- = 1; pass '$- = 1';
$% = 1; pass '$% = 1';
$| = 1; pass '$| = 1';

# test a NULLed GV
my $t = tempfile;
open FOO, ">", $t or die;
select(FOO);
my $io = *FOO{IO};
*FOO = 0;
$^ = 1; pass 'empty GV: $^ = 1';
$~ = 1; pass 'empty GV: $~ = 1';
$= = 1; pass 'empty GV: $= = 1';
$- = 1; pass 'empty GV: $- = 1';
$% = 1; pass 'empty GV: $% = 1';
$| = 1; pass 'empty GV: $| = 1';
close $io;

# Switch to STDERR for this test, so we do not lose our test output
my $stderr = *STDERR;
select($stderr);
$stderr = 1;
ok close(), 'close';
