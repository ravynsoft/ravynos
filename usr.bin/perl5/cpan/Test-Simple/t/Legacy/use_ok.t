#!/usr/bin/perl -w

use strict;
use warnings;

use lib 't/lib';
use Test::More;

note "Basic use_ok"; {
    package Foo::one;
    ::use_ok("Symbol");
    ::ok( defined &gensym,        'use_ok() no args exports defaults' );
}


note "With one arg"; {
    package Foo::two;
    ::use_ok("Symbol", qw(qualify));
    ::ok( !defined &gensym,       '  one arg, defaults overridden' );
    ::ok( defined &qualify,       '  right function exported' );
}


note "Multiple args"; {
    package Foo::three;
    ::use_ok("Symbol", qw(gensym ungensym));
    ::ok( defined &gensym && defined &ungensym,   '  multiple args' );
}


note "Defining constants"; {
    package Foo::four;
    my $warn; local $SIG{__WARN__} = sub { $warn .= shift; };
    ::use_ok("constant", qw(foo bar));
    ::ok( defined &foo, 'constant' );
    ::is( $warn, undef, 'no warning');
}


note "use Module VERSION"; {
    package Foo::five;
    ::use_ok("Symbol", 1.02);
}


note "use Module VERSION does not call import"; {
    package Foo::six;
    ::use_ok("NoExporter", 1.02);
}


{
    package Foo::seven;
    local $SIG{__WARN__} = sub {
        # Old perls will warn on X.YY_ZZ style versions.  Not our problem
        warn @_ unless $_[0] =~ /^Argument "\d+\.\d+_\d+" isn't numeric/;
    };
    ::use_ok("Test::More", 0.47);
}


note "Signals are preserved"; {
    package Foo::eight;
    local $SIG{__DIE__};
    ::use_ok("SigDie");
    ::ok(defined $SIG{__DIE__}, '  SIG{__DIE__} preserved');
}


note "Line numbers preserved"; {
    my $package = "that_cares_about_line_numbers";

    # Store the output of caller.
    my @caller;
    {
        package that_cares_about_line_numbers;

        sub import {
            @caller = caller;
            return;
        }

        $INC{"$package.pm"} = 1;  # fool use into thinking it's already loaded
    }

    ::use_ok($package);
    my $line = __LINE__-1;
    ::is( $caller[0], __PACKAGE__,      "caller package preserved" );
    ::is( $caller[1], __FILE__,         "  file" );
    ::is( $caller[2], $line,            "  line" );
}


note "not confused by functions vs class names"; {
    $INC{"ok.pm"} = 1;
    use_ok("ok");               # ok is a function inside Test::More

    $INC{"Foo/bar.pm"} = 1;
    sub Foo::bar { 42 }
    use_ok("Foo::bar");         # Confusing a class name with a function name
}

done_testing;
