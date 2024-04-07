package ExtUtils::CBuilder::Base;
use strict;
use warnings;
use File::Spec;
use File::Basename;
use Cwd ();
use Config;
use Text::ParseWords;
use IPC::Cmd qw(can_run);
use File::Temp qw(tempfile);

our $VERSION = '0.280238'; # VERSION

# More details about C/C++ compilers:
# http://developers.sun.com/sunstudio/documentation/product/compiler.jsp
# http://gcc.gnu.org/
# http://publib.boulder.ibm.com/infocenter/comphelp/v101v121/index.jsp
# http://msdn.microsoft.com/en-us/vstudio/default.aspx

my %cc2cxx = (
    # first line order is important to support wrappers like in pkgsrc
    cc => [ 'c++', 'CC', 'aCC', 'cxx', ], # Sun Studio, HP ANSI C/C++ Compilers
    gcc => [ 'g++' ], # GNU Compiler Collection
    xlc => [ 'xlC' ], # IBM C/C++ Set, xlc without thread-safety
    xlc_r => [ 'xlC_r' ], # IBM C/C++ Set, xlc with thread-safety
    cl    => [ 'cl' ], # Microsoft Visual Studio
);

sub new {
  my $class = shift;
  my $self = bless {@_}, $class;

  $self->{properties}{perl} = $class->find_perl_interpreter
    or warn "Warning: Can't locate your perl binary";

  while (my ($k,$v) = each %Config) {
    $self->{config}{$k} = $v unless exists $self->{config}{$k};
  }
  $self->{config}{cc} = $ENV{CC} if defined $ENV{CC};
  $self->{config}{ccflags} = join(" ", $self->{config}{ccflags}, $ENV{CFLAGS})
     if defined $ENV{CFLAGS};
  $self->{config}{cxx} = $ENV{CXX} if defined $ENV{CXX};
  $self->{config}{cxxflags} = $ENV{CXXFLAGS} if defined $ENV{CXXFLAGS};
  $self->{config}{ld} = $ENV{LD} if defined $ENV{LD};
  $self->{config}{ldflags} = join(" ", $self->{config}{ldflags}, $ENV{LDFLAGS})
     if defined $ENV{LDFLAGS};

  unless ( exists $self->{config}{cxx} ) {

    my ($ccbase, $ccpath, $ccsfx ) = fileparse($self->{config}{cc}, qr/\.[^.]*/);

    ## If the path is just "cc", fileparse returns $ccpath as "./"
    $ccpath = "" if $self->{config}{cc} =~ /^\Q$ccbase$ccsfx\E$/;
      
    foreach my $cxx (@{$cc2cxx{$ccbase}}) {
      my $cxx1 = File::Spec->catfile( $ccpath, $cxx . $ccsfx);

      if( can_run( $cxx1 ) ) {
        $self->{config}{cxx} = $cxx1;
	last;
      }
      my $cxx2 = $cxx . $ccsfx;

      if( can_run( $cxx2 ) ) {
        $self->{config}{cxx} = $cxx2;
	last;
      }

      if( can_run( $cxx ) ) {
        $self->{config}{cxx} = $cxx;
	last;
      }
    }
    unless ( exists $self->{config}{cxx} ) {
      $self->{config}{cxx} = $self->{config}{cc};
      my $cflags = $self->{config}{ccflags};
      $self->{config}{cxxflags} = '-x c++';
      $self->{config}{cxxflags} .= " $cflags" if defined $cflags;
    }
  }

  return $self;
}

sub find_perl_interpreter {
  my $perl;
  File::Spec->file_name_is_absolute($perl = $^X)
    or -f ($perl = $Config::Config{perlpath})
    or ($perl = $^X); # XXX how about using IPC::Cmd::can_run here?
  return $perl;
}

sub add_to_cleanup {
  my $self = shift;
  foreach (@_) {
    $self->{files_to_clean}{$_} = 1;
  }
}

sub cleanup {
  my $self = shift;
  foreach my $file (keys %{$self->{files_to_clean}}) {
    unlink $file;
  }
}

sub get_config {
    return %{ $_[0]->{config} };
}

sub object_file {
  my ($self, $filename) = @_;

  # File name, minus the suffix
  (my $file_base = $filename) =~ s/\.[^.]+$//;
  return "$file_base$self->{config}{obj_ext}";
}

sub arg_include_dirs {
  my $self = shift;
  return map {"-I$_"} @_;
}

sub arg_nolink { '-c' }

sub arg_object_file {
  my ($self, $file) = @_;
  return ('-o', $file);
}

