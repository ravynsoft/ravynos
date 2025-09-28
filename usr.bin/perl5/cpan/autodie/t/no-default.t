#!/usr/bin/perl

package foo;
use warnings;
use strict;
use Test::More tests => 2;
use autodie;


use_system();
ok("system() works with a lexical 'no autodie' block (github issue #69");
break_system();

sub break_system {
    no autodie;
    open(my $fh, "<", 'NONEXISTENT');
    ok("survived failing open");
}

sub use_system {
    system($^X, '-e' , 1);
}
1;
