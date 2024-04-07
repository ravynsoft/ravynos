#!/usr/bin/perl -w

# check nofork logic on systems which *can* fork()
# NOTE maybe a good candidate for xt/author or something.

BEGIN {
    use lib 't/lib';
}

use strict;
use warnings;

use Config;
use Test::More (
    $Config{d_fork}
    ? 'no_plan'
    : ( 'skip_all' => 'your system already has no fork' )
);
use IO::c55Capture;    # for util

use TAP::Harness;

sub backticks {
    my (@args) = @_;

    util::stdout_of( sub { system(@args) and die "error $?" } );
}

my @libs = map "-I$_", @INC;
my @perl = ( $^X, @libs );
my $mod = 'TAP::Parser::Iterator::Process';

{    # just check the introspective method to start...
    my $code = qq(print $mod->_use_open3 ? 1 : 2);
    {
        my $ans = backticks( @perl, '-MNoFork', "-M$mod", '-e', $code );
        is( $ans, 2, 'says not to fork' );
    }
    {
        local $ENV{PERL5OPT};    # punt: prevent propogating -MNoFork
        my $ans = backticks( @perl, "-M$mod", '-e', $code );
        is( $ans, 1, 'says to fork' );
    }
}

{                                # and make sure we can run a test
    my $capture = IO::c55Capture->new_handle;
    local *STDERR;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            switches  => [ @libs, "-MNoFork" ],
            stdout    => $capture,
        }
    );
    $harness->runtests('t/sample-tests/simple');
    my @output = tied($$capture)->dump;
    is pop @output, "Result: PASS\n", 'status OK';
    pop @output;                 # get rid of summary line
    is( $output[-1], "All tests successful.\n", 'ran with no fork' );
}

# vim:ts=4:sw=4:et:sta
