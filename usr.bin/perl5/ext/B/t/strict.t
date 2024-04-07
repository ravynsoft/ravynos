#!./perl -w

use strict;
use warnings;

use Config;
use Test::More;

BEGIN {
    if ( ( $Config{'extensions'} !~ /\bB\b/ ) ) {
        plan skip_all => "Perl was not compiled with B";
        exit 0;
    }
}

use strict;
use warnings;

use B ();
use O ();

foreach my $module (qw/B O/) {
    my $path  = $INC{ $module . '.pm' };
    my $check = "$^X -cw -Mstrict $path 2>&1";
    my $got   = `$check`;
    is( $got, "$path syntax OK\n", "$module.pm compiles without errors" )
      or diag($got);
}

done_testing();
