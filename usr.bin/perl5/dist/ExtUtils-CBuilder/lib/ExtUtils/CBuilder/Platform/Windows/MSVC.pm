package ExtUtils::CBuilder::Platform::Windows::MSVC;

our $VERSION = '0.280238'; # VERSION

use warnings;
use strict;

sub arg_exec_file {
  my ($self, $file) = @_;
  return "/OUT:$file";
}

sub format_compiler_cmd {
  my ($self, %spec) = @_;

  foreach my $path ( @{ $spec{includes} || [] },
                     @{ $spec{perlinc}  || [] } ) {
    $path = '-I' . $path;
  }

  %spec = $self->write_compiler_script(%spec)
    if $spec{use_scripts};

  return [ grep {defined && length} (
    $spec{cc},'-nologo','-c',
    @{$spec{includes}}      ,
    @{$spec{cflags}}        ,
    @{$spec{optimize}}      ,
    @{$spec{defines}}       ,
    @{$spec{perlinc}}       ,
    "-Fo$spec{output}"      ,
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

  print $SCRIPT join( "\n",
    map { ref $_ ? @{$_} : $_ }
    grep defined,
    delete(
      @spec{ qw(includes cflags optimize defines perlinc) } )
  );

  push @{$spec{includes}}, '@"' . $script . '"';

  return %spec;
}

sub format_linker_cmd {
  my ($self, %spec) = @_;
  my $cf = $self->{config};

  foreach my $path ( @{$spec{libpath}} ) {
    $path = "-libpath:$path";
  }

  my $output = $spec{output};
  my $manifest = $spec{manifest};

  $spec{def_file}  &&= '-def:'      . $spec{def_file};
  $spec{output}    &&= '-out:'      . $spec{output};
  $spec{manifest}  &&= '-manifest ' . $spec{manifest};
  $spec{implib}    &&= '-implib:'   . $spec{implib};
  $spec{map_file}  &&= '-map:'      . $spec{map_file};

  %spec = $self->write_linker_script(%spec)
    if $spec{use_scripts};

  my @cmds; # Stores the series of commands needed to build the module.

  push @cmds, [ grep {defined && length} (
    $spec{ld}               ,
    @{$spec{lddlflags}}     ,
    @{$spec{libpath}}       ,
    @{$spec{other_ldflags}} ,
    @{$spec{startup}}       ,
    @{$spec{objects}}       ,
    $spec{map_file}         ,
    $spec{libperl}          ,
    @{$spec{perllibs}}      ,
    $spec{def_file}         ,
    $spec{implib}           ,
    $spec{output}           ,
  ) ];

  # Embed the manifest file if it exists
  push @cmds, [
    'if', 'exist', $manifest, 'mt', '-nologo', $spec{manifest}, '-outputresource:' . "$output;2"
  ];

  return @cmds;
}

sub write_linker_script {
  my ($self, %spec) = @_;

  my $script = File::Spec->catfile( $spec{srcdir},
                                    $spec{basename} . '.lds' );

  $self->add_to_cleanup($script);

  print "Generating script '$script'\n" if !$self->{quiet};

  my $SCRIPT = IO::File->new( ">$script" )
    or die( "Could not create script '$script': $!" );

  print $SCRIPT join( "\n",
    map { ref $_ ? @{$_} : $_ }
    grep defined,
    delete(
      @spec{ qw(lddlflags libpath other_ldflags
                startup objects libperl perllibs
                def_file implib map_file)            } )
  );

  push @{$spec{lddlflags}}, '@"' . $script . '"';

  return %spec;
}

1;


