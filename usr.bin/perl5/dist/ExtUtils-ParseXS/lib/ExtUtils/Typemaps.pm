package ExtUtils::Typemaps;
use 5.006001;
use strict;
use warnings;
our $VERSION = '3.51';

require ExtUtils::ParseXS;
require ExtUtils::ParseXS::Constants;
require ExtUtils::Typemaps::InputMap;
require ExtUtils::Typemaps::OutputMap;
require ExtUtils::Typemaps::Type;

=head1 NAME

ExtUtils::Typemaps - Read/Write/Modify Perl/XS typemap files

=head1 SYNOPSIS

  # read/create file
  my $typemap = ExtUtils::Typemaps->new(file => 'typemap');
  # alternatively create an in-memory typemap
  # $typemap = ExtUtils::Typemaps->new();
  # alternatively create an in-memory typemap by parsing a string
  # $typemap = ExtUtils::Typemaps->new(string => $sometypemap);

  # add a mapping
  $typemap->add_typemap(ctype => 'NV', xstype => 'T_NV');
  $typemap->add_inputmap(
     xstype => 'T_NV', code => '$var = ($type)SvNV($arg);'
  );
  $typemap->add_outputmap(
     xstype => 'T_NV', code => 'sv_setnv($arg, (NV)$var);'
  );
  $typemap->add_string(string => $typemapstring);
                                           # will be parsed and merged

  # remove a mapping (same for remove_typemap and remove_outputmap...)
  $typemap->remove_inputmap(xstype => 'SomeType');

  # save a typemap to a file
  $typemap->write(file => 'anotherfile.map');

  # merge the other typemap into this one
  $typemap->merge(typemap => $another_typemap);

=head1 DESCRIPTION

This module can read, modify, create and write Perl XS typemap files. If you don't know
what a typemap is, please confer the L<perlxstut> and L<perlxs> manuals.

The module is not entirely round-trip safe: For example it currently simply strips all comments.
The order of entries in the maps is, however, preserved.

We check for duplicate entries in the typemap, but do not check for missing
C<TYPEMAP> entries for C<INPUTMAP> or C<OUTPUTMAP> entries since these might be hidden
in a different typemap.

=head1 METHODS

=cut

=head2 new

Returns a new typemap object. Takes an optional C<file> parameter.
If set, the given file will be read. If the file doesn't exist, an empty typemap
is returned.

Alternatively, if the C<string> parameter is given, the supplied
string will be parsed instead of a file.

=cut

sub new {
  my $class = shift;
  my %args = @_;

  if (defined $args{file} and defined $args{string}) {
    die("Cannot handle both 'file' and 'string' arguments to constructor");
  }

  my $self = bless {
    file            => undef,
    %args,
    typemap_section => [],
    typemap_lookup  => {},
    input_section   => [],
    input_lookup    => {},
    output_section  => [],
    output_lookup   => {},
  } => $class;

  $self->_init();

  return $self;
}

sub _init {
  my $self = shift;
  if (defined $self->{string}) {
    $self->_parse(\($self->{string}), $self->{lineno_offset}, $self->{fake_filename});
    delete $self->{string};
  }
  elsif (defined $self->{file} and -e $self->{file}) {
    open my $fh, '<', $self->{file}
      or die "Cannot open typemap file '"
             . $self->{file} . "' for reading: $!";
    local $/ = undef;
    my $string = <$fh>;
    $self->_parse(\$string, $self->{lineno_offset}, $self->{file});
  }
}


=head2 file

Get/set the file that the typemap is written to when the
C<write> method is called.

=cut

sub file {
  $_[0]->{file} = $_[1] if @_ > 1;
  $_[0]->{file}
}

=head2 add_typemap

Add a C<TYPEMAP> entry to the typemap.

Required named arguments: The C<ctype> (e.g. C<ctype =E<gt> 'double'>)
and the C<xstype> (e.g. C<xstype =E<gt> 'T_NV'>).

Optional named arguments: C<replace =E<gt> 1> forces removal/replacement of
existing C<TYPEMAP> entries of the same C<ctype>. C<skip =E<gt> 1>
triggers a I<"first come first serve"> logic by which new entries that conflict
with existing entries are silently ignored.

