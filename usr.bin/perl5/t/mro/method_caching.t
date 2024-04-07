#!./perl

BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
no strict 'refs'; # we do a lot of this
use warnings;
no warnings 'redefine'; # we do a lot of this
no warnings 'prototype'; # we do a lot of this

{
    package MCTest::Base;
    sub foo { return $_[1]+1 };

    package MCTest::Derived;
    our @ISA = qw/MCTest::Base/;

    package Foo; our @FOO = qw//;
}

# These are various ways of re-defining MCTest::Base::foo and checking whether the method is cached when it shouldn't be
my @testsubs = (
    sub { is(MCTest::Derived->foo(0), 1); },
    sub { eval 'sub MCTest::Base::foo { return $_[1]+2 }'; is(MCTest::Derived->foo(0), 2); },
    sub { eval 'sub MCTest::Base::foo($) { return $_[1]+3 }'; is(MCTest::Derived->foo(0), 3); },
    sub { eval 'sub MCTest::Base::foo($) { 4 }'; is(MCTest::Derived->foo(0), 4); },
    sub { *MCTest::Base::foo = sub { $_[1]+5 }; is(MCTest::Derived->foo(0), 5); },
    sub { local *MCTest::Base::foo = sub { $_[1]+6 }; is(MCTest::Derived->foo(0), 6); },
    sub { is(MCTest::Derived->foo(0), 5); },
    sub { sub FFF { $_[1]+7 }; local *MCTest::Base::foo = *FFF; is(MCTest::Derived->foo(0), 7); },
    sub { is(MCTest::Derived->foo(0), 5); },
    sub { { local *MCTest::Base::can = sub { "tomatoes" };
            MCTest::Derived->can(0); }
          is(MCTest::Derived->can("isa"), \&UNIVERSAL::isa,
              'removing method when unwinding local *method=sub{}'); },
    sub { sub peas { "peas" }
          { local *MCTest::Base::can = *peas;
            MCTest::Derived->can(0); }
          is(MCTest::Derived->can("isa"), \&UNIVERSAL::isa,
              'removing method when unwinding local *method=*other'); },
    sub { sub DDD { $_[1]+8 }; *MCTest::Base::foo = *DDD; is(MCTest::Derived->foo(0), 8); },
    sub { *ASDF::asdf = sub { $_[1]+9 }; *MCTest::Base::foo = \&ASDF::asdf; is(MCTest::Derived->foo(0), 9); },
    sub { undef *MCTest::Base::foo; eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { eval "sub MCTest::Base::foo($);"; *MCTest::Base::foo = \&ASDF::asdf; is(MCTest::Derived->foo(0), 9); },
    sub { *XYZ = sub { $_[1]+10 }; ${MCTest::Base::}{foo} = \&XYZ; is(MCTest::Derived->foo(0), 10); },
    sub { ${MCTest::Base::}{foo} = sub { $_[1]+11 }; is(MCTest::Derived->foo(0), 11); },

    sub { undef *MCTest::Base::foo; eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MCTest::Base; sub foo { $_[1]+12 }'; is(MCTest::Derived->foo(0), 12); },
    sub { eval 'package ZZZ; sub foo { $_[1]+13 }'; *MCTest::Base::foo = \&ZZZ::foo; is(MCTest::Derived->foo(0), 13); },
    sub { ${MCTest::Base::}{foo} = sub { $_[1]+14 }; is(MCTest::Derived->foo(0), 14); },
    # 5.8.8 fails this one
    sub { undef *{MCTest::Base::}; eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MCTest::Base; sub foo { $_[1]+15 }'; is(MCTest::Derived->foo(0), 15); },
    sub { undef %{MCTest::Base::}; eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MCTest::Base; sub foo { $_[1]+16 }'; is(MCTest::Derived->foo(0), 16); },
    sub { %{MCTest::Base::} = (); eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MCTest::Base; sub foo { $_[1]+17 }'; is(MCTest::Derived->foo(0), 17); },
    # 5.8.8 fails this one too
    sub { *{MCTest::Base::} = *{Foo::}; eval { MCTest::Derived->foo(0) }; like($@, qr/locate object method/); },
    sub { *MCTest::Derived::foo = \&MCTest::Base::foo; eval { MCTest::Derived::foo(0,0) }; ok(!$@); undef *MCTest::Derived::foo },
    sub { eval 'package MCTest::Base; sub foo { $_[1]+18 }'; is(MCTest::Derived->foo(0), 18); },

    # Redefining through a glob alias
    sub { *A = *{'MCTest::Base::foo'}; eval 'sub A { $_[1]+19 }';
          is(MCTest::Derived->foo(0), 19,
            'redefining sub through glob alias via decl'); },
    sub { SKIP: {
              skip_if_miniperl("no XS");
              eval { require XS::APItest; }
                or skip "XS::APItest not available", 1;
              *A = *{'MCTest::Base::foo'};
              XS::APItest::newCONSTSUB(\%main::, "A", 0, 20);
              is (MCTest::Derived->foo(0), 20,
                  'redefining sub through glob alias via newXS');
        } },
    sub { undef *{'MCTest::Base::foo'}; *A = *{'MCTest::Base::foo'};
          eval { no warnings 'once'; local *UNIVERSAL::foo = sub {96};
                 MCTest::Derived->foo };
          ()=\&A;
          eval { MCTest::Derived->foo };
          like($@, qr/Undefined subroutine/,
            'redefining sub through glob alias via stub vivification'); },
    sub { *A = *{'MCTest::Base::foo'};
          local *A = sub { 21 };
          is(MCTest::Derived->foo, 21,
            'redef sub through glob alias via local cv-to-glob assign'); },
    sub { *A = *{'MCTest::Base::foo'};
          eval 'sub MCTest::Base::foo { 22 }';
          { local *A = sub { 23 }; MCTest::Derived->foo }
          is(MCTest::Derived->foo, 22,
            'redef sub through glob alias via localisation unwinding'); },
    sub { *A = *{'MCTest::Base::foo'}; *A = sub { 24 };
          is(MCTest::Derived->foo(0), 24,
            'redefining sub through glob alias via cv-to-glob assign'); },
);

plan(tests => scalar(@testsubs));

$_->() for (@testsubs);
