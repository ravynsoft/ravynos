# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
package CPAN::Tarzip;
use strict;
use vars qw($VERSION @ISA $BUGHUNTING);
use CPAN::Debug;
use File::Basename qw(basename);
$VERSION = "5.5013";
# module is internal to CPAN.pm

@ISA = qw(CPAN::Debug); ## no critic
$BUGHUNTING ||= 0; # released code must have turned off

# it's ok if file doesn't exist, it just matters if it is .gz or .bz2
sub new {
    my($class,$file) = @_;
    $CPAN::Frontend->mydie("CPAN::Tarzip->new called without arg") unless defined $file;
    my $me = { FILE => $file };
    if ($file =~ /\.(bz2|gz|zip|tbz|tgz)$/i) {
        $me->{ISCOMPRESSED} = 1;
    } else {
        $me->{ISCOMPRESSED} = 0;
    }
    if (0) {
    } elsif ($file =~ /\.(?:bz2|tbz)$/i) {
        unless ($me->{UNGZIPPRG} = $CPAN::Config->{bzip2}) {
            my $bzip2 = _my_which("bzip2");
            if ($bzip2) {
                $me->{UNGZIPPRG} = $bzip2;
            } else {
                $CPAN::Frontend->mydie(qq{
CPAN.pm needs the external program bzip2 in order to handle '$file'.
Please install it now and run 'o conf init bzip2' from the
CPAN shell prompt to register it as external program.
});
            }
        }
    } else {
        $me->{UNGZIPPRG} = _my_which("gzip");
    }
    $me->{TARPRG} = _my_which("tar") || _my_which("gtar");
    bless $me, $class;
}

sub _zlib_ok () {
    $CPAN::META->has_inst("Compress::Zlib") or return;
    Compress::Zlib->can('gzopen');
}

sub _my_which {
    my($what) = @_;
    if ($CPAN::Config->{$what}) {
        return $CPAN::Config->{$what};
    }
    if ($CPAN::META->has_inst("File::Which")) {
        return File::Which::which($what);
    }
    my @cand = MM->maybe_command($what);
    return $cand[0] if @cand;
    require File::Spec;
    my $component;
  PATH_COMPONENT: foreach $component (File::Spec->path()) {
        next unless defined($component) && $component;
        my($abs) = File::Spec->catfile($component,$what);
        if (MM->maybe_command($abs)) {
            return $abs;
        }
    }
    return;
}

sub gzip {
    my($self,$read) = @_;
    my $write = $self->{FILE};
    if (_zlib_ok) {
        my($buffer,$fhw);
        $fhw = FileHandle->new($read)
            or $CPAN::Frontend->mydie("Could not open $read: $!");
        my $cwd = `pwd`;
        my $gz = Compress::Zlib::gzopen($write, "wb")
            or $CPAN::Frontend->mydie("Cannot gzopen $write: $! (pwd is $cwd)\n");
        binmode($fhw);
        $gz->gzwrite($buffer)
            while read($fhw,$buffer,4096) > 0 ;
        $gz->gzclose() ;
        $fhw->close;
        return 1;
    } else {
        my $command = CPAN::HandleConfig->safe_quote($self->{UNGZIPPRG});
        system(qq{$command -c "$read" > "$write"})==0;
    }
}


sub gunzip {
    my($self,$write) = @_;
    my $read = $self->{FILE};
    if (_zlib_ok) {
        my($buffer,$fhw);
        $fhw = FileHandle->new(">$write")
            or $CPAN::Frontend->mydie("Could not open >$write: $!");
        my $gz = Compress::Zlib::gzopen($read, "rb")
            or $CPAN::Frontend->mydie("Cannot gzopen $read: $!\n");
        binmode($fhw);
        $fhw->print($buffer)
            while $gz->gzread($buffer) > 0 ;
        $CPAN::Frontend->mydie("Error reading from $read: $!\n")
            if $gz->gzerror != Compress::Zlib::Z_STREAM_END();
        $gz->gzclose() ;
        $fhw->close;
        return 1;
    } else {
        my $command = CPAN::HandleConfig->safe_quote($self->{UNGZIPPRG});
        system(qq{$command -d -c "$read" > "$write"})==0;
    }
}


