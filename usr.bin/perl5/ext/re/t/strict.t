#!./perl

# Most of the strict effects are tested for in t/re/reg_mesgs.t

BEGIN {
        require Config;
        if (($Config::Config{'extensions'} !~ /\bre\b/) ){
                print "1..0 # Skip -- Perl configured without re module\n";
                exit 0;
        }
}

use strict;

use Test::More tests => 10;
BEGIN { require_ok( 're' ); }

{
    my @w;
    no warnings;
    local $SIG{__WARN__};
    BEGIN { $SIG{__WARN__} = sub { push @w, @_ } };
    qr/\b*/;
    BEGIN { is(scalar @w, 0, 'No default-on warnings for qr/\b*/'); }
    BEGIN {undef @w; }

    {
        use re 'strict';
        qr/\b*/;

        BEGIN { is(scalar @w, 1, 'use re "strict" turns on warnings'); }

        BEGIN { undef @w; }

        no re 'strict';
        qr/\b*/;

        BEGIN { is(scalar @w, 0, 'no re "strict" restores warnings state'); }
    }

    BEGIN {undef @w; }
    qr/\b*/;
    BEGIN { is(scalar @w, 0, 'dropping out of "strict" scope reverts warnings default'); }

    {
        use re 'strict';
        qr/\b*/;

        BEGIN { is(scalar @w, 1, 'use re "strict" turns on warnings'); }

        no re 'strict';
        BEGIN {undef @w; }
        qr/\b*/;
        BEGIN { is(scalar @w, 0, 'turning off "strict" scope reverts warnings default'); }
    }

    {
        use warnings 'regexp';
        BEGIN {undef @w; }
        qr/\b*/;
        BEGIN { is(scalar @w, 1, 'use warnings "regexp" works'); }

        use re 'strict';
        BEGIN {undef @w; }
        qr/\b*/;
        BEGIN { is(scalar @w, 1, 'use re "strict" keeps warnings on'); }

        no re 'strict';
        BEGIN {undef @w; }
        qr/\b*/;
        BEGIN { is(scalar @w, 1, 'turning off "strict" scope doesn\'t affect warnings that were already on'); }
    }
}
