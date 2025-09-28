# Net::Cmd.pm
#
# Copyright (C) 1995-2006 Graham Barr.  All rights reserved.
# Copyright (C) 2013-2016, 2020, 2022 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package Net::Cmd;

use 5.008001;

use strict;
use warnings;

use Carp;
use Exporter;
use Symbol 'gensym';
use Errno 'EINTR';

BEGIN {
  if (ord "A" == 193) {
    require Convert::EBCDIC;

    #    Convert::EBCDIC->import;
  }
}

our $VERSION = "3.15";
our @ISA     = qw(Exporter);
our @EXPORT  = qw(CMD_INFO CMD_OK CMD_MORE CMD_REJECT CMD_ERROR CMD_PENDING);

use constant CMD_INFO    => 1;
use constant CMD_OK      => 2;
use constant CMD_MORE    => 3;
use constant CMD_REJECT  => 4;
use constant CMD_ERROR   => 5;
use constant CMD_PENDING => 0;

use constant DEF_REPLY_CODE => 421;

my %debug = ();

my $tr = ord "A" == 193 ? Convert::EBCDIC->new() : undef;

sub toebcdic {
  my $cmd = shift;

  unless (exists ${*$cmd}{'net_cmd_asciipeer'}) {
    my $string    = $_[0];
    my $ebcdicstr = $tr->toebcdic($string);
    ${*$cmd}{'net_cmd_asciipeer'} = $string !~ /^\d+/ && $ebcdicstr =~ /^\d+/;
  }

  ${*$cmd}{'net_cmd_asciipeer'}
    ? $tr->toebcdic($_[0])
    : $_[0];
}


sub toascii {
  my $cmd = shift;
  ${*$cmd}{'net_cmd_asciipeer'}
    ? $tr->toascii($_[0])
    : $_[0];
}


sub _print_isa {
  no strict 'refs'; ## no critic (TestingAndDebugging::ProhibitNoStrict)

  my $pkg = shift;
  my $cmd = $pkg;

  $debug{$pkg} ||= 0;

  my %done = ();
  my @do   = ($pkg);
  my %spc  = ($pkg, "");

  while ($pkg = shift @do) {
    next if defined $done{$pkg};

    $done{$pkg} = 1;

    my $v =
      defined ${"${pkg}::VERSION"}
      ? "(" . ${"${pkg}::VERSION"} . ")"
      : "";

    my $spc = $spc{$pkg};
    $cmd->debug_print(1, "${spc}${pkg}${v}\n");

    if (@{"${pkg}::ISA"}) {
      @spc{@{"${pkg}::ISA"}} = ("  " . $spc{$pkg}) x @{"${pkg}::ISA"};
      unshift(@do, @{"${pkg}::ISA"});
    }
  }
}


sub debug {
  @_ == 1 or @_ == 2 or croak 'usage: $obj->debug([$level])';

  my ($cmd, $level) = @_;
  my $pkg    = ref($cmd) || $cmd;
  my $oldval = 0;

  if (ref($cmd)) {
    $oldval = ${*$cmd}{'net_cmd_debug'} || 0;
  }
  else {
    $oldval = $debug{$pkg} || 0;
  }

  return $oldval
    unless @_ == 2;

  $level = $debug{$pkg} || 0
    unless defined $level;

  _print_isa($pkg)
    if ($level && !exists $debug{$pkg});

  if (ref($cmd)) {
    ${*$cmd}{'net_cmd_debug'} = $level;
  }
  else {
    $debug{$pkg} = $level;
  }

  $oldval;
}


sub message {
  @_ == 1 or croak 'usage: $obj->message()';

  my $cmd = shift;

  wantarray
    ? @{${*$cmd}{'net_cmd_resp'}}
    : join("", @{${*$cmd}{'net_cmd_resp'}});
}


sub debug_text { $_[2] }


sub debug_print {
  my ($cmd, $out, $text) = @_;
  print STDERR $cmd, ($out ? '>>> ' : '<<< '), $cmd->debug_text($out, $text);
}


