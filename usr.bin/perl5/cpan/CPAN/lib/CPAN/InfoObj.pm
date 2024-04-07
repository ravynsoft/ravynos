# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::InfoObj;
use strict;

use CPAN::Debug;
@CPAN::InfoObj::ISA = qw(CPAN::Debug);

use Cwd qw(chdir);

use vars qw(
            $VERSION
);
$VERSION = "5.5";

sub ro {
    my $self = shift;
    exists $self->{RO} and return $self->{RO};
}

#-> sub CPAN::InfoObj::cpan_userid
sub cpan_userid {
    my $self = shift;
    my $ro = $self->ro;
    if ($ro) {
        return $ro->{CPAN_USERID} || "N/A";
    } else {
        $self->debug("ID[$self->{ID}]");
        # N/A for bundles found locally
        return "N/A";
    }
}

sub id { shift->{ID}; }

#-> sub CPAN::InfoObj::new ;
sub new {
    my $this = bless {}, shift;
    %$this = @_;
    $this
}

# The set method may only be used by code that reads index data or
# otherwise "objective" data from the outside world. All session
# related material may do anything else with instance variables but
# must not touch the hash under the RO attribute. The reason is that
# the RO hash gets written to Metadata file and is thus persistent.

#-> sub CPAN::InfoObj::safe_chdir ;
sub safe_chdir {
  my($self,$todir) = @_;
  # we die if we cannot chdir and we are debuggable
  Carp::confess("safe_chdir called without todir argument")
        unless defined $todir and length $todir;
  if (chdir $todir) {
    $self->debug(sprintf "changed directory to %s", CPAN::anycwd())
        if $CPAN::DEBUG;
  } else {
    if (-e $todir) {
        unless (-x $todir) {
            unless (chmod 0755, $todir) {
                my $cwd = CPAN::anycwd();
                $CPAN::Frontend->mywarn("I have neither the -x permission nor the ".
                                        "permission to change the permission; cannot ".
                                        "chdir to '$todir'\n");
                $CPAN::Frontend->mysleep(5);
                $CPAN::Frontend->mydie(qq{Could not chdir from cwd[$cwd] }.
                                       qq{to todir[$todir]: $!});
            }
        }
    } else {
        $CPAN::Frontend->mydie("Directory '$todir' has gone. Cannot continue.\n");
    }
    if (chdir $todir) {
      $self->debug(sprintf "changed directory to %s", CPAN::anycwd())
          if $CPAN::DEBUG;
    } else {
      my $cwd = CPAN::anycwd();
      $CPAN::Frontend->mydie(qq{Could not chdir from cwd[$cwd] }.
                             qq{to todir[$todir] (a chmod has been issued): $!});
    }
  }
}

#-> sub CPAN::InfoObj::set ;
sub set {
    my($self,%att) = @_;
    my $class = ref $self;

    # This must be ||=, not ||, because only if we write an empty
    # reference, only then the set method will write into the readonly
    # area. But for Distributions that spring into existence, maybe
    # because of a typo, we do not like it that they are written into
    # the readonly area and made permanent (at least for a while) and
    # that is why we do not "allow" other places to call ->set.
    unless ($self->id) {
        CPAN->debug("Bug? Empty ID, rejecting");
        return;
    }
    my $ro = $self->{RO} =
        $CPAN::META->{readonly}{$class}{$self->id} ||= {};

    while (my($k,$v) = each %att) {
        $ro->{$k} = $v;
    }
}

#-> sub CPAN::InfoObj::as_glimpse ;
sub as_glimpse {
    my($self) = @_;
    my(@m);
    my $class = ref($self);
    $class =~ s/^CPAN:://;
    my $id = $self->can("pretty_id") ? $self->pretty_id : $self->{ID};
    push @m, sprintf "%-15s %s\n", $class, $id;
    join "", @m;
}

#-> sub CPAN::InfoObj::as_string ;
sub as_string {
    my($self) = @_;
    my(@m);
    my $class = ref($self);
    $class =~ s/^CPAN:://;
    push @m, $class, " id = $self->{ID}\n";
    my $ro;
    unless ($ro = $self->ro) {
        if (substr($self->{ID},-1,1) eq ".") { # directory
            $ro = +{};
        } else {
            $CPAN::Frontend->mywarn("Unknown object $self->{ID}\n");
            $CPAN::Frontend->mysleep(5);
            return;
        }
    }
    for (sort keys %$ro) {
        # next if m/^(ID|RO)$/;
        my $extra = "";
        if ($_ eq "CPAN_USERID") {
            $extra .= " (";
            $extra .= $self->fullname;
            my $email; # old perls!
            if ($email = $CPAN::META->instance("CPAN::Author",
                                               $self->cpan_userid
                                              )->email) {
                $extra .= " <$email>";
            } else {
                $extra .= " <no email>";
            }
            $extra .= ")";
        } elsif ($_ eq "FULLNAME") { # potential UTF-8 conversion
            push @m, sprintf "    %-12s %s\n", $_, $self->fullname;
            next;
        }
        next unless defined $ro->{$_};
        push @m, sprintf "    %-12s %s%s\n", $_, $ro->{$_}, $extra;
    }
  KEY: for (sort keys %$self) {
        next if m/^(ID|RO)$/;
        unless (defined $self->{$_}) {
            delete $self->{$_};
            next KEY;
        }
        if (ref($self->{$_}) eq "ARRAY") {
            push @m, sprintf "    %-12s %s\n", $_, "@{$self->{$_}}";
        } elsif (ref($self->{$_}) eq "HASH") {
            my $value;
            if (/^CONTAINSMODS$/) {
                $value = join(" ",sort keys %{$self->{$_}});
            } elsif (/^prereq_pm$/) {
                my @value;
                my $v = $self->{$_};
                for my $x (sort keys %$v) {
                    my @svalue;
                    for my $y (sort keys %{$v->{$x}}) {
                        push @svalue, "$y=>$v->{$x}{$y}";
                    }
                    push @value, "$x\:" . join ",", @svalue if @svalue;
                }
                $value = join ";", @value;
            } else {
                $value = $self->{$_};
            }
            push @m, sprintf(
                             "    %-12s %s\n",
                             $_,
                             $value,
                            );
        } else {
            push @m, sprintf "    %-12s %s\n", $_, $self->{$_};
        }
    }
    join "", @m, "\n";
}

#-> sub CPAN::InfoObj::fullname ;
sub fullname {
    my($self) = @_;
    $CPAN::META->instance("CPAN::Author",$self->cpan_userid)->fullname;
}

#-> sub CPAN::InfoObj::dump ;
sub dump {
    my($self, $what) = @_;
    unless ($CPAN::META->has_inst("Data::Dumper")) {
        $CPAN::Frontend->mydie("dump command requires Data::Dumper installed");
    }
    local $Data::Dumper::Sortkeys;
    $Data::Dumper::Sortkeys = 1;
    my $out = Data::Dumper::Dumper($what ? eval $what : $self);
    if (length $out > 100000) {
        my $fh_pager = FileHandle->new;
        local($SIG{PIPE}) = "IGNORE";
        my $pager = $CPAN::Config->{'pager'} || "cat";
        $fh_pager->open("|$pager")
            or die "Could not open pager $pager\: $!";
        $fh_pager->print($out);
        close $fh_pager;
    } else {
        $CPAN::Frontend->myprint($out);
    }
}

1;
