use strict;
use warnings;
use Test::More;
use Module::Metadata;
use lib "t/lib/0_2";

plan tests => 4;

require Foo;
is($Foo::VERSION, 0.2, 'affirmed version of loaded module');

my $meta = Module::Metadata->new_from_module("Foo", inc => [ "t/lib/0_1" ] );
is($meta->version, 0.1, 'extracted proper version from scanned module');

is($Foo::VERSION, 0.2, 'loaded module still retains its version');

ok(eval "use Foo 0.2; 1", 'successfully loaded module again')
    or diag 'got exception: ', $@;






