package BaseIncOptional;

BEGIN { package main;
    is $INC[-1], '.', 'trailing dot remains in @INC during optional module load from base';
    ok eval('require t::lib::Dummy'), '... and modules load fine from .' or diag "$@";
    delete $INC{'t/lib/Dummy.pm'};
}

use lib 't/lib/on-head';

push @INC, 't/lib/on-tail';

1;
