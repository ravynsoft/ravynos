# Net::Netrc.pm
#
# Copyright (C) 1995-1998 Graham Barr.  All rights reserved.
# Copyright (C) 2013-2014, 2020 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package Net::Netrc;

use 5.008001;

use strict;
use warnings;

use Carp;
use FileHandle;

our $VERSION = "3.15";

our $TESTING;

my %netrc = ();

sub _readrc {
  my($class, $host) = @_;
  my ($home, $file);

  if ($^O eq "MacOS") {
    $home = $ENV{HOME} || `pwd`;
    chomp($home);
    $file = ($home =~ /:$/ ? $home . "netrc" : $home . ":netrc");
  }
  else {

    # Some OS's don't have "getpwuid", so we default to $ENV{HOME}
    $home = eval { (getpwuid($>))[7] } || $ENV{HOME};
    $home ||= $ENV{HOMEDRIVE} . ($ENV{HOMEPATH} || '') if defined $ENV{HOMEDRIVE};
    if (-e $home . "/.netrc") {
      $file = $home . "/.netrc";
    }
    elsif (-e $home . "/_netrc") {
      $file = $home . "/_netrc";
    }
    else {
      return unless $TESTING;
    }
  }

  my ($login, $pass, $acct) = (undef, undef, undef);
  my $fh;
  local $_;

  $netrc{default} = undef;

  # OS/2 and Win32 do not handle stat in a way compatible with this check :-(
  unless ($^O eq 'os2'
    || $^O eq 'MSWin32'
    || $^O eq 'MacOS'
    || $^O =~ /^cygwin/)
  {
    my @stat = stat($file);

    if (@stat) {
      if ($stat[2] & 077) { ## no critic (ValuesAndExpressions::ProhibitLeadingZeros)
        carp "Bad permissions: $file";
        return;
      }
      if ($stat[4] != $<) {
        carp "Not owner: $file";
        return;
      }
    }
  }

  if ($fh = FileHandle->new($file, "r")) {
    my ($mach, $macdef, $tok, @tok) = (0, 0);

    while (<$fh>) {
      undef $macdef if /\A\n\Z/;

      if ($macdef) {
        push(@$macdef, $_);
        next;
      }

      s/^\s*//;
      chomp;

      while (length && s/^("((?:[^"]+|\\.)*)"|((?:[^\\\s]+|\\.)*))\s*//) {
        (my $tok = $+) =~ s/\\(.)/$1/g;
        push(@tok, $tok);
      }

    TOKEN:
      while (@tok) {
        if ($tok[0] eq "default") {
          shift(@tok);
          $mach = bless {}, $class;
          $netrc{default} = [$mach];

          next TOKEN;
        }

        last TOKEN
          unless @tok > 1;

        $tok = shift(@tok);

        if ($tok eq "machine") {
          my $host = shift @tok;
          $mach = bless {machine => $host}, $class;

          $netrc{$host} = []
            unless exists($netrc{$host});
          push(@{$netrc{$host}}, $mach);
        }
        elsif ($tok =~ /^(login|password|account)$/) {
          next TOKEN unless $mach;
          my $value = shift @tok;

          # Following line added by rmerrell to remove '/' escape char in .netrc
          $value =~ s/\/\\/\\/g;
          $mach->{$1} = $value;
        }
        elsif ($tok eq "macdef") {
          next TOKEN unless $mach;
          my $value = shift @tok;
          $mach->{macdef} = {}
            unless exists $mach->{macdef};
          $macdef = $mach->{machdef}{$value} = [];
        }
      }
    }
    $fh->close();
  }
}


sub lookup {
  my ($class, $mach, $login) = @_;

  $class->_readrc()
    unless exists $netrc{default};

  $mach ||= 'default';
  undef $login
    if $mach eq 'default';

  if (exists $netrc{$mach}) {
    if (defined $login) {
      foreach my $m (@{$netrc{$mach}}) {
        return $m
          if (exists $m->{login} && $m->{login} eq $login);
      }
      return;
    }
    return $netrc{$mach}->[0];
  }

  return $netrc{default}->[0]
    if defined $netrc{default};

  return;
}


