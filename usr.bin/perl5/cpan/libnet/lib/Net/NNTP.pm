# Net::NNTP.pm
#
# Copyright (C) 1995-1997 Graham Barr.  All rights reserved.
# Copyright (C) 2013-2016, 2020 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package Net::NNTP;

use 5.008001;

use strict;
use warnings;

use Carp;
use IO::Socket;
use Net::Cmd;
use Net::Config;
use Time::Local;

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
  my $obj;

  $host ||= $ENV{NNTPSERVER} || $ENV{NEWSHOST};

  my $hosts = defined $host ? [$host] : $NetConfig{nntp_hosts};

  @{$hosts} = qw(news)
    unless @{$hosts};

  my %connect = ( Proto => 'tcp');

  if ($arg{SSL}) {
    # SSL from start
    die $nossl_warn if ! $ssl_class;
    $arg{Port} ||= 563;
    $connect{$_} = $arg{$_} for(grep { m{^SSL_} } keys %arg);
  }

  foreach my $o (qw(LocalAddr LocalPort Timeout)) {
    $connect{$o} = $arg{$o} if exists $arg{$o};
  }
  $connect{$family_key} = $arg{Domain} || $arg{Family};
  $connect{Timeout} = 120 unless defined $connect{Timeout};
  $connect{PeerPort} = $arg{Port} || 'nntp(119)';
  foreach my $h (@{$hosts}) {
    $connect{PeerAddr} = $h;
    $obj = $type->SUPER::new(%connect) or next;
    ${*$obj}{'net_nntp_host'} = $h;
    ${*$obj}{'net_nntp_arg'} = \%arg;
    if ($arg{SSL}) {
      Net::NNTP::_SSL->start_SSL($obj,%arg) or next;
    }
  }

  return
    unless defined $obj;

  $obj->autoflush(1);
  $obj->debug(exists $arg{Debug} ? $arg{Debug} : undef);

  unless ($obj->response() == CMD_OK) {
    $obj->close;
    return;
  }

  my $c = $obj->code;
  my @m = $obj->message;

  unless (exists $arg{Reader} && $arg{Reader} == 0) {

    # if server is INN and we have transfer rights the we are currently
    # talking to innd not nnrpd
    if ($obj->reader) {

      # If reader succeeds the we need to consider this code to determine postok
      $c = $obj->code;
    }
    else {

      # I want to ignore this failure, so restore the previous status.
      $obj->set_status($c, \@m);
    }
  }

  ${*$obj}{'net_nntp_post'} = $c == 200 ? 1 : 0;

  $obj;
}


sub host {
  my $me = shift;
  ${*$me}{'net_nntp_host'};
}


sub debug_text {
  my $nntp  = shift;
  my $inout = shift;
  my $text  = shift;

  if ( (ref($nntp) and $nntp->code == 350 and $text =~ /^(\S+)/)
    || ($text =~ /^(authinfo\s+pass)/io))
  {
    $text = "$1 ....\n";
  }

  $text;
}


sub postok {
  @_ == 1 or croak 'usage: $nntp->postok()';
  my $nntp = shift;
  ${*$nntp}{'net_nntp_post'} || 0;
}


sub starttls {
  my $self = shift;
  $ssl_class or die $nossl_warn;
  $self->_STARTTLS or return;
  Net::NNTP::_SSL->start_SSL($self,
    %{ ${*$self}{'net_nntp_arg'} }, # (ssl) args given in new
    @_   # more (ssl) args
  ) or return;
  return 1;
}


sub article {
  @_ >= 1 && @_ <= 3 or croak 'usage: $nntp->article([{$msgid|$msgnum}[, $fh]])';
  my $nntp = shift;
  my @fh;

  @fh = (pop) if @_ == 2 || (@_ && (ref($_[0]) || ref(\$_[0]) eq 'GLOB'));

  $nntp->_ARTICLE(@_)
    ? $nntp->read_until_dot(@fh)
    : undef;
}


sub articlefh {
  @_ >= 1 && @_ <= 2 or croak 'usage: $nntp->articlefh([{$msgid|$msgnum}])';
  my $nntp = shift;

  return unless $nntp->_ARTICLE(@_);
  return $nntp->tied_fh;
}


