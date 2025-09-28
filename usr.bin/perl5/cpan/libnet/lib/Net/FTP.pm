# Net::FTP.pm
#
# Copyright (C) 1995-2004 Graham Barr.  All rights reserved.
# Copyright (C) 2013-2017, 2020, 2022 Steve Hay.  All rights reserved.
# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.
#
# Documentation (at end) improved 1996 by Nathan Torkington <gnat@frii.com>.

package Net::FTP;

use 5.008001;

use strict;
use warnings;

use Carp;
use Fcntl qw(O_WRONLY O_RDONLY O_APPEND O_CREAT O_TRUNC);
use IO::Socket;
use Net::Cmd;
use Net::Config;
use Socket;
use Time::Local;

our $VERSION = '3.15';

our $IOCLASS;
my $family_key;
BEGIN {
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
  my $inet6_class = eval {
    require IO::Socket::IP;
    no warnings 'numeric';
    IO::Socket::IP->VERSION(0.25);
  } && 'IO::Socket::IP' || eval {
    require IO::Socket::INET6;
    no warnings 'numeric';
    IO::Socket::INET6->VERSION(2.62);
  } && 'IO::Socket::INET6';

  sub can_ssl   { $ssl_class };
  sub can_inet6 { $inet6_class };

  $IOCLASS = $ssl_class || $inet6_class || 'IO::Socket::INET';
  $family_key =
    ( $ssl_class ? $ssl_class->can_ipv6 : $inet6_class || '' )
      eq 'IO::Socket::IP'
      ? 'Family' : 'Domain';
}

our @ISA = ('Exporter','Net::Cmd',$IOCLASS);

use constant TELNET_IAC => 255;
use constant TELNET_IP  => 244;
use constant TELNET_DM  => 242;

use constant EBCDIC => ord 'A' == 193;

sub new {
  my $pkg = shift;
  my ($peer, %arg);
  if (@_ % 2) {
    $peer = shift;
    %arg  = @_;
  }
  else {
    %arg  = @_;
    $peer = delete $arg{Host};
  }

  my $host      = $peer;
  my $fire      = undef;
  my $fire_type = undef;

  if (exists($arg{Firewall}) || Net::Config->requires_firewall($peer)) {
         $fire = $arg{Firewall}
      || $ENV{FTP_FIREWALL}
      || $NetConfig{ftp_firewall}
      || undef;

    if (defined $fire) {
      $peer = $fire;
      delete $arg{Port};
           $fire_type = $arg{FirewallType}
        || $ENV{FTP_FIREWALL_TYPE}
        || $NetConfig{firewall_type}
        || undef;
    }
  }

  my %tlsargs;
  if (can_ssl()) {
    # for name verification strip port from domain:port, ipv4:port, [ipv6]:port
    (my $hostname = $host) =~s{(?<!:):\d+$}{};
    %tlsargs = (
      SSL_verifycn_scheme => 'ftp',
      SSL_verifycn_name => $hostname,
      # use SNI if supported by IO::Socket::SSL
      $pkg->can_client_sni ? (SSL_hostname => $hostname):(),
      # reuse SSL session of control connection in data connections
      SSL_session_cache_size => 10,
      SSL_session_key => $hostname,
    );
    # user defined SSL arg
    $tlsargs{$_} = $arg{$_} for(grep { m{^SSL_} } keys %arg);
    $tlsargs{SSL_reuse_ctx} = IO::Socket::SSL::SSL_Context->new(%tlsargs)
      or return;

  } elsif ($arg{SSL}) {
    croak("IO::Socket::SSL >= 2.007 needed for SSL support");
  }

  my $ftp = $pkg->SUPER::new(
    PeerAddr  => $peer,
    PeerPort  => $arg{Port} || ($arg{SSL} ? 'ftps(990)' : 'ftp(21)'),
    LocalAddr => $arg{'LocalAddr'},
    $family_key => $arg{Domain} || $arg{Family},
    Proto     => 'tcp',
    Timeout   => defined $arg{Timeout} ? $arg{Timeout} : 120,
    %tlsargs,
    $arg{SSL} ? ():( SSL_startHandshake => 0 ),
  ) or return;

  ${*$ftp}{'net_ftp_host'}    = $host;                             # Remote hostname
  ${*$ftp}{'net_ftp_type'}    = 'A';                               # ASCII/binary/etc mode
  ${*$ftp}{'net_ftp_blksize'} = abs($arg{'BlockSize'} || 10240);

  ${*$ftp}{'net_ftp_localaddr'} = $arg{'LocalAddr'};
  ${*$ftp}{'net_ftp_domain'} = $arg{Domain} || $arg{Family};

  ${*$ftp}{'net_ftp_firewall'} = $fire
    if (defined $fire);
  ${*$ftp}{'net_ftp_firewall_type'} = $fire_type
    if (defined $fire_type);

  ${*$ftp}{'net_ftp_passive'} =
      int exists $arg{Passive} ? $arg{Passive}
    : exists $ENV{FTP_PASSIVE} ? $ENV{FTP_PASSIVE}
    : defined $fire            ? $NetConfig{ftp_ext_passive}
    : $NetConfig{ftp_int_passive};    # Whew! :-)

  ${*$ftp}{net_ftp_tlsargs} = \%tlsargs if %tlsargs;
  if ($arg{SSL}) {
    ${*$ftp}{net_ftp_tlsprot} = 'P';
    ${*$ftp}{net_ftp_tlsdirect} = 1;
  }

  $ftp->hash(exists $arg{Hash} ? $arg{Hash} : 0, 1024);

  $ftp->autoflush(1);

  $ftp->debug(exists $arg{Debug} ? $arg{Debug} : undef);

  unless ($ftp->response() == CMD_OK) {
    $ftp->close();
    # keep @$ if no message. Happens, when response did not start with a code.
    $@ = $ftp->message || $@;
    undef $ftp;
  }

  $ftp;
}

##
## User interface methods
##


sub host {
  my $me = shift;
  ${*$me}{'net_ftp_host'};
}

sub passive {
  my $ftp = shift;
  return ${*$ftp}{'net_ftp_passive'} unless @_;
  ${*$ftp}{'net_ftp_passive'} = shift;
}


sub hash {
  my $ftp = shift;    # self

  my ($h, $b) = @_;
  unless ($h) {
    delete ${*$ftp}{'net_ftp_hash'};
    return [\*STDERR, 0];
  }
  ($h, $b) = (ref($h) ? $h : \*STDERR, $b || 1024);
  select((select($h), $| = 1)[0]);
  $b = 512 if $b < 512;
  ${*$ftp}{'net_ftp_hash'} = [$h, $b];
}


sub quit {
  my $ftp = shift;

  $ftp->_QUIT;
  $ftp->close;
}


sub DESTROY { }


sub ascii  { shift->type('A', @_); }
sub binary { shift->type('I', @_); }


sub ebcdic {
  carp "TYPE E is unsupported, shall default to I";
  shift->type('E', @_);
}


sub byte {
  carp "TYPE L is unsupported, shall default to I";
  shift->type('L', @_);
}

# Allow the user to send a command directly, BE CAREFUL !!


sub quot {
  my $ftp = shift;
  my $cmd = shift;

  $ftp->command(uc $cmd, @_);
  $ftp->response();
}


sub site {
  my $ftp = shift;

  $ftp->command("SITE", @_);
  $ftp->response();
}


