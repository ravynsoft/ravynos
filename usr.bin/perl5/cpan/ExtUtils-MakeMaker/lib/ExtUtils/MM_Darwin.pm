package ExtUtils::MM_Darwin;

use strict;
use warnings;

BEGIN {
    require ExtUtils::MM_Unix;
    our @ISA = qw( ExtUtils::MM_Unix );
}

our $VERSION = '7.70';
$VERSION =~ tr/_//d;


=head1 NAME

ExtUtils::MM_Darwin - special behaviors for OS X

=head1 SYNOPSIS

    For internal MakeMaker use only

=head1 DESCRIPTION

See L<ExtUtils::MM_Unix> or L<ExtUtils::MM_Any> for documentation on the
methods overridden here.

=head2 Overridden Methods

=head3 init_dist

Turn off Apple tar's tendency to copy resource forks as "._foo" files.

=cut

sub init_dist {
    my $self = shift;

    # Thank you, Apple, for breaking tar and then breaking the work around.
    # 10.4 wants COPY_EXTENDED_ATTRIBUTES_DISABLE while 10.5 wants
    # COPYFILE_DISABLE.  I'm not going to push my luck and instead just
    # set both.
    $self->{TAR} ||=
        'COPY_EXTENDED_ATTRIBUTES_DISABLE=1 COPYFILE_DISABLE=1 tar';

    $self->SUPER::init_dist(@_);
}

=head3 cflags

Over-ride Apple's automatic setting of -Werror

=cut

sub cflags {
    my($self,$libperl)=@_;
    return $self->{CFLAGS} if $self->{CFLAGS};
    return '' unless $self->needs_linking();

    my $base = $self->SUPER::cflags($libperl);

    foreach (split /\n/, $base) {
        /^(\S*)\s*=\s*(\S*)$/ and $self->{$1} = $2;
    };
    $self->{CCFLAGS} .= " -Wno-error=implicit-function-declaration";

    return $self->{CFLAGS} = qq{
CCFLAGS = $self->{CCFLAGS}
OPTIMIZE = $self->{OPTIMIZE}
PERLTYPE = $self->{PERLTYPE}
};
}

1;
