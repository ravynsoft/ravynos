# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::FTP;
use strict;

use Errno ();
use Fcntl qw(:flock);
use File::Basename qw(dirname);
use File::Path qw(mkpath);
use CPAN::FTP::netrc;
use vars qw($connect_to_internet_ok $Ua $Thesite $ThesiteURL $Themethod);

@CPAN::FTP::ISA = qw(CPAN::Debug);

use vars qw(
            $VERSION
);
$VERSION = "5.5016";

sub _plus_append_open {
    my($fh, $file) = @_;
    my $parent_dir = dirname $file;
    mkpath $parent_dir;
    my($cnt);
    until (open $fh, "+>>$file") {
        next if exists &Errno::EAGAIN && $! == &Errno::EAGAIN; # don't increment on EAGAIN
        $CPAN::Frontend->mydie("Could not open '$file' after 10000 tries: $!") if ++$cnt > 100000;
        sleep 0.0001;
        mkpath $parent_dir;
    }
}

#-> sub CPAN::FTP::ftp_statistics
# if they want to rewrite, they need to pass in a filehandle
sub _ftp_statistics {
    my($self,$fh) = @_;
    my $ftpstats_size = $CPAN::Config->{ftpstats_size};
    return if defined $ftpstats_size && $ftpstats_size <= 0;
    my $locktype = $fh ? LOCK_EX : LOCK_SH;
    # XXX On Windows flock() implements mandatory locking, so we can
    # XXX only use shared locking to still allow _yaml_loadfile() to
    # XXX read from the file using a different filehandle.
    $locktype = LOCK_SH if $^O eq "MSWin32";

    $fh ||= FileHandle->new;
    my $file = File::Spec->catfile($CPAN::Config->{cpan_home},"FTPstats.yml");
    _plus_append_open($fh,$file);
    my $sleep = 1;
    my $waitstart;
    while (!CPAN::_flock($fh, $locktype|LOCK_NB)) {
        $waitstart ||= localtime();
        if ($sleep>3) {
            my $now = localtime();
            $CPAN::Frontend->mywarn("$now: waiting for read lock on '$file' (since $waitstart)\n");
        }
        sleep($sleep); # this sleep must not be overridden;
                       # Frontend->mysleep with AUTOMATED_TESTING has
                       # provoked complete lock contention on my NFS
        if ($sleep <= 6) {
            $sleep+=0.5;
        } else {
            # retry to get a fresh handle. If it is NFS and the handle is stale, we will never get an flock
            _plus_append_open($fh, $file);
        }
    }
    my $stats = eval { CPAN->_yaml_loadfile($file, {loadblessed => 1}); };
    if ($@) {
        if (ref $@) {
            if (ref $@ eq "CPAN::Exception::yaml_not_installed") {
                chomp $@;
                $CPAN::Frontend->myprintonce("Warning (usually harmless): $@\n");
                return;
            } elsif (ref $@ eq "CPAN::Exception::yaml_process_error") {
                my $time = time;
                my $to = "$file.$time";
                $CPAN::Frontend->mywarn("Error reading '$file': $@
  Trying to stash it away as '$to' to prevent further interruptions.
  You may want to remove that file later.\n");
                # may fail because somebody else has moved it away in the meantime:
                rename $file, $to or $CPAN::Frontend->mywarn("Could not rename '$file' to '$to': $!\n");
                return;
            }
        } else {
            $CPAN::Frontend->mydie($@);
        }
    }
    CPAN::_flock($fh, LOCK_UN);
    return $stats->[0];
}

#-> sub CPAN::FTP::_mytime
sub _mytime () {
    if (CPAN->has_inst("Time::HiRes")) {
        return Time::HiRes::time();
    } else {
        return time;
    }
}

#-> sub CPAN::FTP::_new_stats
sub _new_stats {
    my($self,$file) = @_;
    my $ret = {
               file => $file,
               attempts => [],
               start => _mytime,
              };
    $ret;
}

#-> sub CPAN::FTP::_add_to_statistics
sub _add_to_statistics {
    my($self,$stats) = @_;
    my $yaml_module = CPAN::_yaml_module();
    $self->debug("yaml_module[$yaml_module]") if $CPAN::DEBUG;
    if ($CPAN::META->has_inst($yaml_module)) {
        $stats->{thesiteurl} = $ThesiteURL;
        $stats->{end} = CPAN::FTP::_mytime();
        my $fh = FileHandle->new;
        my $time = time;
        my $sdebug = 0;
        my @debug;
        @debug = $time if $sdebug;
        my $fullstats = $self->_ftp_statistics($fh);
        close $fh if $fh && defined(fileno($fh));
        $fullstats->{history} ||= [];
        push @debug, scalar @{$fullstats->{history}} if $sdebug;
        push @debug, time if $sdebug;
        push @{$fullstats->{history}}, $stats;
        # YAML.pm 0.62 is unacceptably slow with 999;
        # YAML::Syck 0.82 has no noticable performance problem with 999;
        my $ftpstats_size = $CPAN::Config->{ftpstats_size};
        $ftpstats_size = 99 unless defined $ftpstats_size;
        my $ftpstats_period = $CPAN::Config->{ftpstats_period} || 14;
        while (
               @{$fullstats->{history} || []}
               &&
               (
                @{$fullstats->{history}} > $ftpstats_size
                || $time - $fullstats->{history}[0]{start} > 86400*$ftpstats_period
               )
              ) {
            shift @{$fullstats->{history}}
        }
        push @debug, scalar @{$fullstats->{history}} if $sdebug;
        push @debug, time if $sdebug;
        push @debug, scalar localtime($fullstats->{history}[0]{start}) if $sdebug;
        # need no eval because if this fails, it is serious
        my $sfile = File::Spec->catfile($CPAN::Config->{cpan_home},"FTPstats.yml");
        CPAN->_yaml_dumpfile("$sfile.$$",$fullstats);
        if ( $sdebug ) {
            local $CPAN::DEBUG = 512; # FTP
            push @debug, time;
            CPAN->debug(sprintf("DEBUG history: before_read[%d]before[%d]at[%d]".
                                "after[%d]at[%d]oldest[%s]dumped backat[%d]",
                                @debug,
                               ));
        }
        # Win32 cannot rename a file to an existing filename
        unlink($sfile) if ($^O eq 'MSWin32' or $^O eq 'os2');
	_copy_stat($sfile, "$sfile.$$") if -e $sfile;
        rename "$sfile.$$", $sfile
            or $CPAN::Frontend->mywarn("Could not rename '$sfile.$$' to '$sfile': $!\nGiving up\n");
    }
}

# Copy some stat information (owner, group, mode and) from one file to
# another.
# This is a utility function which might be moved to a utility repository.
#-> sub CPAN::FTP::_copy_stat
sub _copy_stat {
    my($src, $dest) = @_;
    my @stat = stat($src);
    if (!@stat) {
	$CPAN::Frontend->mywarn("Can't stat '$src': $!\n");
	return;
    }

    eval {
	chmod $stat[2], $dest
	    or $CPAN::Frontend->mywarn("Can't chmod '$dest' to " . sprintf("0%o", $stat[2]) . ": $!\n");
    };
    warn $@ if $@;
    eval {
	chown $stat[4], $stat[5], $dest
	    or do {
		my $save_err = $!; # otherwise it's lost in the get... calls
		$CPAN::Frontend->mywarn("Can't chown '$dest' to " .
					(getpwuid($stat[4]))[0] . "/" .
					(getgrgid($stat[5]))[0] . ": $save_err\n"
				       );
	    };
    };
    warn $@ if $@;
}

# if file is CHECKSUMS, suggest the place where we got the file to be
# checked from, maybe only for young files?
#-> sub CPAN::FTP::_recommend_url_for
sub _recommend_url_for {
    my($self, $file, $urllist) = @_;
    if ($file =~ s|/CHECKSUMS(.gz)?$||) {
        my $fullstats = $self->_ftp_statistics();
        my $history = $fullstats->{history} || [];
        while (my $last = pop @$history) {
            last if $last->{end} - time > 3600; # only young results are interesting
            next unless $last->{file}; # dirname of nothing dies!
            next unless $file eq dirname($last->{file});
            return $last->{thesiteurl};
        }
    }
    if ($CPAN::Config->{randomize_urllist}
        &&
        rand(1) < $CPAN::Config->{randomize_urllist}
       ) {
        $urllist->[int rand scalar @$urllist];
    } else {
        return ();
    }
}

#-> sub CPAN::FTP::_get_urllist
sub _get_urllist {
    my($self, $with_defaults) = @_;
    $with_defaults ||= 0;
    CPAN->debug("with_defaults[$with_defaults]") if $CPAN::DEBUG;

    $CPAN::Config->{urllist} ||= [];
    unless (ref $CPAN::Config->{urllist} eq 'ARRAY') {
        $CPAN::Frontend->mywarn("Malformed urllist; ignoring.  Configuration file corrupt?\n");
        $CPAN::Config->{urllist} = [];
    }
    my @urllist = grep { defined $_ and length $_ } @{$CPAN::Config->{urllist}};
    push @urllist, @CPAN::Defaultsites if $with_defaults;
    for my $u (@urllist) {
        CPAN->debug("u[$u]") if $CPAN::DEBUG;
        if (UNIVERSAL::can($u,"text")) {
            $u->{TEXT} .= "/" unless substr($u->{TEXT},-1) eq "/";
        } else {
            $u .= "/" unless substr($u,-1) eq "/";
            $u = CPAN::URL->new(TEXT => $u, FROM => "USER");
        }
    }
    \@urllist;
}

#-> sub CPAN::FTP::ftp_get ;
sub ftp_get {
    my($class,$host,$dir,$file,$target) = @_;
    $class->debug(
                  qq[Going to fetch file [$file] from dir [$dir]
        on host [$host] as local [$target]\n]
                 ) if $CPAN::DEBUG;
    my $ftp = Net::FTP->new($host);
    unless ($ftp) {
        $CPAN::Frontend->mywarn("  Could not connect to host '$host' with Net::FTP\n");
        return;
    }
    return 0 unless defined $ftp;
    $ftp->debug(1) if $CPAN::DEBUG{'FTP'} & $CPAN::DEBUG;
    $class->debug(qq[Going to login("anonymous","$Config::Config{cf_email}")]);
    unless ( $ftp->login("anonymous",$Config::Config{'cf_email'}) ) {
        my $msg = $ftp->message;
        $CPAN::Frontend->mywarn("  Couldn't login on $host: $msg\n");
        return;
    }
    unless ( $ftp->cwd($dir) ) {
        my $msg = $ftp->message;
        $CPAN::Frontend->mywarn("  Couldn't cwd $dir: $msg\n");
        return;
    }
    $ftp->binary;
    $class->debug(qq[Going to ->get("$file","$target")\n]) if $CPAN::DEBUG;
    unless ( $ftp->get($file,$target) ) {
        my $msg = $ftp->message;
        $CPAN::Frontend->mywarn("  Couldn't fetch $file from $host: $msg\n");
        return;
    }
    $ftp->quit; # it's ok if this fails
    return 1;
}

# If more accuracy is wanted/needed, Chris Leach sent me this patch...

 # > *** /install/perl/live/lib/CPAN.pm- Wed Sep 24 13:08:48 1997
 # > --- /tmp/cp Wed Sep 24 13:26:40 1997
 # > ***************
 # > *** 1562,1567 ****
 # > --- 1562,1580 ----
 # >       return 1 if substr($url,0,4) eq "file";
 # >       return 1 unless $url =~ m|://([^/]+)|;
 # >       my $host = $1;
 # > +     my $proxy = $CPAN::Config->{'http_proxy'} || $ENV{'http_proxy'};
 # > +     if ($proxy) {
 # > +         $proxy =~ m|://([^/:]+)|;
 # > +         $proxy = $1;
 # > +         my $noproxy = $CPAN::Config->{'no_proxy'} || $ENV{'no_proxy'};
 # > +         if ($noproxy) {
 # > +             if ($host !~ /$noproxy$/) {
 # > +                 $host = $proxy;
 # > +             }
 # > +         } else {
 # > +             $host = $proxy;
 # > +         }
 # > +     }
 # >       require Net::Ping;
 # >       return 1 unless $Net::Ping::VERSION >= 2;
 # >       my $p;


#-> sub CPAN::FTP::localize ;
sub localize {
    my($self,$file,$aslocal,$force,$with_defaults) = @_;
    $force ||= 0;
    Carp::croak( "Usage: ->localize(cpan_file,as_local_file[,\$force])" )
        unless defined $aslocal;
    if ($CPAN::DEBUG){
        require Carp;
        my $longmess = Carp::longmess();
        $self->debug("file[$file] aslocal[$aslocal] force[$force] carplongmess[$longmess]");
    }
    for ($CPAN::Config->{connect_to_internet_ok}) {
        $connect_to_internet_ok = $_ if not defined $connect_to_internet_ok and defined $_;
    }
    my $ph = $CPAN::Config->{pushy_https};
    if (!defined $ph || $ph) {
        return $self->localize_2021($file,$aslocal,$force,$with_defaults);
    } else {
        return $self->localize_1995ff($file,$aslocal,$force,$with_defaults);
    }
}

sub have_promising_aslocal {
    my($self, $aslocal, $force) = @_;
    if (-f $aslocal && -r _ && !($force & 1)) {
        my $size;
        if ($size = -s $aslocal) {
            $self->debug("aslocal[$aslocal]size[$size]") if $CPAN::DEBUG;
            return 1;
        } else {
            # empty file from a previous unsuccessful attempt to download it
            unlink $aslocal or
                $CPAN::Frontend->mydie("Found a zero-length '$aslocal' that I ".
                                       "could not remove.");
        }
    }
    return;
}

#-> sub CPAN::FTP::localize ;
sub localize_2021 {
    my($self,$file,$aslocal,$force,$with_defaults) = @_;
    return $aslocal if $self->have_promising_aslocal($aslocal, $force);
    my($aslocal_dir) = dirname($aslocal);
    my $ret;
    $self->mymkpath($aslocal_dir);
    my $aslocal_tempfile = $aslocal . ".tmp" . $$;
    my $base;
    if (
           ($CPAN::META->has_usable('HTTP::Tiny')
            && $CPAN::META->has_usable('Net::SSLeay')
            && $CPAN::META->has_usable('IO::Socket::SSL')
           )
        || $CPAN::Config->{curl}
        || $CPAN::Config->{wget}
       ) {
        for my $prx (qw(https_proxy no_proxy)) {
            $ENV{$prx} = $CPAN::Config->{$prx} if $CPAN::Config->{$prx};
        }
        $base = "https://cpan.org/";
    } else {
        my @missing_modules = grep { ! $CPAN::META->has_usable($_) } qw(HTTP::Tiny Net::SSLeay IO::Socket::SSL);
        my $miss = join ", ", map { "'$_'" } @missing_modules;
        my $modules = @missing_modules == 1 ? "module" : "modules";
        $CPAN::Frontend->mywarn("Missing or unusable $modules $miss, and found neither curl nor wget installed.\n");
        if ($CPAN::META->has_usable('HTTP::Tiny')) {
            $CPAN::Frontend->mywarn("Need to fall back to http.\n")
        }
        for my $prx (qw(http_proxy no_proxy)) {
            $ENV{$prx} = $CPAN::Config->{$prx} if $CPAN::Config->{$prx};
        }
        $base = "http://www.cpan.org/";
    }
    $ret = $self->hostdl_2021($base,$file,$aslocal_tempfile);
    if ($ret) { # c&p from below
        CPAN->debug("ret[$ret]aslocal[$aslocal]") if $CPAN::DEBUG;
        if ($ret eq $aslocal_tempfile) {
            # if we got it exactly as we asked for, only then we
            # want to rename
            rename $aslocal_tempfile, $aslocal
                or $CPAN::Frontend->mydie("Error while trying to rename ".
                                          "'$ret' to '$aslocal': $!");
            $ret = $aslocal;
        }
    } else {
        unlink $aslocal_tempfile;
        return;
    }
    return $ret;
}

sub hostdl_2021 {
    my($self, $base, $file, $aslocal) = @_; # the $aslocal is $aslocal_tempfile in the caller (old convention)
    my $proxy_vars = $self->_proxy_vars($base);
    my($proto) = $base =~ /^(https?)/;
    my $url = "$base$file";
    # hostdl_2021 may be called with either http or https urls
    if (
        $CPAN::META->has_usable('HTTP::Tiny')
        &&
        (
         $proto eq "http"
         ||
         (    $CPAN::META->has_usable('Net::SSLeay')
              && $CPAN::META->has_usable('IO::Socket::SSL')   )
        )
       ){
        # mostly c&p from below
        require CPAN::HTTP::Client;
        my $chc = CPAN::HTTP::Client->new(
            proxy => $CPAN::Config->{http_proxy} || $ENV{http_proxy},
            no_proxy => $CPAN::Config->{no_proxy} || $ENV{no_proxy},
        );
        for my $try ( $url, ( $url !~ /\.gz(?!\n)\Z/ ? "$url.gz" : () ) ) {
            $CPAN::Frontend->myprint("Fetching with HTTP::Tiny:\n$try\n");
            my $res = eval { $chc->mirror($try, $aslocal) };
            if ( $res && $res->{success} ) {
                my $now = time;
                utime $now, $now, $aslocal; # download time is more
                                            # important than upload
                                            # time
                return $aslocal;
            }
            elsif ( $res && $res->{status} ne '599') {
                $CPAN::Frontend->myprint(sprintf(
                        "HTTP::Tiny failed with code[%s] message[%s]\n",
                        $res->{status},
                        $res->{reason},
                    )
                );
            }
            elsif ( $res && $res->{status} eq '599') {
                $CPAN::Frontend->myprint(sprintf(
                        "HTTP::Tiny failed with an internal error: %s\n",
                        $res->{content},
                    )
                );
            }
            else {
                my $err = $@ || 'Unknown error';
                $CPAN::Frontend->myprint(sprintf(
                        "Error downloading with HTTP::Tiny: %s\n", $err
                    )
                );
            }
        }
    } elsif ($CPAN::Config->{curl} || $CPAN::Config->{wget}){
        # c&p from further down
        my($src_switch, $stdout_redir);
        my($devnull) = $CPAN::Config->{devnull} || "";
      DLPRG: for my $dlprg (qw(curl wget)) {
            my $dlprg_configured = $CPAN::Config->{$dlprg};
            next unless defined $dlprg_configured && length $dlprg_configured;
            my $funkyftp = CPAN::HandleConfig->safe_quote($dlprg_configured);
            if ($dlprg eq "wget") {
                $src_switch = " -O \"$aslocal\"";
                $stdout_redir = "";
            } elsif ($dlprg eq 'curl') {
                $src_switch   = ' -L -f -s -S --netrc-optional';
                $stdout_redir = " > \"$aslocal\"";
                if ($proxy_vars->{http_proxy}) {
                    $src_switch .= qq{ -U "$proxy_vars->{proxy_user}:$proxy_vars->{proxy_pass}" -x "$proxy_vars->{http_proxy}"};
                }
            }
            $CPAN::Frontend->myprint(
                                     qq[
Trying with
    $funkyftp$src_switch
to get
    $url
]);
            my($system) =
                "$funkyftp$src_switch \"$url\" $devnull$stdout_redir";
            $self->debug("system[$system]") if $CPAN::DEBUG;
            my($wstatus) = system($system);
            if ($wstatus == 0) {
                return $aslocal;
            } else {
                my $estatus = $wstatus >> 8;
                my $size = -f $aslocal ?
                    ", left\n$aslocal with size ".-s _ :
                    "\nWarning: expected file [$aslocal] doesn't exist";
                $CPAN::Frontend->myprint(qq{
    Function system("$system")
    returned status $estatus (wstat $wstatus)$size
    });
            }
        } # DLPRG
    } # curl, wget
    return;
}

#-> sub CPAN::FTP::localize ;
sub localize_1995ff {
    my($self,$file,$aslocal,$force,$with_defaults) = @_;
    if ($^O eq 'MacOS') {
        # Comment by AK on 2000-09-03: Uniq short filenames would be
        # available in CHECKSUMS file
        my($name, $path) = File::Basename::fileparse($aslocal, '');
        if (length($name) > 31) {
            $name =~ s/(
                        \.(
                           readme(\.(gz|Z))? |
                           (tar\.)?(gz|Z) |
                           tgz |
                           zip |
                           pm\.(gz|Z)
                          )
                       )$//x;
            my $suf = $1;
            my $size = 31 - length($suf);
            while (length($name) > $size) {
                chop $name;
            }
            $name .= $suf;
            $aslocal = File::Spec->catfile($path, $name);
        }
    }

    return $aslocal if $self->have_promising_aslocal($aslocal, $force);
    my($maybe_restore) = 0;
    if (-f $aslocal) {
        rename $aslocal, "$aslocal.bak$$";
        $maybe_restore++;
    }

    my($aslocal_dir) = dirname($aslocal);
    # Inheritance is not easier to manage than a few if/else branches
    if ($CPAN::META->has_usable('LWP::UserAgent')) {
        unless ($Ua) {
            CPAN::LWP::UserAgent->config;
            eval {$Ua = CPAN::LWP::UserAgent->new;}; # Why is has_usable still not fit enough?
            if ($@) {
                $CPAN::Frontend->mywarn("CPAN::LWP::UserAgent->new dies with $@\n")
                    if $CPAN::DEBUG;
            } else {
                my($var);
                $Ua->proxy('ftp',  $var)
                    if $var = $CPAN::Config->{ftp_proxy} || $ENV{ftp_proxy};
                $Ua->proxy('http', $var)
                    if $var = $CPAN::Config->{http_proxy} || $ENV{http_proxy};
                $Ua->no_proxy($var)
                    if $var = $CPAN::Config->{no_proxy} || $ENV{no_proxy};
            }
        }
    }
    for my $prx (qw(ftp_proxy http_proxy no_proxy)) {
        $ENV{$prx} = $CPAN::Config->{$prx} if $CPAN::Config->{$prx};
    }

    # Try the list of urls for each single object. We keep a record
    # where we did get a file from
    my(@reordered,$last);
    my $ccurllist = $self->_get_urllist($with_defaults);
    $last = $#$ccurllist;
    if ($force & 2) { # local cpans probably out of date, don't reorder
        @reordered = (0..$last);
    } else {
        @reordered =
            sort {
                (substr($ccurllist->[$b],0,4) eq "file")
                    <=>
                (substr($ccurllist->[$a],0,4) eq "file")
                    or
                defined($ThesiteURL)
                    and
                ($ccurllist->[$b] eq $ThesiteURL)
                    <=>
                ($ccurllist->[$a] eq $ThesiteURL)
            } 0..$last;
    }
    my(@levels);
    $Themethod ||= "";
    $self->debug("Themethod[$Themethod]reordered[@reordered]") if $CPAN::DEBUG;
    my @all_levels = (
                      ["dleasy",   "file"],
                      ["dleasy"],
                      ["dlhard"],
                      ["dlhardest"],
                      ["dleasy",   "http","defaultsites"],
                      ["dlhard",   "http","defaultsites"],
                      ["dleasy",   "ftp", "defaultsites"],
                      ["dlhard",   "ftp", "defaultsites"],
                      ["dlhardest","",    "defaultsites"],
                     );
    if ($Themethod) {
        @levels = grep {$_->[0] eq $Themethod} @all_levels;
        push @levels, grep {$_->[0] ne $Themethod} @all_levels;
    } else {
        @levels = @all_levels;
    }
    @levels = qw/dleasy/ if $^O eq 'MacOS';
    my($levelno);
    local $ENV{FTP_PASSIVE} =
        exists $CPAN::Config->{ftp_passive} ?
        $CPAN::Config->{ftp_passive} : 1;
    my $ret;
    my $stats = $self->_new_stats($file);
  LEVEL: for $levelno (0..$#levels) {
        my $level_tuple = $levels[$levelno];
        my($level,$scheme,$sitetag) = @$level_tuple;
        $self->mymkpath($aslocal_dir) unless $scheme && "file" eq $scheme;
        my $defaultsites = $sitetag && $sitetag eq "defaultsites" && !@$ccurllist;
        my @urllist;
        if ($defaultsites) {
            unless (defined $connect_to_internet_ok) {
                $CPAN::Frontend->myprint(sprintf qq{
I would like to connect to one of the following sites to get '%s':

%s
},
                                         $file,
                                         join("",map { " ".$_->text."\n" } @CPAN::Defaultsites),
                                        );
                my $answer = CPAN::Shell::colorable_makemaker_prompt("Is it OK to try to connect to the Internet?", "yes");
                if ($answer =~ /^y/i) {
                    $connect_to_internet_ok = 1;
                } else {
                    $connect_to_internet_ok = 0;
                }
            }
            if ($connect_to_internet_ok) {
                @urllist = @CPAN::Defaultsites;
            } else {
                my $sleep = 2;
                # the tricky thing about dying here is that everybody
                # believes that calls to exists() or all_objects() are
                # safe.
                require CPAN::Exception::blocked_urllist;
                die CPAN::Exception::blocked_urllist->new;
            }
        } else { # ! $defaultsites
            my @host_seq = $level =~ /dleasy/ ?
                @reordered : 0..$last;  # reordered has file and $Thesiteurl first
            @urllist = map { $ccurllist->[$_] } @host_seq;
        }
        $self->debug("synth. urllist[@urllist]") if $CPAN::DEBUG;
        my $aslocal_tempfile = $aslocal . ".tmp" . $$;
        if (my $recommend = $self->_recommend_url_for($file,\@urllist)) {
            @urllist = grep { $_ ne $recommend } @urllist;
            unshift @urllist, $recommend;
        }
        $self->debug("synth. urllist[@urllist]") if $CPAN::DEBUG;
        $ret = $self->hostdlxxx($level,$scheme,\@urllist,$file,$aslocal_tempfile,$stats);
        if ($ret) {
            CPAN->debug("ret[$ret]aslocal[$aslocal]") if $CPAN::DEBUG;
            if ($ret eq $aslocal_tempfile) {
                # if we got it exactly as we asked for, only then we
                # want to rename
                rename $aslocal_tempfile, $aslocal
                    or $CPAN::Frontend->mydie("Error while trying to rename ".
                                              "'$ret' to '$aslocal': $!");
                $ret = $aslocal;
            }
            elsif (-f $ret && $scheme eq 'file' ) {
                # it's a local file, so there's nothing left to do, we
                # let them read from where it is
            }
            $Themethod = $level;
            my $now = time;
            # utime $now, $now, $aslocal; # too bad, if we do that, we
                                          # might alter a local mirror
            $self->debug("level[$level]") if $CPAN::DEBUG;
            last LEVEL;
        } else {
            unlink $aslocal_tempfile;
            last if $CPAN::Signal; # need to cleanup
        }
    }
    if ($ret) {
        $stats->{filesize} = -s $ret;
    }
    $self->debug("before _add_to_statistics") if $CPAN::DEBUG;
    $self->_add_to_statistics($stats);
    $self->debug("after _add_to_statistics") if $CPAN::DEBUG;
    if ($ret) {
        unlink "$aslocal.bak$$";
        return $ret;
    }
    unless ($CPAN::Signal) {
        my(@mess);
        local $" = " ";
        if (@{$CPAN::Config->{urllist}}) {
            push @mess,
                qq{Please check, if the URLs I found in your configuration file \(}.
                    join(", ", @{$CPAN::Config->{urllist}}).
                        qq{\) are valid.};
        } else {
            push @mess, qq{Your urllist is empty!};
        }
        push @mess, qq{The urllist can be edited.},
            qq{E.g. with 'o conf urllist push ftp://myurl/'};
        $CPAN::Frontend->mywarn(Text::Wrap::wrap("","","@mess"). "\n\n");
        $CPAN::Frontend->mydie("Could not fetch $file\n");
    }
    if ($maybe_restore) {
        rename "$aslocal.bak$$", $aslocal;
        $CPAN::Frontend->myprint("Trying to get away with old file:\n" .
                                 $self->ls($aslocal) . "\n");
        return $aslocal;
    }
    return;
}

sub mymkpath {
    my($self, $aslocal_dir) = @_;
    mkpath($aslocal_dir);
    $CPAN::Frontend->mywarn(qq{Warning: You are not allowed to write into }.
                            qq{directory "$aslocal_dir".
    I\'ll continue, but if you encounter problems, they may be due
    to insufficient permissions.\n}) unless -w $aslocal_dir;
}

sub hostdlxxx {
    my $self = shift;
    my $level = shift;
    my $scheme = shift;
    my $h = shift;
    $h = [ grep /^\Q$scheme\E:/, @$h ] if $scheme;
    my $method = "host$level";
    $self->$method($h, @_);
}

sub _set_attempt {
    my($self,$stats,$method,$url) = @_;
    push @{$stats->{attempts}}, {
                                 method => $method,
                                 start => _mytime,
                                 url => $url,
                                };
}

# package CPAN::FTP;
sub hostdleasy { #called from hostdlxxx
    my($self,$host_seq,$file,$aslocal,$stats) = @_;
    my($ro_url);
  HOSTEASY: for $ro_url (@$host_seq) {
        $self->_set_attempt($stats,"dleasy",$ro_url);
        my $url = "$ro_url$file";
        $self->debug("localizing perlish[$url]") if $CPAN::DEBUG;
        if ($url =~ /^file:/) {
            my $l;
            if ($CPAN::META->has_inst('URI::URL')) {
                my $u =  URI::URL->new($url);
                $l = $u->file;
            } else { # works only on Unix, is poorly constructed, but
                # hopefully better than nothing.
                # RFC 1738 says fileurl BNF is
                # fileurl = "file://" [ host | "localhost" ] "/" fpath
                # Thanks to "Mark D. Baushke" <mdb@cisco.com> for
                # the code
                ($l = $url) =~ s|^file://[^/]*/|/|; # discard the host part
                $l =~ s|^file:||;                   # assume they
                                                    # meant
                                                    # file://localhost
                $l =~ s|^/||s
                    if ! -f $l && $l =~ m|^/\w:|;   # e.g. /P:
            }
            $self->debug("local file[$l]") if $CPAN::DEBUG;
            if ( -f $l && -r _) {
                $ThesiteURL = $ro_url;
                return $l;
            }
            # If request is for a compressed file and we can find the
            # uncompressed file also, return the path of the uncompressed file
            # otherwise, decompress it and return the resulting path
            if ($l =~ /(.+)\.gz$/) {
                my $ungz = $1;
                if ( -f $ungz && -r _) {
                    $ThesiteURL = $ro_url;
                    return $ungz;
                }
                elsif (-f $l && -r _) {
                    eval { CPAN::Tarzip->new($l)->gunzip($aslocal) };
                    if ( -f $aslocal && -s _) {
                        $ThesiteURL = $ro_url;
                        return $aslocal;
                    }
                    elsif (! -s $aslocal) {
                        unlink $aslocal;
                    }
                    elsif (-f $l) {
                        $CPAN::Frontend->mywarn("Error decompressing '$l': $@\n")
                            if $@;
                        return;
                    }
                }
            }
            # Otherwise, return the local file path if it exists
            elsif ( -f $l && -r _) {
                $ThesiteURL = $ro_url;
                return $l;
            }
            # If we can't find it, but there is a compressed version
            # of it, then decompress it
            elsif (-f "$l.gz") {
                $self->debug("found compressed $l.gz") if $CPAN::DEBUG;
                eval { CPAN::Tarzip->new("$l.gz")->gunzip($aslocal) };
                if ( -f $aslocal) {
                    $ThesiteURL = $ro_url;
                    return $aslocal;
                }
                else {
                    $CPAN::Frontend->mywarn("Error decompressing '$l': $@\n")
                        if $@;
                    return;
                }
            }
            $CPAN::Frontend->mywarn("Could not find '$l'\n");
        }
        $self->debug("it was not a file URL") if $CPAN::DEBUG;
        if ($CPAN::META->has_usable('LWP')) {
            $CPAN::Frontend->myprint("Fetching with LWP:\n$url\n");
            unless ($Ua) {
                CPAN::LWP::UserAgent->config;
                eval { $Ua = CPAN::LWP::UserAgent->new; };
                if ($@) {
                    $CPAN::Frontend->mywarn("CPAN::LWP::UserAgent->new dies with $@\n");
                }
            }
            my $res = $Ua->mirror($url, $aslocal);
            if ($res->is_success) {
                $ThesiteURL = $ro_url;
                my $now = time;
                utime $now, $now, $aslocal; # download time is more
                                            # important than upload
                                            # time
                return $aslocal;
            } elsif ($url !~ /\.gz(?!\n)\Z/) {
                my $gzurl = "$url.gz";
                $CPAN::Frontend->myprint("Fetching with LWP:\n$gzurl\n");
                $res = $Ua->mirror($gzurl, "$aslocal.gz");
                if ($res->is_success) {
                    if (eval {CPAN::Tarzip->new("$aslocal.gz")->gunzip($aslocal)}) {
                        $ThesiteURL = $ro_url;
                        return $aslocal;
                    }
                }
            } else {
                $CPAN::Frontend->myprint(sprintf(
                                                 "LWP failed with code[%s] message[%s]\n",
                                                 $res->code,
                                                 $res->message,
                                                ));
                # Alan Burlison informed me that in firewall environments
                # Net::FTP can still succeed where LWP fails. So we do not
                # skip Net::FTP anymore when LWP is available.
            }
        } elsif ($url =~ /^http:/i && $CPAN::META->has_usable('HTTP::Tiny')) {
            require CPAN::HTTP::Client;
            my $chc = CPAN::HTTP::Client->new(
                proxy => $CPAN::Config->{http_proxy} || $ENV{http_proxy},
                no_proxy => $CPAN::Config->{no_proxy} || $ENV{no_proxy},
            );
            for my $try ( $url, ( $url !~ /\.gz(?!\n)\Z/ ? "$url.gz" : () ) ) {
                $CPAN::Frontend->myprint("Fetching with HTTP::Tiny:\n$try\n");
                my $res = eval { $chc->mirror($try, $aslocal) };
                if ( $res && $res->{success} ) {
                    $ThesiteURL = $ro_url;
                    my $now = time;
                    utime $now, $now, $aslocal; # download time is more
                                                # important than upload
                                                # time
                    return $aslocal;
                }
                elsif ( $res && $res->{status} ne '599') {
                    $CPAN::Frontend->myprint(sprintf(
                            "HTTP::Tiny failed with code[%s] message[%s]\n",
                            $res->{status},
                            $res->{reason},
                        )
                    );
                }
                elsif ( $res && $res->{status} eq '599') {
                    $CPAN::Frontend->myprint(sprintf(
                            "HTTP::Tiny failed with an internal error: %s\n",
                            $res->{content},
                        )
                    );
                }
                else {
                    my $err = $@ || 'Unknown error';
                    $CPAN::Frontend->myprint(sprintf(
                            "Error downloading with HTTP::Tiny: %s\n", $err
                        )
                    );
                }
            }
        }
        return if $CPAN::Signal;
        if ($url =~ m|^ftp://(.*?)/(.*)/(.*)|) {
            # that's the nice and easy way thanks to Graham
            $self->debug("recognized ftp") if $CPAN::DEBUG;
            my($host,$dir,$getfile) = ($1,$2,$3);
            if ($CPAN::META->has_usable('Net::FTP')) {
                $dir =~ s|/+|/|g;
                $CPAN::Frontend->myprint("Fetching with Net::FTP:\n$url\n");
                $self->debug("getfile[$getfile]dir[$dir]host[$host]" .
                             "aslocal[$aslocal]") if $CPAN::DEBUG;
                if (CPAN::FTP->ftp_get($host,$dir,$getfile,$aslocal)) {
                    $ThesiteURL = $ro_url;
                    return $aslocal;
                }
                if ($aslocal !~ /\.gz(?!\n)\Z/) {
                    my $gz = "$aslocal.gz";
                    $CPAN::Frontend->myprint("Fetching with Net::FTP\n$url.gz\n");
                    if (CPAN::FTP->ftp_get($host,
                                           $dir,
                                           "$getfile.gz",
                                           $gz) &&
                        eval{CPAN::Tarzip->new($gz)->gunzip($aslocal)}
                    ) {
                        $ThesiteURL = $ro_url;
                        return $aslocal;
                    }
                }
                # next HOSTEASY;
            } else {
                CPAN->debug("Net::FTP does not count as usable atm") if $CPAN::DEBUG;
            }
        }
        if (
            UNIVERSAL::can($ro_url,"text")
            and
            $ro_url->{FROM} eq "USER"
           ) {
            ##address #17973: default URLs should not try to override
            ##user-defined URLs just because LWP is not available
            my $ret = $self->hostdlhard([$ro_url],$file,$aslocal,$stats);
            return $ret if $ret;
        }
        return if $CPAN::Signal;
    }
}

# package CPAN::FTP;
sub hostdlhard {
    my($self,$host_seq,$file,$aslocal,$stats) = @_;

    # Came back if Net::FTP couldn't establish connection (or
    # failed otherwise) Maybe they are behind a firewall, but they
    # gave us a socksified (or other) ftp program...

    my($ro_url);
    my($devnull) = $CPAN::Config->{devnull} || "";
    # < /dev/null ";
    my($aslocal_dir) = dirname($aslocal);
    mkpath($aslocal_dir);
    my $some_dl_success = 0;
    my $any_attempt = 0;
 HOSTHARD: for $ro_url (@$host_seq) {
        $self->_set_attempt($stats,"dlhard",$ro_url);
        my $url = "$ro_url$file";
        my($proto,$host,$dir,$getfile);

        # Courtesy Mark Conty mark_conty@cargill.com change from
        # if ($url =~ m|^ftp://(.*?)/(.*)/(.*)|) {
        # to
        if ($url =~ m|^([^:]+)://(.*?)/(.*)/(.*)|) {
            # proto not yet used
            ($proto,$host,$dir,$getfile) = ($1,$2,$3,$4);
        } else {
            next HOSTHARD; # who said, we could ftp anything except ftp?
        }
        next HOSTHARD if $proto eq "file"; # file URLs would have had
                                           # success above. Likely a bogus URL

        # making at least one attempt against a host
        $any_attempt++;

        $self->debug("localizing funkyftpwise[$url]") if $CPAN::DEBUG;

        # Try the most capable first and leave ncftp* for last as it only
        # does FTP.
        my $proxy_vars = $self->_proxy_vars($ro_url);
      DLPRG: for my $f (qw(curl wget lynx ncftpget ncftp)) {
            my $funkyftp = CPAN::HandleConfig->safe_quote($CPAN::Config->{$f});
            next DLPRG unless defined $funkyftp;
            next DLPRG if $funkyftp =~ /^\s*$/;

            my($src_switch) = "";
            my($chdir) = "";
            my($stdout_redir) = " > \"$aslocal\"";
            if ($f eq "lynx") {
                $src_switch = " -source";
            } elsif ($f eq "ncftp") {
                next DLPRG unless $url =~ m{\Aftp://};
                $src_switch = " -c";
            } elsif ($f eq "wget") {
                $src_switch = " -O \"$aslocal\"";
                $stdout_redir = "";
            } elsif ($f eq 'curl') {
                $src_switch = ' -L -f -s -S --netrc-optional';
                if ($proxy_vars->{http_proxy}) {
                    $src_switch .= qq{ -U "$proxy_vars->{proxy_user}:$proxy_vars->{proxy_pass}" -x "$proxy_vars->{http_proxy}"};
                }
            } elsif ($f eq "ncftpget") {
                next DLPRG unless $url =~ m{\Aftp://};
                $chdir = "cd $aslocal_dir && ";
                $stdout_redir = "";
            }
            $CPAN::Frontend->myprint(
                                     qq[
Trying with
    $funkyftp$src_switch
to get
    $url
]);
            my($system) =
                "$chdir$funkyftp$src_switch \"$url\" $devnull$stdout_redir";
            $self->debug("system[$system]") if $CPAN::DEBUG;
            my($wstatus) = system($system);
            if ($f eq "lynx") {
                # lynx returns 0 when it fails somewhere
                if (-s $aslocal) {
                    my $content = do { local *FH;
                                       open FH, $aslocal or die;
                                       local $/;
                                       <FH> };
                    if ($content =~ /^<.*(<title>[45]|Error [45])/si) {
                        $CPAN::Frontend->mywarn(qq{
No success, the file that lynx has downloaded looks like an error message:
$content
});
                        $CPAN::Frontend->mysleep(1);
                        next DLPRG;
                    }
                    $some_dl_success++;
                } else {
                    $CPAN::Frontend->myprint(qq{
No success, the file that lynx has downloaded is an empty file.
});
                    next DLPRG;
                }
            }
            if ($wstatus == 0) {
                if (-s $aslocal) {
                    # Looks good
                    $some_dl_success++;
                }
                $ThesiteURL = $ro_url;
                return $aslocal;
            } else {
                my $estatus = $wstatus >> 8;
                my $size = -f $aslocal ?
                    ", left\n$aslocal with size ".-s _ :
                    "\nWarning: expected file [$aslocal] doesn't exist";
                $CPAN::Frontend->myprint(qq{
    Function system("$system")
    returned status $estatus (wstat $wstatus)$size
    });
            }
            return if $CPAN::Signal;
        } # download/transfer programs (DLPRG)
    } # host
    return unless $any_attempt;
    if ($some_dl_success) {
        $CPAN::Frontend->mywarn("Warning: doesn't seem we had substantial success downloading '$aslocal'. Don't know how to proceed.\n");
    } else {
        $CPAN::Frontend->mywarn("Warning: no success downloading '$aslocal'. Giving up on it.\n");
    }
    return;
}

#-> CPAN::FTP::_proxy_vars
sub _proxy_vars {
    my($self,$url) = @_;
    my $ret = +{};
    my $http_proxy = $CPAN::Config->{'http_proxy'} || $ENV{'http_proxy'};
    if ($http_proxy) {
        my($host) = $url =~ m|://([^/:]+)|;
        my $want_proxy = 1;
        my $noproxy = $CPAN::Config->{'no_proxy'} || $ENV{'no_proxy'} || "";
        my @noproxy = split /\s*,\s*/, $noproxy;
        if ($host) {
          DOMAIN: for my $domain (@noproxy) {
                if ($host =~ /\Q$domain\E$/) { # cf. LWP::UserAgent
                    $want_proxy = 0;
                    last DOMAIN;
                }
            }
        } else {
            $CPAN::Frontend->mywarn("  Could not determine host from http_proxy '$http_proxy'\n");
        }
        if ($want_proxy) {
            my($user, $pass) =
                CPAN::HTTP::Credentials->get_proxy_credentials();
            $ret = {
                    proxy_user => $user,
                    proxy_pass => $pass,
                    http_proxy => $http_proxy
                  };
        }
    }
    return $ret;
}

# package CPAN::FTP;
sub hostdlhardest {
    my($self,$host_seq,$file,$aslocal,$stats) = @_;

    return unless @$host_seq;
    my($ro_url);
    my($aslocal_dir) = dirname($aslocal);
    mkpath($aslocal_dir);
    my $ftpbin = $CPAN::Config->{ftp};
    unless ($ftpbin && length $ftpbin && MM->maybe_command($ftpbin)) {
        $CPAN::Frontend->myprint("No external ftp command available\n\n");
        return;
    }
    $CPAN::Frontend->mywarn(qq{
As a last resort we now switch to the external ftp command '$ftpbin'
to get '$aslocal'.

Doing so often leads to problems that are hard to diagnose.

If you're the victim of such problems, please consider unsetting the
ftp config variable with

    o conf ftp ""
    o conf commit

});
    $CPAN::Frontend->mysleep(2);
  HOSTHARDEST: for $ro_url (@$host_seq) {
        $self->_set_attempt($stats,"dlhardest",$ro_url);
        my $url = "$ro_url$file";
        $self->debug("localizing ftpwise[$url]") if $CPAN::DEBUG;
        unless ($url =~ m|^ftp://(.*?)/(.*)/(.*)|) {
            next;
        }
        my($host,$dir,$getfile) = ($1,$2,$3);
        my $timestamp = 0;
        my($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,
            $ctime,$blksize,$blocks) = stat($aslocal);
        $timestamp = $mtime ||= 0;
        my($netrc) = CPAN::FTP::netrc->new;
        my($netrcfile) = $netrc->netrc;
        my($verbose) = $CPAN::DEBUG{'FTP'} & $CPAN::DEBUG ? " -v" : "";
        my $targetfile = File::Basename::basename($aslocal);
        my(@dialog);
        push(
             @dialog,
             "lcd $aslocal_dir",
             "cd /",
             map("cd $_", split /\//, $dir), # RFC 1738
             "bin",
             "passive",
             "get $getfile $targetfile",
             "quit"
        );
        if (! $netrcfile) {
            CPAN->debug("No ~/.netrc file found") if $CPAN::DEBUG;
        } elsif ($netrc->hasdefault || $netrc->contains($host)) {
            CPAN->debug(sprintf("hasdef[%d]cont($host)[%d]",
                                $netrc->hasdefault,
                                $netrc->contains($host))) if $CPAN::DEBUG;
            if ($netrc->protected) {
                my $dialog = join "", map { "    $_\n" } @dialog;
                my $netrc_explain;
                if ($netrc->contains($host)) {
                    $netrc_explain = "Relying that your .netrc entry for '$host' ".
                        "manages the login";
                } else {
                    $netrc_explain = "Relying that your default .netrc entry ".
                        "manages the login";
                }
                $CPAN::Frontend->myprint(qq{
  Trying with external ftp to get
    '$url'
  $netrc_explain
  Sending the dialog
$dialog
}
                );
                $self->talk_ftp("$ftpbin$verbose $host",
                                @dialog);
                ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
                    $atime,$mtime,$ctime,$blksize,$blocks) = stat($aslocal);
                $mtime ||= 0;
                if ($mtime > $timestamp) {
                    $CPAN::Frontend->myprint("GOT $aslocal\n");
                    $ThesiteURL = $ro_url;
                    return $aslocal;
                } else {
                    $CPAN::Frontend->myprint("Hmm... Still failed!\n");
                }
                    return if $CPAN::Signal;
            } else {
                $CPAN::Frontend->mywarn(qq{Your $netrcfile is not }.
                                        qq{correctly protected.\n});
            }
        } else {
            $CPAN::Frontend->mywarn("Your ~/.netrc neither contains $host
  nor does it have a default entry\n");
        }

        # OK, they don't have a valid ~/.netrc. Use 'ftp -n'
        # then and login manually to host, using e-mail as
        # password.
        $CPAN::Frontend->myprint(qq{Issuing "$ftpbin$verbose -n"\n});
        unshift(
                @dialog,
                "open $host",
                "user anonymous $Config::Config{'cf_email'}"
        );
        my $dialog = join "", map { "    $_\n" } @dialog;
        $CPAN::Frontend->myprint(qq{
  Trying with external ftp to get
    $url
  Sending the dialog
$dialog
}
        );
        $self->talk_ftp("$ftpbin$verbose -n", @dialog);
        ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
            $atime,$mtime,$ctime,$blksize,$blocks) = stat($aslocal);
        $mtime ||= 0;
        if ($mtime > $timestamp) {
            $CPAN::Frontend->myprint("GOT $aslocal\n");
            $ThesiteURL = $ro_url;
            return $aslocal;
        } else {
            $CPAN::Frontend->myprint("Bad luck... Still failed!\n");
        }
        return if $CPAN::Signal;
        $CPAN::Frontend->mywarn("Can't access URL $url.\n\n");
        $CPAN::Frontend->mysleep(2);
    } # host
}

# package CPAN::FTP;
sub talk_ftp {
    my($self,$command,@dialog) = @_;
    my $fh = FileHandle->new;
    $fh->open("|$command") or die "Couldn't open ftp: $!";
    foreach (@dialog) { $fh->print("$_\n") }
    $fh->close; # Wait for process to complete
    my $wstatus = $?;
    my $estatus = $wstatus >> 8;
    $CPAN::Frontend->myprint(qq{
Subprocess "|$command"
  returned status $estatus (wstat $wstatus)
}) if $wstatus;
}

# find2perl needs modularization, too, all the following is stolen
# from there
# CPAN::FTP::ls
sub ls {
    my($self,$name) = @_;
    my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$sizemm,
     $atime,$mtime,$ctime,$blksize,$blocks) = lstat($name);

    my($perms,%user,%group);
    my $pname = $name;

    if ($blocks) {
        $blocks = int(($blocks + 1) / 2);
    }
    else {
        $blocks = int(($sizemm + 1023) / 1024);
    }

    if    (-f _) { $perms = '-'; }
    elsif (-d _) { $perms = 'd'; }
    elsif (-c _) { $perms = 'c'; $sizemm = &sizemm; }
    elsif (-b _) { $perms = 'b'; $sizemm = &sizemm; }
    elsif (-p _) { $perms = 'p'; }
    elsif (-S _) { $perms = 's'; }
    else         { $perms = 'l'; $pname .= ' -> ' . readlink($_); }

    my(@rwx) = ('---','--x','-w-','-wx','r--','r-x','rw-','rwx');
    my(@moname) = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
    my $tmpmode = $mode;
    my $tmp = $rwx[$tmpmode & 7];
    $tmpmode >>= 3;
    $tmp = $rwx[$tmpmode & 7] . $tmp;
    $tmpmode >>= 3;
    $tmp = $rwx[$tmpmode & 7] . $tmp;
    substr($tmp,2,1) =~ tr/-x/Ss/ if -u _;
    substr($tmp,5,1) =~ tr/-x/Ss/ if -g _;
    substr($tmp,8,1) =~ tr/-x/Tt/ if -k _;
    $perms .= $tmp;

    my $user = $user{$uid} || $uid;   # too lazy to implement lookup
    my $group = $group{$gid} || $gid;

    my($sec,$min,$hour,$mday,$mon,$year) = localtime($mtime);
    my($timeyear);
    my($moname) = $moname[$mon];
    if (-M _ > 365.25 / 2) {
        $timeyear = $year + 1900;
    }
    else {
        $timeyear = sprintf("%02d:%02d", $hour, $min);
    }

    sprintf "%5lu %4ld %-10s %2d %-8s %-8s %8s %s %2d %5s %s\n",
             $ino,
                  $blocks,
                       $perms,
                             $nlink,
                                 $user,
                                      $group,
                                           $sizemm,
                                               $moname,
                                                  $mday,
                                                      $timeyear,
                                                          $pname;
}

1;
