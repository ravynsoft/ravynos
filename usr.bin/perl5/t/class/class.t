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
    class Test1 {
        method hello { return "hello, world"; }
    }

    my $obj = Test1->new;
    isa_ok($obj, "Test1", '$obj');

    is($obj->hello, "hello, world", '$obj->hello');
}

# Classes are still regular packages
{
    class Test2 {
        my $ok = "OK";
        sub NotAMethod { return $ok }
    }

    is(Test2::NotAMethod(), "OK", 'Class can contain regular subs');
}

# Classes accept full package names
{
    class Test3::Foo {
        method hello { return "This" }
    }
    is(Test3::Foo->new->hello, "This", 'Class supports fully-qualified package names');
}

# Unit class
{
    class Test4::A;
    method m { return "unit-A" }

    class Test4::B;
    method m { return "unit-B" }

    package main;
    ok(eq_array([Test4::A->new->m, Test4::B->new->m], ["unit-A", "unit-B"]),
        'Unit class syntax works');
}

# Class {BLOCK} syntax parses like package
{
    my $result = "";
    eval q{
        $result .= "a(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
        class Test5 1.23 {
            $result .= "b(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
        }
        $result .= "c(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
    } or die $@;
    is($result, "a(main/main)\nb(Test5/Test5)\nc(main/main)\n",
        'class sets __PACKAGE__ correctly');
    is($Test5::VERSION, 1.23, 'class NAME VERSION { BLOCK } sets $VERSION');
}

# Unit class syntax parses like package
{
    my $result = "";
    eval q{
        $result .= "a(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
        class Test6 4.56;
        $result .= "b(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
        package main;
        $result .= "c(" . __PACKAGE__ . "/" . eval("__PACKAGE__") . ")\n";
    } or die $@;
    is($result, "a(main/main)\nb(Test6/Test6)\nc(main/main)\n",
        'class sets __PACKAGE__ correctly');
    is($Test6::VERSION, 4.56, 'class NAME VERSION; sets $VERSION');
}

done_testing;
