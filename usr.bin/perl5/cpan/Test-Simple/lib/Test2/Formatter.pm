package Test2::Formatter;
use strict;
use warnings;

our $VERSION = '1.302194';


my %ADDED;
sub import {
    my $class = shift;
    return if $class eq __PACKAGE__;
    return if $ADDED{$class}++;
    require Test2::API;
    Test2::API::test2_formatter_add($class);
}

sub new_root {
    my $class = shift;
    return $class->new(@_);
}

sub supports_tables { 0 }

sub hide_buffered { 1 }

sub terminate { }

sub finalize { }

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Formatter - Namespace for formatters.

=head1 DESCRIPTION

This is the namespace for formatters. This is an empty package.

=head1 CREATING FORMATTERS

A formatter is any package or object with a C<write($event, $num)> method.

    package Test2::Formatter::Foo;
    use strict;
    use warnings;

    sub write {
        my $self_or_class = shift;
        my ($event, $assert_num) = @_;
        ...
    }

    sub hide_buffered { 1 }

    sub terminate { }

    sub finalize { }

    sub supports_tables { return $BOOL }

    sub new_root {
        my $class = shift;
        ...
        $class->new(@_);
    }

    1;

The C<write> method is a method, so it either gets a class or instance. The two
arguments are the C<$event> object it should record, and the C<$assert_num>
which is the number of the current assertion (ok), or the last assertion if
this event is not itself an assertion. The assertion number may be any integer 0
or greater, and may be undefined in some cases.

The C<hide_buffered()> method must return a boolean. This is used to tell
buffered subtests whether or not to send it events as they are being buffered.
See L<Test2::API/"run_subtest(...)"> for more information.

The C<terminate> and C<finalize> methods are optional methods called that you
can implement if the format you're generating needs to handle these cases, for
example if you are generating XML and need close open tags.

The C<terminate> method is called when an event's C<terminate> method returns
true, for example when a L<Test2::Event::Plan> has a C<'skip_all'> plan, or
when a L<Test2::Event::Bail> event is sent. The C<terminate> method is passed
a single argument, the L<Test2::Event> object which triggered the terminate.

The C<finalize> method is always the last thing called on the formatter, I<<
except when C<terminate> is called for a Bail event >>. It is passed the
following arguments:

The C<supports_tables> method should be true if the formatter supports directly
rendering table data from the C<info> facets. This is a newer feature and many
older formatters may not support it. When not supported the formatter falls
back to rendering C<detail> instead of the C<table> data.

The C<new_root> method is used when constructing a root formatter. The default
is to just delegate to the regular C<new()> method, most formatters can ignore
this.

=over 4

=item * The number of tests that were planned

=item * The number of tests actually seen

=item * The number of tests which failed

=item * A boolean indicating whether or not the test suite passed

=item * A boolean indicating whether or not this call is for a subtest

=back

The C<new_root> method is called when C<Test2::API::Stack> Initializes the root
hub for the first time. Most formatters will simply have this call C<<
$class->new >>, which is the default behavior. Some formatters however may want
to take extra action during construction of the root formatter, this is where
they can do that.

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
