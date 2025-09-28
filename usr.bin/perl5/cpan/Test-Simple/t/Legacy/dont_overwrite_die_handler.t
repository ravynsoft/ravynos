#!/usr/bin/perl -w
use Config; # To prevent conflict with some strawberry-portable versions

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Carp qw/cluck/;

# Make sure this is in place before Test::More is loaded.
my $started = 0;
my $handler_called;
BEGIN {
    $SIG{__DIE__} = sub { $handler_called++; cluck 'Died early!' unless $started };
}

use Test::More tests => 2;

$started = 1;
ok !eval { die };
is $handler_called, 1, 'existing DIE handler not overridden';
