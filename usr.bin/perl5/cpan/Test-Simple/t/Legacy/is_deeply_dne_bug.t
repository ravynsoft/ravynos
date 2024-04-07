#!/usr/bin/perl -w

# test for rt.cpan.org 20768
#
# There was a bug where the internal "does not exist" object could get
# confused with an overloaded object.

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use Test::More tests => 2;

{
    package Foo;

    use overload
    'eq' => \&overload_equiv,
    '==' => \&overload_equiv;

    sub new {
        return bless {}, shift;
    }

    sub overload_equiv {
        if (ref($_[0]) ne 'Foo' || ref($_[1]) ne 'Foo') {
            print ref($_[0]), " ", ref($_[1]), "\n";
            die "Invalid object passed to overload_equiv\n";
        }

        return 1; # change to 0 ... makes little difference
    }
}

my $obj1 = Foo->new();
my $obj2 = Foo->new();

eval { is_deeply([$obj1, $obj2], [$obj1, $obj2]); };
is $@, '';

