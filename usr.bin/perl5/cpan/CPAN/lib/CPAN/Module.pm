# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Module;
use strict;
@CPAN::Module::ISA = qw(CPAN::InfoObj);

use vars qw(
            $VERSION
);
$VERSION = "5.5003";

BEGIN {
    # alarm() is not implemented in perl 5.6.x and earlier under Windows
    *ALARM_IMPLEMENTED = sub () { $] >= 5.007 || $^O !~ /MSWin/ };
}

# Accessors
#-> sub CPAN::Module::userid
sub userid {
    my $self = shift;
    my $ro = $self->ro;
    return unless $ro;
    return $ro->{userid} || $ro->{CPAN_USERID};
}
#-> sub CPAN::Module::description
sub description {
    my $self = shift;
    my $ro = $self->ro or return "";
    $ro->{description}
}

#-> sub CPAN::Module::distribution
sub distribution {
    my($self) = @_;
    CPAN::Shell->expand("Distribution",$self->cpan_file);
}

#-> sub CPAN::Module::_is_representative_module
sub _is_representative_module {
    my($self) = @_;
    return $self->{_is_representative_module} if defined $self->{_is_representative_module};
    my $pm = $self->cpan_file or return $self->{_is_representative_module} = 0;
    $pm =~ s|.+/||;
    $pm =~ s{\.(?:tar\.(bz2|gz|Z)|t(?:gz|bz)|zip)$}{}i; # see base_id
    $pm =~ s|-\d+\.\d+.+$||;
    $pm =~ s|-[\d\.]+$||;
    $pm =~ s/-/::/g;
    $self->{_is_representative_module} = $pm eq $self->{ID} ? 1 : 0;
    # warn "DEBUG: $pm eq $self->{ID} => $self->{_is_representative_module}";
    $self->{_is_representative_module};
}

#-> sub CPAN::Module::undelay
sub undelay {
    my $self = shift;
    delete $self->{later};
    if ( my $dist = CPAN::Shell->expand("Distribution", $self->cpan_file) ) {
        $dist->undelay;
    }
}