As an alternative to the named parameters usage, you may pass in
an C<ExtUtils::Typemaps::Type> object as first argument, a copy of which will be
added to the typemap. In that case, only the C<replace> or C<skip> named parameters
may be used after the object. Example:

  $map->add_typemap($type_obj, replace => 1);

=cut

sub add_typemap {
  my $self = shift;
  my $type;
  my %args;

  if ((@_ % 2) == 1) {
    my $orig = shift;
    $type = $orig->new();
    %args = @_;
  }
  else {
    %args = @_;
    my $ctype = $args{ctype};
    die("Need ctype argument") if not defined $ctype;
    my $xstype = $args{xstype};
    die("Need xstype argument") if not defined $xstype;

    $type = ExtUtils::Typemaps::Type->new(
      xstype      => $xstype,
      'prototype' => $args{'prototype'},
      ctype       => $ctype,
    );
  }

  if ($args{skip} and $args{replace}) {
    die("Cannot use both 'skip' and 'replace'");
  }

  if ($args{replace}) {
    $self->remove_typemap(ctype => $type->ctype);
  }
  elsif ($args{skip}) {
    return() if exists $self->{typemap_lookup}{$type->ctype};
  }
  else {
    $self->validate(typemap_xstype => $type->xstype, ctype => $type->ctype);
  }

  # store
  push @{$self->{typemap_section}}, $type;
  # remember type for lookup, too.
  $self->{typemap_lookup}{$type->tidy_ctype} = $#{$self->{typemap_section}};

  return 1;
}

=head2 add_inputmap

Add an C<INPUT> entry to the typemap.

Required named arguments:
The C<xstype> (e.g. C<xstype =E<gt> 'T_NV'>)
and the C<code> to associate with it for input.

Optional named arguments: C<replace =E<gt> 1> forces removal/replacement of
existing C<INPUT> entries of the same C<xstype>. C<skip =E<gt> 1>
triggers a I<"first come first serve"> logic by which new entries that conflict
with existing entries are silently ignored.

As an alternative to the named parameters usage, you may pass in
an C<ExtUtils::Typemaps::InputMap> object as first argument, a copy of which will be
added to the typemap. In that case, only the C<replace> or C<skip> named parameters
may be used after the object. Example:

  $map->add_inputmap($type_obj, replace => 1);

=cut

sub add_inputmap {
  my $self = shift;
  my $input;
  my %args;

  if ((@_ % 2) == 1) {
    my $orig = shift;
    $input = $orig->new();
    %args = @_;
  }
  else {
    %args = @_;
    my $xstype = $args{xstype};
    die("Need xstype argument") if not defined $xstype;
    my $code = $args{code};
    die("Need code argument") if not defined $code;

    $input = ExtUtils::Typemaps::InputMap->new(
      xstype => $xstype,
      code   => $code,
    );
  }

  if ($args{skip} and $args{replace}) {
    die("Cannot use both 'skip' and 'replace'");
  }

  if ($args{replace}) {
    $self->remove_inputmap(xstype => $input->xstype);
  }
  elsif ($args{skip}) {
    return() if exists $self->{input_lookup}{$input->xstype};
  }
  else {
    $self->validate(inputmap_xstype => $input->xstype);
  }

  # store
  push @{$self->{input_section}}, $input;
  # remember type for lookup, too.
  $self->{input_lookup}{$input->xstype} = $#{$self->{input_section}};

  return 1;
}

=head2 add_outputmap

Add an C<OUTPUT> entry to the typemap.
Works exactly the same as C<add_inputmap>.

=cut

sub add_outputmap {
  my $self = shift;
  my $output;
  my %args;

  if ((@_ % 2) == 1) {
    my $orig = shift;
    $output = $orig->new();
    %args = @_;
  }
  else {
    %args = @_;
    my $xstype = $args{xstype};
    die("Need xstype argument") if not defined $xstype;
    my $code = $args{code};
    die("Need code argument") if not defined $code;

    $output = ExtUtils::Typemaps::OutputMap->new(
      xstype => $xstype,
      code   => $code,
    );
  }

  if ($args{skip} and $args{replace}) {
    die("Cannot use both 'skip' and 'replace'");
  }

  if ($args{replace}) {
    $self->remove_outputmap(xstype => $output->xstype);
  }
  elsif ($args{skip}) {
    return() if exists $self->{output_lookup}{$output->xstype};
  }
  else {
    $self->validate(outputmap_xstype => $output->xstype);
  }

  # store
  push @{$self->{output_section}}, $output;
  # remember type for lookup, too.
  $self->{output_lookup}{$output->xstype} = $#{$self->{output_section}};

  return 1;
}

