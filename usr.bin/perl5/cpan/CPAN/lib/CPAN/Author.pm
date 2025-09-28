# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Author;
use strict;

use CPAN::InfoObj;
@CPAN::Author::ISA = qw(CPAN::InfoObj);
use vars qw(
            $VERSION
);
$VERSION = "5.5002";

package CPAN::Author;
use strict;

#-> sub CPAN::Author::force
sub force {
    my $self = shift;
    $self->{force}++;
}

#-> sub CPAN::Author::force
sub unforce {
    my $self = shift;
    delete $self->{force};
}

#-> sub CPAN::Author::id
sub id {
    my $self = shift;
    my $id = $self->{ID};
    $CPAN::Frontend->mydie("Illegal author id[$id]") unless $id =~ /^[A-Z]/;
    $id;
}

#-> sub CPAN::Author::as_glimpse ;
sub as_glimpse {
    my($self) = @_;
    my(@m);
    my $class = ref($self);
    $class =~ s/^CPAN:://;
    push @m, sprintf(qq{%-15s %s ("%s" <%s>)\n},
                     $class,
                     $self->{ID},
                     $self->fullname,
                     $self->email);
    join "", @m;
}

#-> sub CPAN::Author::fullname ;
sub fullname {
    shift->ro->{FULLNAME};
}
*name = \&fullname;

#-> sub CPAN::Author::email ;
sub email    { shift->ro->{EMAIL}; }

#-> sub CPAN::Author::ls ;
sub ls {
    my $self = shift;
    my $glob = shift || "";
    my $silent = shift || 0;
    my $id = $self->id;

    # adapted from CPAN::Distribution::verifyCHECKSUM ;
    my(@csf); # chksumfile
    @csf = $self->id =~ /(.)(.)(.*)/;
    $csf[1] = join "", @csf[0,1];
    $csf[2] = join "", @csf[1,2]; # ("A","AN","ANDK")
    my(@dl);
    @dl = $self->dir_listing([$csf[0],"CHECKSUMS"], 0, 1);
    unless (grep {$_->[2] eq $csf[1]} @dl) {
        $CPAN::Frontend->myprint("Directory $csf[1]/ does not exist\n") unless $silent ;
        return;
    }
    @dl = $self->dir_listing([@csf[0,1],"CHECKSUMS"], 0, 1);
    unless (grep {$_->[2] eq $csf[2]} @dl) {
        $CPAN::Frontend->myprint("Directory $id/ does not exist\n") unless $silent;
        return;
    }
    @dl = $self->dir_listing([@csf,"CHECKSUMS"], 1, 1);
    if ($glob) {
        if ($CPAN::META->has_inst("Text::Glob")) {
            $glob =~ s|/$|/*|;
            my $rglob = Text::Glob::glob_to_regex($glob);
            CPAN->debug("glob[$glob]rglob[$rglob]dl[@dl]") if $CPAN::DEBUG;
            my @tmpdl = grep { $_->[2] =~ /$rglob/ } @dl;
            if (1==@tmpdl && $tmpdl[0][0]==0) {
                $rglob = Text::Glob::glob_to_regex("$glob/*");
                @dl = grep { $_->[2] =~ /$rglob/ } @dl;
            } else {
                @dl = @tmpdl;
            }
            CPAN->debug("rglob[$rglob]dl[@dl]") if $CPAN::DEBUG;
        } else {
            $CPAN::Frontend->mydie("Text::Glob not installed, cannot proceed");
        }
    }
    unless ($silent >= 2) {
        $CPAN::Frontend->myprint
            (
             join "",
             map {
                 sprintf
                     (
                      "%8d %10s %s/%s%s\n",
                      $_->[0],
                      $_->[1],
                      $id,
                      $_->[2],
                      0==$_->[0]?"/":"",
                     )
                 } sort { $a->[2] cmp $b->[2] } @dl
            );
    }
    @dl;
}

# returns an array of arrays, the latter contain (size,mtime,filename)
#-> sub CPAN::Author::dir_listing ;
sub dir_listing {
    my $self = shift;
    my $chksumfile = shift;
    my $recursive = shift;
    my $may_ftp = shift;

    my $lc_want =
        File::Spec->catfile($CPAN::Config->{keep_source_where},
                            "authors", "id", @$chksumfile);

    my $fh;

    CPAN->debug("chksumfile[@$chksumfile]recursive[$recursive]may_ftp[$may_ftp]") if $CPAN::DEBUG;
    # Purge and refetch old (pre-PGP) CHECKSUMS; they are a security
    # hazard.  (Without GPG installed they are not that much better,
    # though.)
    $fh = FileHandle->new;
    if (open($fh, $lc_want)) {
        my $line = <$fh>; close $fh;
        unlink($lc_want) unless $line =~ /PGP/;
    }

    local($") = "/";
    # connect "force" argument with "index_expire".
    my $force = $self->{force};
    if (my @stat = stat $lc_want) {
        $force ||= $stat[9] + $CPAN::Config->{index_expire}*86400 <= time;
    }
    my $lc_file;
    if ($may_ftp) {
        $lc_file = eval {
            CPAN::FTP->localize
                    (
                     "authors/id/@$chksumfile",
                     $lc_want,
                     $force,
                    );
        };
        unless ($lc_file) {
            $CPAN::Frontend->myprint("Trying $lc_want.gz\n");
            $chksumfile->[-1] .= ".gz";
            $lc_file = eval {
                CPAN::FTP->localize
                        ("authors/id/@$chksumfile",
                         "$lc_want.gz",
                         1,
                        );
            };
            if ($lc_file) {
                $lc_file =~ s{\.gz(?!\n)\Z}{}; #};
                eval{CPAN::Tarzip->new("$lc_file.gz")->gunzip($lc_file)};
            } else {
                return;
            }
        }
    } else {
        $lc_file = $lc_want;
        # we *could* second-guess and if the user has a file: URL,
        # then we could look there. But on the other hand, if they do
        # have a file: URL, why did they choose to set
        # $CPAN::Config->{show_upload_date} to false?
    }

    # adapted from CPAN::Distribution::CHECKSUM_check_file ;
    $fh = FileHandle->new;
    my($cksum);
    if (open $fh, $lc_file) {
        local($/);
        my $eval = <$fh>;
        $eval =~ s/\015?\012/\n/g;
        close $fh;
        my($compmt) = Safe->new();
        $cksum = $compmt->reval($eval);
        if ($@) {
            rename $lc_file, "$lc_file.bad";
            Carp::confess($@) if $@;
        }
    } elsif ($may_ftp) {
        Carp::carp ("Could not open '$lc_file' for reading.");
    } else {
        # Maybe should warn: "You may want to set show_upload_date to a true value"
        return;
    }
    my(@result,$f);
    for $f (sort keys %$cksum) {
        if (exists $cksum->{$f}{isdir}) {
            if ($recursive) {
                my(@dir) = @$chksumfile;
                pop @dir;
                push @dir, $f, "CHECKSUMS";
                push @result, [ 0, "-", $f ];
                push @result, map {
                    [$_->[0], $_->[1], "$f/$_->[2]"]
                } $self->dir_listing(\@dir,1,$may_ftp);
            } else {
                push @result, [ 0, "-", $f ];
            }
        } else {
            push @result, [
                           ($cksum->{$f}{"size"}||0),
                           $cksum->{$f}{"mtime"}||"---",
                           $f
                          ];
        }
    }
    @result;
}

#-> sub CPAN::Author::reports
sub reports {
    $CPAN::Frontend->mywarn("reports on authors not implemented.
Please file a bugreport if you need this.\n");
}

1;
