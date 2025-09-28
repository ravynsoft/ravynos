package ExtUtils::CBuilder::Platform::Windows;
use strict;
use warnings;

use File::Basename;
use File::Spec;

use ExtUtils::CBuilder::Base;
use IO::File;

our $VERSION = '0.280238'; # VERSION
our @ISA = qw(ExtUtils::CBuilder::Base);

=begin comment

The compiler-specific packages implement functions for generating properly
formatted commandlines for the compiler being used. Each package
defines two primary functions 'format_linker_cmd()' &
'format_compiler_cmd()' that accepts a list of named arguments (a
hash) and returns a list of formatted options suitable for invoking the
compiler. By default, if the compiler supports scripting of its
operation then a script file is built containing the options while
those options are removed from the commandline, and a reference to the
script is pushed onto the commandline in their place. Scripting the
compiler in this way helps to avoid the problems associated with long
commandlines under some shells.

=end comment

=cut

sub new {
  my $class = shift;
  my $self = $class->SUPER::new(@_);
  my $cf = $self->{config};

  # Inherit from an appropriate compiler driver class
  my $driver = "ExtUtils::CBuilder::Platform::Windows::" . $self->_compiler_type;
  eval "require $driver" or die "Could not load compiler driver: $@";
  unshift @ISA, $driver;

  return $self;
}

sub _compiler_type {
  my $self = shift;
  my $cc = $self->{config}{cc};

  return (  $cc =~ /cl(\.exe)?$/ ? 'MSVC'
	  : $cc =~ /bcc32(\.exe)?$/ ? 'BCC'
	  : 'GCC');
}

