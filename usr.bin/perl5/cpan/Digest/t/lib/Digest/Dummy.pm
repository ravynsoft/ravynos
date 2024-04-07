package Digest::Dummy;

use strict;
use warnings;

our $VERSION = 1;
our @ISA     = qw(Digest::base);

require Digest::base;

sub new {
    my $class = shift;
    my $d     = shift || "ooo";
    bless { d => $d }, $class;
}

sub add    { }
sub digest { shift->{d} }

1;

