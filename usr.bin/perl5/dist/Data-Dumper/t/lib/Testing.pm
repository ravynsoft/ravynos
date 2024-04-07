package Testing;
use 5.008_001;
use strict;
use warnings;
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(_dumptostr);
use Carp;

sub _dumptostr {
    my ($obj) = @_;
    return join '', $obj->Dump;
}

1;
