# Net::POP3.pm
#
# Copyright (C) 1995-2004 Graham Barr.  All rights reserved.
# Copyright (C) 2013-2016, 2020 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package Net::POP3;

use 5.008001;

use strict;
use warnings;

use Carp;
use IO::Socket;
use Net::Cmd;
use Net::Config;

our $VERSION = "3.15";

# Code for detecting if we can use SSL
my $ssl_class = eval {
  require IO::Socket::SSL;
  # first version with default CA on most platforms
  no warnings 'numeric';
  IO::Socket::SSL->VERSION(2.007);
} && 'IO::Socket::SSL';

my $nossl_warn = !$ssl_class &&
  'To use SSL please install IO::Socket::SSL with version>=2.007';

# Code for detecting if we can use IPv6
my $family_key = 'Domain';
my $inet6_class = eval {
  require IO::Socket::IP;
  no warnings 'numeric';
  IO::Socket::IP->VERSION(0.25) || die;
  $family_key = 'Family';
} && 'IO::Socket::IP' || eval {
  require IO::Socket::INET6;
  no warnings 'numeric';
  IO::Socket::INET6->VERSION(2.62);
} && 'IO::Socket::INET6';


sub can_ssl   { $ssl_class };
sub can_inet6 { $inet6_class };

our @ISA = ('Net::Cmd', $inet6_class || 'IO::Socket::INET');

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  my ($host, %arg);
  if (@_ % 2) {
    $host = shift;
    %arg  = @_;
  }
  else {
    %arg  = @_;
    $host = delete $arg{Host};
  }
  my $hosts = defined $host ? [$host] : $NetConfig{pop3_hosts};
  my $obj;

  if ($arg{SSL}) {
    # SSL from start
    die $nossl_warn if !$ssl_class;
    $arg{Port} ||= 995;
  }

  $arg{Timeout} = 120 if ! defined $arg{Timeout};

  foreach my $h (@{$hosts}) {
    $obj = $type->SUPER::new(
      PeerAddr => ($host = $h),
      PeerPort => $arg{Port} || 'pop3(110)',
      Proto => 'tcp',
      $family_key => $arg{Domain} || $arg{Family},
      LocalAddr => $arg{LocalAddr},
      LocalPort => exists($arg{ResvPort}) ? $arg{ResvPort} : $arg{LocalPort},
      Timeout => $arg{Timeout},
      )
      and last;
  }

  return
    unless defined $obj;

  ${*$obj}{'net_pop3_arg'} = \%arg;
  ${*$obj}{'net_pop3_host'} = $host;
  if ($arg{SSL}) {
    Net::POP3::_SSL->start_SSL($obj,%arg) or return;
  }

  $obj->autoflush(1);
  $obj->debug(exists $arg{Debug} ? $arg{Debug} : undef);

  unless ($obj->response() == CMD_OK) {
    $obj->close();
    return;
  }

  ${*$obj}{'net_pop3_banner'} = $obj->message;

  $obj;
}


sub host {
  my $me = shift;
  ${*$me}{'net_pop3_host'};
}

##
## We don't want people sending me their passwords when they report problems
## now do we :-)
##


sub debug_text { $_[2] =~ /^(pass|rpop)/i ? "$1 ....\n" : $_[2]; }


sub login {
  @_ >= 1 && @_ <= 3 or croak 'usage: $pop3->login([$user[, $pass]])';
  my ($me, $user, $pass) = @_;

  if (@_ <= 2) {
    ($user, $pass) = $me->_lookup_credentials($user);
  }

  $me->user($user)
    and $me->pass($pass);
}

sub starttls {
  my $self = shift;
  $ssl_class or die $nossl_warn;
  $self->_STLS or return;
  Net::POP3::_SSL->start_SSL($self,
    %{ ${*$self}{'net_pop3_arg'} }, # (ssl) args given in new
    @_   # more (ssl) args
  ) or return;
  return 1;
}

