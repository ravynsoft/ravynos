#!/usr/bin/perl

# This tests Pod::Checker::Hyperlink

use Test::More;
use Pod::Checker;

my @answers = (
                {
                 'line' => 12,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod'
                },
                {
                 'line' => 14,
                 'node' => 'section',
                 'page' => '"manpage"',
                 'type' => 'pod',
                },
                {
                 'line' => 16,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 20,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 22,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 24,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 26,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 28,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                 'line' => 30,
                 'node' => 'section',
                 'page' => 'manpage',
                 'type' => 'pod',
                },
                {
                  'line' => 36,
                  'node' => '',
                  'page' => 'foo',
                  'type' => 'pod',
                },
                {
                  'line' => 38,
                  'node' => '',
                  'page' => 'bar',
                  'type' => 'pod'
                },
                {
                  'line' => 40,
                  'node' => 'bar',
                  'page' => 'foo',
                  'type' => 'pod'
                },
                {
                  'line' => 42,
                  'node' => 'baz boo',
                  'page' => 'foo',
                  'type' => 'pod'
                },
                {
                  'line' => 50,
                  'node' => 'baz boo',
                  'page' => 'foo bar',
                  'type' => 'pod',
                },
                {
                  'line' => 59,
                  'node' => '',
                  'page' => 'foobar',
                  'type' => 'pod',
                },
                {
                  'line' => 61,
                  'node' => 'bar',
                  'page' => 'foo',
                  'type' => 'pod'
                },
                {
                  'line' => 63,
                  'node' => 'Italic text',
                  'page' => 'foo',
                  'type' => 'pod'
                },
                {
                  'line' => 65,
                  'node' => 'Section with other markup',
                  'page' => 'foo|bar',
                  'type' => 'pod',
                },
                {
                  'line' => 67,
                  'node' => '',
                  'page' => 'chmod',
                  'type' => 'pod',
                },
                {
                  'line' => 69,
                  'node' => '',
                  'page' => 'chmod(2)',
                  'type' => 'man',
                },
                {
                  'line' => 71,
                  'node' => '',
                  'page' => 'chmod(2)',
                  'type' => 'man',
                },
                {
                  'line' => 73,
                  'node' => '',
                  'page' => 'chmod()',
                  'type' => 'pod',
                },
                {
                  'line' => 75,
                  'node' => '',
                  'page' => 'mailto:foo@cpan.org',
                  'type' => 'url',
                },
                {
                  'line' => 77,
                  'node' => '',
                  'page' => 'mailto:foo@cpan.org',
                  'type' => 'url',
                },
                {
                  'line' => 79,
                  'node' => '',
                  'page' => 'http://www.perl.org',
                  'type' => 'url',
                },
                {
                  'line' => 81,
                  'node' => '',
                  'page' => 'http://www.perl.org',
                  'type' => 'url',
                },
            );

plan 'tests' => @answers * 4 + 2;

my $checker = Pod::Checker->new( '-quiet' => 1);
$checker->parse_from_file(\*DATA);

is($checker->num_warnings, 0, "There were no warnings found");
is($checker->num_errors, 0, "There were no errors found");

my @links = $checker->hyperlinks;

for my $i (0 .. @links - 1) {
    is($links[$i]->line(), $answers[$i]->{'line'}, "line() returns '$answers[$i]->{'line'}' correctly");
    is($links[$i]->node(), $answers[$i]->{'node'}, "node() returns '$answers[$i]->{'node'}' correctly");
    is($links[$i]->page(), $answers[$i]->{'page'}, "page() returns '$answers[$i]->{'page'}' correctly");
    is($links[$i]->type(), $answers[$i]->{'type'}, "type() returns '$answers[$i]->{'type'}' correctly");
}

__END__

=head1 NAME

basic.pod - Extracted and expanded from podlators; test various link types

=head1 LINKS

These are all taken from the Pod::Parser tests.

Try out I<LOTS> of different ways of specifying references:

Reference the L<manpage/section>

Reference the L<"manpage"/section>

Reference the L<manpage/"section">

Now try it using the new "|" stuff ...

Reference the L<thistext|manpage/section>|

Reference the L<thistext | manpage / section>|

Reference the L<thistext| manpage/ section>|

Reference the L<thistext |manpage /section>|

Reference the L<thistext|manpage/"section">|

Reference the L<thistext|
manpage/
section>|

And then throw in a few new ones of my own.

L<foo>

L<foo|bar>

L<foo/bar>

L<foo/"baz boo">

L</bar> won't show up because is a link to this page

L</"baz boo"> won't show up because is a link to this page

L</baz boo> won't show up because is a link to this page

L<foo bar/baz boo>

L<"boo var baz"> won't show up because the quotes make it a link to this page

L<bar baz> won't show up because of blanks (deprecated) make it a link to this
page

L</boo>, L</bar>, and L</baz> won't show up because are links to this page

L<fooZ<>bar>

L<Testing I<italics>|foo/bar>

L<foo/I<Italic> text>

L<fooE<verbar>barZ<>/Section C<with> I<B<other> markup>>

L<chmod>

L<chmod(2)>

L<man page with text|chmod(2)>

L<chmod()>

L<mailto:foo@cpan.org>

L<Don't email us|mailto:foo@cpan.org>

L<http://www.perl.org>

L<hyperlink|http://www.perl.org>

=head1 bar

=head2 baz boo

=head3 boo var baz

=head4 bar baz

=cut
