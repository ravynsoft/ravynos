#!./perl -w

BEGIN {
    unshift @INC, 't';
    require Config;
    if ( ( $Config::Config{'extensions'} !~ /\bB\b/ ) ) {
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
    require 'test.pl';
}
plan 1;

# RT #126410 = used to coredump when doing SvSTASH on %version::

TODO: {
    fresh_perl_is(
        'use B; version->new("v5.22.0"); $s = B::svref_2object(\%version::); $s->SvSTASH; print "ok\n"',
        "ok\n", { stderr => 1 }, 'RT #126410 - SvSTASH against %version::'
    );
}