sub authinfo {
  @_ == 3 or croak 'usage: $nntp->authinfo($user, $pass)';
  my ($nntp, $user, $pass) = @_;

  $nntp->_AUTHINFO("USER",      $user) == CMD_MORE
    && $nntp->_AUTHINFO("PASS", $pass) == CMD_OK;
}


sub authinfo_simple {
  @_ == 3 or croak 'usage: $nntp->authinfo_simple($user, $pass)';
  my ($nntp, $user, $pass) = @_;

  $nntp->_AUTHINFO('SIMPLE') == CMD_MORE
    && $nntp->command($user, $pass)->response == CMD_OK;
}


sub body {
  @_ >= 1 && @_ <= 3 or croak 'usage: $nntp->body([{$msgid|$msgnum}[, $fh]])';
  my $nntp = shift;
  my @fh;

  @fh = (pop) if @_ == 2 || (@_ && ref($_[0]) || ref(\$_[0]) eq 'GLOB');

  $nntp->_BODY(@_)
    ? $nntp->read_until_dot(@fh)
    : undef;
}


sub bodyfh {
  @_ >= 1 && @_ <= 2 or croak 'usage: $nntp->bodyfh([{$msgid|$msgnum}])';
  my $nntp = shift;
  return unless $nntp->_BODY(@_);
  return $nntp->tied_fh;
}


sub head {
  @_ >= 1 && @_ <= 3 or croak 'usage: $nntp->head([{$msgid|$msgnum}[, $fh]])';
  my $nntp = shift;
  my @fh;

  @fh = (pop) if @_ == 2 || (@_ && ref($_[0]) || ref(\$_[0]) eq 'GLOB');

  $nntp->_HEAD(@_)
    ? $nntp->read_until_dot(@fh)
    : undef;
}


sub headfh {
  @_ >= 1 && @_ <= 2 or croak 'usage: $nntp->headfh([{$msgid|$msgnum}])';
  my $nntp = shift;
  return unless $nntp->_HEAD(@_);
  return $nntp->tied_fh;
}


sub nntpstat {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->nntpstat([{$msgid|$msgnum}])';
  my $nntp = shift;

  $nntp->_STAT(@_) && $nntp->message =~ /(<[^>]+>)/o
    ? $1
    : undef;
}


