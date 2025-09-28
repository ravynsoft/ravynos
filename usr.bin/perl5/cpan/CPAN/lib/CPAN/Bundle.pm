# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Bundle;
use strict;
use CPAN::Module;
@CPAN::Bundle::ISA = qw(CPAN::Module);

use vars qw(
            $VERSION
);
$VERSION = "5.5005";

sub look {
    my $self = shift;
    $CPAN::Frontend->myprint($self->as_string);
}

#-> CPAN::Bundle::undelay
sub undelay {
    my $self = shift;
    delete $self->{later};
    for my $c ( $self->contains ) {
        my $obj = CPAN::Shell->expandany($c) or next;
        if ($obj->id eq $self->id){
            my $id = $obj->id;
            $CPAN::Frontend->mywarn("$id seems to contain itself, skipping\n");
            next;
        }
        $obj->undelay;
    }
}

# mark as dirty/clean
#-> sub CPAN::Bundle::color_cmd_tmps ;
sub color_cmd_tmps {
    my($self) = shift;
    my($depth) = shift || 0;
    my($color) = shift || 0;
    my($ancestors) = shift || [];
    # a module needs to recurse to its cpan_file, a distribution needs
    # to recurse into its prereq_pms, a bundle needs to recurse into its modules

    return if exists $self->{incommandcolor}
        && $color==1
        && $self->{incommandcolor}==$color;
    if ($depth>=$CPAN::MAX_RECURSION) {
        my $e = CPAN::Exception::RecursiveDependency->new($ancestors);
        if ($e->is_resolvable) {
            return $self->{incommandcolor}=2;
        } else {
            die $e;
        }
    }
    # warn "color_cmd_tmps $depth $color " . $self->id; # sleep 1;

    for my $c ( $self->contains ) {
        my $obj = CPAN::Shell->expandany($c) or next;
        CPAN->debug("c[$c]obj[$obj]") if $CPAN::DEBUG;
        $obj->color_cmd_tmps($depth+1,$color,[@$ancestors, $self->id]);
    }
    # never reached code?
    #if ($color==0) {
      #delete $self->{badtestcnt};
    #}
    $self->{incommandcolor} = $color;
}

#-> sub CPAN::Bundle::as_string ;
sub as_string {
    my($self) = @_;
    $self->contains;
    # following line must be "=", not "||=" because we have a moving target
    $self->{INST_VERSION} = $self->inst_version;
    return $self->SUPER::as_string;
}

