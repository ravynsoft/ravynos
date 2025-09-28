use strict;
use warnings;
# HARNESS-NO-PRELOAD

use Test2::Tools::Tiny;
use Test2::API qw/intercept test2_stack/;

plan 3;

my @warnings;
{
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    require Test::Builder;
};

is(@warnings, 2, "got warnings");

like(
    $warnings[0],
    qr/Test::Builder was loaded after Test2 initialization, this is not recommended/,
    "Warn about late Test::Builder load"
);

like(
    $warnings[1],
    qr/Formatter Test::Builder::Formatter loaded too late to be used as the global formatter/,
    "Got the formatter warning"
);