=head2 add_string

Parses a string as a typemap and merge it into the typemap object.

Required named argument: C<string> to specify the string to parse.

=cut

sub add_string {
  my $self = shift;
  my %args = @_;
  die("Need 'string' argument") if not defined $args{string};

  # no, this is not elegant.
  my $other = ExtUtils::Typemaps->new(string => $args{string});
  $self->merge(typemap => $other);
}

=head2 remove_typemap

Removes a C<TYPEMAP> entry from the typemap.

Required named argument: C<ctype> to specify the entry to remove from the typemap.

Alternatively, you may pass a single C<ExtUtils::Typemaps::Type> object.

=cut

sub remove_typemap {
  my $self = shift;
  my $ctype;
  if (@_ > 1) {
    my %args = @_;
    $ctype = $args{ctype};
    die("Need ctype argument") if not defined $ctype;
    $ctype = tidy_type($ctype);
  }
  else {
    $ctype = $_[0]->tidy_ctype;
  }

  return $self->_remove($ctype, $self->{typemap_section}, $self->{typemap_lookup});
}

=head2 remove_inputmap

Removes an C<INPUT> entry from the typemap.

Required named argument: C<xstype> to specify the entry to remove from the typemap.

Alternatively, you may pass a single C<ExtUtils::Typemaps::InputMap> object.

=cut

sub remove_inputmap {
  my $self = shift;
  my $xstype;
  if (@_ > 1) {
    my %args = @_;
    $xstype = $args{xstype};
    die("Need xstype argument") if not defined $xstype;
  }
  else {
    $xstype = $_[0]->xstype;
  }
  
  return $self->_remove($xstype, $self->{input_section}, $self->{input_lookup});
}

=head2 remove_outputmap

Removes an C<OUTPUT> entry from the typemap.

Required named argument: C<xstype> to specify the entry to remove from the typemap.

Alternatively, you may pass a single C<ExtUtils::Typemaps::OutputMap> object.

=cut

sub remove_outputmap {
  my $self = shift;
  my $xstype;
  if (@_ > 1) {
    my %args = @_;
    $xstype = $args{xstype};
    die("Need xstype argument") if not defined $xstype;
  }
  else {
    $xstype = $_[0]->xstype;
  }
  
  return $self->_remove($xstype, $self->{output_section}, $self->{output_lookup});
}

sub _remove {
  my $self   = shift;
  my $rm     = shift;
  my $array  = shift;
  my $lookup = shift;

  # Just fetch the index of the item from the lookup table
  my $index = $lookup->{$rm};
  return() if not defined $index;

  # Nuke the item from storage
  splice(@$array, $index, 1);

  # Decrement the storage position of all items thereafter
  foreach my $key (keys %$lookup) {
    if ($lookup->{$key} > $index) {
      $lookup->{$key}--;
    }
  }
  return();
}

=head2 get_typemap

Fetches an entry of the TYPEMAP section of the typemap.

Mandatory named arguments: The C<ctype> of the entry.

Returns the C<ExtUtils::Typemaps::Type>
object for the entry if found.

=cut

sub get_typemap {
  my $self = shift;
  die("Need named parameters, got uneven number") if @_ % 2;

  my %args = @_;
  my $ctype = $args{ctype};
  die("Need ctype argument") if not defined $ctype;
  $ctype = tidy_type($ctype);

  my $index = $self->{typemap_lookup}{$ctype};
  return() if not defined $index;
  return $self->{typemap_section}[$index];
}

=head2 get_inputmap

Fetches an entry of the INPUT section of the
typemap.

Mandatory named arguments: The C<xstype> of the
entry or the C<ctype> of the typemap that can be used to find
the C<xstype>. To wit, the following pieces of code
are equivalent:

  my $type = $typemap->get_typemap(ctype => $ctype)
  my $input_map = $typemap->get_inputmap(xstype => $type->xstype);

  my $input_map = $typemap->get_inputmap(ctype => $ctype);

Returns the C<ExtUtils::Typemaps::InputMap>
object for the entry if found.

=cut

