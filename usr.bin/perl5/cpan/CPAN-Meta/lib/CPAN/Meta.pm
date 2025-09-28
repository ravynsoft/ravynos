use 5.006;
use strict;
use warnings;
package CPAN::Meta;

our $VERSION = '2.150010';

#pod =head1 SYNOPSIS
#pod
#pod     use v5.10;
#pod     use strict;
#pod     use warnings;
#pod     use CPAN::Meta;
#pod     use Module::Load;
#pod
#pod     my $meta = CPAN::Meta->load_file('META.json');
#pod
#pod     printf "testing requirements for %s version %s\n",
#pod     $meta->name,
#pod     $meta->version;
#pod
#pod     my $prereqs = $meta->effective_prereqs;
#pod
#pod     for my $phase ( qw/configure runtime build test/ ) {
#pod         say "Requirements for $phase:";
#pod         my $reqs = $prereqs->requirements_for($phase, "requires");
#pod         for my $module ( sort $reqs->required_modules ) {
#pod             my $status;
#pod             if ( eval { load $module unless $module eq 'perl'; 1 } ) {
#pod                 my $version = $module eq 'perl' ? $] : $module->VERSION;
#pod                 $status = $reqs->accepts_module($module, $version)
#pod                         ? "$version ok" : "$version not ok";
#pod             } else {
#pod                 $status = "missing"
#pod             };
#pod             say "  $module ($status)";
#pod         }
#pod     }
#pod
#pod =head1 DESCRIPTION
#pod
#pod Software distributions released to the CPAN include a F<META.json> or, for
#pod older distributions, F<META.yml>, which describes the distribution, its
#pod contents, and the requirements for building and installing the distribution.
#pod The data structure stored in the F<META.json> file is described in
#pod L<CPAN::Meta::Spec>.
#pod
#pod CPAN::Meta provides a simple class to represent this distribution metadata (or
#pod I<distmeta>), along with some helpful methods for interrogating that data.
#pod
#pod The documentation below is only for the methods of the CPAN::Meta object.  For
#pod information on the meaning of individual fields, consult the spec.
#pod
#pod =cut

use Carp qw(carp croak);
use CPAN::Meta::Feature;
use CPAN::Meta::Prereqs;
use CPAN::Meta::Converter;
use CPAN::Meta::Validator;
use Parse::CPAN::Meta 1.4414 ();

BEGIN { *_dclone = \&CPAN::Meta::Converter::_dclone }

#pod =head1 STRING DATA
#pod
#pod The following methods return a single value, which is the value for the
#pod corresponding entry in the distmeta structure.  Values should be either undef
#pod or strings.
#pod
#pod =for :list
#pod * abstract
#pod * description
#pod * dynamic_config
#pod * generated_by
#pod * name
#pod * release_status
#pod * version
#pod
#pod =cut

BEGIN {
  my @STRING_READERS = qw(
    abstract
    description
    dynamic_config
    generated_by
    name
    release_status
    version
  );

  no strict 'refs';
  for my $attr (@STRING_READERS) {
    *$attr = sub { $_[0]{ $attr } };
  }
}

#pod =head1 LIST DATA
#pod
#pod These methods return lists of string values, which might be represented in the
#pod distmeta structure as arrayrefs or scalars:
#pod
#pod =for :list
#pod * authors
#pod * keywords
#pod * licenses
#pod
#pod The C<authors> and C<licenses> methods may also be called as C<author> and
#pod C<license>, respectively, to match the field name in the distmeta structure.
#pod
#pod =cut

BEGIN {
  my @LIST_READERS = qw(
    author
    keywords
    license
  );

  no strict 'refs';
  for my $attr (@LIST_READERS) {
    *$attr = sub {
      my $value = $_[0]{ $attr };
      croak "$attr must be called in list context"
        unless wantarray;
      return @{ _dclone($value) } if ref $value;
      return $value;
    };
  }
}

sub authors  { $_[0]->author }
sub licenses { $_[0]->license }

