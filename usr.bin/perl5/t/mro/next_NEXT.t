#!/usr/bin/perl

use strict;
use warnings;
use NEXT;

chdir 't' if -d 't';
require './test.pl';
plan(tests => 4);

{
    package Foo;
    use strict;
    use warnings;
    use mro 'c3';
    
    sub foo { 'Foo::foo' }
    
    package Fuz;
    use strict;
    use warnings;
    use mro 'c3';
    use base 'Foo';

    sub foo { 'Fuz::foo => ' . (shift)->next::method }
        
    package Bar;
    use strict;
    use warnings;    
    use mro 'c3';
    use base 'Foo';

    sub foo { 'Bar::foo => ' . (shift)->next::method }
    
    package Baz;
    use strict;
    use warnings;    

    use base 'Bar', 'Fuz';
    
    sub foo { 'Baz::foo => ' . (shift)->NEXT::foo }    
}

is(Foo->foo, 'Foo::foo', '... got the right value from Foo->foo');
is(Fuz->foo, 'Fuz::foo => Foo::foo', '... got the right value from Fuz->foo');
is(Bar->foo, 'Bar::foo => Foo::foo', '... got the right value from Bar->foo');

is(Baz->foo, 'Baz::foo => Bar::foo => Fuz::foo => Foo::foo', '... got the right value using NEXT in a subclass of a C3 class');

