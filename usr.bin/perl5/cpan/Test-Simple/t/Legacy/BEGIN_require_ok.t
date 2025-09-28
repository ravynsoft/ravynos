#!/usr/bin/perl -w

# Fixed a problem with BEGIN { use_ok or require_ok } silently failing when there's no
# plan set.  [rt.cpan.org 28345]  Thanks Adriano Ferreira and Yitzchak.

use strict;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use Test::More;

my $result;
BEGIN {
    $result = require_ok("strict");
}

ok $result, "require_ok ran";

done_testing(2);
