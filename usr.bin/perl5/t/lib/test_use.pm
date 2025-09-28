#!perl -w
# Don't use strict because this is for testing use

package test_use;

sub import {
    shift;
    @got = @_;
}

1;
