#!perl -w
# HARNESS-NO-PRELOAD

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::More tests => 17;

# If we skip with the same name, Test::Harness will report it back and
# we won't get lots of false bug reports.
my $Why = "Just testing the skip interface.";

SKIP: {
    skip $Why, 2 
      unless Pigs->can('fly');

    my $pig = Pigs->new;
    $pig->takeoff;

    ok( $pig->altitude > 0,         'Pig is airborne' );
    ok( $pig->airspeed > 0,         '  and moving'    );
}


SKIP: {
    skip "We're not skipping", 2 if 0;

    pass("Inside skip block");
    pass("Another inside");
}


SKIP: {
    skip "Again, not skipping", 2 if 0;

    my($pack, $file, $line) = caller;
    is( $pack || '', '',      'calling package not interfered with' );
    is( $file || '', '',      '  or file' );
    is( $line || '', '',      '  or line' );
}

SKIP: {
    skip $Why, 2 if 1;

    die "A horrible death";
    fail("Deliberate failure");
    fail("And again");
}


{
    my $warning;
    local $SIG{__WARN__} = sub { $warning = join "", @_ };
    SKIP: {
        # perl gets the line number a little wrong on the first
        # statement inside a block.
        1 == 1;
#line 56
        skip $Why;
        fail("So very failed");
    }
    is( $warning, "skip() needs to know \$how_many tests are in the ".
                  "block at $0 line 56\n",
        'skip without $how_many warning' );
}


SKIP: {
    skip "Not skipping here.", 4 if 0;

    pass("This is supposed to run");

    # Testing out nested skips.
    SKIP: {
        skip $Why, 2;
        fail("AHHH!");
        fail("You're a failure");
    }

    pass("This is supposed to run, too");
}

{
    my $warning = '';
    local $SIG{__WARN__} = sub { $warning .= join "", @_ };

    SKIP: {
        skip 1, "This is backwards" if 1;

        pass "This does not run";
    }

    like $warning, qr/^skip\(\) was passed a non-numeric number of tests/;
}