sub apop {
  @_ >= 1 && @_ <= 3 or croak 'usage: $pop3->apop([$user[, $pass]])';
  my ($me, $user, $pass) = @_;
  my $banner;
  my $md;

  if (eval { local $SIG{__DIE__}; require Digest::MD5 }) {
    $md = Digest::MD5->new();
  }
  elsif (eval { local $SIG{__DIE__}; require MD5 }) {
    $md = MD5->new();
  }
  else {
    carp "You need to install Digest::MD5 or MD5 to use the APOP command";
    return;
  }

  return
    unless ($banner = (${*$me}{'net_pop3_banner'} =~ /(<.*>)/)[0]);

  if (@_ <= 2) {
    ($user, $pass) = $me->_lookup_credentials($user);
  }

  $md->add($banner, $pass);

  return
    unless ($me->_APOP($user, $md->hexdigest));

  $me->_get_mailbox_count();
}


sub user {
  @_ == 2 or croak 'usage: $pop3->user($user)';
  $_[0]->_USER($_[1]) ? 1 : undef;
}


sub pass {
  @_ == 2 or croak 'usage: $pop3->pass($pass)';

  my ($me, $pass) = @_;

  return
    unless ($me->_PASS($pass));

  $me->_get_mailbox_count();
}


sub reset {
  @_ == 1 or croak 'usage: $obj->reset()';

  my $me = shift;

  return 0
    unless ($me->_RSET);

  if (defined ${*$me}{'net_pop3_mail'}) {
    local $_;
    foreach (@{${*$me}{'net_pop3_mail'}}) {
      delete $_->{'net_pop3_deleted'};
    }
  }
}


sub last {
  @_ == 1 or croak 'usage: $obj->last()';

  return
    unless $_[0]->_LAST && $_[0]->message =~ /(\d+)/;

  return $1;
}


sub top {
  @_ == 2 || @_ == 3 or croak 'usage: $pop3->top($msgnum[, $numlines])';
  my $me = shift;

  return
    unless $me->_TOP($_[0], $_[1] || 0);

  $me->read_until_dot;
}


sub popstat {
  @_ == 1 or croak 'usage: $pop3->popstat()';
  my $me = shift;

  return ()
    unless $me->_STAT && $me->message =~ /(\d+)\D+(\d+)/;

  ($1 || 0, $2 || 0);
}


sub list {
  @_ == 1 || @_ == 2 or croak 'usage: $pop3->list([$msgnum])';
  my $me = shift;

  return
    unless $me->_LIST(@_);

  if (@_) {
    $me->message =~ /\d+\D+(\d+)/;
    return $1 || undef;
  }

  my $info = $me->read_until_dot
    or return;

  my %hash = map { (/(\d+)\D+(\d+)/) } @$info;

  return \%hash;
}


sub get {
  @_ == 2 or @_ == 3 or croak 'usage: $pop3->get($msgnum[, $fh])';
  my $me = shift;

  return
    unless $me->_RETR(shift);

  $me->read_until_dot(@_);
}


sub getfh {
  @_ == 2 or croak 'usage: $pop3->getfh($msgnum)';
  my $me = shift;

  return unless $me->_RETR(shift);
  return $me->tied_fh;
}


sub delete {
  @_ == 2 or croak 'usage: $pop3->delete($msgnum)';
  my $me = shift;
  return 0 unless $me->_DELE(@_);
  ${*$me}{'net_pop3_deleted'} = 1;
}


sub uidl {
  @_ == 1 || @_ == 2 or croak 'usage: $pop3->uidl([$msgnum])';
  my $me = shift;
  my $uidl;

  $me->_UIDL(@_)
    or return;
  if (@_) {
    $uidl = ($me->message =~ /\d+\s+([\041-\176]+)/)[0];
  }
  else {
    my $ref = $me->read_until_dot
      or return;
    $uidl = {};
    foreach my $ln (@$ref) {
      my ($msg, $uid) = $ln =~ /^\s*(\d+)\s+([\041-\176]+)/;
      $uidl->{$msg} = $uid;
    }
  }
  return $uidl;
}


