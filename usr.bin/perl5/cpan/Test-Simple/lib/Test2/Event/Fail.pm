package Test2::Event::Fail;
use strict;
use warnings;

our $VERSION = '1.302194';

use Test2::EventFacet::Info;

BEGIN {
    require Test2::Event;
    our @ISA = qw(Test2::Event);
    *META_KEY = \&Test2::Util::ExternalMeta::META_KEY;
}

use Test2::Util::HashBase qw{ -name -info };

#############
# Old API
sub summary          { "fail" }
sub increments_count { 1 }
sub diagnostics      { 0 }
sub no_display       { 0 }
sub subtest_id       { undef }
sub terminate        { () }
sub global           { () }
sub sets_plan        { () }

sub causes_fail {
    my $self = shift;
    return 0 if $self->{+AMNESTY} && @{$self->{+AMNESTY}};
    return 1;
}

#############
# New API

sub add_info {
    my $self = shift;

    for my $in (@_) {
        $in = {%$in} if ref($in) ne 'ARRAY';
        $in = Test2::EventFacet::Info->new($in);

        push @{$self->{+INFO}} => $in;
    }
}

sub facet_data {
    my $self = shift;
    my $out = $self->common_facet_data;

    $out->{about}->{details} = 'fail';

    $out->{assert} = {pass => 0, details => $self->{+NAME}};

    $out->{info} = [map {{ %{$_} }} @{$self->{+INFO}}] if $self->{+INFO};

    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Fail - Event for a simple failed assertion

=head1 DESCRIPTION

This is an optimal representation of a failed assertion.

=head1 SYNOPSIS

    use Test2::API qw/context/;

    sub fail {
        my ($name) = @_;
        my $ctx = context();
        $ctx->fail($name);
        $ctx->release;
    }

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
