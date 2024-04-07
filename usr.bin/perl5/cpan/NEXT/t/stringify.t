use warnings;
use strict;
use Test::More tests => 2;

BEGIN { use_ok('NEXT') };


package Foo;

use overload '""' => 'stringify';

use constant BAR => (1..5);

sub new { bless {}, shift }

sub stringify {
    my $self = shift;
    my %result = $self->EVERY::LAST::BAR;
    join '-' => @{ $result{'Foo::BAR'} };
}



package main;

my $foo = Foo->new;
is("$foo", '1-2-3-4-5', 'overloading stringification');