#pod =head1 MAP DATA
#pod
#pod These readers return hashrefs of arbitrary unblessed data structures, each
#pod described more fully in the specification:
#pod
#pod =for :list
#pod * meta_spec
#pod * resources
#pod * provides
#pod * no_index
#pod * prereqs
#pod * optional_features
#pod
#pod =cut

BEGIN {
  my @MAP_READERS = qw(
    meta-spec
    resources
    provides
    no_index

    prereqs
    optional_features
  );

  no strict 'refs';
  for my $attr (@MAP_READERS) {
    (my $subname = $attr) =~ s/-/_/;
    *$subname = sub {
      my $value = $_[0]{ $attr };
      return _dclone($value) if $value;
      return {};
    };
  }
}

#pod =head1 CUSTOM DATA
#pod
#pod A list of custom keys are available from the C<custom_keys> method and
#pod particular keys may be retrieved with the C<custom> method.
#pod
#pod   say $meta->custom($_) for $meta->custom_keys;
#pod
#pod If a custom key refers to a data structure, a deep clone is returned.
#pod
#pod =cut

sub custom_keys {
  return grep { /^x_/i } keys %{$_[0]};
}

sub custom {
  my ($self, $attr) = @_;
  my $value = $self->{$attr};
  return _dclone($value) if ref $value;
  return $value;
}

#pod =method new
#pod
#pod   my $meta = CPAN::Meta->new($distmeta_struct, \%options);
#pod
#pod Returns a valid CPAN::Meta object or dies if the supplied metadata hash
#pod reference fails to validate.  Older-format metadata will be up-converted to
#pod version 2 if they validate against the original stated specification.
#pod
#pod It takes an optional hashref of options. Valid options include:
#pod
#pod =over
#pod
#pod =item *
#pod
#pod lazy_validation -- if true, new will attempt to convert the given metadata
#pod to version 2 before attempting to validate it.  This means than any
#pod fixable errors will be handled by CPAN::Meta::Converter before validation.
#pod (Note that this might result in invalid optional data being silently
#pod dropped.)  The default is false.
#pod
#pod =back
#pod
#pod =cut

sub _new {
  my ($class, $struct, $options) = @_;
  my $self;

  if ( $options->{lazy_validation} ) {
    # try to convert to a valid structure; if succeeds, then return it
    my $cmc = CPAN::Meta::Converter->new( $struct );
    $self = $cmc->convert( version => 2 ); # valid or dies
    return bless $self, $class;
  }
  else {
    # validate original struct
    my $cmv = CPAN::Meta::Validator->new( $struct );
    unless ( $cmv->is_valid) {
      die "Invalid metadata structure. Errors: "
        . join(", ", $cmv->errors) . "\n";
    }
  }

  # up-convert older spec versions
  my $version = $struct->{'meta-spec'}{version} || '1.0';
  if ( $version == 2 ) {
    $self = $struct;
  }
  else {
    my $cmc = CPAN::Meta::Converter->new( $struct );
    $self = $cmc->convert( version => 2 );
  }

  return bless $self, $class;
}

sub new {
  my ($class, $struct, $options) = @_;
  my $self = eval { $class->_new($struct, $options) };
  croak($@) if $@;
  return $self;
}

#pod =method create
#pod
#pod   my $meta = CPAN::Meta->create($distmeta_struct, \%options);
#pod
#pod This is same as C<new()>, except that C<generated_by> and C<meta-spec> fields
#pod will be generated if not provided.  This means the metadata structure is
#pod assumed to otherwise follow the latest L<CPAN::Meta::Spec>.
#pod
#pod =cut

sub create {
  my ($class, $struct, $options) = @_;
  my $version = __PACKAGE__->VERSION || 2;
  $struct->{generated_by} ||= __PACKAGE__ . " version $version" ;
  $struct->{'meta-spec'}{version} ||= int($version);
  my $self = eval { $class->_new($struct, $options) };
  croak ($@) if $@;
  return $self;
}