sub code {
  @_ == 1 or croak 'usage: $obj->code()';

  my $cmd = shift;

  ${*$cmd}{'net_cmd_code'} = $cmd->DEF_REPLY_CODE
    unless exists ${*$cmd}{'net_cmd_code'};

  ${*$cmd}{'net_cmd_code'};
}


sub status {
  @_ == 1 or croak 'usage: $obj->status()';

  my $cmd = shift;

  substr(${*$cmd}{'net_cmd_code'}, 0, 1);
}


sub set_status {
  @_ == 3 or croak 'usage: $obj->set_status($code, $resp)';

  my $cmd = shift;
  my ($code, $resp) = @_;

  $resp = defined $resp ? [$resp] : []
    unless ref($resp);

  (${*$cmd}{'net_cmd_code'}, ${*$cmd}{'net_cmd_resp'}) = ($code, $resp);

  1;
}

sub _syswrite_with_timeout {
  my $cmd = shift;
  my $line = shift;

  my $len    = length($line);
  my $offset = 0;
  my $win    = "";
  vec($win, fileno($cmd), 1) = 1;
  my $timeout = $cmd->timeout || undef;
  my $initial = time;
  my $pending = $timeout;

  local $SIG{PIPE} = 'IGNORE' unless $^O eq 'MacOS';

  while ($len) {
    my $wout;
    my $nfound = select(undef, $wout = $win, undef, $pending);
    if ((defined $nfound and $nfound > 0) or -f $cmd)    # -f for testing on win32
    {
      my $w = syswrite($cmd, $line, $len, $offset);
      if (! defined($w) ) {
        my $err = $!;
        $cmd->close;
        $cmd->_set_status_closed($err);
        return;
      }
      $len -= $w;
      $offset += $w;
    }
    elsif ($nfound == -1) {
      if ( $! == EINTR ) {
        if ( defined($timeout) ) {
          redo if ($pending = $timeout - ( time - $initial ) ) > 0;
          $cmd->_set_status_timeout;
          return;
        }
        redo;
      }
      my $err = $!;
      $cmd->close;
      $cmd->_set_status_closed($err);
      return;
    }
    else {
      $cmd->_set_status_timeout;
      return;
    }
  }

  return 1;
}

sub _set_status_timeout {
  my $cmd = shift;
  my $pkg = ref($cmd) || $cmd;

  $cmd->set_status($cmd->DEF_REPLY_CODE, "[$pkg] Timeout");
  carp(ref($cmd) . ": " . (caller(1))[3] . "(): timeout") if $cmd->debug;
}

sub _set_status_closed {
  my $cmd = shift;
  my $err = shift;
  my $pkg = ref($cmd) || $cmd;

  $cmd->set_status($cmd->DEF_REPLY_CODE, "[$pkg] Connection closed");
  carp(ref($cmd) . ": " . (caller(1))[3]
    . "(): unexpected EOF on command channel: $err") if $cmd->debug;
}

sub _is_closed {
  my $cmd = shift;
  if (!defined fileno($cmd)) {
     $cmd->_set_status_closed($!);
     return 1;
  }
  return 0;
}

sub command {
  my $cmd = shift;

  return $cmd
    if $cmd->_is_closed;

  $cmd->dataend()
    if (exists ${*$cmd}{'net_cmd_last_ch'});

  if (scalar(@_)) {
    my $str = join(
      " ",
      map {
        /\n/
          ? do { my $n = $_; $n =~ tr/\n/ /; $n }
          : $_;
        } @_
    );
    $str = $cmd->toascii($str) if $tr;
    $str .= "\015\012";

    $cmd->debug_print(1, $str)
      if ($cmd->debug);

    # though documented to return undef on failure, the legacy behavior
    # was to return $cmd even on failure, so this odd construct does that
    $cmd->_syswrite_with_timeout($str)
      or return $cmd;
  }

  $cmd;
}


sub ok {
  @_ == 1 or croak 'usage: $obj->ok()';

  my $code = $_[0]->code;
  0 < $code && $code < 400;
}


