package t::Markers;

push @XS::APItest::bhkav, "run/pm";

use t::BHK push => "compile/pm/before";
sub import {
    use t::BHK push => "compile/pm/inside";
    push @XS::APItest::bhkav, "run/import";
}

use t::BHK push => "compile/pm/after";

1;
