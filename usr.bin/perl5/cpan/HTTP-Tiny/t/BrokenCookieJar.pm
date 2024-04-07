package BrokenCookieJar;

use strict;
use warnings;

sub new {
    my $class = shift;
    return bless {} => $class;
}

package BrokenCookieJar2;

use strict;
use warnings;

sub new {
    my $class = shift;
    return bless {} => $class;
}

sub add {
}

1;
