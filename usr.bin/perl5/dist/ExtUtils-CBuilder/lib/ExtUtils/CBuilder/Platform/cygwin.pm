package ExtUtils::CBuilder::Platform::cygwin;

use warnings;
use strict;
use File::Spec;
use ExtUtils::CBuilder::Platform::Unix;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

# TODO: If a specific exe_file name is requested, if the exe created
# doesn't have that name, we might want to rename it.  Apparently asking
# for an exe of "foo" might result in "foo.exe".  Alternatively, we should
# make sure the return value is correctly "foo.exe".
# C.f http://rt.cpan.org/Public/Bug/Display.html?id=41003
sub link_executable {
  my $self = shift;
  return $self->SUPER::link_executable(@_);
}

sub link {
  my ($self, %args) = @_;

  my $lib = $self->{config}{useshrplib} ? 'libperl.dll.a' : 'libperl.a';
  $args{extra_linker_flags} = [
    File::Spec->catfile($self->perl_inc(), $lib),
    $self->split_like_shell($args{extra_linker_flags})
  ];

  return $self->SUPER::link(%args);
}

1;
