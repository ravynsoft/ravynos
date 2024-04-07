package Test2::EventFacet::Info::Table;
use strict;
use warnings;

our $VERSION = '1.302194';

use Carp qw/confess/;

use Test2::Util::HashBase qw{-header -rows -collapse -no_collapse -as_string};

sub init {
    my $self = shift;

    confess "Table may not be empty" unless ref($self->{+ROWS}) eq 'ARRAY' && @{$self->{+ROWS}};

    $self->{+AS_STRING} ||= '<TABLE NOT DISPLAYED>';
}

sub as_hash { my $out = +{%{$_[0]}}; delete $out->{as_string}; $out }

sub info_args {
    my $self = shift;

    my $hash = $self->as_hash;
    my $desc = $self->as_string;

    return (table => $hash, details => $desc);
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Info::Table - Intermediary representation of a table.

=head1 DESCRIPTION

Intermediary representation of a table for use in specialized
L<Test::API::Context> methods which generate L<Test2::EventFacet::Info> facets.

=head1 SYNOPSIS

    use Test2::EventFacet::Info::Table;
    use Test2::API qw/context/;

    sub my_tool {
        my $ctx = context();

        ...

        $ctx->fail(
            $name,
            "failure diag message",
            Test2::EventFacet::Info::Table->new(
                # Required
                rows => [['a', 'b'], ['c', 'd'], ...],

                # Strongly Recommended
                as_string => "... string to print when table cannot be rendered ...",

                # Optional
                header => ['col1', 'col2'],
                collapse => $bool,
                no_collapse => ['col1', ...],
            ),
        );

        ...

        $ctx->release;
    }

    my_tool();

=head1 ATTRIBUTES

=over 4

=item $header_aref = $t->header()

=item $rows_aref = $t->rows()

=item $bool = $t->collapse()

=item $aref = $t->no_collapse()

The above are all directly tied to the table hashref structure described in
L<Test2::EventFacet::Info>.

=item $str = $t->as_string()

This returns the string form of the table if it was set, otherwise it returns
the string C<< "<TABLE NOT DISPLAYED>" >>.

=item $href = $t->as_hash()

This returns the data structure used for tables by L<Test2::EventFacet::Info>.

=item %args = $t->info_args()

This returns the arguments that should be used to construct the proper
L<Test2::EventFacet::Info> structure.

    return (table => $t->as_hash(), details => $t->as_string());

=back

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