sub mdtm {
  my $ftp  = shift;
  my $file = shift;

  # Server Y2K bug workaround
  #
  # sigh; some idiotic FTP servers use ("19%d",tm.tm_year) instead of
  # ("%d",tm.tm_year+1900).  This results in an extra digit in the
  # string returned. To account for this we allow an optional extra
  # digit in the year. Then if the first two digits are 19 we use the
  # remainder, otherwise we subtract 1900 from the whole year.

  $ftp->_MDTM($file)
    && $ftp->message =~ /((\d\d)(\d\d\d?))(\d\d)(\d\d)(\d\d)(\d\d)(\d\d)/
    ? timegm($8, $7, $6, $5, $4 - 1, $2 eq '19' ? ($3 + 1900) : $1)
    : undef;
}


sub size {
  my $ftp  = shift;
  my $file = shift;
  my $io;
  if ($ftp->supported("SIZE")) {
    return $ftp->_SIZE($file)
      ? ($ftp->message =~ /(\d+)\s*(bytes?\s*)?$/)[0]
      : undef;
  }
  elsif ($ftp->supported("STAT")) {
    my @msg;
    return
      unless $ftp->_STAT($file) && (@msg = $ftp->message) == 3;
    foreach my $line (@msg) {
      return (split(/\s+/, $line))[4]
        if $line =~ /^[-rwxSsTt]{10}/;
    }
  }
  else {
    my @files = $ftp->dir($file);
    if (@files) {
      return (split(/\s+/, $1))[4]
        if $files[0] =~ /^([-rwxSsTt]{10}.*)$/;
    }
  }
  undef;
}


sub starttls {
  my $ftp = shift;
  can_ssl() or croak("IO::Socket::SSL >= 2.007 needed for SSL support");
  $ftp->is_SSL and croak("called starttls within SSL session");
  $ftp->_AUTH('TLS') == CMD_OK or return;

  $ftp->connect_SSL or return;
  $ftp->prot('P');
  return 1;
}

sub prot {
  my ($ftp,$prot) = @_;
  $prot eq 'C' or $prot eq 'P' or croak("prot must by C or P");
  $ftp->_PBSZ(0) or return;
  $ftp->_PROT($prot) or return;
  ${*$ftp}{net_ftp_tlsprot} = $prot;
  return 1;
}

sub stoptls {
  my $ftp = shift;
  $ftp->is_SSL or croak("called stoptls outside SSL session");
  ${*$ftp}{net_ftp_tlsdirect} and croak("cannot stoptls direct SSL session");
  $ftp->_CCC() or return;
  $ftp->stop_SSL();
  return 1;
}

sub login {
  my ($ftp, $user, $pass, $acct) = @_;
  my ($ok, $ruser, $fwtype);

  unless (defined $user) {
    require Net::Netrc;

    my $rc = Net::Netrc->lookup(${*$ftp}{'net_ftp_host'});

    ($user, $pass, $acct) = $rc->lpa()
      if ($rc);
  }

  $user ||= "anonymous";
  $ruser = $user;

  $fwtype = ${*$ftp}{'net_ftp_firewall_type'}
    || $NetConfig{'ftp_firewall_type'}
    || 0;

  if ($fwtype && defined ${*$ftp}{'net_ftp_firewall'}) {
    if ($fwtype == 1 || $fwtype == 7) {
      $user .= '@' . ${*$ftp}{'net_ftp_host'};
    }
    else {
      require Net::Netrc;

      my $rc = Net::Netrc->lookup(${*$ftp}{'net_ftp_firewall'});

      my ($fwuser, $fwpass, $fwacct) = $rc ? $rc->lpa() : ();

      if ($fwtype == 5) {
        $user = join('@', $user, $fwuser, ${*$ftp}{'net_ftp_host'});
        $pass = $pass . '@' . $fwpass;
      }
      else {
        if ($fwtype == 2) {
          $user .= '@' . ${*$ftp}{'net_ftp_host'};
        }
        elsif ($fwtype == 6) {
          $fwuser .= '@' . ${*$ftp}{'net_ftp_host'};
        }

        $ok = $ftp->_USER($fwuser);

        return 0 unless $ok == CMD_OK || $ok == CMD_MORE;

        $ok = $ftp->_PASS($fwpass || "");

        return 0 unless $ok == CMD_OK || $ok == CMD_MORE;

        $ok = $ftp->_ACCT($fwacct)
          if defined($fwacct);

        if ($fwtype == 3) {
          $ok = $ftp->command("SITE", ${*$ftp}{'net_ftp_host'})->response;
        }
        elsif ($fwtype == 4) {
          $ok = $ftp->command("OPEN", ${*$ftp}{'net_ftp_host'})->response;
        }

        return 0 unless $ok == CMD_OK || $ok == CMD_MORE;
      }
    }
  }

  $ok = $ftp->_USER($user);

  # Some dumb firewalls don't prefix the connection messages
  $ok = $ftp->response()
    if ($ok == CMD_OK && $ftp->code == 220 && $user =~ /\@/);

  if ($ok == CMD_MORE) {
    unless (defined $pass) {
      require Net::Netrc;

      my $rc = Net::Netrc->lookup(${*$ftp}{'net_ftp_host'}, $ruser);

      ($ruser, $pass, $acct) = $rc->lpa()
        if ($rc);

      $pass = '-anonymous@'
        if (!defined $pass && (!defined($ruser) || $ruser =~ /^anonymous/o));
    }

    $ok = $ftp->_PASS($pass || "");
  }

  $ok = $ftp->_ACCT($acct)
    if (defined($acct) && ($ok == CMD_MORE || $ok == CMD_OK));

  if ($fwtype == 7 && $ok == CMD_OK && defined ${*$ftp}{'net_ftp_firewall'}) {
    my ($f, $auth, $resp) = _auth_id($ftp);
    $ftp->authorize($auth, $resp) if defined($resp);
  }

  $ok == CMD_OK;
}


sub account {
  @_ == 2 or croak 'usage: $ftp->account($acct)';
  my $ftp  = shift;
  my $acct = shift;
  $ftp->_ACCT($acct) == CMD_OK;
}


sub _auth_id {
  my ($ftp, $auth, $resp) = @_;

  unless (defined $resp) {
    require Net::Netrc;

    $auth ||= eval { (getpwuid($>))[0] } || $ENV{NAME};

    my $rc = Net::Netrc->lookup(${*$ftp}{'net_ftp_firewall'}, $auth)
      || Net::Netrc->lookup(${*$ftp}{'net_ftp_firewall'});

    ($auth, $resp) = $rc->lpa()
      if ($rc);
  }
  ($ftp, $auth, $resp);
}


sub authorize {
  @_ >= 1 || @_ <= 3 or croak 'usage: $ftp->authorize([$auth[, $resp]])';

  my ($ftp, $auth, $resp) = &_auth_id;

  my $ok = $ftp->_AUTH($auth || "");

  return $ftp->_RESP($resp || "")
    if ($ok == CMD_MORE);

  $ok == CMD_OK;
}


sub rename {
  @_ == 3 or croak 'usage: $ftp->rename($oldname, $newname)';

  my ($ftp, $oldname, $newname) = @_;

  $ftp->_RNFR($oldname)
    && $ftp->_RNTO($newname);
}


sub type {
  my $ftp    = shift;
  my $type   = shift;
  my $oldval = ${*$ftp}{'net_ftp_type'};

  return $oldval
    unless (defined $type);

  return
    unless ($ftp->_TYPE($type, @_));

  ${*$ftp}{'net_ftp_type'} = join(" ", $type, @_);

  $oldval;
}