sub get_inputmap {
  my $self = shift;
  die("Need named parameters, got uneven number") if @_ % 2;

  my %args = @_;
  my $xstype = $args{xstype};
  my $ctype  = $args{ctype};
  die("Need xstype or ctype argument")
    if not defined $xstype
    and not defined $ctype;
  die("Need xstype OR ctype arguments, not both")
    if defined $xstype and defined $ctype;

  if (defined $ctype) {
    my $tm = $self->get_typemap(ctype => $ctype);
    $xstype = $tm && $tm->xstype;
    return() if not defined $xstype;
  }

  my $index = $self->{input_lookup}{$xstype};
  return() if not defined $index;
  return $self->{input_section}[$index];
}

=head2 get_outputmap

Fetches an entry of the OUTPUT section of the
typemap.

Mandatory named arguments: The C<xstype> of the
entry or the C<ctype> of the typemap that can be used to
resolve the C<xstype>. (See above for an example.)

Returns the C<ExtUtils::Typemaps::InputMap>
object for the entry if found.

=cut

sub get_outputmap {
  my $self = shift;
  die("Need named parameters, got uneven number") if @_ % 2;

  my %args = @_;
  my $xstype = $args{xstype};
  my $ctype  = $args{ctype};
  die("Need xstype or ctype argument")
    if not defined $xstype
    and not defined $ctype;
  die("Need xstype OR ctype arguments, not both")
    if defined $xstype and defined $ctype;

  if (defined $ctype) {
    my $tm = $self->get_typemap(ctype => $ctype);
    $xstype = $tm && $tm->xstype;
    return() if not defined $xstype;
  }

  my $index = $self->{output_lookup}{$xstype};
  return() if not defined $index;
  return $self->{output_section}[$index];
}

=head2 write

Write the typemap to a file. Optionally takes a C<file> argument. If given, the
typemap will be written to the specified file. If not, the typemap is written
to the currently stored file name (see L</file> above, this defaults to the file
it was read from if any).

=cut

sub write {
  my $self = shift;
  my %args = @_;
  my $file = defined $args{file} ? $args{file} : $self->file();
  die("write() needs a file argument (or set the file name of the typemap using the 'file' method)")
    if not defined $file;

  open my $fh, '>', $file
    or die "Cannot open typemap file '$file' for writing: $!";
  print $fh $self->as_string();
  close $fh;
}

=head2 as_string

Generates and returns the string form of the typemap.

=cut

sub as_string {
  my $self = shift;
  my $typemap = $self->{typemap_section};
  my @code;
  push @code, "TYPEMAP\n";
  foreach my $entry (@$typemap) {
    # type kind proto
    # /^(.*?\S)\s+(\S+)\s*($ExtUtils::ParseXS::Constants::PrototypeRegexp*)$/o
    push @code, $entry->ctype . "\t" . $entry->xstype
              . ($entry->proto ne '' ? "\t".$entry->proto : '') . "\n";
  }

  my $input = $self->{input_section};
  if (@$input) {
    push @code, "\nINPUT\n";
    foreach my $entry (@$input) {
      push @code, $entry->xstype, "\n", $entry->code, "\n";
    }
  }

  my $output = $self->{output_section};
  if (@$output) {
    push @code, "\nOUTPUT\n";
    foreach my $entry (@$output) {
      push @code, $entry->xstype, "\n", $entry->code, "\n";
    }
  }
  return join '', @code;
}

=head2 as_embedded_typemap

Generates and returns the string form of the typemap with the
appropriate prefix around it for verbatim inclusion into an
XS file as an embedded typemap. This will return a string like

  TYPEMAP: <<END_OF_TYPEMAP
  ... typemap here (see as_string) ...
  END_OF_TYPEMAP

The method takes care not to use a HERE-doc end marker that
appears in the typemap string itself.

=cut

sub as_embedded_typemap {
  my $self = shift;
  my $string = $self->as_string;

  my @ident_cand = qw(END_TYPEMAP END_OF_TYPEMAP END);
  my $icand = 0;
  my $cand_suffix = "";
  while ($string =~ /^\Q$ident_cand[$icand]$cand_suffix\E\s*$/m) {
    $icand++;
    if ($icand == @ident_cand) {
      $icand = 0;
      ++$cand_suffix;
    }
  }

  my $marker = "$ident_cand[$icand]$cand_suffix";
  return "TYPEMAP: <<$marker;\n$string\n$marker\n";
}

=head2 merge

