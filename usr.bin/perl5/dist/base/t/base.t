#!/usr/bin/perl -w

use strict;
use Test::More tests => 15;

use_ok('base');


package No::Version;

our $Foo;
sub VERSION { 42 }

package Test::Version;

use base qw(No::Version);
::ok( ! defined $No::Version::VERSION, '$VERSION bug' );

# Test Inverse of $VERSION bug base.pm should not clobber existing $VERSION
package Has::Version;

BEGIN { $Has::Version::VERSION = '42' };

package Test::Version2;

use base qw(Has::Version);
::is( $Has::Version::VERSION, 42 );

package main;

my $eval1 = q{
  {
    package Eval1;
    {
      package Eval2;
      use base 'Eval1';
      $Eval2::VERSION = "1.02";
    }
    $Eval1::VERSION = "1.01";
  }
};

eval $eval1;
is( $@, '' );

is( $Eval1::VERSION, 1.01 );

is( $Eval2::VERSION, 1.02 );


eval q{use base 'reallyReAlLyNotexists'};
like( $@, qr/^Base class package "reallyReAlLyNotexists" is empty\./,
                                          'base with empty package');

eval q{use base 'reallyReAlLyNotexists'};
like( $@, qr/^Base class package "reallyReAlLyNotexists" is empty\./,
                                          '  still empty on 2nd load');
{
    my $warning;
    local $SIG{__WARN__} = sub { $warning = shift };
    eval q{package HomoGenous; use base 'HomoGenous';};
    like($warning, qr/^Class 'HomoGenous' tried to inherit from itself/,
                                          '  self-inheriting');
}

{
    BEGIN { $Has::Version_0::VERSION = 0 }

    package Test::Version3;

    use base qw(Has::Version_0);
    ::is( $Has::Version_0::VERSION, 0, '$VERSION==0 preserved' );
}


{
    package Schlozhauer;
    use constant FIELDS => 6;

    package Basilisco;
    eval q{ use base 'Schlozhauer' };
    ::is( $@, '', 'Can coexist with a FIELDS constant' );
}

{
    use lib 't/lib';
    package UsingBroken;
    eval q{use base 'Broken';};
    ::like( $@, qr/^Can't locate ThisModuleDoesNotExist\.pm/,
        'base fails to compile by loading nonexistent module');
}

SKIP: {
    skip "unicode not supported on perl $]", 2 if $] < 5.008;
    eval q{
        package UsingUnicode;
        my $base = "M\N{U+00D8}dule";
        no strict 'refs';
        *{"${base}::foo"} = sub {};
        eval q{use base $base;};
        ::is( $@, '', 'nonexistent unicode module allowed');
    };

    eval q{
        package UsingUtf8;
        my $base = "M\N{U+00D8}dule";
        utf8::encode($base);
        no strict 'refs';
        *{"${base}::foo"} = sub {};
        eval q{use base $base;};
        ::is( $@, '', 'nonexistent utf8 module allowed');
    };
}

{
    package WithHostileINC;
    local @INC = (@INC, "a\nb");
    my $base = "NonExistentModule";
    no strict 'refs';
    *{"${base}::foo"} = sub {};
    eval q{use base $base;};
    ::is( $@, '', 'nonexistent module allowed when @INC has hostile entries');
}
