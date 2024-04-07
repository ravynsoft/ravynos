#!/usr/local/bin/perl

use Config;
use File::Basename qw(&basename &dirname);
use Cwd;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir dirname($0);
my $file = basename($0, '.PL');
$file .= '.com' if $^O eq 'VMS';

open OUT, ">", $file or die "Can't create $file: $!";

print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

print OUT <<"!GROK!THIS!";
$Config{startperl}
    eval 'exec $Config{perlpath} -S \$0 \${1+"\$@"}'
	if 0; # ^ Run only under a shell
!GROK!THIS!

# In the following, perl variables are not expanded during extraction.

print OUT <<'!NO!SUBS!';

=head1 NAME

libnetcfg - configure libnet

=head1 DESCRIPTION

The libnetcfg utility can be used to configure the libnet.
Starting from perl 5.8 libnet is part of the standard Perl
distribution, but the libnetcfg can be used for any libnet
installation.

=head1 USAGE

Without arguments libnetcfg displays the current configuration.

    $ libnetcfg
    # old config ./libnet.cfg
    daytime_hosts        ntp1.none.such
    ftp_int_passive      0
    ftp_testhost         ftp.funet.fi
    inet_domain          none.such
    nntp_hosts           nntp.none.such
    ph_hosts             
    pop3_hosts           pop.none.such
    smtp_hosts           smtp.none.such
    snpp_hosts           
    test_exist           1
    test_hosts           1
    time_hosts           ntp.none.such
    # libnetcfg -h for help
    $ 

It tells where the old configuration file was found (if found).

The C<-h> option will show a usage message.

To change the configuration you will need to use either the C<-c> or
the C<-d> options.

The default name of the old configuration file is by default
"libnet.cfg", unless otherwise specified using the -i option,
C<-i oldfile>, and it is searched first from the current directory,
and then from your module path.

The default name of the new configuration file is "libnet.cfg", and by
default it is written to the current directory, unless otherwise
specified using the -o option, C<-o newfile>.

=head1 SEE ALSO

L<Net::Config>, L<libnetFAQ>

=head1 AUTHORS

Graham Barr, the original Configure script of libnet.

Jarkko Hietaniemi, conversion into libnetcfg for inclusion into Perl 5.8.

=cut

# $Id: Configure,v 1.8 1997/03/04 09:22:32 gbarr Exp $

BEGIN { pop @INC if $INC[-1] eq '.' }
use strict;
use IO::File;
use Getopt::Std;
use ExtUtils::MakeMaker qw(prompt);
use File::Spec;

use vars qw($opt_d $opt_c $opt_h $opt_o $opt_i);

##
##
##

my %cfg = ();
my @cfg = ();

my($libnet_cfg_in,$libnet_cfg_out,$msg,$ans,$def,$have_old);

##
##
##

sub valid_host
{
 my $h = shift;

 defined($h) && (($cfg{'test_exist'} == 0) || gethostbyname($h));
}

##
##
##

sub test_hostnames (\@)
{
 my $hlist = shift;
 my @h = ();
 my $host;
 my $err = 0;

 foreach $host (@$hlist)
  {
   if(valid_host($host))
    {
     push(@h, $host);
     next;
    }
   warn "Bad hostname: '$host'\n";
   $err++;
  }
 @$hlist = @h;
 $err ? join(" ",@h) : undef;
}

##
##
##

sub Prompt
{
 my($prompt,$def) = @_;

 $def = "" unless defined $def;

 chomp($prompt);

 if($opt_d)
  {
   print $prompt,," [",$def,"]\n";
   return $def;
  }
 prompt($prompt,$def);
}

##
##
##

sub get_host_list
{
 my($prompt,$def) = @_;

 $def = join(" ",@$def) if ref($def);

 my @hosts;

 do
  {
   my $ans = Prompt($prompt,$def);

   $ans =~ s/(\A\s+|\s+\Z)//g;

   @hosts = split(/\s+/, $ans);
  }
 while(@hosts && defined($def = test_hostnames(@hosts)));

 \@hosts;
}

##
##
##

sub get_hostname
{
 my($prompt,$def) = @_;

 my $host;

 while(1)
  {
   my $ans = Prompt($prompt,$def);
   $host = ($ans =~ /(\S*)/)[0];
   last
	if(!length($host) || valid_host($host));

   $def =""
	if $def eq $host;

   print <<"EDQ";

*** ERROR:
    Hostname '$host' does not seem to exist, please enter again
    or a single space to clear any default

EDQ
  }

 length $host
	? $host
	: undef;
}