sub arg_share_object_file {
  my ($self, $file) = @_;
  return ($self->split_like_shell($self->{config}{lddlflags}), '-o', $file);
}

sub arg_exec_file {
  my ($self, $file) = @_;
  return ('-o', $file);
}

sub arg_defines {
  my ($self, %args) = @_;
  return map "-D$_=$args{$_}", sort keys %args;
}

sub compile {
  my ($self, %args) = @_;
  die "Missing 'source' argument to compile()" unless defined $args{source};

  my $cf = $self->{config}; # For convenience

  my $object_file = $args{object_file}
    ? $args{object_file}
    : $self->object_file($args{source});

  my $include_dirs_ref =
    (exists($args{include_dirs}) && ref($args{include_dirs}) ne "ARRAY")
      ? [ $args{include_dirs} ]
      : $args{include_dirs};
  my @include_dirs = $self->arg_include_dirs(
    @{ $include_dirs_ref || [] },
    $self->perl_inc(),
  );

  my @defines = $self->arg_defines( %{$args{defines} || {}} );

  my @extra_compiler_flags =
    $self->split_like_shell($args{extra_compiler_flags});
  my @cccdlflags = $self->split_like_shell($cf->{cccdlflags});
  my @ccflags = $self->split_like_shell($args{'C++'} ? $cf->{cxxflags} : $cf->{ccflags});
  my @optimize = $self->split_like_shell($cf->{optimize});
  my @flags = (
    @include_dirs,
    @defines,
    @cccdlflags,
    @extra_compiler_flags,
    $self->arg_nolink,
    @ccflags,
    @optimize,
    $self->arg_object_file($object_file),
  );
  my @cc = $self->split_like_shell($args{'C++'} ? $cf->{cxx} : $cf->{cc});

  $self->do_system(@cc, @flags, $args{source})
    or die "error building $object_file from '$args{source}'";

  return $object_file;
}

sub have_compiler {
  my ($self, $is_cplusplus) = @_;
  my $have_compiler_flag = $is_cplusplus ? "have_cxx" : "have_cc";
  my $suffix = $is_cplusplus ? ".cc" : ".c";
  return $self->{$have_compiler_flag} if defined $self->{$have_compiler_flag};

  my $result;
  my $attempts = 3;
  # tmpdir has issues for some people so fall back to current dir

  # don't clobber existing files (rare, but possible)
  my ( $FH, $tmpfile ) = tempfile( "compilet-XXXXX", SUFFIX => $suffix );
  binmode $FH;

  if ( $is_cplusplus ) {
    print $FH "class Bogus { public: int boot_compilet() { return 1; } };\n";
  }
  else {
    print $FH "int boot_compilet() { return 1; }\n";
  }
  close $FH;

  my ($obj_file, @lib_files);
  eval {
    local $^W = 0;
    local $self->{quiet} = 1;
    $obj_file = $self->compile('C++' => $is_cplusplus, source => $tmpfile);
    @lib_files = $self->link(objects => $obj_file, module_name => 'compilet');
  };
  $result = $@ ? 0 : 1;

  foreach (grep defined, $tmpfile, $obj_file, @lib_files) {
    1 while unlink;
  }

  return $self->{$have_compiler_flag} = $result;
}

sub have_cplusplus {
  push @_, 1;
  goto &have_compiler;
}

sub lib_file {
  my ($self, $dl_file, %args) = @_;
  $dl_file =~ s/\.[^.]+$//;
  $dl_file =~ tr/"//d;

  if (defined $args{module_name} and length $args{module_name}) {
    # Need to create with the same name as DynaLoader will load with.
    require DynaLoader;
    if (defined &DynaLoader::mod2fname) {
      my $lib = DynaLoader::mod2fname([split /::/, $args{module_name}]);
      my ($dev, $lib_dir, undef) = File::Spec->splitpath($dl_file);
      $dl_file = File::Spec->catpath($dev, $lib_dir, $lib);
    }
  }

  $dl_file .= ".$self->{config}{dlext}";

  return $dl_file;
}


sub exe_file {
  my ($self, $dl_file) = @_;
  $dl_file =~ s/\.[^.]+$//;
  $dl_file =~ tr/"//d;
  return "$dl_file$self->{config}{_exe}";
}

sub need_prelink { 0 }

sub extra_link_args_after_prelink { return }

sub prelink {
  my ($self, %args) = @_;

  my ($dl_file_out, $mksymlists_args) = _prepare_mksymlists_args(\%args);

  require ExtUtils::Mksymlists;
  # dl. abbrev for dynamic library
  ExtUtils::Mksymlists::Mksymlists( %{ $mksymlists_args } );

  # Mksymlists will create one of these files
  return grep -e, map "$dl_file_out.$_", qw(ext def opt);
}

