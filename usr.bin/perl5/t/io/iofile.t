#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load IO::File");
}

$|  = 1;
use warnings;
use Config;

plan tests => 3;

# this is essentially the same as a test on a lexical filehandle in
# t/io/open.t, but done in a separate test process against a standard
# filehandle

# check that we can call methods on filehandles auto-magically
# and have IO::File loaded for us
{
    is( $INC{'IO/File.pm'}, undef, "IO::File not loaded" );
    ok( eval { STDOUT->autoflush(1); 1 }, 'STDOUT->autoflush(1) lives' );
    ok( $INC{'IO/File.pm'}, "IO::File now loaded" );
}