##
##
##

sub get_bool ($$)
{
 my($prompt,$def) = @_;

 chomp($prompt);

 my $val = Prompt($prompt,$def ? "yes" : "no");

 $val =~ /^y/i ? 1 : 0;
}

##
##
##

sub get_netmask ($$)
{
 my($prompt,$def) = @_;

 chomp($prompt);

 my %list;
 @list{@$def} = ();

MASK:
 while(1) {
   my $bad = 0;
   my $ans = Prompt($prompt) or last;

   if($ans eq '*') {
     %list = ();
     next;
   }

   if($ans eq '=') {
     print "\n",( %list ? join("\n", sort keys %list) : 'none'),"\n\n";
     next;
   }

   unless ($ans =~ m{^\s*(?:(-?\s*)(\d+(?:\.\d+){0,3})/(\d+))}) {
     warn "Bad netmask '$ans'\n";
     next;
   }

   my($remove,$bits,@ip) = ($1,$3,split(/\./, $2),0,0,0);
   if ( $ip[0] < 1 || $bits < 1 || $bits > 32) {
     warn "Bad netmask '$ans'\n";
     next MASK;
   }
   foreach my $byte (@ip) {
     if ( $byte > 255 ) {
       warn "Bad netmask '$ans'\n";
       next MASK;
     }
   } 

   my $mask = sprintf("%d.%d.%d.%d/%d",@ip[0..3],$bits); 

   if ($remove) {
     delete $list{$mask};
   }
   else {
     $list{$mask} = 1;
   }

  }

 [ keys %list ];
}

##
##
##

sub default_hostname
{
 my $host;
 my @host;

 foreach $host (@_)
  {
   if(defined($host) && valid_host($host))
    {
     return $host
	unless wantarray;
     push(@host,$host);
    }
  }

 return wantarray ? @host : undef;
}

##
##
##

getopts('dcho:i:');

$libnet_cfg_in = "libnet.cfg"
	unless(defined($libnet_cfg_in  = $opt_i));

$libnet_cfg_out = "libnet.cfg"
	unless(defined($libnet_cfg_out = $opt_o));

my %oldcfg = ();

$Net::Config::CONFIGURE = 1; # Suppress load of user overrides
if( -f $libnet_cfg_in )
 {
  %oldcfg = ( %{ local @INC = '.'; do $libnet_cfg_in } );
 }
elsif (eval { require Net::Config }) 
 {
  $have_old = 1;
  %oldcfg = %Net::Config::NetConfig;
 }

map { $cfg{lc $_} = $cfg{$_}; delete $cfg{$_} if /[A-Z]/ } keys %cfg;

#---------------------------------------------------------------------------

if ($opt_h) {
 print <<EOU;
$0: Usage: $0 [-c] [-d] [-i oldconfigile] [-o newconfigfile] [-h]
Without options, the old configuration is shown.

   -c change the configuration
   -d use defaults from the old config (implies -c, non-interactive)
   -i use a specific file as the old config file
   -o use a specific file as the new config file
   -h show this help

The default name of the old configuration file is by default
"libnet.cfg", unless otherwise specified using the -i option,
C<-i oldfile>, and it is searched first from the current directory,
and then from your module path.

The default name of the new configuration file is "libnet.cfg", and by
default it is written to the current directory, unless otherwise
specified using the -o option.

EOU
 exit(0);
}

#---------------------------------------------------------------------------

{
   my $oldcfgfile;
   my @inc;
   push @inc, $ENV{PERL5LIB} if exists $ENV{PERL5LIB};
   push @inc, $ENV{PERLLIB}  if exists $ENV{PERLLIB};
   push @inc, @INC;
   for (@inc) {
    my $trycfgfile = File::Spec->catfile($_, $libnet_cfg_in);
    if (-f $trycfgfile && -r $trycfgfile) {
     $oldcfgfile = $trycfgfile;
     last;
    }
   }
   print "# old config $oldcfgfile\n" if defined $oldcfgfile;
   for (sort keys %oldcfg) {
	printf "%-20s %s\n", $_,
               ref $oldcfg{$_} ? @{$oldcfg{$_}} : $oldcfg{$_};
   }
   unless ($opt_c || $opt_d) {
    print "# $0 -h for help\n";
    exit(0);
   }
}

#---------------------------------------------------------------------------

