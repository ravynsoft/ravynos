#!/usr/bin/perl -w

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
use Test::More tests => 8;

# Symbol and Class::Struct are both non-XS core modules back to 5.004.
# So they'll always be there.
require_ok("Symbol");
ok( $INC{'Symbol.pm'},          "require_ok MODULE" );

require_ok("Class/Struct.pm");
ok( $INC{'Class/Struct.pm'},    "require_ok FILE" );

# Its more trouble than its worth to try to create these filepaths to test
# through require_ok() so we cheat and use the internal logic.
ok !Test::More::_is_module_name('foo:bar');
ok !Test::More::_is_module_name('foo/bar.thing');
ok !Test::More::_is_module_name('Foo::Bar::');
ok Test::More::_is_module_name('V');
