use strict;
use warnings;

use Test::More tests => 1;

# [perl #132910]

package Foo;
sub can { die }

package main;

use Carp;

eval {
    sub { confess-sins }->(bless[], 'Foo');
};
like $@, qr/^-sins at /;
