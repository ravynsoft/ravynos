#!/usr/bin/perl

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}


use Test::More tests => 3;
use Test::Builder;

my $tb = Test::Builder->create;
sub foo { $tb->croak("foo") }
sub bar { $tb->carp("bar")  }

eval { foo() };
is $@, sprintf "foo at %s line %s.\n", $0, __LINE__ - 1;

eval { $tb->croak("this") };
is $@, sprintf "this at %s line %s.\n", $0, __LINE__ - 1;

{
    my $warning = '';
    local $SIG{__WARN__} = sub {
        $warning .= join '', @_;
    };

    bar();
    is $warning, sprintf "bar at %s line %s.\n", $0, __LINE__ - 1;
}