$oldcfg{'test_exist'} = 1 unless exists $oldcfg{'test_exist'};
$oldcfg{'test_hosts'} = 1 unless exists $oldcfg{'test_hosts'};

#---------------------------------------------------------------------------

if($have_old && !$opt_d)
 {
  $msg = <<EDQ;

Ah, I see you already have installed libnet before.

Do you want to modify/update your configuration (y|n) ?
EDQ

 $opt_d = 1
	unless get_bool($msg,0);
 }

#---------------------------------------------------------------------------

$msg = <<EDQ;

This script will prompt you to enter hostnames that can be used as
defaults for some of the modules in the libnet distribution.

To ensure that you do not enter an invalid hostname, I can perform a
lookup on each hostname you enter. If your internet connection is via
a dialup line then you may not want me to perform these lookups, as
it will require you to be on-line.

Do you want me to perform hostname lookups (y|n) ?
EDQ

$cfg{'test_exist'} = get_bool($msg, $oldcfg{'test_exist'});

print <<EDQ unless $cfg{'test_exist'};

*** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***

OK I will not check if the hostnames you give are valid
so be very cafeful

*** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***
EDQ


#---------------------------------------------------------------------------

print <<EDQ;

The following questions all require a list of host names, separated
with spaces. If you do not have a host available for any of the
services, then enter a single space, followed by <CR>. To accept the
default, hit <CR>

EDQ

$msg = 'Enter a list of available NNTP hosts :';

$def = $oldcfg{'nntp_hosts'} ||
	[ default_hostname($ENV{NNTPSERVER},$ENV{NEWSHOST},'news') ];

$cfg{'nntp_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available SMTP hosts :';

$def = $oldcfg{'smtp_hosts'} ||
	[ default_hostname(split(/:/,$ENV{SMTPHOSTS} || ""), 'mailhost') ];

$cfg{'smtp_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available POP3 hosts :';

$def = $oldcfg{'pop3_hosts'} || [];

$cfg{'pop3_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available SNPP hosts :';

$def = $oldcfg{'snpp_hosts'} || [];

$cfg{'snpp_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available PH Hosts   :'  ;

$def = $oldcfg{'ph_hosts'} ||
	[ default_hostname('dirserv') ];

$cfg{'ph_hosts'}   =  get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available TIME Hosts   :'  ;

$def = $oldcfg{'time_hosts'} || [];

$cfg{'time_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = 'Enter a list of available DAYTIME Hosts   :'  ;

$def = $oldcfg{'daytime_hosts'} || $oldcfg{'time_hosts'};

$cfg{'daytime_hosts'} = get_host_list($msg,$def);

#---------------------------------------------------------------------------

$msg = <<EDQ;

Do you have a firewall/ftp proxy  between your machine and the internet 

If you use a SOCKS firewall answer no

(y|n) ?
EDQ

if(get_bool($msg,0)) {

  $msg = <<'EDQ';
What series of FTP commands do you need to send to your
firewall to connect to an external host.

user/pass     => external user & password
fwuser/fwpass => firewall user & password

0) None
1) -----------------------
     USER user@remote.host
     PASS pass
2) -----------------------
     USER fwuser
     PASS fwpass
     USER user@remote.host
     PASS pass
3) -----------------------
     USER fwuser
     PASS fwpass
     SITE remote.site
     USER user
     PASS pass
4) -----------------------
     USER fwuser
     PASS fwpass
     OPEN remote.site
     USER user
     PASS pass
5) -----------------------
     USER user@fwuser@remote.site
     PASS pass@fwpass
6) -----------------------
     USER fwuser@remote.site
     PASS fwpass
     USER user
     PASS pass
7) -----------------------
     USER user@remote.host
     PASS pass
     AUTH fwuser
     RESP fwpass

Choice:
EDQ
 $def = exists $oldcfg{'ftp_firewall_type'}  ? $oldcfg{'ftp_firewall_type'} : 1;
 $ans = Prompt($msg,$def);
 $cfg{'ftp_firewall_type'} = 0+$ans;
 $def = $oldcfg{'ftp_firewall'} || $ENV{FTP_FIREWALL};

 $cfg{'ftp_firewall'} = get_hostname("FTP proxy hostname :", $def);
}
else {
 delete $cfg{'ftp_firewall'};
}


#---------------------------------------------------------------------------