#pod =method load_file
#pod
#pod   my $meta = CPAN::Meta->load_file($distmeta_file, \%options);
#pod
#pod Given a pathname to a file containing metadata, this deserializes the file
#pod according to its file suffix and constructs a new C<CPAN::Meta> object, just
#pod like C<new()>.  It will die if the deserialized version fails to validate
#pod against its stated specification version.
#pod
#pod It takes the same options as C<new()> but C<lazy_validation> defaults to
#pod true.
#pod
#pod =cut

sub load_file {
  my ($class, $file, $options) = @_;
  $options->{lazy_validation} = 1 unless exists $options->{lazy_validation};

  croak "load_file() requires a valid, readable filename"
    unless -r $file;

  my $self;
  eval {
    my $struct = Parse::CPAN::Meta->load_file( $file );
    $self = $class->_new($struct, $options);
  };
  croak($@) if $@;
  return $self;
}

#pod =method load_yaml_string
#pod
#pod   my $meta = CPAN::Meta->load_yaml_string($yaml, \%options);
#pod
#pod This method returns a new CPAN::Meta object using the first document in the
#pod given YAML string.  In other respects it is identical to C<load_file()>.
#pod
#pod =cut

sub load_yaml_string {
  my ($class, $yaml, $options) = @_;
  $options->{lazy_validation} = 1 unless exists $options->{lazy_validation};

  my $self;
  eval {
    my ($struct) = Parse::CPAN::Meta->load_yaml_string( $yaml );
    $self = $class->_new($struct, $options);
  };
  croak($@) if $@;
  return $self;
}

#pod =method load_json_string
#pod
#pod   my $meta = CPAN::Meta->load_json_string($json, \%options);
#pod
#pod This method returns a new CPAN::Meta object using the structure represented by
#pod the given JSON string.  In other respects it is identical to C<load_file()>.
#pod
#pod =cut

sub load_json_string {
  my ($class, $json, $options) = @_;
  $options->{lazy_validation} = 1 unless exists $options->{lazy_validation};

  my $self;
  eval {
    my $struct = Parse::CPAN::Meta->load_json_string( $json );
    $self = $class->_new($struct, $options);
  };
  croak($@) if $@;
  return $self;
}

#pod =method load_string
#pod
#pod   my $meta = CPAN::Meta->load_string($string, \%options);
#pod
#pod If you don't know if a string contains YAML or JSON, this method will use
#pod L<Parse::CPAN::Meta> to guess.  In other respects it is identical to
#pod C<load_file()>.
#pod
#pod =cut

sub load_string {
  my ($class, $string, $options) = @_;
  $options->{lazy_validation} = 1 unless exists $options->{lazy_validation};

  my $self;
  eval {
    my $struct = Parse::CPAN::Meta->load_string( $string );
    $self = $class->_new($struct, $options);
  };
  croak($@) if $@;
  return $self;
}

#pod =method save
#pod
#pod   $meta->save($distmeta_file, \%options);
#pod
#pod Serializes the object as JSON and writes it to the given file.  The only valid
#pod option is C<version>, which defaults to '2'. On Perl 5.8.1 or later, the file
#pod is saved with UTF-8 encoding.
#pod
#pod For C<version> 2 (or higher), the filename should end in '.json'.  L<JSON::PP>
#pod is the default JSON backend. Using another JSON backend requires L<JSON> 2.5 or
#pod later and you must set the C<$ENV{PERL_JSON_BACKEND}> to a supported alternate
#pod backend like L<JSON::XS>.
#pod
#pod For C<version> less than 2, the filename should end in '.yml'.
#pod L<CPAN::Meta::Converter> is used to generate an older metadata structure, which
#pod is serialized to YAML.  CPAN::Meta::YAML is the default YAML backend.  You may
#pod set the C<$ENV{PERL_YAML_BACKEND}> to a supported alternative backend, though
#pod this is not recommended due to subtle incompatibilities between YAML parsers on
#pod CPAN.
#pod
#pod =cut