sub unsupported {
  my $cmd = shift;

  $cmd->set_status(580, 'Unsupported command');

  0;
}


sub getline {
  my $cmd = shift;

  ${*$cmd}{'net_cmd_lines'} ||= [];

  return shift @{${*$cmd}{'net_cmd_lines'}}
    if scalar(@{${*$cmd}{'net_cmd_lines'}});

  my $partial = defined(${*$cmd}{'net_cmd_partial'}) ? ${*$cmd}{'net_cmd_partial'} : "";

  return
    if $cmd->_is_closed;

  my $fd = fileno($cmd);
  my $rin = "";
  vec($rin, $fd, 1) = 1;

  my $buf;

  until (scalar(@{${*$cmd}{'net_cmd_lines'}})) {
    my $timeout = $cmd->timeout || undef;
    my $rout;

    my $select_ret = select($rout = $rin, undef, undef, $timeout);
    if ($select_ret > 0) {
      unless (sysread($cmd, $buf = "", 1024)) {
        my $err = $!;
        $cmd->close;
        $cmd->_set_status_closed($err);
        return;
      }

      substr($buf, 0, 0) = $partial;    ## prepend from last sysread

      my @buf = split(/\015?\012/, $buf, -1);    ## break into lines

      $partial = pop @buf;

      push(@{${*$cmd}{'net_cmd_lines'}}, map {"$_\n"} @buf);

    }
    else {
      $cmd->_set_status_timeout;
      return;
    }
  }

  ${*$cmd}{'net_cmd_partial'} = $partial;

  if ($tr) {
    foreach my $ln (@{${*$cmd}{'net_cmd_lines'}}) {
      $ln = $cmd->toebcdic($ln);
    }
  }

  shift @{${*$cmd}{'net_cmd_lines'}};
}


sub ungetline {
  my ($cmd, $str) = @_;

  ${*$cmd}{'net_cmd_lines'} ||= [];
  unshift(@{${*$cmd}{'net_cmd_lines'}}, $str);
}


sub parse_response {
  return ()
    unless $_[1] =~ s/^(\d\d\d)(.?)//o;
  ($1, $2 eq "-");
}


sub response {
  my $cmd = shift;
  my ($code, $more) = (undef) x 2;

  $cmd->set_status($cmd->DEF_REPLY_CODE, undef); # initialize the response

  while (1) {
    my $str = $cmd->getline();

    return CMD_ERROR
      unless defined($str);

    $cmd->debug_print(0, $str)
      if ($cmd->debug);

    ($code, $more) = $cmd->parse_response($str);
    unless (defined $code) {
      carp("$cmd: response(): parse error in '$str'") if ($cmd->debug);
      $cmd->ungetline($str);
      $@ = $str;   # $@ used as tunneling hack
      return CMD_ERROR;
    }

    ${*$cmd}{'net_cmd_code'} = $code;

    push(@{${*$cmd}{'net_cmd_resp'}}, $str);

    last unless ($more);
  }

  return unless defined $code;
  substr($code, 0, 1);
}


sub read_until_dot {
  my $cmd = shift;
  my $fh  = shift;
  my $arr = [];

  while (1) {
    my $str = $cmd->getline() or return;

    $cmd->debug_print(0, $str)
      if ($cmd->debug & 4);

    last if ($str =~ /^\.\r?\n/o);

    $str =~ s/^\.\././o;

    if (defined $fh) {
      print $fh $str;
    }
    else {
      push(@$arr, $str);
    }
  }

  $arr;
}


