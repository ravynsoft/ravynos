# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::CacheMgr;
use strict;
use CPAN::InfoObj;
@CPAN::CacheMgr::ISA = qw(CPAN::InfoObj CPAN);
use Cwd qw(chdir);
use File::Find;

use vars qw(
            $VERSION
);
$VERSION = "5.5002";

package CPAN::CacheMgr;
use strict;

#-> sub CPAN::CacheMgr::as_string ;
sub as_string {
    eval { require Data::Dumper };
    if ($@) {
        return shift->SUPER::as_string;
    } else {
        return Data::Dumper::Dumper(shift);
    }
}

#-> sub CPAN::CacheMgr::cachesize ;
sub cachesize {
    shift->{DU};
}

#-> sub CPAN::CacheMgr::tidyup ;
sub tidyup {
  my($self) = @_;
  return unless $CPAN::META->{LOCK};
  return unless -d $self->{ID};
  my @toremove = grep { $self->{SIZE}{$_}==0 } @{$self->{FIFO}};
  for my $current (0..$#toremove) {
    my $toremove = $toremove[$current];
    $CPAN::Frontend->myprint(sprintf(
                                     "DEL(%d/%d): %s \n",
                                     $current+1,
                                     scalar @toremove,
                                     $toremove,
                                    )
                            );
    return if $CPAN::Signal;
    $self->_clean_cache($toremove);
    return if $CPAN::Signal;
  }
  $self->{FIFO} = [];
}

#-> sub CPAN::CacheMgr::dir ;
sub dir {
    shift->{ID};
}

#-> sub CPAN::CacheMgr::entries ;
sub entries {
    my($self,$dir) = @_;
    return unless defined $dir;
    $self->debug("reading dir[$dir]") if $CPAN::DEBUG;
    $dir ||= $self->{ID};
    my($cwd) = CPAN::anycwd();
    chdir $dir or Carp::croak("Can't chdir to $dir: $!");
    my $dh = DirHandle->new(File::Spec->curdir)
        or Carp::croak("Couldn't opendir $dir: $!");
    my(@entries);
    for ($dh->read) {
        next if $_ eq "." || $_ eq "..";
        if (-f $_) {
            push @entries, File::Spec->catfile($dir,$_);
        } elsif (-d _) {
            push @entries, File::Spec->catdir($dir,$_);
        } else {
            $CPAN::Frontend->mywarn("Warning: weird direntry in $dir: $_\n");
        }
    }
    chdir $cwd or Carp::croak("Can't chdir to $cwd: $!");
    sort { -M $a <=> -M $b} @entries;
}

#-> sub CPAN::CacheMgr::disk_usage ;
sub disk_usage {
    my($self,$dir,$fast) = @_;
    return if exists $self->{SIZE}{$dir};
    return if $CPAN::Signal;
    my($Du) = 0;
    if (-e $dir) {
        if (-d $dir) {
            unless (-x $dir) {
                unless (chmod 0755, $dir) {
                    $CPAN::Frontend->mywarn("I have neither the -x permission nor the ".
                                            "permission to change the permission; cannot ".
                                            "estimate disk usage of '$dir'\n");
                    $CPAN::Frontend->mysleep(5);
                    return;
                }
            }
        } elsif (-f $dir) {
            # nothing to say, no matter what the permissions
        }
    } else {
        $CPAN::Frontend->mywarn("File or directory '$dir' has gone, ignoring\n");
        return;
    }
    if ($fast) {
        $Du = 0; # placeholder
    } else {
        find(
             sub {
           $File::Find::prune++ if $CPAN::Signal;
           return if -l $_;
           if ($^O eq 'MacOS') {
             require Mac::Files;
             my $cat  = Mac::Files::FSpGetCatInfo($_);
             $Du += $cat->ioFlLgLen() + $cat->ioFlRLgLen() if $cat;
           } else {
             if (-d _) {
               unless (-x _) {
                 unless (chmod 0755, $_) {
                   $CPAN::Frontend->mywarn("I have neither the -x permission nor ".
                                           "the permission to change the permission; ".
                                           "can only partially estimate disk usage ".
                                           "of '$_'\n");
                   $CPAN::Frontend->mysleep(5);
                   return;
                 }
               }
             } else {
               $Du += (-s _);
             }
           }
         },
         $dir
            );
    }
    return if $CPAN::Signal;
    $self->{SIZE}{$dir} = $Du/1024/1024;
    unshift @{$self->{FIFO}}, $dir;
    $self->debug("measured $dir is $Du") if $CPAN::DEBUG;
    $self->{DU} += $Du/1024/1024;
    $self->{DU};
}

