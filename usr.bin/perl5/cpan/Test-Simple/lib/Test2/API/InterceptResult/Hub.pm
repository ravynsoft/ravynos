package Test2::API::InterceptResult::Hub;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::Hub; our @ISA = qw(Test2::Hub) }
use Test2::Util::HashBase;

sub init {
    my $self = shift;
    $self->SUPER::init();
    $self->{+NESTED} = 0;
}

sub inherit {
    my $self = shift;

    $self->{+NESTED} = 0;
}

sub terminate { }

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::API::InterceptResult::Hub - Hub used by InterceptResult.

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
