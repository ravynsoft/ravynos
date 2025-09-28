package Test::Builder::TodoDiag;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::Event::Diag; our @ISA = qw(Test2::Event::Diag) }

sub diagnostics { 0 }

sub facet_data {
    my $self = shift;
    my $out = $self->SUPER::facet_data();
    $out->{info}->[0]->{debug} = 0;
    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test::Builder::TodoDiag - Test::Builder subclass of Test2::Event::Diag

=head1 DESCRIPTION

This is used to encapsulate diag messages created inside TODO.

=head1 SYNOPSIS

You do not need to use this directly.

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