sub alloc {
  my $ftp    = shift;
  my $size   = shift;
  my $oldval = ${*$ftp}{'net_ftp_allo'};

  return $oldval
    unless (defined $size);

  return
    unless ($ftp->supported("ALLO") and $ftp->_ALLO($size, @_));

  ${*$ftp}{'net_ftp_allo'} = join(" ", $size, @_);

  $oldval;
}


sub abort {
  my $ftp = shift;

  send($ftp, pack("CCC", TELNET_IAC, TELNET_IP, TELNET_IAC), MSG_OOB);

  $ftp->command(pack("C", TELNET_DM) . "ABOR");

  ${*$ftp}{'net_ftp_dataconn'}->close()
    if defined ${*$ftp}{'net_ftp_dataconn'};

  $ftp->response();

  $ftp->status == CMD_OK;
}


sub get {
  my ($ftp, $remote, $local, $where) = @_;

  my ($loc, $len, $buf, $resp, $data);
  local *FD;

  my $localfd = ref($local) || ref(\$local) eq "GLOB";

  ($local = $remote) =~ s#^.*/##
    unless (defined $local);

  croak("Bad remote filename '$remote'\n")
    if $remote =~ /[\r\n]/s;

  ${*$ftp}{'net_ftp_rest'} = $where if defined $where;
  my $rest = ${*$ftp}{'net_ftp_rest'};

  delete ${*$ftp}{'net_ftp_port'};
  delete ${*$ftp}{'net_ftp_pasv'};

  $data = $ftp->retr($remote)
    or return;

  if ($localfd) {
    $loc = $local;
  }
  else {
    $loc = \*FD;

    unless (sysopen($loc, $local, O_CREAT | O_WRONLY | ($rest ? O_APPEND: O_TRUNC))) {
      carp "Cannot open Local file $local: $!\n";
      $data->abort;
      return;
    }
  }

  if ($ftp->type eq 'I' && !binmode($loc)) {
    carp "Cannot binmode Local file $local: $!\n";
    $data->abort;
    close($loc) unless $localfd;
    return;
  }

  $buf = '';
  my ($count, $hashh, $hashb, $ref) = (0);

  ($hashh, $hashb) = @$ref
    if ($ref = ${*$ftp}{'net_ftp_hash'});

  my $blksize = ${*$ftp}{'net_ftp_blksize'};
  local $\;    # Just in case

  while (1) {
    last unless $len = $data->read($buf, $blksize);

    if (EBCDIC && $ftp->type ne 'I') {
      $buf = $ftp->toebcdic($buf);
      $len = length($buf);
    }

    if ($hashh) {
      $count += $len;
      print $hashh "#" x (int($count / $hashb));
      $count %= $hashb;
    }
    unless (print $loc $buf) {
      carp "Cannot write to Local file $local: $!\n";
      $data->abort;
      close($loc)
        unless $localfd;
      return;
    }
  }

  print $hashh "\n" if $hashh;

  unless ($localfd) {
    unless (close($loc)) {
      carp "Cannot close file $local (perhaps disk space) $!\n";
      return;
    }
  }

  unless ($data->close())    # implied $ftp->response
  {
    carp "Unable to close datastream";
    return;
  }

  return $local;
}


sub cwd {
  @_ == 1 || @_ == 2 or croak 'usage: $ftp->cwd([$dir])';

  my ($ftp, $dir) = @_;

  $dir = "/" unless defined($dir) && $dir =~ /\S/;

  $dir eq ".."
    ? $ftp->_CDUP()
    : $ftp->_CWD($dir);
}


sub cdup {
  @_ == 1 or croak 'usage: $ftp->cdup()';
  $_[0]->_CDUP;
}


sub pwd {
  @_ == 1 || croak 'usage: $ftp->pwd()';
  my $ftp = shift;

  $ftp->_PWD();
  $ftp->_extract_path;
}

# rmdir( $ftp, $dir, [ $recurse ] )
#
# Removes $dir on remote host via FTP.
# $ftp is handle for remote host
#
# If $recurse is TRUE, the directory and deleted recursively.
# This means all of its contents and subdirectories.
#
# Initial version contributed by Dinkum Software
#
sub rmdir {
  @_ == 2 || @_ == 3 or croak('usage: $ftp->rmdir($dir[, $recurse])');

  # Pick off the args
  my ($ftp, $dir, $recurse) = @_;
  my $ok;

  return $ok
    if $ok = $ftp->_RMD($dir)
    or !$recurse;

  # Try to delete the contents
  # Get a list of all the files in the directory, excluding the current and parent directories
  my @filelist = map { /^(?:\S+;)+ (.+)$/ ? ($1) : () } grep { !/^(?:\S+;)*type=[cp]dir;/i } $ftp->_list_cmd("MLSD", $dir);

  # Fallback to using the less well-defined NLST command if MLSD fails
  @filelist = grep { !/^\.{1,2}$/ } $ftp->ls($dir)
    unless @filelist;

  return
    unless @filelist;    # failed, it is probably not a directory

  return $ftp->delete($dir)
    if @filelist == 1 and $dir eq $filelist[0];

  # Go thru and delete each file or the directory
  foreach my $file (map { m,/, ? $_ : "$dir/$_" } @filelist) {
    next                 # successfully deleted the file
      if $ftp->delete($file);

    # Failed to delete it, assume its a directory
    # Recurse and ignore errors, the final rmdir() will
    # fail on any errors here
    return $ok
      unless $ok = $ftp->rmdir($file, 1);
  }

  # Directory should be empty
  # Try to remove the directory again
  # Pass results directly to caller
  # If any of the prior deletes failed, this
  # rmdir() will fail because directory is not empty
  return $ftp->_RMD($dir);
}


sub restart {
  @_ == 2 || croak 'usage: $ftp->restart($where)';

  my ($ftp, $where) = @_;

  ${*$ftp}{'net_ftp_rest'} = $where;

  return;
}


