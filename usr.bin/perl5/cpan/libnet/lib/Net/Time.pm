# Net::Time.pm
#
# Copyright (C) 1995-2004 Graham Barr.  All rights reserved.
# Copyright (C) 2014, 2020 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package Net::Time;

use 5.008001;

use strict;
use warnings;

use Carp;
use Exporter;
use IO::Select;
use IO::Socket;
use Net::Config;

our @ISA       = qw(Exporter);
our @EXPORT_OK = qw(inet_time inet_daytime);

our $VERSION = "3.15";

our $TIMEOUT = 120;

sub _socket {
  my ($pname, $pnum, $host, $proto, $timeout) = @_;

  $proto ||= 'udp';

  my $port = (getservbyname($pname, $proto))[2] || $pnum;

  my $hosts = defined $host ? [$host] : $NetConfig{$pname . '_hosts'};

  my $me;

  foreach my $addr (@$hosts) {
    $me = IO::Socket::INET->new(
      PeerAddr => $addr,
      PeerPort => $port,
      Proto    => $proto
      )
      and last;
  }

  return unless $me;

  $me->send("\n")
    if $proto eq 'udp';

  $timeout = $TIMEOUT
    unless defined $timeout;

  IO::Select->new($me)->can_read($timeout)
    ? $me
    : undef;
}


sub inet_time {
  my $s      = _socket('time', 37, @_) || return;
  my $buf    = '';
  my $offset = 0 | 0;

  return
    unless defined $s->recv($buf, length(pack("N", 0)));

  # unpack, we | 0 to ensure we have an unsigned
  my $time = (unpack("N", $buf))[0] | 0;

  # the time protocol return time in seconds since 1900, convert
  # it to a the required format

  if ($^O eq "MacOS") {

    # MacOS return seconds since 1904, 1900 was not a leap year.
    $offset = (4 * 31536000) | 0;
  }
  else {

    # otherwise return seconds since 1972, there were 17 leap years between
    # 1900 and 1972
    $offset = (70 * 31536000 + 17 * 86400) | 0;
  }

  $time - $offset;
}


sub inet_daytime {
  my $s   = _socket('daytime', 13, @_) || return;
  my $buf = '';

  defined($s->recv($buf, 1024))
    ? $buf
    : undef;
}

1;

__END__

=head1 NAME

Net::Time - time and daytime network client interface

=head1 SYNOPSIS

    use Net::Time qw(inet_time inet_daytime);

    print inet_time();          # use default host from Net::Config
    print inet_time('localhost');
    print inet_time('localhost', 'tcp');

    print inet_daytime();       # use default host from Net::Config
    print inet_daytime('localhost');
    print inet_daytime('localhost', 'tcp');

=head1 DESCRIPTION

C<Net::Time> provides subroutines that obtain the time on a remote machine.

=head2 Functions

=over 4

=item C<inet_time([$host[, $protocol[, $timeout]]])>

Obtain the time on C<$host>, or some default host if C<$host> is not given
or not defined, using the protocol as defined in RFC868. The optional
argument C<$protocol> should define the protocol to use, either C<tcp> or
C<udp>. The result will be a time value in the same units as returned
by time() or I<undef> upon failure.

=item C<inet_daytime([$host[, $protocol[, $timeout]]])>

Obtain the time on C<$host>, or some default host if C<$host> is not given
or not defined, using the protocol as defined in RFC867. The optional
argument C<$protocol> should define the protocol to use, either C<tcp> or
C<udp>. The result will be an ASCII string or I<undef> upon failure.

=back

=head1 EXPORTS

The following symbols are, or can be, exported by this module:

=over 4

=item Default Exports

I<None>.

=item Optional Exports

C<inet_time>,
C<inet_daytime>.

=item Export Tags

I<None>.

=back

=head1 KNOWN BUGS

I<None>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-2004 Graham Barr.  All rights reserved.

Copyright (C) 2014, 2020 Steve Hay.  All rights reserved.

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
