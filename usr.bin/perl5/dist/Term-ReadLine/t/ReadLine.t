#!./perl -w
use strict;

package Term::ReadLine::Mock;
our @ISA = 'Term::ReadLine::Stub';
sub ReadLine {'Term::ReadLine::Mock'};
sub readline { "a line" }
sub new      { bless {} }

package main;

use Test::More tests => 15;

BEGIN {
    $ENV{PERL_RL} = 'Mock'; # test against our instrumented class
    use_ok('Term::ReadLine');
}

my $t = new Term::ReadLine 'test term::readline';

ok($t, "made something");

isa_ok($t,          'Term::ReadLine::Mock');

for my $method (qw( ReadLine readline addhistory IN OUT MinLine
                    findConsole Attribs Features new ) ) {
    can_ok($t, $method);
}

is($t->ReadLine,    'Term::ReadLine::Mock', "\$object->ReadLine");
is($t->readline,    'a line',               "\$object->readline");

