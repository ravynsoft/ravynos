#!./perl
#
# Check that certain modules don't get loaded when other modules are used.
#

BEGIN {
    chdir 't' if -d 't';
    @INC = qw(. ../lib);
}

use strict;
use warnings;

require "./test.pl";

#
# Format: [Module-that-should-not-be-loaded => modules to test]
#

foreach my $test ([Carp  => qw(warnings Exporter)],
		 ) {
    my ($exclude, @modules) = @$test;

    foreach my $module (@modules) {
        my $prog = <<"        --";
            use $module;
            print exists \$INC {'$exclude.pm'} ? "not ok" : "ok";
        --
        fresh_perl_is ($prog, "ok", {}, "$module does not load $exclude");
    }
}

done_testing();
