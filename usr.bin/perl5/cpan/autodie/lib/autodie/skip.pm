package autodie::skip;
use strict;
use warnings;

our $VERSION = '2.36'; # VERSION

# This package exists purely so people can inherit from it,
# which isn't at all how roles are supposed to work, but it's
# how people will use them anyway.

if ($] < 5.010) {
    # Older Perls don't have a native ->DOES.  Let's provide a cheap
    # imitation here.

    *DOES = sub { return shift->isa(@_); };
}

1;

__END__

=head1 NAME

autodie::skip - Skip a package when throwing autodie exceptions

=head1 SYNPOSIS

    use parent qw(autodie::skip);

=head1 DESCRIPTION

This dummy class exists to signal that the class inheriting it should
be skipped when reporting exceptions from autodie.  This is useful
for utility classes like L<Path::Tiny> that wish to report the location
of where they were called on failure.

If your class has a better way of doing roles, then you should not
load this class and instead simply say that your class I<DOES>
C<autodie::skip> instead.

=head1 AUTHOR

Copyright 2013, Paul Fenwick <pjf@cpan.org>

=head1 LICENSE

This module is free software. You may distribute it under the same
terms as Perl itself.

=head1 SEE ALSO

L<autodie>, L<autodie::exception>

=for Pod::Coverage DOES

=cut