sub save {
  my ($self, $file, $options) = @_;

  my $version = $options->{version} || '2';
  my $layer = $] ge '5.008001' ? ':utf8' : '';

  if ( $version ge '2' ) {
    carp "'$file' should end in '.json'"
      unless $file =~ m{\.json$};
  }
  else {
    carp "'$file' should end in '.yml'"
      unless $file =~ m{\.yml$};
  }

  my $data = $self->as_string( $options );
  open my $fh, ">$layer", $file
    or die "Error opening '$file' for writing: $!\n";

  print {$fh} $data;
  close $fh
    or die "Error closing '$file': $!\n";

  return 1;
}

#pod =method meta_spec_version
#pod
#pod This method returns the version part of the C<meta_spec> entry in the distmeta
#pod structure.  It is equivalent to:
#pod
#pod   $meta->meta_spec->{version};
#pod
#pod =cut

sub meta_spec_version {
  my ($self) = @_;
  return $self->meta_spec->{version};
}

#pod =method effective_prereqs
#pod
#pod   my $prereqs = $meta->effective_prereqs;
#pod
#pod   my $prereqs = $meta->effective_prereqs( \@feature_identifiers );
#pod
#pod This method returns a L<CPAN::Meta::Prereqs> object describing all the
#pod prereqs for the distribution.  If an arrayref of feature identifiers is given,
#pod the prereqs for the identified features are merged together with the
#pod distribution's core prereqs before the CPAN::Meta::Prereqs object is returned.
#pod
#pod =cut

sub effective_prereqs {
  my ($self, $features) = @_;
  $features ||= [];

  my $prereq = CPAN::Meta::Prereqs->new($self->prereqs);

  return $prereq unless @$features;

  my @other = map {; $self->feature($_)->prereqs } @$features;

  return $prereq->with_merged_prereqs(\@other);
}

#pod =method should_index_file
#pod
#pod   ... if $meta->should_index_file( $filename );
#pod
#pod This method returns true if the given file should be indexed.  It decides this
#pod by checking the C<file> and C<directory> keys in the C<no_index> property of
#pod the distmeta structure. Note that neither the version format nor
#pod C<release_status> are considered.
#pod
#pod C<$filename> should be given in unix format.
#pod
#pod =cut

sub should_index_file {
  my ($self, $filename) = @_;

  for my $no_index_file (@{ $self->no_index->{file} || [] }) {
    return if $filename eq $no_index_file;
  }

  for my $no_index_dir (@{ $self->no_index->{directory} }) {
    $no_index_dir =~ s{$}{/} unless $no_index_dir =~ m{/\z};
    return if index($filename, $no_index_dir) == 0;
  }

  return 1;
}

#pod =method should_index_package
#pod
#pod   ... if $meta->should_index_package( $package );
#pod
#pod This method returns true if the given package should be indexed.  It decides
#pod this by checking the C<package> and C<namespace> keys in the C<no_index>
#pod property of the distmeta structure. Note that neither the version format nor
#pod C<release_status> are considered.
#pod
#pod =cut

sub should_index_package {
  my ($self, $package) = @_;

  for my $no_index_pkg (@{ $self->no_index->{package} || [] }) {
    return if $package eq $no_index_pkg;
  }

  for my $no_index_ns (@{ $self->no_index->{namespace} }) {
    return if index($package, "${no_index_ns}::") == 0;
  }

  return 1;
}

#pod =method features
#pod
#pod   my @feature_objects = $meta->features;
#pod
#pod This method returns a list of L<CPAN::Meta::Feature> objects, one for each
#pod optional feature described by the distribution's metadata.
#pod
#pod =cut

sub features {
  my ($self) = @_;

  my $opt_f = $self->optional_features;
  my @features = map {; CPAN::Meta::Feature->new($_ => $opt_f->{ $_ }) }
                 keys %$opt_f;

  return @features;
}

