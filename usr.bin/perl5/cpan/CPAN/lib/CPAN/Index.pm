package CPAN::Index;
use strict;
use vars qw($LAST_TIME $DATE_OF_02 $DATE_OF_03 $HAVE_REANIMATED $VERSION);
$VERSION = "2.29";
@CPAN::Index::ISA = qw(CPAN::Debug);
$LAST_TIME ||= 0;
$DATE_OF_03 ||= 0;
# use constant PROTOCOL => "2.0"; # commented out to avoid warning on upgrade from 1.57
sub PROTOCOL { 2.0 }

#-> sub CPAN::Index::force_reload ;
sub force_reload {
    my($class) = @_;
    $CPAN::Index::LAST_TIME = 0;
    $class->reload(1);
}

my @indexbundle =
    (
     {
      reader => "rd_authindex",
      dir => "authors",
      remotefile => '01mailrc.txt.gz',
      shortlocalfile => '01mailrc.gz',
     },
     {
      reader => "rd_modpacks",
      dir => "modules",
      remotefile => '02packages.details.txt.gz',
      shortlocalfile => '02packag.gz',
     },
     {
      reader => "rd_modlist",
      dir => "modules",
      remotefile => '03modlist.data.gz',
      shortlocalfile => '03mlist.gz',
     },
    );

#-> sub CPAN::Index::reload ;
sub reload {
    my($self,$force) = @_;
    my $time = time;

    # XXX check if a newer one is available. (We currently read it
    # from time to time)
    for ($CPAN::Config->{index_expire}) {
        $_ = 0.001 unless $_ && $_ > 0.001;
    }
    unless (1 || $CPAN::Have_warned->{readmetadatacache}++) {
        # debug here when CPAN doesn't seem to read the Metadata
        require Carp;
        Carp::cluck("META-PROTOCOL[$CPAN::META->{PROTOCOL}]");
    }
    unless ($CPAN::META->{PROTOCOL}) {
        $self->read_metadata_cache;
        $CPAN::META->{PROTOCOL} ||= "1.0";
    }
    if ( $CPAN::META->{PROTOCOL} < PROTOCOL  ) {
        # warn "Setting last_time to 0";
        $LAST_TIME = 0; # No warning necessary
    }
    if ($LAST_TIME + $CPAN::Config->{index_expire}*86400 > $time
        and ! $force) {
        # called too often
        # CPAN->debug("LAST_TIME[$LAST_TIME]index_expire[$CPAN::Config->{index_expire}]time[$time]");
    } elsif (0) {
        # IFF we are developing, it helps to wipe out the memory
        # between reloads, otherwise it is not what a user expects.
        undef $CPAN::META; # Neue Gruendlichkeit since v1.52(r1.274)
        $CPAN::META = CPAN->new;
    } else {
        my($debug,$t2);
        local $LAST_TIME = $time;
        local $CPAN::META->{PROTOCOL} = PROTOCOL;

        my $needshort = $^O eq "dos";

    INX: for my $indexbundle (@indexbundle) {
            my $reader = $indexbundle->{reader};
            my $localfile = $needshort ? $indexbundle->{shortlocalfile} : $indexbundle->{remotefile};
            my $localpath = File::Spec->catfile($indexbundle->{dir}, $localfile);
            my $remote = join "/", $indexbundle->{dir}, $indexbundle->{remotefile};
            my $localized = $self->reload_x($remote, $localpath, $force);
            $self->$reader($localized); # may die but we let the shell catch it
            if ($CPAN::DEBUG){
                $t2 = time;
                $debug = "timing reading 01[".($t2 - $time)."]";
                $time = $t2;
            }
            return if $CPAN::Signal; # this is sometimes lengthy
        }
        $self->write_metadata_cache;
        if ($CPAN::DEBUG){
            $t2 = time;
            $debug .= "03[".($t2 - $time)."]";
            $time = $t2;
        }
        CPAN->debug($debug) if $CPAN::DEBUG;
    }
    if ($CPAN::Config->{build_dir_reuse}) {
        $self->reanimate_build_dir;
    }
    if (CPAN::_sqlite_running()) {
        $CPAN::SQLite->reload(time => $time, force => $force)
            if not $LAST_TIME;
    }
    $LAST_TIME = $time;
    $CPAN::META->{PROTOCOL} = PROTOCOL;
}