sub mkdir {
  @_ == 2 || @_ == 3 or croak 'usage: $ftp->mkdir($dir[, $recurse])';

  my ($ftp, $dir, $recurse) = @_;

  $ftp->_MKD($dir) || $recurse
    or return;

  my $path = $dir;

  unless ($ftp->ok) {
    my @path = split(m#(?=/+)#, $dir);

    $path = "";

    while (@path) {
      $path .= shift @path;

      $ftp->_MKD($path);

      $path = $ftp->_extract_path($path);
    }

    # If the creation of the last element was not successful, see if we
    # can cd to it, if so then return path

    unless ($ftp->ok) {
      my ($status, $message) = ($ftp->status, $ftp->message);
      my $pwd = $ftp->pwd;

      if ($pwd && $ftp->cwd($dir)) {
        $path = $dir;
        $ftp->cwd($pwd);
      }
      else {
        undef $path;
      }
      $ftp->set_status($status, $message);
    }
  }

  $path;
}


sub delete {
  @_ == 2 || croak 'usage: $ftp->delete($filename)';

  $_[0]->_DELE($_[1]);
}


sub put        { shift->_store_cmd("stor", @_) }
sub put_unique { shift->_store_cmd("stou", @_) }
sub append     { shift->_store_cmd("appe", @_) }


sub nlst { shift->_data_cmd("NLST", @_) }
sub list { shift->_data_cmd("LIST", @_) }
sub retr { shift->_data_cmd("RETR", @_) }
sub stor { shift->_data_cmd("STOR", @_) }
sub stou { shift->_data_cmd("STOU", @_) }
sub appe { shift->_data_cmd("APPE", @_) }


sub _store_cmd {
  my ($ftp, $cmd, $local, $remote) = @_;
  my ($loc, $sock, $len, $buf);
  local *FD;

  my $localfd = ref($local) || ref(\$local) eq "GLOB";

  if (!defined($remote) and 'STOU' ne uc($cmd)) {
    croak 'Must specify remote filename with stream input'
      if $localfd;

    require File::Basename;
    $remote = File::Basename::basename($local);
  }
  if (defined ${*$ftp}{'net_ftp_allo'}) {
    delete ${*$ftp}{'net_ftp_allo'};
  }
  else {

    # if the user hasn't already invoked the alloc method since the last
    # _store_cmd call, figure out if the local file is a regular file(not
    # a pipe, or device) and if so get the file size from stat, and send
    # an ALLO command before sending the STOR, STOU, or APPE command.
    my $size = do { local $^W; -f $local && -s _ };    # no ALLO if sending data from a pipe
    ${*$ftp}{'net_ftp_allo'} = $size if $size;
  }
  croak("Bad remote filename '$remote'\n")
    if defined($remote) and $remote =~ /[\r\n]/s;

  if ($localfd) {
    $loc = $local;
  }
  else {
    $loc = \*FD;

    unless (sysopen($loc, $local, O_RDONLY)) {
      carp "Cannot open Local file $local: $!\n";
      return;
    }
  }

  if ($ftp->type eq 'I' && !binmode($loc)) {
    carp "Cannot binmode Local file $local: $!\n";
    return;
  }

  delete ${*$ftp}{'net_ftp_port'};
  delete ${*$ftp}{'net_ftp_pasv'};

  $sock = $ftp->_data_cmd($cmd, grep { defined } $remote)
    or return;

  $remote = ($ftp->message =~ /\w+\s*:\s*(.*)/)[0]
    if 'STOU' eq uc $cmd;

  my $blksize = ${*$ftp}{'net_ftp_blksize'};

  my ($count, $hashh, $hashb, $ref) = (0);

  ($hashh, $hashb) = @$ref
    if ($ref = ${*$ftp}{'net_ftp_hash'});

  while (1) {
    last unless $len = read($loc, $buf = "", $blksize);

    if (EBCDIC && $ftp->type ne 'I') {
      $buf = $ftp->toascii($buf);
      $len = length($buf);
    }

    if ($hashh) {
      $count += $len;
      print $hashh "#" x (int($count / $hashb));
      $count %= $hashb;
    }

    my $wlen;
    unless (defined($wlen = $sock->write($buf, $len)) && $wlen == $len) {
      $sock->abort;
      close($loc)
        unless $localfd;
      print $hashh "\n" if $hashh;
      return;
    }
  }

  print $hashh "\n" if $hashh;

  close($loc)
    unless $localfd;

  $sock->close()
    or return;

  if ('STOU' eq uc $cmd and $ftp->message =~ m/unique\s+file\s*name\s*:\s*(.*)\)|"(.*)"/) {
    require File::Basename;
    $remote = File::Basename::basename($+);
  }

  return $remote;
}


sub port {
    @_ == 1 || @_ == 2 or croak 'usage: $self->port([$port])';
    return _eprt('PORT',@_);
}

sub eprt {
  @_ == 1 || @_ == 2 or croak 'usage: $self->eprt([$port])';
  return _eprt('EPRT',@_);
}

sub _eprt {
  my ($cmd,$ftp,$port) = @_;
  delete ${*$ftp}{net_ftp_intern_port};
  unless ($port) {
    my $listen = ${*$ftp}{net_ftp_listen} ||= $IOCLASS->new(
      Listen    => 1,
      Timeout   => $ftp->timeout,
      LocalAddr => $ftp->sockhost,
      $family_key  => $ftp->sockdomain,
      can_ssl() ? (
        %{ ${*$ftp}{net_ftp_tlsargs} },
        SSL_startHandshake => 0,
      ):(),
    );
    ${*$ftp}{net_ftp_intern_port} = 1;
    my $fam = ($listen->sockdomain == AF_INET) ? 1:2;
    if ( $cmd eq 'EPRT' || $fam == 2 ) {
      $port = "|$fam|".$listen->sockhost."|".$listen->sockport."|";
      $cmd = 'EPRT';
    } else {
      my $p = $listen->sockport;
      $port = join(',',split(m{\.},$listen->sockhost),$p >> 8,$p & 0xff);
    }
  } elsif (ref($port) eq 'ARRAY') {
    $port = join(',',split(m{\.},@$port[0]),@$port[1] >> 8,@$port[1] & 0xff);
  }
  my $ok = $cmd eq 'EPRT' ? $ftp->_EPRT($port) : $ftp->_PORT($port);
  ${*$ftp}{net_ftp_port} = $port if $ok;
  return $ok;
}


sub ls  { shift->_list_cmd("NLST", @_); }
sub dir { shift->_list_cmd("LIST", @_); }


sub pasv {
  my $ftp = shift;
  @_ and croak 'usage: $ftp->port()';
  return $ftp->epsv if $ftp->sockdomain != AF_INET;
  delete ${*$ftp}{net_ftp_intern_port};

  if ( $ftp->_PASV &&
    $ftp->message =~ m{(\d+,\d+,\d+,\d+),(\d+),(\d+)} ) {
    my $port = 256 * $2 + $3;
    ( my $ip = $1 ) =~s{,}{.}g;
    return ${*$ftp}{net_ftp_pasv} = [ $ip,$port ];
  }
  return;
}

sub epsv {
  my $ftp = shift;
  @_ and croak 'usage: $ftp->epsv()';
  delete ${*$ftp}{net_ftp_intern_port};

  $ftp->_EPSV && $ftp->message =~ m{\(([\x33-\x7e])\1\1(\d+)\1\)}
    ? ${*$ftp}{net_ftp_pasv} = [ $ftp->peerhost, $2 ]
    : undef;
}


sub unique_name {
  my $ftp = shift;
  ${*$ftp}{'net_ftp_unique'} || undef;
}


sub supported {
  @_ == 2 or croak 'usage: $ftp->supported($cmd)';
  my $ftp  = shift;
  my $cmd  = uc shift;
  my $hash = ${*$ftp}{'net_ftp_supported'} ||= {};

  return $hash->{$cmd}
    if exists $hash->{$cmd};

  return $hash->{$cmd} = 1
    if $ftp->feature($cmd);

  return $hash->{$cmd} = 0
    unless $ftp->_HELP($cmd);

  my $text = $ftp->message;
  if ($text =~ /following.+commands/i) {
    $text =~ s/^.*\n//;
    while ($text =~ /(\*?)(\w+)(\*?)/sg) {
      $hash->{"\U$2"} = !length("$1$3");
    }
  }
  else {
    $hash->{$cmd} = $text !~ /unimplemented/i;
  }

  $hash->{$cmd} ||= 0;
}

##
## Deprecated methods
##


sub lsl {
  carp "Use of Net::FTP::lsl deprecated, use 'dir'"
    if $^W;
  goto &dir;
}


sub authorise {
  carp "Use of Net::FTP::authorise deprecated, use 'authorize'"
    if $^W;
  goto &authorize;
}


##
## Private methods
##