Merges a given typemap into the object. Note that a failed merge
operation leaves the object in an inconsistent state so clone it if necessary.

Mandatory named arguments: Either C<typemap =E<gt> $another_typemap_obj>
or C<file =E<gt> $path_to_typemap_file> but not both.

Optional arguments: C<replace =E<gt> 1> to force replacement
of existing typemap entries without warning or C<skip =E<gt> 1>
to skip entries that exist already in the typemap.

=cut

sub merge {
  my $self = shift;
  my %args = @_;

  if (exists $args{typemap} and exists $args{file}) {
    die("Need {file} OR {typemap} argument. Not both!");
  }
  elsif (not exists $args{typemap} and not exists $args{file}) {
    die("Need {file} or {typemap} argument!");
  }

  my @params;
  push @params, 'replace' => $args{replace} if exists $args{replace};
  push @params, 'skip' => $args{skip} if exists $args{skip};

  my $typemap = $args{typemap};
  if (not defined $typemap) {
    $typemap = ref($self)->new(file => $args{file}, @params);
  }

  # FIXME breaking encapsulation. Add accessor code.
  foreach my $entry (@{$typemap->{typemap_section}}) {
    $self->add_typemap( $entry, @params );
  }

  foreach my $entry (@{$typemap->{input_section}}) {
    $self->add_inputmap( $entry, @params );
  }

  foreach my $entry (@{$typemap->{output_section}}) {
    $self->add_outputmap( $entry, @params );
  }

  return 1;
}

=head2 is_empty

Returns a bool indicating whether this typemap is entirely empty.

=cut

sub is_empty {
  my $self = shift;

  return @{ $self->{typemap_section} } == 0
      && @{ $self->{input_section} } == 0
      && @{ $self->{output_section} } == 0;
}

=head2 list_mapped_ctypes

Returns a list of the C types that are mappable by
this typemap object.

=cut

sub list_mapped_ctypes {
  my $self = shift;
  return sort keys %{ $self->{typemap_lookup} };
}

=head2 _get_typemap_hash

Returns a hash mapping the C types to the XS types:

  {
    'char **' => 'T_PACKEDARRAY',
    'bool_t' => 'T_IV',
    'AV *' => 'T_AVREF',
    'InputStream' => 'T_IN',
    'double' => 'T_DOUBLE',
    # ...
  }

This is documented because it is used by C<ExtUtils::ParseXS>,
but it's not intended for general consumption. May be removed
at any time.

=cut

sub _get_typemap_hash {
  my $self = shift;
  my $lookup  = $self->{typemap_lookup};
  my $storage = $self->{typemap_section};

  my %rv;
  foreach my $ctype (keys %$lookup) {
    $rv{$ctype} = $storage->[ $lookup->{$ctype} ]->xstype;
  }

  return \%rv;
}

=head2 _get_inputmap_hash

Returns a hash mapping the XS types (identifiers) to the
corresponding INPUT code:

  {
    'T_CALLBACK' => '   $var = make_perl_cb_$type($arg)
  ',
    'T_OUT' => '    $var = IoOFP(sv_2io($arg))
  ',
    'T_REF_IV_PTR' => '   if (sv_isa($arg, \\"${ntype}\\")) {
    # ...
  }

This is documented because it is used by C<ExtUtils::ParseXS>,
but it's not intended for general consumption. May be removed
at any time.

=cut

sub _get_inputmap_hash {
  my $self = shift;
  my $lookup  = $self->{input_lookup};
  my $storage = $self->{input_section};

  my %rv;
  foreach my $xstype (keys %$lookup) {
    $rv{$xstype} = $storage->[ $lookup->{$xstype} ]->code;

    # Squash trailing whitespace to one line break
    # This isn't strictly necessary, but makes the output more similar
    # to the original ExtUtils::ParseXS.
    $rv{$xstype} =~ s/\s*\z/\n/;
  }

  return \%rv;
}


=head2 _get_outputmap_hash

Returns a hash mapping the XS types (identifiers) to the
corresponding OUTPUT code:

  {
    'T_CALLBACK' => '   sv_setpvn($arg, $var.context.value().chp(),
                $var.context.value().size());
  ',
    'T_OUT' => '    {
            GV *gv = (GV *)sv_newmortal();
            gv_init_pvn(gv, gv_stashpvs("$Package",1),
                       "__ANONIO__",10,0);
            if ( do_open(gv, "+>&", 3, FALSE, 0, 0, $var) )
                sv_setsv(
                  $arg,
                  sv_bless(newRV((SV*)gv), gv_stashpv("$Package",1))
                );
            else
                $arg = &PL_sv_undef;
         }
  ',
    # ...
  }

