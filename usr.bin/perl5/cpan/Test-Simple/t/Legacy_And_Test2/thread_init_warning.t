use strict;
use warnings;

use Test2::Util qw/CAN_THREAD/;
BEGIN {
    unless(CAN_THREAD) {
        require Test::More;
        Test::More->import(skip_all => "threads are not supported");
    }
}

use threads;

my @warns;
{
    local $SIG{__WARN__} = sub { push @warns => @_ };
    require Test::More;
}

Test::More::is_deeply(\@warns, [], "No init warning");

Test::More::done_testing();
