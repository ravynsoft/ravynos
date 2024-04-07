#!perl

# This script tests not only the interface for XS AUTOLOAD routines to find
# out the sub name, but also that that interface does not interfere with
# prototypes, the way it did before 5.15.4.

use strict;
use warnings;

use Test::More tests => 26;

use XS::APItest;

is XS::APItest::AutoLoader::frob(), 'frob', 'name passed to XS AUTOLOAD';
is "XS::APItest::AutoLoader::fr\0b"->(), "fr\0b",
  'name with embedded null passed to XS AUTOLOAD';
is "XS::APItest::AutoLoader::fr\x{1ed9}b"->(), "fr\x{1ed9}b",
  'Unicode name passed to XS AUTOLOAD';

*AUTOLOAD = *XS::APItest::AutoLoader::AUTOLOADp;

is frob(), 'frob', 'name passed to XS AUTOLOAD with proto';
is prototype \&AUTOLOAD, '*$', 'prototype is unchanged';
is "fr\0b"->(), "fr\0b",
  'name with embedded null passed to XS AUTOLOAD with proto';
is prototype \&AUTOLOAD, '*$', 'proto unchanged after embedded-null call';
is "fr\x{1ed9}b"->(), "fr\x{1ed9}b",
  'Unicode name passed to XS AUTOLOAD with proto';
is prototype \&AUTOLOAD, '*$', 'prototype is unchanged after Unicode call';

# Test that the prototype was preserved from the parser’s point of view

ok !eval "sub { ::AUTOLOAD(1) }",
   'parse failure due to AUTOLOAD prototype';
ok eval "sub { ::AUTOLOAD(1,2) }", 'successful parse respecting prototype'
  or diag $@;

package fribble { sub a { return 7 } }
no warnings 'once';
*a = \&AUTOLOAD;
'$'->();
# &a('fribble') will return '$'
# But if intuit_method does not see the (*...) proto, this compiles as
# fribble->a
no strict;
is eval 'a fribble, 3', '$', 'intuit_method sees * in AUTOLOAD proto'
  or diag $@;

# precedence check
# *$ should parse as a list operator, but right now the AUTOLOAD
# sub name is $
is join(" ", eval 'a "b", "c"'), '$',
   'precedence determination respects prototype of AUTOLOAD sub';

{
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    eval 'sub a($){}';
    like $w, qr/^Prototype mismatch: sub main::a \(\*\$\) vs \(\$\)/m,
        'proto warnings respect AUTOLOAD prototypes';
    undef $w;
    *a = \&AUTOLOAD;
    like $w, qr/^Prototype mismatch: sub main::a \(\$\) vs \(\*\$\)/m,
        'GV assignment proto warnings respect AUTOLOAD prototypes';
}


#
# This is a test for AUTOLOAD implemented as an XSUB.
# It tests that $AUTOLOAD is set correctly, including the
# case of inheritance.
#
# Rationale: Due to change ed850460, $AUTOLOAD is not currently set
# for XSUB AUTOLOADs at all.  Instead, as of adb5a9ae the PV of the
# AUTOLOAD XSUB is set to the name of the method. We cruelly test it
# regardless.
#

# First, make sure we have the XS AUTOLOAD available for testing
ok(XS::APItest::AUTOLOADtest->can('AUTOLOAD'), 'Test class ->can AUTOLOAD');

# Used to communicate from the XS AUTOLOAD to Perl land
our $the_method;

# First, set up the Perl equivalent to what we're testing in
# XS so we have a comparison
package PerlBase;
our $AUTOLOAD;
sub AUTOLOAD {
  Test::More::ok(defined $AUTOLOAD);
  return 1 if not defined $AUTOLOAD;
  $main::the_method = $AUTOLOAD;
  return 0;
}

package PerlDerived;
our @ISA = qw(PerlBase);

package Derived;
our @ISA = qw(XS::APItest::AUTOLOADtest);

package main;

# Test Perl AUTOLOAD in base class directly
$the_method = undef;
is(PerlBase->Blah(), 0,
   "Perl AUTOLOAD gets called and returns success");
is($the_method, 'PerlBase::Blah',
   'Scalar set to correct class/method name');

# Test Perl AUTOLOAD in derived class
$the_method = undef;
is(PerlDerived->Boo(), 0,
   'Perl AUTOLOAD on derived class gets called and returns success');
is($the_method, 'PerlDerived::Boo',
   'Scalar set to correct class/method name');

# Test XS AUTOLOAD in base class directly
$the_method = undef;
is(XS::APItest::AUTOLOADtest->Blah(), 0,
     'XS AUTOLOAD gets called and returns success');
is($the_method, 'XS::APItest::AUTOLOADtest::Blah',
     'Scalar set to correct class/method name');

# Test XS AUTOLOAD in derived class directly
$the_method = undef;
is(Derived->Foo(), 0,
     'XS AUTOLOAD gets called and returns success');
is($the_method, 'Derived::Foo',
     'Scalar set to correct class/method name');
