#!/usr/bin/perl -w

# Test our simulation of pod2man

use strict;
use warnings;
use lib 't/lib';

use ExtUtils::Command::MM;

use Test::More tests => 3;

# The argument to perm_rw was optional.
# [rt.cpan.org 35190]
{
    my $warnings;
    local $SIG{__WARN__} = sub {
        $warnings .= join '', @_;
    };

    pod2man("--perm_rw");

    like $warnings, qr/Option perm_rw requires an argument/;
};


# Simulate the failure of Pod::Man loading.
# pod2man() should react gracefully.
{
    local @INC = @INC;
    unshift @INC, sub {
        die "Simulated Pod::Man failure\n" if $_[1] eq 'Pod/Man.pm';
    };
    local %INC = %INC;
    delete $INC{"Pod/Man.pm"};

    my $warnings;
    local $SIG{__WARN__} = sub {
        $warnings .= join '', @_;
    };

    ok !pod2man();
    is $warnings, <<'END'
Pod::Man is not available: Simulated Pod::Man failure
Man pages will not be generated during this install.
END

}
