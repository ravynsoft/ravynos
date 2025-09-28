#!./perl

BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require q(./test.pl);
    set_up_inc('../lib');
}

use strict;
use warnings;

use utf8;
use open qw( :utf8 :std );

plan(tests => 1);

require mro;

=pod

This example is take from: http://www.python.org/2.3/mro.html

"Serious order disagreement" # From Guido
class O: pass
class X(O): pass
class Y(O): pass
class A(X,Y): pass
class B(Y,X): pass
try:
    class Z(A,B): pass #creates Z(A,B) in Python 2.2
except TypeError:
    pass # Z(A,B) cannot be created in Python 2.3

=cut

{
    package ẋ;
    
    package Ƴ;
    
    package ẋƳ;
    our @ISA = ('ẋ', 'Ƴ');
    
    package Ƴẋ;
    our @ISA = ('Ƴ', 'ẋ');

    package Ȥ;
    our @ISA = ('ẋƳ', 'Ƴẋ');
}

eval { mro::get_linear_isa('Ȥ', 'c3') };
like($@, qr/^Inconsistent /, '... got the right error with an inconsistent hierarchy');