sub ping {
  @_ == 2 or croak 'usage: $pop3->ping($user)';
  my $me = shift;

  return () unless $me->_PING(@_) && $me->message =~ /(\d+)\D+(\d+)/;

  ($1 || 0, $2 || 0);
}


sub _lookup_credentials {
  my ($me, $user) = @_;

  require Net::Netrc;

       $user ||= eval { local $SIG{__DIE__}; (getpwuid($>))[0] }
    || $ENV{NAME}
    || $ENV{USER}
    || $ENV{LOGNAME};

  my $m = Net::Netrc->lookup(${*$me}{'net_pop3_host'}, $user);
  $m ||= Net::Netrc->lookup(${*$me}{'net_pop3_host'});

  my $pass = $m
    ? $m->password || ""
    : "";

  ($user, $pass);
}


sub _get_mailbox_count {
  my ($me) = @_;
  my $ret = ${*$me}{'net_pop3_count'} =
    ($me->message =~ /(\d+)\s+message/io) ? $1 : ($me->popstat)[0];

  $ret ? $ret : "0E0";
}


sub _STAT { shift->command('STAT'       )->response() == CMD_OK }
sub _LIST { shift->command('LIST',    @_)->response() == CMD_OK }
sub _RETR { shift->command('RETR', $_[0])->response() == CMD_OK }
sub _DELE { shift->command('DELE', $_[0])->response() == CMD_OK }
sub _NOOP { shift->command('NOOP'       )->response() == CMD_OK }
sub _RSET { shift->command('RSET'       )->response() == CMD_OK }
sub _QUIT { shift->command('QUIT'       )->response() == CMD_OK }
sub _TOP  { shift->command( 'TOP',    @_)->response() == CMD_OK }
sub _UIDL { shift->command('UIDL',    @_)->response() == CMD_OK }
sub _USER { shift->command('USER', $_[0])->response() == CMD_OK }
sub _PASS { shift->command('PASS', $_[0])->response() == CMD_OK }
sub _APOP { shift->command('APOP',    @_)->response() == CMD_OK }
sub _PING { shift->command('PING', $_[0])->response() == CMD_OK }
sub _RPOP { shift->command('RPOP', $_[0])->response() == CMD_OK }
sub _LAST { shift->command('LAST'       )->response() == CMD_OK }
sub _CAPA { shift->command('CAPA'       )->response() == CMD_OK }
sub _STLS { shift->command("STLS",     )->response() == CMD_OK }


sub quit {
  my $me = shift;

  $me->_QUIT;
  $me->close;
}


sub DESTROY {
  my $me = shift;

  if (defined fileno($me) and ${*$me}{'net_pop3_deleted'}) {
    $me->reset;
    $me->quit;
  }
}

##
## POP3 has weird responses, so we emulate them to look the same :-)
##


sub response {
  my $cmd  = shift;
  my $str  = $cmd->getline() or return;
  my $code = "500";

  $cmd->debug_print(0, $str)
    if ($cmd->debug);

  if ($str =~ s/^\+OK\s*//io) {
    $code = "200";
  }
  elsif ($str =~ s/^\+\s*//io) {
    $code = "300";
  }
  else {
    $str =~ s/^-ERR\s*//io;
  }

  ${*$cmd}{'net_cmd_resp'} = [$str];
  ${*$cmd}{'net_cmd_code'} = $code;

  substr($code, 0, 1);
}


sub capa {
  my $this = shift;
  my ($capa, %capabilities);

  # Fake a capability here
  $capabilities{APOP} = '' if ($this->banner() =~ /<.*>/);

  if ($this->_CAPA()) {
    $capabilities{CAPA} = 1;
    $capa = $this->read_until_dot();
    %capabilities = (%capabilities, map {/^\s*(\S+)\s*(.*)/} @$capa);
  }
  else {

    # Check AUTH for SASL capabilities
    if ($this->command('AUTH')->response() == CMD_OK) {
      my $mechanism = $this->read_until_dot();
      $capabilities{SASL} = join " ", map {m/([A-Z0-9_-]+)/} @{$mechanism};
    }
  }

  return ${*$this}{'net_pop3e_capabilities'} = \%capabilities;
}


