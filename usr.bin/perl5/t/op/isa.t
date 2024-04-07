#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

use strict;
use feature 'isa';

plan 14;

package BaseClass {}
package DerivedClass { our @ISA = qw(BaseClass) }
package CustomClass {
   sub isa { length($_[1]) == 9; }
}

my $baseobj = bless {}, "BaseClass";
my $derivedobj = bless {}, "DerivedClass";
my $customobj = bless {}, "CustomClass";

# Bareword package name
ok($baseobj isa BaseClass, '$baseobj isa BaseClass');
ok(not($baseobj isa Another::Class), '$baseobj is not Another::Class');

# String package name
ok($baseobj isa "BaseClass",         '$baseobj isa BaseClass');
ok(not($baseobj isa "DerivedClass"), '$baseobj is not DerivedClass');

ok($derivedobj isa "DerivedClass", '$derivedobj isa DerivedClass');
ok($derivedobj isa "BaseClass",    '$derivedobj isa BaseClass');

# Expression giving a package name
my $classname = "DerivedClass";
ok($derivedobj isa $classname, '$derivedobj isa DerivedClass via SV');

# Invoked on instance which overrides ->isa
ok($customobj isa "Something",          '$customobj isa Something');
ok(not($customobj isa "SomethingElse"), '$customobj isa SomethingElse');

ok(not(undef isa "BaseClass"), 'undef is not BaseClass');
ok(not([] isa "BaseClass"),    'ARRAYref is not BaseClass');

# Test that isa object method still works.

ok($baseobj->isa('BaseClass'), '$baseobj isa BaseClass using object method');
ok(not($baseobj->isa('DerivedClass')), '$baseobj is not BaseClass using object method');
ok($derivedobj->isa('BaseClass'), '$derivedobj isa BaseClass using object method');

# TODO: Consider 
#    LHS = other class