#-> sub CPAN::Bundle::contains ;
sub contains {
    my($self) = @_;
    my($inst_file) = $self->inst_file || "";
    my($id) = $self->id;
    $self->debug("inst_file[$inst_file]id[$id]") if $CPAN::DEBUG;
    if ($inst_file && CPAN::Version->vlt($self->inst_version,$self->cpan_version)) {
        undef $inst_file;
    }
    unless ($inst_file) {
        # Try to get at it in the cpan directory
        $self->debug("no inst_file") if $CPAN::DEBUG;
        my $cpan_file;
        $CPAN::Frontend->mydie("I don't know a bundle with ID '$id'\n") unless
              $cpan_file = $self->cpan_file;
        if ($cpan_file eq "N/A") {
            $CPAN::Frontend->mywarn("Bundle '$id' not found on disk and not on CPAN. Maybe stale symlink? Maybe removed during session?\n");
            return;
        }
        my $dist = $CPAN::META->instance('CPAN::Distribution',
                                         $self->cpan_file);
        $self->debug("before get id[$dist->{ID}]") if $CPAN::DEBUG;
        $dist->get;
        $self->debug("after get id[$dist->{ID}]") if $CPAN::DEBUG;
        my($todir) = $CPAN::Config->{'cpan_home'};
        my(@me,$from,$to,$me);
        @me = split /::/, $self->id;
        $me[-1] .= ".pm";
        $me = File::Spec->catfile(@me);
        my $build_dir;
        unless ($build_dir = $dist->{build_dir}) {
            $CPAN::Frontend->mywarn("Warning: cannot determine bundle content without a build_dir.\n");
            return;
        }
        $from = $self->find_bundle_file($build_dir,join('/',@me));
        $to = File::Spec->catfile($todir,$me);
        File::Path::mkpath(File::Basename::dirname($to));
        File::Copy::copy($from, $to)
              or Carp::confess("Couldn't copy $from to $to: $!");
        $inst_file = $to;
    }
    my @result;
    my $fh = FileHandle->new;
    local $/ = "\n";
    open($fh,$inst_file) or die "Could not open '$inst_file': $!";
    my $in_cont = 0;
    $self->debug("inst_file[$inst_file]") if $CPAN::DEBUG;
    while (<$fh>) {
        $in_cont = m/^=(?!head1\s+(?i-xsm:CONTENTS))/ ? 0 :
            m/^=head1\s+(?i-xsm:CONTENTS)/ ? 1 : $in_cont;
        next unless $in_cont;
        next if /^=/;
        s/\#.*//;
        next if /^\s+$/;
        chomp;
        push @result, (split " ", $_, 2)[0];
    }
    close $fh;
    delete $self->{STATUS};
    $self->{CONTAINS} = \@result;
    $self->debug("CONTAINS[@result]") if $CPAN::DEBUG;
    unless (@result) {
        $CPAN::Frontend->mywarn(qq{
The bundle file "$inst_file" may be a broken
bundlefile. It seems not to contain any bundle definition.
Please check the file and if it is bogus, please delete it.
Sorry for the inconvenience.
});
    }
    @result;
}