#-> sub CPAN::Index::reanimate_build_dir ;
sub reanimate_build_dir {
    my($self) = @_;
    unless ($CPAN::META->has_inst($CPAN::Config->{yaml_module}||"YAML")) {
        return;
    }
    return if $HAVE_REANIMATED++;
    my $d = $CPAN::Config->{build_dir};
    my $dh = DirHandle->new;
    opendir $dh, $d or return; # does not exist
    my $dirent;
    my $i = 0;
    my $painted = 0;
    my $restored = 0;
    my $start = CPAN::FTP::_mytime();
    my @candidates = map { $_->[0] }
        sort { $b->[1] <=> $a->[1] }
            map { [ $_, -M File::Spec->catfile($d,$_) ] }
                grep {/(.+)\.yml$/ && -d File::Spec->catfile($d,$1)} readdir $dh;
    if ( @candidates ) {
        $CPAN::Frontend->myprint
            (sprintf("Reading %d yaml file%s from %s/\n",
                    scalar @candidates,
                    @candidates==1 ? "" : "s",
                    $CPAN::Config->{build_dir}
                    ));
      DISTRO: for $i (0..$#candidates) {
            my $dirent = $candidates[$i];
            my $y = eval {CPAN->_yaml_loadfile(File::Spec->catfile($d,$dirent), {loadblessed => 1})};
            if ($@) {
                warn "Error while parsing file '$dirent'; error: '$@'";
                next DISTRO;
            }
            my $c = $y->[0];
            if ($c && $c->{perl} && $c->{distribution} && CPAN->_perl_fingerprint($c->{perl})) {
                my $key = $c->{distribution}{ID};
                for my $k (keys %{$c->{distribution}}) {
                    if ($c->{distribution}{$k}
                        && ref $c->{distribution}{$k}
                        && UNIVERSAL::isa($c->{distribution}{$k},"CPAN::Distrostatus")) {
                        $c->{distribution}{$k}{COMMANDID} = $i - @candidates;
                    }
                }

                #we tried to restore only if element already
                #exists; but then we do not work with metadata
                #turned off.
                my $do
                    = $CPAN::META->{readwrite}{'CPAN::Distribution'}{$key}
                        = $c->{distribution};
                for my $skipper (qw(
                                    badtestcnt
                                    configure_requires_later
                                    configure_requires_later_for
                                    force_update
                                    later
                                    later_for
                                    notest
                                    should_report
                                    sponsored_mods
                                    prefs
                                    negative_prefs_cache
                                  )) {
                    delete $do->{$skipper};
                }
                if ($do->can("tested_ok_but_not_installed")) {
                    if ($do->tested_ok_but_not_installed) {
                        $CPAN::META->is_tested($do->{build_dir},$do->{make_test}{TIME});
                    } else {
                        next DISTRO;
                    }
                }
                $restored++;
            }
            $i++;
            while (($painted/76) < ($i/@candidates)) {
                $CPAN::Frontend->myprint(".");
                $painted++;
            }
        }
    }
    else {
        $CPAN::Frontend->myprint("Build_dir empty, nothing to restore\n");
    }
    my $took = CPAN::FTP::_mytime() - $start;
    $CPAN::Frontend->myprint(sprintf(
                                     "DONE\nRestored the state of %s (in %.4f secs)\n",
                                     $restored || "none",
                                     $took,
                                    ));
}