sub datasend {
  my $cmd  = shift;
  my $arr  = @_ == 1 && ref($_[0]) ? $_[0] : \@_;
  my $line = join("", @$arr);

  # Perls < 5.10.1 (with the exception of 5.8.9) have a performance problem with
  # the substitutions below when dealing with strings stored internally in
  # UTF-8, so downgrade them (if possible).
  # Data passed to datasend() should be encoded to octets upstream already so
  # shouldn't even have the UTF-8 flag on to start with, but if it so happens
  # that the octets are stored in an upgraded string (as can sometimes occur)
  # then they would still downgrade without fail anyway.
  # Only Unicode codepoints > 0xFF stored in an upgraded string will fail to
  # downgrade. We fail silently in that case, and a "Wide character in print"
  # warning will be emitted later by syswrite().
  utf8::downgrade($line, 1) if $] < 5.010001 && $] != 5.008009;

  return 0
    if $cmd->_is_closed;

  my $last_ch = ${*$cmd}{'net_cmd_last_ch'};

  # We have not send anything yet, so last_ch = "\012" means we are at the start of a line
  $last_ch = ${*$cmd}{'net_cmd_last_ch'} = "\012" unless defined $last_ch;

  return 1 unless length $line;

  if ($cmd->debug) {
    foreach my $b (split(/\n/, $line)) {
      $cmd->debug_print(1, "$b\n");
    }
  }

  $line =~ tr/\r\n/\015\012/ unless "\r" eq "\015";

  my $first_ch = '';

  if ($last_ch eq "\015") {
    # Remove \012 so it does not get prefixed with another \015 below
    # and escape the . if there is one following it because the fixup
    # below will not find it
    $first_ch = "\012" if $line =~ s/^\012(\.?)/$1$1/;
  }
  elsif ($last_ch eq "\012") {
    # Fixup below will not find the . as the first character of the buffer
    $first_ch = "." if $line =~ /^\./;
  }

  $line =~ s/\015?\012(\.?)/\015\012$1$1/sg;

  substr($line, 0, 0) = $first_ch;

  ${*$cmd}{'net_cmd_last_ch'} = substr($line, -1, 1);

  $cmd->_syswrite_with_timeout($line)
    or return;

  1;
}


sub rawdatasend {
  my $cmd  = shift;
  my $arr  = @_ == 1 && ref($_[0]) ? $_[0] : \@_;
  my $line = join("", @$arr);

  return 0
    if $cmd->_is_closed;

  return 1
    unless length($line);

  if ($cmd->debug) {
    my $b = "$cmd>>> ";
    print STDERR $b, join("\n$b", split(/\n/, $line)), "\n";
  }

  $cmd->_syswrite_with_timeout($line)
    or return;

  1;
}


sub dataend {
  my $cmd = shift;

  return 0
    if $cmd->_is_closed;

  my $ch = ${*$cmd}{'net_cmd_last_ch'};
  my $tosend;

  if (!defined $ch) {
    return 1;
  }
  elsif ($ch ne "\012") {
    $tosend = "\015\012";
  }

  $tosend .= ".\015\012";

  $cmd->debug_print(1, ".\n")
    if ($cmd->debug);

  $cmd->_syswrite_with_timeout($tosend)
    or return 0;

  delete ${*$cmd}{'net_cmd_last_ch'};

  $cmd->response() == CMD_OK;
}

# read and write to tied filehandle
sub tied_fh {
  my $cmd = shift;
  ${*$cmd}{'net_cmd_readbuf'} = '';
  my $fh = gensym();
  tie *$fh, ref($cmd), $cmd;
  return $fh;
}

# tie to myself
sub TIEHANDLE {
  my $class = shift;
  my $cmd   = shift;
  return $cmd;
}

# Tied filehandle read.  Reads requested data length, returning
# end-of-file when the dot is encountered.
sub READ {
  my $cmd = shift;
  my ($len, $offset) = @_[1, 2];
  return unless exists ${*$cmd}{'net_cmd_readbuf'};
  my $done = 0;
  while (!$done and length(${*$cmd}{'net_cmd_readbuf'}) < $len) {
    ${*$cmd}{'net_cmd_readbuf'} .= $cmd->getline() or return;
    $done++ if ${*$cmd}{'net_cmd_readbuf'} =~ s/^\.\r?\n\Z//m;
  }

  $_[0] = '';
  substr($_[0], $offset + 0) = substr(${*$cmd}{'net_cmd_readbuf'}, 0, $len);
  substr(${*$cmd}{'net_cmd_readbuf'}, 0, $len) = '';
  delete ${*$cmd}{'net_cmd_readbuf'} if $done;

  return length $_[0];
}


