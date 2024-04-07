# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Distribution;
use strict;
use Cwd qw(chdir);
use CPAN::Distroprefs;
use CPAN::InfoObj;
use File::Path ();
use POSIX ":sys_wait_h"; 
@CPAN::Distribution::ISA = qw(CPAN::InfoObj);
use vars qw($VERSION);
$VERSION = "2.34";

my $run_allow_installing_within_test = 1; # boolean; either in test or in install, there is no third option

# no prepare, because prepare is not a command on the shell command line
# TODO: clear instance cache on reload
my %instance;
for my $method (qw(get make test install)) {
    no strict 'refs';
    for my $prefix (qw(pre post)) {
        my $hookname = sprintf "%s_%s", $prefix, $method;
        *$hookname = sub {
            my($self) = @_;
            for my $plugin (@{$CPAN::Config->{plugin_list}}) {
                my($plugin_proper,$args) = split /=/, $plugin, 2;
                $args = "" unless defined $args;
                if ($CPAN::META->has_inst($plugin_proper)){
                    my @args = split /,/, $args;
                    $instance{$plugin} ||= $plugin_proper->new(@args);
                    if ($instance{$plugin}->can($hookname)) {
                        $instance{$plugin}->$hookname($self);
                    }
                } else {
                    $CPAN::Frontend->mydie("Plugin '$plugin_proper' not found for hook '$hookname'");
                }
            }
        };
    }
}

# Accessors
sub cpan_comment {
    my $self = shift;
    my $ro = $self->ro or return;
    $ro->{CPAN_COMMENT}
}

#-> CPAN::Distribution::undelay
sub undelay {
    my $self = shift;
    for my $delayer (
                     "configure_requires_later",
                     "configure_requires_later_for",
                     "later",
                     "later_for",
                    ) {
        delete $self->{$delayer};
    }
}

#-> CPAN::Distribution::is_dot_dist
sub is_dot_dist {
    my($self) = @_;
    return substr($self->id,-1,1) eq ".";
}

