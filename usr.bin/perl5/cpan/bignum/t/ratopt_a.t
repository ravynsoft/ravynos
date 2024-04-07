# -*- mode: perl; -*-

###############################################################################

use strict;
use warnings;

use Test::More tests => 3;

my @CLASSES = qw/Math::BigRat/;

# bigrat (bug until v0.15)
use bigrat a => 2;

foreach my $class (@CLASSES) {
    is($class->accuracy(), 2, "$class accuracy = 2");
}

eval { bigrat->import(accuracy => '42') };

is($@, '', 'no error');

foreach my $class (@CLASSES) {
    is($class->accuracy(), 42, "$class accuracy = 42");
}
