package OS2::PrfDB;

use strict;

require Exporter;
use XSLoader;
use Tie::Hash;

our $debug;
our @ISA = qw(Exporter Tie::Hash);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
our @EXPORT = qw(
		 AnyIni UserIni SystemIni
		);
our $VERSION = '0.04';

XSLoader::load 'OS2::PrfDB', $VERSION;

# Preloaded methods go here.

sub AnyIni {
  new_from_int OS2::PrfDB::Hini OS2::Prf::System(0), 
  'Anyone of two "systemish" databases', 1;
}

sub UserIni {
  new_from_int OS2::PrfDB::Hini OS2::Prf::System(1), 'User settings database', 1;
}

sub SystemIni {
  new_from_int OS2::PrfDB::Hini OS2::Prf::System(2),'System settings database',1;
}

# Internal structure 0 => HINI, 1 => array of entries, 2 => iterator.

sub TIEHASH {
  die "Usage: tie %arr, OS2::PrfDB, filename\n" unless @_ == 2;
  my ($obj, $file) = @_;
  my $hini = ref $file eq 'OS2::PrfDB::Hini' ? $file 
					     : new OS2::PrfDB::Hini $file;
  die "Error opening profile database `$file': $!" unless $hini;
  # print "tiehash `@_', hini $hini\n" if $debug;
  bless [$hini, undef, undef];
}

sub STORE {
  my ($self, $key, $val) = @_;
  die unless @_ == 3;
  die unless ref $val eq 'HASH';
  my %sub;
  tie %sub, 'OS2::PrfDB::Sub', $self->[0], $key;
  %sub = %$val;
}

sub FETCH {
  my ($self, $key) = @_;
  die unless @_ == 2;
  my %sub;
  tie %sub, 'OS2::PrfDB::Sub', $self->[0], $key;
  \%sub;
}

sub DELETE {
  my ($self, $key) = @_;
  die unless @_ == 2;
  my %sub;
  tie %sub, 'OS2::PrfDB::Sub', $self->[0], $key;
  %sub = ();
}

# CLEAR ???? - deletion of the whole

sub EXISTS {
  my ($self, $key) = @_;
  die unless @_ == 2;
  return OS2::Prf::GetLength($self->[0]->[0], $key, undef) >= 0;
}

sub FIRSTKEY {
  my $self = shift;
  my $keys = OS2::Prf::Get($self->[0]->[0], undef, undef);
  return undef unless defined $keys;
  chop($keys);
  $self->[1] = [split /\0/, $keys];
  # print "firstkey1 $self, `$self->[3]->[0], $self->[3]->[1]'\n" if $debug;
  $self->[2] = 0;
  return $self->[1]->[0];
	  # OS2::Prf::Get($self->[0]->[0], $self->[2], $self->[3]->[0]));
}

sub NEXTKEY {
  # print "nextkey `@_'\n" if $debug;
  my $self = shift;
  return undef unless $self->[2]++ < $#{$self->[1]};
  my $key = $self->[1]->[$self->[2]];
  return $key; #, OS2::Prf::Get($self->[0]->[0], $self->[2], $key));
}

package OS2::PrfDB::Hini;

sub new {
  die "Usage: new OS2::PrfDB::Hini filename\n" unless @_ == 2;
  shift;
  my $file = shift;
  my $hini = OS2::Prf::Open($file);
  die "Error opening profile database `$file': $!" unless $hini;
  bless [$hini, $file];
}

# Takes HINI and file name:

sub new_from_int { shift; bless [@_] }

# Internal structure 0 => HINI, 1 => filename, 2 => do-not-close.

sub DESTROY {
  my $self = shift; 
  my $hini = $self->[0];
  unless ($self->[2]) {
    OS2::Prf::Close($hini) or die "Error closing profile `$self->[1]': $!";
  }
}

package OS2::PrfDB::Sub;
use Tie::Hash;

our $debug;
our @ISA = qw{Tie::Hash};

# Internal structure 0 => HINI, 1 => array of entries, 2 => iterator,
# 3 => appname.

sub TIEHASH {
  die "Usage: tie %arr, OS2::PrfDB::Sub, filename, appname\n" unless @_ == 3;
  my ($obj, $file, $app) = @_;
  my $hini = ref $file eq 'OS2::PrfDB::Hini' ? $file 
					     : new OS2::PrfDB::Hini $file;
  die "Error opening profile database `$file': $!" unless $hini;
  # print "tiehash `@_', hini $hini\n" if $debug;
  bless [$hini, undef, undef, $app];
}

