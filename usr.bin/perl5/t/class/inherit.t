#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

use v5.36;
use feature 'class';
no warnings 'experimental::class';

{
    class Test1A {
        field $inita = "base";
        method inita { return $inita; }
        field $adja;
        ADJUST { $adja = "base class" }
        method adja { return $adja; }
    }

    class Test1B :isa(Test1A) {
        field $initb = "derived";
        method initb { return $initb; }
        field $adjb;
        ADJUST { $adjb = "derived class" }
        method adjb { return $adjb; }
    }

    my $obj = Test1B->new;
    ok($obj isa Test1B, 'Object is its own class');
    ok($obj isa Test1A, 'Object is also its base class');

    ok(eq_array(\@Test1B::ISA, ["Test1A"]), '@Test1B::ISA is set correctly');

    is($obj->initb, "derived",       'Object has derived class initialised field');
    is($obj->adjb,  "derived class", 'Object has derived class ADJUSTed field');

    can_ok($obj, "inita");
    is($obj->inita, "base",      'Object has base class initialised field');
    can_ok($obj, "adja");
    is($obj->adja, "base class", 'Object has base class ADJUSTed field');

    class Test1C :isa(    Test1A    ) { }

    my $objc = Test1C->new;
    ok($objc isa Test1A, ':isa attribute trims whitespace');
}

{
    class Test2A 1.23 { }

    class Test2B :isa(Test2A 1.0) { } # OK

    ok(!defined eval "class Test2C :isa(Test2A 2.0) {}; 1",
        ':isa() version test can throw');
    like($@, qr/^Test2A version 2\.0 required--this is only version 1\.23 at /,
        'Exception thrown from :isa version test');
}

{
    class Test3A {
        field $x :param;
        method x { return $x; }
    }

    class Test3B :isa(Test3A) {}

    my $obj = Test3B->new(x => "X");
    is($obj->x, "X", 'Constructor params passed through to superclass');
}

{
    class Test4A { }

    class Test4B :isa(Test4A);

    package main;
    my $obj = Test4B->new;
    ok($obj isa Test4A, 'Unit class syntax allows :isa');
}

done_testing;
