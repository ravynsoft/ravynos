package ExtUtils::CBuilder::Platform::Windows::BCC;

our $VERSION = '0.280238'; # VERSION

use strict;
use warnings;

sub format_compiler_cmd {
  my ($self, %spec) = @_;

  foreach my $path ( @{ $spec{includes} || [] },
                     @{ $spec{perlinc}  || [] } ) {
    $path = '-I' . $path;
  }

  %spec = $self->write_compiler_script(%spec)
    if $spec{use_scripts};

  return [ grep {defined && length} (
    $spec{cc}, '-c'         ,
    @{$spec{includes}}      ,
    @{$spec{cflags}}        ,
    @{$spec{optimize}}      ,
    @{$spec{defines}}       ,
    @{$spec{perlinc}}       ,
    "-o$spec{output}"       ,
    $spec{source}           ,
  ) ];
}

sub write_compiler_script {
  my ($self, %spec) = @_;

  my $script = File::Spec->catfile( $spec{srcdir},
                                    $spec{basename} . '.ccs' );

  $self->add_to_cleanup($script);

  print "Generating script '$script'\n" if !$self->{quiet};

  my $SCRIPT = IO::File->new( ">$script" )
    or die( "Could not create script '$script': $!" );

  # XXX Borland "response files" seem to be unable to accept macro
  # definitions containing quoted strings. Escaping strings with
  # backslash doesn't work, and any level of quotes are stripped. The
  # result is a floating point number in the source file where a
  # string is expected. So we leave the macros on the command line.
  print $SCRIPT join( "\n",
    map { ref $_ ? @{$_} : $_ }
    grep defined,
    delete(
      @spec{ qw(includes cflags optimize perlinc) } )
  );

  push @{$spec{includes}}, '@"' . $script . '"';

  return %spec;
}

sub format_linker_cmd {
  my ($self, %spec) = @_;

  foreach my $path ( @{$spec{libpath}} ) {
    $path = "-L$path";
  }

  push( @{$spec{startup}}, 'c0d32.obj' )
    unless ( $spec{startup} && @{$spec{startup}} );

  %spec = $self->write_linker_script(%spec)
    if $spec{use_scripts};

  return [ grep {defined && length} (
    $spec{ld}               ,
    @{$spec{lddlflags}}     ,
    @{$spec{libpath}}       ,
    @{$spec{other_ldflags}} ,
    @{$spec{startup}}       ,
    @{$spec{objects}}       , ',',
    $spec{output}           , ',',
    $spec{map_file}         , ',',
    $spec{libperl}          ,
    @{$spec{perllibs}}      , ',',
    $spec{def_file}
  ) ];
}

sub write_linker_script {
  my ($self, %spec) = @_;

  # To work around Borlands "unique" commandline syntax,
  # two scripts are used:

  my $ld_script = File::Spec->catfile( $spec{srcdir},
                                       $spec{basename} . '.lds' );
  my $ld_libs   = File::Spec->catfile( $spec{srcdir},
                                       $spec{basename} . '.lbs' );

  $self->add_to_cleanup($ld_script, $ld_libs);

  print "Generating scripts '$ld_script' and '$ld_libs'.\n" if !$self->{quiet};

  # Script 1: contains options & names of object files.
  my $LD_SCRIPT = IO::File->new( ">$ld_script" )
    or die( "Could not create linker script '$ld_script': $!" );

  print $LD_SCRIPT join( " +\n",
    map { @{$_} }
    grep defined,
    delete(
      @spec{ qw(lddlflags libpath other_ldflags startup objects) } )
  );

  # Script 2: contains name of libs to link against.
  my $LD_LIBS = IO::File->new( ">$ld_libs" )
    or die( "Could not create linker script '$ld_libs': $!" );

  print $LD_LIBS join( " +\n",
     (delete $spec{libperl}  || ''),
    @{delete $spec{perllibs} || []},
  );

  push @{$spec{lddlflags}}, '@"' . $ld_script  . '"';
  push @{$spec{perllibs}},  '@"' . $ld_libs    . '"';

  return %spec;
}

1;


