#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Test::More;

use Tie::Handle;

{
    package Foo;
    @ISA = qw(Tie::StdHandle);
}

# For backwards compatibility with 5.8.x
ok( Foo->can("TIEHANDLE"), "loading Tie::Handle loads TieStdHandle" );

done_testing();
