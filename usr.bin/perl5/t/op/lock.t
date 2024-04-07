#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(. ../lib) );
}
plan tests => 5;

is \lock $foo, \$foo, 'lock returns a scalar argument';
is  lock @foo, \@foo, 'lock returns a ref to its array argument';
is  lock %foo, \%foo, 'lock returns a ref to its hash argument';
is  lock &foo, \&foo, 'lock returns a ref to its code argument';

sub eulavl : lvalue { $x }
is  lock &eulavl, \&eulavl, 'lock returns a ref to its lvalue sub arg';