sub capabilities {
  my $this = shift;

  ${*$this}{'net_pop3e_capabilities'} || $this->capa;
}


sub auth {
  my ($self, $username, $password) = @_;

  eval {
    require MIME::Base64;
    require Authen::SASL;
  } or $self->set_status(500, ["Need MIME::Base64 and Authen::SASL todo auth"]), return 0;

  my $capa       = $self->capa;
  my $mechanisms = $capa->{SASL} || 'CRAM-MD5';

  my $sasl;

  if (ref($username) and UNIVERSAL::isa($username, 'Authen::SASL')) {
    $sasl = $username;
    my $user_mech = $sasl->mechanism || '';
    my @user_mech = split(/\s+/, $user_mech);
    my %user_mech;
    @user_mech{@user_mech} = ();

    my @server_mech = split(/\s+/, $mechanisms);
    my @mech = @user_mech
      ? grep { exists $user_mech{$_} } @server_mech
      : @server_mech;
    unless (@mech) {
      $self->set_status(
        500,
        [ 'Client SASL mechanisms (',
          join(', ', @user_mech),
          ') do not match the SASL mechnism the server announces (',
          join(', ', @server_mech), ')',
        ]
      );
      return 0;
    }

    $sasl->mechanism(join(" ", @mech));
  }
  else {
    die "auth(username, password)" if not length $username;
    $sasl = Authen::SASL->new(
      mechanism => $mechanisms,
      callback  => {
        user     => $username,
        pass     => $password,
        authname => $username,
      }
    );
  }

  # We should probably allow the user to pass the host, but I don't
  # currently know and SASL mechanisms that are used by smtp that need it
  my ($hostname) = split /:/, ${*$self}{'net_pop3_host'};
  my $client = eval { $sasl->client_new('pop', $hostname, 0) };

  unless ($client) {
    my $mech = $sasl->mechanism;
    $self->set_status(
      500,
      [ " Authen::SASL failure: $@",
        '(please check if your local Authen::SASL installation',
        "supports mechanism '$mech'"
      ]
    );
    return 0;
  }

  my ($token) = $client->client_start
    or do {
    my $mech = $client->mechanism;
    $self->set_status(
      500,
      [ ' Authen::SASL failure:  $client->client_start ',
        "mechanism '$mech' hostname #$hostname#",
        $client->error
      ]
    );
    return 0;
    };

  # We don't support sasl mechanisms that encrypt the socket traffic.
  # todo that we would really need to change the ISA hierarchy
  # so we don't inherit from IO::Socket, but instead hold it in an attribute

  my @cmd = ("AUTH", $client->mechanism);
  my $code;

  push @cmd, MIME::Base64::encode_base64($token, '')
    if defined $token and length $token;

  while (($code = $self->command(@cmd)->response()) == CMD_MORE) {

    my ($token) = $client->client_step(MIME::Base64::decode_base64(($self->message)[0])) or do {
      $self->set_status(
        500,
        [ ' Authen::SASL failure:  $client->client_step ',
          "mechanism '", $client->mechanism, " hostname #$hostname#, ",
          $client->error
        ]
      );
      return 0;
    };

    @cmd = (MIME::Base64::encode_base64(defined $token ? $token : '', ''));
  }

  $code == CMD_OK;
}


sub banner {
  my $this = shift;

  return ${*$this}{'net_pop3_banner'};
}

