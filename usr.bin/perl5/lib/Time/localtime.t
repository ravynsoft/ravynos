#!./perl

use Test::More;

my(@times, @methods);
BEGIN {
    @times   = (-2**55, -2**50, -2**33, -2**31-1, -1, 0, 1, 2**31-1, 2**33, 2**50, 2**55, time);
    @methods = qw(sec min hour mday mon year wday yday isdst);

    use_ok Time::localtime;
}

for my $time (@times) {
    my $localtime = localtime $time;          # This is the OO localtime.
    my @localtime = CORE::localtime $time;    # This is the localtime function

    is @localtime, 9, "localtime($time)";
    for my $method (@methods) {
        is $localtime->$method, shift @localtime, "localtime($time)->$method";
    }
}

done_testing();
