#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

use v5.36;
use feature 'class';
no warnings qw( experimental::class experimental::builtin );

use builtin qw( blessed reftype );

{
    class Test1 {
        field $x :param;
        method x { return $x; }
    }

    my $obj = Test1->new(x => 123);
    is($obj->x, 123, 'Value of $x set by constructor');

    # The following tests aren't really related to construction, just the
    # general nature of object instance refs. If this test file gets too long
    # they could be moved to their own file.
    is(ref $obj, "Test1", 'ref of $obj');
    is(blessed $obj, "Test1", 'blessed of $obj');
    is(reftype $obj, "OBJECT", 'reftype of $obj');

    # num/stringification of object without overload
    is($obj+0, builtin::refaddr($obj), 'numified object');
    like("$obj", qr/^Test1=OBJECT\(0x[[:xdigit:]]+\)$/, 'stringified object' );

    ok(!eval { Test1->new(x => 123, y => 456); 1 }, 'Unrecognised parameter fails');
    like($@, qr/^Unrecognised parameters for "Test1" constructor: y at /,
        'Exception thrown by constructor for unrecogniser parameter');
}

{
    class Test2 {
        use overload
            '0+' => sub { return 12345 },
            '""' => sub { "<Test2 instance>" },
            fallback => 1;
    }

    my $obj = Test2->new;
    is($obj+0, 12345, 'numified object with overload');
    is("$obj", "<Test2 instance>", 'stringified object with overload' );
}

done_testing;
