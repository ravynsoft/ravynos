#!/usr/bin/perl
#
# Tests for Pod::ParseLink.
#
# Copyright 2001, 2009, 2018, 2020, 2022 by Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 28;

BEGIN {
    use_ok('Pod::ParseLink');
}

# The format of each entry in this array is the L<> text followed by the
# five-element parse returned by parselink.
#<<<
our @TESTS = (
    ['foo'           => (undef, 'foo',              'foo', undef,     'pod')],
    ['foo|bar'       => ('foo', 'foo',              'bar', undef,     'pod')],
    ['foo/bar'       => (undef, '"bar" in foo',     'foo', 'bar',     'pod')],
    ['foo/"baz boo"' => (undef, '"baz boo" in foo', 'foo', 'baz boo', 'pod')],
    ['/bar'          => (undef, '"bar"',            undef, 'bar',     'pod')],
    ['/"baz boo"'    => (undef, '"baz boo"',        undef, 'baz boo', 'pod')],
    ['/baz boo'      => (undef, '"baz boo"',        undef, 'baz boo', 'pod')],
    [
        'foo bar/baz boo' =>
          (undef, '"baz boo" in foo bar', 'foo bar', 'baz boo', 'pod'),
    ],
    [
        'foo bar  /  baz boo' =>
          (undef, '"baz boo" in foo bar', 'foo bar', 'baz boo', 'pod'),
    ],
    [
        "foo\nbar\nbaz\n/\nboo" =>
          (undef, '"boo" in foo bar baz', 'foo bar baz', 'boo', 'pod'),
    ],
    ['anchor|name/section' => qw(anchor anchor name section pod)],
    ['"boo var baz"' => (undef, '"boo var baz"', undef, 'boo var baz', 'pod')],
    ['bar baz'       => (undef, '"bar baz"', undef, 'bar baz', 'pod')],
    [
        '"boo bar baz / baz boo"' => (
            undef, '"boo bar baz / baz boo"',
            undef, 'boo bar baz / baz boo',
            'pod',
        ),
    ],
    ['fooZ<>bar' => (undef, 'fooZ<>bar', 'fooZ<>bar', undef, 'pod')],
    [
        'Testing I<italics>|foo/bar' =>
          ('Testing I<italics>', 'Testing I<italics>', 'foo', 'bar', 'pod'),
    ],
    [
        'foo/I<Italic> text' =>
          (undef, '"I<Italic> text" in foo', 'foo', 'I<Italic> text', 'pod'),
    ],
    [
        'fooE<verbar>barZ<>/Section C<with> I<B<other> markup' => (
            undef,
            '"Section C<with> I<B<other> markup" in fooE<verbar>barZ<>',
            'fooE<verbar>barZ<>',
            'Section C<with> I<B<other> markup',
            'pod',
        ),
    ],
    [
        'Nested L<http://www.perl.org/>|fooE<sol>bar' => (
            'Nested L<http://www.perl.org/>',
            'Nested L<http://www.perl.org/>',
            'fooE<sol>bar', undef, 'pod',
        ),
    ],
    ['ls(1)' => (undef, 'ls(1)', 'ls(1)', undef, 'man')],
    [
        '  perlfunc(1)/open  ' =>
          (undef, '"open" in perlfunc(1)', 'perlfunc(1)', 'open', 'man'),
    ],
    [
        'some manual page|perl(1)' =>
          ('some manual page', 'some manual page', 'perl(1)', undef, 'man'),
    ],
    [
        'http://www.perl.org/' => (
            undef, 'http://www.perl.org/', 'http://www.perl.org/', undef,
            'url',
        ),
    ],
    [
        'news:yld72axzc8.fsf@windlord.stanford.edu' => (
            undef,
            'news:yld72axzc8.fsf@windlord.stanford.edu',
            'news:yld72axzc8.fsf@windlord.stanford.edu',
            undef, 'url',
        ),
    ],
    [
        'link|http://www.perl.org/' =>
          ('link', 'link', 'http://www.perl.org/', undef, 'url'),
    ],
    [
        '0|http://www.perl.org/' =>
          ('0', '0', 'http://www.perl.org/', undef, 'url'),
    ],
    ['0|Pod::Parser' => ('0', '0', 'Pod::Parser', undef, 'pod')],
);
#>>>

# Run all of the tests.
for my $test (@TESTS) {
    my ($link, @expected) = @$test;
    my @results = parselink($link);
    my $pretty = $link;
    $pretty =~ s{\n}{\\n}xmsg;
    is_deeply(\@results, \@expected, $pretty);
}
