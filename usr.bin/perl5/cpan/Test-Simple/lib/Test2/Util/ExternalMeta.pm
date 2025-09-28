package Test2::Util::ExternalMeta;
use strict;
use warnings;

our $VERSION = '1.302194';


use Carp qw/croak/;

sub META_KEY() { '_meta' }

our @EXPORT = qw/meta set_meta get_meta delete_meta/;
BEGIN { require Exporter; our @ISA = qw(Exporter) }

sub set_meta {
    my $self = shift;
    my ($key, $value) = @_;

    validate_key($key);

    $self->{+META_KEY} ||= {};
    $self->{+META_KEY}->{$key} = $value;
}

sub get_meta {
    my $self = shift;
    my ($key) = @_;

    validate_key($key);

    my $meta = $self->{+META_KEY} or return undef;
    return $meta->{$key};
}

sub delete_meta {
    my $self = shift;
    my ($key) = @_;

    validate_key($key);

    my $meta = $self->{+META_KEY} or return undef;
    delete $meta->{$key};
}

sub meta {
    my $self = shift;
    my ($key, $default) = @_;

    validate_key($key);

    my $meta = $self->{+META_KEY};
    return undef unless $meta || defined($default);

    unless($meta) {
        $meta = {};
        $self->{+META_KEY} = $meta;
    }

    $meta->{$key} = $default
        if defined($default) && !defined($meta->{$key});

    return $meta->{$key};
}

sub validate_key {
    my $key = shift;

    return if $key && !ref($key);

    my $render_key = defined($key) ? "'$key'" : 'undef';
    croak "Invalid META key: $render_key, keys must be true, and may not be references";
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Util::ExternalMeta - Allow third party tools to safely attach meta-data
to your instances.

=head1 DESCRIPTION

This package lets you define a clear, and consistent way to allow third party
tools to attach meta-data to your instances. If your object consumes this
package, and imports its methods, then third party meta-data has a safe place
to live.

=head1 SYNOPSIS

    package My::Object;
    use strict;
    use warnings;

    use Test2::Util::ExternalMeta qw/meta get_meta set_meta delete_meta/;

    ...

Now to use it:

    my $inst = My::Object->new;

    $inst->set_meta(foo => 'bar');
    my $val = $inst->get_meta('foo');

=head1 WHERE IS THE DATA STORED?

This package assumes your instances are blessed hashrefs, it will not work if
that is not true. It will store all meta-data in the C<_meta> key on your
objects hash. If your object makes use of the C<_meta> key in its underlying
hash, then there is a conflict and you cannot use this package.

=head1 EXPORTS

=over 4

=item $val = $obj->meta($key)

=item $val = $obj->meta($key, $default)

This will get the value for a specified meta C<$key>. Normally this will return
C<undef> when there is no value for the C<$key>, however you can specify a
C<$default> value to set when no value is already set.

=item $val = $obj->get_meta($key)

This will get the value for a specified meta C<$key>. This does not have the
C<$default> overhead that C<meta()> does.

=item $val = $obj->delete_meta($key)

This will remove the value of a specified meta C<$key>. The old C<$val> will be
returned.

=item $obj->set_meta($key, $val)

Set the value of a specified meta C<$key>.

=back

=head1 META-KEY RESTRICTIONS

Meta keys must be defined, and must be true when used as a boolean. Keys may
not be references. You are free to stringify a reference C<"$ref"> for use as a
key, but this package will not stringify it for you.

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
