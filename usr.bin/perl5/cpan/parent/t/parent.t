#!/usr/bin/perl -w

BEGIN {
   if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        chdir '../lib/parent';
        @INC = '..';
    }
}

use strict;
use Test::More tests => 9;

use_ok('parent');


package No::Version;

our $Foo;
sub VERSION { 42 }

package Test::Version;

use parent -norequire, 'No::Version';
::is( $No::Version::VERSION, undef,          '$VERSION gets left alone' );

# Test Inverse: parent.pm should not clobber existing $VERSION
package Has::Version;

BEGIN { $Has::Version::VERSION = '42' };

package Test::Version2;

use parent -norequire, 'Has::Version';
::is( $Has::Version::VERSION, 42 );

package main;

my $eval1 = q{
  {
    package Eval1;
    {
      package Eval2;
      use parent -norequire, 'Eval1';
      $Eval2::VERSION = "1.02";
    }
    $Eval1::VERSION = "1.01";
  }
};

eval $eval1;
is( $@, '' );

# String comparisons, just to be safe from floating-point errors
is( $Eval1::VERSION, '1.01' );

is( $Eval2::VERSION, '1.02' );

my $expected= q{/^Can't locate reallyReAlLyNotexists.pm in \@INC \(\@INC[\w ]+:/};
$expected= q{/^Can't locate reallyReAlLyNotexists.pm in \@INC \(you may need to install the reallyReAlLyNotexists module\) \(\@INC[\w ]+:/}
    if 5.017005 <= $];

eval q{use parent 'reallyReAlLyNotexists'};
like( $@, $expected, 'baseclass that does not exist');

eval q{use parent 'reallyReAlLyNotexists'};
like( $@, $expected, '  still failing on 2nd load');

{
    BEGIN { $Has::Version_0::VERSION = 0 }

    package Test::Version3;

    use parent -norequire, 'Has::Version_0';
    ::is( $Has::Version_0::VERSION, 0, '$VERSION==0 preserved' );
}