sub group {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->group([$group])';
  my $nntp = shift;
  my $grp  = ${*$nntp}{'net_nntp_group'};

  return $grp
    unless (@_ || wantarray);

  my $newgrp = shift;

  $newgrp = (defined($grp) and length($grp)) ? $grp : ""
    unless defined($newgrp) and length($newgrp);

  return 
    unless $nntp->_GROUP($newgrp) and $nntp->message =~ /(\d+)\s+(\d+)\s+(\d+)\s+(\S+)/;

  my ($count, $first, $last, $group) = ($1, $2, $3, $4);

  # group may be replied as '(current group)'
  $group = ${*$nntp}{'net_nntp_group'}
    if $group =~ /\(/;

  ${*$nntp}{'net_nntp_group'} = $group;

  wantarray
    ? ($count, $first, $last, $group)
    : $group;
}


sub help {
  @_ == 1 or croak 'usage: $nntp->help()';
  my $nntp = shift;

  $nntp->_HELP
    ? $nntp->read_until_dot
    : undef;
}


sub ihave {
  @_ >= 2 or croak 'usage: $nntp->ihave($msgid[, $message])';
  my $nntp  = shift;
  my $msgid = shift;

  $nntp->_IHAVE($msgid) && $nntp->datasend(@_)
    ? @_ == 0 || $nntp->dataend
    : undef;
}


sub last {
  @_ == 1 or croak 'usage: $nntp->last()';
  my $nntp = shift;

  $nntp->_LAST && $nntp->message =~ /(<[^>]+>)/o
    ? $1
    : undef;
}


sub list {
  @_ == 1 or croak 'usage: $nntp->list()';
  my $nntp = shift;

  $nntp->_LIST
    ? $nntp->_grouplist
    : undef;
}


sub newgroups {
  @_ >= 2 or croak 'usage: $nntp->newgroups($since[, $distributions])';
  my $nntp = shift;
  my $since = _timestr(shift);
  my $distributions = shift || "";

  $distributions = join(",", @{$distributions})
    if ref($distributions);

  $nntp->_NEWGROUPS($since, $distributions)
    ? $nntp->_grouplist
    : undef;
}


sub newnews {
  @_ >= 2 && @_ <= 4
    or croak 'usage: $nntp->newnews($since[, $groups[, $distributions]])';
  my $nntp = shift;
  my $since = _timestr(shift);
  my $groups = @_ ? shift : $nntp->group;
  my $distributions = shift || "";

  $groups ||= "*";
  $groups = join(",", @{$groups})
    if ref($groups);

  $distributions = join(",", @{$distributions})
    if ref($distributions);

  $nntp->_NEWNEWS($groups, $since, $distributions)
    ? $nntp->_articlelist
    : undef;
}


sub next {
  @_ == 1 or croak 'usage: $nntp->next()';
  my $nntp = shift;

  $nntp->_NEXT && $nntp->message =~ /(<[^>]+>)/o
    ? $1
    : undef;
}


sub post {
  @_ >= 1 or croak 'usage: $nntp->post([$message])';
  my $nntp = shift;

  $nntp->_POST() && $nntp->datasend(@_)
    ? @_ == 0 || $nntp->dataend
    : undef;
}


sub postfh {
  my $nntp = shift;
  return unless $nntp->_POST();
  return $nntp->tied_fh;
}


sub quit {
  @_ == 1 or croak 'usage: $nntp->quit()';
  my $nntp = shift;

  $nntp->_QUIT;
  $nntp->close;
}


sub slave {
  @_ == 1 or croak 'usage: $nntp->slave()';
  my $nntp = shift;

  $nntp->_SLAVE;
}

##
## The following methods are not implemented by all servers
##


sub active {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->active([$pattern])';
  my $nntp = shift;

  $nntp->_LIST('ACTIVE', @_)
    ? $nntp->_grouplist
    : undef;
}


sub active_times {
  @_ == 1 or croak 'usage: $nntp->active_times()';
  my $nntp = shift;

  $nntp->_LIST('ACTIVE.TIMES')
    ? $nntp->_grouplist
    : undef;
}


sub distributions {
  @_ == 1 or croak 'usage: $nntp->distributions()';
  my $nntp = shift;

  $nntp->_LIST('DISTRIBUTIONS')
    ? $nntp->_description
    : undef;
}


sub distribution_patterns {
  @_ == 1 or croak 'usage: $nntp->distribution_patterns()';
  my $nntp = shift;

  my $arr;
  local $_;

  ## no critic (ControlStructures::ProhibitMutatingListFunctions)
  $nntp->_LIST('DISTRIB.PATS')
    && ($arr = $nntp->read_until_dot)
    ? [grep { /^\d/ && (chomp, $_ = [split /:/]) } @$arr]
    : undef;
}


sub newsgroups {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->newsgroups([$pattern])';
  my $nntp = shift;

  $nntp->_LIST('NEWSGROUPS', @_)
    ? $nntp->_description
    : undef;
}


sub overview_fmt {
  @_ == 1 or croak 'usage: $nntp->overview_fmt()';
  my $nntp = shift;

  $nntp->_LIST('OVERVIEW.FMT')
    ? $nntp->_articlelist
    : undef;
}


sub subscriptions {
  @_ == 1 or croak 'usage: $nntp->subscriptions()';
  my $nntp = shift;

  $nntp->_LIST('SUBSCRIPTIONS')
    ? $nntp->_articlelist
    : undef;
}


sub listgroup {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->listgroup([$group])';
  my $nntp = shift;

  $nntp->_LISTGROUP(@_)
    ? $nntp->_articlelist
    : undef;
}


sub reader {
  @_ == 1 or croak 'usage: $nntp->reader()';
  my $nntp = shift;

  $nntp->_MODE('READER');
}


sub xgtitle {
  @_ == 1 || @_ == 2 or croak 'usage: $nntp->xgtitle([$pattern])';
  my $nntp = shift;

  $nntp->_XGTITLE(@_)
    ? $nntp->_description
    : undef;
}


sub xhdr {
  @_ >= 2 && @_ <= 4 or croak 'usage: $nntp->xhdr($header[, $message_spec])';
  my $nntp = shift;
  my $header = shift;
  my $arg = _msg_arg(@_);

  $nntp->_XHDR($header, $arg)
    ? $nntp->_description
    : undef;
}


sub xover {
  @_ == 2 || @_ == 3 or croak 'usage: $nntp->xover($message_spec)';
  my $nntp = shift;
  my $arg  = _msg_arg(@_);

  $nntp->_XOVER($arg)
    ? $nntp->_fieldlist
    : undef;
}


sub xpat {
  @_ == 4 || @_ == 5 or croak 'usage: $nntp->xpat($header, $pattern, $message_spec )';
  my $nntp = shift;
  my $header = shift;
  my $pattern = shift;
  my $arg = _msg_arg(@_);

  $pattern = join(" ", @$pattern)
    if ref($pattern);

  $nntp->_XPAT($header, $arg, $pattern)
    ? $nntp->_description
    : undef;
}


sub xpath {
  @_ == 2 or croak 'usage: $nntp->xpath($message_id)';
  my ($nntp, $message_id) = @_;

  return
    unless $nntp->_XPATH($message_id);

  my $m;
  ($m = $nntp->message) =~ s/^\d+\s+//o;
  my @p = split /\s+/, $m;

  wantarray ? @p : $p[0];
}


sub xrover {
  @_ == 2 || @_ == 3 or croak 'usage: $nntp->xrover($message_spec)';
  my $nntp = shift;
  my $arg  = _msg_arg(@_);

  $nntp->_XROVER($arg)
    ? $nntp->_description
    : undef;
}


sub date {
  @_ == 1 or croak 'usage: $nntp->date()';
  my $nntp = shift;

  $nntp->_DATE
    && $nntp->message =~ /(\d{4})(\d\d)(\d\d)(\d\d)(\d\d)(\d\d)/
    ? timegm($6, $5, $4, $3, $2 - 1, $1)
    : undef;
}


##
## Private subroutines
##


sub _msg_arg {
  my $spec = shift;
  my $arg  = "";

  if (@_) {
    carp "Depriciated passing of two message numbers, " . "pass a reference"
      if $^W;
    $spec = [$spec, $_[0]];
  }

  if (defined $spec) {
    if (ref($spec)) {
      $arg = $spec->[0];
      if (defined $spec->[1]) {
        $arg .= "-"
          if $spec->[1] != $spec->[0];
        $arg .= $spec->[1]
          if $spec->[1] > $spec->[0];
      }
    }
    else {
      $arg = $spec;
    }
  }

  $arg;
}


sub _timestr {
  my $time = shift;
  my @g    = reverse((gmtime($time))[0 .. 5]);
  $g[1] += 1;
  $g[0] %= 100;
  sprintf "%02d%02d%02d %02d%02d%02d GMT", @g;
}


sub _grouplist {
  my $nntp = shift;
  my $arr  = $nntp->read_until_dot
    or return;

  my $hash = {};

  foreach my $ln (@$arr) {
    my @a = split(/[\s\n]+/, $ln);
    $hash->{$a[0]} = [@a[1, 2, 3]];
  }

  $hash;
}


sub _fieldlist {
  my $nntp = shift;
  my $arr  = $nntp->read_until_dot
    or return;

  my $hash = {};

  foreach my $ln (@$arr) {
    my @a = split(/[\t\n]/, $ln);
    my $m = shift @a;
    $hash->{$m} = [@a];
  }

  $hash;
}


sub _articlelist {
  my $nntp = shift;
  my $arr  = $nntp->read_until_dot;

  chomp(@$arr)
    if $arr;

  $arr;
}


sub _description {
  my $nntp = shift;
  my $arr  = $nntp->read_until_dot
    or return;

  my $hash = {};

  foreach my $ln (@$arr) {
    chomp($ln);

    $hash->{$1} = $ln
      if $ln =~ s/^\s*(\S+)\s*//o;
  }

  $hash;

}

##
## The commands
##


sub _ARTICLE  { shift->command('ARTICLE',  @_)->response == CMD_OK }
sub _AUTHINFO { shift->command('AUTHINFO', @_)->response }
sub _BODY     { shift->command('BODY',     @_)->response == CMD_OK }
sub _DATE      { shift->command('DATE')->response == CMD_INFO }
sub _GROUP     { shift->command('GROUP', @_)->response == CMD_OK }
sub _HEAD      { shift->command('HEAD', @_)->response == CMD_OK }
sub _HELP      { shift->command('HELP', @_)->response == CMD_INFO }
sub _IHAVE     { shift->command('IHAVE', @_)->response == CMD_MORE }
sub _LAST      { shift->command('LAST')->response == CMD_OK }
sub _LIST      { shift->command('LIST', @_)->response == CMD_OK }
sub _LISTGROUP { shift->command('LISTGROUP', @_)->response == CMD_OK }
sub _NEWGROUPS { shift->command('NEWGROUPS', @_)->response == CMD_OK }
sub _NEWNEWS   { shift->command('NEWNEWS', @_)->response == CMD_OK }
sub _NEXT      { shift->command('NEXT')->response == CMD_OK }
sub _POST      { shift->command('POST', @_)->response == CMD_MORE }
sub _QUIT      { shift->command('QUIT', @_)->response == CMD_OK }
sub _SLAVE     { shift->command('SLAVE', @_)->response == CMD_OK }
sub _STARTTLS  { shift->command("STARTTLS")->response() == CMD_MORE }
sub _STAT      { shift->command('STAT', @_)->response == CMD_OK }
sub _MODE      { shift->command('MODE', @_)->response == CMD_OK }
sub _XGTITLE   { shift->command('XGTITLE', @_)->response == CMD_OK }
sub _XHDR      { shift->command('XHDR', @_)->response == CMD_OK }
sub _XPAT      { shift->command('XPAT', @_)->response == CMD_OK }
sub _XPATH     { shift->command('XPATH', @_)->response == CMD_OK }
sub _XOVER     { shift->command('XOVER', @_)->response == CMD_OK }
sub _XROVER    { shift->command('XROVER', @_)->response == CMD_OK }
sub _XTHREAD   { shift->unsupported }
sub _XSEARCH   { shift->unsupported }
sub _XINDEX    { shift->unsupported }

##
## IO/perl methods
##


sub DESTROY {
  my $nntp = shift;
  defined(fileno($nntp)) && $nntp->quit;
}

{
  package Net::NNTP::_SSL;
  our @ISA = ( $ssl_class ? ($ssl_class):(), 'Net::NNTP' );
  sub starttls { die "NNTP connection is already in SSL mode" }
  sub start_SSL {
    my ($class,$nntp,%arg) = @_;
    delete @arg{ grep { !m{^SSL_} } keys %arg };
    ( $arg{SSL_verifycn_name} ||= $nntp->host )
        =~s{(?<!:):[\w()]+$}{}; # strip port
    $arg{SSL_hostname} = $arg{SSL_verifycn_name}
        if ! defined $arg{SSL_hostname} && $class->can_client_sni;
    my $ok = $class->SUPER::start_SSL($nntp,
      SSL_verifycn_scheme => 'nntp',
      %arg
    );
    $@ = $ssl_class->errstr if !$ok;
    return $ok;
  }
}




1;

__END__

=head1 NAME

Net::NNTP - NNTP Client class

=head1 SYNOPSIS

    use Net::NNTP;

    $nntp = Net::NNTP->new("some.host.name");
    $nntp->quit;

    # start with SSL, e.g. nntps
    $nntp = Net::NNTP->new("some.host.name", SSL => 1);

    # start with plain and upgrade to SSL
    $nntp = Net::NNTP->new("some.host.name");
    $nntp->starttls;


=head1 DESCRIPTION

C<Net::NNTP> is a class implementing a simple NNTP client in Perl as described
in RFC977 and RFC4642.
With L<IO::Socket::SSL> installed it also provides support for implicit and
explicit TLS encryption, i.e. NNTPS or NNTP+STARTTLS.

The Net::NNTP class is a subclass of Net::Cmd and (depending on avaibility) of
IO::Socket::IP, IO::Socket::INET6 or IO::Socket::INET.

=head2 Class Methods

=over 4

=item C<new([$host][, %options])>

This is the constructor for a new Net::NNTP object. C<$host> is the
name of the remote host to which a NNTP connection is required. If not
given then it may be passed as the C<Host> option described below. If no host is passed
then two environment variables are checked, first C<NNTPSERVER> then
C<NEWSHOST>, then C<Net::Config> is checked, and if a host is not found
then C<news> is used.

C<%options> are passed in a hash like fashion, using key and value pairs.
Possible options are:

B<Host> - NNTP host to connect to. It may be a single scalar, as defined for
the C<PeerAddr> option in L<IO::Socket::INET>, or a reference to
an array with hosts to try in turn. The L</host> method will return the value
which was used to connect to the host.

B<Port> - port to connect to.
Default - 119 for plain NNTP and 563 for immediate SSL (nntps).

B<SSL> - If the connection should be done from start with SSL, contrary to later
upgrade with C<starttls>.
You can use SSL arguments as documented in L<IO::Socket::SSL>, but it will
usually use the right arguments already.

B<Timeout> - Maximum time, in seconds, to wait for a response from the
NNTP server, a value of zero will cause all IO operations to block.
(default: 120)

B<Debug> - Enable the printing of debugging information to STDERR

B<Reader> - If the remote server is INN then initially the connection
will be to innd, by default C<Net::NNTP> will issue a C<MODE READER> command
so that the remote server becomes nnrpd. If the C<Reader> option is given
with a value of zero, then this command will not be sent and the
connection will be left talking to innd.

B<LocalAddr> and B<LocalPort> - These parameters are passed directly
to IO::Socket to allow binding the socket to a specific local address and port.

B<Domain> - This parameter is passed directly to IO::Socket and makes it
possible to enforce IPv4 connections even if L<IO::Socket::IP> is used as super
class. Alternatively B<Family> can be used.

=back

=head2 Object Methods

Unless otherwise stated all methods return either a I<true> or I<false>
value, with I<true> meaning that the operation was a success. When a method
states that it returns a value, failure will be returned as I<undef> or an
empty list.

C<Net::NNTP> inherits from C<Net::Cmd> so methods defined in C<Net::Cmd> may
be used to send commands to the remote NNTP server in addition to the methods
documented here.

=over 4

=item C<host()>

Returns the value used by the constructor, and passed to IO::Socket::INET,
to connect to the host.

=item C<starttls()>

Upgrade existing plain connection to SSL.
Any arguments necessary for SSL must be given in C<new> already.

=item C<article([{$msgid|$msgnum}[, $fh]])>

Retrieve the header, a blank line, then the body (text) of the
specified article. 

If C<$fh> is specified then it is expected to be a valid filehandle
and the result will be printed to it, on success a true value will be
returned. If C<$fh> is not specified then the return value, on success,
will be a reference to an array containing the article requested, each
entry in the array will contain one line of the article.

If no arguments are passed then the current article in the currently
selected newsgroup is fetched.

C<$msgnum> is a numeric id of an article in the current newsgroup, and
will change the current article pointer.  C<$msgid> is the message id of
an article as shown in that article's header.  It is anticipated that the
client will obtain the C<$msgid> from a list provided by the C<newnews>
command, from references contained within another article, or from the
message-id provided in the response to some other commands.

If there is an error then C<undef> will be returned.

=item C<body([{$msgid|$msgnum}[, [$fh]])>

Like C<article> but only fetches the body of the article.

=item C<head([{$msgid|$msgnum}[, [$fh]])>

Like C<article> but only fetches the headers for the article.

=item C<articlefh([{$msgid|$msgnum}])>

=item C<bodyfh([{$msgid|$msgnum}])>

=item C<headfh([{$msgid|$msgnum}])>

These are similar to article(), body() and head(), but rather than
returning the requested data directly, they return a tied filehandle
from which to read the article.

=item C<nntpstat([{$msgid|$msgnum}])>

The C<nntpstat> command is similar to the C<article> command except that no
text is returned.  When selecting by message number within a group,
the C<nntpstat> command serves to set the "current article pointer" without
sending text.

Using the C<nntpstat> command to
select by message-id is valid but of questionable value, since a
selection by message-id does B<not> alter the "current article pointer".

Returns the message-id of the "current article".

=item C<group([$group])>

Set and/or get the current group. If C<$group> is not given then information
is returned on the current group.

In a scalar context it returns the group name.

In an array context the return value is a list containing, the number
of articles in the group, the number of the first article, the number
of the last article and the group name.

=item C<help()>

Request help text (a short summary of commands that are understood by this
implementation) from the server. Returns the text or undef upon failure.

=item C<ihave($msgid[, $message])>

The C<ihave> command informs the server that the client has an article
whose id is C<$msgid>.  If the server desires a copy of that
article and C<$message> has been given then it will be sent.

Returns I<true> if the server desires the article and C<$message> was
successfully sent, if specified.

If C<$message> is not specified then the message must be sent using the
C<datasend> and C<dataend> methods from L<Net::Cmd>

C<$message> can be either an array of lines or a reference to an array
and must be encoded by the caller to octets of whatever encoding is required,
e.g. by using the Encode module's C<encode()> function.

=item C<last()>

Set the "current article pointer" to the previous article in the current
newsgroup.

Returns the message-id of the article.

=item C<date()>

Returns the date on the remote server. This date will be in a UNIX time
format (seconds since 1970)

=item C<postok()>

C<postok> will return I<true> if the servers initial response indicated
that it will allow posting.

=item C<authinfo($user, $pass)>

Authenticates to the server (using the original AUTHINFO USER / AUTHINFO PASS
form, defined in RFC2980) using the supplied username and password.  Please
note that the password is sent in clear text to the server.  This command
should not be used with valuable passwords unless the connection to the server
is somehow protected.

=item C<authinfo_simple($user, $pass)>

Authenticates to the server (using the proposed NNTP V2 AUTHINFO SIMPLE form,
defined and deprecated in RFC2980) using the supplied username and password.
As with L</authinfo> the password is sent in clear text.

=item C<list()>

Obtain information about all the active newsgroups. The results is a reference
to a hash where the key is a group name and each value is a reference to an
array. The elements in this array are:- the last article number in the group,
the first article number in the group and any information flags about the group.

=item C<newgroups($since[, $distributions])>

C<$since> is a time value and C<$distributions> is either a distribution
pattern or a reference to a list of distribution patterns.
The result is the same as C<list>, but the
groups return will be limited to those created after C<$since> and, if
specified, in one of the distribution areas in C<$distributions>. 

=item C<newnews($since[, $groups[, $distributions]])>

C<$since> is a time value. C<$groups> is either a group pattern or a reference
to a list of group patterns. C<$distributions> is either a distribution
pattern or a reference to a list of distribution patterns.

Returns a reference to a list which contains the message-ids of all news posted
after C<$since>, that are in a groups which matched C<$groups> and a
distribution which matches C<$distributions>.

=item C<next()>

Set the "current article pointer" to the next article in the current
newsgroup.

Returns the message-id of the article.

=item C<post([$message])>

Post a new article to the news server. If C<$message> is specified and posting
is allowed then the message will be sent.

If C<$message> is not specified then the message must be sent using the
C<datasend> and C<dataend> methods from L<Net::Cmd>

C<$message> can be either an array of lines or a reference to an array
and must be encoded by the caller to octets of whatever encoding is required,
e.g. by using the Encode module's C<encode()> function.

The message, either sent via C<datasend> or as the C<$message>
parameter, must be in the format as described by RFC822 and must
contain From:, Newsgroups: and Subject: headers.

=item C<postfh()>

Post a new article to the news server using a tied filehandle.  If
posting is allowed, this method will return a tied filehandle that you
can print() the contents of the article to be posted.  You must
explicitly close() the filehandle when you are finished posting the
article, and the return value from the close() call will indicate
whether the message was successfully posted.

=item C<slave()>

Tell the remote server that I am not a user client, but probably another
news server.

=item C<quit()>

Quit the remote server and close the socket connection.

=item C<can_inet6()>

Returns whether we can use IPv6.

=item C<can_ssl()>

Returns whether we can use SSL.

=back

=head2 Extension Methods

These methods use commands that are not part of the RFC977 documentation. Some
servers may not support all of them.

=over 4

=item C<newsgroups([$pattern])>

Returns a reference to a hash where the keys are all the group names which
match C<$pattern>, or all of the groups if no pattern is specified, and
each value contains the description text for the group.

=item C<distributions()>

Returns a reference to a hash where the keys are all the possible
distribution names and the values are the distribution descriptions.

=item C<distribution_patterns()>

Returns a reference to an array where each element, itself an array
reference, consists of the three fields of a line of the distrib.pats list
maintained by some NNTP servers, namely: a weight, a wildmat and a value
which the client may use to construct a Distribution header.

=item C<subscriptions()>

Returns a reference to a list which contains a list of groups which
are recommended for a new user to subscribe to.

=item C<overview_fmt()>

Returns a reference to an array which contain the names of the fields returned
by C<xover>.

=item C<active_times()>

Returns a reference to a hash where the keys are the group names and each
value is a reference to an array containing the time the groups was created
and an identifier, possibly an Email address, of the creator.

=item C<active([$pattern])>

Similar to C<list> but only active groups that match the pattern are returned.
C<$pattern> can be a group pattern.

=item C<xgtitle($pattern)>

Returns a reference to a hash where the keys are all the group names which
match C<$pattern> and each value is the description text for the group.

=item C<xhdr($header, $message_spec)>

Obtain the header field C<$header> for all the messages specified. 

The return value will be a reference
to a hash where the keys are the message numbers and each value contains
the text of the requested header for that message.

=item C<xover($message_spec)>

The return value will be a reference
to a hash where the keys are the message numbers and each value contains
a reference to an array which contains the overview fields for that
message.

The names of the fields can be obtained by calling C<overview_fmt>.

=item C<xpath($message_id)>

Returns the path name to the file on the server which contains the specified
message.

=item C<xpat($header, $pattern, $message_spec)>

The result is the same as C<xhdr> except the is will be restricted to
headers where the text of the header matches C<$pattern>

=item C<xrover($message_spec)>

The XROVER command returns reference information for the article(s)
specified.

Returns a reference to a HASH where the keys are the message numbers and the
values are the References: lines from the articles

=item C<listgroup([$group])>

Returns a reference to a list of all the active messages in C<$group>, or
the current group if C<$group> is not specified.

=item C<reader()>

Tell the server that you are a reader and not another server.

This is required by some servers. For example if you are connecting to
an INN server and you have transfer permission your connection will
be connected to the transfer daemon, not the NNTP daemon. Issuing
this command will cause the transfer daemon to hand over control
to the NNTP daemon.

Some servers do not understand this command, but issuing it and ignoring
the response is harmless.

=back

=head2 Unsupported

The following NNTP command are unsupported by the package, and there are
no plans to do so.

    AUTHINFO GENERIC
    XTHREAD
    XSEARCH
    XINDEX

=head2 Definitions

=over 4

=item $message_spec

C<$message_spec> is either a single message-id, a single message number, or
a reference to a list of two message numbers.

If C<$message_spec> is a reference to a list of two message numbers and the
second number in a range is less than or equal to the first then the range
represents all messages in the group after the first message number.

B<NOTE> For compatibility reasons only with earlier versions of Net::NNTP
a message spec can be passed as a list of two numbers, this is deprecated
and a reference to the list should now be passed

=item $pattern

The C<NNTP> protocol uses the C<WILDMAT> format for patterns.
The WILDMAT format was first developed by Rich Salz based on
the format used in the UNIX "find" command to articulate
file names. It was developed to provide a uniform mechanism
for matching patterns in the same manner that the UNIX shell
matches filenames.

Patterns are implicitly anchored at the
beginning and end of each string when testing for a match.

There are five pattern matching operations other than a strict
one-to-one match between the pattern and the source to be
checked for a match.

The first is an asterisk C<*> to match any sequence of zero or more
characters.

The second is a question mark C<?> to match any single character. The
third specifies a specific set of characters.

The set is specified as a list of characters, or as a range of characters
where the beginning and end of the range are separated by a minus (or dash)
character, or as any combination of lists and ranges. The dash can
also be included in the set as a character it if is the beginning
or end of the set. This set is enclosed in square brackets. The
close square bracket C<]> may be used in a set if it is the first
character in the set.

The fourth operation is the same as the
logical not of the third operation and is specified the same
way as the third with the addition of a caret character C<^> at
the beginning of the test string just inside the open square
bracket.

The final operation uses the backslash character to
invalidate the special meaning of an open square bracket C<[>,
the asterisk, backslash or the question mark. Two backslashes in
sequence will result in the evaluation of the backslash as a
character with no special meaning.

=over 4

=item Examples

=item C<[^]-]>

matches any single character other than a close square
bracket or a minus sign/dash.

=item C<*bdc>

matches any string that ends with the string "bdc"
including the string "bdc" (without quotes).

=item C<[0-9a-zA-Z]>

matches any single printable alphanumeric ASCII character.

=item C<a??d>

matches any four character string which begins
with a and ends with d.

=back

=back

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

See L<https://rt.cpan.org/Dist/Display.html?Status=Active&Queue=libnet>.

=head1 SEE ALSO

L<Net::Cmd>,
L<IO::Socket::SSL>.

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-1997 Graham Barr.  All rights reserved.

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
