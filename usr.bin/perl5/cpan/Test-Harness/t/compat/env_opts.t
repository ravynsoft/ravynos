#!/usr/bin/perl -w

use strict;
use warnings;
use Test::More tests => 12;

use Test::Harness;

sub _has_module {
    my $module = shift;
    eval "use $module";
    return $@ ? 0 : 1;
}

{

    # Should add a fake home dir? to test the rc stuff..
    local $ENV{HARNESS_OPTIONS} = 'j4:c';

    ok my $harness = Test::Harness::_new_harness, 'made harness';
    is( $harness->color, 1, "set color correctly" );
    is( $harness->jobs,  4, "set jobs correctly" );
}
SKIP: {
    skip 'Can\'t locate object method "color" via package "TAP::Formatter::HTML" (RT 82738)',4;
    skip "requires TAP::Formatter::HTML", 4
      unless _has_module('TAP::Formatter::HTML');

    local $ENV{HARNESS_OPTIONS} = 'j4:c:fTAP-Formatter-HTML';

    ok my $harness = Test::Harness::_new_harness, 'made harness';
    is( $harness->color, 1, "set color correctly" );
    is( $harness->jobs,  4, "set jobs correctly" );
    is( $harness->formatter_class, "TAP::Formatter::HTML",
        "correct formatter" );

}
SKIP: {
    skip "requires TAP::Harness::Archive", 5
      unless _has_module('TAP::Harness::Archive');

    # Test archive
    local $ENV{HARNESS_OPTIONS} = 'j4:c:a/archive.tgz';

    ok my $harness = Test::Harness::_new_harness, 'made harness';
    is( $harness->color, 1, "set color correctly" );
    is( $harness->jobs,  4, "set jobs correctly" );
    isa_ok( $harness, "TAP::Harness::Archive", "correct harness subclass" );

    # XXX: this is nasty :(
    is( $harness->{__archive_file}, "/archive.tgz", "correct archive found" );

}