sub READLINE {
  my $cmd = shift;

  # in this context, we use the presence of readbuf to
  # indicate that we have not yet reached the eof
  return unless exists ${*$cmd}{'net_cmd_readbuf'};
  my $line = $cmd->getline;
  return if $line =~ /^\.\r?\n/;
  $line;
}


sub PRINT {
  my $cmd = shift;
  my ($buf, $len, $offset) = @_;
  $len ||= length($buf);
  $offset += 0;
  return unless $cmd->datasend(substr($buf, $offset, $len));
  ${*$cmd}{'net_cmd_sending'}++;    # flag that we should call dataend()
  return $len;
}


sub CLOSE {
  my $cmd = shift;
  my $r = exists(${*$cmd}{'net_cmd_sending'}) ? $cmd->dataend : 1;
  delete ${*$cmd}{'net_cmd_readbuf'};
  delete ${*$cmd}{'net_cmd_sending'};
  $r;
}

1;

__END__


=head1 NAME

Net::Cmd - Network Command class (as used by FTP, SMTP etc)

=head1 SYNOPSIS

    use Net::Cmd;

    @ISA = qw(Net::Cmd);

=head1 DESCRIPTION

C<Net::Cmd> is a collection of methods that can be inherited by a sub-class
of C<IO::Socket::INET>. These methods implement the functionality required for a
command based protocol, for example FTP and SMTP.

If your sub-class does not also derive from C<IO::Socket::INET> or similar (e.g.
C<IO::Socket::IP>, C<IO::Socket::INET6> or C<IO::Socket::SSL>) then you must
provide the following methods by other means yourself: C<close()> and
C<timeout()>.

=head2 Public Methods

These methods provide a user interface to the C<Net::Cmd> object.

=over 4

=item C<debug($level)>

Set the level of debug information for this object. If C<$level> is not given
then the current state is returned. Otherwise the state is changed to 
C<$level> and the previous state returned. 

Different packages
may implement different levels of debug but a non-zero value results in 
copies of all commands and responses also being sent to STDERR.

If C<$level> is C<undef> then the debug level will be set to the default
debug level for the class.

This method can also be called as a I<static> method to set/get the default
debug level for a given class.

=item C<message()>

Returns the text message returned from the last command. In a scalar
context it returns a single string, in a list context it will return
each line as a separate element. (See L<PSEUDO RESPONSES> below.)

=item C<code()>

Returns the 3-digit code from the last command. If a command is pending
then the value 0 is returned. (See L<PSEUDO RESPONSES> below.)

=item C<ok()>

Returns non-zero if the last code value was greater than zero and
less than 400. This holds true for most command servers. Servers
where this does not hold may override this method.

=item C<status()>

Returns the most significant digit of the current status code. If a command
is pending then C<CMD_PENDING> is returned.

=item C<datasend($data)>

Send data to the remote server, converting LF to CRLF. Any line starting
with a '.' will be prefixed with another '.'.
C<$data> may be an array or a reference to an array.
The C<$data> passed in must be encoded by the caller to octets of whatever
encoding is required, e.g. by using the Encode module's C<encode()> function.

=item C<dataend()>

End the sending of data to the remote server. This is done by ensuring that
the data already sent ends with CRLF then sending '.CRLF' to end the
transmission. Once this data has been sent C<dataend> calls C<response> and
returns true if C<response> returns CMD_OK.

=back

=head2 Protected Methods

These methods are not intended to be called by the user, but used or 
over-ridden by a sub-class of C<Net::Cmd>

=over 4

=item C<debug_print($dir, $text)>

Print debugging information. C<$dir> denotes the direction I<true> being
data being sent to the server. Calls C<debug_text> before printing to
STDERR.

