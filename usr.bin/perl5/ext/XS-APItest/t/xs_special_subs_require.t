#!perl -w
BEGIN {
    # Hush the used only once warning.
    no warnings 'once';
    $XS::APItest::WARNINGS_ON_BOOTSTRAP = 1;
}

use strict;
use warnings;
use Test::More tests => 103;

# Doing this longhand cut&paste makes it clear
# BEGIN and INIT are FIFO, CHECK and END are LIFO
BEGIN {
    print "# First BEGIN\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called");
    is($XS::APItest::INIT_called, undef, "INIT not called");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

CHECK {
    print "# First CHECK\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
    is($XS::APItest::INIT_called, undef, "INIT not called");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

INIT {
    print "# First INIT\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
    is($XS::APItest::INIT_called, undef, "INIT not called");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

END {
    print "# First END\n";
    is($XS::APItest::BEGIN_called, 1, "BEGIN called");
    is($XS::APItest::BEGIN_called_PP, 1, "BEGIN called");
    is($XS::APItest::UNITCHECK_called, 1, "UNITCHECK called");
    is($XS::APItest::UNITCHECK_called_PP, 1, "UNITCHECK called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
    is($XS::APItest::INIT_called, undef, "INIT not called (too late)");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called (too late)");
    is($XS::APItest::END_called, 1, "END called");
    is($XS::APItest::END_called_PP, 1, "END called");
}

print "# First body\n";
is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not called");
is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
is($XS::APItest::INIT_called, undef, "INIT not called (too late)");
is($XS::APItest::INIT_called_PP, undef, "INIT not called (too late)");
is($XS::APItest::END_called, undef, "END not yet called");
is($XS::APItest::END_called_PP, undef, "END not yet called");

{
    my @trap;
    local $SIG{__WARN__} = sub { push @trap, join "!", @_ };
    require XS::APItest;

    @trap = sort @trap;
    is(scalar @trap, 2, "There were 2 warnings");
    like($trap[0], qr "^Too late to run CHECK block");
    like($trap[1], qr "^Too late to run INIT block");
}

print "# Second body\n";
is($XS::APItest::BEGIN_called, 1, "BEGIN called");
is($XS::APItest::BEGIN_called_PP, 1, "BEGIN called");
is($XS::APItest::UNITCHECK_called, 1, "UNITCHECK called");;
is($XS::APItest::UNITCHECK_called_PP, 1, "UNITCHECK called");
is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
is($XS::APItest::INIT_called, undef, "INIT not called (too late)");
is($XS::APItest::INIT_called_PP, undef, "INIT not called (too late)");
is($XS::APItest::END_called, undef, "END not yet called");
is($XS::APItest::END_called_PP, undef, "END not yet called");

BEGIN {
    print "# Second BEGIN\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called");
    is($XS::APItest::INIT_called, undef, "INIT not called");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

CHECK {
    print "# Second CHECK\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not yet called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called");
    is($XS::APItest::INIT_called, undef, "INIT not called");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

INIT {
    print "# Second INIT\n";
    is($XS::APItest::BEGIN_called, undef, "BEGIN not yet called");
    is($XS::APItest::BEGIN_called_PP, undef, "BEGIN not yet called");
    is($XS::APItest::UNITCHECK_called, undef, "UNITCHECK not yet called");
    is($XS::APItest::UNITCHECK_called_PP, undef, "UNITCHECK not yet called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
    is($XS::APItest::INIT_called, undef, "INIT not called (too late)");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called (too late)");
    is($XS::APItest::END_called, undef, "END not yet called");
    is($XS::APItest::END_called_PP, undef, "END not yet called");
}

END {
    print "# Second END\n";
    is($XS::APItest::BEGIN_called, 1, "BEGIN called");
    is($XS::APItest::BEGIN_called_PP, 1, "BEGIN called");
    is($XS::APItest::UNITCHECK_called, 1, "UNITCHECK called");
    is($XS::APItest::UNITCHECK_called_PP, 1, "UNITCHECK called");
    is($XS::APItest::CHECK_called, undef, "CHECK not called (too late)");
    is($XS::APItest::CHECK_called_PP, undef, "CHECK not called (too late)");
    is($XS::APItest::INIT_called, undef, "INIT not called (too late)");
    is($XS::APItest::INIT_called_PP, undef, "INIT not called (too late)");
    is($XS::APItest::END_called, 1, "END called");
    is($XS::APItest::END_called_PP, 1, "END called");
}
