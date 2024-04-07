package File::Spec::AmigaOS;

use strict;
require File::Spec::Unix;

our $VERSION = '3.88';
$VERSION =~ tr/_//d;

our @ISA = qw(File::Spec::Unix);

=head1 NAME

File::Spec::AmigaOS - File::Spec for AmigaOS

=head1 SYNOPSIS

 require File::Spec::AmigaOS; # Done automatically by File::Spec
                              # if needed

=head1 DESCRIPTION

Methods for manipulating file specifications.

=head1 METHODS

=over 2

=item tmpdir

Returns $ENV{TMPDIR} or if that is unset, "/t".

=cut

my $tmpdir;
sub tmpdir {
  return $tmpdir if defined $tmpdir;
  $tmpdir = $_[0]->_tmpdir( $ENV{TMPDIR}, "/t" );
}

=item file_name_is_absolute

Returns true if there's a colon in the file name,
or if it begins with a slash.

=cut

sub file_name_is_absolute {
  my ($self, $file) = @_;

  # Not 100% robust as a "/" must not preceded a ":"
  # but this cannot happen in a well formed path.
  return $file =~ m{^/|:}s;
}

=back

All the other methods are from L<File::Spec::Unix>.

=cut

1;
