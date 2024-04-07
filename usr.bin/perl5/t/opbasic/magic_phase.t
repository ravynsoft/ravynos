#!./perl

use strict;
use warnings;

# Test ${^GLOBAL_PHASE}
#
# Test::More, t/test.pl, etc., assert plans in END, which happens before global
# destruction. We do not want to use those programs/libraries here, so we
# place this file in directory t/opbasic.

BEGIN { print "1..7\n" }

sub ok ($$) {
    print "not " if !$_[0];
    print "ok";
    print " - $_[1]" if defined $_[1];
    print "\n";
}

BEGIN {
    ok ${^GLOBAL_PHASE} eq 'START', 'START';
}

CHECK {
    ok ${^GLOBAL_PHASE} eq 'CHECK', 'CHECK';
}

INIT {
    ok ${^GLOBAL_PHASE} eq 'INIT', 'INIT';
}

ok ${^GLOBAL_PHASE} eq 'RUN', 'RUN';

sub Moo::DESTROY {
    ok ${^GLOBAL_PHASE} eq 'RUN', 'DESTROY is run-time too, usually';
}

my $tiger = bless {}, Moo::;

sub Kooh::DESTROY {
    ok ${^GLOBAL_PHASE} eq 'DESTRUCT', 'DESTRUCT';
}

our $affe = bless {}, Kooh::;

END {
    ok ${^GLOBAL_PHASE} eq 'END', 'END';
}
