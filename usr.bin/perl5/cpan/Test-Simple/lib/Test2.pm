package Test2;
use strict;
use warnings;

our $VERSION = '1.302194';


1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2 - Framework for writing test tools that all work together.

=head1 DESCRIPTION

Test2 is a new testing framework produced by forking L<Test::Builder>,
completely refactoring it, adding many new features and capabilities.

=head2 WHAT IS NEW?

=over 4

=item Easier to test new testing tools.

From the beginning Test2 was built with introspection capabilities. With
Test::Builder it was difficult at best to capture test tool output for
verification. Test2 Makes it easy with C<Test2::API::intercept()>.

=item Better diagnostics capabilities.

Test2 uses an L<Test2::API::Context> object to track filename, line number, and
tool details. This object greatly simplifies tracking for where errors should
be reported.

=item Event driven.

Test2 based tools produce events which get passed through a processing system
before being output by a formatter. This event system allows for rich plugin
and extension support.

=item More complete API.

Test::Builder only provided a handful of methods for generating lines of TAP.
Test2 took inventory of everything people were doing with Test::Builder that
required hacking it up. Test2 made public API functions for nearly all the
desired functionality people didn't previously have.

=item Support for output other than TAP.

Test::Builder assumed everything would end up as TAP. Test2 makes no such
assumption. Test2 provides ways for you to specify alternative and custom
formatters.

=item Subtest implementation is more sane.

The Test::Builder implementation of subtests was certifiably insane. Test2 uses
a stacked event hub system that greatly improves how subtests are implemented.

=item Support for threading/forking.

Test2 support for forking and threading can be turned on using L<Test2::IPC>.
Once turned on threading and forking operate sanely and work as one would
expect.

=back

=head1 GETTING STARTED

If you are interested in writing tests using new tools then you should look at
L<Test2::Suite>. L<Test2::Suite> is a separate cpan distribution that contains
many tools implemented on Test2.

If you are interested in writing new tools you should take a look at
L<Test2::API> first.

=head1 NAMESPACE LAYOUT

This describes the namespace layout for the Test2 ecosystem. Not all the
namespaces listed here are part of the Test2 distribution, some are implemented
in L<Test2::Suite>.

=head2 Test2::Tools::

This namespace is for sets of tools. Modules in this namespace should export
tools like C<ok()> and C<is()>. Most things written for Test2 should go here.
Modules in this namespace B<MUST NOT> export subs from other tools. See the
L</Test2::Bundle::> namespace if you want to do that.

=head2 Test2::Plugin::

This namespace is for plugins. Plugins are modules that change or enhance the
behavior of Test2. An example of a plugin is a module that sets the encoding to
utf8 globally. Another example is a module that causes a bail-out event after
the first test failure.

=head2 Test2::Bundle::

This namespace is for bundles of tools and plugins. Loading one of these may
load multiple tools and plugins. Modules in this namespace should not implement
tools directly. In general modules in this namespace should load tools and
plugins, then re-export things into the consumers namespace.

=head2 Test2::Require::

This namespace is for modules that cause a test to be skipped when conditions
do not allow it to run. Examples would be modules that skip the test on older
perls, or when non-essential modules have not been installed.

=head2 Test2::Formatter::

Formatters live under this namespace. L<Test2::Formatter::TAP> is the only
formatter currently. It is acceptable for third party distributions to create
new formatters under this namespace.

=head2 Test2::Event::

Events live under this namespace. It is considered acceptable for third party
distributions to add new event types in this namespace.

=head2 Test2::Hub::

Hub subclasses (and some hub utility objects) live under this namespace. It is
perfectly reasonable for third party distributions to add new hub subclasses in
this namespace.

=head2 Test2::IPC::

The IPC subsystem lives in this namespace. There are not many good reasons to
add anything to this namespace, with exception of IPC drivers.

=head3 Test2::IPC::Driver::

IPC drivers live in this namespace. It is fine to create new IPC drivers and to
put them in this namespace.

=head2 Test2::Util::

This namespace is for general utilities used by testing tools. Please be
considerate when adding new modules to this namespace.

=head2 Test2::API::

This is for Test2 API and related packages.

=head2 Test2::

The Test2:: namespace is intended for extensions and frameworks. Tools,
Plugins, etc should not go directly into this namespace. However extensions
that are used to build tools and plugins may go here.

In short: If the module exports anything that should be run directly by a test
script it should probably NOT go directly into C<Test2::XXX>.

=head1 SEE ALSO

L<Test2::API> - Primary API functions.

L<Test2::API::Context> - Detailed documentation of the context object.

L<Test2::IPC> - The IPC system used for threading/fork support.

L<Test2::Formatter> - Formatters such as TAP live here.

L<Test2::Event> - Events live in this namespace.

L<Test2::Hub> - All events eventually funnel through a hub. Custom hubs are how
C<intercept()> and C<run_subtest()> are implemented.

=head1 CONTACTING US

Many Test2 developers and users lurk on L<irc://irc.perl.org/#perl-qa> and
L<irc://irc.perl.org/#toolchain>. We also have a slack team that can be joined
by anyone with an C<@cpan.org> email address L<https://perl-test2.slack.com/>
If you do not have an C<@cpan.org> email you can ask for a slack invite by
emailing Chad Granum E<lt>exodist@cpan.orgE<gt>.

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