#-> sub CPAN::CacheMgr::_clean_cache ;
sub _clean_cache {
    my($self,$dir) = @_;
    return unless -e $dir;
    unless (File::Spec->canonpath(File::Basename::dirname($dir))
            eq File::Spec->canonpath($CPAN::Config->{build_dir})) {
        $CPAN::Frontend->mywarn("Directory '$dir' not below $CPAN::Config->{build_dir}, ".
                                "will not remove\n");
        $CPAN::Frontend->mysleep(5);
        return;
    }
    $self->debug("have to rmtree $dir, will free $self->{SIZE}{$dir}")
        if $CPAN::DEBUG;
    File::Path::rmtree($dir);
    my $id_deleted = 0;
    if ($dir !~ /\.yml$/ && -f "$dir.yml") {
        my $yaml_module = CPAN::_yaml_module();
        if ($CPAN::META->has_inst($yaml_module)) {
            my($peek_yaml) = eval { CPAN->_yaml_loadfile("$dir.yml"); };
            if ($@) {
                $CPAN::Frontend->mywarn("(parse error on '$dir.yml' removing anyway)");
                unlink "$dir.yml" or
                    $CPAN::Frontend->mywarn("(Could not unlink '$dir.yml': $!)");
                return;
            } elsif (my $id = $peek_yaml->[0]{distribution}{ID}) {
                $CPAN::META->delete("CPAN::Distribution", $id);

                # XXX we should restore the state NOW, otherwise this
                # distro does not exist until we read an index. BUG ALERT(?)

                # $CPAN::Frontend->mywarn (" +++\n");
                $id_deleted++;
            }
        }
        unlink "$dir.yml"; # may fail
        unless ($id_deleted) {
            CPAN->debug("no distro found associated with '$dir'");
        }
    }
    $self->{DU} -= $self->{SIZE}{$dir};
    delete $self->{SIZE}{$dir};
}

#-> sub CPAN::CacheMgr::new ;
sub new {
    my($class,$phase) = @_;
    $phase ||= "atstart";
    my $time = time;
    my($debug,$t2);
    $debug = "";
    my $self = {
        ID => $CPAN::Config->{build_dir},
        MAX => $CPAN::Config->{'build_cache'},
        SCAN => $CPAN::Config->{'scan_cache'} || 'atstart',
        DU => 0
    };
    $CPAN::Frontend->mydie("Unknown scan_cache argument: $self->{SCAN}")
        unless $self->{SCAN} =~ /never|atstart|atexit/;
    File::Path::mkpath($self->{ID});
    my $dh = DirHandle->new($self->{ID});
    bless $self, $class;
    $self->scan_cache($phase);
    $t2 = time;
    $debug .= "timing of CacheMgr->new: ".($t2 - $time);
    $time = $t2;
    CPAN->debug($debug) if $CPAN::DEBUG;
    $self;
}

#-> sub CPAN::CacheMgr::scan_cache ;
sub scan_cache {
    my ($self, $phase) = @_;
    $phase = '' unless defined $phase;
    return unless $phase eq $self->{SCAN};
    return unless $CPAN::META->{LOCK};
    $CPAN::Frontend->myprint(
                             sprintf("Scanning cache %s for sizes\n",
                             $self->{ID}));
    my $e;
    my @entries = $self->entries($self->{ID});
    my $i = 0;
    my $painted = 0;
    for $e (@entries) {
        my $symbol = ".";
        if ($self->{DU} > $self->{MAX}) {
            $symbol = "-";
            $self->disk_usage($e,1);
        } else {
            $self->disk_usage($e);
        }
        $i++;
        while (($painted/76) < ($i/@entries)) {
            $CPAN::Frontend->myprint($symbol);
            $painted++;
        }
        return if $CPAN::Signal;
    }
    $CPAN::Frontend->myprint("DONE\n");
    $self->tidyup;
}

1;
