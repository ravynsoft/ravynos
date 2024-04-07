#!/usr/bin/perl -w

use strict;

use Test::More tests => 1;

eval q{
    use strict;
    no warnings; # Suppress a "helpful" warning on STDERR
    use autodie qw(open);
    $open = 1;
};
like($@, qr/Global symbol "\$open" requires explicit package name/,
     'autodie does not break "use strict;"');