if (defined $cfg{'ftp_firewall'})
 {
  print <<EDQ;

By default Net::FTP assumes that it only needs to use a firewall if it
cannot resolve the name of the host given. This only works if your DNS
system is setup to only resolve internal hostnames. If this is not the
case and your DNS will resolve external hostnames, then another method
is needed. Net::Config can do this if you provide the netmasks that
describe your internal network. Each netmask should be entered in the
form x.x.x.x/y, for example 127.0.0.0/8 or 214.8.16.32/24

EDQ
$def = [];
if(ref($oldcfg{'local_netmask'}))
 {
  $def = $oldcfg{'local_netmask'};
   print "Your current netmasks are :\n\n\t",
	join("\n\t",@{$def}),"\n\n";
 }

print "
Enter one netmask at each prompt, prefix with a - to remove a netmask
from the list, enter a '*' to clear the whole list, an '=' to show the
current list and an empty line to continue with Configure.

";

  my $mask = get_netmask("netmask :",$def);
  $cfg{'local_netmask'} = $mask if ref($mask) && @$mask;
 }

#---------------------------------------------------------------------------

###$msg =<<EDQ;
###
###SOCKS is a commonly used firewall protocol. If you use SOCKS firewalls
###then enter a list of hostames
###
###Enter a list of available SOCKS hosts :
###EDQ
###
###$def = $cfg{'socks_hosts'} ||
###	[ default_hostname($ENV{SOCKS5_SERVER},
###			   $ENV{SOCKS_SERVER},
###			   $ENV{SOCKS4_SERVER}) ];
###
###$cfg{'socks_hosts'}   =  get_host_list($msg,$def);

#---------------------------------------------------------------------------

print <<EDQ;

Normally when FTP needs a data connection the client tells the server
a port to connect to, and the server initiates a connection to the client.

Some setups, in particular firewall setups, can/do not work using this
protocol. In these situations the client must make the connection to the
server, this is called a passive transfer.
EDQ

if (defined $cfg{'ftp_firewall'}) {
  $msg = "\nShould all FTP connections via a firewall/proxy be passive (y|n) ?";

  $def = $oldcfg{'ftp_ext_passive'} || 0;

  $cfg{'ftp_ext_passive'} = get_bool($msg,$def);

  $msg = "\nShould all other FTP connections be passive (y|n) ?";

}
else {
  $msg = "\nShould all FTP connections be passive (y|n) ?";
}

$def = $oldcfg{'ftp_int_passive'} || 0;

$cfg{'ftp_int_passive'} = get_bool($msg,$def);


#---------------------------------------------------------------------------

$def = $oldcfg{'inet_domain'} || $ENV{LOCALDOMAIN};

$ans = Prompt("\nWhat is your local internet domain name :",$def);

$cfg{'inet_domain'} = ($ans =~ /(\S+)/)[0];

#---------------------------------------------------------------------------

$msg = <<EDQ;

If you specified some default hosts above, it is possible for me to
do some basic tests when you run 'make test'

This will cause 'make test' to be quite a bit slower and, if your
internet connection is via dialup, will require you to be on-line
unless the hosts are local.

Do you want me to run these tests (y|n) ?
EDQ

$cfg{'test_hosts'} = get_bool($msg,$oldcfg{'test_hosts'});

#---------------------------------------------------------------------------

$msg = <<EDQ;

To allow Net::FTP to be tested I will need a hostname. This host
should allow anonymous access and have a /pub directory

What host can I use :
EDQ

$cfg{'ftp_testhost'} = get_hostname($msg,$oldcfg{'ftp_testhost'})
	if $cfg{'test_hosts'};


print "\n";

#---------------------------------------------------------------------------

my $fh = IO::File->new($libnet_cfg_out, "w") or
	die "Cannot create '$libnet_cfg_out': $!";

print "Writing $libnet_cfg_out\n";

print $fh "{\n";

my $key;
foreach $key (keys %cfg) {
    my $val = $cfg{$key};
    if(!defined($val)) {
	$val = "undef";
    }
    elsif(ref($val)) {
	$val = '[' . join(",",
	    map {
		my $v = "undef";
		if(defined $_) {
		    ($v = $_) =~ s/'/\'/sog;
		    $v = "'" . $v . "'";
		}
		$v;
	    } @$val ) . ']';
    }
    else {
	$val =~ s/'/\'/sog;
	$val = "'" . $val . "'" if $val =~ /\D/;
    }
    print $fh "\t'",$key,"' => ",$val,",\n";
}

print $fh "}\n";

$fh->close;

############################################################################
############################################################################

exit 0;
!NO!SUBS!

close OUT or die "Can't close $file: $!";
chmod 0755, $file or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir $origdir;
