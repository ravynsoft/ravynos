#!perl -wT
use strict;
use Test::More tests => 1;

use_ok( 'Sys::Syslog' );

diag( "Testing Sys::Syslog $Sys::Syslog::VERSION, Perl $], $^X" )
    unless $ENV{PERL_CORE};
