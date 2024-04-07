#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config;
    import Config;

}
use strict;

require './test.pl';

plan(4);

like(runperl(switches => ['-Irun/flib', '-Mbroken'], stderr => 1),
     qr/^Global symbol "\$x" requires explicit package name \(did you (?x:
        )forget to declare "my \$x"\?\) at run\/flib\/broken.pm line 6\./,
     "Ensure -Irun/flib produces correct filename in warnings");

like(runperl(switches => ['-Irun/flib/', '-Mbroken'], stderr => 1),
     qr/^Global symbol "\$x" requires explicit package name \(did you (?x:
        )forget to declare "my \$x"\?\) at run\/flib\/broken.pm line 6\./,
     "Ensure -Irun/flib/ produces correct filename in warnings");

SKIP: {
    my $no_pmc;
    foreach(Config::non_bincompat_options()) {
	if($_ eq "PERL_DISABLE_PMC"){
	    $no_pmc = 1;
	    last;
	}
    }

    if ( $no_pmc ) {
        skip('Tests fail without PMC support', 2);
    }

    like(runperl(switches => ['-Irun/flib', '-Mt2'], prog => 'print t2::id()', stderr => 1),
         qr/^t2pmc$/,
         "Ensure -Irun/flib loads pmc");

    like(runperl(switches => ['-Irun/flib/', '-Mt2'], prog => 'print t2::id()', stderr => 1),
         qr/^t2pmc$/,
         "Ensure -Irun/flib/ loads pmc");
}