sub gtest {
    my($self) = @_;
    return $self->{GTEST} if exists $self->{GTEST};
    defined $self->{FILE} or $CPAN::Frontend->mydie("gtest called but no FILE specified");
    my $read = $self->{FILE};
    my $success;
    if ($read=~/\.(?:bz2|tbz)$/ && $CPAN::META->has_inst("Compress::Bzip2")) {
        my($buffer,$len);
        $len = 0;
        my $gz = Compress::Bzip2::bzopen($read, "rb")
            or $CPAN::Frontend->mydie(sprintf("Cannot bzopen %s: %s\n",
                                              $read,
                                              $Compress::Bzip2::bzerrno));
        while ($gz->bzread($buffer) > 0 ) {
            $len += length($buffer);
            $buffer = "";
        }
        my $err = $gz->bzerror;
        $success = ! $err || $err == Compress::Bzip2::BZ_STREAM_END();
        if ($len == -s $read) {
            $success = 0;
            CPAN->debug("hit an uncompressed file") if $CPAN::DEBUG;
        }
        $gz->gzclose();
        CPAN->debug("err[$err]success[$success]") if $CPAN::DEBUG;
    } elsif ( $read=~/\.(?:gz|tgz)$/ && _zlib_ok ) {
        # After I had reread the documentation in zlib.h, I discovered that
        # uncompressed files do not lead to an gzerror (anymore?).
        my($buffer,$len);
        $len = 0;
        my $gz = Compress::Zlib::gzopen($read, "rb")
            or $CPAN::Frontend->mydie(sprintf("Cannot gzopen %s: %s\n",
                                              $read,
                                              $Compress::Zlib::gzerrno));
        while ($gz->gzread($buffer) > 0 ) {
            $len += length($buffer);
            $buffer = "";
        }
        my $err = $gz->gzerror;
        $success = ! $err || $err == Compress::Zlib::Z_STREAM_END();
        if ($len == -s $read) {
            $success = 0;
            CPAN->debug("hit an uncompressed file") if $CPAN::DEBUG;
        }
        $gz->gzclose();
        CPAN->debug("err[$err]success[$success]") if $CPAN::DEBUG;
    } elsif (!$self->{ISCOMPRESSED}) {
        $success = 0;
    } else {
        my $command = CPAN::HandleConfig->safe_quote($self->{UNGZIPPRG});
        $success = 0==system(qq{$command -qdt "$read"});
    }
    return $self->{GTEST} = $success;
}


sub TIEHANDLE {
    my($class,$file) = @_;
    my $ret;
    $class->debug("file[$file]");
    my $self = $class->new($file);
    if (0) {
    } elsif (!$self->gtest) {
        my $fh = FileHandle->new($file)
            or $CPAN::Frontend->mydie("Could not open file[$file]: $!");
        binmode $fh;
        $self->{FH} = $fh;
        $class->debug("via uncompressed FH");
    } elsif ($file =~ /\.(?:bz2|tbz)$/ && $CPAN::META->has_inst("Compress::Bzip2")) {
        my $gz = Compress::Bzip2::bzopen($file,"rb") or
            $CPAN::Frontend->mydie("Could not bzopen $file");
        $self->{GZ} = $gz;
        $class->debug("via Compress::Bzip2");
    } elsif ($file =~/\.(?:gz|tgz)$/ && _zlib_ok) {
        my $gz = Compress::Zlib::gzopen($file,"rb") or
            $CPAN::Frontend->mydie("Could not gzopen $file");
        $self->{GZ} = $gz;
        $class->debug("via Compress::Zlib");
    } else {
        my $gzip = CPAN::HandleConfig->safe_quote($self->{UNGZIPPRG});
        my $pipe = "$gzip -d -c $file |";
        my $fh = FileHandle->new($pipe) or $CPAN::Frontend->mydie("Could not pipe[$pipe]: $!");
        binmode $fh;
        $self->{FH} = $fh;
        $class->debug("via external $gzip");
    }
    $self;
}


sub READLINE {
    my($self) = @_;
    if (exists $self->{GZ}) {
        my $gz = $self->{GZ};
        my($line,$bytesread);
        $bytesread = $gz->gzreadline($line);
        return undef if $bytesread <= 0;
        return $line;
    } else {
        my $fh = $self->{FH};
        return scalar <$fh>;
    }
}


sub READ {
    my($self,$ref,$length,$offset) = @_;
    $CPAN::Frontend->mydie("read with offset not implemented") if defined $offset;
    if (exists $self->{GZ}) {
        my $gz = $self->{GZ};
        my $byteread = $gz->gzread($$ref,$length);# 30eaf79e8b446ef52464b5422da328a8
        return $byteread;
    } else {
        my $fh = $self->{FH};
        return read($fh,$$ref,$length);
    }
}


