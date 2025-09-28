package ExtUtils::CBuilder::Platform::android;

use warnings;
use strict;
use File::Spec;
use ExtUtils::CBuilder::Platform::Unix;
use Config;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

# The Android linker will not recognize symbols from
# libperl unless the module explicitly depends on it.
sub link {
  my ($self, %args) = @_;

  if ($self->{config}{useshrplib} eq 'true') {
    $args{extra_linker_flags} = [
      $self->split_like_shell($args{extra_linker_flags}),
      '-L' . $self->perl_inc(),
      '-lperl',
      $self->split_like_shell($Config{perllibs}),
    ];
  }

  # Several modules on CPAN rather rightfully expect being
  # able to pass $so_file to DynaLoader::dl_load_file and
  # have it Just Work.  However, $so_file will more likely
  # than not be a relative path, and unless the module 
  # author subclasses MakeMaker/Module::Build to modify
  # LD_LIBRARY_PATH, which would be insane, Android's linker
  # won't find the .so
  # So we make this all work by returning an absolute path.
  my($so_file, @so_tmps) = $self->SUPER::link(%args);
  $so_file = File::Spec->rel2abs($so_file);
  return wantarray ? ($so_file, @so_tmps) : $so_file;
}

1;
