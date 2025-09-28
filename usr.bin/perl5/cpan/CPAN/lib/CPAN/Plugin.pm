package CPAN::Plugin;

use strict;
use warnings;

our $VERSION = '0.97';

require CPAN;

######################################################################

sub new {                                # ;
    my ($class, %params) = @_;

    my $self = +{
        (ref $class ? (%$class) : ()),
        %params,
    };

    $self = bless $self, ref $class ? ref $class : $class;

    unless (ref $class) {
        local $_;
        no warnings 'once';
        $CPAN::META->use_inst ($_) for $self->plugin_requires;
    }

    $self;
}

######################################################################
sub plugin_requires {                    # ;
}

######################################################################
sub distribution_object {                # ;
    my ($self) = @_;
    $self->{distribution_object};
}

######################################################################
sub distribution {                       # ;
    my ($self) = @_;

    my $distribution = $self->distribution_object->id;
    CPAN::Shell->expand("Distribution",$distribution)
      or $self->frontend->mydie("Unknowns distribution '$distribution'\n");
}

######################################################################
sub distribution_info {                  # ;
    my ($self) = @_;

    CPAN::DistnameInfo->new ($self->distribution->id);
}

######################################################################
sub build_dir {                          # ;
    my ($self) = @_;

    my $build_dir = $self->distribution->{build_dir}
      or $self->frontend->mydie("Distribution has not been built yet, cannot proceed");
}

######################################################################
sub is_xs {                              #
    my ($self) = @_;

    my @xs = glob File::Spec->catfile ($self->build_dir, '*.xs'); # quick try

    unless (@xs) {
        require ExtUtils::Manifest;
        my $manifest_file = File::Spec->catfile ($self->build_dir, "MANIFEST");
        my $manifest = ExtUtils::Manifest::maniread($manifest_file);
        @xs = grep /\.xs$/, keys %$manifest;
    }

    scalar @xs;
}

######################################################################

package CPAN::Plugin;

1;

__END__

=pod

=head1 NAME

CPAN::Plugin - Base class for CPAN shell extensions

=head1 SYNOPSIS

   package CPAN::Plugin::Flurb;
   use parent 'CPAN::Plugin';

   sub post_test {
     my ($self, $distribution_object) = @_;
     $self = $self->new (distribution_object => $distribution_object);
     ...;
   }

=head1 DESCRIPTION

=head2 Alpha Status

The plugin system in the CPAN shell was introduced in version 2.07 and
is still considered experimental.

=head2 How Plugins work?

See L<CPAN/"Plugin support">.

=head1 METHODS

=head2 plugin_requires

returns list of packages given plugin requires for functionality.
This list is evaluated using C<< CPAN->use_inst >> method.

=head2 distribution_object

Get current distribution object.

=head2 distribution

=head2 distribution_info

=head2 build_dir

Simple delegatees for misc parameters derived from distribution

=head2 is_xs

Predicate to detect whether package contains XS.

=head1 AUTHOR

Branislav Zahradnik <barney@cpan.org>

=cut