=item C<debug_text($dir, $text)>

This method is called to print debugging information. C<$text> is
the text being sent. The method should return the text to be printed.

This is primarily meant for the use of modules such as FTP where passwords
are sent, but we do not want to display them in the debugging information.

=item C<command($cmd[, $args, ... ])>

Send a command to the command server. All arguments are first joined with
a space character and CRLF is appended, this string is then sent to the
command server.

Returns undef upon failure.

=item C<unsupported()>

Sets the status code to 580 and the response text to 'Unsupported command'.
Returns zero.

=item C<response()>

Obtain a response from the server. Upon success the most significant digit
of the status code is returned. Upon failure, timeout etc., I<CMD_ERROR> is
returned.

=item C<parse_response($text)>

This method is called by C<response> as a method with one argument. It should
return an array of 2 values, the 3-digit status code and a flag which is true
when this is part of a multi-line response and this line is not the last.

=item C<getline()>

Retrieve one line, delimited by CRLF, from the remote server. Returns I<undef>
upon failure.

B<NOTE>: If you do use this method for any reason, please remember to add
some C<debug_print> calls into your method.

=item C<ungetline($text)>

Unget a line of text from the server.

=item C<rawdatasend($data)>

Send data to the remote server without performing any conversions. C<$data>
is a scalar.
As with C<datasend()>, the C<$data> passed in must be encoded by the caller
to octets of whatever encoding is required, e.g. by using the Encode module's
C<encode()> function.

=item C<read_until_dot()>

Read data from the remote server until a line consisting of a single '.'.
Any lines starting with '..' will have one of the '.'s removed.

Returns a reference to a list containing the lines, or I<undef> upon failure.

=item C<tied_fh()>

Returns a filehandle tied to the Net::Cmd object.  After issuing a
command, you may read from this filehandle using read() or <>.  The
filehandle will return EOF when the final dot is encountered.
Similarly, you may write to the filehandle in order to send data to
the server after issuing a command that expects data to be written.

See the Net::POP3 and Net::SMTP modules for examples of this.

=back

=head2 Pseudo Responses

Normally the values returned by C<message()> and C<code()> are
obtained from the remote server, but in a few circumstances, as
detailed below, C<Net::Cmd> will return values that it sets. You
can alter this behavior by overriding DEF_REPLY_CODE() to specify
a different default reply code, or overriding one of the specific
error handling methods below.

=over 4

=item Initial value

Before any command has executed or if an unexpected error occurs
C<code()> will return "421" (temporary connection failure) and
C<message()> will return undef.

=item Connection closed

If the underlying C<IO::Handle> is closed, or if there are
any read or write failures, the file handle will be forced closed,
and C<code()> will return "421" (temporary connection failure)
and C<message()> will return "[$pkg] Connection closed"
(where $pkg is the name of the class that subclassed C<Net::Cmd>).
The _set_status_closed() method can be overridden to set a different
message (by calling set_status()) or otherwise trap this error.

=item Timeout

If there is a read or write timeout C<code()> will return "421"
(temporary connection failure) and C<message()> will return
"[$pkg] Timeout" (where $pkg is the name of the class
that subclassed C<Net::Cmd>). The _set_status_timeout() method
can be overridden to set a different message (by calling set_status())
or otherwise trap this error.

=back

=head1 EXPORTS

The following symbols are, or can be, exported by this module:

=over 4

=item Default Exports

C<CMD_INFO>,
C<CMD_OK>,
C<CMD_MORE>,
C<CMD_REJECT>,
C<CMD_ERROR>,
C<CMD_PENDING>.

(These correspond to possible results of C<response()> and C<status()>.)

=item Optional Exports

I<None>.

=item Export Tags

I<None>.

=back

=head1 KNOWN BUGS

See L<https://rt.cpan.org/Dist/Display.html?Status=Active&Queue=libnet>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-2006 Graham Barr.  All rights reserved.

Copyright (C) 2013-2016, 2020, 2022 Steve Hay.  All rights reserved.

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
