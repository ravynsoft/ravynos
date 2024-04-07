#!/usr/bin/perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load attributes");
}

plan tests => 4;

# test for bug #38475: parsing errors with multiline attributes

package Antler;

use Attribute::Handlers;

sub TypeCheck :ATTR(CODE,RAWDATA) {
    ::ok(1);
}

sub WrongAttr :ATTR(CODE,RAWDATA) {
    ::ok(0);
}

sub CheckData :ATTR(RAWDATA) {
    # check that the $data element contains the given attribute parameters.

    if ($_[4] eq "12, 14") {
        ::ok(1)
    }
    else {
        ::ok(0)
    }
}

sub CheckEmptyValue :ATTR() {
    if (not defined $_[4]) {
        ::ok(1)
    }
    else {
        ::ok(0)
    }
}

package Deer;
use base 'Antler';

sub something : TypeCheck(
    QNET::Util::Object,
    QNET::Util::Object,
    QNET::Util::Object
) { #           WrongAttr (perl tokenizer bug)
    # keep this ^ lined up !
    return 42;
}

something();

sub c :CheckData(12, 14) {};

sub d1 :CheckEmptyValue() {};
sub d2 :CheckEmptyValue {};