This is documented because it is used by C<ExtUtils::ParseXS>,
but it's not intended for general consumption. May be removed
at any time.

=cut

sub _get_outputmap_hash {
  my $self = shift;
  my $lookup  = $self->{output_lookup};
  my $storage = $self->{output_section};

  my %rv;
  foreach my $xstype (keys %$lookup) {
    $rv{$xstype} = $storage->[ $lookup->{$xstype} ]->code;

    # Squash trailing whitespace to one line break
    # This isn't strictly necessary, but makes the output more similar
    # to the original ExtUtils::ParseXS.
    $rv{$xstype} =~ s/\s*\z/\n/;
  }

  return \%rv;
}

=head2 _get_prototype_hash

Returns a hash mapping the C types of the typemap to their
corresponding prototypes.

  {
    'char **' => '$',
    'bool_t' => '$',
    'AV *' => '$',
    'InputStream' => '$',
    'double' => '$',
    # ...
  }

This is documented because it is used by C<ExtUtils::ParseXS>,
but it's not intended for general consumption. May be removed
at any time.

=cut

sub _get_prototype_hash {
  my $self = shift;
  my $lookup  = $self->{typemap_lookup};
  my $storage = $self->{typemap_section};

  my %rv;
  foreach my $ctype (keys %$lookup) {
    $rv{$ctype} = $storage->[ $lookup->{$ctype} ]->proto || '$';
  }

  return \%rv;
}



# make sure that the provided types wouldn't collide with what's
# in the object already.
sub validate {
  my $self = shift;
  my %args = @_;

  if ( exists $args{ctype}
       and exists $self->{typemap_lookup}{tidy_type($args{ctype})} )
  {
    die("Multiple definition of ctype '$args{ctype}' in TYPEMAP section");
  }

  if ( exists $args{inputmap_xstype}
       and exists $self->{input_lookup}{$args{inputmap_xstype}} )
  {
    die("Multiple definition of xstype '$args{inputmap_xstype}' in INPUTMAP section");
  }

  if ( exists $args{outputmap_xstype}
       and exists $self->{output_lookup}{$args{outputmap_xstype}} )
  {
    die("Multiple definition of xstype '$args{outputmap_xstype}' in OUTPUTMAP section");
  }

  return 1;
}

=head2 clone

Creates and returns a clone of a full typemaps object.

Takes named parameters: If C<shallow> is true,
the clone will share the actual individual type/input/outputmap objects,
but not share their storage. Use with caution. Without C<shallow>,
the clone will be fully independent.

=cut

sub clone {
  my $proto = shift;
  my %args = @_;

  my $self;
  if ($args{shallow}) {
    $self = bless( {
      %$proto,
      typemap_section => [@{$proto->{typemap_section}}],
      typemap_lookup  => {%{$proto->{typemap_lookup}}},
      input_section   => [@{$proto->{input_section}}],
      input_lookup    => {%{$proto->{input_lookup}}},
      output_section  => [@{$proto->{output_section}}],
      output_lookup   => {%{$proto->{output_lookup}}},
    } => ref($proto) );
  }
  else {
    $self = bless( {
      %$proto,
      typemap_section => [map $_->new, @{$proto->{typemap_section}}],
      typemap_lookup  => {%{$proto->{typemap_lookup}}},
      input_section   => [map $_->new, @{$proto->{input_section}}],
      input_lookup    => {%{$proto->{input_lookup}}},
      output_section  => [map $_->new, @{$proto->{output_section}}],
      output_lookup   => {%{$proto->{output_lookup}}},
    } => ref($proto) );
  }

  return $self;
}

=head2 tidy_type

Function to (heuristically) canonicalize a C type. Works to some
degree with C++ types.

    $halfway_canonical_type = tidy_type($ctype);

Moved from C<ExtUtils::ParseXS>.

=cut

