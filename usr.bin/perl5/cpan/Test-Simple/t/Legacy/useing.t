BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::More tests => 5;

require_ok('Test::Builder');
require_ok("Test::More");
require_ok("Test::Simple");

{
    package Foo;
    use Test::More import => [qw(ok is can_ok)];
    can_ok('Foo', qw(ok is can_ok));
    ok( !Foo->can('like'),  'import working properly' );
}