#pod =method feature
#pod
#pod   my $feature_object = $meta->feature( $identifier );
#pod
#pod This method returns a L<CPAN::Meta::Feature> object for the optional feature
#pod with the given identifier.  If no feature with that identifier exists, an
#pod exception will be raised.
#pod
#pod =cut

sub feature {
  my ($self, $ident) = @_;

  croak "no feature named $ident"
    unless my $f = $self->optional_features->{ $ident };

  return CPAN::Meta::Feature->new($ident, $f);
}

#pod =method as_struct
#pod
#pod   my $copy = $meta->as_struct( \%options );
#pod
#pod This method returns a deep copy of the object's metadata as an unblessed hash
#pod reference.  It takes an optional hashref of options.  If the hashref contains
#pod a C<version> argument, the copied metadata will be converted to the version
#pod of the specification and returned.  For example:
#pod
#pod   my $old_spec = $meta->as_struct( {version => "1.4"} );
#pod
#pod =cut

sub as_struct {
  my ($self, $options) = @_;
  my $struct = _dclone($self);
  if ( $options->{version} ) {
    my $cmc = CPAN::Meta::Converter->new( $struct );
    $struct = $cmc->convert( version => $options->{version} );
  }
  return $struct;
}

#pod =method as_string
#pod
#pod   my $string = $meta->as_string( \%options );
#pod
#pod This method returns a serialized copy of the object's metadata as a character
#pod string.  (The strings are B<not> UTF-8 encoded.)  It takes an optional hashref
#pod of options.  If the hashref contains a C<version> argument, the copied metadata
#pod will be converted to the version of the specification and returned.  For
#pod example:
#pod
#pod   my $string = $meta->as_string( {version => "1.4"} );
#pod
#pod For C<version> greater than or equal to 2, the string will be serialized as
#pod JSON.  For C<version> less than 2, the string will be serialized as YAML.  In
#pod both cases, the same rules are followed as in the C<save()> method for choosing
#pod a serialization backend.
#pod
#pod The serialized structure will include a C<x_serialization_backend> entry giving
#pod the package and version used to serialize.  Any existing key in the given
#pod C<$meta> object will be clobbered.
#pod
#pod =cut

sub as_string {
  my ($self, $options) = @_;

  my $version = $options->{version} || '2';

  my $struct;
  if ( $self->meta_spec_version ne $version ) {
    my $cmc = CPAN::Meta::Converter->new( $self->as_struct );
    $struct = $cmc->convert( version => $version );
  }
  else {
    $struct = $self->as_struct;
  }

  my ($data, $backend);
  if ( $version ge '2' ) {
    $backend = Parse::CPAN::Meta->json_backend();
    local $struct->{x_serialization_backend} = sprintf '%s version %s',
      $backend, $backend->VERSION;
    $data = $backend->new->pretty->canonical->encode($struct);
  }
  else {
    $backend = Parse::CPAN::Meta->yaml_backend();
    local $struct->{x_serialization_backend} = sprintf '%s version %s',
      $backend, $backend->VERSION;
    $data = eval { no strict 'refs'; &{"$backend\::Dump"}($struct) };
    if ( $@ ) {
      croak $backend->can('errstr') ? $backend->errstr : $@
    }
  }

  return $data;
}

# Used by JSON::PP, etc. for "convert_blessed"
sub TO_JSON {
  return { %{ $_[0] } };
}

1;

# ABSTRACT: the distribution metadata for a CPAN dist

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta - the distribution metadata for a CPAN dist

=head1 VERSION

version 2.150010

=head1 SYNOPSIS

    use v5.10;
    use strict;
    use warnings;
    use CPAN::Meta;
    use Module::Load;

    my $meta = CPAN::Meta->load_file('META.json');

    printf "testing requirements for %s version %s\n",
    $meta->name,
    $meta->version;

    my $prereqs = $meta->effective_prereqs;

    for my $phase ( qw/configure runtime build test/ ) {
        say "Requirements for $phase:";
        my $reqs = $prereqs->requirements_for($phase, "requires");
        for my $module ( sort $reqs->required_modules ) {
            my $status;
            if ( eval { load $module unless $module eq 'perl'; 1 } ) {
                my $version = $module eq 'perl' ? $] : $module->VERSION;
                $status = $reqs->accepts_module($module, $version)
                        ? "$version ok" : "$version not ok";
            } else {
                $status = "missing"
            };
            say "  $module ($status)";
        }
    }

