use 5.008001;
use strict;
use warnings;
package Parse::CPAN::Meta;
# ABSTRACT: Parse META.yml and META.json CPAN metadata files

our $VERSION = '2.150010';

use Exporter;
use Carp 'croak';

our @ISA = qw/Exporter/;
our @EXPORT_OK = qw/Load LoadFile/;

sub load_file {
  my ($class, $filename) = @_;

  my $meta = _slurp($filename);

  if ($filename =~ /\.ya?ml$/) {
    return $class->load_yaml_string($meta);
  }
  elsif ($filename =~ /\.json$/) {
    return $class->load_json_string($meta);
  }
  else {
    $class->load_string($meta); # try to detect yaml/json
  }
}

sub load_string {
  my ($class, $string) = @_;
  if ( $string =~ /^---/ ) { # looks like YAML
    return $class->load_yaml_string($string);
  }
  elsif ( $string =~ /^\s*\{/ ) { # looks like JSON
    return $class->load_json_string($string);
  }
  else { # maybe doc-marker-free YAML
    return $class->load_yaml_string($string);
  }
}

sub load_yaml_string {
  my ($class, $string) = @_;
  my $backend = $class->yaml_backend();
  my $data = eval { no strict 'refs'; &{"$backend\::Load"}($string) };
  croak $@ if $@;
  return $data || {}; # in case document was valid but empty
}

sub load_json_string {
  my ($class, $string) = @_;
  require Encode;
  # load_json_string takes characters, decode_json expects bytes
  my $encoded = Encode::encode('UTF-8', $string, Encode::PERLQQ());
  my $data = eval { $class->json_decoder()->can('decode_json')->($encoded) };
  croak $@ if $@;
  return $data || {};
}

sub yaml_backend {
  if ($ENV{PERL_CORE} or not defined $ENV{PERL_YAML_BACKEND} ) {
    _can_load( 'CPAN::Meta::YAML', 0.011 )
      or croak "CPAN::Meta::YAML 0.011 is not available\n";
    return "CPAN::Meta::YAML";
  }
  else {
    my $backend = $ENV{PERL_YAML_BACKEND};
    _can_load( $backend )
      or croak "Could not load PERL_YAML_BACKEND '$backend'\n";
    $backend->can("Load")
      or croak "PERL_YAML_BACKEND '$backend' does not implement Load()\n";
    return $backend;
  }
}

sub json_decoder {
  if ($ENV{PERL_CORE}) {
    _can_load( 'JSON::PP' => 2.27300 )
      or croak "JSON::PP 2.27300 is not available\n";
    return 'JSON::PP';
  }
  if (my $decoder = $ENV{CPAN_META_JSON_DECODER}) {
    _can_load( $decoder )
      or croak "Could not load CPAN_META_JSON_DECODER '$decoder'\n";
    $decoder->can('decode_json')
      or croak "No decode_json sub provided by CPAN_META_JSON_DECODER '$decoder'\n";
    return $decoder;
  }
  return $_[0]->json_backend;
}

sub json_backend {
  if ($ENV{PERL_CORE}) {
    _can_load( 'JSON::PP' => 2.27300 )
      or croak "JSON::PP 2.27300 is not available\n";
    return 'JSON::PP';
  }
  if (my $backend = $ENV{CPAN_META_JSON_BACKEND}) {
    _can_load( $backend )
      or croak "Could not load CPAN_META_JSON_BACKEND '$backend'\n";
    $backend->can('new')
      or croak "No constructor provided by CPAN_META_JSON_BACKEND '$backend'\n";
    return $backend;
  }
  if (! $ENV{PERL_JSON_BACKEND} or $ENV{PERL_JSON_BACKEND} eq 'JSON::PP') {
    _can_load( 'JSON::PP' => 2.27300 )
      or croak "JSON::PP 2.27300 is not available\n";
    return 'JSON::PP';
  }
  else {
    _can_load( 'JSON' => 2.5 )
      or croak  "JSON 2.5 is required for " .
                "\$ENV{PERL_JSON_BACKEND} = '$ENV{PERL_JSON_BACKEND}'\n";
    return "JSON";
  }
}

sub _slurp {
  require Encode;
  open my $fh, "<:raw", "$_[0]" ## no critic
    or die "can't open $_[0] for reading: $!";
  my $content = do { local $/; <$fh> };
  $content = Encode::decode('UTF-8', $content, Encode::PERLQQ());
  return $content;
}

sub _can_load {
  my ($module, $version) = @_;
  (my $file = $module) =~ s{::}{/}g;
  $file .= ".pm";
  return 1 if $INC{$file};
  return 0 if exists $INC{$file}; # prior load failed
  eval { require $file; 1 }
    or return 0;
  if ( defined $version ) {
    eval { $module->VERSION($version); 1 }
      or return 0;
  }
  return 1;
}

# Kept for backwards compatibility only
# Create an object from a file
sub LoadFile ($) { ## no critic
  return Load(_slurp(shift));
}

# Parse a document from a string.
sub Load ($) { ## no critic
  require CPAN::Meta::YAML;
  my $object = eval { CPAN::Meta::YAML::Load(shift) };
  croak $@ if $@;
  return $object;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Parse::CPAN::Meta - Parse META.yml and META.json CPAN metadata files

=head1 VERSION

version 2.150010

=head1 SYNOPSIS

    #############################################
    # In your file

    ---
    name: My-Distribution
    version: 1.23
    resources:
      homepage: "http://example.com/dist/My-Distribution"


    #############################################
    # In your program

    use Parse::CPAN::Meta;

    my $distmeta = Parse::CPAN::Meta->load_file('META.yml');

    # Reading properties
    my $name     = $distmeta->{name};
    my $version  = $distmeta->{version};
    my $homepage = $distmeta->{resources}{homepage};

=head1 DESCRIPTION

B<Parse::CPAN::Meta> is a parser for F<META.json> and F<META.yml> files, using
L<JSON::PP> and/or L<CPAN::Meta::YAML>.

B<Parse::CPAN::Meta> provides three methods: C<load_file>, C<load_json_string>,
and C<load_yaml_string>.  These will read and deserialize CPAN metafiles, and
are described below in detail.

B<Parse::CPAN::Meta> provides a legacy API of only two functions,
based on the YAML functions of the same name. Wherever possible,
identical calling semantics are used.  These may only be used with YAML sources.

All error reporting is done with exceptions (die'ing).

Note that META files are expected to be in UTF-8 encoding, only.  When
converted string data, it must first be decoded from UTF-8.

=begin Pod::Coverage




=end Pod::Coverage

=head1 METHODS

=head2 load_file

  my $metadata_structure = Parse::CPAN::Meta->load_file('META.json');

  my $metadata_structure = Parse::CPAN::Meta->load_file('META.yml');

This method will read the named file and deserialize it to a data structure,
determining whether it should be JSON or YAML based on the filename.
The file will be read using the ":utf8" IO layer.

=head2 load_yaml_string

  my $metadata_structure = Parse::CPAN::Meta->load_yaml_string($yaml_string);

This method deserializes the given string of YAML and returns the first
document in it.  (CPAN metadata files should always have only one document.)
If the source was UTF-8 encoded, the string must be decoded before calling
C<load_yaml_string>.

=head2 load_json_string

  my $metadata_structure = Parse::CPAN::Meta->load_json_string($json_string);

This method deserializes the given string of JSON and the result.
If the source was UTF-8 encoded, the string must be decoded before calling
C<load_json_string>.

=head2 load_string

  my $metadata_structure = Parse::CPAN::Meta->load_string($some_string);

If you don't know whether a string contains YAML or JSON data, this method
will use some heuristics and guess.  If it can't tell, it assumes YAML.

=head2 yaml_backend

  my $backend = Parse::CPAN::Meta->yaml_backend;

Returns the module name of the YAML serializer. See L</ENVIRONMENT>
for details.

=head2 json_backend

  my $backend = Parse::CPAN::Meta->json_backend;

Returns the module name of the JSON serializer.  If C<CPAN_META_JSON_BACKEND>
is set, this will be whatever that's set to.  If not, this will either
be L<JSON::PP> or L<JSON>.  If C<PERL_JSON_BACKEND> is set,
this will return L<JSON> as further delegation is handled by
the L<JSON> module.  See L</ENVIRONMENT> for details.

=head2 json_decoder

  my $decoder = Parse::CPAN::Meta->json_decoder;

Returns the module name of the JSON decoder.  Unlike L</json_backend>, this
is not necessarily a full L<JSON>-style module, but only something that will
provide a C<decode_json> subroutine.  If C<CPAN_META_JSON_DECODER> is set,
this will be whatever that's set to.  If not, this will be whatever has
been selected as L</json_backend>.  See L</ENVIRONMENT> for more notes.

=head1 FUNCTIONS

For maintenance clarity, no functions are exported by default.  These functions
are available for backwards compatibility only and are best avoided in favor of
C<load_file>.

=head2 Load

  my @yaml = Parse::CPAN::Meta::Load( $string );

Parses a string containing a valid YAML stream into a list of Perl data
structures.

=head2 LoadFile

  my @yaml = Parse::CPAN::Meta::LoadFile( 'META.yml' );

Reads the YAML stream from a file instead of a string.

=head1 ENVIRONMENT

=head2 CPAN_META_JSON_DECODER

By default, L<JSON::PP> will be used for deserializing JSON data.  If the
C<CPAN_META_JSON_DECODER> environment variable exists, this is expected to
be the name of a loadable module that provides a C<decode_json> subroutine,
which will then be used for deserialization.  Relying only on the existence
of said subroutine allows for maximum compatibility, since this API is
provided by all of L<JSON::PP>, L<JSON::XS>, L<Cpanel::JSON::XS>,
L<JSON::MaybeXS>, L<JSON::Tiny>, and L<Mojo::JSON>.

=head2 CPAN_META_JSON_BACKEND

By default, L<JSON::PP> will be used for deserializing JSON data.  If the
C<CPAN_META_JSON_BACKEND> environment variable exists, this is expected to
be the name of a loadable module that provides the L<JSON> API, since
downstream code expects to be able to call C<new> on this class.  As such,
while L<JSON::PP>, L<JSON::XS>, L<Cpanel::JSON::XS> and L<JSON::MaybeXS> will
work for this, to use L<Mojo::JSON> or L<JSON::Tiny> for decoding requires
setting L</CPAN_META_JSON_DECODER>.

=head2 PERL_JSON_BACKEND

If the C<CPAN_META_JSON_BACKEND> environment variable does not exist, and if
C<PERL_JSON_BACKEND> environment variable exists, is true and is not
"JSON::PP", then the L<JSON> module (version 2.5 or greater) will be loaded and
used to interpret C<PERL_JSON_BACKEND>.  If L<JSON> is not installed or is too
old, an exception will be thrown.  Note that at the time of writing, the only
useful values are 1, which will tell L<JSON> to guess, or L<JSON::XS> - if
you want to use a newer JSON module, see L</CPAN_META_JSON_BACKEND>.

=head2 PERL_YAML_BACKEND

By default, L<CPAN::Meta::YAML> will be used for deserializing YAML data. If
the C<PERL_YAML_BACKEND> environment variable is defined, then it is interpreted
as a module to use for deserialization.  The given module must be installed,
must load correctly and must implement the C<Load()> function or an exception
will be thrown.

=head1 AUTHORS

=over 4

=item *

David Golden <dagolden@cpan.org>

=item *

Ricardo Signes <rjbs@cpan.org>

=item *

Adam Kennedy <adamk@cpan.org>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by David Golden, Ricardo Signes, Adam Kennedy and Contributors.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