# add the A/AN/ stuff
#-> CPAN::Distribution::normalize
sub normalize {
    my($self,$s) = @_;
    $s = $self->id unless defined $s;
    if (substr($s,-1,1) eq ".") {
        # using a global because we are sometimes called as static method
        if (!$CPAN::META->{LOCK}
            && !$CPAN::Have_warned->{"$s is unlocked"}++
           ) {
            $CPAN::Frontend->mywarn("You are visiting the local directory
  '$s'
  without lock, take care that concurrent processes do not do likewise.\n");
            $CPAN::Frontend->mysleep(1);
        }
        if ($s eq ".") {
            $s = "$CPAN::iCwd/.";
        } elsif (File::Spec->file_name_is_absolute($s)) {
        } elsif (File::Spec->can("rel2abs")) {
            $s = File::Spec->rel2abs($s);
        } else {
            $CPAN::Frontend->mydie("Your File::Spec is too old, please upgrade File::Spec");
        }
        CPAN->debug("s[$s]") if $CPAN::DEBUG;
        unless ($CPAN::META->exists("CPAN::Distribution", $s)) {
            for ($CPAN::META->instance("CPAN::Distribution", $s)) {
                $_->{build_dir} = $s;
                $_->{archived} = "local_directory";
                $_->{unwrapped} = CPAN::Distrostatus->new("YES -- local_directory");
            }
        }
    } elsif (
        $s =~ tr|/|| == 1
        or
        $s !~ m|[A-Z]/[A-Z-0-9]{2}/[A-Z-0-9]{2,}/|
       ) {
        return $s if $s =~ m:^N/A|^Contact Author: ;
        $s =~ s|^(.)(.)([^/]*/)(.+)$|$1/$1$2/$1$2$3$4|;
        CPAN->debug("s[$s]") if $CPAN::DEBUG;
    }
    $s;
}

#-> sub CPAN::Distribution::author ;
sub author {
    my($self) = @_;
    my($authorid);
    if (substr($self->id,-1,1) eq ".") {
        $authorid = "LOCAL";
    } else {
        ($authorid) = $self->pretty_id =~ /^([\w\-]+)/;
    }
    CPAN::Shell->expand("Author",$authorid);
}

# tries to get the yaml from CPAN instead of the distro itself:
# EXPERIMENTAL, UNDOCUMENTED AND UNTESTED, for Tels
sub fast_yaml {
    my($self) = @_;
    my $meta = $self->pretty_id;
    $meta =~ s/\.(tar.gz|tgz|zip|tar.bz2)/.meta/;
    my(@ls) = CPAN::Shell->globls($meta);
    my $norm = $self->normalize($meta);

    my($local_file);
    my($local_wanted) =
        File::Spec->catfile(
                            $CPAN::Config->{keep_source_where},
                            "authors",
                            "id",
                            split(/\//,$norm)
                           );
    $self->debug("Doing localize") if $CPAN::DEBUG;
    unless ($local_file =
            CPAN::FTP->localize("authors/id/$norm",
                                $local_wanted)) {
        $CPAN::Frontend->mydie("Giving up on downloading yaml file '$local_wanted'\n");
    }
    my $yaml = CPAN->_yaml_loadfile($local_file)->[0];
}

#-> sub CPAN::Distribution::cpan_userid
sub cpan_userid {
    my $self = shift;
    if ($self->{ID} =~ m{[A-Z]/[A-Z\-]{2}/([A-Z\-]+)/}) {
        return $1;
    }
    return $self->SUPER::cpan_userid;
}

#-> sub CPAN::Distribution::pretty_id
sub pretty_id {
    my $self = shift;
    my $id = $self->id;
    return $id unless $id =~ m|^./../|;
    substr($id,5);
}

#-> sub CPAN::Distribution::base_id
sub base_id {
    my $self = shift;
    my $id = $self->pretty_id();
    my $base_id = File::Basename::basename($id);
    $base_id =~ s{\.(?:tar\.(bz2|gz|Z)|t(?:gz|bz)|zip)$}{}i;
    return $base_id;
}

#-> sub CPAN::Distribution::tested_ok_but_not_installed
sub tested_ok_but_not_installed {
    my $self = shift;
    return (
           $self->{make_test}
        && $self->{build_dir}
        && (UNIVERSAL::can($self->{make_test},"failed") ?
             ! $self->{make_test}->failed :
             $self->{make_test} =~ /^YES/
            )
        && (
            !$self->{install}
            ||
            $self->{install}->failed
           )
    );
}


# mark as dirty/clean for the sake of recursion detection. $color=1
# means "in use", $color=0 means "not in use anymore". $color=2 means
# we have determined prereqs now and thus insist on passing this
# through (at least) once again.

#-> sub CPAN::Distribution::color_cmd_tmps ;
sub color_cmd_tmps {
    my($self) = shift;
    my($depth) = shift || 0;
    my($color) = shift || 0;
    my($ancestors) = shift || [];
    # a distribution needs to recurse into its prereq_pms
    $self->debug("color_cmd_tmps[$depth,$color,@$ancestors]") if $CPAN::DEBUG;

    return if exists $self->{incommandcolor}
        && $color==1
        && $self->{incommandcolor}==$color;
    $CPAN::MAX_RECURSION||=0; # silence 'once' warnings
    if ($depth>=$CPAN::MAX_RECURSION) {
        my $e = CPAN::Exception::RecursiveDependency->new($ancestors);
        if ($e->is_resolvable) {
            return $self->{incommandcolor}=2;
        } else {
            die $e;
        }
    }
    # warn "color_cmd_tmps $depth $color " . $self->id; # sleep 1;
    my $prereq_pm = $self->prereq_pm;
    if (defined $prereq_pm) {
        # XXX also optional_req & optional_breq? -- xdg, 2012-04-01
        # A: no, optional deps may recurse -- ak, 2014-05-07
      PREREQ: for my $pre (sort(
                keys %{$prereq_pm->{requires}||{}},
                keys %{$prereq_pm->{build_requires}||{}},
            )) {
            next PREREQ if $pre eq "perl";
            my $premo;
            unless ($premo = CPAN::Shell->expand("Module",$pre)) {
                $CPAN::Frontend->mywarn("prerequisite module[$pre] not known\n");
                $CPAN::Frontend->mysleep(0.2);
                next PREREQ;
            }
            $premo->color_cmd_tmps($depth+1,$color,[@$ancestors, $self->id]);
        }
    }
    if ($color==0) {
        delete $self->{sponsored_mods};

        # as we are at the end of a command, we'll give up this
        # reminder of a broken test. Other commands may test this guy
        # again. Maybe 'badtestcnt' should be renamed to
        # 'make_test_failed_within_command'?
        delete $self->{badtestcnt};
    }
    $self->{incommandcolor} = $color;
}

#-> sub CPAN::Distribution::as_string ;
sub as_string {
    my $self = shift;
    $self->containsmods;
    $self->upload_date;
    $self->SUPER::as_string(@_);
}

#-> sub CPAN::Distribution::containsmods ;
sub containsmods {
    my $self = shift;
    return sort keys %{$self->{CONTAINSMODS}} if exists $self->{CONTAINSMODS};
    my $dist_id = $self->{ID};
    for my $mod ($CPAN::META->all_objects("CPAN::Module")) {
        my $mod_file = $mod->cpan_file or next;
        my $mod_id = $mod->{ID} or next;
        # warn "mod_file[$mod_file] dist_id[$dist_id] mod_id[$mod_id]";
        # sleep 1;
        if ($CPAN::Signal) {
            delete $self->{CONTAINSMODS};
            return;
        }
        $self->{CONTAINSMODS}{$mod_id} = undef if $mod_file eq $dist_id;
    }
    sort keys %{$self->{CONTAINSMODS}||={}};
}

#-> sub CPAN::Distribution::upload_date ;
sub upload_date {
    my $self = shift;
    return $self->{UPLOAD_DATE} if exists $self->{UPLOAD_DATE};
    my(@local_wanted) = split(/\//,$self->id);
    my $filename = pop @local_wanted;
    push @local_wanted, "CHECKSUMS";
    my $author = CPAN::Shell->expand("Author",$self->cpan_userid);
    return unless $author;
    my @dl = $author->dir_listing(\@local_wanted,0,$CPAN::Config->{show_upload_date});
    return unless @dl;
    my($dirent) = grep { $_->[2] eq $filename } @dl;
    # warn sprintf "dirent[%s]id[%s]", $dirent, $self->id;
    return unless $dirent->[1];
    return $self->{UPLOAD_DATE} = $dirent->[1];
}

#-> sub CPAN::Distribution::uptodate ;
sub uptodate {
    my($self) = @_;
    my $c;
    foreach $c ($self->containsmods) {
        my $obj = CPAN::Shell->expandany($c);
        unless ($obj->uptodate) {
            my $id = $self->pretty_id;
            $self->debug("$id not uptodate due to $c") if $CPAN::DEBUG;
            return 0;
        }
    }
    return 1;
}

#-> sub CPAN::Distribution::called_for ;
sub called_for {
    my($self,$id) = @_;
    $self->{CALLED_FOR} = $id if defined $id;
    return $self->{CALLED_FOR};
}

#-> sub CPAN::Distribution::shortcut_get ;
# return values: undef means don't shortcut; 0 means shortcut as fail;
# and 1 means shortcut as success
sub shortcut_get {
    my ($self) = @_;

    if (exists $self->{cleanup_after_install_done}) {
        if ($self->{force_update}) {
            delete $self->{cleanup_after_install_done};
        } else {
            my $id = $self->{CALLED_FOR} || $self->pretty_id;
            return $self->success(
                "Has already been *installed and cleaned up in the staging area* within this session, will not work on it again; if you really want to start over, try something like `force get $id`"
            );
        }
    }

    if (my $why = $self->check_disabled) {
        $self->{unwrapped} = CPAN::Distrostatus->new("NO $why");
        # XXX why is this goodbye() instead of just print/warn?
        # Alternatively, should other print/warns here be goodbye()?
        # -- xdg, 2012-04-05
        return $self->goodbye("[disabled] -- NA $why");
    }

    $self->debug("checking already unwrapped[$self->{ID}]") if $CPAN::DEBUG;
    if (exists $self->{build_dir} && -d $self->{build_dir}) {
        # this deserves print, not warn:
        return $self->success("Has already been unwrapped into directory ".
            "$self->{build_dir}"
        );
    }

    # XXX I'm not sure this should be here because it's not really
    # a test for whether get should continue or return; this is
    # a side effect -- xdg, 2012-04-05
    $self->debug("checking missing build_dir[$self->{ID}]") if $CPAN::DEBUG;
    if (exists $self->{build_dir} && ! -d $self->{build_dir}){
        # we have lost it.
        $self->fforce(""); # no method to reset all phases but not set force (dodge)
        return undef; # no shortcut
    }

    # although we talk about 'force' we shall not test on
    # force directly. New model of force tries to refrain from
    # direct checking of force.
    $self->debug("checking unwrapping error[$self->{ID}]") if $CPAN::DEBUG;
    if ( exists $self->{unwrapped} and (
            UNIVERSAL::can($self->{unwrapped},"failed") ?
            $self->{unwrapped}->failed :
            $self->{unwrapped} =~ /^NO/ )
    ) {
        return $self->goodbye("Unwrapping had some problem, won't try again without force");
    }

    return undef; # no shortcut
}

#-> sub CPAN::Distribution::get ;
sub get {
    my($self) = @_;

    $self->pre_get();

    $self->debug("checking goto id[$self->{ID}]") if $CPAN::DEBUG;
    if (my $goto = $self->prefs->{goto}) {
        $self->post_get();
        return $self->goto($goto);
    }

    if ( defined( my $sc = $self->shortcut_get) ) {
        $self->post_get();
        return $sc;
    }

    local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
                           ? $ENV{PERL5LIB}
                           : ($ENV{PERLLIB} || "");
    local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
    # local $ENV{PERL_USE_UNSAFE_INC} = exists $ENV{PERL_USE_UNSAFE_INC} ? $ENV{PERL_USE_UNSAFE_INC} : 1; # get
    $CPAN::META->set_perl5lib;
    local $ENV{MAKEFLAGS}; # protect us from outer make calls

    my $sub_wd = CPAN::anycwd(); # for cleaning up as good as possible

    my($local_file);
    # XXX I don't think this check needs to be here, as it
    # is already checked in shortcut_get() -- xdg, 2012-04-05
    unless ($self->{build_dir} && -d $self->{build_dir}) {
        $self->get_file_onto_local_disk;
        if ($CPAN::Signal){
            $self->post_get();
            return;
        }
        $self->check_integrity;
        if ($CPAN::Signal){
            $self->post_get();
            return;
        }
        (my $packagedir,$local_file) = $self->run_preps_on_packagedir;
        # XXX why is this check here? -- xdg, 2012-04-08
        if (exists $self->{writemakefile} && ref $self->{writemakefile}
           && $self->{writemakefile}->can("failed") &&
           $self->{writemakefile}->failed) {
           #
            $self->post_get();
            return;
        }
        $packagedir ||= $self->{build_dir};
        $self->{build_dir} = $packagedir;
    }

    # XXX should this move up to after run_preps_on_packagedir?
    # Otherwise, failing writemakefile can return without
    # a $CPAN::Signal check -- xdg, 2012-04-05
    if ($CPAN::Signal) {
        $self->safe_chdir($sub_wd);
        $self->post_get();
        return;
    }
    unless ($self->patch){
        $self->post_get();
        return;
    }
    $self->store_persistent_state;

    $self->post_get();

    return 1; # success
}

#-> CPAN::Distribution::get_file_onto_local_disk
sub get_file_onto_local_disk {
    my($self) = @_;

    return if $self->is_dot_dist;
    my($local_file);
    my($local_wanted) =
        File::Spec->catfile(
                            $CPAN::Config->{keep_source_where},
                            "authors",
                            "id",
                            split(/\//,$self->id)
                           );

    $self->debug("Doing localize") if $CPAN::DEBUG;
    unless ($local_file =
            CPAN::FTP->localize("authors/id/$self->{ID}",
                                $local_wanted)) {
        my $note = "";
        if ($CPAN::Index::DATE_OF_02) {
            $note = "Note: Current database in memory was generated ".
                "on $CPAN::Index::DATE_OF_02\n";
        }
        $CPAN::Frontend->mydie("Giving up on '$local_wanted'\n$note");
    }

    $self->debug("local_wanted[$local_wanted]local_file[$local_file]") if $CPAN::DEBUG;
    $self->{localfile} = $local_file;
}


#-> CPAN::Distribution::check_integrity
sub check_integrity {
    my($self) = @_;

    return if $self->is_dot_dist;
    if ($CPAN::META->has_inst("Digest::SHA")) {
        $self->debug("Digest::SHA is installed, verifying");
        $self->verifyCHECKSUM;
    } else {
        $self->debug("Digest::SHA is NOT installed");
    }
}

#-> CPAN::Distribution::run_preps_on_packagedir
sub run_preps_on_packagedir {
    my($self) = @_;
    return if $self->is_dot_dist;

    $CPAN::META->{cachemgr} ||= CPAN::CacheMgr->new(); # unsafe meta access, ok
    my $builddir = $CPAN::META->{cachemgr}->dir; # unsafe meta access, ok
    $self->safe_chdir($builddir);
    $self->debug("Removing tmp-$$") if $CPAN::DEBUG;
    File::Path::rmtree("tmp-$$");
    unless (mkdir "tmp-$$", 0755) {
        $CPAN::Frontend->unrecoverable_error(<<EOF);
Couldn't mkdir '$builddir/tmp-$$': $!

Cannot continue: Please find the reason why I cannot make the
directory
$builddir/tmp-$$
and fix the problem, then retry.

EOF
    }
    if ($CPAN::Signal) {
        return;
    }
    $self->safe_chdir("tmp-$$");

    #
    # Unpack the goods
    #
    my $local_file = $self->{localfile};
    my $ct = eval{CPAN::Tarzip->new($local_file)};
    unless ($ct) {
        $self->{unwrapped} = CPAN::Distrostatus->new("NO");
        delete $self->{build_dir};
        return;
    }
    if ($local_file =~ /(\.tar\.(bz2|gz|Z)|\.tgz)(?!\n)\Z/i) {
        $self->{was_uncompressed}++ unless eval{$ct->gtest()};
        $self->untar_me($ct);
    } elsif ( $local_file =~ /\.zip(?!\n)\Z/i ) {
        $self->unzip_me($ct);
    } else {
        $self->{was_uncompressed}++ unless $ct->gtest();
        $local_file = $self->handle_singlefile($local_file);
    }

    # we are still in the tmp directory!
    # Let's check if the package has its own directory.
    my $dh = DirHandle->new(File::Spec->curdir)
        or Carp::croak("Couldn't opendir .: $!");
    my @readdir = grep $_ !~ /^\.\.?(?!\n)\Z/s, $dh->read; ### MAC??
    if (grep { $_ eq "pax_global_header" } @readdir) {
        $CPAN::Frontend->mywarn("Your (un)tar seems to have extracted a file named 'pax_global_header'
from the tarball '$local_file'.
This is almost certainly an error. Please upgrade your tar.
I'll ignore this file for now.
See also http://rt.cpan.org/Ticket/Display.html?id=38932\n");
        $CPAN::Frontend->mysleep(5);
        @readdir = grep { $_ ne "pax_global_header" } @readdir;
    }
    $dh->close;
    my $tdir_base;
    my $from_dir;
    my @dirents;
    if (@readdir == 1 && -d $readdir[0]) {
        $tdir_base = $readdir[0];
        $from_dir = File::Spec->catdir(File::Spec->curdir,$readdir[0]);
        my($mode) = (stat $from_dir)[2];
        chmod $mode | 00755, $from_dir; # JONATHAN/Math-Calculus-TaylorSeries-0.1.tar.gz has 0644
        my $dh2;
        unless ($dh2 = DirHandle->new($from_dir)) {
            my $why = sprintf
                (
                 "Couldn't opendir '%s', mode '%o': %s",
                 $from_dir,
                 $mode,
                 $!,
                );
            $CPAN::Frontend->mywarn("$why\n");
            $self->{writemakefile} = CPAN::Distrostatus->new("NO -- $why");
            return;
        }
        @dirents = grep $_ !~ /^\.\.?(?!\n)\Z/s, $dh2->read; ### MAC??
    } else {
        my $userid = $self->cpan_userid;
        CPAN->debug("userid[$userid]");
        if (!$userid or $userid eq "N/A") {
            $userid = "anon";
        }
        $tdir_base = $userid;
        $from_dir = File::Spec->curdir;
        @dirents = @readdir;
    }
    my $packagedir;
    my $eexist = ($CPAN::META->has_usable("Errno") && defined &Errno::EEXIST)
        ? &Errno::EEXIST : undef;
    for(my $suffix = 0; ; $suffix++) {
        $packagedir = File::Spec->catdir($builddir, "$tdir_base-$suffix");
        my $parent = $builddir;
        mkdir($packagedir, 0777) and last;
        if((defined($eexist) && $! != $eexist) || $suffix == 999) {
            $CPAN::Frontend->mydie("Cannot create directory $packagedir: $!\n");
        }
    }
    my $f;
    for $f (@dirents) { # is already without "." and ".."
        my $from = File::Spec->catfile($from_dir,$f);
        my($mode) = (stat $from)[2];
        chmod $mode | 00755, $from if -d $from; # OTTO/Pod-Trial-LinkImg-0.005.tgz
        my $to = File::Spec->catfile($packagedir,$f);
        unless (File::Copy::move($from,$to)) {
            my $err = $!;
            $from = File::Spec->rel2abs($from);
            $CPAN::Frontend->mydie(
                "Couldn't move $from to $to: $err; #82295? ".
                "CPAN::VERSION=$CPAN::VERSION; ".
                "File::Copy::VERSION=$File::Copy::VERSION; ".
                "$from " . (-e $from ? "exists; " : "does not exist; ").
                "$to " . (-e $to ? "exists; " : "does not exist; ").
                "cwd=" . CPAN::anycwd() . ";"
            );
        }
    }
    $self->{build_dir} = $packagedir;
    $self->safe_chdir($builddir);
    File::Path::rmtree("tmp-$$");

    $self->safe_chdir($packagedir);
    $self->_signature_business();
    $self->safe_chdir($builddir);

    return($packagedir,$local_file);
}

#-> sub CPAN::Distribution::pick_meta_file ;
sub pick_meta_file {
    my($self, $filter) = @_;
    $filter = '.' unless defined $filter;

    my $build_dir;
    unless ($build_dir = $self->{build_dir}) {
        # maybe permission on build_dir was missing
        $CPAN::Frontend->mywarn("Warning: cannot determine META.yml without a build_dir.\n");
        return;
    }

    my $has_cm = $CPAN::META->has_usable("CPAN::Meta");
    my $has_pcm = $CPAN::META->has_usable("Parse::CPAN::Meta");

    my @choices;
    push @choices, 'MYMETA.json' if $has_cm;
    push @choices, 'MYMETA.yml' if $has_cm || $has_pcm;
    push @choices, 'META.json' if $has_cm;
    push @choices, 'META.yml' if $has_cm || $has_pcm;

    for my $file ( grep { /$filter/ } @choices ) {
        my $path = File::Spec->catfile( $build_dir, $file );
        return $path if -f $path
    }

    return;
}

#-> sub CPAN::Distribution::parse_meta_yml ;
sub parse_meta_yml {
    my($self, $yaml) = @_;
    $self->debug(sprintf("parse_meta_yml[%s]",$yaml||'undef')) if $CPAN::DEBUG;
    my $build_dir = $self->{build_dir} or die "PANIC: cannot parse yaml without a build_dir";
    $yaml ||= File::Spec->catfile($build_dir,"META.yml");
    $self->debug("meta[$yaml]") if $CPAN::DEBUG;
    return unless -f $yaml;
    my $early_yaml;
    eval {
        $CPAN::META->has_inst("Parse::CPAN::Meta") or die;
        die "Parse::CPAN::Meta yaml too old" unless $Parse::CPAN::Meta::VERSION >= "1.40";
        # P::C::M returns last document in scalar context
        $early_yaml = Parse::CPAN::Meta::LoadFile($yaml);
    };
    unless ($early_yaml) {
        eval { $early_yaml = CPAN->_yaml_loadfile($yaml)->[0]; };
    }
    $self->debug(sprintf("yaml[%s]", $early_yaml || 'UNDEF')) if $CPAN::DEBUG;
    $self->debug($early_yaml) if $CPAN::DEBUG && $early_yaml;
    if (!ref $early_yaml or ref $early_yaml ne "HASH"){
        # fix rt.cpan.org #95271
        $CPAN::Frontend->mywarn("The content of '$yaml' is not a HASH reference. Cannot use it.\n");
        return {};
    }
    return $early_yaml || undef;
}

#-> sub CPAN::Distribution::satisfy_requires ;
# return values: 1 means requirements are satisfied;
# and 0 means not satisfied (and maybe queued)
sub satisfy_requires {
    my ($self) = @_;
    $self->debug("Entering satisfy_requires") if $CPAN::DEBUG;
    if (my @prereq = $self->unsat_prereq("later")) {
        if ($CPAN::DEBUG){
            require Data::Dumper;
            my $prereq = Data::Dumper->new(\@prereq)->Terse(1)->Indent(0)->Dump;
            $self->debug("unsatisfied[$prereq]");
        }
        if ($prereq[0][0] eq "perl") {
            my $need = "requires perl '$prereq[0][1]'";
            my $id = $self->pretty_id;
            $CPAN::Frontend->mywarn("$id $need; you have only $]; giving up\n");
            $self->{make} = CPAN::Distrostatus->new("NO $need");
            $self->store_persistent_state;
            die "[prereq] -- NOT OK\n";
        } else {
            my $follow = eval { $self->follow_prereqs("later",@prereq); };
            if (0) {
            } elsif ($follow) {
                return; # we need deps
            } elsif ($@ && ref $@ && $@->isa("CPAN::Exception::RecursiveDependency")) {
                $CPAN::Frontend->mywarn($@);
                die "[depend] -- NOT OK\n";
            }
        }
    }
    return 1;
}

#-> sub CPAN::Distribution::satisfy_configure_requires ;
# return values: 1 means configure_require is satisfied;
# and 0 means not satisfied (and maybe queued)
sub satisfy_configure_requires {
    my($self) = @_;
    $self->debug("Entering satisfy_configure_requires") if $CPAN::DEBUG;
    my $enable_configure_requires = 1;
    if (!$enable_configure_requires) {
        return 1;
        # if we return 1 here, everything is as before we introduced
        # configure_requires that means, things with
        # configure_requires simply fail, all others succeed
    }
    my @prereq = $self->unsat_prereq("configure_requires_later");
    $self->debug(sprintf "configure_requires[%s]", join(",",map {join "/",@$_} @prereq)) if $CPAN::DEBUG;
    return 1 unless @prereq;
    $self->debug(\@prereq) if $CPAN::DEBUG;
    if ($self->{configure_requires_later}) {
        for my $k (sort keys %{$self->{configure_requires_later_for}||{}}) {
            if ($self->{configure_requires_later_for}{$k}>1) {
                my $type = "";
                for my $p (@prereq) {
                    if ($p->[0] eq $k) {
                        $type = $p->[1];
                    }
                }
                $type = " $type" if $type;
                $CPAN::Frontend->mywarn("Warning: unmanageable(?) prerequisite $k$type");
                sleep 1;
            }
        }
    }
    if ($prereq[0][0] eq "perl") {
        my $need = "requires perl '$prereq[0][1]'";
        my $id = $self->pretty_id;
        $CPAN::Frontend->mywarn("$id $need; you have only $]; giving up\n");
        $self->{make} = CPAN::Distrostatus->new("NO $need");
        $self->store_persistent_state;
        return $self->goodbye("[prereq] -- NOT OK");
    } else {
        my $follow = eval {
            $self->follow_prereqs("configure_requires_later", @prereq);
        };
        if (0) {
        } elsif ($follow) {
            return; # we need deps
        } elsif ($@ && ref $@ && $@->isa("CPAN::Exception::RecursiveDependency")) {
            $CPAN::Frontend->mywarn($@);
            return $self->goodbye("[depend] -- NOT OK");
        }
        else {
          return $self->goodbye("[configure_requires] -- NOT OK");
        }
    }
    die "never reached";
}

#-> sub CPAN::Distribution::choose_MM_or_MB ;
sub choose_MM_or_MB {
    my($self) = @_;
    $self->satisfy_configure_requires() or return;
    my $local_file = $self->{localfile};
    my($mpl) = File::Spec->catfile($self->{build_dir},"Makefile.PL");
    my($mpl_exists) = -f $mpl;
    unless ($mpl_exists) {
        # NFS has been reported to have racing problems after the
        # renaming of a directory in some environments.
        # This trick helps.
        $CPAN::Frontend->mysleep(1);
        my $mpldh = DirHandle->new($self->{build_dir})
            or Carp::croak("Couldn't opendir $self->{build_dir}: $!");
        $mpl_exists = grep /^Makefile\.PL$/, $mpldh->read;
        $mpldh->close;
    }
    my $prefer_installer = "eumm"; # eumm|mb
    if (-f File::Spec->catfile($self->{build_dir},"Build.PL")) {
        if ($mpl_exists) { # they *can* choose
            if ($CPAN::META->has_inst("Module::Build")) {
                $prefer_installer = CPAN::HandleConfig->prefs_lookup(
                  $self, q{prefer_installer}
                );
                # M::B <= 0.35 left a DATA handle open that
                # causes problems upgrading M::B on Windows
                close *Module::Build::Version::DATA
                  if fileno *Module::Build::Version::DATA;
            }
        } else {
            $prefer_installer = "mb";
        }
    }
    if (lc($prefer_installer) eq "rand") {
        $prefer_installer = rand()<.5 ? "eumm" : "mb";
    }
    if (lc($prefer_installer) eq "mb") {
        $self->{modulebuild} = 1;
    } elsif ($self->{archived} eq "patch") {
        # not an edge case, nothing to install for sure
        my $why = "A patch file cannot be installed";
        $CPAN::Frontend->mywarn("Refusing to handle this file: $why\n");
        $self->{writemakefile} = CPAN::Distrostatus->new("NO $why");
    } elsif (! $mpl_exists) {
        $self->_edge_cases($mpl,$local_file);
    }
    if ($self->{build_dir}
        &&
        $CPAN::Config->{build_dir_reuse}
       ) {
        $self->store_persistent_state;
    }
    return $self;
}

# see also reanimate_build_dir
#-> CPAN::Distribution::store_persistent_state
sub store_persistent_state {
    my($self) = @_;
    my $dir = $self->{build_dir};
    unless (defined $dir && length $dir) {
        my $id = $self->id;
        $CPAN::Frontend->mywarnonce("build_dir of $id is not known, ".
                                    "will not store persistent state\n");
        return;
    }
    # self-build-dir
    my $sbd = Cwd::realpath(
        File::Spec->catdir($dir,                       File::Spec->updir ())
                           );
    # config-build-dir
    my $cbd = Cwd::realpath(
        # the catdir is a workaround for bug https://rt.cpan.org/Ticket/Display.html?id=101283
        File::Spec->catdir($CPAN::Config->{build_dir}, File::Spec->curdir())
    );
    unless ($sbd eq $cbd) {
        $CPAN::Frontend->mywarnonce("Directory '$dir' not below $CPAN::Config->{build_dir}, ".
                                    "will not store persistent state\n");
        return;
    }
    my $file = sprintf "%s.yml", $dir;
    my $yaml_module = CPAN::_yaml_module();
    if ($CPAN::META->has_inst($yaml_module)) {
        CPAN->_yaml_dumpfile(
                             $file,
                             {
                              time => time,
                              perl => CPAN::_perl_fingerprint(),
                              distribution => $self,
                             }
                            );
    } else {
        $CPAN::Frontend->myprintonce("'$yaml_module' not installed, ".
                                    "will not store persistent state\n");
    }
}

#-> CPAN::Distribution::try_download
sub try_download {
    my($self,$patch) = @_;
    my $norm = $self->normalize($patch);
    my($local_wanted) =
        File::Spec->catfile(
                            $CPAN::Config->{keep_source_where},
                            "authors",
                            "id",
                            split(/\//,$norm),
                           );
    $self->debug("Doing localize") if $CPAN::DEBUG;
    return CPAN::FTP->localize("authors/id/$norm",
                               $local_wanted);
}

{
    my $stdpatchargs = "";
    #-> CPAN::Distribution::patch
    sub patch {
        my($self) = @_;
        $self->debug("checking patches id[$self->{ID}]") if $CPAN::DEBUG;
        my $patches = $self->prefs->{patches};
        $patches ||= "";
        $self->debug("patches[$patches]") if $CPAN::DEBUG;
        if ($patches) {
            return unless @$patches;
            $self->safe_chdir($self->{build_dir});
            CPAN->debug("patches[$patches]") if $CPAN::DEBUG;
            my $patchbin = $CPAN::Config->{patch};
            unless ($patchbin && length $patchbin) {
                $CPAN::Frontend->mydie("No external patch command configured\n\n".
                                       "Please run 'o conf init /patch/'\n\n");
            }
            unless (MM->maybe_command($patchbin)) {
                $CPAN::Frontend->mydie("No external patch command available\n\n".
                                       "Please run 'o conf init /patch/'\n\n");
            }
            $patchbin = CPAN::HandleConfig->safe_quote($patchbin);
            local $ENV{PATCH_GET} = 0; # formerly known as -g0
            unless ($stdpatchargs) {
                my $system = "$patchbin --version |";
                local *FH;
                open FH, $system or die "Could not fork '$system': $!";
                local $/ = "\n";
                my $pversion;
              PARSEVERSION: while (<FH>) {
                    if (/^patch\s+([\d\.]+)/) {
                        $pversion = $1;
                        last PARSEVERSION;
                    }
                }
                if ($pversion) {
                    $stdpatchargs = "-N --fuzz=3";
                } else {
                    $stdpatchargs = "-N";
                }
            }
            my $countedpatches = @$patches == 1 ? "1 patch" : (scalar @$patches . " patches");
            $CPAN::Frontend->myprint("Applying $countedpatches:\n");
            my $patches_dir = $CPAN::Config->{patches_dir};
            for my $patch (@$patches) {
                if ($patches_dir && !File::Spec->file_name_is_absolute($patch)) {
                    my $f = File::Spec->catfile($patches_dir, $patch);
                    $patch = $f if -f $f;
                }
                unless (-f $patch) {
                    CPAN->debug("not on disk: patch[$patch]") if $CPAN::DEBUG;
                    if (my $trydl = $self->try_download($patch)) {
                        $patch = $trydl;
                    } else {
                        my $fail = "Could not find patch '$patch'";
                        $CPAN::Frontend->mywarn("$fail; cannot continue\n");
                        $self->{unwrapped} = CPAN::Distrostatus->new("NO -- $fail");
                        delete $self->{build_dir};
                        return;
                    }
                }
                $CPAN::Frontend->myprint("  $patch\n");
                my $readfh = CPAN::Tarzip->TIEHANDLE($patch);

                my $pcommand;
                my($ppp,$pfiles) = $self->_patch_p_parameter($readfh);
                if ($ppp eq "applypatch") {
                    $pcommand = "$CPAN::Config->{applypatch} -verbose";
                } else {
                    my $thispatchargs = join " ", $stdpatchargs, $ppp;
                    $pcommand = "$patchbin $thispatchargs";
                    require Config; # usually loaded from CPAN.pm
                    if ($Config::Config{osname} eq "solaris") {
                        # native solaris patch cannot patch readonly files
                        for my $file (@{$pfiles||[]}) {
                            my @stat = stat $file or next;
                            chmod $stat[2] | 0600, $file; # may fail
                        }
                    }
                }

                $readfh = CPAN::Tarzip->TIEHANDLE($patch); # open again
                my $writefh = FileHandle->new;
                $CPAN::Frontend->myprint("  $pcommand\n");
                unless (open $writefh, "|$pcommand") {
                    my $fail = "Could not fork '$pcommand'";
                    $CPAN::Frontend->mywarn("$fail; cannot continue\n");
                    $self->{unwrapped} = CPAN::Distrostatus->new("NO -- $fail");
                    delete $self->{build_dir};
                    return;
                }
                binmode($writefh);
                while (my $x = $readfh->READLINE) {
                    print $writefh $x;
                }
                unless (close $writefh) {
                    my $fail = "Could not apply patch '$patch'";
                    $CPAN::Frontend->mywarn("$fail; cannot continue\n");
                    $self->{unwrapped} = CPAN::Distrostatus->new("NO -- $fail");
                    delete $self->{build_dir};
                    return;
                }
            }
            $self->{patched}++;
        }
        return 1;
    }
}

# may return
# - "applypatch"
# - ("-p0"|"-p1", $files)
sub _patch_p_parameter {
    my($self,$fh) = @_;
    my $cnt_files   = 0;
    my $cnt_p0files = 0;
    my @files;
    local($_);
    while ($_ = $fh->READLINE) {
        if (
            $CPAN::Config->{applypatch}
            &&
            /\#\#\#\# ApplyPatch data follows \#\#\#\#/
           ) {
            return "applypatch"
        }
        next unless /^[\*\+]{3}\s(\S+)/;
        my $file = $1;
        push @files, $file;
        $cnt_files++;
        $cnt_p0files++ if -f $file;
        CPAN->debug("file[$file]cnt_files[$cnt_files]cnt_p0files[$cnt_p0files]")
            if $CPAN::DEBUG;
    }
    return "-p1" unless $cnt_files;
    my $opt_p = $cnt_files==$cnt_p0files ? "-p0" : "-p1";
    return ($opt_p, \@files);
}

#-> sub CPAN::Distribution::_edge_cases
# with "configure" or "Makefile" or single file scripts
sub _edge_cases {
    my($self,$mpl,$local_file) = @_;
    $self->debug(sprintf("makefilepl[%s]anycwd[%s]",
                         $mpl,
                         CPAN::anycwd(),
                        )) if $CPAN::DEBUG;
    my $build_dir = $self->{build_dir};
    my($configure) = File::Spec->catfile($build_dir,"Configure");
    if (-f $configure) {
        # do we have anything to do?
        $self->{configure} = $configure;
    } elsif (-f File::Spec->catfile($build_dir,"Makefile")) {
        $CPAN::Frontend->mywarn(qq{
Package comes with a Makefile and without a Makefile.PL.
We\'ll try to build it with that Makefile then.
});
        $self->{writemakefile} = CPAN::Distrostatus->new("YES");
        $CPAN::Frontend->mysleep(2);
    } else {
        my $cf = $self->called_for || "unknown";
        if ($cf =~ m|/|) {
            $cf =~ s|.*/||;
            $cf =~ s|\W.*||;
        }
        $cf =~ s|[/\\:]||g;     # risk of filesystem damage
        $cf = "unknown" unless length($cf);
        if (my $crud = $self->_contains_crud($build_dir)) {
            my $why = qq{Package contains $crud; not recognized as a perl package, giving up};
            $CPAN::Frontend->mywarn("$why\n");
            $self->{writemakefile} = CPAN::Distrostatus->new(qq{NO -- $why});
            return;
        }
        $CPAN::Frontend->mywarn(qq{Package seems to come without Makefile.PL.
  (The test -f "$mpl" returned false.)
  Writing one on our own (setting NAME to $cf)\a\n});
        $self->{had_no_makefile_pl}++;
        $CPAN::Frontend->mysleep(3);

        # Writing our own Makefile.PL

        my $exefile_stanza = "";
        if ($self->{archived} eq "maybe_pl") {
            $exefile_stanza = $self->_exefile_stanza($build_dir,$local_file);
        }

        my $fh = FileHandle->new;
        $fh->open(">$mpl")
            or Carp::croak("Could not open >$mpl: $!");
        $fh->print(
                   qq{# This Makefile.PL has been autogenerated by the module CPAN.pm
# because there was no Makefile.PL supplied.
# Autogenerated on: }.scalar localtime().qq{

use ExtUtils::MakeMaker;
WriteMakefile(
              NAME => q[$cf],$exefile_stanza
             );
});
        $fh->close;
    }
}

#-> CPAN;:Distribution::_contains_crud
sub _contains_crud {
    my($self,$dir) = @_;
    my(@dirs, $dh, @files);
    opendir $dh, $dir or return;
    my $dirent;
    for $dirent (readdir $dh) {
        next if $dirent =~ /^\.\.?$/;
        my $path = File::Spec->catdir($dir,$dirent);
        if (-d $path) {
            push @dirs, $dirent;
        } elsif (-f $path) {
            push @files, $dirent;
        }
    }
    if (@dirs && @files) {
        return "both files[@files] and directories[@dirs]";
    } elsif (@files > 2) {
        return "several files[@files] but no Makefile.PL or Build.PL";
    }
    return;
}

#-> CPAN;:Distribution::_exefile_stanza
sub _exefile_stanza {
    my($self,$build_dir,$local_file) = @_;

            my $fh = FileHandle->new;
            my $script_file = File::Spec->catfile($build_dir,$local_file);
            $fh->open($script_file)
                or Carp::croak("Could not open script '$script_file': $!");
            local $/ = "\n";
            # parse name and prereq
            my($state) = "poddir";
            my($name, $prereq) = ("", "");
            while (<$fh>) {
                if ($state eq "poddir" && /^=head\d\s+(\S+)/) {
                    if ($1 eq 'NAME') {
                        $state = "name";
                    } elsif ($1 eq 'PREREQUISITES') {
                        $state = "prereq";
                    }
                } elsif ($state =~ m{^(name|prereq)$}) {
                    if (/^=/) {
                        $state = "poddir";
                    } elsif (/^\s*$/) {
                        # nop
                    } elsif ($state eq "name") {
                        if ($name eq "") {
                            ($name) = /^(\S+)/;
                            $state = "poddir";
                        }
                    } elsif ($state eq "prereq") {
                        $prereq .= $_;
                    }
                } elsif (/^=cut\b/) {
                    last;
                }
            }
            $fh->close;

            for ($name) {
                s{.*<}{};       # strip X<...>
                s{>.*}{};
            }
            chomp $prereq;
            $prereq = join " ", split /\s+/, $prereq;
            my($PREREQ_PM) = join("\n", map {
                s{.*<}{};       # strip X<...>
                s{>.*}{};
                if (/[\s\'\"]/) { # prose?
                } else {
                    s/[^\w:]$//; # period?
                    " "x28 . "'$_' => 0,";
                }
            } split /\s*,\s*/, $prereq);

            if ($name) {
                my $to_file = File::Spec->catfile($build_dir, $name);
                rename $script_file, $to_file
                    or die "Can't rename $script_file to $to_file: $!";
            }

    return "
              EXE_FILES => ['$name'],
              PREREQ_PM => {
$PREREQ_PM
                           },
";
}

#-> CPAN::Distribution::_signature_business
sub _signature_business {
    my($self) = @_;
    my $check_sigs = CPAN::HandleConfig->prefs_lookup($self,
                                                      q{check_sigs});
    if ($check_sigs) {
        if ($CPAN::META->has_inst("Module::Signature")) {
            if (-f "SIGNATURE") {
                $self->debug("Module::Signature is installed, verifying") if $CPAN::DEBUG;
                my $rv = Module::Signature::verify();
                if ($rv != Module::Signature::SIGNATURE_OK() and
                    $rv != Module::Signature::SIGNATURE_MISSING()) {
                    $CPAN::Frontend->mywarn(
                                            qq{\nSignature invalid for }.
                                            qq{distribution file. }.
                                            qq{Please investigate.\n\n}
                                           );

                    my $wrap =
                        sprintf(qq{I'd recommend removing %s. Some error occurred   }.
                                qq{while checking its signature, so it could        }.
                                qq{be invalid. Maybe you have configured            }.
                                qq{your 'urllist' with a bad URL. Please check this }.
                                qq{array with 'o conf urllist' and retry. Or        }.
                                qq{examine the distribution in a subshell. Try
  look %s
and run
  cpansign -v
},
                                $self->{localfile},
                                $self->pretty_id,
                               );
                    $self->{signature_verify} = CPAN::Distrostatus->new("NO");
                    $CPAN::Frontend->mywarn(Text::Wrap::wrap("","",$wrap));
                    $CPAN::Frontend->mysleep(5) if $CPAN::Frontend->can("mysleep");
                } else {
                    $self->{signature_verify} = CPAN::Distrostatus->new("YES");
                    $self->debug("Module::Signature has verified") if $CPAN::DEBUG;
                }
            } else {
                $CPAN::Frontend->mywarn(qq{Package came without SIGNATURE\n\n});
            }
        } else {
            $self->debug("Module::Signature is NOT installed") if $CPAN::DEBUG;
        }
    }
}

#-> CPAN::Distribution::untar_me ;
sub untar_me {
    my($self,$ct) = @_;
    $self->{archived} = "tar";
    my $result = eval { $ct->untar() };
    if ($result) {
        $self->{unwrapped} = CPAN::Distrostatus->new("YES");
    } else {
        # unfortunately we have no $@ here, Tarzip is using mydie which dies with "\n"
        $self->{unwrapped} = CPAN::Distrostatus->new("NO -- untar failed");
    }
}

# CPAN::Distribution::unzip_me ;
sub unzip_me {
    my($self,$ct) = @_;
    $self->{archived} = "zip";
    if (eval { $ct->unzip() }) {
        $self->{unwrapped} = CPAN::Distrostatus->new("YES");
    } else {
        $self->{unwrapped} = CPAN::Distrostatus->new("NO -- unzip failed during unzip");
    }
    return;
}

sub handle_singlefile {
    my($self,$local_file) = @_;

    if ( $local_file =~ /\.pm(\.(gz|Z))?(?!\n)\Z/ ) {
        $self->{archived} = "pm";
    } elsif ( $local_file =~ /\.patch(\.(gz|bz2))?(?!\n)\Z/ ) {
        $self->{archived} = "patch";
    } else {
        $self->{archived} = "maybe_pl";
    }

    my $to = File::Basename::basename($local_file);
    if ($to =~ s/\.(gz|Z)(?!\n)\Z//) {
        if (eval{CPAN::Tarzip->new($local_file)->gunzip($to)}) {
            $self->{unwrapped} = CPAN::Distrostatus->new("YES");
        } else {
            $self->{unwrapped} = CPAN::Distrostatus->new("NO -- uncompressing failed");
        }
    } else {
        if (File::Copy::cp($local_file,".")) {
            $self->{unwrapped} = CPAN::Distrostatus->new("YES");
        } else {
            $self->{unwrapped} = CPAN::Distrostatus->new("NO -- copying failed");
        }
    }
    return $to;
}

#-> sub CPAN::Distribution::new ;
sub new {
    my($class,%att) = @_;

    # $CPAN::META->{cachemgr} ||= CPAN::CacheMgr->new();

    my $this = { %att };
    return bless $this, $class;
}

#-> sub CPAN::Distribution::look ;
sub look {
    my($self) = @_;

    if ($^O eq 'MacOS') {
      $self->Mac::BuildTools::look;
      return;
    }

    if (  $CPAN::Config->{'shell'} ) {
        $CPAN::Frontend->myprint(qq{
Trying to open a subshell in the build directory...
});
    } else {
        $CPAN::Frontend->myprint(qq{
Your configuration does not define a value for subshells.
Please define it with "o conf shell <your shell>"
});
        return;
    }
    my $dist = $self->id;
    my $dir;
    unless ($dir = $self->dir) {
        $self->get;
    }
    unless ($dir ||= $self->dir) {
        $CPAN::Frontend->mywarn(qq{
Could not determine which directory to use for looking at $dist.
});
        return;
    }
    my $pwd  = CPAN::anycwd();
    $self->safe_chdir($dir);
    $CPAN::Frontend->myprint(qq{Working directory is $dir\n});
    {
        local $ENV{CPAN_SHELL_LEVEL} = $ENV{CPAN_SHELL_LEVEL}||0;
        $ENV{CPAN_SHELL_LEVEL} += 1;
        my $shell = CPAN::HandleConfig->safe_quote($CPAN::Config->{'shell'});

        local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
            ? $ENV{PERL5LIB}
                : ($ENV{PERLLIB} || "");

        local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
        # local $ENV{PERL_USE_UNSAFE_INC} = exists $ENV{PERL_USE_UNSAFE_INC} ? $ENV{PERL_USE_UNSAFE_INC} : 1; # look
        $CPAN::META->set_perl5lib;
        local $ENV{MAKEFLAGS}; # protect us from outer make calls

        unless (system($shell) == 0) {
            my $code = $? >> 8;
            $CPAN::Frontend->mywarn("Subprocess shell exit code $code\n");
        }
    }
    $self->safe_chdir($pwd);
}

# CPAN::Distribution::cvs_import ;
sub cvs_import {
    my($self) = @_;
    $self->get;
    my $dir = $self->dir;

    my $package = $self->called_for;
    my $module = $CPAN::META->instance('CPAN::Module', $package);
    my $version = $module->cpan_version;

    my $userid = $self->cpan_userid;

    my $cvs_dir = (split /\//, $dir)[-1];
    $cvs_dir =~ s/-\d+[^-]+(?!\n)\Z//;
    my $cvs_root =
      $CPAN::Config->{cvsroot} || $ENV{CVSROOT};
    my $cvs_site_perl =
      $CPAN::Config->{cvs_site_perl} || $ENV{CVS_SITE_PERL};
    if ($cvs_site_perl) {
        $cvs_dir = "$cvs_site_perl/$cvs_dir";
    }
    my $cvs_log = qq{"imported $package $version sources"};
    $version =~ s/\./_/g;
    # XXX cvs: undocumented and unclear how it was meant to work
    my @cmd = ('cvs', '-d', $cvs_root, 'import', '-m', $cvs_log,
               "$cvs_dir", $userid, "v$version");

    my $pwd  = CPAN::anycwd();
    chdir($dir) or $CPAN::Frontend->mydie(qq{Could not chdir to "$dir": $!});

    $CPAN::Frontend->myprint(qq{Working directory is $dir\n});

    $CPAN::Frontend->myprint(qq{@cmd\n});
    system(@cmd) == 0 or
    # XXX cvs
        $CPAN::Frontend->mydie("cvs import failed");
    chdir($pwd) or $CPAN::Frontend->mydie(qq{Could not chdir to "$pwd": $!});
}

#-> sub CPAN::Distribution::readme ;
sub readme {
    my($self) = @_;
    my($dist) = $self->id;
    my($sans,$suffix) = $dist =~ /(.+)\.(tgz|tar[\._-]gz|tar\.Z|zip)$/;
    $self->debug("sans[$sans] suffix[$suffix]\n") if $CPAN::DEBUG;
    my($local_file);
    my($local_wanted) =
        File::Spec->catfile(
                            $CPAN::Config->{keep_source_where},
                            "authors",
                            "id",
                            split(/\//,"$sans.readme"),
                           );
    my $readme = "authors/id/$sans.readme";
    $self->debug("Doing localize for '$readme'") if $CPAN::DEBUG;
    $local_file = CPAN::FTP->localize($readme,
                                      $local_wanted)
        or $CPAN::Frontend->mydie(qq{No $sans.readme found});

    if ($^O eq 'MacOS') {
        Mac::BuildTools::launch_file($local_file);
        return;
    }

    my $fh_pager = FileHandle->new;
    local($SIG{PIPE}) = "IGNORE";
    my $pager = $CPAN::Config->{'pager'} || "cat";
    $fh_pager->open("|$pager")
        or die "Could not open pager $pager\: $!";
    my $fh_readme = FileHandle->new;
    $fh_readme->open($local_file)
        or $CPAN::Frontend->mydie(qq{Could not open "$local_file": $!});
    $CPAN::Frontend->myprint(qq{
Displaying file
  $local_file
with pager "$pager"
});
    $fh_pager->print(<$fh_readme>);
    $fh_pager->close;
}

#-> sub CPAN::Distribution::verifyCHECKSUM ;
sub verifyCHECKSUM {
    my($self) = @_;
  EXCUSE: {
        my @e;
        $self->{CHECKSUM_STATUS} ||= "";
        $self->{CHECKSUM_STATUS} eq "OK" and push @e, "Checksum was ok";
        $CPAN::Frontend->myprint(join "", map {"  $_\n"} @e) and return if @e;
    }
    my($lc_want,$lc_file,@local,$basename);
    @local = split(/\//,$self->id);
    pop @local;
    push @local, "CHECKSUMS";
    $lc_want =
        File::Spec->catfile($CPAN::Config->{keep_source_where},
                            "authors", "id", @local);
    local($") = "/";
    if (my $size = -s $lc_want) {
        $self->debug("lc_want[$lc_want]size[$size]") if $CPAN::DEBUG;
        my @stat = stat $lc_want;
        my $epoch_starting_support_of_cpan_path = 1637471530;
        if ($stat[9] >= $epoch_starting_support_of_cpan_path) {
            if ($self->CHECKSUM_check_file($lc_want, 1)) {
                return $self->{CHECKSUM_STATUS} = "OK";
            }
        } else {
            unlink $lc_want;
        }
    }
    $lc_file = CPAN::FTP->localize("authors/id/@local",
                                   $lc_want,1);
    unless ($lc_file) {
        $CPAN::Frontend->myprint("Trying $lc_want.gz\n");
        $local[-1] .= ".gz";
        $lc_file = CPAN::FTP->localize("authors/id/@local",
                                       "$lc_want.gz",1);
        if ($lc_file) {
            $lc_file =~ s/\.gz(?!\n)\Z//;
            eval{CPAN::Tarzip->new("$lc_file.gz")->gunzip($lc_file)};
        } else {
            return;
        }
    }
    if ($self->CHECKSUM_check_file($lc_file)) {
        return $self->{CHECKSUM_STATUS} = "OK";
    }
}

#-> sub CPAN::Distribution::SIG_check_file ;
sub SIG_check_file {
    my($self,$chk_file) = @_;
    my $rv = eval { Module::Signature::_verify($chk_file) };

    if ($rv eq Module::Signature::CANNOT_VERIFY()) {
        $CPAN::Frontend->myprint(qq{\nSignature for }.
                                 qq{file $chk_file could not be verified for an unknown reason. }.
                                 $self->as_string.
                                 qq{Module::Signature verification returned value $rv\n\n}
                                );

        my $wrap = qq{The manual says for this case: Cannot verify the
OpenPGP signature, maybe due to the lack of a network connection to
the key server, or if neither gnupg nor Crypt::OpenPGP exists on the
system. You probably want to analyse the situation and if you cannot
fix it you will have to decide whether you want to stop this session
or you want to turn off signature verification. The latter would be
done with the command 'o conf init check_sigs'};

        $CPAN::Frontend->mydie(Text::Wrap::wrap("","",$wrap));
    } if ($rv == Module::Signature::SIGNATURE_OK()) {
        $CPAN::Frontend->myprint("Signature for $chk_file ok\n");
        return $self->{SIG_STATUS} = "OK";
    } else {
        $CPAN::Frontend->mywarn(qq{\nSignature invalid for }.
                                 qq{file $chk_file. }.
                                 qq{Please investigate.\n\n}.
                                 $self->as_string.
                                 qq{Module::Signature verification returned value $rv\n\n}
                                );

        my $wrap = qq{I\'d recommend removing $chk_file. Its signature
is invalid. Maybe you have configured your 'urllist' with
a bad URL. Please check this array with 'o conf urllist', and
retry.};

        $CPAN::Frontend->mydie(Text::Wrap::wrap("","",$wrap));
    }
}

#-> sub CPAN::Distribution::CHECKSUM_check_file ;

# sloppy is 1 when we have an old checksums file that maybe is good
# enough

sub CHECKSUM_check_file {
    my($self,$chk_file,$sloppy) = @_;
    my($cksum,$file,$basename);

    $sloppy ||= 0;
    $self->debug("chk_file[$chk_file]sloppy[$sloppy]") if $CPAN::DEBUG;
    my $check_sigs = CPAN::HandleConfig->prefs_lookup($self,
                                                      q{check_sigs});
    if ($check_sigs) {
        if ($CPAN::META->has_inst("Module::Signature")) {
            $self->debug("Module::Signature is installed, verifying") if $CPAN::DEBUG;
            $self->SIG_check_file($chk_file);
        } else {
            $self->debug("Module::Signature is NOT installed") if $CPAN::DEBUG;
        }
    }

    $file = $self->{localfile};
    $basename = File::Basename::basename($file);
    my($signed_data);
    my $fh = FileHandle->new;
    if ($check_sigs) {
        my $tempdir;
        if ($CPAN::META->has_usable("File::Temp")) {
            $tempdir = File::Temp::tempdir("CHECKSUMS-XXXX", CLEANUP => 1, DIR => "/tmp" );
        } else {
            $tempdir = File::Spec->catdir(File::Spec->tmpdir, "CHECKSUMS-$$");
            File::Path::mkpath($tempdir);
        }
        my $tempfile = File::Spec->catfile($tempdir, "CHECKSUMS.$$");
        unlink $tempfile; # ignore missing file
        my $devnull = File::Spec->devnull;
        my $gpg = $CPAN::Config->{gpg} or
            $CPAN::Frontend->mydie("Your configuration suggests that you do not have 'gpg' installed. This is needed to verify checksums with the config variable 'check_sigs' on. Please configure it with 'o conf init gpg'");
        my $system = qq{"$gpg" --verify --batch --no-tty --output "$tempfile" "$chk_file" 2> "$devnull"};
        0 == system $system or $CPAN::Frontend->mydie("gpg run was failing, cannot continue: $system");
        open $fh, $tempfile or $CPAN::Frontend->mydie("Could not open $tempfile: $!");
        local $/;
        $signed_data = <$fh>;
        close $fh;
        File::Path::rmtree($tempdir);
    } else {
        my $fh = FileHandle->new;
        if (open $fh, $chk_file) {
            local($/);
            $signed_data = <$fh>;
        } else {
            $CPAN::Frontend->mydie("Could not open $chk_file for reading");
        }
        close $fh;
    }
    $signed_data =~ s/\015?\012/\n/g;
    my($compmt) = Safe->new();
    $cksum = $compmt->reval($signed_data);
    if ($@) {
        rename $chk_file, "$chk_file.bad";
        Carp::confess($@) if $@;
    }

    if (! ref $cksum or ref $cksum ne "HASH") {
        $CPAN::Frontend->mywarn(qq{
Warning: checksum file '$chk_file' broken.

When trying to read that file I expected to get a hash reference
for further processing, but got garbage instead.
});
        my $answer = CPAN::Shell::colorable_makemaker_prompt("Proceed nonetheless?", "no");
        $answer =~ /^\s*y/i or $CPAN::Frontend->mydie("Aborted.\n");
        $self->{CHECKSUM_STATUS} = "NIL -- CHECKSUMS file broken";
        return;
    } elsif (exists $cksum->{$basename} && ! exists $cksum->{$basename}{cpan_path}) {
        $CPAN::Frontend->mywarn(qq{
Warning: checksum file '$chk_file' not conforming.

The cksum does not contain the key 'cpan_path' for '$basename'.
});
        my $answer = CPAN::Shell::colorable_makemaker_prompt("Proceed nonetheless?", "no");
        $answer =~ /^\s*y/i or $CPAN::Frontend->mydie("Aborted.\n");
        $self->{CHECKSUM_STATUS} = "NIL -- CHECKSUMS file without cpan_path";
        return;
    } elsif (exists $cksum->{$basename} && substr($self->{ID},0,length($cksum->{$basename}{cpan_path}))
             ne $cksum->{$basename}{cpan_path}) {
        $CPAN::Frontend->mywarn(qq{
Warning: checksum file not matching path '$self->{ID}'.

The cksum contain the key 'cpan_path=$cksum->{$basename}{cpan_path}'
which does not match the ID of the distribution '$self->{ID}'.
Something's suspicious might be going on here. Please investigate.

});
        my $answer = CPAN::Shell::colorable_makemaker_prompt("Proceed nonetheless?", "no");
        $answer =~ /^\s*y/i or $CPAN::Frontend->mydie("Aborted.\n");
        $self->{CHECKSUM_STATUS} = "NIL -- CHECKSUMS non-matching cpan_path vs. ID";
        return;
    } elsif (exists $cksum->{$basename}{sha256}) {
        $self->debug("Found checksum for $basename:" .
                     "$cksum->{$basename}{sha256}\n") if $CPAN::DEBUG;

        open($fh, $file);
        binmode $fh;
        my $eq = $self->eq_CHECKSUM($fh,$cksum->{$basename}{sha256});
        $fh->close;
        $fh = CPAN::Tarzip->TIEHANDLE($file);

        unless ($eq) {
            my $dg = Digest::SHA->new(256);
            my($data,$ref);
            $ref = \$data;
            while ($fh->READ($ref, 4096) > 0) {
                $dg->add($data);
            }
            my $hexdigest = $dg->hexdigest;
            $eq += $hexdigest eq $cksum->{$basename}{'sha256-ungz'};
        }

        if ($eq) {
            $CPAN::Frontend->myprint("Checksum for $file ok\n");
            return $self->{CHECKSUM_STATUS} = "OK";
        } else {
            $CPAN::Frontend->myprint(qq{\nChecksum mismatch for }.
                                     qq{distribution file. }.
                                     qq{Please investigate.\n\n}.
                                     $self->as_string,
                                     $CPAN::META->instance(
                                                           'CPAN::Author',
                                                           $self->cpan_userid
                                                          )->as_string);

            my $wrap = qq{I\'d recommend removing $file. Its
checksum is incorrect. Maybe you have configured your 'urllist' with
a bad URL. Please check this array with 'o conf urllist', and
retry.};

            $CPAN::Frontend->mydie(Text::Wrap::wrap("","",$wrap));

            # former versions just returned here but this seems a
            # serious threat that deserves a die

            # $CPAN::Frontend->myprint("\n\n");
            # sleep 3;
            # return;
        }
        # close $fh if fileno($fh);
    } else {
        return if $sloppy;
        unless ($self->{CHECKSUM_STATUS}) {
            $CPAN::Frontend->mywarn(qq{
Warning: No checksum for $basename in $chk_file.

The cause for this may be that the file is very new and the checksum
has not yet been calculated, but it may also be that something is
going awry right now.
});
            my $answer = CPAN::Shell::colorable_makemaker_prompt("Proceed?", "yes");
            $answer =~ /^\s*y/i or $CPAN::Frontend->mydie("Aborted.\n");
        }
        $self->{CHECKSUM_STATUS} = "NIL -- distro not in CHECKSUMS file";
        return;
    }
}

#-> sub CPAN::Distribution::eq_CHECKSUM ;
sub eq_CHECKSUM {
    my($self,$fh,$expect) = @_;
    if ($CPAN::META->has_inst("Digest::SHA")) {
        my $dg = Digest::SHA->new(256);
        my($data);
        while (read($fh, $data, 4096)) {
            $dg->add($data);
        }
        my $hexdigest = $dg->hexdigest;
        # warn "fh[$fh] hex[$hexdigest] aexp[$expectMD5]";
        return $hexdigest eq $expect;
    }
    return 1;
}

#-> sub CPAN::Distribution::force ;

# Both CPAN::Modules and CPAN::Distributions know if "force" is in
# effect by autoinspection, not by inspecting a global variable. One
# of the reason why this was chosen to work that way was the treatment
# of dependencies. They should not automatically inherit the force
# status. But this has the downside that ^C and die() will return to
# the prompt but will not be able to reset the force_update
# attributes. We try to correct for it currently in the read_metadata
# routine, and immediately before we check for a Signal. I hope this
# works out in one of v1.57_53ff

# "Force get forgets previous error conditions"

#-> sub CPAN::Distribution::fforce ;
sub fforce {
  my($self, $method) = @_;
  $self->force($method,1);
}

#-> sub CPAN::Distribution::force ;
sub force {
  my($self, $method,$fforce) = @_;
  my %phase_map = (
                   get => [
                           "unwrapped",
                           "build_dir",
                           "archived",
                           "localfile",
                           "CHECKSUM_STATUS",
                           "signature_verify",
                           "prefs",
                           "prefs_file",
                           "prefs_file_doc",
                           "cleanup_after_install_done",
                          ],
                   make => [
                            "writemakefile",
                            "make",
                            "modulebuild",
                            "prereq_pm",
                            "cleanup_after_install_done",
                           ],
                   test => [
                            "badtestcnt",
                            "make_test",
                            "cleanup_after_install_done",
                          ],
                   install => [
                               "install",
                               "cleanup_after_install_done",
                              ],
                   unknown => [
                               "reqtype",
                               "yaml_content",
                               "cleanup_after_install_done",
                              ],
                  );
  my $methodmatch = 0;
  my $ldebug = 0;
 PHASE: for my $phase (qw(unknown get make test install)) { # order matters
      $methodmatch = 1 if $fforce || ($method && $phase eq $method);
      next unless $methodmatch;
    ATTRIBUTE: for my $att (@{$phase_map{$phase}}) {
          if ($phase eq "get") {
              if (substr($self->id,-1,1) eq "."
                  && $att =~ /(unwrapped|build_dir|archived)/ ) {
                  # cannot be undone for local distros
                  next ATTRIBUTE;
              }
              if ($att eq "build_dir"
                  && $self->{build_dir}
                  && $CPAN::META->{is_tested}
                 ) {
                  delete $CPAN::META->{is_tested}{$self->{build_dir}};
              }
          } elsif ($phase eq "test") {
              if ($att eq "make_test"
                  && $self->{make_test}
                  && $self->{make_test}{COMMANDID}
                  && $self->{make_test}{COMMANDID} == $CPAN::CurrentCommandId
                 ) {
                  # endless loop too likely
                  next ATTRIBUTE;
              }
          }
          delete $self->{$att};
          if ($ldebug || $CPAN::DEBUG) {
              # local $CPAN::DEBUG = 16; # Distribution
              CPAN->debug(sprintf "id[%s]phase[%s]att[%s]", $self->id, $phase, $att);
          }
      }
  }
  if ($method && $method =~ /make|test|install/) {
    $self->{force_update} = 1; # name should probably have been force_install
  }
}

#-> sub CPAN::Distribution::notest ;
sub notest {
  my($self, $method) = @_;
  # $CPAN::Frontend->mywarn("XDEBUG: set notest for $self $method");
  $self->{"notest"}++; # name should probably have been force_install
}

#-> sub CPAN::Distribution::unnotest ;
sub unnotest {
  my($self) = @_;
  # warn "XDEBUG: deleting notest";
  delete $self->{notest};
}

#-> sub CPAN::Distribution::unforce ;
sub unforce {
  my($self) = @_;
  delete $self->{force_update};
}

#-> sub CPAN::Distribution::isa_perl ;
sub isa_perl {
  my($self) = @_;
  my $file = File::Basename::basename($self->id);
  if ($file =~ m{ ^ perl
                  (
                   -(5\.\d+\.\d+)
                   |
                   (5)[._-](00[0-5](?:_[0-4][0-9])?)
                  )
                  \.tar[._-](?:gz|bz2)
                  (?!\n)\Z
                }xs) {
    my $perl_version;
    if ($2) {
        $perl_version = $2;
    } else {
        $perl_version = "$3.$4";
    }
    return $perl_version;
  } elsif ($self->cpan_comment
           &&
           $self->cpan_comment =~ /isa_perl\(.+?\)/) {
    return $1;
  }
}


#-> sub CPAN::Distribution::perl ;
sub perl {
    my ($self) = @_;
    if (! $self) {
        use Carp qw(carp);
        carp __PACKAGE__ . "::perl was called without parameters.";
    }
    return CPAN::HandleConfig->safe_quote($CPAN::Perl);
}

#-> sub CPAN::Distribution::shortcut_prepare ;
# return values: undef means don't shortcut; 0 means shortcut as fail;
# and 1 means shortcut as success

sub shortcut_prepare {
    my ($self) = @_;

    $self->debug("checking archive type[$self->{ID}]") if $CPAN::DEBUG;
    if (!$self->{archived} || $self->{archived} eq "NO") {
        return $self->goodbye("Is neither a tar nor a zip archive.");
    }

    $self->debug("checking unwrapping[$self->{ID}]") if $CPAN::DEBUG;
    if (!$self->{unwrapped}
        || (
            UNIVERSAL::can($self->{unwrapped},"failed") ?
            $self->{unwrapped}->failed :
            $self->{unwrapped} =~ /^NO/
            )) {
        return $self->goodbye("Had problems unarchiving. Please build manually");
    }

    $self->debug("checking signature[$self->{ID}]") if $CPAN::DEBUG;
    if ( ! $self->{force_update}
        && exists $self->{signature_verify}
        && (
                UNIVERSAL::can($self->{signature_verify},"failed") ?
                $self->{signature_verify}->failed :
                $self->{signature_verify} =~ /^NO/
            )
    ) {
        return $self->goodbye("Did not pass the signature test.");
    }

    $self->debug("checking writemakefile[$self->{ID}]") if $CPAN::DEBUG;
    if ($self->{writemakefile}) {
        if (
                UNIVERSAL::can($self->{writemakefile},"failed") ?
                $self->{writemakefile}->failed :
                $self->{writemakefile} =~ /^NO/
            ) {
            # XXX maybe a retry would be in order?
            my $err = UNIVERSAL::can($self->{writemakefile},"text") ?
                $self->{writemakefile}->text :
                    $self->{writemakefile};
            $err =~ s/^NO\s*(--\s+)?//;
            $err ||= "Had some problem writing Makefile";
            $err .= ", not re-running";
            return $self->goodbye($err);
        } else {
            return $self->success("Has already been prepared");
        }
    }

    $self->debug("checking configure_requires_later[$self->{ID}]") if $CPAN::DEBUG;
    if( my $later = $self->{configure_requires_later} ) { # see also undelay
        return $self->goodbye($later);
    }

    return undef; # no shortcut
}

sub prepare {
    my ($self) = @_;

    $self->get
        or return;

    if ( defined( my $sc = $self->shortcut_prepare) ) {
        return $sc;
    }

    local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
                           ? $ENV{PERL5LIB}
                           : ($ENV{PERLLIB} || "");
    local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
    local $ENV{PERL_USE_UNSAFE_INC} =
        exists $ENV{PERL_USE_UNSAFE_INC} && defined $ENV{PERL_USE_UNSAFE_INC}
        ? $ENV{PERL_USE_UNSAFE_INC} : 1; # prepare
    $CPAN::META->set_perl5lib;
    local $ENV{MAKEFLAGS}; # protect us from outer make calls

    if ($CPAN::Signal) {
        delete $self->{force_update};
        return;
    }

    my $builddir = $self->dir or
        $CPAN::Frontend->mydie("PANIC: Cannot determine build directory\n");

    unless (chdir $builddir) {
        $CPAN::Frontend->mywarn("Couldn't chdir to '$builddir': $!");
        return;
    }

    if ($CPAN::Signal) {
        delete $self->{force_update};
        return;
    }

    $self->debug("Changed directory to $builddir") if $CPAN::DEBUG;

    local $ENV{PERL_AUTOINSTALL} = $ENV{PERL_AUTOINSTALL} || '';
    local $ENV{PERL_EXTUTILS_AUTOINSTALL} = $ENV{PERL_EXTUTILS_AUTOINSTALL} || '';
    $self->choose_MM_or_MB
        or return;

    my $configurator = $self->{configure} ? "Configure"
                     : $self->{modulebuild} ? "Build.PL"
                     : "Makefile.PL";

    $CPAN::Frontend->myprint("Configuring ".$self->id." with $configurator\n");

    if ($CPAN::Config->{prerequisites_policy} eq "follow") {
        $ENV{PERL_AUTOINSTALL}          ||= "--defaultdeps";
        $ENV{PERL_EXTUTILS_AUTOINSTALL} ||= "--defaultdeps";
    }

    my $system;
    my $pl_commandline;
    if ($self->prefs->{pl}) {
        $pl_commandline = $self->prefs->{pl}{commandline};
    }
    local $ENV{PERL} = defined $ENV{PERL}? $ENV{PERL} : $^X;
    local $ENV{PERL5_CPAN_IS_EXECUTING} = $ENV{PERL5_CPAN_IS_EXECUTING} || '';
    local $ENV{PERL_MM_USE_DEFAULT} = 1 if $CPAN::Config->{use_prompt_default};
    local $ENV{NONINTERACTIVE_TESTING} = 1 if $CPAN::Config->{use_prompt_default};
    if ($pl_commandline) {
        $system = $pl_commandline;
        $ENV{PERL} = $^X;
    } elsif ($self->{'configure'}) {
        $system = $self->{'configure'};
    } elsif ($self->{modulebuild}) {
        my($perl) = $self->perl or die "Couldn\'t find executable perl\n";
        my $mbuildpl_arg = $self->_make_phase_arg("pl");
        $system = sprintf("%s Build.PL%s",
                          $perl,
                          $mbuildpl_arg ? " $mbuildpl_arg" : "",
                         );
    } else {
        my($perl) = $self->perl or die "Couldn\'t find executable perl\n";
        my $switch = "";
# This needs a handler that can be turned on or off:
#        $switch = "-MExtUtils::MakeMaker ".
#            "-Mops=:default,:filesys_read,:filesys_open,require,chdir"
#            if $] > 5.00310;
        my $makepl_arg = $self->_make_phase_arg("pl");
        $ENV{PERL5_CPAN_IS_EXECUTING} = File::Spec->catfile($self->{build_dir},
                                                            "Makefile.PL");
        $system = sprintf("%s%s Makefile.PL%s",
                          $perl,
                          $switch ? " $switch" : "",
                          $makepl_arg ? " $makepl_arg" : "",
                         );
    }
    my $pl_env;
    if ($self->prefs->{pl}) {
        $pl_env = $self->prefs->{pl}{env};
    }
    local @ENV{keys %$pl_env} = values %$pl_env if $pl_env;
    if (exists $self->{writemakefile}) {
    } else {
        local($SIG{ALRM}) = sub { die "inactivity_timeout reached\n" };
        my($ret,$pid,$output);
        $@ = "";
        my $go_via_alarm;
        if ($CPAN::Config->{inactivity_timeout}) {
            require Config;
            if ($Config::Config{d_alarm}
                &&
                $Config::Config{d_alarm} eq "define"
               ) {
                $go_via_alarm++
            } else {
                $CPAN::Frontend->mywarn("Warning: you have configured the config ".
                                        "variable 'inactivity_timeout' to ".
                                        "'$CPAN::Config->{inactivity_timeout}'. But ".
                                        "on this machine the system call 'alarm' ".
                                        "isn't available. This means that we cannot ".
                                        "provide the feature of intercepting long ".
                                        "waiting code and will turn this feature off.\n"
                                       );
                $CPAN::Config->{inactivity_timeout} = 0;
            }
        }
        if ($go_via_alarm) {
            if ( $self->_should_report('pl') ) {
                ($output, $ret) = CPAN::Reporter::record_command(
                    $system,
                    $CPAN::Config->{inactivity_timeout},
                );
                CPAN::Reporter::grade_PL( $self, $system, $output, $ret );
            }
            else {
                eval {
                    alarm $CPAN::Config->{inactivity_timeout};
                    local $SIG{CHLD}; # = sub { wait };
                    if (defined($pid = fork)) {
                        if ($pid) { #parent
                            # wait;
                            waitpid $pid, 0;
                        } else {    #child
                            # note, this exec isn't necessary if
                            # inactivity_timeout is 0. On the Mac I'd
                            # suggest, we set it always to 0.
                            exec $system;
                        }
                    } else {
                        $CPAN::Frontend->myprint("Cannot fork: $!");
                        return;
                    }
                };
                alarm 0;
                if ($@) {
                    kill 9, $pid;
                    waitpid $pid, 0;
                    my $err = "$@";
                    $CPAN::Frontend->myprint($err);
                    $self->{writemakefile} = CPAN::Distrostatus->new("NO $err");
                    $@ = "";
                    $self->store_persistent_state;
                    return $self->goodbye("$system -- TIMED OUT");
                }
            }
        } else {
            if (my $expect_model = $self->_prefs_with_expect("pl")) {
                # XXX probably want to check _should_report here and warn
                # about not being able to use CPAN::Reporter with expect
                $ret = $self->_run_via_expect($system,'writemakefile',$expect_model);
                if (! defined $ret
                    && $self->{writemakefile}
                    && $self->{writemakefile}->failed) {
                    # timeout
                    return;
                }
            }
            elsif ( $self->_should_report('pl') ) {
                ($output, $ret) = eval { CPAN::Reporter::record_command($system) };
                if (! defined $output or $@) {
                    my $err = $@ || "Unknown error";
                    $CPAN::Frontend->mywarn("Error while running PL phase: $err\n");
                    $self->{writemakefile} = CPAN::Distrostatus
                        ->new("NO '$system' returned status $ret and no output");
                    return $self->goodbye("$system -- NOT OK");
                }
                CPAN::Reporter::grade_PL( $self, $system, $output, $ret );
            }
            else {
                $ret = system($system);
            }
            if ($ret != 0) {
                $self->{writemakefile} = CPAN::Distrostatus
                    ->new("NO '$system' returned status $ret");
                $CPAN::Frontend->mywarn("Warning: No success on command[$system]\n");
                $self->store_persistent_state;
                return $self->goodbye("$system -- NOT OK");
            }
        }
        if (-f "Makefile" || -f "Build" || ($^O eq 'VMS' && (-f 'descrip.mms' || -f 'Build.com'))) {
            $self->{writemakefile} = CPAN::Distrostatus->new("YES");
            delete $self->{make_clean}; # if cleaned before, enable next
            $self->store_persistent_state;
            return $self->success("$system -- OK");
        } else {
            my $makefile = $self->{modulebuild} ? "Build" : "Makefile";
            my $why = "No '$makefile' created";
            $CPAN::Frontend->mywarn($why);
            $self->{writemakefile} = CPAN::Distrostatus
                ->new(qq{NO -- $why\n});
            $self->store_persistent_state;
            return $self->goodbye("$system -- NOT OK");
        }
    }
    $self->store_persistent_state;
    return 1; # success
}

#-> sub CPAN::Distribution::shortcut_make ;
# return values: undef means don't shortcut; 0 means shortcut as fail;
# and 1 means shortcut as success
sub shortcut_make {
    my ($self) = @_;

    $self->debug("checking make/build results[$self->{ID}]") if $CPAN::DEBUG;
    if (defined $self->{make}) {
        if (UNIVERSAL::can($self->{make},"failed") ?
            $self->{make}->failed :
            $self->{make} =~ /^NO/
        ) {
            if ($self->{force_update}) {
                # Trying an already failed 'make' (unless somebody else blocks)
                return undef; # no shortcut
            } else {
                # introduced for turning recursion detection into a distrostatus
                my $error = length $self->{make}>3
                    ? substr($self->{make},3) : "Unknown error";
                $self->store_persistent_state;
                return $self->goodbye("Could not make: $error\n");
            }
        } else {
            return $self->success("Has already been made")
        }
    }
    return undef; # no shortcut
}

#-> sub CPAN::Distribution::make ;
sub make {
    my($self) = @_;

    $self->pre_make();

    if (exists $self->{cleanup_after_install_done}) {
        $self->post_make();
        return $self->get;
    }

    $self->debug("checking goto id[$self->{ID}]") if $CPAN::DEBUG;
    if (my $goto = $self->prefs->{goto}) {
        $self->post_make();
        return $self->goto($goto);
    }
    # Emergency brake if they said install Pippi and get newest perl

    # XXX Would this make more sense in shortcut_prepare, since
    # that doesn't make sense on a perl dist either?  Broader
    # question: what is the purpose of suggesting force install
    # on a perl distribution?  That seems unlikely to result in
    # such a dependency being satisfied, even if the perl is
    # successfully installed.  This situation is tantamount to
    # a prereq on a version of perl greater than the current one
    # so I think we should just abort. -- xdg, 2012-04-06
    if ($self->isa_perl) {
        if (
            $self->called_for ne $self->id &&
            ! $self->{force_update}
        ) {
            # if we die here, we break bundles
            $CPAN::Frontend
                ->mywarn(sprintf(
                            qq{The most recent version "%s" of the module "%s"
is part of the perl-%s distribution. To install that, you need to run
  force install %s   --or--
  install %s
},
                             $CPAN::META->instance(
                                                   'CPAN::Module',
                                                   $self->called_for
                                                  )->cpan_version,
                             $self->called_for,
                             $self->isa_perl,
                             $self->called_for,
                             $self->pretty_id,
                            ));
            $self->{make} = CPAN::Distrostatus->new("NO isa perl");
            $CPAN::Frontend->mysleep(1);
            $self->post_make();
            return;
        }
    }

    unless ($self->prepare){
        $self->post_make();
        return;
    }

    if ( defined( my $sc = $self->shortcut_make) ) {
        $self->post_make();
        return $sc;
    }

    if ($CPAN::Signal) {
        delete $self->{force_update};
        $self->post_make();
        return;
    }

    my $builddir = $self->dir or
        $CPAN::Frontend->mydie("PANIC: Cannot determine build directory\n");

    unless (chdir $builddir) {
        $CPAN::Frontend->mywarn("Couldn't chdir to '$builddir': $!");
        $self->post_make();
        return;
    }

    my $make = $self->{modulebuild} ? "Build" : "make";
    $CPAN::Frontend->myprint(sprintf "Running %s for %s\n", $make, $self->id);
    local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
                           ? $ENV{PERL5LIB}
                           : ($ENV{PERLLIB} || "");
    local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
    local $ENV{PERL_USE_UNSAFE_INC} =
        exists $ENV{PERL_USE_UNSAFE_INC} && defined $ENV{PERL_USE_UNSAFE_INC}
        ? $ENV{PERL_USE_UNSAFE_INC} : 1; # make
    $CPAN::META->set_perl5lib;
    local $ENV{MAKEFLAGS}; # protect us from outer make calls

    if ($CPAN::Signal) {
        delete $self->{force_update};
        $self->post_make();
        return;
    }

    if ($^O eq 'MacOS') {
        Mac::BuildTools::make($self);
        $self->post_make();
        return;
    }

    my %env;
    while (my($k,$v) = each %ENV) {
        next if defined $v;
        $env{$k} = '';
    }
    local @ENV{keys %env} = values %env;
    my $satisfied = eval { $self->satisfy_requires };
    if ($@) {
        return $self->goodbye($@);
    }
    unless ($satisfied){
        $self->post_make();
        return;
    }
    if ($CPAN::Signal) {
        delete $self->{force_update};
        $self->post_make();
        return;
    }

    # need to chdir again, because $self->satisfy_requires might change the directory
    unless (chdir $builddir) {
        $CPAN::Frontend->mywarn("Couldn't chdir to '$builddir': $!");
        $self->post_make();
        return;
    }

    my $system;
    my $make_commandline;
    if ($self->prefs->{make}) {
        $make_commandline = $self->prefs->{make}{commandline};
    }
    local $ENV{PERL} = defined $ENV{PERL}? $ENV{PERL} : $^X;
    local $ENV{PERL_MM_USE_DEFAULT} = 1 if $CPAN::Config->{use_prompt_default};
    local $ENV{NONINTERACTIVE_TESTING} = 1 if $CPAN::Config->{use_prompt_default};
    if ($make_commandline) {
        $system = $make_commandline;
        $ENV{PERL} = CPAN::find_perl();
    } else {
        if ($self->{modulebuild}) {
            unless (-f "Build" || ($^O eq 'VMS' && -f 'Build.com')) {
                my $cwd = CPAN::anycwd();
                $CPAN::Frontend->mywarn("Alert: no Build file available for 'make $self->{id}'".
                                        " in cwd[$cwd]. Danger, Will Robinson!\n");
                $CPAN::Frontend->mysleep(5);
            }
            $system = join " ", $self->_build_command(), $CPAN::Config->{mbuild_arg};
        } else {
            $system = join " ", $self->_make_command(),  $CPAN::Config->{make_arg};
        }
        $system =~ s/\s+$//;
        my $make_arg = $self->_make_phase_arg("make");
        $system = sprintf("%s%s",
                          $system,
                          $make_arg ? " $make_arg" : "",
                         );
    }
    my $make_env;
    if ($self->prefs->{make}) {
        $make_env = $self->prefs->{make}{env};
    }
    local @ENV{keys %$make_env} = values %$make_env if $make_env;
    my $expect_model = $self->_prefs_with_expect("make");
    my $want_expect = 0;
    if ( $expect_model && @{$expect_model->{talk}} ) {
        my $can_expect = $CPAN::META->has_inst("Expect");
        if ($can_expect) {
            $want_expect = 1;
        } else {
            $CPAN::Frontend->mywarn("Expect not installed, falling back to ".
                                    "system()\n");
        }
    }
    my ($system_ok, $system_err);
    if ($want_expect) {
        # XXX probably want to check _should_report here and
        # warn about not being able to use CPAN::Reporter with expect
        $system_ok = $self->_run_via_expect($system,'make',$expect_model) == 0;
    }
    elsif ( $self->_should_report('make') ) {
        my ($output, $ret) = CPAN::Reporter::record_command($system);
        CPAN::Reporter::grade_make( $self, $system, $output, $ret );
        $system_ok = ! $ret;
    }
    else {
        my $rc = system($system);
        $system_ok = $rc == 0;
        $system_err = $! if $rc == -1;
    }
    $self->introduce_myself;
    if ( $system_ok ) {
        $CPAN::Frontend->myprint("  $system -- OK\n");
        $self->{make} = CPAN::Distrostatus->new("YES");
    } else {
        $self->{writemakefile} ||= CPAN::Distrostatus->new("YES");
        $self->{make} = CPAN::Distrostatus->new("NO");
        $CPAN::Frontend->mywarn("  $system -- NOT OK\n");
        $CPAN::Frontend->mywarn("  $system_err\n") if defined $system_err;
    }
    $self->store_persistent_state;

    $self->post_make();

    return !! $system_ok;
}

# CPAN::Distribution::goodbye ;
sub goodbye {
    my($self,$goodbye) = @_;
    my $id = $self->pretty_id;
    $CPAN::Frontend->mywarn("  $id\n  $goodbye\n");
    return 0; # must be explicit false, not undef
}

sub success {
    my($self,$why) = @_;
    my $id = $self->pretty_id;
    $CPAN::Frontend->myprint("  $id\n  $why\n");
    return 1;
}

# CPAN::Distribution::_run_via_expect ;
sub _run_via_expect {
    my($self,$system,$phase,$expect_model) = @_;
    CPAN->debug("system[$system]expect_model[$expect_model]") if $CPAN::DEBUG;
    if ($CPAN::META->has_inst("Expect")) {
        my $expo = Expect->new;  # expo Expect object;
        $expo->spawn($system);
        $expect_model->{mode} ||= "deterministic";
        if ($expect_model->{mode} eq "deterministic") {
            return $self->_run_via_expect_deterministic($expo,$phase,$expect_model);
        } elsif ($expect_model->{mode} eq "anyorder") {
            return $self->_run_via_expect_anyorder($expo,$phase,$expect_model);
        } else {
            die "Panic: Illegal expect mode: $expect_model->{mode}";
        }
    } else {
        $CPAN::Frontend->mywarn("Expect not installed, falling back to system()\n");
        return system($system);
    }
}

sub _run_via_expect_anyorder {
    my($self,$expo,$phase,$expect_model) = @_;
    my $timeout = $expect_model->{timeout} || 5;
    my $reuse = $expect_model->{reuse};
    my @expectacopy = @{$expect_model->{talk}}; # we trash it!
    my $but = "";
    my $timeout_start = time;
  EXPECT: while () {
        my($eof,$ran_into_timeout);
        # XXX not up to the full power of expect. one could certainly
        # wrap all of the talk pairs into a single expect call and on
        # success tweak it and step ahead to the next question. The
        # current implementation unnecessarily limits itself to a
        # single match.
        my @match = $expo->expect(1,
                                  [ eof => sub {
                                        $eof++;
                                    } ],
                                  [ timeout => sub {
                                        $ran_into_timeout++;
                                    } ],
                                  -re => eval"qr{.}",
                                 );
        if ($match[2]) {
            $but .= $match[2];
        }
        $but .= $expo->clear_accum;
        if ($eof) {
            $expo->soft_close;
            return $expo->exitstatus();
        } elsif ($ran_into_timeout) {
            # warn "DEBUG: they are asking a question, but[$but]";
            for (my $i = 0; $i <= $#expectacopy; $i+=2) {
                my($next,$send) = @expectacopy[$i,$i+1];
                my $regex = eval "qr{$next}";
                # warn "DEBUG: will compare with regex[$regex].";
                if ($but =~ /$regex/) {
                    # warn "DEBUG: will send send[$send]";
                    $expo->send($send);
                    # never allow reusing an QA pair unless they told us
                    splice @expectacopy, $i, 2 unless $reuse;
                    $but =~ s/(?s:^.*?)$regex//;
                    $timeout_start = time;
                    next EXPECT;
                }
            }
            my $have_waited = time - $timeout_start;
            if ($have_waited < $timeout) {
                # warn "DEBUG: have_waited[$have_waited]timeout[$timeout]";
                next EXPECT;
            }
            my $why = "could not answer a question during the dialog";
            $CPAN::Frontend->mywarn("Failing: $why\n");
            $self->{$phase} =
                CPAN::Distrostatus->new("NO $why");
            return 0;
        }
    }
}

sub _run_via_expect_deterministic {
    my($self,$expo,$phase,$expect_model) = @_;
    my $ran_into_timeout;
    my $ran_into_eof;
    my $timeout = $expect_model->{timeout} || 15; # currently unsettable
    my $expecta = $expect_model->{talk};
  EXPECT: for (my $i = 0; $i <= $#$expecta; $i+=2) {
        my($re,$send) = @$expecta[$i,$i+1];
        CPAN->debug("timeout[$timeout]re[$re]") if $CPAN::DEBUG;
        my $regex = eval "qr{$re}";
        $expo->expect($timeout,
                      [ eof => sub {
                            my $but = $expo->clear_accum;
                            $CPAN::Frontend->mywarn("EOF (maybe harmless)
expected[$regex]\nbut[$but]\n\n");
                            $ran_into_eof++;
                        } ],
                      [ timeout => sub {
                            my $but = $expo->clear_accum;
                            $CPAN::Frontend->mywarn("TIMEOUT
expected[$regex]\nbut[$but]\n\n");
                            $ran_into_timeout++;
                        } ],
                      -re => $regex);
        if ($ran_into_timeout) {
            # note that the caller expects 0 for success
            $self->{$phase} =
                CPAN::Distrostatus->new("NO timeout during expect dialog");
            return 0;
        } elsif ($ran_into_eof) {
            last EXPECT;
        }
        $expo->send($send);
    }
    $expo->soft_close;
    return $expo->exitstatus();
}

#-> CPAN::Distribution::_validate_distropref
sub _validate_distropref {
    my($self,@args) = @_;
    if (
        $CPAN::META->has_inst("CPAN::Kwalify")
        &&
        $CPAN::META->has_inst("Kwalify")
       ) {
        eval {CPAN::Kwalify::_validate("distroprefs",@args);};
        if ($@) {
            $CPAN::Frontend->mywarn($@);
        }
    } else {
        CPAN->debug("not validating '@args'") if $CPAN::DEBUG;
    }
}

#-> CPAN::Distribution::_find_prefs
sub _find_prefs {
    my($self) = @_;
    my $distroid = $self->pretty_id;
    #CPAN->debug("distroid[$distroid]") if $CPAN::DEBUG;
    my $prefs_dir = $CPAN::Config->{prefs_dir};
    return if $prefs_dir =~ /^\s*$/;
    eval { File::Path::mkpath($prefs_dir); };
    if ($@) {
        $CPAN::Frontend->mydie("Cannot create directory $prefs_dir");
    }
    # shortcut if there are no distroprefs files
    {
      my $dh = DirHandle->new($prefs_dir) or $CPAN::Frontend->mydie("Couldn't open '$prefs_dir': $!");
      my @files = map { /\.(yml|dd|st)\z/i } $dh->read;
      return unless @files;
    }
    my $yaml_module = CPAN::_yaml_module();
    my $ext_map = {};
    my @extensions;
    if ($CPAN::META->has_inst($yaml_module)) {
        $ext_map->{yml} = 'CPAN';
    } else {
        my @fallbacks;
        if ($CPAN::META->has_inst("Data::Dumper")) {
            push @fallbacks, $ext_map->{dd} = 'Data::Dumper';
        }
        if ($CPAN::META->has_inst("Storable")) {
            push @fallbacks, $ext_map->{st} = 'Storable';
        }
        if (@fallbacks) {
            local $" = " and ";
            unless ($self->{have_complained_about_missing_yaml}++) {
                $CPAN::Frontend->mywarnonce("'$yaml_module' not installed, falling back ".
                                            "to @fallbacks to read prefs '$prefs_dir'\n");
            }
        } else {
            unless ($self->{have_complained_about_missing_yaml}++) {
                $CPAN::Frontend->mywarnonce("'$yaml_module' not installed, cannot ".
                                            "read prefs '$prefs_dir'\n");
            }
        }
    }
    my $finder = CPAN::Distroprefs->find($prefs_dir, $ext_map);
    DIRENT: while (my $result = $finder->next) {
        if ($result->is_warning) {
            $CPAN::Frontend->mywarn($result->as_string);
            $CPAN::Frontend->mysleep(1);
            next DIRENT;
        } elsif ($result->is_fatal) {
            $CPAN::Frontend->mydie($result->as_string);
        }

        my @prefs = @{ $result->prefs };

      ELEMENT: for my $y (0..$#prefs) {
            my $pref = $prefs[$y];
            $self->_validate_distropref($pref->data, $result->abs, $y);

            # I don't know why we silently skip when there's no match, but
            # complain if there's an empty match hashref, and there's no
            # comment explaining why -- hdp, 2008-03-18
            unless ($pref->has_any_match) {
                next ELEMENT;
            }

            unless ($pref->has_valid_subkeys) {
                $CPAN::Frontend->mydie(sprintf
                    "Nonconforming .%s file '%s': " .
                    "missing match/* subattribute. " .
                    "Please remove, cannot continue.",
                    $result->ext, $result->abs,
                );
            }

            my $arg = {
                env          => \%ENV,
                distribution => $distroid,
                perl         => \&CPAN::find_perl,
                perlconfig   => \%Config::Config,
                module       => sub { [ $self->containsmods ] },
            };

            if ($pref->matches($arg)) {
                return {
                    prefs => $pref->data,
                    prefs_file => $result->abs,
                    prefs_file_doc => $y,
                };
            }

        }
    }
    return;
}

# CPAN::Distribution::prefs
sub prefs {
    my($self) = @_;
    if (exists $self->{negative_prefs_cache}
        &&
        $self->{negative_prefs_cache} != $CPAN::CurrentCommandId
       ) {
        delete $self->{negative_prefs_cache};
        delete $self->{prefs};
    }
    if (exists $self->{prefs}) {
        return $self->{prefs}; # XXX comment out during debugging
    }
    if ($CPAN::Config->{prefs_dir}) {
        CPAN->debug("prefs_dir[$CPAN::Config->{prefs_dir}]") if $CPAN::DEBUG;
        my $prefs = $self->_find_prefs();
        $prefs ||= ""; # avoid warning next line
        CPAN->debug("prefs[$prefs]") if $CPAN::DEBUG;
        if ($prefs) {
            for my $x (qw(prefs prefs_file prefs_file_doc)) {
                $self->{$x} = $prefs->{$x};
            }
            my $bs = sprintf(
                             "%s[%s]",
                             File::Basename::basename($self->{prefs_file}),
                             $self->{prefs_file_doc},
                            );
            my $filler1 = "_" x 22;
            my $filler2 = int(66 - length($bs))/2;
            $filler2 = 0 if $filler2 < 0;
            $filler2 = " " x $filler2;
            $CPAN::Frontend->myprint("
$filler1 D i s t r o P r e f s $filler1
$filler2 $bs $filler2
");
            $CPAN::Frontend->mysleep(1);
            return $self->{prefs};
        }
    }
    $self->{negative_prefs_cache} = $CPAN::CurrentCommandId;
    return $self->{prefs} = +{};
}

# CPAN::Distribution::_make_phase_arg
sub _make_phase_arg {
    my($self, $phase) = @_;
    my $_make_phase_arg;
    my $prefs = $self->prefs;
    if (
        $prefs
        && exists $prefs->{$phase}
        && exists $prefs->{$phase}{args}
        && $prefs->{$phase}{args}
       ) {
        $_make_phase_arg = join(" ",
                           map {CPAN::HandleConfig
                                 ->safe_quote($_)} @{$prefs->{$phase}{args}},
                          );
    }

# cpan[2]> o conf make[TAB]
# make                       make_install_make_command
# make_arg                   makepl_arg
# make_install_arg
# cpan[2]> o conf mbuild[TAB]
# mbuild_arg                    mbuild_install_build_command
# mbuild_install_arg            mbuildpl_arg

    my $mantra; # must switch make/mbuild here
    if ($self->{modulebuild}) {
        $mantra = "mbuild";
    } else {
        $mantra = "make";
    }
    my %map = (
               pl => "pl_arg",
               make => "_arg",
               test => "_test_arg", # does not really exist but maybe
                                    # will some day and now protects
                                    # us from unini warnings
               install => "_install_arg",
              );
    my $phase_underscore_meshup = $map{$phase};
    my $what = sprintf "%s%s", $mantra, $phase_underscore_meshup;

    $_make_phase_arg ||= $CPAN::Config->{$what};
    return $_make_phase_arg;
}

# CPAN::Distribution::_make_command
sub _make_command {
    my ($self) = @_;
    if ($self) {
        return
            CPAN::HandleConfig
                ->safe_quote(
                             CPAN::HandleConfig->prefs_lookup($self,
                                                              q{make})
                             || $Config::Config{make}
                             || 'make'
                            );
    } else {
        # Old style call, without object. Deprecated
        Carp::confess("CPAN::_make_command() used as function. Don't Do That.");
        return
          safe_quote(undef,
                     CPAN::HandleConfig->prefs_lookup($self,q{make})
                     || $CPAN::Config->{make}
                     || $Config::Config{make}
                     || 'make');
    }
}

sub _make_install_make_command {
    my ($self) = @_;
    my $mimc =
        CPAN::HandleConfig->prefs_lookup($self, q{make_install_make_command});
    return $self->_make_command() unless $mimc;

    # Quote the "make install" make command on Windows, where it is commonly
    # found in, e.g., C:\Program Files\... and therefore needs quoting. We can't
    # do this in general because the command maybe "sudo make..." (i.e. a
    # program with arguments), but that is unlikely to be the case on Windows.
    $mimc = CPAN::HandleConfig->safe_quote($mimc) if $^O eq 'MSWin32';

    return $mimc;
}

#-> sub CPAN::Distribution::is_locally_optional
sub is_locally_optional {
    my($self, $prereq_pm, $prereq) = @_;
    $prereq_pm ||= $self->{prereq_pm};
    my($nmo,$opt);
    for my $rt (qw(requires build_requires)) {
        if (exists $prereq_pm->{$rt}{$prereq}) {
            # rt 121914
            $nmo ||= $CPAN::META->instance("CPAN::Module",$prereq);
            my $av = $nmo->available_version;
            return 0 if !$av || CPAN::Version->vlt($av,$prereq_pm->{$rt}{$prereq});
        }
        if (exists $prereq_pm->{"opt_$rt"}{$prereq}) {
            $opt = 1;
        }
    }
    return $opt||0;
}

#-> sub CPAN::Distribution::follow_prereqs ;
sub follow_prereqs {
    my($self) = shift;
    my($slot) = shift;
    my(@prereq_tuples) = grep {$_->[0] ne "perl"} @_;
    return unless @prereq_tuples;
    my(@good_prereq_tuples);
    for my $p (@prereq_tuples) {
        # e.g. $p = ['Devel::PartialDump', 'r', 1]
        # promote if possible
        if ($p->[1] =~ /^(r|c)$/) {
            push @good_prereq_tuples, $p;
        } elsif ($p->[1] =~ /^(b)$/) {
            my $reqtype = CPAN::Queue->reqtype_of($p->[0]);
            if ($reqtype =~ /^(r|c)$/) {
                push @good_prereq_tuples, [$p->[0], $reqtype, $p->[2]];
            } else {
                push @good_prereq_tuples, $p;
            }
        } else {
            die "Panic: in follow_prereqs: reqtype[$p->[1]] seen, should never happen";
        }
    }
    my $pretty_id = $self->pretty_id;
    my %map = (
               b => "build_requires",
               r => "requires",
               c => "commandline",
              );
    my($filler1,$filler2,$filler3,$filler4);
    my $unsat = "Unsatisfied dependencies detected during";
    my $w = length($unsat) > length($pretty_id) ? length($unsat) : length($pretty_id);
    {
        my $r = int(($w - length($unsat))/2);
        my $l = $w - length($unsat) - $r;
        $filler1 = "-"x4 . " "x$l;
        $filler2 = " "x$r . "-"x4 . "\n";
    }
    {
        my $r = int(($w - length($pretty_id))/2);
        my $l = $w - length($pretty_id) - $r;
        $filler3 = "-"x4 . " "x$l;
        $filler4 = " "x$r . "-"x4 . "\n";
    }
    $CPAN::Frontend->
        myprint("$filler1 $unsat $filler2".
                "$filler3 $pretty_id $filler4".
                join("", map {sprintf "    %s \[%s%s]\n", $_->[0], $map{$_->[1]}, $self->is_locally_optional(undef,$_->[0]) ? ",optional" : ""} @good_prereq_tuples),
               );
    my $follow = 0;
    if ($CPAN::Config->{prerequisites_policy} eq "follow") {
        $follow = 1;
    } elsif ($CPAN::Config->{prerequisites_policy} eq "ask") {
        my $answer = CPAN::Shell::colorable_makemaker_prompt(
"Shall I follow them and prepend them to the queue
of modules we are processing right now?", "yes");
        $follow = $answer =~ /^\s*y/i;
    } else {
        my @prereq = map { $_->[0] } @good_prereq_tuples;
        local($") = ", ";
        $CPAN::Frontend->
            myprint("  Ignoring dependencies on modules @prereq\n");
    }
    if ($follow) {
        my $id = $self->id;
        my(@to_queue_mand,@to_queue_opt);
        for my $gp (@good_prereq_tuples) {
            my($prereq,$reqtype,$optional) = @$gp;
            my $qthing = +{qmod=>$prereq,reqtype=>$reqtype,optional=>$optional};
            if ($optional &&
                $self->is_locally_optional(undef,$prereq)
               ){
                # Since we do not depend on this one, we do not need
                # this in a mandatory arrangement:
                push @to_queue_opt, $qthing;
            } else {
                my $any = CPAN::Shell->expandany($prereq);
                $self->{$slot . "_for"}{$any->id}++;
                if ($any) {
                    unless ($optional) {
                        # No recursion check in an optional area of the tree
                        $any->color_cmd_tmps(0,2);
                    }
                } else {
                    $CPAN::Frontend->mywarn("Warning (maybe a bug): Cannot expand prereq '$prereq'\n");
                    $CPAN::Frontend->mysleep(2);
                }
                # order everything that is not locally_optional just
                # like mandatory items: this keeps leaves before
                # branches
                unshift @to_queue_mand, $qthing;
            }
        }
        if (@to_queue_mand) {
            unshift @to_queue_mand, {qmod => $id, reqtype => $self->{reqtype}, optional=> !$self->{mandatory}};
            CPAN::Queue->jumpqueue(@to_queue_opt,@to_queue_mand);
            $self->{$slot} = "Delayed until after prerequisites";
            return 1; # signal we need dependencies
        } elsif (@to_queue_opt) {
            CPAN::Queue->jumpqueue(@to_queue_opt);
        }
    }
    return;
}

sub _feature_depends {
    my($self) = @_;
    my $meta_yml = $self->parse_meta_yml();
    my $optf = $meta_yml->{optional_features} or return;
    if (!ref $optf or ref $optf ne "HASH"){
        $CPAN::Frontend->mywarn("The content of optional_features is not a HASH reference. Cannot use it.\n");
        $optf = {};
    }
    my $wantf = $self->prefs->{features} or return;
    if (!ref $wantf or ref $wantf ne "ARRAY"){
        $CPAN::Frontend->mywarn("The content of 'features' is not an ARRAY reference. Cannot use it.\n");
        $wantf = [];
    }
    my $dep = +{};
    for my $wf (@$wantf) {
        if (my $f = $optf->{$wf}) {
            $CPAN::Frontend->myprint("Found the demanded feature '$wf' that ".
                                     "is accompanied by this description:\n".
                                     $f->{description}.
                                     "\n\n"
                                    );
            # configure_requires currently not in the spec, unlikely to be useful anyway
            for my $reqtype (qw(configure_requires build_requires requires)) {
                my $reqhash = $f->{$reqtype} or next;
                while (my($k,$v) = each %$reqhash) {
                    $dep->{$reqtype}{$k} = $v;
                }
            }
        } else {
            $CPAN::Frontend->mywarn("The demanded feature '$wf' was not ".
                                    "found in the META.yml file".
                                    "\n\n"
                                   );
        }
    }
    $dep;
}

sub prereqs_for_slot {
    my($self,$slot) = @_;
    my($prereq_pm);
    unless ($CPAN::META->has_usable("CPAN::Meta::Requirements")) {
        my $whynot = "not available";
        if (defined $CPAN::Meta::Requirements::VERSION) {
            $whynot = "version $CPAN::Meta::Requirements::VERSION not sufficient";
        }
        $CPAN::Frontend->mywarn("CPAN::Meta::Requirements $whynot\n");
        my $before = "";
        if ($self->{CALLED_FOR}){
            if ($self->{CALLED_FOR} =~
                /^(
                     CPAN::Meta::Requirements
                 |CPAN::DistnameInfo
                 |version
                 |parent
                 |ExtUtils::MakeMaker
                 |Test::Harness
                 )$/x) {
                $CPAN::Frontend->mywarn("Please install CPAN::Meta::Requirements ".
                    "as soon as possible; it is needed for a reliable operation of ".
                    "the cpan shell; setting requirements to nil for '$1' for now ".
                    "to prevent deadlock during bootstrapping\n");
                return;
            }
            $before = " before $self->{CALLED_FOR}";
        }
        $CPAN::Frontend->mydie("Please install CPAN::Meta::Requirements manually$before");
    }
    my $merged = CPAN::Meta::Requirements->new;
    my $prefs_depends = $self->prefs->{depends}||{};
    my $feature_depends = $self->_feature_depends();
    if ($slot eq "configure_requires_later") {
        for my $hash (  $self->configure_requires,
                        $prefs_depends->{configure_requires},
                        $feature_depends->{configure_requires},
        ) {
            $merged->add_requirements(
                CPAN::Meta::Requirements->from_string_hash($hash)
            );
        }
        if (-f "Build.PL"
            && ! -f File::Spec->catfile($self->{build_dir},"Makefile.PL")
            && ! @{[ $merged->required_modules ]}
            && ! $CPAN::META->has_inst("Module::Build")
           ) {
            $CPAN::Frontend->mywarn(
              "  Warning: CPAN.pm discovered Module::Build as undeclared prerequisite.\n".
              "  Adding it now as such.\n"
            );
            $CPAN::Frontend->mysleep(5);
            $merged->add_minimum( "Module::Build" => 0 );
            delete $self->{writemakefile};
        }
        $prereq_pm = {}; # configure_requires defined as "b"
    } elsif ($slot eq "later") {
        my $prereq_pm_0 = $self->prereq_pm || {};
        for my $reqtype (qw(requires build_requires opt_requires opt_build_requires)) {
            $prereq_pm->{$reqtype} = {%{$prereq_pm_0->{$reqtype}||{}}}; # copy to not pollute it
            for my $dep ($prefs_depends,$feature_depends) {
                for my $k (keys %{$dep->{$reqtype}||{}}) {
                    $prereq_pm->{$reqtype}{$k} = $dep->{$reqtype}{$k};
                }
            }
        }
        # XXX what about optional_req|breq? -- xdg, 2012-04-01
        for my $hash (
            $prereq_pm->{requires},
            $prereq_pm->{build_requires},
            $prereq_pm->{opt_requires},
            $prereq_pm->{opt_build_requires},

        ) {
            $merged->add_requirements(
                CPAN::Meta::Requirements->from_string_hash($hash)
            );
        }
    } else {
        die "Panic: illegal slot '$slot'";
    }
    return ($merged->as_string_hash, $prereq_pm);
}

#-> sub CPAN::Distribution::unsat_prereq ;
# return ([Foo,"r"],[Bar,"b"]) for normal modules
# return ([perl=>5.008]) if we need a newer perl than we are running under
# (sorry for the inconsistency, it was an accident)
sub unsat_prereq {
    my($self,$slot) = @_;
    my($merged_hash,$prereq_pm) = $self->prereqs_for_slot($slot);
    my(@need);
    unless ($CPAN::META->has_usable("CPAN::Meta::Requirements")) {
        $CPAN::Frontend->mywarn("CPAN::Meta::Requirements not available, please install as soon as possible, trying to continue with severly limited capabilities\n");
        return;
    }
    my $merged = CPAN::Meta::Requirements->from_string_hash($merged_hash);
    my @merged = sort $merged->required_modules;
    CPAN->debug("all merged_prereqs[@merged]") if $CPAN::DEBUG;
  NEED: for my $need_module ( @merged ) {
        my $need_version = $merged->requirements_for_module($need_module);
        my($available_version,$inst_file,$available_file,$nmo);
        if ($need_module eq "perl") {
            $available_version = $];
            $available_file = CPAN::find_perl();
        } else {
            if (CPAN::_sqlite_running()) {
                CPAN::Index->reload;
                $CPAN::SQLite->search("CPAN::Module",$need_module);
            }
            $nmo = $CPAN::META->instance("CPAN::Module",$need_module);
            $inst_file = $nmo->inst_file || '';
            $available_file = $nmo->available_file || '';
            $available_version = $nmo->available_version;
            if ($nmo->uptodate) {
                my $accepts = eval {
                    $merged->accepts_module($need_module, $available_version);
                };
                unless ($accepts) {
                    my $rq = $merged->requirements_for_module( $need_module );
                    $CPAN::Frontend->mywarn(
                        "Warning: Version '$available_version' of ".
                        "'$need_module' is up to date but does not ".
                        "fulfill requirements ($rq). I will continue, ".
                        "but chances to succeed are low.\n");
                }
                next NEED;
            }

            # if they have not specified a version, we accept any
            # installed one; in that case inst_file is always
            # sufficient and available_file is sufficient on
            # both build_requires and configure_requires
            my $sufficient = $inst_file ||
                ( exists $prereq_pm->{requires}{$need_module} ? 0 : $available_file );
            if ( $sufficient
                and ( # a few quick short circuits
                     not defined $need_version
                     or $need_version eq '0'    # "==" would trigger warning when not numeric
                     or $need_version eq "undef"
                    )) {
                unless ($nmo->inst_deprecated) {
                    next NEED;
                }
            }
        }

        # We only want to install prereqs if either they're not installed
        # or if the installed version is too old. We cannot omit this
        # check, because if 'force' is in effect, nobody else will check.
        # But we don't want to accept a deprecated module installed as part
        # of the Perl core, so we continue if the available file is the installed
        # one and is deprecated

        if ( $available_file ) {
            my $fulfills_all_version_rqs = $self->_fulfills_all_version_rqs
                (
                 $need_module,
                 $available_file,
                 $available_version,
                 $need_version,
                );
            if ( $inst_file
                       && $available_file eq $inst_file
                       && $nmo->inst_deprecated
                     ) {
                # continue installing as a prereq. we really want that
                # because the deprecated module may spit out warnings
                # and third party did not know until today. Only one
                # exception is OK, because CPANPLUS is special after
                # all:
                if ( $fulfills_all_version_rqs and
                     $nmo->id =~ /^CPANPLUS(?:::Dist::Build)$/
                   ) {
                    # here we have an available version that is good
                    # enough although deprecated (preventing circular
                    # loop CPANPLUS => CPANPLUS::Dist::Build RT#83042)
                    next NEED;
                }
            } elsif (
                $self->{reqtype} # e.g. maybe we came via goto?
                && $self->{reqtype} =~ /^(r|c)$/
                && (   exists $prereq_pm->{requires}{$need_module}
                    || exists $prereq_pm->{opt_requires}{$need_module} )
                && $nmo
                && !$inst_file
            ) {
                # continue installing as a prereq; this may be a
                # distro we already used when it was a build_requires
                # so we did not install it. But suddenly somebody
                # wants it as a requires
                my $need_distro = $nmo->distribution;
                if ($need_distro->{install} && $need_distro->{install}->failed && $need_distro->{install}->text =~ /is only/) {
                    my $id = $need_distro->pretty_id;
                    $CPAN::Frontend->myprint("Promoting $id from build_requires to requires due $need_module\n");
                    delete $need_distro->{install}; # promote to another installation attempt
                    $need_distro->{reqtype} = "r";
                    $need_distro->install;
                    next NEED;
                }
            }
            else {
                next NEED if $fulfills_all_version_rqs;
            }
        }

        if ($need_module eq "perl") {
            return ["perl", $need_version];
        }
        $self->{sponsored_mods}{$need_module} ||= 0;
        CPAN->debug("need_module[$need_module]s/s/n[$self->{sponsored_mods}{$need_module}]") if $CPAN::DEBUG;
        if (my $sponsoring = $self->{sponsored_mods}{$need_module}++) {
            # We have already sponsored it and for some reason it's still
            # not available. So we do ... what??

            # if we push it again, we have a potential infinite loop

            # The following "next" was a very problematic construct.
            # It helped a lot but broke some day and had to be
            # replaced.

            # We must be able to deal with modules that come again and
            # again as a prereq and have themselves prereqs and the
            # queue becomes long but finally we would find the correct
            # order. The RecursiveDependency check should trigger a
            # die when it's becoming too weird. Unfortunately removing
            # this next breaks many other things.

            # The bug that brought this up is described in Todo under
            # "5.8.9 cannot install Compress::Zlib"

            # next; # this is the next that had to go away

            # The following "next NEED" are fine and the error message
            # explains well what is going on. For example when the DBI
            # fails and consequently DBD::SQLite fails and now we are
            # processing CPAN::SQLite. Then we must have a "next" for
            # DBD::SQLite. How can we get it and how can we identify
            # all other cases we must identify?

            my $do = $nmo->distribution;
            next NEED unless $do; # not on CPAN
            if (CPAN::Version->vcmp($need_version, $nmo->ro->{CPAN_VERSION}) > 0){
                $CPAN::Frontend->mywarn("Warning: Prerequisite ".
                                        "'$need_module => $need_version' ".
                                        "for '$self->{ID}' seems ".
                                        "not available according to the indices\n"
                                       );
                next NEED;
            }
          NOSAYER: for my $nosayer (
                                    "unwrapped",
                                    "writemakefile",
                                    "signature_verify",
                                    "make",
                                    "make_test",
                                    "install",
                                    "make_clean",
                                   ) {
                if ($do->{$nosayer}) {
                    my $selfid = $self->pretty_id;
                    my $did = $do->pretty_id;
                    if (UNIVERSAL::can($do->{$nosayer},"failed") ?
                        $do->{$nosayer}->failed :
                        $do->{$nosayer} =~ /^NO/) {
                        if ($nosayer eq "make_test"
                            &&
                            $do->{make_test}{COMMANDID} != $CPAN::CurrentCommandId
                           ) {
                            next NOSAYER;
                        }
                        ### XXX  don't complain about missing optional deps -- xdg, 2012-04-01
                        if ($self->is_locally_optional($prereq_pm, $need_module)) {
                            # don't complain about failing optional prereqs
                        }
                        else {
                            $CPAN::Frontend->mywarn("Warning: Prerequisite ".
                                                    "'$need_module => $need_version' ".
                                                    "for '$selfid' failed when ".
                                                    "processing '$did' with ".
                                                    "'$nosayer => $do->{$nosayer}'. Continuing, ".
                                                    "but chances to succeed are limited.\n"
                                                );
                            $CPAN::Frontend->mysleep($sponsoring/10);
                        }
                        next NEED;
                    } else { # the other guy succeeded
                        if ($nosayer =~ /^(install|make_test)$/) {
                            # we had this with
                            # DMAKI/DateTime-Calendar-Chinese-0.05.tar.gz
                            # in 2007-03 for 'make install'
                            # and 2008-04: #30464 (for 'make test')
                            # $CPAN::Frontend->mywarn("Warning: Prerequisite ".
                            #                         "'$need_module => $need_version' ".
                            #                         "for '$selfid' already built ".
                            #                         "but the result looks suspicious. ".
                            #                         "Skipping another build attempt, ".
                            #                         "to prevent looping endlessly.\n"
                            #                        );
                            next NEED;
                        }
                    }
                }
            }
        }
        my $needed_as;
        if (0) {
        } elsif (exists $prereq_pm->{requires}{$need_module}
            || exists $prereq_pm->{opt_requires}{$need_module}
        ) {
            $needed_as = "r";
        } elsif ($slot eq "configure_requires_later") {
            # in ae872487d5 we said: C< we have not yet run the
            # {Build,Makefile}.PL, we must presume "r" >; but the
            # meta.yml standard says C< These dependencies are not
            # required after the distribution is installed. >; so now
            # we change it back to "b" and care for the proper
            # promotion later.
            $needed_as = "b";
        } else {
            $needed_as = "b";
        }
        # here need to flag as optional for recommends/suggests
        # -- xdg, 2012-04-01
        $self->debug(sprintf "%s manadory?[%s]",
                     $self->pretty_id,
                     $self->{mandatory})
            if $CPAN::DEBUG;
        my $optional = !$self->{mandatory}
            || $self->is_locally_optional($prereq_pm, $need_module);
        push @need, [$need_module,$needed_as,$optional];
    }
    my @unfolded = map { "[".join(",",@$_)."]" } @need;
    CPAN->debug("returning from unsat_prereq[@unfolded]") if $CPAN::DEBUG;
    @need;
}

sub _fulfills_all_version_rqs {
    my($self,$need_module,$available_file,$available_version,$need_version) = @_;
    my(@all_requirements) = split /\s*,\s*/, $need_version;
    local($^W) = 0;
    my $ok = 0;
  RQ: for my $rq (@all_requirements) {
        if ($rq =~ s|>=\s*||) {
        } elsif ($rq =~ s|>\s*||) {
            # 2005-12: one user
            if (CPAN::Version->vgt($available_version,$rq)) {
                $ok++;
            }
            next RQ;
        } elsif ($rq =~ s|!=\s*||) {
            # 2005-12: no user
            if (CPAN::Version->vcmp($available_version,$rq)) {
                $ok++;
                next RQ;
            } else {
                $ok=0;
                last RQ;
            }
        } elsif ($rq =~ m|<=?\s*|) {
            # 2005-12: no user
            $CPAN::Frontend->mywarn("Downgrading not supported (rq[$rq])\n");
            $ok++;
            next RQ;
        } elsif ($rq =~ s|==\s*||) {
            # 2009-07: ELLIOTJS/Perl-Critic-1.099_002.tar.gz
            if (CPAN::Version->vcmp($available_version,$rq)) {
                $ok=0;
                last RQ;
            } else {
                $ok++;
                next RQ;
            }
        }
        if (! CPAN::Version->vgt($rq, $available_version)) {
            $ok++;
        }
        CPAN->debug(sprintf("need_module[%s]available_file[%s]".
                            "available_version[%s]rq[%s]ok[%d]",
                            $need_module,
                            $available_file,
                            $available_version,
                            CPAN::Version->readable($rq),
                            $ok,
                           )) if $CPAN::DEBUG;
    }
    my $ret = $ok == @all_requirements;
    CPAN->debug(sprintf("need_module[%s]ok[%s]all_requirements[%d]",$need_module, $ok, scalar @all_requirements)) if $CPAN::DEBUG;
    return $ret;
}

#-> sub CPAN::Distribution::read_meta
# read any sort of meta files, return CPAN::Meta object if no errors
sub read_meta {
    my($self) = @_;
    my $meta_file = $self->pick_meta_file
        or return;

    return unless $CPAN::META->has_usable("CPAN::Meta");
    my $meta = eval { CPAN::Meta->load_file($meta_file)}
        or return;

    # Very old EU::MM could have wrong META
    if ($meta_file eq 'META.yml'
        && $meta->generated_by =~ /ExtUtils::MakeMaker version ([\d\._]+)/
    ) {
        my $eummv = do { local $^W = 0; $1+0; };
        return if $eummv < 6.2501;
    }

    return $meta;
}

#-> sub CPAN::Distribution::read_yaml ;
# XXX This should be DEPRECATED -- dagolden, 2011-02-05
sub read_yaml {
    my($self) = @_;
    my $meta_file = $self->pick_meta_file('\.yml$');
    $self->debug("meta_file[$meta_file]") if $CPAN::DEBUG;
    return unless $meta_file;
    my $yaml;
    eval { $yaml = $self->parse_meta_yml($meta_file) };
    if ($@ or ! $yaml) {
        return undef; # if we die, then we cannot read YAML's own META.yml
    }
    # not "authoritative"
    if (defined $yaml && (! ref $yaml || ref $yaml ne "HASH")) {
        $CPAN::Frontend->mywarn("META.yml does not seem to be conforming, cannot use it.\n");
        $yaml = undef;
    }
    $self->debug(sprintf "yaml[%s]", $yaml || "UNDEF")
        if $CPAN::DEBUG;
    $self->debug($yaml) if $CPAN::DEBUG && $yaml;
    # MYMETA.yml is static and authoritative by definition
    if ( $meta_file =~ /MYMETA\.yml/ ) {
      return $yaml;
    }
    # META.yml is authoritative only if dynamic_config is defined and false
    if ( defined $yaml->{dynamic_config} && ! $yaml->{dynamic_config} ) {
      return $yaml;
    }
    # otherwise, we can't use what we found
    return undef;
}

#-> sub CPAN::Distribution::configure_requires ;
sub configure_requires {
    my($self) = @_;
    return unless my $meta_file = $self->pick_meta_file('^META');
    if (my $meta_obj = $self->read_meta) {
        my $prereqs = $meta_obj->effective_prereqs;
        my $cr = $prereqs->requirements_for(qw/configure requires/);
        return $cr ? $cr->as_string_hash : undef;
    }
    else {
        my $yaml = eval { $self->parse_meta_yml($meta_file) };
        return $yaml->{configure_requires};
    }
}

#-> sub CPAN::Distribution::prereq_pm ;
sub prereq_pm {
    my($self) = @_;
    return unless $self->{writemakefile}  # no need to have succeeded
                                          # but we must have run it
        || $self->{modulebuild};
    unless ($self->{build_dir}) {
        return;
    }
    # no Makefile/Build means configuration aborted, so don't look for prereqs
    my $makefile  = File::Spec->catfile($self->{build_dir}, $^O eq 'VMS' ? 'descrip.mms' : 'Makefile');
    my $buildfile = File::Spec->catfile($self->{build_dir}, $^O eq 'VMS' ? 'Build.com' : 'Build');
    return unless   -f $makefile || -f $buildfile;
    CPAN->debug(sprintf "writemakefile[%s]modulebuild[%s]",
                $self->{writemakefile}||"",
                $self->{modulebuild}||"",
               ) if $CPAN::DEBUG;
    my($req,$breq, $opt_req, $opt_breq);
    my $meta_obj = $self->read_meta;
    # META/MYMETA is only authoritative if dynamic_config is false
    if ($meta_obj && ! $meta_obj->dynamic_config) {
        my $prereqs = $meta_obj->effective_prereqs;
        my $requires = $prereqs->requirements_for(qw/runtime requires/);
        my $build_requires = $prereqs->requirements_for(qw/build requires/);
        my $test_requires = $prereqs->requirements_for(qw/test requires/);
        # XXX we don't yet distinguish build vs test, so merge them for now
        $build_requires->add_requirements($test_requires);
        $req = $requires->as_string_hash;
        $breq = $build_requires->as_string_hash;

        # XXX assemble optional_req && optional_breq from recommends/suggests
        # depending on corresponding policies -- xdg, 2012-04-01
        CPAN->use_inst("CPAN::Meta::Requirements");
        my $opt_runtime = CPAN::Meta::Requirements->new;
        my $opt_build   = CPAN::Meta::Requirements->new;
        if ( $CPAN::Config->{recommends_policy} ) {
            $opt_runtime->add_requirements( $prereqs->requirements_for(qw/runtime recommends/));
            $opt_build->add_requirements(   $prereqs->requirements_for(qw/build recommends/));
            $opt_build->add_requirements(   $prereqs->requirements_for(qw/test  recommends/));

        }
        if ( $CPAN::Config->{suggests_policy} ) {
            $opt_runtime->add_requirements( $prereqs->requirements_for(qw/runtime suggests/));
            $opt_build->add_requirements(   $prereqs->requirements_for(qw/build suggests/));
            $opt_build->add_requirements(   $prereqs->requirements_for(qw/test  suggests/));
        }
        $opt_req = $opt_runtime->as_string_hash;
        $opt_breq = $opt_build->as_string_hash;
    }
    elsif (my $yaml = $self->read_yaml) { # often dynamic_config prevents a result here
        $req =  $yaml->{requires} || {};
        $breq =  $yaml->{build_requires} || {};
        if ( $CPAN::Config->{recommends_policy} ) {
            $opt_req = $yaml->{recommends} || {};
        }
        undef $req unless ref $req eq "HASH" && %$req;
        if ($req) {
            if ($yaml->{generated_by} &&
                $yaml->{generated_by} =~ /ExtUtils::MakeMaker version ([\d\._]+)/) {
                my $eummv = do { local $^W = 0; $1+0; };
                if ($eummv < 6.2501) {
                    # thanks to Slaven for digging that out: MM before
                    # that could be wrong because it could reflect a
                    # previous release
                    undef $req;
                }
            }
            my $areq;
            my $do_replace;
            foreach my $k (sort keys %{$req||{}}) {
                my $v = $req->{$k};
                next unless defined $v;
                if ($v =~ /\d/) {
                    $areq->{$k} = $v;
                } elsif ($k =~ /[A-Za-z]/ &&
                         $v =~ /[A-Za-z]/ &&
                         $CPAN::META->exists("CPAN::Module",$v)
                        ) {
                    $CPAN::Frontend->mywarn("Suspicious key-value pair in META.yml's ".
                                            "requires hash: $k => $v; I'll take both ".
                                            "key and value as a module name\n");
                    $CPAN::Frontend->mysleep(1);
                    $areq->{$k} = 0;
                    $areq->{$v} = 0;
                    $do_replace++;
                }
            }
            $req = $areq if $do_replace;
        }
    }
    else {
        $CPAN::Frontend->mywarnonce("Could not read metadata file. Falling back to other ".
                                    "methods to determine prerequisites\n");
    }

    unless ($req || $breq) {
        my $build_dir;
        unless ( $build_dir = $self->{build_dir} ) {
            return;
        }
        my $makefile = File::Spec->catfile($build_dir,"Makefile");
        my $fh;
        if (-f $makefile
            and
            $fh = FileHandle->new("<$makefile\0")) {
            CPAN->debug("Getting prereq from Makefile") if $CPAN::DEBUG;
            local($/) = "\n";
            while (<$fh>) {
                last if /MakeMaker post_initialize section/;
                my($p) = m{^[\#]
                           \s+PREREQ_PM\s+=>\s+(.+)
                       }x;
                next unless $p;
                # warn "Found prereq expr[$p]";

                #  Regexp modified by A.Speer to remember actual version of file
                #  PREREQ_PM hash key wants, then add to
                while ( $p =~ m/(?:\s)([\w\:]+)=>(q\[.*?\]|undef),?/g ) {
                    my($m,$n) = ($1,$2);
                    # When a prereq is mentioned twice: let the bigger
                    # win; usual culprit is that they declared
                    # build_requires separately from requires; see
                    # rt.cpan.org #47774
                    my($prevn);
                    if ( defined $req->{$m} ) {
                        $prevn = $req->{$m};
                    }
                    if ($n =~ /^q\[(.*?)\]$/) {
                        $n = $1;
                    }
                    if (!$prevn || CPAN::Version->vlt($prevn, $n)){
                        $req->{$m} = $n;
                    }
                }
                last;
            }
        }
    }
    unless ($req || $breq) {
        my $build_dir = $self->{build_dir} or die "Panic: no build_dir?";
        my $buildfile = File::Spec->catfile($build_dir,"Build");
        if (-f $buildfile) {
            CPAN->debug("Found '$buildfile'") if $CPAN::DEBUG;
            my $build_prereqs = File::Spec->catfile($build_dir,"_build","prereqs");
            if (-f $build_prereqs) {
                CPAN->debug("Getting prerequisites from '$build_prereqs'") if $CPAN::DEBUG;
                my $content = do { local *FH;
                                   open FH, $build_prereqs
                                       or $CPAN::Frontend->mydie("Could not open ".
                                                                 "'$build_prereqs': $!");
                                   local $/;
                                   <FH>;
                               };
                my $bphash = eval $content;
                if ($@) {
                } else {
                    $req  = $bphash->{requires} || +{};
                    $breq = $bphash->{build_requires} || +{};
                }
            }
        }
    }
    # XXX needs to be adapted for optional_req & optional_breq -- xdg, 2012-04-01
    if ($req || $breq || $opt_req || $opt_breq ) {
        return $self->{prereq_pm} = {
           requires => $req,
           build_requires => $breq,
           opt_requires => $opt_req,
           opt_build_requires => $opt_breq,
       };
    }
}

#-> sub CPAN::Distribution::shortcut_test ;
# return values: undef means don't shortcut; 0 means shortcut as fail;
# and 1 means shortcut as success
sub shortcut_test {
    my ($self) = @_;

    $self->debug("checking badtestcnt[$self->{ID}]") if $CPAN::DEBUG;
    $self->{badtestcnt} ||= 0;
    if ($self->{badtestcnt} > 0) {
        require Data::Dumper;
        CPAN->debug(sprintf "NOREPEAT[%s]", Data::Dumper::Dumper($self)) if $CPAN::DEBUG;
        return $self->goodbye("Won't repeat unsuccessful test during this command");
    }

    for my $slot ( qw/later configure_requires_later/ ) {
        $self->debug("checking $slot slot[$self->{ID}]") if $CPAN::DEBUG;
        return $self->success($self->{$slot})
        if $self->{$slot};
    }

    $self->debug("checking if tests passed[$self->{ID}]") if $CPAN::DEBUG;
    if ( $self->{make_test} ) {
        if (
            UNIVERSAL::can($self->{make_test},"failed") ?
            $self->{make_test}->failed :
            $self->{make_test} =~ /^NO/
        ) {
            if (
                UNIVERSAL::can($self->{make_test},"commandid")
                &&
                $self->{make_test}->commandid == $CPAN::CurrentCommandId
            ) {
                return $self->goodbye("Has already been tested within this command");
            }
        } else {
            # if global "is_tested" has been cleared, we need to mark this to
            # be added to PERL5LIB if not already installed
            if ($self->tested_ok_but_not_installed) {
                $CPAN::META->is_tested($self->{build_dir},$self->{make_test}{TIME});
            }
            return $self->success("Has already been tested successfully");
        }
    }

    if ($self->{notest}) {
        $self->{make_test} = CPAN::Distrostatus->new("YES");
        return $self->success("Skipping test because of notest pragma");
    }

    return undef; # no shortcut
}

#-> sub CPAN::Distribution::_exe_files ;
sub _exe_files {
    my($self) = @_;
    return unless $self->{writemakefile}  # no need to have succeeded
                                          # but we must have run it
        || $self->{modulebuild};
    unless ($self->{build_dir}) {
        return;
    }
    CPAN->debug(sprintf "writemakefile[%s]modulebuild[%s]",
                $self->{writemakefile}||"",
                $self->{modulebuild}||"",
               ) if $CPAN::DEBUG;
    my $build_dir;
    unless ( $build_dir = $self->{build_dir} ) {
        return;
    }
    my $makefile = File::Spec->catfile($build_dir,"Makefile");
    my $fh;
    my @exe_files;
    if (-f $makefile
        and
        $fh = FileHandle->new("<$makefile\0")) {
        CPAN->debug("Getting exefiles from Makefile") if $CPAN::DEBUG;
        local($/) = "\n";
        while (<$fh>) {
            last if /MakeMaker post_initialize section/;
            my($p) = m{^[\#]
                       \s+EXE_FILES\s+=>\s+\[(.+)\]
                  }x;
            next unless $p;
            # warn "Found exefiles expr[$p]";
            my @p = split /,\s*/, $p;
            for my $p2 (@p) {
                if ($p2 =~ /^q\[(.+)\]/) {
                    push @exe_files, $1;
                }
            }
        }
    }
    return \@exe_files if @exe_files;
    my $buildparams = File::Spec->catfile($build_dir,"_build","build_params");
    if (-f $buildparams) {
        CPAN->debug("Found '$buildparams'") if $CPAN::DEBUG;
        my $x = do $buildparams;
        for my $sf ($x->[2]{script_files}) {
            if (my $reftype = ref $sf) {
                if ($reftype eq "ARRAY") {
                    push @exe_files, @$sf;
                }
                elsif ($reftype eq "HASH") {
                    push @exe_files, keys %$sf;
                }
                else {
                    $CPAN::Frontend->mywarn("Invalid reftype $reftype for Build.PL 'script_files'\n");
                }
            }
            elsif (defined $sf) {
                push @exe_files, $sf;
            }
        }
    }
    return \@exe_files;
}

#-> sub CPAN::Distribution::test ;
sub test {
    my($self) = @_;

    $self->pre_test();

    if (exists $self->{cleanup_after_install_done}) {
        $self->post_test();
        return $self->make;
    }

    $self->debug("checking goto id[$self->{ID}]") if $CPAN::DEBUG;
    if (my $goto = $self->prefs->{goto}) {
        $self->post_test();
        return $self->goto($goto);
    }

    unless ($self->make){
        $self->post_test();
        return;
    }

    if ( defined( my $sc = $self->shortcut_test ) ) {
        $self->post_test();
        return $sc;
    }

    if ($CPAN::Signal) {
        delete $self->{force_update};
        $self->post_test();
        return;
    }
    # warn "XDEBUG: checking for notest: $self->{notest} $self";
    my $make = $self->{modulebuild} ? "Build" : "make";

    local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
                           ? $ENV{PERL5LIB}
                           : ($ENV{PERLLIB} || "");

    local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
    local $ENV{PERL_USE_UNSAFE_INC} =
        exists $ENV{PERL_USE_UNSAFE_INC} && defined $ENV{PERL_USE_UNSAFE_INC}
        ? $ENV{PERL_USE_UNSAFE_INC} : 1; # test
    $CPAN::META->set_perl5lib;
    local $ENV{MAKEFLAGS}; # protect us from outer make calls
    local $ENV{PERL_MM_USE_DEFAULT} = 1 if $CPAN::Config->{use_prompt_default};
    local $ENV{NONINTERACTIVE_TESTING} = 1 if $CPAN::Config->{use_prompt_default};

    if ($run_allow_installing_within_test) {
        my($allow_installing, $why) = $self->_allow_installing;
        if (! $allow_installing) {
            $CPAN::Frontend->mywarn("Testing/Installation stopped: $why\n");
            $self->introduce_myself;
            $self->{make_test} = CPAN::Distrostatus->new("NO -- testing/installation stopped due $why");
            $CPAN::Frontend->mywarn("  [testing] -- NOT OK\n");
            delete $self->{force_update};
            $self->post_test();
            return;
        }
    }
    $CPAN::Frontend->myprint(sprintf "Running %s test for %s\n", $make, $self->pretty_id);

    my $builddir = $self->dir or
        $CPAN::Frontend->mydie("PANIC: Cannot determine build directory\n");

    unless (chdir $builddir) {
        $CPAN::Frontend->mywarn("Couldn't chdir to '$builddir': $!");
        $self->post_test();
        return;
    }

    $self->debug("Changed directory to $self->{build_dir}")
        if $CPAN::DEBUG;

    if ($^O eq 'MacOS') {
        Mac::BuildTools::make_test($self);
        $self->post_test();
        return;
    }

    if ($self->{modulebuild}) {
        my $thm = CPAN::Shell->expand("Module","Test::Harness");
        my $v = $thm->inst_version;
        if (CPAN::Version->vlt($v,2.62)) {
            # XXX Eric Wilhelm reported this as a bug: klapperl:
            # Test::Harness 3.0 self-tests, so that should be 'unless
            # installing Test::Harness'
            unless ($self->id eq $thm->distribution->id) {
                $CPAN::Frontend->mywarn(qq{The version of your Test::Harness is only
  '$v', you need at least '2.62'. Please upgrade your Test::Harness.\n});
                $self->{make_test} = CPAN::Distrostatus->new("NO Test::Harness too old");
                $self->post_test();
                return;
            }
        }
    }

    if ( ! $self->{force_update}  ) {
        # bypass actual tests if "trust_test_report_history" and have a report
        my $have_tested_fcn;
        if (   $CPAN::Config->{trust_test_report_history}
            && $CPAN::META->has_inst("CPAN::Reporter::History")
            && ( $have_tested_fcn = CPAN::Reporter::History->can("have_tested" ))) {
            if ( my @reports = $have_tested_fcn->( dist => $self->base_id ) ) {
                # Do nothing if grade was DISCARD
                if ( $reports[-1]->{grade} =~ /^(?:PASS|UNKNOWN)$/ ) {
                    $self->{make_test} = CPAN::Distrostatus->new("YES");
                    # if global "is_tested" has been cleared, we need to mark this to
                    # be added to PERL5LIB if not already installed
                    if ($self->tested_ok_but_not_installed) {
                        $CPAN::META->is_tested($self->{build_dir},$self->{make_test}{TIME});
                    }
                    $CPAN::Frontend->myprint("Found prior test report -- OK\n");
                    $self->post_test();
                    return;
                }
                elsif ( $reports[-1]->{grade} =~ /^(?:FAIL|NA)$/ ) {
                    $self->{make_test} = CPAN::Distrostatus->new("NO");
                    $self->{badtestcnt}++;
                    $CPAN::Frontend->mywarn("Found prior test report -- NOT OK\n");
                    $self->post_test();
                    return;
                }
            }
        }
    }

    my $system;
    my $prefs_test = $self->prefs->{test};
    if (my $commandline
        = exists $prefs_test->{commandline} ? $prefs_test->{commandline} : "") {
        $system = $commandline;
        $ENV{PERL} = CPAN::find_perl();
    } elsif ($self->{modulebuild}) {
        $system = sprintf "%s test", $self->_build_command();
        unless (-e "Build" || ($^O eq 'VMS' && -e "Build.com")) {
            my $id = $self->pretty_id;
            $CPAN::Frontend->mywarn("Alert: no 'Build' file found while trying to test '$id'");
        }
    } else {
        $system = join " ", $self->_make_command(), "test";
    }
    my $make_test_arg = $self->_make_phase_arg("test");
    $system = sprintf("%s%s",
                      $system,
                      $make_test_arg ? " $make_test_arg" : "",
                     );
    my($tests_ok);
    my $test_env;
    if ($self->prefs->{test}) {
        $test_env = $self->prefs->{test}{env};
    }
    local @ENV{keys %$test_env} = values %$test_env if $test_env;
    my $expect_model = $self->_prefs_with_expect("test");
    my $want_expect = 0;
    if ( $expect_model && @{$expect_model->{talk}} ) {
        my $can_expect = $CPAN::META->has_inst("Expect");
        if ($can_expect) {
            $want_expect = 1;
        } else {
            $CPAN::Frontend->mywarn("Expect not installed, falling back to ".
                                    "testing without\n");
        }
    }

 FORK: {
        my $pid = fork;
        if (! defined $pid) { # contention
            warn "Contention '$!', sleeping 2";
            sleep 2;
            redo FORK;
        } elsif ($pid) { # parent
            if ($^O eq "MSWin32") {
                wait;
            } else {
            SUPERVISE: while (waitpid($pid, WNOHANG) <= 0) {
                    if ($CPAN::Signal) {
                        kill 9, -$pid;
                    }
                    sleep 1;
                }
            }
            $tests_ok = !$?;
        } else { # child
            POSIX::setsid() unless $^O eq "MSWin32";
            my $c_ok;
            $|=1;
            if ($want_expect) {
                if ($self->_should_report('test')) {
                    $CPAN::Frontend->mywarn("Reporting via CPAN::Reporter is currently ".
                        "not supported when distroprefs specify ".
                        "an interactive test\n");
                }
                $c_ok = $self->_run_via_expect($system,'test',$expect_model) == 0;
            } elsif ( $self->_should_report('test') ) {
                $c_ok = CPAN::Reporter::test($self, $system);
            } else {
                $c_ok = system($system) == 0;
            }
            exit !$c_ok;
        }
    } # FORK

    $self->introduce_myself;
    my $but = $self->_make_test_illuminate_prereqs();
    if ( $tests_ok ) {
        if ($but) {
            $CPAN::Frontend->mywarn("Tests succeeded but $but\n");
            $self->{make_test} = CPAN::Distrostatus->new("NO $but");
            $self->store_persistent_state;
            $self->post_test();
            return $self->goodbye("[dependencies] -- NA");
        }
        $CPAN::Frontend->myprint("  $system -- OK\n");
        $self->{make_test} = CPAN::Distrostatus->new("YES");
        $CPAN::META->is_tested($self->{build_dir},$self->{make_test}{TIME});
        # probably impossible to need the next line because badtestcnt
        # has a lifespan of one command
        delete $self->{badtestcnt};
    } else {
        if ($but) {
            $but .= "; additionally test harness failed";
            $CPAN::Frontend->mywarn("$but\n");
            $self->{make_test} = CPAN::Distrostatus->new("NO $but");
        } elsif ( $self->{force_update} ) {
            $self->{make_test} = CPAN::Distrostatus->new(
                "NO but failure ignored because 'force' in effect"
            );
        } elsif ($CPAN::Signal) {
            $self->{make_test} = CPAN::Distrostatus->new("NO -- Interrupted");
        } else {
            $self->{make_test} = CPAN::Distrostatus->new("NO");
        }
        $self->{badtestcnt}++;
        $CPAN::Frontend->mywarn("  $system -- NOT OK\n");
        CPAN::Shell->optprint
              ("hint",
               sprintf
               ("//hint// to see the cpan-testers results for installing this module, try:
  reports %s\n",
                $self->pretty_id));
    }
    $self->store_persistent_state;

    $self->post_test();

    return $self->{force_update} ? 1 : !! $tests_ok;
}

sub _make_test_illuminate_prereqs {
    my($self) = @_;
    my @prereq;

    # local $CPAN::DEBUG = 16; # Distribution
    for my $m (sort keys %{$self->{sponsored_mods}}) {
        next unless $self->{sponsored_mods}{$m} > 0;
        my $m_obj = CPAN::Shell->expand("Module",$m) or next;
        # XXX we need available_version which reflects
        # $ENV{PERL5LIB} so that already tested but not yet
        # installed modules are counted.
        my $available_version = $m_obj->available_version;
        my $available_file = $m_obj->available_file;
        if ($available_version &&
            !CPAN::Version->vlt($available_version,$self->{prereq_pm}{$m})
           ) {
            CPAN->debug("m[$m] good enough available_version[$available_version]")
                if $CPAN::DEBUG;
        } elsif ($available_file
                 && (
                     !$self->{prereq_pm}{$m}
                     ||
                     $self->{prereq_pm}{$m} == 0
                    )
                ) {
            # lex Class::Accessor::Chained::Fast which has no $VERSION
            CPAN->debug("m[$m] have available_file[$available_file]")
                if $CPAN::DEBUG;
        } else {
            push @prereq, $m
                unless $self->is_locally_optional(undef, $m);
        }
    }
    my $but;
    if (@prereq) {
        my $cnt = @prereq;
        my $which = join ",", @prereq;
        $but = $cnt == 1 ? "one dependency not OK ($which)" :
            "$cnt dependencies missing ($which)";
    }
    $but;
}

sub _prefs_with_expect {
    my($self,$where) = @_;
    return unless my $prefs = $self->prefs;
    return unless my $where_prefs = $prefs->{$where};
    if ($where_prefs->{expect}) {
        return {
                mode => "deterministic",
                timeout => 15,
                talk => $where_prefs->{expect},
               };
    } elsif ($where_prefs->{"eexpect"}) {
        return $where_prefs->{"eexpect"};
    }
    return;
}

#-> sub CPAN::Distribution::clean ;
sub clean {
    my($self) = @_;
    my $make = $self->{modulebuild} ? "Build" : "make";
    $CPAN::Frontend->myprint(sprintf "Running %s clean for %s\n", $make, $self->pretty_id);
    unless (exists $self->{archived}) {
        $CPAN::Frontend->mywarn("Distribution seems to have never been unzipped".
                                "/untarred, nothing done\n");
        return 1;
    }
    unless (exists $self->{build_dir}) {
        $CPAN::Frontend->mywarn("Distribution has no own directory, nothing to do.\n");
        return 1;
    }
    if (exists $self->{writemakefile}
        and $self->{writemakefile}->failed
       ) {
        $CPAN::Frontend->mywarn("No Makefile, don't know how to 'make clean'\n");
        return 1;
    }
  EXCUSE: {
        my @e;
        exists $self->{make_clean} and $self->{make_clean} eq "YES" and
            push @e, "make clean already called once";
        $CPAN::Frontend->myprint(join "", map {"  $_\n"} @e) and return if @e;
    }
    chdir "$self->{build_dir}" or
        Carp::confess("Couldn't chdir to $self->{build_dir}: $!");
    $self->debug("Changed directory to $self->{build_dir}") if $CPAN::DEBUG;

    if ($^O eq 'MacOS') {
        Mac::BuildTools::make_clean($self);
        return;
    }

    my $system;
    if ($self->{modulebuild}) {
        unless (-f "Build") {
            my $cwd = CPAN::anycwd();
            $CPAN::Frontend->mywarn("Alert: no Build file available for 'clean $self->{id}".
                                    " in cwd[$cwd]. Danger, Will Robinson!");
            $CPAN::Frontend->mysleep(5);
        }
        $system = sprintf "%s clean", $self->_build_command();
    } else {
        $system  = join " ", $self->_make_command(), "clean";
    }
    my $system_ok = system($system) == 0;
    $self->introduce_myself;
    if ( $system_ok ) {
      $CPAN::Frontend->myprint("  $system -- OK\n");

      # $self->force;

      # Jost Krieger pointed out that this "force" was wrong because
      # it has the effect that the next "install" on this distribution
      # will untar everything again. Instead we should bring the
      # object's state back to where it is after untarring.

      for my $k (qw(
                    force_update
                    install
                    writemakefile
                    make
                    make_test
                   )) {
          delete $self->{$k};
      }
      $self->{make_clean} = CPAN::Distrostatus->new("YES");

    } else {
      # Hmmm, what to do if make clean failed?

      $self->{make_clean} = CPAN::Distrostatus->new("NO");
      $CPAN::Frontend->mywarn(qq{  $system -- NOT OK\n});

      # 2006-02-27: seems silly to me to force a make now
      # $self->force("make"); # so that this directory won't be used again

    }
    $self->store_persistent_state;
}

#-> sub CPAN::Distribution::check_disabled ;
sub check_disabled {
    my ($self) = @_;
    $self->debug("checking disabled id[$self->{ID}]") if $CPAN::DEBUG;
    if ($self->prefs->{disabled} && ! $self->{force_update}) {
        return sprintf(
                            "Disabled via prefs file '%s' doc %d",
                            $self->{prefs_file},
                            $self->{prefs_file_doc},
                            );
    }
    return;
}

#-> sub CPAN::Distribution::goto ;
sub goto {
    my($self,$goto) = @_;
    $goto = $self->normalize($goto);
    my $why = sprintf(
                      "Goto '$goto' via prefs file '%s' doc %d",
                      $self->{prefs_file},
                      $self->{prefs_file_doc},
                     );
    $self->{unwrapped} = CPAN::Distrostatus->new("NO $why");
    # 2007-07-16 akoenig : Better than NA would be if we could inherit
    # the status of the $goto distro but given the exceptional nature
    # of 'goto' I feel reluctant to implement it
    my $goodbye_message = "[goto] -- NA $why";
    $self->goodbye($goodbye_message);

    # inject into the queue

    CPAN::Queue->delete($self->id);
    CPAN::Queue->jumpqueue({qmod => $goto, reqtype => $self->{reqtype}});

    # and run where we left off

    my($method) = (caller(1))[3];
    my $goto_do = CPAN->instance("CPAN::Distribution",$goto);
    $goto_do->called_for($self->called_for) unless $goto_do->called_for;
    $goto_do->{mandatory} ||= $self->{mandatory};
    $goto_do->{reqtype}   ||= $self->{reqtype};
    $goto_do->{coming_from} = $self->pretty_id;
    $goto_do->$method();
    CPAN::Queue->delete_first($goto);
    # XXX delete_first returns undef; is that what this should return
    # up the call stack, eg. return $sefl->goto($goto) -- xdg, 2012-04-04
}

#-> sub CPAN::Distribution::shortcut_install ;
# return values: undef means don't shortcut; 0 means shortcut as fail;
# and 1 means shortcut as success
sub shortcut_install {
    my ($self) = @_;

    $self->debug("checking previous install results[$self->{ID}]") if $CPAN::DEBUG;
    if (exists $self->{install}) {
        my $text = UNIVERSAL::can($self->{install},"text") ?
            $self->{install}->text :
                $self->{install};
        if ($text =~ /^YES/) {
            $CPAN::META->is_installed($self->{build_dir});
            return $self->success("Already done");
        } elsif ($text =~ /is only/) {
            # e.g. 'is only build_requires': may be overruled later
            return $self->goodbye($text);
        } else {
            # comment in Todo on 2006-02-11; maybe retry?
            return $self->goodbye("Already tried without success");
        }
    }

    for my $slot ( qw/later configure_requires_later/ ) {
        return $self->success($self->{$slot})
        if $self->{$slot};
    }

    return undef;
}

#-> sub CPAN::Distribution::is_being_sponsored ;

# returns true if we find a distro object in the queue that has
# sponsored this one
sub is_being_sponsored {
    my($self) = @_;
    my $iterator = CPAN::Queue->iterator;
 QITEM: while (my $q = $iterator->()) {
        my $s = $q->as_string;
        my $obj = CPAN::Shell->expandany($s) or next QITEM;
        my $type = ref $obj;
        if ( $type eq 'CPAN::Distribution' ){
            for my $module (sort keys %{$obj->{sponsored_mods} || {}}) {
                return 1 if grep { $_ eq $module } $self->containsmods;
            }
        }
    }
    return 0;
}

#-> sub CPAN::Distribution::install ;
sub install {
    my($self) = @_;

    $self->pre_install();

    if (exists $self->{cleanup_after_install_done}) {
        return $self->test;
    }

    $self->debug("checking goto id[$self->{ID}]") if $CPAN::DEBUG;
    if (my $goto = $self->prefs->{goto}) {
        $self->goto($goto);
        $self->post_install();
        return;
    }

    unless ($self->test) {
        $self->post_install();
        return;
    }

    if ( defined( my $sc = $self->shortcut_install ) ) {
        $self->post_install();
        return $sc;
    }

    if ($CPAN::Signal) {
        delete $self->{force_update};
        $self->post_install();
        return;
    }

    my $builddir = $self->dir or
        $CPAN::Frontend->mydie("PANIC: Cannot determine build directory\n");

    unless (chdir $builddir) {
        $CPAN::Frontend->mywarn("Couldn't chdir to '$builddir': $!");
        $self->post_install();
        return;
    }

    $self->debug("Changed directory to $self->{build_dir}")
        if $CPAN::DEBUG;

    my $make = $self->{modulebuild} ? "Build" : "make";
    $CPAN::Frontend->myprint(sprintf "Running %s install for %s\n", $make, $self->pretty_id);

    if ($^O eq 'MacOS') {
        Mac::BuildTools::make_install($self);
        $self->post_install();
        return;
    }

    my $system;
    if (my $commandline = $self->prefs->{install}{commandline}) {
        $system = $commandline;
        $ENV{PERL} = CPAN::find_perl();
    } elsif ($self->{modulebuild}) {
        my($mbuild_install_build_command) =
            exists $CPAN::HandleConfig::keys{mbuild_install_build_command} &&
                $CPAN::Config->{mbuild_install_build_command} ?
                    $CPAN::Config->{mbuild_install_build_command} :
                        $self->_build_command();
        my $install_directive = $^O eq 'VMS' ? '"install"' : 'install';
        $system = sprintf("%s %s %s",
                          $mbuild_install_build_command,
                          $install_directive,
                          $CPAN::Config->{mbuild_install_arg},
                         );
    } else {
        my($make_install_make_command) = $self->_make_install_make_command();
        $system = sprintf("%s install %s",
                          $make_install_make_command,
                          $CPAN::Config->{make_install_arg},
                         );
    }

    my($stderr) = $^O eq "MSWin32" || $^O eq 'VMS' ? "" : " 2>&1 ";
    my $brip = CPAN::HandleConfig->prefs_lookup($self,
                                                q{build_requires_install_policy});
    $brip ||="ask/yes";
    my $id = $self->id;
    my $reqtype = $self->{reqtype} ||= "c"; # in doubt it was a command
    my $want_install = "yes";
    if ($reqtype eq "b") {
        if ($brip eq "no") {
            $want_install = "no";
        } elsif ($brip =~ m|^ask/(.+)|) {
            my $default = $1;
            $default = "yes" unless $default =~ /^(y|n)/i;
            $want_install =
                CPAN::Shell::colorable_makemaker_prompt
                      ("$id is just needed temporarily during building or testing. ".
                       "Do you want to install it permanently?",
                       $default);
        }
    }
    unless ($want_install =~ /^y/i) {
        my $is_only = "is only 'build_requires'";
        $self->{install} = CPAN::Distrostatus->new("NO -- $is_only");
        delete $self->{force_update};
        $self->goodbye("Not installing because $is_only");
        $self->post_install();
        return;
    }
    local $ENV{PERL5LIB} = defined($ENV{PERL5LIB})
                           ? $ENV{PERL5LIB}
                           : ($ENV{PERLLIB} || "");

    local $ENV{PERL5OPT} = defined $ENV{PERL5OPT} ? $ENV{PERL5OPT} : "";
    local $ENV{PERL_USE_UNSAFE_INC} =
        exists $ENV{PERL_USE_UNSAFE_INC} && defined $ENV{PERL_USE_UNSAFE_INC}
        ? $ENV{PERL_USE_UNSAFE_INC} : 1; # install
    $CPAN::META->set_perl5lib;
    local $ENV{PERL_MM_USE_DEFAULT} = 1 if $CPAN::Config->{use_prompt_default};
    local $ENV{NONINTERACTIVE_TESTING} = 1 if $CPAN::Config->{use_prompt_default};

    my $install_env;
    if ($self->prefs->{install}) {
        $install_env = $self->prefs->{install}{env};
    }
    local @ENV{keys %$install_env} = values %$install_env if $install_env;

    if (! $run_allow_installing_within_test) {
        my($allow_installing, $why) = $self->_allow_installing;
        if (! $allow_installing) {
            $CPAN::Frontend->mywarn("Installation stopped: $why\n");
            $self->introduce_myself;
            $self->{install} = CPAN::Distrostatus->new("NO -- installation stopped due $why");
            $CPAN::Frontend->mywarn("  $system -- NOT OK\n");
            delete $self->{force_update};
            $self->post_install();
            return;
        }
    }
    my($pipe) = FileHandle->new("$system $stderr |");
    unless ($pipe) {
        $CPAN::Frontend->mywarn("Can't execute $system: $!");
        $self->introduce_myself;
        $self->{install} = CPAN::Distrostatus->new("NO");
        $CPAN::Frontend->mywarn("  $system -- NOT OK\n");
        delete $self->{force_update};
        $self->post_install();
        return;
    }
    my($makeout) = "";
    while (<$pipe>) {
        print $_; # intentionally NOT use Frontend->myprint because it
                  # looks irritating when we markup in color what we
                  # just pass through from an external program
        $makeout .= $_;
    }
    $pipe->close;
    my $close_ok = $? == 0;
    $self->introduce_myself;
    if ( $close_ok ) {
        $CPAN::Frontend->myprint("  $system -- OK\n");
        $CPAN::META->is_installed($self->{build_dir});
        $self->{install} = CPAN::Distrostatus->new("YES");
        if ($CPAN::Config->{'cleanup_after_install'}
            && ! $self->is_dot_dist
            && ! $self->is_being_sponsored) {
            my $parent = File::Spec->catdir( $self->{build_dir}, File::Spec->updir );
            chdir $parent or $CPAN::Frontend->mydie("Couldn't chdir to $parent: $!\n");
            File::Path::rmtree($self->{build_dir});
            my $yml = "$self->{build_dir}.yml";
            if (-e $yml) {
                unlink $yml or $CPAN::Frontend->mydie("Couldn't unlink $yml: $!\n");
            }
            $self->{cleanup_after_install_done}=1;
        }
    } else {
        $self->{install} = CPAN::Distrostatus->new("NO");
        $CPAN::Frontend->mywarn("  $system -- NOT OK\n");
        my $mimc =
            CPAN::HandleConfig->prefs_lookup($self,
                                             q{make_install_make_command});
        if (
            $makeout =~ /permission/s
            && $> > 0
            && (
                ! $mimc
                || $mimc eq (CPAN::HandleConfig->prefs_lookup($self,
                                                              q{make}))
               )
           ) {
            $CPAN::Frontend->myprint(
                                     qq{----\n}.
                                     qq{  You may have to su }.
                                     qq{to root to install the package\n}.
                                     qq{  (Or you may want to run something like\n}.
                                     qq{    o conf make_install_make_command 'sudo make'\n}.
                                     qq{  to raise your permissions.}
                                    );
        }
    }
    delete $self->{force_update};
    unless ($CPAN::Config->{'cleanup_after_install'}) {
        $self->store_persistent_state;
    }

    $self->post_install();

    return !! $close_ok;
}

sub blib_pm_walk {
    my @queue = grep { -e $_ } File::Spec->catdir("blib","lib"), File::Spec->catdir("blib","arch");
    return sub {
    LOOP: {
            if (@queue) {
                my $file = shift @queue;
                if (-d $file) {
                    my $dh;
                    opendir $dh, $file or next;
                    my @newfiles = map {
                        my @ret;
                        my $maybedir = File::Spec->catdir($file, $_);
                        if (-d $maybedir) {
                            unless (File::Spec->catdir("blib","arch","auto") eq $maybedir) {
                                # prune the blib/arch/auto directory, no pm files there
                                @ret = $maybedir;
                            }
                        } elsif (/\.pm$/) {
                            my $mustbefile = File::Spec->catfile($file, $_);
                            if (-f $mustbefile) {
                                @ret = $mustbefile;
                            }
                        }
                        @ret;
                    } grep {
                        $_ ne "."
                            && $_ ne ".."
                        } readdir $dh;
                    push @queue, @newfiles;
                    redo LOOP;
                } else {
                    return $file;
                }
            } else {
                return;
            }
        }
    };
}

sub _allow_installing {
    my($self) = @_;
    my $id = my $pretty_id = $self->pretty_id;
    if ($self->{CALLED_FOR}) {
        $id .= " (called for $self->{CALLED_FOR})";
    }
    my $allow_down   = CPAN::HandleConfig->prefs_lookup($self,q{allow_installing_module_downgrades});
    $allow_down      ||= "ask/yes";
    my $allow_outdd  = CPAN::HandleConfig->prefs_lookup($self,q{allow_installing_outdated_dists});
    $allow_outdd     ||= "ask/yes";
    return 1 if
           $allow_down  eq "yes"
        && $allow_outdd eq "yes";
    if (($allow_outdd ne "yes") && ! $CPAN::META->has_inst('CPAN::DistnameInfo')) {
        return 1 if grep { $_ eq 'CPAN::DistnameInfo'} $self->containsmods;
        if ($allow_outdd ne "yes") {
            $CPAN::Frontend->mywarn("The current configuration of allow_installing_outdated_dists is '$allow_outdd', but for this option we would need 'CPAN::DistnameInfo' installed. Please install 'CPAN::DistnameInfo' as soon as possible. As long as we are not equipped with 'CPAN::DistnameInfo' this option does not take effect\n");
            $allow_outdd = "yes";
        }
    }
    return 1 if
           $allow_down  eq "yes"
        && $allow_outdd eq "yes";
    my($dist_version, $dist_dist);
    if ($allow_outdd ne "yes"){
        my $dni = CPAN::DistnameInfo->new($pretty_id);
        $dist_version = $dni->version;
        $dist_dist    = $dni->dist;
    }
    my $iterator = blib_pm_walk();
    my(@down,@outdd);
    while (my $file = $iterator->()) {
        my $version = CPAN::Module->parse_version($file);
        my($volume, $directories, $pmfile) = File::Spec->splitpath( $file );
        my @dirs = File::Spec->splitdir( $directories );
        my(@blib_plus1) = splice @dirs, 0, 2;
        my($pmpath) = File::Spec->catfile(grep { length($_) } @dirs, $pmfile);
        unless ($allow_down eq "yes") {
            if (my $inst_file = $self->_file_in_path($pmpath, \@INC)) {
                my $inst_version = CPAN::Module->parse_version($inst_file);
                my $cmp = CPAN::Version->vcmp($version, $inst_version);
                if ($cmp) {
                    if ($cmp < 0) {
                        push @down, { pmpath => $pmpath, version => $version, inst_version => $inst_version };
                    }
                }
                if (@down) {
                    my $why = "allow_installing_module_downgrades: $id contains downgrading module(s) (e.g. '$down[0]{pmpath}' would downgrade installed '$down[0]{inst_version}' to '$down[0]{version}')";
                    if (my($default) = $allow_down =~ m|^ask/(.+)|) {
                        $default = "yes" unless $default =~ /^(y|n)/i;
                        my $answer = CPAN::Shell::colorable_makemaker_prompt
                                ("$why. Do you want to allow installing it?",
                                 $default, "colorize_warn");
                        $allow_down = $answer =~ /^\s*y/i ? "yes" : "no";
                    }
                    if ($allow_down eq "no") {
                        return (0, $why);
                    }
                }
            }
        }
        unless ($allow_outdd eq "yes") {
            my @pmpath = (@dirs, $pmfile);
            $pmpath[-1] =~ s/\.pm$//;
            my $mo = CPAN::Shell->expand("Module",join "::", grep { length($_) } @pmpath);
            if ($mo) {
                my $cpan_version = $mo->cpan_version;
                my $is_lower = CPAN::Version->vlt($version, $cpan_version);
                my $other_dist;
                if (my $mo_dist = $mo->distribution) {
                    $other_dist = $mo_dist->pretty_id;
                    my $dni = CPAN::DistnameInfo->new($other_dist);
                    if ($dni->dist eq $dist_dist){
                        if (CPAN::Version->vgt($dni->version, $dist_version)) {
                            push @outdd, {
                                pmpath       => $pmpath,
                                cpan_path    => $dni->pathname,
                                dist_version => $dni->version,
                                dist_dist    => $dni->dist,
                            };
                        }
                    }
                }
            }
            if (@outdd && $allow_outdd ne "yes") {
                my $why = "allow_installing_outdated_dists: $id contains module(s) that are indexed on the CPAN with a different distro: (e.g. '$outdd[0]{pmpath}' is indexed with '$outdd[0]{cpan_path}')";
                if ($outdd[0]{dist_dist} eq $dist_dist) {
                    $why .= ", and this has a higher distribution-version, i.e. version '$outdd[0]{dist_version}' is higher than '$dist_version')";
                }
                if (my($default) = $allow_outdd =~ m|^ask/(.+)|) {
                    $default = "yes" unless $default =~ /^(y|n)/i;
                    my $answer = CPAN::Shell::colorable_makemaker_prompt
                        ("$why. Do you want to allow installing it?",
                         $default, "colorize_warn");
                    $allow_outdd = $answer =~ /^\s*y/i ? "yes" : "no";
                }
                if ($allow_outdd eq "no") {
                    return (0, $why);
                }
            }
        }
    }
    return 1;
}

sub _file_in_path { # similar to CPAN::Module::_file_in_path
    my($self,$pmpath,$incpath) = @_;
    my($dir,@packpath);
    foreach $dir (@$incpath) {
        my $pmfile = File::Spec->catfile($dir,$pmpath);
        if (-f $pmfile) {
            return $pmfile;
        }
    }
    return;
}
sub introduce_myself {
    my($self) = @_;
    $CPAN::Frontend->myprint(sprintf("  %s\n",$self->pretty_id));
}

#-> sub CPAN::Distribution::dir ;
sub dir {
    shift->{build_dir};
}

#-> sub CPAN::Distribution::perldoc ;
sub perldoc {
    my($self) = @_;

    my($dist) = $self->id;
    my $package = $self->called_for;

    if ($CPAN::META->has_inst("Pod::Perldocs")) {
        my($perl) = $self->perl
            or $CPAN::Frontend->mydie("Couldn't find executable perl\n");
        my @args = ($perl, q{-MPod::Perldocs}, q{-e},
                    q{Pod::Perldocs->run()}, $package);
        my($wstatus);
        unless ( ($wstatus = system(@args)) == 0 ) {
            my $estatus = $wstatus >> 8;
            $CPAN::Frontend->myprint(qq{
    Function system("@args")
    returned status $estatus (wstat $wstatus)
    });
        }
    }
    else {
        $self->_display_url( $CPAN::Defaultdocs . $package );
    }
}

#-> sub CPAN::Distribution::_check_binary ;
sub _check_binary {
    my ($dist,$shell,$binary) = @_;
    my ($pid,$out);

    $CPAN::Frontend->myprint(qq{ + _check_binary($binary)\n})
      if $CPAN::DEBUG;

    if ($CPAN::META->has_inst("File::Which")) {
        return File::Which::which($binary);
    } else {
        local *README;
        $pid = open README, "which $binary|"
            or $CPAN::Frontend->mywarn(qq{Could not fork 'which $binary': $!\n});
        return unless $pid;
        while (<README>) {
            $out .= $_;
        }
        close README
            or $CPAN::Frontend->mywarn("Could not run 'which $binary': $!\n")
                and return;
    }

    $CPAN::Frontend->myprint(qq{   + $out \n})
      if $CPAN::DEBUG && $out;

    return $out;
}

#-> sub CPAN::Distribution::_display_url ;
sub _display_url {
    my($self,$url) = @_;
    my($res,$saved_file,$pid,$out);

    $CPAN::Frontend->myprint(qq{ + _display_url($url)\n})
      if $CPAN::DEBUG;

    # should we define it in the config instead?
    my $html_converter = "html2text.pl";

    my $web_browser = $CPAN::Config->{'lynx'} || undef;
    my $web_browser_out = $web_browser
        ? CPAN::Distribution->_check_binary($self,$web_browser)
        : undef;

    if ($web_browser_out) {
        # web browser found, run the action
        my $browser = CPAN::HandleConfig->safe_quote($CPAN::Config->{'lynx'});
        $CPAN::Frontend->myprint(qq{system[$browser $url]})
            if $CPAN::DEBUG;
        $CPAN::Frontend->myprint(qq{
Displaying URL
  $url
with browser $browser
});
        $CPAN::Frontend->mysleep(1);
        system("$browser $url");
        if ($saved_file) { 1 while unlink($saved_file) }
    } else {
        # web browser not found, let's try text only
        my $html_converter_out =
            CPAN::Distribution->_check_binary($self,$html_converter);
        $html_converter_out = CPAN::HandleConfig->safe_quote($html_converter_out);

        if ($html_converter_out ) {
            # html2text found, run it
            $saved_file = CPAN::Distribution->_getsave_url( $self, $url );
            $CPAN::Frontend->mydie(qq{ERROR: problems while getting $url\n})
                unless defined($saved_file);

            local *README;
            $pid = open README, "$html_converter $saved_file |"
                or $CPAN::Frontend->mydie(qq{
Could not fork '$html_converter $saved_file': $!});
            my($fh,$filename);
            if ($CPAN::META->has_usable("File::Temp")) {
                $fh = File::Temp->new(
                                      dir      => File::Spec->tmpdir,
                                      template => 'cpan_htmlconvert_XXXX',
                                      suffix => '.txt',
                                      unlink => 0,
                                     );
                $filename = $fh->filename;
            } else {
                $filename = "cpan_htmlconvert_$$.txt";
                $fh = FileHandle->new();
                open $fh, ">$filename" or die;
            }
            while (<README>) {
                $fh->print($_);
            }
            close README or
                $CPAN::Frontend->mydie(qq{Could not run '$html_converter $saved_file': $!});
            my $tmpin = $fh->filename;
            $CPAN::Frontend->myprint(sprintf(qq{
Run '%s %s' and
saved output to %s\n},
                                             $html_converter,
                                             $saved_file,
                                             $tmpin,
                                            )) if $CPAN::DEBUG;
            close $fh;
            local *FH;
            open FH, $tmpin
                or $CPAN::Frontend->mydie(qq{Could not open "$tmpin": $!});
            my $fh_pager = FileHandle->new;
            local($SIG{PIPE}) = "IGNORE";
            my $pager = $CPAN::Config->{'pager'} || "cat";
            $fh_pager->open("|$pager")
                or $CPAN::Frontend->mydie(qq{
Could not open pager '$pager': $!});
            $CPAN::Frontend->myprint(qq{
Displaying URL
  $url
with pager "$pager"
});
            $CPAN::Frontend->mysleep(1);
            $fh_pager->print(<FH>);
            $fh_pager->close;
        } else {
            # coldn't find the web browser or html converter
            $CPAN::Frontend->myprint(qq{
You need to install lynx or $html_converter to use this feature.});
        }
    }
}

#-> sub CPAN::Distribution::_getsave_url ;
sub _getsave_url {
    my($dist, $shell, $url) = @_;

    $CPAN::Frontend->myprint(qq{ + _getsave_url($url)\n})
      if $CPAN::DEBUG;

    my($fh,$filename);
    if ($CPAN::META->has_usable("File::Temp")) {
        $fh = File::Temp->new(
                              dir      => File::Spec->tmpdir,
                              template => "cpan_getsave_url_XXXX",
                              suffix => ".html",
                              unlink => 0,
                             );
        $filename = $fh->filename;
    } else {
        $fh = FileHandle->new;
        $filename = "cpan_getsave_url_$$.html";
    }
    my $tmpin = $filename;
    if ($CPAN::META->has_usable('LWP')) {
        $CPAN::Frontend->myprint("Fetching with LWP:
  $url
");
        my $Ua;
        CPAN::LWP::UserAgent->config;
        eval { $Ua = CPAN::LWP::UserAgent->new; };
        if ($@) {
            $CPAN::Frontend->mywarn("ERROR: CPAN::LWP::UserAgent->new dies with $@\n");
            return;
        } else {
            my($var);
            $Ua->proxy('http', $var)
                if $var = $CPAN::Config->{http_proxy} || $ENV{http_proxy};
            $Ua->no_proxy($var)
                if $var = $CPAN::Config->{no_proxy} || $ENV{no_proxy};
        }

        my $req = HTTP::Request->new(GET => $url);
        $req->header('Accept' => 'text/html');
        my $res = $Ua->request($req);
        if ($res->is_success) {
            $CPAN::Frontend->myprint(" + request successful.\n")
                if $CPAN::DEBUG;
            print $fh $res->content;
            close $fh;
            $CPAN::Frontend->myprint(qq{ + saved content to $tmpin \n})
                if $CPAN::DEBUG;
            return $tmpin;
        } else {
            $CPAN::Frontend->myprint(sprintf(
                                             "LWP failed with code[%s], message[%s]\n",
                                             $res->code,
                                             $res->message,
                                            ));
            return;
        }
    } else {
        $CPAN::Frontend->mywarn("  LWP not available\n");
        return;
    }
}

#-> sub CPAN::Distribution::_build_command
sub _build_command {
    my($self) = @_;
    if ($^O eq "MSWin32") { # special code needed at least up to
                            # Module::Build 0.2611 and 0.2706; a fix
                            # in M:B has been promised 2006-01-30
        my($perl) = $self->perl or $CPAN::Frontend->mydie("Couldn't find executable perl\n");
        return "$perl ./Build";
    }
    elsif ($^O eq 'VMS') {
        return "$^X Build.com";
    }
    return "./Build";
}

#-> sub CPAN::Distribution::_should_report
sub _should_report {
    my($self, $phase) = @_;
    die "_should_report() requires a 'phase' argument"
        if ! defined $phase;

    return unless $CPAN::META->has_usable("CPAN::Reporter");

    # configured
    my $test_report = CPAN::HandleConfig->prefs_lookup($self,
                                                       q{test_report});
    return unless $test_report;

    # don't repeat if we cached a result
    return $self->{should_report}
        if exists $self->{should_report};

    # don't report if we generated a Makefile.PL
    if ( $self->{had_no_makefile_pl} ) {
        $CPAN::Frontend->mywarn(
            "Will not send CPAN Testers report with generated Makefile.PL.\n"
        );
        return $self->{should_report} = 0;
    }

    # available
    if ( ! $CPAN::META->has_inst("CPAN::Reporter")) {
        $CPAN::Frontend->mywarnonce(
            "CPAN::Reporter not installed.  No reports will be sent.\n"
        );
        return $self->{should_report} = 0;
    }

    # capable
    my $crv = CPAN::Reporter->VERSION;
    if ( CPAN::Version->vlt( $crv, 0.99 ) ) {
        # don't cache $self->{should_report} -- need to check each phase
        if ( $phase eq 'test' ) {
            return 1;
        }
        else {
            $CPAN::Frontend->mywarn(
                "Reporting on the '$phase' phase requires CPAN::Reporter 0.99, but \n" .
                "you only have version $crv\.  Only 'test' phase reports will be sent.\n"
            );
            return;
        }
    }

    # appropriate
    if ($self->is_dot_dist) {
        $CPAN::Frontend->mywarn("Reporting via CPAN::Reporter is disabled ".
                                "for local directories\n");
        return $self->{should_report} = 0;
    }
    if ($self->prefs->{patches}
        &&
        @{$self->prefs->{patches}}
        &&
        $self->{patched}
       ) {
        $CPAN::Frontend->mywarn("Reporting via CPAN::Reporter is disabled ".
                                "when the source has been patched\n");
        return $self->{should_report} = 0;
    }

    # proceed and cache success
    return $self->{should_report} = 1;
}

#-> sub CPAN::Distribution::reports
sub reports {
    my($self) = @_;
    my $pathname = $self->id;
    $CPAN::Frontend->myprint("Distribution: $pathname\n");

    unless ($CPAN::META->has_inst("CPAN::DistnameInfo")) {
        $CPAN::Frontend->mydie("CPAN::DistnameInfo not installed; cannot continue");
    }
    unless ($CPAN::META->has_usable("LWP")) {
        $CPAN::Frontend->mydie("LWP not installed; cannot continue");
    }
    unless ($CPAN::META->has_usable("File::Temp")) {
        $CPAN::Frontend->mydie("File::Temp not installed; cannot continue");
    }

    my $format;
    if ($CPAN::META->has_inst("YAML::XS") || $CPAN::META->has_inst("YAML::Syck")){
        $format = 'yaml';
    }
    elsif (!$format && $CPAN::META->has_inst("JSON::PP") ) {
        $format = 'json';
    }
    else {
        $CPAN::Frontend->mydie("JSON::PP not installed, cannot continue");
    }

    my $d = CPAN::DistnameInfo->new($pathname);

    my $dist      = $d->dist;      # "CPAN-DistnameInfo"
    my $version   = $d->version;   # "0.02"
    my $maturity  = $d->maturity;  # "released"
    my $filename  = $d->filename;  # "CPAN-DistnameInfo-0.02.tar.gz"
    my $cpanid    = $d->cpanid;    # "GBARR"
    my $distvname = $d->distvname; # "CPAN-DistnameInfo-0.02"

    my $url = sprintf "http://www.cpantesters.org/show/%s.%s", $dist, $format;

    CPAN::LWP::UserAgent->config;
    my $Ua;
    eval { $Ua = CPAN::LWP::UserAgent->new; };
    if ($@) {
        $CPAN::Frontend->mydie("CPAN::LWP::UserAgent->new dies with $@\n");
    }
    $CPAN::Frontend->myprint("Fetching '$url'...");
    my $resp = $Ua->get($url);
    unless ($resp->is_success) {
        $CPAN::Frontend->mydie(sprintf "Could not download '%s': %s\n", $url, $resp->code);
    }
    $CPAN::Frontend->myprint("DONE\n\n");
    my $unserialized;
    if ( $format eq 'yaml' ) {
        my $yaml = $resp->content;
        # what a long way round!
        my $fh = File::Temp->new(
                                 dir      => File::Spec->tmpdir,
                                 template => 'cpan_reports_XXXX',
                                 suffix => '.yaml',
                                 unlink => 0,
                                );
        my $tfilename = $fh->filename;
        print $fh $yaml;
        close $fh or $CPAN::Frontend->mydie("Could not close '$tfilename': $!");
        $unserialized = CPAN->_yaml_loadfile($tfilename)->[0];
        unlink $tfilename or $CPAN::Frontend->mydie("Could not unlink '$tfilename': $!");
    } else {
        require JSON::PP;
        $unserialized = JSON::PP->new->utf8->decode($resp->content);
    }
    my %other_versions;
    my $this_version_seen;
    for my $rep (@$unserialized) {
        my $rversion = $rep->{version};
        if ($rversion eq $version) {
            unless ($this_version_seen++) {
                $CPAN::Frontend->myprint ("$rep->{version}:\n");
            }
            my $arch = $rep->{archname} || $rep->{platform}        || '????';
            my $grade = $rep->{action}  || $rep->{status}          || '????';
            my $ostext = $rep->{ostext} || ucfirst($rep->{osname}) || '????';
            $CPAN::Frontend->myprint
                (sprintf("%1s%1s%-4s %s on %s %s (%s)\n",
                         $arch eq $Config::Config{archname}?"*":"",
                         $grade eq "PASS"?"+":$grade eq"FAIL"?"-":"",
                         $grade,
                         $rep->{perl},
                         $ostext,
                         $rep->{osvers},
                         $arch,
                        ));
        } else {
            $other_versions{$rep->{version}}++;
        }
    }
    unless ($this_version_seen) {
        $CPAN::Frontend->myprint("No reports found for version '$version'
Reports for other versions:\n");
        for my $v (sort keys %other_versions) {
            $CPAN::Frontend->myprint(" $v\: $other_versions{$v}\n");
        }
    }
    $url = substr($url,0,-4) . 'html';
    $CPAN::Frontend->myprint("See $url for details\n");
}

1;
