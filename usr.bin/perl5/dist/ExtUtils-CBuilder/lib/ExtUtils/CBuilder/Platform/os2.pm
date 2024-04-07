package ExtUtils::CBuilder::Platform::os2;

use warnings;
use strict;
use ExtUtils::CBuilder::Platform::Unix;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Platform::Unix);

sub need_prelink { 1 }

sub prelink {
  # Generate import libraries (XXXX currently near .DEF; should be near DLL!)
  my $self = shift;
  my %args = @_;

  my @res = $self->SUPER::prelink(%args);
  die "Unexpected number of DEF files" unless @res == 1;
  die "Can't find DEF file in the output"
    unless $res[0] =~ m,^(.*)\.def$,si;
  my $libname = "$1$self->{config}{lib_ext}";	# Put .LIB file near .DEF file
  $self->do_system('emximp', '-o', $libname, $res[0]) or die "emxexp: res=$?";
  return (@res, $libname);
}

sub _do_link {
  my $self = shift;
  my ($how, %args) = @_;
  if ($how eq 'lib_file'
      and (defined $args{module_name} and length $args{module_name})) {

    # Now know the basename, find directory parts via lib_file, or objects
    my $objs = ( (ref $args{objects}) ? $args{objects} : [$args{objects}] );
    my $near_obj = $self->lib_file(@$objs);
    my $exp_dir = ($near_obj =~ m,(.*)[/\\],s ? "$1/" : '' );

    $args{dl_file} = $1 if $near_obj =~ m,(.*)\.,s; # put ExportList near OBJ

    # XXX _do_link does not have place to put libraries?
    push @$objs, $self->perl_inc() . "/libperl$self->{config}{lib_ext}";
    $args{objects} = $objs;
  }
  # Some 'env' do exec(), thus return too early when run from ksh;
  # To avoid 'env', remove (useless) shrpenv
  local $self->{config}{shrpenv} = '';
  return $self->SUPER::_do_link($how, %args);
}

sub extra_link_args_after_prelink {
  # Add .DEF file to the link line
  my ($self, %args) = @_;

  my @DEF = grep /\.def$/i, @{$args{prelink_res}};
  die "More than one .def files created by 'prelink' stage" if @DEF > 1;
  # XXXX No "$how" argument here, so how to test for dynamic link?
  die "No .def file created by 'prelink' stage"
    unless @DEF or not @{$args{prelink_res}};

  my @after_libs = ($OS2::is_aout ? ()
      : $self->perl_inc() . "/libperl_override$self->{config}{lib_ext}");
  # , "-L", "-lperl"
  (@after_libs, @DEF);
}

sub link_executable {
  # ldflags is not expecting .exe extension given on command line; remove -Zexe
  my $self = shift;
  local $self->{config}{ldflags} = $self->{config}{ldflags};
  $self->{config}{ldflags} =~ s/(?<!\S)-Zexe(?!\S)//;
  return $self->SUPER::link_executable(@_);
}


1;
