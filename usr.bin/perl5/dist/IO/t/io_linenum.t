#!./perl

# test added 29th April 1999 by Paul Johnson (pjcj@transeda.com)
# updated    28th May   1999 by Paul Johnson

my $File;

BEGIN {
    $File = __FILE__;
    require strict; strict->import();
}

use Test::More tests => 12;

use IO::File;

sub lineno
{
  my ($f) = @_;
  my $l;
  $l .= "$. ";
  $l .= $f->input_line_number;
  $l .= " $.";                     # check $. before and after input_line_number
  $l;
}

my $t;

open (F, '<', $File) or die $!;
my $io = IO::File->new($File) or die $!;

<F> for (1 .. 10);
is(lineno($io), "10 0 10");

$io->getline for (1 .. 5);
is(lineno($io), "5 5 5");

<F>;
is(lineno($io), "11 5 11");

$io->getline;
is(lineno($io), "6 6 6");

$t = tell F;                                        # tell F; provokes a warning
is(lineno($io), "11 6 11");

<F>;
is(lineno($io), "12 6 12");

select F;
is(lineno($io), "12 6 12");

<F> for (1 .. 10);
is(lineno($io), "22 6 22");

$io->getline for (1 .. 5);
is(lineno($io), "11 11 11");

$t = tell F;
# We used to have problems here before local $. worked.
# input_line_number() used to use select and tell.  When we did the
# same, that mechanism brise.  It should work now.
is(lineno($io), "22 11 22");

{
  local $.;
  $io->getline for (1 .. 5);
  is(lineno($io), "16 16 16");
}

is(lineno($io), "22 16 22");
