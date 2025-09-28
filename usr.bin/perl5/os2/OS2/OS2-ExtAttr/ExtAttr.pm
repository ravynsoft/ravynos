package OS2::ExtAttr;

use strict;
use XSLoader;

our $VERSION = '0.04';
XSLoader::load 'OS2::ExtAttr', $VERSION;

# Preloaded methods go here.

# Format of the array: 
# 0 ead, 1 file name, 2 file handle. 3 length, 4 position, 5 need to write.

sub TIEHASH {
  my $class = shift;
  my $ea = _create() || die "Cannot create EA: $!";
  my $file = shift;
  my ($name, $handle);
  if (ref $file eq 'GLOB' or ref \$file eq 'GLOB') {
    die "File handle is not opened" unless $handle = fileno $file;
    _read($ea, undef, $handle, 0);
  } else {
    $name = $file;
    _read($ea, $name, 0, 0);
  }
  bless [$ea, $name, $handle, 0, 0, 0], $class;
}

sub DESTROY {
  my $eas = shift;
  # 0 means: discard eas which are not in $eas->[0].
  _write( $eas->[0], $eas->[1], $eas->[2], 0) and die "Cannot write EA: $!"
    if $eas->[5];
  _destroy( $eas->[0] );
}

sub FIRSTKEY {
  my $eas = shift;
  $eas->[3] = _count($eas->[0]);
  $eas->[4] = 1;
  return undef if $eas->[4] > $eas->[3];
  return _get_name($eas->[0], $eas->[4]);
}

sub NEXTKEY {
  my $eas = shift;
  $eas->[4]++;
  return undef if $eas->[4] > $eas->[3];
  return _get_name($eas->[0], $eas->[4]);
}

sub FETCH {
  my $eas = shift;
  my $index = _find($eas->[0], shift);
  return undef if $index <= 0;
  return value($eas->[0], $index);
}

sub EXISTS {
  my $eas = shift;
  return _find($eas->[0], shift) > 0;
}

sub STORE {
  my $eas = shift;
  $eas->[5] = 1;
  add($eas->[0], shift, shift) > 0 or die "Error setting EA: $!";
}

sub DELETE {
  my $eas = shift;
  my $index = _find($eas->[0], shift);
  return undef if $index <= 0;
  my $value = value($eas->[0], $index);
  _delete($eas->[0], $index) and die "Error deleting EA: $!";
  $eas->[5] = 1;
  return $value;
}

sub CLEAR {
  my $eas = shift;
  _clear($eas->[0]);
  $eas->[5] = 1;
}

# Here are additional methods:

*new = \&TIEHASH;

sub copy {
  my $eas = shift;
  my $file = shift;
  my ($name, $handle);
  if (ref $file eq 'GLOB' or ref \$file eq 'GLOB') {
    die "File handle is not opened" unless $handle = fileno $file;
    _write($eas->[0], undef, $handle, 0) or die "Cannot write EA: $!";
  } else {
    $name = $file;
    _write($eas->[0], $name, 0, 0) or die "Cannot write EA: $!";
  }
}

sub update {
  my $eas = shift;
  # 0 means: discard eas which are not in $eas->[0].
  _write( $eas->[0], $eas->[1], $eas->[2], 0) and die "Cannot write EA: $!";
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

OS2::ExtAttr - Perl access to extended attributes.

=head1 SYNOPSIS

  use OS2::ExtAttr;
  tie %ea, 'OS2::ExtAttr', 'my.file';
  print $ea{eaname};
  $ea{myfield} = 'value';

  untie %ea;

=head1 DESCRIPTION

The package provides low-level and high-level interface to Extended
Attributes under OS/2. 

=head2 High-level interface: C<tie>

The only argument of tie() is a file name, or an open file handle.

Note that all the changes of the tied hash happen in core, to
propagate it to disk the tied hash should be untie()ed or should go
out of scope. Alternatively, one may use the low-level C<update>
method on the corresponding object. Example:

  tied(%hash)->update;

Note also that setting/getting EA flag is not supported by the
high-level interface, one should use the low-level interface
instead. To use it on a tied hash one needs undocumented way to find
C<eas> give the tied hash.

=head2 Low-level interface

Two low-level methods are supported by the objects: copy() and
update(). The copy() takes one argument: the name of a file to copy
the attributes to, or an opened file handle. update() takes no
arguments, and is discussed above.

Three convenience functions are provided:

  value($eas, $key)
  add($eas, $key, $value [, $flag])
  replace($eas, $key, $value [, $flag])

The default value for C<flag> is 0.

In addition, all the C<_ea_*> and C<_ead_*> functions defined in EMX
library are supported, with leading C<_ea> and C<_ead> stripped.

=head1 AUTHOR

Ilya Zakharevich, ilya@math.ohio-state.edu

=head1 SEE ALSO

perl(1).

=cut