#-> sub CPAN::Index::reload_x ;
sub reload_x {
    my($cl,$wanted,$localname,$force) = @_;
    $force |= 2; # means we're dealing with an index here
    CPAN::HandleConfig->load; # we should guarantee loading wherever
                              # we rely on Config XXX
    $localname ||= $wanted;
    my $abs_wanted = File::Spec->catfile($CPAN::Config->{'keep_source_where'},
                                         $localname);
    if (
        -f $abs_wanted &&
        -M $abs_wanted < $CPAN::Config->{'index_expire'} &&
        !($force & 1)
       ) {
        my $s = $CPAN::Config->{'index_expire'} == 1 ? "" : "s";
        $cl->debug(qq{$abs_wanted younger than $CPAN::Config->{'index_expire'} }.
                   qq{day$s. I\'ll use that.});
        return $abs_wanted;
    } else {
        $force |= 1; # means we're quite serious about it.
    }
    return CPAN::FTP->localize($wanted,$abs_wanted,$force);
}

#-> sub CPAN::Index::rd_authindex ;
sub rd_authindex {
    my($cl, $index_target) = @_;
    return unless defined $index_target;
    return if CPAN::_sqlite_running();
    my @lines;
    $CPAN::Frontend->myprint("Reading '$index_target'\n");
    local(*FH);
    tie *FH, 'CPAN::Tarzip', $index_target;
    local($/) = "\n";
    local($_);
    push @lines, split /\012/ while <FH>;
    my $i = 0;
    my $painted = 0;
    foreach (@lines) {
        my($userid,$fullname,$email) =
            m/alias\s+(\S+)\s+\"([^\"\<]*)\s+\<(.*)\>\"/;
        $fullname ||= $email;
        if ($userid && $fullname && $email) {
            my $userobj = $CPAN::META->instance('CPAN::Author',$userid);
            $userobj->set('FULLNAME' => $fullname, 'EMAIL' => $email);
        } else {
            CPAN->debug(sprintf "line[%s]", $_) if $CPAN::DEBUG;
        }
        $i++;
        while (($painted/76) < ($i/@lines)) {
            $CPAN::Frontend->myprint(".");
            $painted++;
        }
        return if $CPAN::Signal;
    }
    $CPAN::Frontend->myprint("DONE\n");
}

sub userid {
  my($self,$dist) = @_;
  $dist = $self->{'id'} unless defined $dist;
  my($ret) = $dist =~ m|(?:\w/\w\w/)?([^/]+)/|;
  $ret;
}

#-> sub CPAN::Index::rd_modpacks ;
sub rd_modpacks {
    my($self, $index_target) = @_;
    return unless defined $index_target;
    return if CPAN::_sqlite_running();
    $CPAN::Frontend->myprint("Reading '$index_target'\n");
    my $fh = CPAN::Tarzip->TIEHANDLE($index_target);
    local $_;
    CPAN->debug(sprintf "start[%d]", time) if $CPAN::DEBUG;
    my $slurp = "";
    my $chunk;
    while (my $bytes = $fh->READ(\$chunk,8192)) {
        $slurp.=$chunk;
    }
    my @lines = split /\012/, $slurp;
    CPAN->debug(sprintf "end[%d]", time) if $CPAN::DEBUG;
    undef $fh;
    # read header
    my($line_count,$last_updated);
    while (@lines) {
        my $shift = shift(@lines);
        last if $shift =~ /^\s*$/;
        $shift =~ /^Line-Count:\s+(\d+)/ and $line_count = $1;
        $shift =~ /^Last-Updated:\s+(.+)/ and $last_updated = $1;
    }
    CPAN->debug("line_count[$line_count]last_updated[$last_updated]") if $CPAN::DEBUG;
    my $errors = 0;
    if (not defined $line_count) {

        $CPAN::Frontend->mywarn(qq{Warning: Your $index_target does not contain a Line-Count header.
Please check the validity of the index file by comparing it to more
than one CPAN mirror. I'll continue but problems seem likely to
happen.\a
});
        $errors++;
        $CPAN::Frontend->mysleep(5);
    } elsif ($line_count != scalar @lines) {

        $CPAN::Frontend->mywarn(sprintf qq{Warning: Your %s
contains a Line-Count header of %d but I see %d lines there. Please
check the validity of the index file by comparing it to more than one
CPAN mirror. I'll continue but problems seem likely to happen.\a\n},
$index_target, $line_count, scalar(@lines));

    }
    if (not defined $last_updated) {

        $CPAN::Frontend->mywarn(qq{Warning: Your $index_target does not contain a Last-Updated header.
Please check the validity of the index file by comparing it to more
than one CPAN mirror. I'll continue but problems seem likely to
happen.\a
});
        $errors++;
        $CPAN::Frontend->mysleep(5);
    } else {

        $CPAN::Frontend
            ->myprint(sprintf qq{  Database was generated on %s\n},
                      $last_updated);
        $DATE_OF_02 = $last_updated;

        my $age = time;
        if ($CPAN::META->has_inst('HTTP::Date')) {
            require HTTP::Date;
            $age -= HTTP::Date::str2time($last_updated);
        } else {
            $CPAN::Frontend->mywarn("  HTTP::Date not available\n");
            require Time::Local;
            my(@d) = $last_updated =~ / (\d+) (\w+) (\d+) (\d+):(\d+):(\d+) /;
            $d[1] = index("Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec", $d[1])/4;
            $age -= $d[1]>=0 ? Time::Local::timegm(@d[5,4,3,0,1,2]) : 0;
        }
        $age /= 3600*24;
        if ($age > 30) {

            $CPAN::Frontend
                ->mywarn(sprintf
                         qq{Warning: This index file is %d days old.
  Please check the host you chose as your CPAN mirror for staleness.
  I'll continue but problems seem likely to happen.\a\n},
                         $age);

        } elsif ($age < -1) {

            $CPAN::Frontend
                ->mywarn(sprintf
                         qq{Warning: Your system date is %d days behind this index file!
  System time:          %s
  Timestamp index file: %s
  Please fix your system time, problems with the make command expected.\n},
                         -$age,
                         scalar gmtime,
                         $DATE_OF_02,
                        );

        }
    }


    # A necessity since we have metadata_cache: delete what isn't
    # there anymore
    my $secondtime = $CPAN::META->exists("CPAN::Module","CPAN");
    CPAN->debug("secondtime[$secondtime]") if $CPAN::DEBUG;
    my(%exists);
    my $i = 0;
    my $painted = 0;
 LINE: foreach (@lines) {
        # before 1.56 we split into 3 and discarded the rest. From
        # 1.57 we assign remaining text to $comment thus allowing to
        # influence isa_perl
        my($mod,$version,$dist,$comment) = split " ", $_, 4;
        unless ($mod && defined $version && $dist) {
            require Dumpvalue;
            my $dv = Dumpvalue->new(tick => '"');
            $CPAN::Frontend->mywarn(sprintf "Could not split line[%s]\n", $dv->stringify($_));
            if ($errors++ >= 5){
                $CPAN::Frontend->mydie("Giving up parsing your $index_target, too many errors");
            }
            next LINE;
        }
        my($bundle,$id,$userid);

        if ($mod eq 'CPAN' &&
            ! (
            CPAN::Queue->exists('Bundle::CPAN') ||
            CPAN::Queue->exists('CPAN')
            )
        ) {
            local($^W)= 0;
            if ($version > $CPAN::VERSION) {
                $CPAN::Frontend->mywarn(qq{
  New CPAN.pm version (v$version) available.
  [Currently running version is v$CPAN::VERSION]
  You might want to try
    install CPAN
    reload cpan
  to both upgrade CPAN.pm and run the new version without leaving
  the current session.

}); #});
                $CPAN::Frontend->mysleep(2);
                $CPAN::Frontend->myprint(qq{\n});
            }
            last if $CPAN::Signal;
        } elsif ($mod =~ /^Bundle::(.*)/) {
            $bundle = $1;
        }

        if ($bundle) {
            $id =  $CPAN::META->instance('CPAN::Bundle',$mod);
            # Let's make it a module too, because bundles have so much
            # in common with modules.

            # Changed in 1.57_63: seems like memory bloat now without
            # any value, so commented out

            # $CPAN::META->instance('CPAN::Module',$mod);

        } else {

            # instantiate a module object
            $id = $CPAN::META->instance('CPAN::Module',$mod);

        }

        # Although CPAN prohibits same name with different version the
        # indexer may have changed the version for the same distro
        # since the last time ("Force Reindexing" feature)
        if ($id->cpan_file ne $dist
            ||
            $id->cpan_version ne $version
           ) {
            $userid = $id->userid || $self->userid($dist);
            $id->set(
                     'CPAN_USERID' => $userid,
                     'CPAN_VERSION' => $version,
                     'CPAN_FILE' => $dist,
                    );
        }

        # instantiate a distribution object
        if ($CPAN::META->exists('CPAN::Distribution',$dist)) {
        # we do not need CONTAINSMODS unless we do something with
        # this dist, so we better produce it on demand.

        ## my $obj = $CPAN::META->instance(
        ##                                 'CPAN::Distribution' => $dist
        ##                                );
        ## $obj->{CONTAINSMODS}{$mod} = undef; # experimental
        } else {
            $CPAN::META->instance(
                                  'CPAN::Distribution' => $dist
                                 )->set(
                                        'CPAN_USERID' => $userid,
                                        'CPAN_COMMENT' => $comment,
                                       );
        }
        if ($secondtime) {
            for my $name ($mod,$dist) {
                # $self->debug("exists name[$name]") if $CPAN::DEBUG;
                $exists{$name} = undef;
            }
        }
        $i++;
        while (($painted/76) < ($i/@lines)) {
            $CPAN::Frontend->myprint(".");
            $painted++;
        }
        return if $CPAN::Signal;
    }
    $CPAN::Frontend->myprint("DONE\n");
    if ($secondtime) {
        for my $class (qw(CPAN::Module CPAN::Bundle CPAN::Distribution)) {
            for my $o ($CPAN::META->all_objects($class)) {
                next if exists $exists{$o->{ID}};
                $CPAN::META->delete($class,$o->{ID});
                # CPAN->debug("deleting ID[$o->{ID}] in class[$class]")
                #     if $CPAN::DEBUG;
            }
        }
    }
}

#-> sub CPAN::Index::rd_modlist ;
sub rd_modlist {
    my($cl,$index_target) = @_;
    return unless defined $index_target;
    return if CPAN::_sqlite_running();
    $CPAN::Frontend->myprint("Reading '$index_target'\n");
    my $fh = CPAN::Tarzip->TIEHANDLE($index_target);
    local $_;
    my $slurp = "";
    my $chunk;
    while (my $bytes = $fh->READ(\$chunk,8192)) {
        $slurp.=$chunk;
    }
    my @eval2 = split /\012/, $slurp;

    while (@eval2) {
        my $shift = shift(@eval2);
        if ($shift =~ /^Date:\s+(.*)/) {
            if ($DATE_OF_03 eq $1) {
                $CPAN::Frontend->myprint("Unchanged.\n");
                return;
            }
            ($DATE_OF_03) = $1;
        }
        last if $shift =~ /^\s*$/;
    }
    push @eval2, q{CPAN::Modulelist->data;};
    local($^W) = 0;
    my($compmt) = Safe->new("CPAN::Safe1");
    my($eval2) = join("\n", @eval2);
    CPAN->debug(sprintf "length of eval2[%d]", length $eval2) if $CPAN::DEBUG;
    my $ret = $compmt->reval($eval2);
    Carp::confess($@) if $@;
    return if $CPAN::Signal;
    my $i = 0;
    my $until = keys(%$ret);
    my $painted = 0;
    CPAN->debug(sprintf "until[%d]", $until) if $CPAN::DEBUG;
    for (sort keys %$ret) {
        my $obj = $CPAN::META->instance("CPAN::Module",$_);
        delete $ret->{$_}{modid}; # not needed here, maybe elsewhere
        $obj->set(%{$ret->{$_}});
        $i++;
        while (($painted/76) < ($i/$until)) {
            $CPAN::Frontend->myprint(".");
            $painted++;
        }
        return if $CPAN::Signal;
    }
    $CPAN::Frontend->myprint("DONE\n");
}

#-> sub CPAN::Index::write_metadata_cache ;
sub write_metadata_cache {
    my($self) = @_;
    return unless $CPAN::Config->{'cache_metadata'};
    return if CPAN::_sqlite_running();
    return unless $CPAN::META->has_usable("Storable");
    my $cache;
    foreach my $k (qw(CPAN::Bundle CPAN::Author CPAN::Module
                      CPAN::Distribution)) {
        $cache->{$k} = $CPAN::META->{readonly}{$k}; # unsafe meta access, ok
    }
    my $metadata_file = File::Spec->catfile($CPAN::Config->{cpan_home},"Metadata");
    $cache->{last_time} = $LAST_TIME;
    $cache->{DATE_OF_02} = $DATE_OF_02;
    $cache->{PROTOCOL} = PROTOCOL;
    $CPAN::Frontend->myprint("Writing $metadata_file\n");
    eval { Storable::nstore($cache, $metadata_file) };
    $CPAN::Frontend->mywarn($@) if $@; # ?? missing "\n" after $@ in mywarn ??
}

#-> sub CPAN::Index::read_metadata_cache ;
sub read_metadata_cache {
    my($self) = @_;
    return unless $CPAN::Config->{'cache_metadata'};
    return if CPAN::_sqlite_running();
    return unless $CPAN::META->has_usable("Storable");
    my $metadata_file = File::Spec->catfile($CPAN::Config->{cpan_home},"Metadata");
    return unless -r $metadata_file and -f $metadata_file;
    $CPAN::Frontend->myprint("Reading '$metadata_file'\n");
    my $cache;
    eval { $cache = Storable::retrieve($metadata_file) };
    $CPAN::Frontend->mywarn($@) if $@; # ?? missing "\n" after $@ in mywarn ??
    if (!$cache || !UNIVERSAL::isa($cache, 'HASH')) {
        $LAST_TIME = 0;
        return;
    }
    if (exists $cache->{PROTOCOL}) {
        if (PROTOCOL > $cache->{PROTOCOL}) {
            $CPAN::Frontend->mywarn(sprintf("Ignoring Metadata cache written ".
                                            "with protocol v%s, requiring v%s\n",
                                            $cache->{PROTOCOL},
                                            PROTOCOL)
                                   );
            return;
        }
    } else {
        $CPAN::Frontend->mywarn("Ignoring Metadata cache written ".
                                "with protocol v1.0\n");
        return;
    }
    my $clcnt = 0;
    my $idcnt = 0;
    while(my($class,$v) = each %$cache) {
        next unless $class =~ /^CPAN::/;
        $CPAN::META->{readonly}{$class} = $v; # unsafe meta access, ok
        while (my($id,$ro) = each %$v) {
            $CPAN::META->{readwrite}{$class}{$id} ||=
                $class->new(ID=>$id, RO=>$ro);
            $idcnt++;
        }
        $clcnt++;
    }
    unless ($clcnt) { # sanity check
        $CPAN::Frontend->myprint("Warning: Found no data in $metadata_file\n");
        return;
    }
    if ($idcnt < 1000) {
        $CPAN::Frontend->myprint("Warning: Found only $idcnt objects ".
                                 "in $metadata_file\n");
        return;
    }
    $CPAN::META->{PROTOCOL} ||=
        $cache->{PROTOCOL}; # reading does not up or downgrade, but it
                            # does initialize to some protocol
    $LAST_TIME = $cache->{last_time};
    $DATE_OF_02 = $cache->{DATE_OF_02};
    $CPAN::Frontend->myprint("  Database was generated on $DATE_OF_02\n")
        if defined $DATE_OF_02; # An old cache may not contain DATE_OF_02
    return;
}

1;
