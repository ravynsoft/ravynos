#!./perl -w
use strict;

use Test::More tests => 37;

my $File = 'README';

use IO::File;

my $io = IO::File->new($File);
isa_ok($io, 'IO::File', "Opening $File");

my $line = $io->getline();
like($line, qr/^This is the/, "Read first line");

my ($list, $context) = $io->getline();
is($list, "\n", "Read second line");
is($context, undef, "Did not read third line with getline() in list context");

$line = $io->getline();
like($line, qr/^This distribution/, "Read third line");

my @lines = $io->getlines();
cmp_ok(@lines, '>', 3, "getlines reads lots of lines");
like($lines[-2], qr/^Share and Enjoy!/, "Share and Enjoy!");

$line = $io->getline();
is($line, undef, "geline reads no more at EOF");

@lines = $io->getlines();
is(@lines, 0, "gelines reads no more at EOF");

# And again
$io = IO::File->new($File);
isa_ok($io, 'IO::File', "Opening $File");

$line = $io->getline();
like($line, qr/^This is the/, "Read first line again");

is(eval {
    $line = $io->getline("Boom");
    1;
   }, undef, "eval caught an exception");
like($@, qr/^usage.*getline\(\) at .*\bio_getline\.t line /, 'getline usage');
like($line, qr/^This is the/, '$line unchanged');

is(eval {
    ($list, $context) = $io->getlines("Boom");
    1;
   }, undef, "eval caught an exception");
like($@, qr/^usage.*getlines\(\) at .*\bio_getline\.t line /, 'getlines usage');
is($list, "\n", '$list unchanged');

is(eval {
    $line = $io->getlines();
    1;
   }, undef, "eval caught an exception");
like($@, qr/^Can't call .*getlines in a scalar context.* at .*\bio_getline\.t line /,
     'getlines in scalar context croaks');
like($line, qr/^This is the/, '$line unchanged');

is(eval {
    $io->getlines();
    1;
   }, undef, "eval caught an exception");
like($@, qr/^Can't call .*getlines in a scalar context.* at .*\bio_getline\.t line /,
     'getlines in void context croaks');
like($line, qr/^This is the/, '$line unchanged');

($list, $context) = $io->getlines();
is($list, "\n", "Read second line");
like($context, qr/^This distribution/, "Read third line");

{
    package TiedHandle;

    sub TIEHANDLE {
        return bless ["Tick", "tick", "tick"];
    }

    sub READLINE {
        my $fh = shift;
        die "Boom!"
            unless @$fh;
        return shift @$fh
            unless wantarray;
        return splice @$fh;
    }
}

tie *FH, 'TiedHandle';

is(*FH->getline(), "Tick", "tied handle read works");
($list, $context) = *FH->getline();
is($list, "tick", "tied handle read works in list context 0");
is($context, undef, "tied handle read works in list context 1");
is(*FH->getline(), "tick", "tied handle read works again");
is(eval {
    $line = *FH->getline();
    1;
   }, undef, "eval on tied handle caught an exception");
like($@, qr/^Boom!/,
     'getline on tied handle propagates exception');
like($line, qr/^This is the/, '$line unchanged');

tie *FH, 'TiedHandle';

($list, $context) = *FH->getlines();
is($list, "Tick", "tied handle read works in list context 2");
is($context, "tick", "tied handle read works in list context 3");
is(eval {
    ($list, $context) = *FH->getlines();
    1;
   }, undef, "eval on tied handle caught an exception again");
like($@, qr/^Boom!/,
     'getlines on tied handle propagates exception');
is($list, "Tick", '$line unchanged');
