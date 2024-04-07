package ExtUtils::CBuilder::Platform::darwin;

use warnings;
use strict;
use ExtUtils::CBuilder::Platform::Unix;
use Config;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

my ($osver) = split /\./, $Config{osvers};
my $apple_cor = $^X eq "/usr/bin/perl" && $osver >= 18;

sub compile {
  my $self = shift;
  my $cf = $self->{config};

  # -flat_namespace isn't a compile flag, it's a linker flag.  But
  # it's mistakenly in Config.pm as both.  Make the correction here.
  local $cf->{ccflags} = $cf->{ccflags};
  $cf->{ccflags} =~ s/-flat_namespace//;

  # XCode 12 makes this fatal, breaking tons of XS modules
  $cf->{ccflags} .= ($cf->{ccflags} ? ' ' : '').'-Wno-error=implicit-function-declaration';

  $self->SUPER::compile(@_);
}

sub arg_include_dirs {
    my $self = shift;

    if ($apple_cor) {
        my $perl_inc = $self->perl_inc;
        return map {
           $_ eq $perl_inc ? ("-iwithsysroot", $_ ) : "-I$_"
        } @_;
    }
    else {
        return $self->SUPER::arg_include_dirs(@_);
    }
}

1;