sub STORE {
  my ($self, $key, $val) = @_;
  die unless @_ == 3;
  OS2::Prf::Set($self->[0]->[0], $self->[3], $key, $val);
}

sub FETCH {
  my ($self, $key) = @_;
  die unless @_ == 2;
  OS2::Prf::Get($self->[0]->[0], $self->[3], $key);
}

sub DELETE {
  my ($self, $key) = @_;
  die unless @_ == 2;
  OS2::Prf::Set($self->[0]->[0], $self->[3], $key, undef);
}

# CLEAR ???? - deletion of the whole

sub EXISTS {
  my ($self, $key) = @_;
  die unless @_ == 2;
  return OS2::Prf::GetLength($self->[0]->[0], $self->[3], $key) >= 0;
}

sub FIRSTKEY {
  my $self = shift;
  my $keys = OS2::Prf::Get($self->[0]->[0], $self->[3], undef);
  return undef unless defined $keys;
  chop($keys);
  $self->[1] = [split /\0/, $keys];
  # print "firstkey1 $self, `$self->[3]->[0], $self->[3]->[1]'\n" if $debug;
  $self->[2] = 0;
  return $self->[1]->[0];
	  # OS2::Prf::Get($self->[0]->[0], $self->[2], $self->[3]->[0]));
}

sub NEXTKEY {
  # print "nextkey `@_'\n" if $debug;
  my $self = shift;
  return undef unless $self->[2]++ < $#{$self->[1]};
  my $key = $self->[1]->[$self->[2]];
  return $key; #, OS2::Prf::Get($self->[0]->[0], $self->[2], $key));
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

OS2::PrfDB - Perl extension for access to OS/2 setting database.

=head1 SYNOPSIS

  use OS2::PrfDB;
  tie %settings, OS2::PrfDB, 'my.ini';
  tie %subsettings, OS2::PrfDB::Sub, 'my.ini', 'mykey';

  print "$settings{firstkey}{subkey}\n";
  print "$subsettings{subkey}\n";

  tie %system, OS2::PrfDB, SystemIni;
  $system{myapp}{mykey} = "myvalue";


=head1 DESCRIPTION

The extension provides both high-level and low-level access to .ini
files. 

=head2 High level access

High-level access is the tie-hash access via two packages:
C<OS2::PrfDB> and C<OS2::PrfDB::Sub>. First one supports one argument,
the name of the file to open, the second one the name of the file to
open and so called I<Application name>, or the primary key of the
database.

  tie %settings, OS2::PrfDB, 'my.ini';
  tie %subsettings, OS2::PrfDB::Sub, 'my.ini', 'mykey';

One may substitute a handle for already opened ini-file instead of the
file name (obtained via low-level access functions). In particular, 3
functions SystemIni(), UserIni(), and AnyIni() provide handles to the
"systemish" databases. AniIni will read from both, and write into User
database.

=head2 Low-level access

Low-level access functions reside in the package C<OS2::Prf>. They are

=over 14

=item C<Open(file)>

Opens the database, returns an I<integer handle>.

=item C<Close(hndl)>

Closes the database given an I<integer handle>.

=item C<Get(hndl, appname, key)>

Retrieves data from the database given 2-part-key C<appname> C<key>.
If C<key> is C<undef>, return the "\0" delimited list of C<key>s,
terminated by \0. If C<appname> is C<undef>, returns the list of
possible C<appname>s in the same form.

=item C<GetLength(hndl, appname, key)>

Same as above, but returns the length of the value.

=item C<Set(hndl, appname, key, value [ , length ])>

Sets the value. If the C<value> is not defined, removes the C<key>. If
the C<key> is not defined, removes the C<appname>.

=item C<System(val)>

Return an I<integer handle> associated with the system database. If
C<val> is 1, it is I<User> database, if 2, I<System> database, if
0, handle for "both" of them: the handle works for read from any one,
and for write into I<User> one.

=item C<Profiles()>

returns a reference to a list of two strings, giving names of the
I<User> and I<System> databases.

=item C<SetUser(file)>

B<(Not tested.)> Sets the profile name of the I<User> database. The
application should have a message queue to use this function!

=back

=head2 Integer handles

To convert a name or an integer handle into an object acceptable as
argument to tie() interface, one may use the following functions from
the package C<OS2::Prf::Hini>:

=over 14

=item C<new(package, file)>

=item C<new_from_int(package, int_hndl [ , filename ])>

=back

=head2 Exports

SystemIni(), UserIni(), and AnyIni().

=head1 AUTHOR

Ilya Zakharevich, ilya@math.ohio-state.edu

=head1 SEE ALSO

perl(1).

=cut

