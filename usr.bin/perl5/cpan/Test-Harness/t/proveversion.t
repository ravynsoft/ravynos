#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

sub _has_TAP_Formatter_HTML {
    eval "use TAP::Formatter::HTML 0.10";
    #https://rt.cpan.org/Ticket/Display.html?id=74364
    return $@ ? 0 : 1;
}

use strict;
use warnings;
use Test::More tests => 1;
use IO::c55Capture;    # for util

SKIP: {
    skip "requires TAP::Formatter::HTML 0.10", 1 unless _has_TAP_Formatter_HTML();

    my $ans = util::stdout_of(
        sub {
            system( $^X,
                "bin/prove",
                "-l",
                "--formatter=TAP::Formatter::HTML",
                "--tapversion=13",
                "t/sample-tests/simple_yaml_missing_version13"
            ) and die "error $?";
        }
    );
    like(
        $ans, qr/li class="yml"/,
        "prove --tapversion=13 simple_yaml_missing_version13"
    );
}