=head1 DESCRIPTION

Software distributions released to the CPAN include a F<META.json> or, for
older distributions, F<META.yml>, which describes the distribution, its
contents, and the requirements for building and installing the distribution.
The data structure stored in the F<META.json> file is described in
L<CPAN::Meta::Spec>.

CPAN::Meta provides a simple class to represent this distribution metadata (or
I<distmeta>), along with some helpful methods for interrogating that data.

The documentation below is only for the methods of the CPAN::Meta object.  For
information on the meaning of individual fields, consult the spec.

=head1 METHODS

=head2 new

  my $meta = CPAN::Meta->new($distmeta_struct, \%options);

Returns a valid CPAN::Meta object or dies if the supplied metadata hash
reference fails to validate.  Older-format metadata will be up-converted to
version 2 if they validate against the original stated specification.

It takes an optional hashref of options. Valid options include:

=over

=item *

lazy_validation -- if true, new will attempt to convert the given metadata
to version 2 before attempting to validate it.  This means than any
fixable errors will be handled by CPAN::Meta::Converter before validation.
(Note that this might result in invalid optional data being silently
dropped.)  The default is false.

=back

=head2 create

  my $meta = CPAN::Meta->create($distmeta_struct, \%options);

This is same as C<new()>, except that C<generated_by> and C<meta-spec> fields
will be generated if not provided.  This means the metadata structure is
assumed to otherwise follow the latest L<CPAN::Meta::Spec>.

=head2 load_file

  my $meta = CPAN::Meta->load_file($distmeta_file, \%options);

Given a pathname to a file containing metadata, this deserializes the file
according to its file suffix and constructs a new C<CPAN::Meta> object, just
like C<new()>.  It will die if the deserialized version fails to validate
against its stated specification version.

It takes the same options as C<new()> but C<lazy_validation> defaults to
true.

=head2 load_yaml_string

  my $meta = CPAN::Meta->load_yaml_string($yaml, \%options);

This method returns a new CPAN::Meta object using the first document in the
given YAML string.  In other respects it is identical to C<load_file()>.

=head2 load_json_string

  my $meta = CPAN::Meta->load_json_string($json, \%options);

This method returns a new CPAN::Meta object using the structure represented by
the given JSON string.  In other respects it is identical to C<load_file()>.

=head2 load_string

  my $meta = CPAN::Meta->load_string($string, \%options);

If you don't know if a string contains YAML or JSON, this method will use
L<Parse::CPAN::Meta> to guess.  In other respects it is identical to
C<load_file()>.

=head2 save

  $meta->save($distmeta_file, \%options);

Serializes the object as JSON and writes it to the given file.  The only valid
option is C<version>, which defaults to '2'. On Perl 5.8.1 or later, the file
is saved with UTF-8 encoding.

For C<version> 2 (or higher), the filename should end in '.json'.  L<JSON::PP>
is the default JSON backend. Using another JSON backend requires L<JSON> 2.5 or
later and you must set the C<$ENV{PERL_JSON_BACKEND}> to a supported alternate
backend like L<JSON::XS>.

For C<version> less than 2, the filename should end in '.yml'.
L<CPAN::Meta::Converter> is used to generate an older metadata structure, which
is serialized to YAML.  CPAN::Meta::YAML is the default YAML backend.  You may
set the C<$ENV{PERL_YAML_BACKEND}> to a supported alternative backend, though
this is not recommended due to subtle incompatibilities between YAML parsers on
CPAN.

=head2 meta_spec_version