sub DESTROY {
    my($self) = @_;
    if (exists $self->{GZ}) {
        my $gz = $self->{GZ};
        $gz->gzclose() if defined $gz; # hard to say if it is allowed
                                       # to be undef ever. AK, 2000-09
    } else {
        my $fh = $self->{FH};
        $fh->close if defined $fh;
    }
    undef $self;
}

sub untar {
    my($self) = @_;
    my $file = $self->{FILE};
    my($prefer) = 0;

    my $exttar = $self->{TARPRG} || "";
    $exttar = "" if $exttar =~ /^\s+$/; # user refuses to use it
    my $extgzip = $self->{UNGZIPPRG} || "";
    $extgzip = "" if $extgzip =~ /^\s+$/; # user refuses to use it

    if (0) { # makes changing order easier
    } elsif ($BUGHUNTING) {
        $prefer=2;
    } elsif ($CPAN::Config->{prefer_external_tar}) {
        $prefer = 1;
    } elsif (
             $CPAN::META->has_usable("Archive::Tar")
             &&
             _zlib_ok ) {
        my $prefer_external_tar = $CPAN::Config->{prefer_external_tar};
        unless (defined $prefer_external_tar) {
            if ($^O =~ /(MSWin32|solaris)/) {
                $prefer_external_tar = 0;
            } else {
                $prefer_external_tar = 1;
            }
        }
        $prefer = $prefer_external_tar ? 1 : 2;
    } elsif ($exttar && $extgzip) {
        # no modules and not bz2
        $prefer = 1;
        # but solaris binary tar is a problem
        if ($^O eq 'solaris' && qx($exttar --version 2>/dev/null) !~ /gnu/i) {
            $CPAN::Frontend->mywarn(<< 'END_WARN');

WARNING: Many CPAN distributions were archived with GNU tar and some of
them may be incompatible with Solaris tar.  We respectfully suggest you
configure CPAN to use a GNU tar instead ("o conf init tar") or install
a recent Archive::Tar instead;

END_WARN
        }
    } else {
        my $foundtar = $exttar ? "'$exttar'" : "nothing";
        my $foundzip = $extgzip ? "'$extgzip'" : $foundtar ? "nothing" : "also nothing";
        my $foundAT;
        if ($CPAN::META->has_usable("Archive::Tar")) {
            $foundAT = sprintf "'%s'", "Archive::Tar::"->VERSION;
        } else {
            $foundAT = "nothing";
        }
        my $foundCZ;
        if (_zlib_ok) {
            $foundCZ = sprintf "'%s'", "Compress::Zlib::"->VERSION;
        } elsif ($foundAT) {
            $foundCZ = "nothing";
        } else {
            $foundCZ = "also nothing";
        }
        $CPAN::Frontend->mydie(qq{

CPAN.pm needs either the external programs tar and gzip -or- both
modules Archive::Tar and Compress::Zlib installed.

For tar I found $foundtar, for gzip $foundzip.

For Archive::Tar I found $foundAT, for Compress::Zlib $foundCZ;

Can't continue cutting file '$file'.
});
    }
    my $tar_verb = "v";
    if (defined $CPAN::Config->{tar_verbosity}) {
        $tar_verb = $CPAN::Config->{tar_verbosity} eq "none" ? "" :
            $CPAN::Config->{tar_verbosity};
    }
    if ($prefer==1) { # 1 => external gzip+tar
        my($system);
        my $is_compressed = $self->gtest();
        my $tarcommand = CPAN::HandleConfig->safe_quote($exttar);
        if ($is_compressed) {
            my $command = CPAN::HandleConfig->safe_quote($extgzip);
            $system = qq{$command -d -c }.
                qq{< "$file" | $tarcommand x${tar_verb}f -};
        } else {
            $system = qq{$tarcommand x${tar_verb}f "$file"};
        }
        if (system($system) != 0) {
            # people find the most curious tar binaries that cannot handle
            # pipes
            if ($is_compressed) {
                (my $ungzf = $file) =~ s/\.gz(?!\n)\Z//;
                $ungzf = basename $ungzf;
                my $ct = CPAN::Tarzip->new($file);
                if ($ct->gunzip($ungzf)) {
                    $CPAN::Frontend->myprint(qq{Uncompressed $file successfully\n});
                } else {
                    $CPAN::Frontend->mydie(qq{Couldn\'t uncompress $file\n});
                }
                $file = $ungzf;
            }
            $system = qq{$tarcommand x${tar_verb}f "$file"};
            $CPAN::Frontend->myprint(qq{Using Tar:$system:\n});
            my $ret = system($system);
            if ($ret==0) {
                $CPAN::Frontend->myprint(qq{Untarred $file successfully\n});
            } else {
                if ($? == -1) {
                    $CPAN::Frontend->mydie(sprintf qq{Couldn\'t untar %s: '%s'\n},
                                           $file, $!);
                } elsif ($? & 127) {
                    $CPAN::Frontend->mydie(sprintf qq{Couldn\'t untar %s: child died with signal %d, %s coredump\n},
                                           $file, ($? & 127),  ($? & 128) ? 'with' : 'without');
                } else {
                    $CPAN::Frontend->mydie(sprintf qq{Couldn\'t untar %s: child exited with value %d\n},
                                           $file, $? >> 8);
                }
            }
            return 1;
        } else {
            return 1;
        }
    } elsif ($prefer==2) { # 2 => modules
        unless ($CPAN::META->has_usable("Archive::Tar")) {
            $CPAN::Frontend->mydie("Archive::Tar not installed, please install it to continue");
        }
        # Make sure AT does not use uid/gid/permissions in the archive
        # This leaves it to the user's umask instead
        local $Archive::Tar::CHMOD = 1;
        local $Archive::Tar::SAME_PERMISSIONS = 0;
        # Make sure AT leaves current user as owner
        local $Archive::Tar::CHOWN = 0;
        my $tar = Archive::Tar->new($file,1);
        my $af; # archive file
        my @af;
        if ($BUGHUNTING) {
            # RCS 1.337 had this code, it turned out unacceptable slow but
            # it revealed a bug in Archive::Tar. Code is only here to hunt
            # the bug again. It should never be enabled in published code.
            # GDGraph3d-0.53 was an interesting case according to Larry
            # Virden.
            warn(">>>Bughunting code enabled<<< " x 20);
            for $af ($tar->list_files) {
                if ($af =~ m!^(/|\.\./)!) {
                    $CPAN::Frontend->mydie("ALERT: Archive contains ".
                                           "illegal member [$af]");
                }
                $CPAN::Frontend->myprint("$af\n");
                $tar->extract($af); # slow but effective for finding the bug
                return if $CPAN::Signal;
            }
        } else {
            for $af ($tar->list_files) {
                if ($af =~ m!^(/|\.\./)!) {
                    $CPAN::Frontend->mydie("ALERT: Archive contains ".
                                           "illegal member [$af]");
                }
                if ($tar_verb eq "v" || $tar_verb eq "vv") {
                    $CPAN::Frontend->myprint("$af\n");
                }
                push @af, $af;
                return if $CPAN::Signal;
            }
            $tar->extract(@af) or
                $CPAN::Frontend->mydie("Could not untar with Archive::Tar.");
        }

        Mac::BuildTools::convert_files([$tar->list_files], 1)
            if ($^O eq 'MacOS');

        return 1;
    }
}

sub unzip {
    my($self) = @_;
    my $file = $self->{FILE};
    if ($CPAN::META->has_inst("Archive::Zip")) {
        # blueprint of the code from Archive::Zip::Tree::extractTree();
        my $zip = Archive::Zip->new();
        my $status;
        $status = $zip->read($file);
        $CPAN::Frontend->mydie("Read of file[$file] failed\n")
            if $status != Archive::Zip::AZ_OK();
        $CPAN::META->debug("Successfully read file[$file]") if $CPAN::DEBUG;
        my @members = $zip->members();
        for my $member ( @members ) {
            my $af = $member->fileName();
            if ($af =~ m!^(/|\.\./)!) {
                $CPAN::Frontend->mydie("ALERT: Archive contains ".
                                       "illegal member [$af]");
            }
            $status = $member->extractToFileNamed( $af );
            $CPAN::META->debug("af[$af]status[$status]") if $CPAN::DEBUG;
            $CPAN::Frontend->mydie("Extracting of file[$af] from zipfile[$file] failed\n") if
                $status != Archive::Zip::AZ_OK();
            return if $CPAN::Signal;
        }
        return 1;
    } elsif ( my $unzip = $CPAN::Config->{unzip}  ) {
        my @system = ($unzip, $file);
        return system(@system) == 0;
    }
    else {
            $CPAN::Frontend->mydie(<<"END");

Can't unzip '$file':

You have not configured an 'unzip' program and do not have Archive::Zip
installed.  Please either install Archive::Zip or else configure 'unzip'
by running the command 'o conf init unzip' from the CPAN shell prompt.

END
    }
}

1;

__END__

=head1 NAME

CPAN::Tarzip - internal handling of tar archives for CPAN.pm

=head1 LICENSE

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