# mark as dirty/clean
#-> sub CPAN::Module::color_cmd_tmps ;
sub color_cmd_tmps {
    my($self) = shift;
    my($depth) = shift || 0;
    my($color) = shift || 0;
    my($ancestors) = shift || [];
    # a module needs to recurse to its cpan_file

    return if exists $self->{incommandcolor}
        && $color==1
        && $self->{incommandcolor}==$color;
    return if $color==0 && !$self->{incommandcolor};
    if ($color>=1) {
        if ( $self->uptodate ) {
            $self->{incommandcolor} = $color;
            return;
        } elsif (my $have_version = $self->available_version) {
            # maybe what we have is good enough
            if (@$ancestors) {
                my $who_asked_for_me = $ancestors->[-1];
                my $obj = CPAN::Shell->expandany($who_asked_for_me);
                if (0) {
                } elsif ($obj->isa("CPAN::Bundle")) {
                    # bundles cannot specify a minimum version
                    return;
                } elsif ($obj->isa("CPAN::Distribution")) {
                    if (my $prereq_pm = $obj->prereq_pm) {
                        for my $k (keys %$prereq_pm) {
                            if (my $want_version = $prereq_pm->{$k}{$self->id}) {
                                if (CPAN::Version->vcmp($have_version,$want_version) >= 0) {
                                    $self->{incommandcolor} = $color;
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        $self->{incommandcolor} = $color; # set me before recursion,
                                          # so we can break it
    }
    if ($depth>=$CPAN::MAX_RECURSION) {
        my $e = CPAN::Exception::RecursiveDependency->new($ancestors);
        if ($e->is_resolvable) {
            return $self->{incommandcolor}=2;
        } else {
            die $e;
        }
    }
    # warn "color_cmd_tmps $depth $color " . $self->id; # sleep 1;

    if ( my $dist = CPAN::Shell->expand("Distribution", $self->cpan_file) ) {
        $dist->color_cmd_tmps($depth+1,$color,[@$ancestors, $self->id]);
    }
    # unreached code?
    # if ($color==0) {
    #    delete $self->{badtestcnt};
    # }
    $self->{incommandcolor} = $color;
}

#-> sub CPAN::Module::as_glimpse ;
sub as_glimpse {
    my($self) = @_;
    my(@m);
    my $class = ref($self);
    $class =~ s/^CPAN:://;
    my $color_on = "";
    my $color_off = "";
    if (
        $CPAN::Shell::COLOR_REGISTERED
        &&
        $CPAN::META->has_inst("Term::ANSIColor")
        &&
        $self->description
       ) {
        $color_on = Term::ANSIColor::color("green");
        $color_off = Term::ANSIColor::color("reset");
    }
    my $uptodateness = " ";
    unless ($class eq "Bundle") {
        my $u = $self->uptodate;
        $uptodateness = $u ? "=" : "<" if defined $u;
    };
    my $id = do {
        my $d = $self->distribution;
        $d ? $d -> pretty_id : $self->cpan_userid;
    };
    push @m, sprintf("%-7s %1s %s%-22s%s (%s)\n",
                     $class,
                     $uptodateness,
                     $color_on,
                     $self->id,
                     $color_off,
                     $id,
                    );
    join "", @m;
}

#-> sub CPAN::Module::dslip_status
sub dslip_status {
    my($self) = @_;
    my($stat);
    # development status
    @{$stat->{D}}{qw,i c a b R M S,}     = qw,idea
                                              pre-alpha alpha beta released
                                              mature standard,;
    # support level
    @{$stat->{S}}{qw,m d u n a,}         = qw,mailing-list
                                              developer comp.lang.perl.*
                                              none abandoned,;
    # language
    @{$stat->{L}}{qw,p c + o h,}         = qw,perl C C++ other hybrid,;
    # interface
    @{$stat->{I}}{qw,f r O p h n,}       = qw,functions
                                              references+ties
                                              object-oriented pragma
                                              hybrid none,;
    # public licence
    @{$stat->{P}}{qw,p g l b a 2 o d r n,} = qw,Standard-Perl
                                              GPL LGPL
                                              BSD Artistic Artistic_2
                                              open-source
                                              distribution_allowed
                                              restricted_distribution
                                              no_licence,;
    for my $x (qw(d s l i p)) {
        $stat->{$x}{' '} = 'unknown';
        $stat->{$x}{'?'} = 'unknown';
    }
    my $ro = $self->ro;
    return +{} unless $ro && $ro->{statd};
    return {
            D  => $ro->{statd},
            S  => $ro->{stats},
            L  => $ro->{statl},
            I  => $ro->{stati},
            P  => $ro->{statp},
            DV => $stat->{D}{$ro->{statd}},
            SV => $stat->{S}{$ro->{stats}},
            LV => $stat->{L}{$ro->{statl}},
            IV => $stat->{I}{$ro->{stati}},
            PV => $stat->{P}{$ro->{statp}},
           };
}

#-> sub CPAN::Module::as_string ;
sub as_string {
    my($self) = @_;
    my(@m);
    CPAN->debug("$self entering as_string") if $CPAN::DEBUG;
    my $class = ref($self);
    $class =~ s/^CPAN:://;
    local($^W) = 0;
    push @m, $class, " id = $self->{ID}\n";
    my $sprintf = "    %-12s %s\n";
    push @m, sprintf($sprintf, 'DESCRIPTION', $self->description)
        if $self->description;
    my $sprintf2 = "    %-12s %s (%s)\n";
    my($userid);
    $userid = $self->userid;
    if ( $userid ) {
        my $author;
        if ($author = CPAN::Shell->expand('Author',$userid)) {
            my $email = "";
            my $m; # old perls
            if ($m = $author->email) {
                $email = " <$m>";
            }
            push @m, sprintf(
                             $sprintf2,
                             'CPAN_USERID',
                             $userid,
                             $author->fullname . $email
                            );
        }
    }
    push @m, sprintf($sprintf, 'CPAN_VERSION', $self->cpan_version)
        if $self->cpan_version;
    if (my $cpan_file = $self->cpan_file) {
        push @m, sprintf($sprintf, 'CPAN_FILE', $cpan_file);
        if (my $dist = CPAN::Shell->expand("Distribution",$cpan_file)) {
            my $upload_date = $dist->upload_date;
            if ($upload_date) {
                push @m, sprintf($sprintf, 'UPLOAD_DATE', $upload_date);
            }
        }
    }
    my $sprintf3 = "    %-12s %1s%1s%1s%1s%1s (%s,%s,%s,%s,%s)\n";
    my $dslip = $self->dslip_status;
    push @m, sprintf(
                     $sprintf3,
                     'DSLIP_STATUS',
                     @{$dslip}{qw(D S L I P DV SV LV IV PV)},
                    ) if $dslip->{D};
    my $local_file = $self->inst_file;
    unless ($self->{MANPAGE}) {
        my $manpage;
        if ($local_file) {
            $manpage = $self->manpage_headline($local_file);
        } else {
            # If we have already untarred it, we should look there
            my $dist = $CPAN::META->instance('CPAN::Distribution',
                                             $self->cpan_file);
            # warn "dist[$dist]";
            # mff=manifest file; mfh=manifest handle
            my($mff,$mfh);
            if (
                $dist->{build_dir}
                and
                (-f  ($mff = File::Spec->catfile($dist->{build_dir}, "MANIFEST")))
                and
                $mfh = FileHandle->new($mff)
               ) {
                CPAN->debug("mff[$mff]") if $CPAN::DEBUG;
                my $lfre = $self->id; # local file RE
                $lfre =~ s/::/./g;
                $lfre .= "\\.pm\$";
                my($lfl); # local file file
                local $/ = "\n";
                my(@mflines) = <$mfh>;
                for (@mflines) {
                    s/^\s+//;
                    s/\s.*//s;
                }
                while (length($lfre)>5 and !$lfl) {
                    ($lfl) = grep /$lfre/, @mflines;
                    CPAN->debug("lfl[$lfl]lfre[$lfre]") if $CPAN::DEBUG;
                    $lfre =~ s/.+?\.//;
                }
                $lfl =~ s/\s.*//; # remove comments
                $lfl =~ s/\s+//g; # chomp would maybe be too system-specific
                my $lfl_abs = File::Spec->catfile($dist->{build_dir},$lfl);
                # warn "lfl_abs[$lfl_abs]";
                if (-f $lfl_abs) {
                    $manpage = $self->manpage_headline($lfl_abs);
                }
            }
        }
        $self->{MANPAGE} = $manpage if $manpage;
    }
    my($item);
    for $item (qw/MANPAGE/) {
        push @m, sprintf($sprintf, $item, $self->{$item})
            if exists $self->{$item};
    }
    for $item (qw/CONTAINS/) {
        push @m, sprintf($sprintf, $item, join(" ",@{$self->{$item}}))
            if exists $self->{$item} && @{$self->{$item}};
    }
    push @m, sprintf($sprintf, 'INST_FILE',
                     $local_file || "(not installed)");
    push @m, sprintf($sprintf, 'INST_VERSION',
                     $self->inst_version) if $local_file;
    if (%{$CPAN::META->{is_tested}||{}}) { # XXX needs to be methodified somehow
        my $available_file = $self->available_file;
        if ($available_file && $available_file ne $local_file) {
            push @m, sprintf($sprintf, 'AVAILABLE_FILE', $available_file);
            push @m, sprintf($sprintf, 'AVAILABLE_VERSION', $self->available_version);
        }
    }
    join "", @m, "\n";
}

#-> sub CPAN::Module::manpage_headline
sub manpage_headline {
    my($self,$local_file) = @_;
    my(@local_file) = $local_file;
    $local_file =~ s/\.pm(?!\n)\Z/.pod/;
    push @local_file, $local_file;
    my(@result,$locf);
    for $locf (@local_file) {
        next unless -f $locf;
        my $fh = FileHandle->new($locf)
            or $Carp::Frontend->mydie("Couldn't open $locf: $!");
        my $inpod = 0;
        local $/ = "\n";
        while (<$fh>) {
            $inpod = m/^=(?!head1\s+NAME\s*$)/ ? 0 :
                m/^=head1\s+NAME\s*$/ ? 1 : $inpod;
            next unless $inpod;
            next if /^=/;
            next if /^\s+$/;
            chomp;
            push @result, $_;
        }
        close $fh;
        last if @result;
    }
    for (@result) {
        s/^\s+//;
        s/\s+$//;
    }
    join " ", @result;
}

#-> sub CPAN::Module::cpan_file ;
# Note: also inherited by CPAN::Bundle
sub cpan_file {
    my $self = shift;
    # CPAN->debug(sprintf "id[%s]", $self->id) if $CPAN::DEBUG;
    unless ($self->ro) {
        CPAN::Index->reload;
    }
    my $ro = $self->ro;
    if ($ro && defined $ro->{CPAN_FILE}) {
        return $ro->{CPAN_FILE};
    } else {
        my $userid = $self->userid;
        if ( $userid ) {
            if ($CPAN::META->exists("CPAN::Author",$userid)) {
                my $author = $CPAN::META->instance("CPAN::Author",
                                                   $userid);
                my $fullname = $author->fullname;
                my $email = $author->email;
                unless (defined $fullname && defined $email) {
                    return sprintf("Contact Author %s",
                                   $userid,
                                  );
                }
                return "Contact Author $fullname <$email>";
            } else {
                return "Contact Author $userid (Email address not available)";
            }
        } else {
            return "N/A";
        }
    }
}

#-> sub CPAN::Module::cpan_version ;
sub cpan_version {
    my $self = shift;

    my $ro = $self->ro;
    unless ($ro) {
        # Can happen with modules that are not on CPAN
        $ro = {};
    }
    $ro->{CPAN_VERSION} = 'undef'
        unless defined $ro->{CPAN_VERSION};
    $ro->{CPAN_VERSION};
}

#-> sub CPAN::Module::force ;
sub force {
    my($self) = @_;
    $self->{force_update} = 1;
}

#-> sub CPAN::Module::fforce ;
sub fforce {
    my($self) = @_;
    $self->{force_update} = 2;
}

#-> sub CPAN::Module::notest ;
sub notest {
    my($self) = @_;
    # $CPAN::Frontend->mywarn("XDEBUG: set notest for Module");
    $self->{notest}++;
}

#-> sub CPAN::Module::rematein ;
sub rematein {
    my($self,$meth) = @_;
    $CPAN::Frontend->myprint(sprintf("Running %s for module '%s'\n",
                                     $meth,
                                     $self->id));
    my $cpan_file = $self->cpan_file;
    if ($cpan_file eq "N/A" || $cpan_file =~ /^Contact Author/) {
        $CPAN::Frontend->mywarn(sprintf qq{
  The module %s isn\'t available on CPAN.

  Either the module has not yet been uploaded to CPAN, or it is
  temporary unavailable. Please contact the author to find out
  more about the status. Try 'i %s'.
},
                                $self->id,
                                $self->id,
                               );
        return;
    }
    my $pack = $CPAN::META->instance('CPAN::Distribution',$cpan_file);
    $pack->called_for($self->id);
    if (exists $self->{force_update}) {
        if ($self->{force_update} == 2) {
            $pack->fforce($meth);
        } else {
            $pack->force($meth);
        }
    }
    $pack->notest($meth) if exists $self->{notest} && $self->{notest};

    $pack->{reqtype} ||= "";
    CPAN->debug("dist-reqtype[$pack->{reqtype}]".
                "self-reqtype[$self->{reqtype}]") if $CPAN::DEBUG;
        if ($pack->{reqtype}) {
            if ($pack->{reqtype} eq "b" && $self->{reqtype} =~ /^[rc]$/) {
                $pack->{reqtype} = $self->{reqtype};
                if (
                    exists $pack->{install}
                    &&
                    (
                     UNIVERSAL::can($pack->{install},"failed") ?
                     $pack->{install}->failed :
                     $pack->{install} =~ /^NO/
                    )
                   ) {
                    delete $pack->{install};
                    $CPAN::Frontend->mywarn
                        ("Promoting $pack->{ID} from 'build_requires' to 'requires'");
                }
            }
        } else {
            $pack->{reqtype} = $self->{reqtype};
        }

    my $success = eval {
        $pack->$meth();
    };
    my $err = $@;
    $pack->unforce if $pack->can("unforce") && exists $self->{force_update};
    $pack->unnotest if $pack->can("unnotest") && exists $self->{notest};
    delete $self->{force_update};
    delete $self->{notest};
    if ($err) {
        die $err;
    }
    return $success;
}

#-> sub CPAN::Module::perldoc ;
sub perldoc { shift->rematein('perldoc') }
#-> sub CPAN::Module::readme ;
sub readme  { shift->rematein('readme') }
#-> sub CPAN::Module::look ;
sub look    { shift->rematein('look') }
#-> sub CPAN::Module::cvs_import ;
sub cvs_import { shift->rematein('cvs_import') }
#-> sub CPAN::Module::get ;
sub get     { shift->rematein('get',@_) }
#-> sub CPAN::Module::make ;
sub make    { shift->rematein('make') }
#-> sub CPAN::Module::test ;
sub test   {
    my $self = shift;
    # $self->{badtestcnt} ||= 0;
    $self->rematein('test',@_);
}

#-> sub CPAN::Module::deprecated_in_core ;
sub deprecated_in_core {
    my ($self) = @_;
    return unless $CPAN::META->has_inst('Module::CoreList') && Module::CoreList->can('is_deprecated');
    return Module::CoreList::is_deprecated($self->{ID});
}

#-> sub CPAN::Module::inst_deprecated;
# Indicates whether the *installed* version of the module is a deprecated *and*
# installed as part of the Perl core library path
sub inst_deprecated {
    my ($self) = @_;
    my $inst_file = $self->inst_file or return;
    return $self->deprecated_in_core && $self->_in_priv_or_arch($inst_file);
}

#-> sub CPAN::Module::uptodate ;
sub uptodate {
    my ($self) = @_;
    local ($_);
    my $inst = $self->inst_version or return 0;
    my $cpan = $self->cpan_version;
    return 0 if CPAN::Version->vgt($cpan,$inst) || $self->inst_deprecated;
    CPAN->debug
        (join
         ("",
          "returning uptodate. ",
          "cpan[$cpan]inst[$inst]",
         )) if $CPAN::DEBUG;
    return 1;
}

# returns true if installed in privlib or archlib
sub _in_priv_or_arch {
    my($self,$inst_file) = @_;
    foreach my $pair (
        [qw(sitearchexp archlibexp)],
        [qw(sitelibexp privlibexp)]
    ) {
        my ($site, $priv) = @Config::Config{@$pair};
        if ($^O eq 'VMS') {
            for my $d ($site, $priv) { $d = VMS::Filespec::unixify($d) };
        }
        s!/*$!!g foreach $site, $priv;
        next if $site eq $priv;

        if ($priv eq substr($inst_file,0,length($priv))) {
            return 1;
        }
    }
    return 0;
}

#-> sub CPAN::Module::install ;
sub install {
    my($self) = @_;
    my($doit) = 0;
    if ($self->uptodate
        &&
        not exists $self->{force_update}
       ) {
        $CPAN::Frontend->myprint(sprintf("%s is up to date (%s).\n",
                                         $self->id,
                                         $self->inst_version,
                                        ));
    } else {
        $doit = 1;
    }
    my $ro = $self->ro;
    if ($ro && $ro->{stats} && $ro->{stats} eq "a") {
        $CPAN::Frontend->mywarn(qq{
\n\n\n     ***WARNING***
     The module $self->{ID} has no active maintainer (CPAN support level flag 'abandoned').\n\n\n
});
        $CPAN::Frontend->mysleep(5);
    }
    return $doit ? $self->rematein('install') : 1;
}
#-> sub CPAN::Module::clean ;
sub clean  { shift->rematein('clean') }

#-> sub CPAN::Module::inst_file ;
sub inst_file {
    my($self) = @_;
    $self->_file_in_path([@INC]);
}

#-> sub CPAN::Module::available_file ;
sub available_file {
    my($self) = @_;
    my $sep = $Config::Config{path_sep};
    my $perllib = $ENV{PERL5LIB};
    $perllib = $ENV{PERLLIB} unless defined $perllib;
    my @perllib = split(/$sep/,$perllib) if defined $perllib;
    my @cpan_perl5inc;
    if ($CPAN::Perl5lib_tempfile) {
        my $yaml = CPAN->_yaml_loadfile($CPAN::Perl5lib_tempfile);
        @cpan_perl5inc = @{$yaml->[0]{inc} || []};
    }
    $self->_file_in_path([@cpan_perl5inc,@perllib,@INC]);
}

#-> sub CPAN::Module::file_in_path ;
sub _file_in_path {
    my($self,$path) = @_;
    my($dir,@packpath);
    @packpath = split /::/, $self->{ID};
    $packpath[-1] .= ".pm";
    if (@packpath == 1 && $packpath[0] eq "readline.pm") {
        unshift @packpath, "Term", "ReadLine"; # historical reasons
    }
    foreach $dir (@$path) {
        my $pmfile = File::Spec->catfile($dir,@packpath);
        if (-f $pmfile) {
            return $pmfile;
        }
    }
    return;
}

#-> sub CPAN::Module::xs_file ;
sub xs_file {
    my($self) = @_;
    my($dir,@packpath);
    @packpath = split /::/, $self->{ID};
    push @packpath, $packpath[-1];
    $packpath[-1] .= "." . $Config::Config{'dlext'};
    foreach $dir (@INC) {
        my $xsfile = File::Spec->catfile($dir,'auto',@packpath);
        if (-f $xsfile) {
            return $xsfile;
        }
    }
    return;
}

#-> sub CPAN::Module::inst_version ;
sub inst_version {
    my($self) = @_;
    my $parsefile = $self->inst_file or return;
    my $have = $self->parse_version($parsefile);
    $have;
}

#-> sub CPAN::Module::inst_version ;
sub available_version {
    my($self) = @_;
    my $parsefile = $self->available_file or return;
    my $have = $self->parse_version($parsefile);
    $have;
}

#-> sub CPAN::Module::parse_version ;
sub parse_version {
    my($self,$parsefile) = @_;
    if (ALARM_IMPLEMENTED) {
        my $timeout = (exists($CPAN::Config{'version_timeout'}))
                            ? $CPAN::Config{'version_timeout'}
                            : 15;
        alarm($timeout);
    }
    my $have = eval {
        local $SIG{ALRM} = sub { die "alarm\n" };
        MM->parse_version($parsefile);
    };
    if ($@) {
        $CPAN::Frontend->mywarn("Error while parsing version number in file '$parsefile'\n");
    }
    alarm(0) if ALARM_IMPLEMENTED;
    my $leastsanity = eval { defined $have && length $have; };
    $have = "undef" unless $leastsanity;
    $have =~ s/^ //; # since the %vd hack these two lines here are needed
    $have =~ s/ $//; # trailing whitespace happens all the time

    $have = CPAN::Version->readable($have);

    $have =~ s/\s*//g; # stringify to float around floating point issues
    $have; # no stringify needed, \s* above matches always
}

#-> sub CPAN::Module::reports
sub reports {
    my($self) = @_;
    $self->distribution->reports;
}

1;
