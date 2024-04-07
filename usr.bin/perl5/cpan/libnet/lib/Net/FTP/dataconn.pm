##
## Generic data connection package
##

package Net::FTP::dataconn;

use 5.008001;

use strict;
use warnings;

use Carp;
use Errno;
use Net::Cmd;

our $VERSION = '3.15';

$Net::FTP::IOCLASS or die "please load Net::FTP before Net::FTP::dataconn";
our @ISA = $Net::FTP::IOCLASS;

sub reading {
  my $data = shift;
  ${*$data}{'net_ftp_bytesread'} = 0;
}


sub abort {
  my $data = shift;
  my $ftp  = ${*$data}{'net_ftp_cmd'};

  # no need to abort if we have finished the xfer
  return $data->close
    if ${*$data}{'net_ftp_eof'};

  # for some reason if we continuously open RETR connections and not
  # read a single byte, then abort them after a while the server will
  # close our connection, this prevents the unexpected EOF on the
  # command channel -- GMB
  if (exists ${*$data}{'net_ftp_bytesread'}
    && (${*$data}{'net_ftp_bytesread'} == 0))
  {
    my $buf     = "";
    my $timeout = $data->timeout;
    $data->can_read($timeout) && sysread($data, $buf, 1);
  }

  ${*$data}{'net_ftp_eof'} = 1;    # fake

  $ftp->abort;                     # this will close me
}


sub _close {
  my $data = shift;
  my $ftp  = ${*$data}{'net_ftp_cmd'};

  $data->SUPER::close();

  delete ${*$ftp}{'net_ftp_dataconn'}
    if defined $ftp
    && exists ${*$ftp}{'net_ftp_dataconn'}
    && $data == ${*$ftp}{'net_ftp_dataconn'};
}


sub close {
  my $data = shift;
  my $ftp  = ${*$data}{'net_ftp_cmd'};

  if (exists ${*$data}{'net_ftp_bytesread'} && !${*$data}{'net_ftp_eof'}) {
    my $junk;
    eval { local($SIG{__DIE__}); $data->read($junk, 1, 0) };
    return $data->abort unless ${*$data}{'net_ftp_eof'};
  }

  $data->_close;

  return unless defined $ftp;

  $ftp->response() == CMD_OK
    && $ftp->message =~ /unique file name:\s*(\S*)\s*\)/
    && (${*$ftp}{'net_ftp_unique'} = $1);

  $ftp->status == CMD_OK;
}


sub _select {
  my ($data, $timeout, $do_read) = @_;
  my ($rin, $rout, $win, $wout, $tout, $nfound);

  vec($rin = '', fileno($data), 1) = 1;

  ($win, $rin) = ($rin, $win) unless $do_read;

  while (1) {
    $nfound = select($rout = $rin, $wout = $win, undef, $tout = $timeout);

    last if $nfound >= 0;

    croak "select: $!"
      unless $!{EINTR};
  }

  $nfound;
}


sub can_read {
  _select(@_[0, 1], 1);
}


sub can_write {
  _select(@_[0, 1], 0);
}


sub cmd {
  my $ftp = shift;

  ${*$ftp}{'net_ftp_cmd'};
}


sub bytes_read {
  my $ftp = shift;

  ${*$ftp}{'net_ftp_bytesread'} || 0;
}

1;

__END__

=head1 NAME

Net::FTP::dataconn - FTP Client data connection class

=head1 SYNOPSIS

    # Perform IO operations on an FTP client data connection object:

    $num_bytes_read = $obj->read($buffer, $size);
    $num_bytes_read = $obj->read($buffer, $size, $timeout);

    $num_bytes_written = $obj->write($buffer, $size);
    $num_bytes_written = $obj->write($buffer, $size, $timeout);

    $num_bytes_read_so_far = $obj->bytes_read();

    $obj->abort();

    $closed_successfully = $obj->close();

=head1 DESCRIPTION

Some of the methods defined in C<Net::FTP> return an object which will
be derived from this class. The dataconn class itself is derived from
the C<IO::Socket::INET> class, so any normal IO operations can be performed.
However the following methods are defined in the dataconn class and IO should
be performed using these.

=over 4

=item C<read($buffer, $size[, $timeout])>

Read C<$size> bytes of data from the server and place it into C<$buffer>, also
performing any <CRLF> translation necessary. C<$timeout> is optional, if not
given, the timeout value from the command connection will be used.

Returns the number of bytes read before any <CRLF> translation.

=item C<write($buffer, $size[, $timeout])>

Write C<$size> bytes of data from C<$buffer> to the server, also
performing any <CRLF> translation necessary. C<$timeout> is optional, if not
given, the timeout value from the command connection will be used.

Returns the number of bytes written before any <CRLF> translation.

=item C<bytes_read()>

Returns the number of bytes read so far.

=item C<abort()>

Abort the current data transfer.

=item C<close()>

Close the data connection and get a response from the FTP server. Returns
I<true> if the connection was closed successfully and the first digit of
the response from the server was a '2'.

=back

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

I<None>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1997-2010 Graham Barr.  All rights reserved.

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
