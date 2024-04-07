use strict; use warnings;

package Memoize::Storable;
our $VERSION = '1.16';

use Storable 1.002 (); # for lock_* function variants

our $Verbose;

sub TIEHASH {
  my $package = shift;
  my $filename = shift;
  my $truehash = (-e $filename) ? Storable::lock_retrieve($filename) : {};
  my %options;
  print STDERR "Memoize::Storable::TIEHASH($filename, @_)\n" if $Verbose;
  @options{@_} = (1) x @_;
  my $self = 
    {FILENAME => $filename, 
     H => $truehash, 
     OPTIONS => \%options
    };
  bless $self => $package;
}

sub STORE {
  my $self = shift;
  print STDERR "Memoize::Storable::STORE(@_)\n" if $Verbose;
  $self->{H}{$_[0]} = $_[1];
}

sub FETCH {
  my $self = shift;
  print STDERR "Memoize::Storable::FETCH(@_)\n" if $Verbose;
  $self->{H}{$_[0]};
}

sub EXISTS {
  my $self = shift;
  print STDERR "Memoize::Storable::EXISTS(@_)\n" if $Verbose;
  exists $self->{H}{$_[0]};
}

sub DESTROY {
  my $self= shift;
  print STDERR "Memoize::Storable::DESTROY(@_)\n" if $Verbose;
  if ($self->{OPTIONS}{'nstore'}) {
    Storable::lock_nstore($self->{H}, $self->{FILENAME});
  } else {
    Storable::lock_store($self->{H}, $self->{FILENAME});
  }
}

sub FIRSTKEY {
  'Fake hash from Memoize::Storable';
}

sub NEXTKEY {
  undef;
}

1;

__END__

=pod

=head1 NAME

Memoize::Storable - store Memoized data in Storable database

=head1 DESCRIPTION

See L<Memoize>.

=cut