This method returns the version part of the C<meta_spec> entry in the distmeta
structure.  It is equivalent to:

  $meta->meta_spec->{version};

=head2 effective_prereqs

  my $prereqs = $meta->effective_prereqs;

  my $prereqs = $meta->effective_prereqs( \@feature_identifiers );

This method returns a L<CPAN::Meta::Prereqs> object describing all the
prereqs for the distribution.  If an arrayref of feature identifiers is given,
the prereqs for the identified features are merged together with the
distribution's core prereqs before the CPAN::Meta::Prereqs object is returned.

=head2 should_index_file

  ... if $meta->should_index_file( $filename );

This method returns true if the given file should be indexed.  It decides this
by checking the C<file> and C<directory> keys in the C<no_index> property of
the distmeta structure. Note that neither the version format nor
C<release_status> are considered.

C<$filename> should be given in unix format.

=head2 should_index_package

  ... if $meta->should_index_package( $package );

This method returns true if the given package should be indexed.  It decides
this by checking the C<package> and C<namespace> keys in the C<no_index>
property of the distmeta structure. Note that neither the version format nor
C<release_status> are considered.

=head2 features

  my @feature_objects = $meta->features;

This method returns a list of L<CPAN::Meta::Feature> objects, one for each
optional feature described by the distribution's metadata.

=head2 feature

  my $feature_object = $meta->feature( $identifier );

This method returns a L<CPAN::Meta::Feature> object for the optional feature
with the given identifier.  If no feature with that identifier exists, an
exception will be raised.

=head2 as_struct

  my $copy = $meta->as_struct( \%options );

This method returns a deep copy of the object's metadata as an unblessed hash
reference.  It takes an optional hashref of options.  If the hashref contains
a C<version> argument, the copied metadata will be converted to the version
of the specification and returned.  For example:

  my $old_spec = $meta->as_struct( {version => "1.4"} );

=head2 as_string

  my $string = $meta->as_string( \%options );

This method returns a serialized copy of the object's metadata as a character
string.  (The strings are B<not> UTF-8 encoded.)  It takes an optional hashref
of options.  If the hashref contains a C<version> argument, the copied metadata
will be converted to the version of the specification and returned.  For
example:

  my $string = $meta->as_string( {version => "1.4"} );

For C<version> greater than or equal to 2, the string will be serialized as
JSON.  For C<version> less than 2, the string will be serialized as YAML.  In
both cases, the same rules are followed as in the C<save()> method for choosing
a serialization backend.

The serialized structure will include a C<x_serialization_backend> entry giving
the package and version used to serialize.  Any existing key in the given
C<$meta> object will be clobbered.

=head1 STRING DATA

The following methods return a single value, which is the value for the
corresponding entry in the distmeta structure.  Values should be either undef
or strings.

=over 4

=item *

abstract

=item *

description

=item *

dynamic_config

=item *

generated_by

=item *

name

=item *

release_status

=item *

version

=back

=head1 LIST DATA

These methods return lists of string values, which might be represented in the
distmeta structure as arrayrefs or scalars:

=over 4

=item *

authors

=item *

keywords

=item *

licenses

=back

The C<authors> and C<licenses> methods may also be called as C<author> and
C<license>, respectively, to match the field name in the distmeta structure.

=head1 MAP DATA

These readers return hashrefs of arbitrary unblessed data structures, each
described more fully in the specification:

=over 4

=item *

meta_spec

=item *

resources

=item *

provides

=item *

no_index

=item *

prereqs

=item *

optional_features

=back

=head1 CUSTOM DATA

A list of custom keys are available from the C<custom_keys> method and
particular keys may be retrieved with the C<custom> method.

  say $meta->custom($_) for $meta->custom_keys;

If a custom key refers to a data structure, a deep clone is returned.

=for Pod::Coverage TO_JSON abstract author authors custom custom_keys description dynamic_config
generated_by keywords license licenses meta_spec name no_index
optional_features prereqs provides release_status resources version

=head1 BUGS

