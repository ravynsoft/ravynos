#!/usr/bin/perl

use warnings;
use strict;
use Test::More tests => 17;

use XS::APItest;

BEGIN { push @INC, '.' } # t/BHK.pm is in ext/XS-APItest/ directory

use t::BHK ();      # make sure it gets compiled early

BEGIN { package XS::APItest; *main::bhkav = \@XS::APItest::bhkav }

# 'use t::BHK' switches on recording hooks, and clears @bhkav.
# 'no t::BHK' switches recording off again.
# 'use t::BHK push => "foo"' pushes onto @bhkav

use t::BHK;
    1;
no t::BHK;

BEGIN { is_deeply \@bhkav, [], "no blocks" }

use t::BHK;
    {
        1;
    }
no t::BHK;

BEGIN { is_deeply \@bhkav, 
    [[start => 1], qw/pre_end post_end/], 
    "plain block";
}

use t::BHK;
    if (1) { 1 }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],
        [start => 0],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ], 
    "if block";
}

use t::BHK;
    for (1) { 1 }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],
        [start => 0],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ],
    "for loop";
}

use t::BHK;
    {
        { 1; }
    }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],
        [start => 1],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ],
    "nested blocks";
}

use t::BHK;
    use t::BHK push => "before";
    {
        use t::BHK push => "inside";
    }
    use t::BHK push => "after";
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        "before",
        [start => 1],
        "inside",
        qw/pre_end post_end/,
        "after"
    ],
    "hooks called in the correct places";
}

use t::BHK;
    BEGIN { 1 }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],
        qw/pre_end post_end/,
    ],
    "BEGIN block";
}

use t::BHK; t::BHK->import;
    eval "1";
no t::BHK; t::BHK->unimport;

BEGIN { is_deeply \@bhkav, [], "string eval (compile)" }
is_deeply \@bhkav, 
    [
        [eval => "entereval"],
        [start => 1],
        qw/pre_end post_end/,
    ], 
    "string eval (run)";

delete @INC{qw{t/Null.pm t/Block.pm}};

t::BHK->import;
    do "t/Null.pm";
t::BHK->unimport;

is_deeply \@bhkav,
    [
        [eval => "dofile"],
        [start => 1],
        qw/pre_end post_end/,
    ],
    "do file (null)";

t::BHK->import;
    do "t/Block.pm";
t::BHK->unimport;

is_deeply \@bhkav,
    [
        [eval => "dofile"],
        [start => 1],
        [start => 1],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ],
    "do file (single block)";

delete @INC{qw{t/Null.pm t/Block.pm}};

t::BHK->import;
    require t::Null;
t::BHK->unimport;

is_deeply \@bhkav,
    [
        [eval => "require"],
        [start => 1],
        qw/pre_end post_end/,
    ],
    "require (null)";

t::BHK->import;
    require t::Block;
t::BHK->unimport;

is_deeply \@bhkav,
    [
        [eval => "require"],
        [start => 1],
        [start => 1],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ],
    "require (single block)";

BEGIN { delete $INC{"t/Block.pm"} }

use t::BHK;
    use t::Block;
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [eval => "require"],
        [start => 1],
        [start => 1],
        qw/pre_end post_end/,
        qw/pre_end post_end/,
    ],
    "use (single block)";
}

BEGIN { delete $INC{"t/Markers.pm"} }

use t::BHK;
    use t::BHK push => "compile/main/before";
    use t::Markers;
    use t::BHK push => "compile/main/after";
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        "compile/main/before",
        [eval => "require"],
        [start => 1],
            "compile/pm/before",
            [start => 1],
                "compile/pm/inside",
            qw/pre_end post_end/,
            "compile/pm/after",
        qw/pre_end post_end/,
        "run/pm",
        "run/import",
        "compile/main/after",
    ],
    "use with markers";
}

# OK, now some *really* evil stuff...

BEGIN {
    package EvalDestroy;

    sub DESTROY { $_[0]->() }
}

use t::BHK;
    {
        BEGIN {
            # grumbleSCOPECHECKgrumble
            push @XS::APItest::COMPILE_SCOPE_CONTAINER, 
                bless sub {
                    push @bhkav, "DESTROY";
                }, "EvalDestroy";
        }
        1;
    }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],                   # block
            [start => 1],               # BEGIN
                [start => 1],           # sub
                qw/pre_end post_end/,
            qw/pre_end post_end/,
        "pre_end",
            "DESTROY", 
        "post_end",
    ],
    "compile-time DESTROY comes between pre_ and post_end";
}

use t::BHK;
    {
        BEGIN { 
            push @XS::APItest::COMPILE_SCOPE_CONTAINER, 
                bless sub {
                    eval "{1}";
                }, "EvalDestroy";
        }
        1;
    }
no t::BHK;

BEGIN { is_deeply \@bhkav,
    [
        [start => 1],                   # block
            [start => 1],               # BEGIN
                [start => 1],           # sub
                qw/pre_end post_end/,
            qw/pre_end post_end/,
        "pre_end",
            [eval => "entereval"],
            [start => 1],               # eval
                [start => 1],           # block inside eval
                qw/pre_end post_end/,
            qw/pre_end post_end/,
        "post_end",
    ],
    "evil eval-in-DESTROY tricks";
}
