package BaseIncMandatory;

BEGIN { package main;
    is $INC[-1], '.', 'trailing dot remains in @INC during mandatory module load from base';
    ok eval('require t::lib::Dummy'), '... and modules load fine from .' or diag "$@";
    delete $INC{'t/lib/Dummy.pm'};
}

1;