# native quoting, not shell quoting
sub quote_literal {
  my ($self, $string) = @_;

  # some of these characters don't need to be quoted for "native" quoting, but
  # quote them anyway so they are more likely to make it through cmd.exe
  if (length $string && $string !~ /[ \t\n\x0b"|<>%]/) {
    return $string;
  }

  $string =~ s{(\\*)(?="|\z)}{$1$1}g;
  $string =~ s{"}{\\"}g;

  return qq{"$string"};
}

sub split_like_shell {
  # Since Windows will pass the whole command string (not an argument
  # array) to the target program and make the program parse it itself,
  # we don't actually need to do any processing here.
  (my $self, local $_) = @_;

  return @$_ if defined() && UNIVERSAL::isa($_, 'ARRAY');
  return unless defined() && length();
  return ($_);
}

sub do_system {
  # See above
  my $self = shift;
  my $cmd = join ' ',
    grep length,
    map {$a=$_;$a=~s/\t/ /g;$a=~s/^\s+|\s+$//;$a}
    grep defined, @_;

  if (!$self->{quiet}) {
    print $cmd . "\n";
  }
  local $self->{quiet} = 1;
  return $self->SUPER::do_system($cmd);
}

sub arg_defines {
  my ($self, %args) = @_;
  s/"/\\"/g foreach values %args;
  return map qq{"-D$_=$args{$_}"}, sort keys %args;
}

sub compile {
  my ($self, %args) = @_;
  my $cf = $self->{config};

  die "Missing 'source' argument to compile()" unless defined $args{source};

  $args{include_dirs} = [ $args{include_dirs} ]
    if exists($args{include_dirs}) && ref($args{include_dirs}) ne "ARRAY";

  my ($basename, $srcdir) =
    ( File::Basename::fileparse($args{source}, '\.[^.]+$') )[0,1];

  $srcdir ||= File::Spec->curdir();

  my @defines = $self->arg_defines( %{ $args{defines} || {} } );

  my %spec = (
    srcdir      => $srcdir,
    builddir    => $srcdir,
    basename    => $basename,
    source      => $args{source},
    output      => $args{object_file} || File::Spec->catfile($srcdir, $basename) . $cf->{obj_ext},
    cc          => $cf->{cc},
    cflags      => [
                     $self->split_like_shell($cf->{ccflags}),
                     $self->split_like_shell($cf->{cccdlflags}),
                     $self->split_like_shell($args{extra_compiler_flags}),
                   ],
    optimize    => [ $self->split_like_shell($cf->{optimize})    ],
    defines     => \@defines,
    includes    => [ @{$args{include_dirs} || []} ],
    perlinc     => [
                     $self->perl_inc(),
                     $self->split_like_shell($cf->{incpath}),
                   ],
    use_scripts => 1, # XXX provide user option to change this???
  );

  $self->normalize_filespecs(
    \$spec{source},
    \$spec{output},
     $spec{includes},
     $spec{perlinc},
  );

  my @cmds = $self->format_compiler_cmd(%spec);
  while ( my $cmd = shift @cmds ) {
    $self->do_system( @$cmd )
      or die "error building $cf->{dlext} file from '$args{source}'";
  }

  (my $out = $spec{output}) =~ tr/'"//d;
  return $out;
}

sub need_prelink { 1 }

sub link {
  my ($self, %args) = @_;
  my $cf = $self->{config};

  my @objects = ( ref $args{objects} eq 'ARRAY' ? @{$args{objects}} : $args{objects} );
  my $to = join '', (File::Spec->splitpath($objects[0]))[0,1];
  $to ||= File::Spec->curdir();

  (my $file_base = $args{module_name}) =~ s/.*:://;
  my $output = $args{lib_file} ||
    File::Spec->catfile($to, "$file_base.$cf->{dlext}");

  # if running in perl source tree, look for libs there, not installed
  my $lddlflags = $cf->{lddlflags};
  my $perl_src = $self->perl_src();
  $lddlflags =~ s{\Q$cf->{archlibexp}\E[\\/]CORE}{$perl_src/lib/CORE} if $perl_src;

  my %spec = (
    srcdir        => $to,
    builddir      => $to,
    startup       => [ ],
    objects       => \@objects,
    libs          => [ ],
    output        => $output,
    ld            => $cf->{ld},
    libperl       => $cf->{libperl},
    perllibs      => [ $self->split_like_shell($cf->{perllibs})  ],
    libpath       => [ $self->split_like_shell($cf->{libpth})    ],
    lddlflags     => [ $self->split_like_shell($lddlflags) ],
    other_ldflags => [ $self->split_like_shell($args{extra_linker_flags} || '') ],
    use_scripts   => 1, # XXX provide user option to change this???
  );

  unless ( $spec{basename} ) {
    ($spec{basename} = $args{module_name}) =~ s/.*:://;
  }

  $spec{srcdir}   = File::Spec->canonpath( $spec{srcdir}   );
  $spec{builddir} = File::Spec->canonpath( $spec{builddir} );

  $spec{output}    ||= File::Spec->catfile( $spec{builddir},
                                            $spec{basename}  . '.'.$cf->{dlext}   );
  $spec{manifest}  ||= $spec{output} . '.manifest';
  $spec{implib}    ||= File::Spec->catfile( $spec{builddir},
                                            $spec{basename}  . $cf->{lib_ext} );
  $spec{explib}    ||= File::Spec->catfile( $spec{builddir},
                                            $spec{basename}  . '.exp'  );
  if ($cf->{cc} eq 'cl') {
    $spec{dbg_file}  ||= File::Spec->catfile( $spec{builddir},
                                            $spec{basename}  . '.pdb'  );
  }
  elsif ($cf->{cc} eq 'bcc32') {
    $spec{dbg_file}  ||= File::Spec->catfile( $spec{builddir},
                                            $spec{basename}  . '.tds'  );
  }
  $spec{def_file}  ||= File::Spec->catfile( $spec{srcdir}  ,
                                            $spec{basename}  . '.def'  );
  $spec{base_file} ||= File::Spec->catfile( $spec{srcdir}  ,
                                            $spec{basename}  . '.base' );

  $self->add_to_cleanup(
    grep defined,
    @{[ @spec{qw(manifest implib explib dbg_file def_file base_file map_file)} ]}
  );

  foreach my $opt ( qw(output manifest implib explib dbg_file def_file map_file base_file) ) {
    $self->normalize_filespecs( \$spec{$opt} );
  }

  foreach my $opt ( qw(libpath startup objects) ) {
    $self->normalize_filespecs( $spec{$opt} );
  }

  (my $def_base = $spec{def_file}) =~ tr/'"//d;
  $def_base =~ s/\.def$//;
  $self->prelink( %args,
                  dl_name => $args{module_name},
                  dl_file => $def_base,
                  dl_base => $spec{basename} );

  my @cmds = $self->format_linker_cmd(%spec);
  while ( my $cmd = shift @cmds ) {
    $self->do_system( @$cmd );
  }

  $spec{output} =~ tr/'"//d;
  return wantarray
    ? grep defined, @spec{qw[output manifest implib explib dbg_file def_file map_file base_file]}
    : $spec{output};
}

# canonize & quote paths
sub normalize_filespecs {
  my ($self, @specs) = @_;
  foreach my $spec ( grep defined, @specs ) {
    if ( ref $spec eq 'ARRAY') {
      $self->normalize_filespecs( map {\$_} grep defined, @$spec )
    } elsif ( ref $spec eq 'SCALAR' ) {
      $$spec =~ tr/"//d if $$spec;
      next unless $$spec;
      $$spec = '"' . File::Spec->canonpath($$spec) . '"';
    } elsif ( ref $spec eq '' ) {
      $spec = '"' . File::Spec->canonpath($spec) . '"';
    } else {
      die "Don't know how to normalize " . (ref $spec || $spec) . "\n";
    }
  }
}

# directory of perl's include files
sub perl_inc {
  my $self = shift;

  my $perl_src = $self->perl_src();

  if ($perl_src) {
    File::Spec->catdir($perl_src, "lib", "CORE");
  } else {
    File::Spec->catdir($self->{config}{archlibexp},"CORE");
  }
}

1;

__END__

=head1 NAME

ExtUtils::CBuilder::Platform::Windows - Builder class for Windows platforms

=head1 DESCRIPTION

This module implements the Windows-specific parts of ExtUtils::CBuilder.
Most of the Windows-specific stuff has to do with compiling and
linking C code.  Currently we support the 3 compilers perl itself
supports: MSVC, BCC, and GCC.

This module inherits from C<ExtUtils::CBuilder::Base>, so any functionality
not implemented here will be implemented there.  The interfaces are
defined by the L<ExtUtils::CBuilder> documentation.

=head1 AUTHOR

Ken Williams <ken@mathforum.org>

Most of the code here was written by Randy W. Sims <RandyS@ThePierianSpring.org>.

=head1 SEE ALSO

perl(1), ExtUtils::CBuilder(3), ExtUtils::MakeMaker(3)

=cut