sub _extract_path {
  my ($ftp, $path) = @_;

  # This tries to work both with and without the quote doubling
  # convention (RFC 959 requires it, but the first 3 servers I checked
  # didn't implement it).  It will fail on a server which uses a quote in
  # the message which isn't a part of or surrounding the path.
  $ftp->ok
    && $ftp->message =~ /(?:^|\s)\"(.*)\"(?:$|\s)/
    && ($path = $1) =~ s/\"\"/\"/g;

  $path;
}

##
## Communication methods
##


sub _dataconn {
  my $ftp = shift;
  my $pkg = "Net::FTP::" . $ftp->type;
  eval "require " . $pkg ## no critic (BuiltinFunctions::ProhibitStringyEval)
    or croak("cannot load $pkg required for type ".$ftp->type);
  $pkg =~ s/ /_/g;
  delete ${*$ftp}{net_ftp_dataconn};

  my $conn;
  my $pasv = ${*$ftp}{net_ftp_pasv};
  if ($pasv) {
    $conn = $pkg->new(
      PeerAddr  => $pasv->[0],
      PeerPort  => $pasv->[1],
      LocalAddr => ${*$ftp}{net_ftp_localaddr},
      $family_key => ${*$ftp}{net_ftp_domain},
      Timeout   => $ftp->timeout,
      can_ssl() ? (
        SSL_startHandshake => 0,
        %{${*$ftp}{net_ftp_tlsargs}},
      ):(),
    ) or return;
  } elsif (my $listen =  delete ${*$ftp}{net_ftp_listen}) {
    $conn = $listen->accept($pkg) or return;
    $conn->timeout($ftp->timeout);
    close($listen);
  } else {
    croak("no listener in active mode");
  }

  if (( ${*$ftp}{net_ftp_tlsprot} || '') eq 'P') {
    if ($conn->connect_SSL) {
      # SSL handshake ok
    } else {
      carp("failed to ssl upgrade dataconn: $IO::Socket::SSL::SSL_ERROR");
      return;
    }
  }

  ${*$ftp}{net_ftp_dataconn} = $conn;
  ${*$conn} = "";
  ${*$conn}{net_ftp_cmd} = $ftp;
  ${*$conn}{net_ftp_blksize} = ${*$ftp}{net_ftp_blksize};
  return $conn;
}


sub _list_cmd {
  my $ftp = shift;
  my $cmd = uc shift;

  delete ${*$ftp}{'net_ftp_port'};
  delete ${*$ftp}{'net_ftp_pasv'};

  my $data = $ftp->_data_cmd($cmd, @_);

  return
    unless (defined $data);

  require Net::FTP::A;
  bless $data, "Net::FTP::A";    # Force ASCII mode

  my $databuf = '';
  my $buf     = '';
  my $blksize = ${*$ftp}{'net_ftp_blksize'};

  while ($data->read($databuf, $blksize)) {
    $buf .= $databuf;
  }

  my $list = [split(/\n/, $buf)];

  $data->close();

  if (EBCDIC) {
    for (@$list) { $_ = $ftp->toebcdic($_) }
  }

  wantarray
    ? @{$list}
    : $list;
}


sub _data_cmd {
  my $ftp   = shift;
  my $cmd   = uc shift;
  my $ok    = 1;
  my $where = delete ${*$ftp}{'net_ftp_rest'} || 0;
  my $arg;

  for my $arg (@_) {
    croak("Bad argument '$arg'\n")
      if $arg =~ /[\r\n]/s;
  }

  if ( ${*$ftp}{'net_ftp_passive'}
    && !defined ${*$ftp}{'net_ftp_pasv'}
    && !defined ${*$ftp}{'net_ftp_port'})
  {
    return unless defined $ftp->pasv;

    if ($where and !$ftp->_REST($where)) {
      my ($status, $message) = ($ftp->status, $ftp->message);
      $ftp->abort;
      $ftp->set_status($status, $message);
      return;
    }

    # first send command, then open data connection
    # otherwise the peer might not do a full accept (with SSL
    # handshake if PROT P)
    $ftp->command($cmd, @_);
    my $data = $ftp->_dataconn();
    if (CMD_INFO == $ftp->response()) {
      $data->reading
        if $data && $cmd =~ /RETR|LIST|NLST|MLSD/;
      return $data;
    }
    $data->_close if $data;

    return;
  }

  $ok = $ftp->port
    unless (defined ${*$ftp}{'net_ftp_port'}
    || defined ${*$ftp}{'net_ftp_pasv'});

  $ok = $ftp->_REST($where)
    if $ok && $where;

  return
    unless $ok;

  if ($cmd =~ /(STOR|APPE|STOU)/ and exists ${*$ftp}{net_ftp_allo} and
      $ftp->supported("ALLO"))
  {
    $ftp->_ALLO(delete ${*$ftp}{net_ftp_allo})
      or return;
  }

  $ftp->command($cmd, @_);

  return 1
    if (defined ${*$ftp}{'net_ftp_pasv'});

  $ok = CMD_INFO == $ftp->response();

  return $ok
    unless exists ${*$ftp}{'net_ftp_intern_port'};

  if ($ok) {
    my $data = $ftp->_dataconn();

    $data->reading
      if $data && $cmd =~ /RETR|LIST|NLST|MLSD/;

    return $data;
  }


  close(delete ${*$ftp}{'net_ftp_listen'});

  return;
}

##
## Over-ride methods (Net::Cmd)
##


sub debug_text { $_[2] =~ /^(pass|resp|acct)/i ? "$1 ....\n" : $_[2]; }


sub command {
  my $ftp = shift;

  delete ${*$ftp}{'net_ftp_port'};
  $ftp->SUPER::command(@_);
}


sub response {
  my $ftp  = shift;
  my $code = $ftp->SUPER::response() || 5;    # assume 500 if undef

  delete ${*$ftp}{'net_ftp_pasv'}
    if ($code != CMD_MORE && $code != CMD_INFO);

  $code;
}


sub parse_response {
  return ($1, $2 eq "-")
    if $_[1] =~ s/^(\d\d\d)([- ]?)//o;

  my $ftp = shift;

  # Darn MS FTP server is a load of CRAP !!!!
  # Expect to see undef here.
  return ()
    unless 0 + (${*$ftp}{'net_cmd_code'} || 0);

  (${*$ftp}{'net_cmd_code'}, 1);
}

##
## Allow 2 servers to talk directly
##


sub pasv_xfer_unique {
  my ($sftp, $sfile, $dftp, $dfile) = @_;
  $sftp->pasv_xfer($sfile, $dftp, $dfile, 1);
}


sub pasv_xfer {
  my ($sftp, $sfile, $dftp, $dfile, $unique) = @_;

  ($dfile = $sfile) =~ s#.*/##
    unless (defined $dfile);

  my $port = $sftp->pasv
    or return;

  $dftp->port($port)
    or return;

  return
    unless ($unique ? $dftp->stou($dfile) : $dftp->stor($dfile));

  unless ($sftp->retr($sfile) && $sftp->response == CMD_INFO) {
    $sftp->retr($sfile);
    $dftp->abort;
    $dftp->response();
    return;
  }

  $dftp->pasv_wait($sftp);
}


sub pasv_wait {
  @_ == 2 or croak 'usage: $ftp->pasv_wait($non_pasv_server)';

  my ($ftp, $non_pasv_server) = @_;
  my ($file, $rin, $rout);

  vec($rin = '', fileno($ftp), 1) = 1;
  select($rout = $rin, undef, undef, undef);

  my $dres = $ftp->response();
  my $sres = $non_pasv_server->response();

  return
    unless $dres == CMD_OK && $sres == CMD_OK;

  return
    unless $ftp->ok() && $non_pasv_server->ok();

  return $1
    if $ftp->message =~ /unique file name:\s*(\S*)\s*\)/;

  return $1
    if $non_pasv_server->message =~ /unique file name:\s*(\S*)\s*\)/;

  return 1;
}