sub tidy_type {
  local $_ = shift;

  # for templated C++ types, do some bit of flawed canonicalization
  # wrt. templates at least
  if (/[<>]/) {
    s/\s*([<>])\s*/$1/g;
    s/>>/> >/g;
  }

  # rationalise any '*' by joining them into bunches and removing whitespace
  s#\s*(\*+)\s*#$1#g;
  s#(\*+)# $1 #g ;

  # trim leading & trailing whitespace
  s/^\s+//; s/\s+$//;

  # change multiple whitespace into a single space
  s/\s+/ /g;

  $_;
}



sub _parse {
  my $self = shift;
  my $stringref = shift;
  my $lineno_offset = shift;
  $lineno_offset = 0 if not defined $lineno_offset;
  my $filename = shift;
  $filename = '<string>' if not defined $filename;

  my $replace = $self->{replace};
  my $skip    = $self->{skip};
  die "Can only replace OR skip" if $replace and $skip;
  my @add_params;
  push @add_params, replace => 1 if $replace;
  push @add_params, skip    => 1 if $skip;

  # TODO comments should round-trip, currently ignoring
  # TODO order of sections, multiple sections of same type
  # Heavily influenced by ExtUtils::ParseXS
  my $section = 'typemap';
  my $lineno = $lineno_offset;
  my $junk = "";
  my $current = \$junk;
  my @input_expr;
  my @output_expr;
  while ($$stringref =~ /^(.*)$/gcm) {
    local $_ = $1;
    ++$lineno;
    chomp;
    next if /^\s*#/;
    if (/^INPUT\s*$/) {
      $section = 'input';
      $current = \$junk;
      next;
    }
    elsif (/^OUTPUT\s*$/) {
      $section = 'output';
      $current = \$junk;
      next;
    }
    elsif (/^TYPEMAP\s*$/) {
      $section = 'typemap';
      $current = \$junk;
      next;
    }
    
    if ($section eq 'typemap') {
      my $line = $_;
      s/^\s+//; s/\s+$//;
      next if $_ eq '' or /^#/;
      my($type, $kind, $proto) = /^(.*?\S)\s+(\S+)\s*($ExtUtils::ParseXS::Constants::PrototypeRegexp*)$/o
        or warn("Warning: File '$filename' Line $lineno '$line' TYPEMAP entry needs 2 or 3 columns\n"),
           next;
      # prototype defaults to '$'
      $proto = '$' unless $proto;
      warn("Warning: File '$filename' Line $lineno '$line' Invalid prototype '$proto'\n")
        unless _valid_proto_string($proto);
      $self->add_typemap(
        ExtUtils::Typemaps::Type->new(
          xstype => $kind, proto => $proto, ctype => $type
        ),
        @add_params
      );
    } elsif (/^\s/) {
      s/\s+$//;
      $$current .= $$current eq '' ? $_ : "\n".$_;
    } elsif ($_ eq '') {
      next;
    } elsif ($section eq 'input') {
      s/\s+$//;
      push @input_expr, {xstype => $_, code => ''};
      $current = \$input_expr[-1]{code};
    } else { # output section
      s/\s+$//;
      push @output_expr, {xstype => $_, code => ''};
      $current = \$output_expr[-1]{code};
    }

  } # end while lines

  foreach my $inexpr (@input_expr) {
    $self->add_inputmap( ExtUtils::Typemaps::InputMap->new(%$inexpr), @add_params );
  }
  foreach my $outexpr (@output_expr) {
    $self->add_outputmap( ExtUtils::Typemaps::OutputMap->new(%$outexpr), @add_params );
  }

  return 1;
}

# taken from ExtUtils::ParseXS
sub _valid_proto_string {
  my $string = shift;
  if ($string =~ /^$ExtUtils::ParseXS::Constants::PrototypeRegexp+$/o) {
    return $string;
  }

  return 0 ;
}

# taken from ExtUtils::ParseXS (C_string)
sub _escape_backslashes {
  my $string = shift;
  $string =~ s[\\][\\\\]g;
  $string;
}

=head1 CAVEATS

Inherits some evil code from C<ExtUtils::ParseXS>.

=head1 SEE ALSO

The parser is heavily inspired from the one in L<ExtUtils::ParseXS>.

For details on typemaps: L<perlxstut>, L<perlxs>.

=head1 AUTHOR

Steffen Mueller C<<smueller@cpan.org>>

=head1 COPYRIGHT & LICENSE

Copyright 2009, 2010, 2011, 2012, 2013 Steffen Mueller

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut

1;

