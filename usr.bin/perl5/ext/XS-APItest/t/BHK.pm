package t::BHK;

sub import   { 
    shift;
    unless (@_) {
        XS::APItest::bhk_record(1);
        return;
    }
    if ($_[0] eq "push") {
        push @XS::APItest::bhkav, $_[1];
        return;
    }
}
sub unimport { XS::APItest::bhk_record(0) }

1;