sub login {
  my $me = shift;

  exists $me->{login}
    ? $me->{login}
    : undef;
}


sub account {
  my $me = shift;

  exists $me->{account}
    ? $me->{account}
    : undef;
}


sub password {
  my $me = shift;

  exists $me->{password}
    ? $me->{password}
    : undef;
}


sub lpa {
  my $me = shift;
  ($me->login, $me->password, $me->account);
}

1;

__END__

=head1 NAME

Net::Netrc - OO interface to users netrc file

=head1 SYNOPSIS

    use Net::Netrc;

    $mach = Net::Netrc->lookup('some.machine');
    $login = $mach->login;
    ($login, $password, $account) = $mach->lpa;

=head1 DESCRIPTION

C<Net::Netrc> is a class implementing a simple interface to the .netrc file
used as by the ftp program.

C<Net::Netrc> also implements security checks just like the ftp program,
these checks are, first that the .netrc file must be owned by the user and 
second the ownership permissions should be such that only the owner has
read and write access. If these conditions are not met then a warning is
output and the .netrc file is not read.

=head2 The F<.netrc> File

The .netrc file contains login and initialization information used by the
auto-login process.  It resides in the user's home directory.  The following
tokens are recognized; they may be separated by spaces, tabs, or new-lines:

=over 4

=item machine name

Identify a remote machine name. The auto-login process searches
the .netrc file for a machine token that matches the remote machine
specified.  Once a match is made, the subsequent .netrc tokens
are processed, stopping when the end of file is reached or an-
other machine or a default token is encountered.

=item default

This is the same as machine name except that default matches
any name.  There can be only one default token, and it must be
after all machine tokens.  This is normally used as:

    default login anonymous password user@site

thereby giving the user automatic anonymous login to machines
not specified in .netrc.

=item login name

Identify a user on the remote machine.  If this token is present,
the auto-login process will initiate a login using the
specified name.

=item password string

Supply a password.  If this token is present, the auto-login
process will supply the specified string if the remote server
requires a password as part of the login process.

=item account string

Supply an additional account password.  If this token is present,
the auto-login process will supply the specified string
if the remote server requires an additional account password.

=item macdef name

Define a macro. C<Net::Netrc> only parses this field to be compatible
with I<ftp>.

=back

=head2 Class Methods

The constructor for a C<Net::Netrc> object is not called new as it does not
really create a new object. But instead is called C<lookup> as this is
essentially what it does.

=over 4

=item C<lookup($machine[, $login])>

Lookup and return a reference to the entry for C<$machine>. If C<$login> is given
then the entry returned will have the given login. If C<$login> is not given then
the first entry in the .netrc file for C<$machine> will be returned.

If a matching entry cannot be found, and a default entry exists, then a
reference to the default entry is returned.

If there is no matching entry found and there is no default defined, or
no .netrc file is found, then C<undef> is returned.

=back

=head2 Object Methods

=over 4

=item C<login()>

Return the login id for the netrc entry

=item C<password()>

Return the password for the netrc entry

=item C<account()>

Return the account information for the netrc entry

=item C<lpa()>

Return a list of login, password and account information for the netrc entry

=back

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

See L<https://rt.cpan.org/Dist/Display.html?Status=Active&Queue=libnet>.

=head1 SEE ALSO

L<Net::Cmd>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-1998 Graham Barr.  All rights reserved.

Copyright (C) 2013-2014, 2020 Steve Hay.  All rights reserved.

=head1 LICENCE

This module is free software; you can redistribute it and/or modify it under the
same terms as Perl itself, i.e. under the terms of either the GNU General Public
License or the Artistic License, as specified in the F<LICENCE> file.

=head1 VERSION

Version 3.15

=head1 DATE

20 March 2023

=head1 HISTORY

See the F<Changes> file.

=cut