{
  package Net::POP3::_SSL;
  our @ISA = ( $ssl_class ? ($ssl_class):(), 'Net::POP3' );
  sub starttls { die "POP3 connection is already in SSL mode" }
  sub start_SSL {
    my ($class,$pop3,%arg) = @_;
    delete @arg{ grep { !m{^SSL_} } keys %arg };
    ( $arg{SSL_verifycn_name} ||= $pop3->host )
        =~s{(?<!:):[\w()]+$}{}; # strip port
    $arg{SSL_hostname} = $arg{SSL_verifycn_name}
        if ! defined $arg{SSL_hostname} && $class->can_client_sni;
    $arg{SSL_verifycn_scheme} ||= 'pop3';
    my $ok = $class->SUPER::start_SSL($pop3,%arg);
    $@ = $ssl_class->errstr if !$ok;
    return $ok;
  }
}



1;

__END__

=head1 NAME

Net::POP3 - Post Office Protocol 3 Client class (RFC1939)

=head1 SYNOPSIS

    use Net::POP3;

    # Constructors
    $pop = Net::POP3->new('pop3host');
    $pop = Net::POP3->new('pop3host', Timeout => 60);
    $pop = Net::POP3->new('pop3host', SSL => 1, Timeout => 60);

    if ($pop->login($username, $password) > 0) {
      my $msgnums = $pop->list; # hashref of msgnum => size
      foreach my $msgnum (keys %$msgnums) {
        my $msg = $pop->get($msgnum);
        print @$msg;
        $pop->delete($msgnum);
      }
    }

    $pop->quit;

=head1 DESCRIPTION

This module implements a client interface to the POP3 protocol, enabling
a perl5 application to talk to POP3 servers. This documentation assumes
that you are familiar with the POP3 protocol described in RFC1939.
With L<IO::Socket::SSL> installed it also provides support for implicit and
explicit TLS encryption, i.e. POP3S or POP3+STARTTLS.

A new Net::POP3 object must be created with the I<new> method. Once
this has been done, all POP3 commands are accessed via method calls
on the object.

The Net::POP3 class is a subclass of Net::Cmd and (depending on avaibility) of
IO::Socket::IP, IO::Socket::INET6 or IO::Socket::INET.

=head2 Class Methods

=over 4

=item C<new([$host][, %options])>

This is the constructor for a new Net::POP3 object. C<$host> is the
name of the remote host to which an POP3 connection is required.

C<$host> is optional. If C<$host> is not given then it may instead be
passed as the C<Host> option described below. If neither is given then
the C<POP3_Hosts> specified in C<Net::Config> will be used.

C<%options> are passed in a hash like fashion, using key and value pairs.
Possible options are:

B<Host> - POP3 host to connect to. It may be a single scalar, as defined for
the C<PeerAddr> option in L<IO::Socket::INET>, or a reference to
an array with hosts to try in turn. The L</host> method will return the value
which was used to connect to the host.

B<Port> - port to connect to.
Default - 110 for plain POP3 and 995 for POP3s (direct SSL).

B<SSL> - If the connection should be done from start with SSL, contrary to later
upgrade with C<starttls>.
You can use SSL arguments as documented in L<IO::Socket::SSL>, but it will
usually use the right arguments already.

B<LocalAddr> and B<LocalPort> - These parameters are passed directly
to IO::Socket to allow binding the socket to a specific local address and port.
For compatibility with older versions B<ResvPort> can be used instead of
B<LocalPort>.

B<Domain> - This parameter is passed directly to IO::Socket and makes it
possible to enforce IPv4 connections even if L<IO::Socket::IP> is used as super
class. Alternatively B<Family> can be used.

B<Timeout> - Maximum time, in seconds, to wait for a response from the
POP3 server (default: 120)

B<Debug> - Enable debugging information

=back

=head2 Object Methods

Unless otherwise stated all methods return either a I<true> or I<false>
value, with I<true> meaning that the operation was a success. When a method
states that it returns a value, failure will be returned as I<undef> or an
empty list.

C<Net::POP3> inherits from C<Net::Cmd> so methods defined in C<Net::Cmd> may
be used to send commands to the remote POP3 server in addition to the methods
documented here.

=over 4

=item C<host()>

Returns the value used by the constructor, and passed to IO::Socket::INET,
to connect to the host.

=item C<auth($username, $password)>

Attempt SASL authentication.

=item C<user($user)>

Send the USER command.