sub _prepare_mksymlists_args {
  my $args = shift;
  ($args->{dl_file} = $args->{dl_name}) =~ s/.*::// unless $args->{dl_file};

  my %mksymlists_args = (
    DL_VARS  => $args->{dl_vars}      || [],
    DL_FUNCS => $args->{dl_funcs}     || {},
    FUNCLIST => $args->{dl_func_list} || [],
    IMPORTS  => $args->{dl_imports}   || {},
    NAME     => $args->{dl_name},    # Name of the Perl module
    DLBASE   => $args->{dl_base},    # Basename of DLL file
    FILE     => $args->{dl_file},    # Dir + Basename of symlist file
    VERSION  => (defined $args->{dl_version} ? $args->{dl_version} : '0.0'),
  );
  return ($args->{dl_file}, \%mksymlists_args);
}

sub link {
  my ($self, %args) = @_;
  return $self->_do_link('lib_file', lddl => 1, %args);
}

sub link_executable {
  my ($self, %args) = @_;
  return $self->_do_link('exe_file', lddl => 0, %args);
}

sub _do_link {
  my ($self, $type, %args) = @_;

  my $cf = $self->{config}; # For convenience

  my $objects = delete $args{objects};
  $objects = [$objects] unless ref $objects;
  my $out = $args{$type} || $self->$type($objects->[0], %args);

  my @temp_files;
  @temp_files =
    $self->prelink(%args, dl_name => $args{module_name})
      if $args{lddl} && $self->need_prelink;

  my @linker_flags = (
    $self->split_like_shell($args{extra_linker_flags}),
    $self->extra_link_args_after_prelink(
       %args, dl_name => $args{module_name}, prelink_res => \@temp_files
    )
  );

  my @output = $args{lddl}
    ? $self->arg_share_object_file($out)
    : $self->arg_exec_file($out);
  my @shrp = $self->split_like_shell($cf->{shrpenv});
  my @ld = $self->split_like_shell($cf->{ld});

  $self->do_system(@shrp, @ld, @output, @$objects, @linker_flags)
    or die "error building $out from @$objects";

  return wantarray ? ($out, @temp_files) : $out;
}

sub quote_literal {
  my ($self, $string) = @_;

  if (length $string && $string !~ /[^a-zA-Z0-9,._+@%\/-]/) {
    return $string;
  }

  $string =~ s{'}{'\\''}g;

  return "'$string'";
}

sub do_system {
  my ($self, @cmd) = @_;
  if (!$self->{quiet}) {
    my $full = join ' ', map $self->quote_literal($_), @cmd;
    print $full . "\n";
  }
  return !system(@cmd);
}

sub split_like_shell {
  my ($self, $string) = @_;

  return () unless defined($string);
  return @$string if UNIVERSAL::isa($string, 'ARRAY');
  $string =~ s/^\s+|\s+$//g;
  return () unless length($string);

  # Text::ParseWords replaces all 'escaped' characters with themselves, which completely
  # breaks paths under windows. As such, we forcibly replace backwards slashes with forward
  # slashes on windows.
  $string =~ s@\\@/@g if $^O eq 'MSWin32';

  return Text::ParseWords::shellwords($string);
}

# if building perl, perl's main source directory
sub perl_src {
  # N.B. makemaker actually searches regardless of PERL_CORE, but
  # only squawks at not finding it if PERL_CORE is set

  return unless $ENV{PERL_CORE};

  my $Updir = File::Spec->updir;
  my $dir   = File::Spec->curdir;

  # Try up to 5 levels upwards
  for (0..10) {
    if (
      -f File::Spec->catfile($dir,"config_h.SH")
      &&
      -f File::Spec->catfile($dir,"perl.h")
      &&
      -f File::Spec->catfile($dir,"lib","Exporter.pm")
    ) {
      return Cwd::realpath( $dir );
    }

    $dir = File::Spec->catdir($dir, $Updir);
  }

  warn "PERL_CORE is set but I can't find your perl source!\n";
  return ''; # return empty string if $ENV{PERL_CORE} but can't find dir ???
}

# directory of perl's include files
sub perl_inc {
  my $self = shift;

  $self->perl_src() || File::Spec->catdir($self->{config}{archlibexp},"CORE");
}

sub DESTROY {
  my $self = shift;
  local($., $@, $!, $^E, $?);
  $self->cleanup();
}

1;

# vim: ts=2 sw=2 et:
