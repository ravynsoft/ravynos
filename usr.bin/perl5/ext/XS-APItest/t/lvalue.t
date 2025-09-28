# Miscellaneous tests for XS lvalue functions

use warnings;
use strict;

use Test::More tests => 4;

use XS::APItest 'lv_temp_object';


{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };

    # [perl #31946]
    lv_temp_object() = 75;
    like $w, qr/Useless assignment to a temporary at/,
	'warning when assigning to temp returned from XS lv sub';
    undef $w;
    (lv_temp_object()) = 75;
    like $w, qr/Useless assignment to a temporary at/,
	'warning when list-assigning to temp returned from XS lv sub';

    $w = undef;
    {
	package XS::APItest::TempObj;
	use overload '.=' => sub { $::assigned = $_[1] };
    }
    lv_temp_object() .= 63;
    is $::assigned, 63, 'overloaded .= on temp obj returned from lv sub';
    is $w, undef, 'no warning from overloaded .= on temp obj';
}