=item C<pass($pass)>

Send the PASS command. Returns the number of messages in the mailbox.

=item C<login([$user[, $pass]])>

Send both the USER and PASS commands. If C<$pass> is not given the
C<Net::POP3> uses C<Net::Netrc> to lookup the password using the host
and username. If the username is not specified then the current user name
will be used.

Returns the number of messages in the mailbox. However if there are no
messages on the server the string C<"0E0"> will be returned. This is
will give a true value in a boolean context, but zero in a numeric context.

If there was an error authenticating the user then I<undef> will be returned.

=item C<starttls(%sslargs)>

Upgrade existing plain connection to SSL.
You can use SSL arguments as documented in L<IO::Socket::SSL>, but it will
usually use the right arguments already.

=item C<apop([$user[, $pass]])>

Authenticate with the server identifying as C<$user> with password C<$pass>.
Similar to L</login>, but the password is not sent in clear text.

To use this method you must have the Digest::MD5 or the MD5 module installed,
otherwise this method will return I<undef>.

=item C<banner()>

Return the sever's connection banner

=item C<capa()>

Return a reference to a hash of the capabilities of the server.  APOP
is added as a pseudo capability.  Note that I've been unable to
find a list of the standard capability values, and some appear to
be multi-word and some are not.  We make an attempt at intelligently
parsing them, but it may not be correct.

=item C<capabilities()>

Just like capa, but only uses a cache from the last time we asked
the server, so as to avoid asking more than once.

=item C<top($msgnum[, $numlines])>

Get the header and the first C<$numlines> of the body for the message
C<$msgnum>. Returns a reference to an array which contains the lines of text
read from the server.

=item C<list([$msgnum])>

If called with an argument the C<list> returns the size of the message
in octets.

If called without arguments a reference to a hash is returned. The
keys will be the C<$msgnum>'s of all undeleted messages and the values will
be their size in octets.

=item C<get($msgnum[, $fh])>

Get the message C<$msgnum> from the remote mailbox. If C<$fh> is not given
then get returns a reference to an array which contains the lines of
text read from the server. If C<$fh> is given then the lines returned
from the server are printed to the filehandle C<$fh>.

=item C<getfh($msgnum)>

As per get(), but returns a tied filehandle.  Reading from this
filehandle returns the requested message.  The filehandle will return
EOF at the end of the message and should not be reused.

=item C<last()>

Returns the highest C<$msgnum> of all the messages accessed.

=item C<popstat()>

Returns a list of two elements. These are the number of undeleted
elements and the size of the mbox in octets.

=item C<ping($user)>

Returns a list of two elements. These are the number of new messages
and the total number of messages for C<$user>.

=item C<uidl([$msgnum])>

Returns a unique identifier for C<$msgnum> if given. If C<$msgnum> is not
given C<uidl> returns a reference to a hash where the keys are the
message numbers and the values are the unique identifiers.

=item C<delete($msgnum)>

Mark message C<$msgnum> to be deleted from the remote mailbox. All messages
that are marked to be deleted will be removed from the remote mailbox
when the server connection closed.

=item C<reset()>

Reset the status of the remote POP3 server. This includes resetting the
status of all messages to not be deleted.

=item C<quit()>

Quit and close the connection to the remote POP3 server. Any messages marked
as deleted will be deleted from the remote mailbox.

=item C<can_inet6()>

Returns whether we can use IPv6.

=item C<can_ssl()>

Returns whether we can use SSL.

=back

=head2 Notes

If a C<Net::POP3> object goes out of scope before C<quit> method is called
then the C<reset> method will called before the connection is closed. This
means that any messages marked to be deleted will not be.

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

See L<https://rt.cpan.org/Dist/Display.html?Status=Active&Queue=libnet>.

=head1 SEE ALSO

L<Net::Netrc>,
L<Net::Cmd>,
L<IO::Socket::SSL>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-2004 Graham Barr.  All rights reserved.

Copyright (C) 2013-2016, 2020 Steve Hay.  All rights reserved.

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