sub feature {
  @_ == 2 or croak 'usage: $ftp->feature($name)';
  my ($ftp, $name) = @_;

  my $feature = ${*$ftp}{net_ftp_feature} ||= do {
    my @feat;

    # Example response
    # 211-Features:
    #  MDTM
    #  REST STREAM
    #  SIZE
    # 211 End

    @feat = map { /^\s+(.*\S)/ } $ftp->message
      if $ftp->_FEAT;

    \@feat;
  };

  return grep { /^\Q$name\E\b/i } @$feature;
}


sub cmd { shift->command(@_)->response() }

########################################
#
# RFC959 + RFC2428 + RFC4217 commands
#


sub _ABOR { shift->command("ABOR")->response() == CMD_OK }
sub _ALLO { shift->command("ALLO", @_)->response() == CMD_OK }
sub _CDUP { shift->command("CDUP")->response() == CMD_OK }
sub _NOOP { shift->command("NOOP")->response() == CMD_OK }
sub _PASV { shift->command("PASV")->response() == CMD_OK }
sub _QUIT { shift->command("QUIT")->response() == CMD_OK }
sub _DELE { shift->command("DELE", @_)->response() == CMD_OK }
sub _CWD  { shift->command("CWD", @_)->response() == CMD_OK }
sub _PORT { shift->command("PORT", @_)->response() == CMD_OK }
sub _RMD  { shift->command("RMD", @_)->response() == CMD_OK }
sub _MKD  { shift->command("MKD", @_)->response() == CMD_OK }
sub _PWD  { shift->command("PWD", @_)->response() == CMD_OK }
sub _TYPE { shift->command("TYPE", @_)->response() == CMD_OK }
sub _RNTO { shift->command("RNTO", @_)->response() == CMD_OK }
sub _RESP { shift->command("RESP", @_)->response() == CMD_OK }
sub _MDTM { shift->command("MDTM", @_)->response() == CMD_OK }
sub _SIZE { shift->command("SIZE", @_)->response() == CMD_OK }
sub _HELP { shift->command("HELP", @_)->response() == CMD_OK }
sub _STAT { shift->command("STAT", @_)->response() == CMD_OK }
sub _FEAT { shift->command("FEAT", @_)->response() == CMD_OK }
sub _PBSZ { shift->command("PBSZ", @_)->response() == CMD_OK }
sub _PROT { shift->command("PROT", @_)->response() == CMD_OK }
sub _CCC  { shift->command("CCC", @_)->response() == CMD_OK }
sub _EPRT { shift->command("EPRT", @_)->response() == CMD_OK }
sub _EPSV { shift->command("EPSV", @_)->response() == CMD_OK }
sub _APPE { shift->command("APPE", @_)->response() == CMD_INFO }
sub _LIST { shift->command("LIST", @_)->response() == CMD_INFO }
sub _NLST { shift->command("NLST", @_)->response() == CMD_INFO }
sub _RETR { shift->command("RETR", @_)->response() == CMD_INFO }
sub _STOR { shift->command("STOR", @_)->response() == CMD_INFO }
sub _STOU { shift->command("STOU", @_)->response() == CMD_INFO }
sub _RNFR { shift->command("RNFR", @_)->response() == CMD_MORE }
sub _REST { shift->command("REST", @_)->response() == CMD_MORE }
sub _PASS { shift->command("PASS", @_)->response() }
sub _ACCT { shift->command("ACCT", @_)->response() }
sub _AUTH { shift->command("AUTH", @_)->response() }


sub _USER {
  my $ftp = shift;
  my $ok  = $ftp->command("USER", @_)->response();

  # A certain brain dead firewall :-)
  $ok = $ftp->command("user", @_)->response()
    unless $ok == CMD_MORE or $ok == CMD_OK;

  $ok;
}


sub _SMNT { shift->unsupported(@_) }
sub _MODE { shift->unsupported(@_) }
sub _SYST { shift->unsupported(@_) }
sub _STRU { shift->unsupported(@_) }
sub _REIN { shift->unsupported(@_) }


1;

__END__

=head1 NAME

Net::FTP - FTP Client class

=head1 SYNOPSIS

    use Net::FTP;

    $ftp = Net::FTP->new("some.host.name", Debug => 0)
      or die "Cannot connect to some.host.name: $@";

    $ftp->login("anonymous",'-anonymous@')
      or die "Cannot login ", $ftp->message;

    $ftp->cwd("/pub")
      or die "Cannot change working directory ", $ftp->message;

    $ftp->get("that.file")
      or die "get failed ", $ftp->message;

    $ftp->quit;

=head1 DESCRIPTION

C<Net::FTP> is a class implementing a simple FTP client in Perl as
described in RFC959.  It provides wrappers for the commonly used subset of the
RFC959 commands.
If L<IO::Socket::IP> or L<IO::Socket::INET6> is installed it also provides
support for IPv6 as defined in RFC2428.
And with L<IO::Socket::SSL> installed it provides support for implicit FTPS
and explicit FTPS as defined in RFC4217.

The Net::FTP class is a subclass of Net::Cmd and (depending on avaibility) of
IO::Socket::IP, IO::Socket::INET6 or IO::Socket::INET.

=head2 Overview

FTP stands for File Transfer Protocol.  It is a way of transferring
files between networked machines.  The protocol defines a client
(whose commands are provided by this module) and a server (not
implemented in this module).  Communication is always initiated by the
client, and the server responds with a message and a status code (and
sometimes with data).

The FTP protocol allows files to be sent to or fetched from the
server.  Each transfer involves a B<local file> (on the client) and a
B<remote file> (on the server).  In this module, the same file name
will be used for both local and remote if only one is specified.  This
means that transferring remote file C</path/to/file> will try to put
that file in C</path/to/file> locally, unless you specify a local file
name.

