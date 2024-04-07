package ExtUtils::CBuilder::Platform::dec_osf;

use warnings;
use strict;
use ExtUtils::CBuilder::Platform::Unix;
use File::Spec;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

sub link_executable {
  my $self = shift;
  # $Config{ld} is 'ld' but that won't work: use the cc instead.
  local $self->{config}{ld} = $self->{config}{cc};
  return $self->SUPER::link_executable(@_);
}

1;
