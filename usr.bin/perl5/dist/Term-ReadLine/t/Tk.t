#!perl

use Test::More;

eval "use Tk; 1" or
    plan skip_all => "Tk is not installed.";

# seeing as the entire point of this test is to test the event handler,
# we need to mock as little as possible.  To keep things tightly controlled,
# we'll use the Stub directly.
BEGIN {
    $ENV{PERL_RL} = 'Stub o=0';
}

my $mw;
eval {
    use File::Spec;
    $mw = MainWindow->new(); $mw->withdraw();
    1;
} or plan skip_all => "Tk can't start. DISPLAY not set?";

# need to delay this so that Tk is loaded first.
require Term::ReadLine;

plan tests => 3;

my $t = Term::ReadLine->new('Tk');
ok($t, "Created object");
is($t->ReadLine, 'Term::ReadLine::Stub', 'Correct type');
$t->tkRunning(1);

my $text = 'some text';
my $T = $text . "\n";

my $w = Tk::after($mw,0,
                  sub {
                      pass("Event loop called");
                      exit 0;
                  });

my $result = $t->readline('Do not press enter>');
fail("Should not get here.");