Please report any bugs or feature using the CPAN Request Tracker.
Bugs can be submitted through the web interface at
L<http://rt.cpan.org/Dist/Display.html?Queue=CPAN-Meta>

When submitting a bug or request, please include a test-file or a patch to an
existing test-file that illustrates the bug or desired feature.

=head1 SEE ALSO

=over 4

=item *

L<CPAN::Meta::Converter>

=item *

L<CPAN::Meta::Validator>

=back

=for :stopwords cpan testmatrix url annocpan anno bugtracker rt cpants kwalitee diff irc mailto metadata placeholders metacpan

=head1 SUPPORT

=head2 Bugs / Feature Requests

Please report any bugs or feature requests through the issue tracker
at L<https://github.com/Perl-Toolchain-Gang/CPAN-Meta/issues>.
You will be notified automatically of any progress on your issue.

=head2 Source Code

This is open source software.  The code repository is available for
public review and contribution under the terms of the license.

L<https://github.com/Perl-Toolchain-Gang/CPAN-Meta>

  git clone https://github.com/Perl-Toolchain-Gang/CPAN-Meta.git

=head1 AUTHORS

=over 4

=item *

David Golden <dagolden@cpan.org>

=item *

Ricardo Signes <rjbs@cpan.org>

=item *

Adam Kennedy <adamk@cpan.org>

=back

=head1 CONTRIBUTORS

=for stopwords Ansgar Burchardt Avar Arnfjord Bjarmason Benjamin Noggle Christopher J. Madsen Chuck Adams Cory G Watson Damyan Ivanov David Golden Eric Wilhelm Graham Knop Gregor Hermann Karen Etheridge Kenichi Ishigaki Kent Fredric Ken Williams Lars Dieckow Leon Timmermans majensen Mark Fowler Matt S Trout Michael G. Schwern Mohammad Anwar mohawk2 moznion Niko Tyni Olaf Alders Olivier Mengué Randy Sims Tomohiro Hosaka

=over 4

=item *

Ansgar Burchardt <ansgar@cpan.org>

=item *

Avar Arnfjord Bjarmason <avar@cpan.org>

=item *

Benjamin Noggle <agwind@users.noreply.github.com>

=item *

Christopher J. Madsen <cjm@cpan.org>

=item *

Chuck Adams <cja987@gmail.com>

=item *

Cory G Watson <gphat@cpan.org>

=item *

Damyan Ivanov <dam@cpan.org>

=item *

David Golden <xdg@xdg.me>

=item *

Eric Wilhelm <ewilhelm@cpan.org>

=item *

Graham Knop <haarg@haarg.org>

=item *

Gregor Hermann <gregoa@debian.org>

=item *

Karen Etheridge <ether@cpan.org>

=item *

Kenichi Ishigaki <ishigaki@cpan.org>

=item *

Kent Fredric <kentfredric@gmail.com>

=item *

Ken Williams <kwilliams@cpan.org>

=item *

Lars Dieckow <daxim@cpan.org>

=item *

Leon Timmermans <leont@cpan.org>

=item *

majensen <maj@fortinbras.us>

=item *

Mark Fowler <markf@cpan.org>

=item *

Matt S Trout <mst@shadowcat.co.uk>

=item *

Michael G. Schwern <mschwern@cpan.org>

=item *

Mohammad S Anwar <mohammad.anwar@yahoo.com>

=item *

mohawk2 <mohawk2@users.noreply.github.com>

=item *

moznion <moznion@gmail.com>

=item *

Niko Tyni <ntyni@debian.org>

=item *

Olaf Alders <olaf@wundersolutions.com>

=item *

Olivier Mengué <dolmen@cpan.org>

=item *

Randy Sims <randys@thepierianspring.org>

=item *

Tomohiro Hosaka <bokutin@bokut.in>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by David Golden, Ricardo Signes, Adam Kennedy and Contributors.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut

__END__


# vim: ts=2 sts=2 sw=2 et :
