package ExtUtils::CBuilder::Platform::aix;

use warnings;
use strict;
use ExtUtils::CBuilder::Platform::Unix;
use File::Spec;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

sub need_prelink { 1 }

sub link {
  my ($self, %args) = @_;
  my $cf = $self->{config};

  (my $baseext = $args{module_name}) =~ s/.*:://;
  my $perl_inc = $self->perl_inc();

  # Massage some very naughty bits in %Config
  local $cf->{lddlflags} = $cf->{lddlflags};
  for ($cf->{lddlflags}) {
    s/\Q$(BASEEXT)\E/$baseext/;
    s/\Q$(PERL_INC)\E/$perl_inc/;
  }

  return $self->SUPER::link(%args);
}


1;