The protocol also defines several standard B<translations> which the
file can undergo during transfer.  These are ASCII, EBCDIC, binary,
and byte.  ASCII is the default type, and indicates that the sender of
files will translate the ends of lines to a standard representation
which the receiver will then translate back into their local
representation.  EBCDIC indicates the file being transferred is in
EBCDIC format.  Binary (also known as image) format sends the data as
a contiguous bit stream.  Byte format transfers the data as bytes, the
values of which remain the same regardless of differences in byte size
between the two machines (in theory - in practice you should only use
this if you really know what you're doing).  This class does not support
the EBCDIC or byte formats, and will default to binary instead if they
are attempted.

=head2 Class Methods

=over 4

=item C<new([$host][, %options])>

This is the constructor for a new Net::FTP object. C<$host> is the
name of the remote host to which an FTP connection is required.

C<$host> is optional. If C<$host> is not given then it may instead be
passed as the C<Host> option described below. 

C<%options> are passed in a hash like fashion, using key and value pairs.
Possible options are:

B<Host> - FTP host to connect to. It may be a single scalar, as defined for
the C<PeerAddr> option in L<IO::Socket::INET>, or a reference to
an array with hosts to try in turn. The L</host> method will return the value
which was used to connect to the host.

B<Firewall> - The name of a machine which acts as an FTP firewall. This can be
overridden by an environment variable C<FTP_FIREWALL>. If specified, and the
given host cannot be directly connected to, then the
connection is made to the firewall machine and the string C<@hostname> is
appended to the login identifier. This kind of setup is also referred to
as an ftp proxy.

B<FirewallType> - The type of firewall running on the machine indicated by
B<Firewall>. This can be overridden by an environment variable
C<FTP_FIREWALL_TYPE>. For a list of permissible types, see the description of
ftp_firewall_type in L<Net::Config>.

B<BlockSize> - This is the block size that Net::FTP will use when doing
transfers. (defaults to 10240)

B<Port> - The port number to connect to on the remote machine for the
FTP connection

B<SSL> - If the connection should be done from start with SSL, contrary to later
upgrade with C<starttls>.

B<SSL_*> - SSL arguments which will be applied when upgrading the control or
data connection to SSL. You can use SSL arguments as documented in
L<IO::Socket::SSL>, but it will usually use the right arguments already.

B<Timeout> - Set a timeout value in seconds (defaults to 120)

B<Debug> - debug level (see the debug method in L<Net::Cmd>)

B<Passive> - If set to a non-zero value then all data transfers will
be done using passive mode. If set to zero then data transfers will be
done using active mode.  If the machine is connected to the Internet
directly, both passive and active mode should work equally well.
Behind most firewall and NAT configurations passive mode has a better
chance of working.  However, in some rare firewall configurations,
active mode actually works when passive mode doesn't.  Some really old
FTP servers might not implement passive transfers.  If not specified,
then the transfer mode is set by the environment variable
C<FTP_PASSIVE> or if that one is not set by the settings done by the
F<libnetcfg> utility.  If none of these apply then passive mode is
used.

B<Hash> - If given a reference to a file handle (e.g., C<\*STDERR>),
print hash marks (#) on that filehandle every 1024 bytes.  This
simply invokes the C<hash()> method for you, so that hash marks
are displayed for all transfers.  You can, of course, call C<hash()>
explicitly whenever you'd like.

B<LocalAddr> - Local address to use for all socket connections. This
argument will be passed to the super class, i.e. L<IO::Socket::INET>
or L<IO::Socket::IP>.

B<Domain> - Domain to use, i.e. AF_INET or AF_INET6. This
argument will be passed to the IO::Socket super class.
This can be used to enforce IPv4 even with L<IO::Socket::IP>
which would default to IPv6.
B<Family> is accepted as alternative name for B<Domain>.

If the constructor fails undef will be returned and an error message will
be in $@

=back

=head2 Object Methods

Unless otherwise stated all methods return either a I<true> or I<false>
value, with I<true> meaning that the operation was a success. When a method
states that it returns a value, failure will be returned as I<undef> or an
empty list.

C<Net::FTP> inherits from C<Net::Cmd> so methods defined in C<Net::Cmd> may
be used to send commands to the remote FTP server in addition to the methods
documented here.

=over 4

=item C<login([$login[, $password[, $account]]])>

Log into the remote FTP server with the given login information. If
no arguments are given then the C<Net::FTP> uses the C<Net::Netrc>
package to lookup the login information for the connected host.
If no information is found then a login of I<anonymous> is used.
If no password is given and the login is I<anonymous> then I<anonymous@>
will be used for password.

If the connection is via a firewall then the C<authorize> method will
be called with no arguments.

=item C<starttls()>

Upgrade existing plain connection to SSL.
The SSL arguments have to be given in C<new> already because they are needed for
data connections too.

=item C<stoptls()>

Downgrade existing SSL connection back to plain.
This is needed to work with some FTP helpers at firewalls, which need to see the
PORT and PASV commands and responses to dynamically open the necessary ports.
In this case C<starttls> is usually only done to protect the authorization.

=item C<prot($level)>

Set what type of data channel protection the client and server will be using.
Only C<$level>s "C" (clear) and "P" (private) are supported.

=item C<host()>

Returns the value used by the constructor, and passed to the IO::Socket super
class to connect to the host.

=item C<account($acct)>

Set a string identifying the user's account.

=item C<authorize([$auth[, $resp]])>

This is a protocol used by some firewall ftp proxies. It is used
to authorise the user to send data out.  If both arguments are not specified
then C<authorize> uses C<Net::Netrc> to do a lookup.

=item C<site($args)>

Send a SITE command to the remote server and wait for a response.

Returns most significant digit of the response code.

=item C<ascii()>

Transfer file in ASCII. CRLF translation will be done if required

=item C<binary()>

Transfer file in binary mode. No transformation will be done.

B<Hint>: If both server and client machines use the same line ending for
text files, then it will be faster to transfer all files in binary mode.

=item C<type([$type])>

Set or get if files will be transferred in ASCII or binary mode.

=item C<rename($oldname, $newname)>

Rename a file on the remote FTP server from C<$oldname> to C<$newname>. This
is done by sending the RNFR and RNTO commands.

=item C<delete($filename)>

Send a request to the server to delete C<$filename>.

=item C<cwd([$dir])>

Attempt to change directory to the directory given in C<$dir>.  If
C<$dir> is C<"..">, the FTP C<CDUP> command is used to attempt to
move up one directory. If no directory is given then an attempt is made
to change the directory to the root directory.

=item C<cdup()>

Change directory to the parent of the current directory.

=item C<passive([$passive])>

Set or get if data connections will be initiated in passive mode.

=item C<pwd()>

Returns the full pathname of the current directory.

=item C<restart($where)>

Set the byte offset at which to begin the next data transfer. Net::FTP simply
records this value and uses it when during the next data transfer. For this
reason this method will not return an error, but setting it may cause
a subsequent data transfer to fail.

=item C<rmdir($dir[, $recurse])>

Remove the directory with the name C<$dir>. If C<$recurse> is I<true> then
C<rmdir> will attempt to delete everything inside the directory.

=item C<mkdir($dir[, $recurse])>

Create a new directory with the name C<$dir>. If C<$recurse> is I<true> then
C<mkdir> will attempt to create all the directories in the given path.

Returns the full pathname to the new directory.

=item C<alloc($size[, $record_size])>

The alloc command allows you to give the ftp server a hint about the size
of the file about to be transferred using the ALLO ftp command. Some storage
systems use this to make intelligent decisions about how to store the file.
The C<$size> argument represents the size of the file in bytes. The
C<$record_size> argument indicates a maximum record or page size for files
sent with a record or page structure.

The size of the file will be determined, and sent to the server
automatically for normal files so that this method need only be called if
you are transferring data from a socket, named pipe, or other stream not
associated with a normal file.

=item C<ls([$dir])>

Get a directory listing of C<$dir>, or the current directory.

In an array context, returns a list of lines returned from the server. In
a scalar context, returns a reference to a list.

=item C<dir([$dir])>

Get a directory listing of C<$dir>, or the current directory in long format.

In an array context, returns a list of lines returned from the server. In
a scalar context, returns a reference to a list.

=item C<get($remote_file[, $local_file[, $where]])>

Get C<$remote_file> from the server and store locally. C<$local_file> may be
a filename or a filehandle. If not specified, the file will be stored in
the current directory with the same leafname as the remote file.

If C<$where> is given then the first C<$where> bytes of the file will
not be transferred, and the remaining bytes will be appended to
the local file if it already exists.

Returns C<$local_file>, or the generated local file name if C<$local_file>
is not given. If an error was encountered undef is returned.

=item C<put($local_file[, $remote_file])>

Put a file on the remote server. C<$local_file> may be a name or a filehandle.
If C<$local_file> is a filehandle then C<$remote_file> must be specified. If
C<$remote_file> is not specified then the file will be stored in the current
directory with the same leafname as C<$local_file>.

Returns C<$remote_file>, or the generated remote filename if C<$remote_file>
is not given.

B<NOTE>: If for some reason the transfer does not complete and an error is
returned then the contents that had been transferred will not be remove
automatically.

=item C<put_unique($local_file[, $remote_file])>

Same as put but uses the C<STOU> command.

Returns the name of the file on the server.

=item C<append($local_file[, $remote_file])>

Same as put but appends to the file on the remote server.

Returns C<$remote_file>, or the generated remote filename if C<$remote_file>
is not given.

=item C<unique_name()>

Returns the name of the last file stored on the server using the
C<STOU> command.

=item C<mdtm($file)>

Returns the I<modification time> of the given file

=item C<size($file)>

Returns the size in bytes for the given file as stored on the remote server.

B<NOTE>: The size reported is the size of the stored file on the remote server.
If the file is subsequently transferred from the server in ASCII mode
and the remote server and local machine have different ideas about
"End Of Line" then the size of file on the local machine after transfer
may be different.

=item C<supported($cmd)>

Returns TRUE if the remote server supports the given command.

=item C<hash([$filehandle_glob_ref[, $bytes_per_hash_mark]])>

Called without parameters, or with the first argument false, hash marks
are suppressed.  If the first argument is true but not a reference to a 
file handle glob, then \*STDERR is used.  The second argument is the number
of bytes per hash mark printed, and defaults to 1024.  In all cases the
return value is a reference to an array of two:  the filehandle glob reference
and the bytes per hash mark.

=item C<feature($name)>

Determine if the server supports the specified feature. The return
value is a list of lines the server responded with to describe the
options that it supports for the given feature. If the feature is
unsupported then the empty list is returned.

  if ($ftp->feature( 'MDTM' )) {
    # Do something
  }

  if (grep { /\bTLS\b/ } $ftp->feature('AUTH')) {
    # Server supports TLS
  }

=back

The following methods can return different results depending on
how they are called. If the user explicitly calls either
of the C<pasv> or C<port> methods then these methods will
return a I<true> or I<false> value. If the user does not
call either of these methods then the result will be a
reference to a C<Net::FTP::dataconn> based object.

=over 4

=item C<nlst([$dir])>

Send an C<NLST> command to the server, with an optional parameter.

=item C<list([$dir])>

Same as C<nlst> but using the C<LIST> command

=item C<retr($file)>

Begin the retrieval of a file called C<$file> from the remote server.

=item C<stor($file)>

Tell the server that you wish to store a file. C<$file> is the
name of the new file that should be created.

=item C<stou($file)>

Same as C<stor> but using the C<STOU> command. The name of the unique
file which was created on the server will be available via the C<unique_name>
method after the data connection has been closed.

=item C<appe($file)>

Tell the server that we want to append some data to the end of a file
called C<$file>. If this file does not exist then create it.

=back

If for some reason you want to have complete control over the data connection,
this includes generating it and calling the C<response> method when required,
then the user can use these methods to do so.

However calling these methods only affects the use of the methods above that
can return a data connection. They have no effect on methods C<get>, C<put>,
C<put_unique> and those that do not require data connections.

=over 4

=item C<port([$port])>

=item C<eprt([$port])>

Send a C<PORT> (IPv4) or C<EPRT> (IPv6) command to the server. If C<$port> is
specified then it is sent to the server. If not, then a listen socket is created
and the correct information sent to the server.

=item C<pasv()>

=item C<epsv()>

Tell the server to go into passive mode (C<pasv> for IPv4, C<epsv> for IPv6).
Returns the text that represents the port on which the server is listening, this
text is in a suitable form to send to another ftp server using the C<port> or
C<eprt> method.

=back

The following methods can be used to transfer files between two remote
servers, providing that these two servers can connect directly to each other.

=over 4

=item C<pasv_xfer($src_file, $dest_server[, $dest_file ])>

This method will do a file transfer between two remote ftp servers. If
C<$dest_file> is omitted then the leaf name of C<$src_file> will be used.

=item C<pasv_xfer_unique($src_file, $dest_server[, $dest_file ])>

Like C<pasv_xfer> but the file is stored on the remote server using
the STOU command.

=item C<pasv_wait($non_pasv_server)>

This method can be used to wait for a transfer to complete between a passive
server and a non-passive server. The method should be called on the passive
server with the C<Net::FTP> object for the non-passive server passed as an
argument.

=item C<abort()>

Abort the current data transfer.

=item C<quit()>

Send the QUIT command to the remote FTP server and close the socket connection.

=back

=head2 Methods for the Adventurous

=over 4

=item C<quot($cmd[, $args])>

Send a command, that Net::FTP does not directly support, to the remote
server and wait for a response.

Returns most significant digit of the response code.

B<WARNING> This call should only be used on commands that do not require
data connections. Misuse of this method can hang the connection.

=item C<can_inet6()>

Returns whether we can use IPv6.

=item C<can_ssl()>

Returns whether we can use SSL.

=back

=head2 The dataconn Class

Some of the methods defined in C<Net::FTP> return an object which will
be derived from the C<Net::FTP::dataconn> class. See L<Net::FTP::dataconn> for
more details.

=head2 Unimplemented

The following RFC959 commands have not been implemented:

=over 4

=item C<SMNT>

Mount a different file system structure without changing login or
accounting information.

=item C<HELP>

Ask the server for "helpful information" (that's what the RFC says) on
the commands it accepts.

=item C<MODE>

Specifies transfer mode (stream, block or compressed) for file to be
transferred.

=item C<SYST>

Request remote server system identification.

=item C<STAT>

Request remote server status.

=item C<STRU>

Specifies file structure for file to be transferred.

=item C<REIN>

Reinitialize the connection, flushing all I/O and account information.

=back

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

See L<https://rt.cpan.org/Dist/Display.html?Status=Active&Queue=libnet>.

=head2 Reporting Bugs

When reporting bugs/problems please include as much information as possible.
It may be difficult for me to reproduce the problem as almost every setup
is different.

A small script which yields the problem will probably be of help. It would
also be useful if this script was run with the extra options C<< Debug => 1 >>
passed to the constructor, and the output sent with the bug report. If you
cannot include a small script then please include a Debug trace from a
run of your program which does yield the problem.

=head1 SEE ALSO

L<Net::Netrc>,
L<Net::Cmd>,
L<IO::Socket::SSL>;

L<ftp(1)>,
L<ftpd(8)>;

L<https://www.ietf.org/rfc/rfc959.txt>,
L<https://www.ietf.org/rfc/rfc2428.txt>,
L<https://www.ietf.org/rfc/rfc4217.txt>.

=head1 ACKNOWLEDGEMENTS

Henry Gabryjelski E<lt>L<henryg@WPI.EDU|mailto:henryg@WPI.EDU>E<gt> - for the
suggestion of creating directories recursively.

Nathan Torkington E<lt>L<gnat@frii.com|mailto:gnat@frii.com>E<gt> - for some
input on the documentation.

Roderick Schertler E<lt>L<roderick@gate.net|mailto:roderick@gate.net>E<gt> - for
various inputs

=head1 AUTHOR

Graham Barr E<lt>L<gbarr@pobox.com|mailto:gbarr@pobox.com>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
libnet as of version 1.22_02.

=head1 COPYRIGHT

Copyright (C) 1995-2004 Graham Barr.  All rights reserved.

Copyright (C) 2013-2017, 2020, 2022 Steve Hay.  All rights reserved.

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