#-> sub CPAN::Bundle::find_bundle_file
# $where is in local format, $what is in unix format
sub find_bundle_file {
    my($self,$where,$what) = @_;
    $self->debug("where[$where]what[$what]") if $CPAN::DEBUG;
### The following two lines let CPAN.pm become Bundle/CPAN.pm :-(
###    my $bu = File::Spec->catfile($where,$what);
###    return $bu if -f $bu;
    my $manifest = File::Spec->catfile($where,"MANIFEST");
    unless (-f $manifest) {
        require ExtUtils::Manifest;
        my $cwd = CPAN::anycwd();
        $self->safe_chdir($where);
        ExtUtils::Manifest::mkmanifest();
        $self->safe_chdir($cwd);
    }
    my $fh = FileHandle->new($manifest)
        or Carp::croak("Couldn't open $manifest: $!");
    local($/) = "\n";
    my $bundle_filename = $what;
    $bundle_filename =~ s|Bundle.*/||;
    my $bundle_unixpath;
    while (<$fh>) {
        next if /^\s*\#/;
        my($file) = /(\S+)/;
        if ($file =~ m|\Q$what\E$|) {
            $bundle_unixpath = $file;
            # return File::Spec->catfile($where,$bundle_unixpath); # bad
            last;
        }
        # retry if she managed to have no Bundle directory
        $bundle_unixpath = $file if $file =~ m|\Q$bundle_filename\E$|;
    }
    return File::Spec->catfile($where, split /\//, $bundle_unixpath)
        if $bundle_unixpath;
    Carp::croak("Couldn't find a Bundle file in $where");
}

# needs to work quite differently from Module::inst_file because of
# cpan_home/Bundle/ directory and the possibility that we have
# shadowing effect. As it makes no sense to take the first in @INC for
# Bundles, we parse them all for $VERSION and take the newest.

#-> sub CPAN::Bundle::inst_file ;
sub inst_file {
    my($self) = @_;
    my($inst_file);
    my(@me);
    @me = split /::/, $self->id;
    $me[-1] .= ".pm";
    my($incdir,$bestv);
    foreach $incdir ($CPAN::Config->{'cpan_home'},@INC) {
        my $parsefile = File::Spec->catfile($incdir, @me);
        CPAN->debug("parsefile[$parsefile]") if $CPAN::DEBUG;
        next unless -f $parsefile;
        my $have = eval { MM->parse_version($parsefile); };
        if ($@) {
            $CPAN::Frontend->mywarn("Error while parsing version number in file '$parsefile'\n");
        }
        if (!$bestv || CPAN::Version->vgt($have,$bestv)) {
            $self->{INST_FILE} = $parsefile;
            $self->{INST_VERSION} = $bestv = $have;
        }
    }
    $self->{INST_FILE};
}

#-> sub CPAN::Bundle::inst_version ;
sub inst_version {
    my($self) = @_;
    $self->inst_file; # finds INST_VERSION as side effect
    $self->{INST_VERSION};
}

#-> sub CPAN::Bundle::rematein ;
sub rematein {
    my($self,$meth) = @_;
    $self->debug("self[$self] meth[$meth]") if $CPAN::DEBUG;
    my($id) = $self->id;
    Carp::croak( "Can't $meth $id, don't have an associated bundle file. :-(\n" )
        unless $self->inst_file || $self->cpan_file;
    my($s,%fail);
    for $s ($self->contains) {
        my($type) = $s =~ m|/| ? 'CPAN::Distribution' :
            $s =~ m|^Bundle::| ? 'CPAN::Bundle' : 'CPAN::Module';
        if ($type eq 'CPAN::Distribution') {
            $CPAN::Frontend->mywarn(qq{
The Bundle }.$self->id.qq{ contains
explicitly a file '$s'.
Going to $meth that.
});
            $CPAN::Frontend->mysleep(5);
        }
        # possibly noisy action:
        $self->debug("type[$type] s[$s]") if $CPAN::DEBUG;
        my $obj = $CPAN::META->instance($type,$s);
        $obj->{reqtype} = $self->{reqtype};
        $obj->{viabundle} ||= { id => $id, reqtype => $self->{reqtype}, optional => !$self->{mandatory}};
        # $obj->$meth();
        # XXX should optional be based on whether bundle was optional? -- xdg, 2012-04-01
        # A: Sure, what could demand otherwise? --andk, 2013-11-25
        CPAN::Queue->queue_item(qmod => $obj->id, reqtype => $self->{reqtype}, optional => !$self->{mandatory});
    }
}

# If a bundle contains another that contains an xs_file we have here,
# we just don't bother I suppose
#-> sub CPAN::Bundle::xs_file
sub xs_file {
    return 0;
}

#-> sub CPAN::Bundle::force ;
sub fforce   { shift->rematein('fforce',@_); }
#-> sub CPAN::Bundle::force ;
sub force   { shift->rematein('force',@_); }
#-> sub CPAN::Bundle::notest ;
sub notest  { shift->rematein('notest',@_); }
#-> sub CPAN::Bundle::get ;
sub get     { shift->rematein('get',@_); }
#-> sub CPAN::Bundle::make ;
sub make    { shift->rematein('make',@_); }
#-> sub CPAN::Bundle::test ;
sub test    {
    my $self = shift;
    # $self->{badtestcnt} ||= 0;
    $self->rematein('test',@_);
}
#-> sub CPAN::Bundle::install ;
sub install {
  my $self = shift;
  $self->rematein('install',@_);
}
#-> sub CPAN::Bundle::clean ;
sub clean   { shift->rematein('clean',@_); }

#-> sub CPAN::Bundle::uptodate ;
sub uptodate {
    my($self) = @_;
    return 0 unless $self->SUPER::uptodate; # we must have the current Bundle def
    my $c;
    foreach $c ($self->contains) {
        my $obj = CPAN::Shell->expandany($c);
        return 0 unless $obj->uptodate;
    }
    return 1;
}

#-> sub CPAN::Bundle::readme ;
sub readme  {
    my($self) = @_;
    my($file) = $self->cpan_file or $CPAN::Frontend->myprint(qq{
No File found for bundle } . $self->id . qq{\n}), return;
    $self->debug("self[$self] file[$file]") if $CPAN::DEBUG;
    $CPAN::META->instance('CPAN::Distribution',$file)->readme;
}

1;
