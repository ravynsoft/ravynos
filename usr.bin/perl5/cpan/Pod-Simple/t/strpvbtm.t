#!/usr/bin/perl -w

# t/strip_verbatim_indent.t.t - check verbatim indent stripping feature

BEGIN {
    chdir 't' if -d 't';
}

use strict;
use warnings;
use lib '../lib';
use Test::More tests => 103;
#use Test::More 'no_plan';

use_ok('Pod::Simple::XHTML') or exit;
use_ok('Pod::Simple::XMLOutStream') or exit;

isa_ok my $parser = Pod::Simple::XHTML->new, 'Pod::Simple::XHTML';

ok $parser->strip_verbatim_indent(' '), 'Should be able to set striper to " "';
ok $parser->strip_verbatim_indent('    '), 'Should be able to set striper to "    "';
ok $parser->strip_verbatim_indent("t"), 'Should be able to set striper to "\\t"';
ok $parser->strip_verbatim_indent(sub { ' ' }), 'Should be able to set striper to coderef';

for my $spec (
    [
        "\n=pod\n\n foo bar baz\n",
        undef,
        qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz</Verbatim></Document>},
        "<pre><code> foo bar baz</code></pre>\n\n",
        'undefined indent'
    ],
    [
        "\n=pod\n\n foo bar baz\n",
        ' ',
        qq{<Document><Verbatim\nxml:space="preserve">foo bar baz</Verbatim></Document>},
        "<pre><code>foo bar baz</code></pre>\n\n",
        'single space indent'
    ],
    [
        "\n=pod\n\n foo bar baz\n",
        '  ',
        qq{<Document><Verbatim\nxml:space="preserve"> foo bar baz</Verbatim></Document>},
        "<pre><code> foo bar baz</code></pre>\n\n",
        'too large indent'
    ],
    [
        "\n=pod\n\n  foo bar baz\n",
        '  ',
        qq{<Document><Verbatim\nxml:space="preserve">foo bar baz</Verbatim></Document>},
        "<pre><code>foo bar baz</code></pre>\n\n",
        'double space indent'
    ],
    [
        "\n=pod\n\n  foo bar baz\n",
        sub { '  ' },
        qq{<Document><Verbatim\nxml:space="preserve">foo bar baz</Verbatim></Document>},
        "<pre><code>foo bar baz</code></pre>\n\n",
        'code ref stripper'
    ],
    [
        "\n=pod\n\n foo bar\n\n baz blez\n",
        ' ',
        qq{<Document><Verbatim\nxml:space="preserve">foo bar\n\nbaz blez</Verbatim></Document>},
        "<pre><code>foo bar\n\nbaz blez</code></pre>\n\n",
        'single space indent and empty line'
    ],
    [
        "\n=pod\n\n foo bar\n\n baz blez\n",
        sub { ' ' },
        qq{<Document><Verbatim\nxml:space="preserve">foo bar\n\nbaz blez</Verbatim></Document>},
        "<pre><code>foo bar\n\nbaz blez</code></pre>\n\n",
        'code ref indent and empty line'
    ],
    [
        "\n=pod\n\n foo bar\n\n baz blez\n",
        sub { (my $s = shift->[0]) =~ s/\S.*//; $s },
        qq{<Document><Verbatim\nxml:space="preserve">foo bar\n\nbaz blez</Verbatim></Document>},
        "<pre><code>foo bar\n\nbaz blez</code></pre>\n\n",
        'heuristic code ref indent'
    ],
    [
        "\n=pod\n\n foo bar\n   baz blez\n",
        sub { s/^\s+// for @{ $_[0] } },
        qq{<Document><Verbatim\nxml:space="preserve">foo bar\nbaz blez</Verbatim></Document>},
        "<pre><code>foo bar\nbaz blez</code></pre>\n\n",
        'militant code ref'
    ],
    [
        "\n=pod\n\n foo (bar\n   baz blez\n",
        sub { (my $i = $_[0]->[0]) =~ s/S.*//; $i },
        qq{<Document><Verbatim\nxml:space="preserve">\n   baz blez</Verbatim></Document>},
        "<pre><code>\n   baz blez</code></pre>\n\n",
        'code ref and paren'
    ],
) {
    my ($pod, $indent, $xml, $xhtml, $desc) = @$spec;
    # Test XML output.
    ok my $p = Pod::Simple::XMLOutStream->new, "Construct XML parser to test $desc";
    $p->hide_line_numbers(1);
    my $output = '';
    $p->output_string( \$output );
    is $indent, $p->strip_verbatim_indent($indent),
        'Set stripper for XML to ' . (defined $indent ? qq{"$indent"} : 'undef');
    ok $p->parse_string_document( $pod ), "Parse POD to XML for $desc";
    is $output, $xml, "Should have expected XML output for $desc";


    # Test XHTML output.
    ok $p = Pod::Simple::XHTML->new, "Construct XHMTL parser to test $desc";
    $p->html_header('');
    $p->html_footer('');
    $output = '';
    $p->output_string( \$output );
    is $indent, $p->strip_verbatim_indent($indent),
        'Set stripper for XHTML to ' . (defined $indent ? qq{"$indent"} : 'undef');
    ok $p->parse_string_document( $pod ), "Parse POD to XHTML for $desc";
    is $output, $xhtml, "Should have expected XHTML output for $desc";
}

for my $spec (
    [
        "\n=pod\n\n\t\tfoo bar baz\n",
        0,
        "<pre><code>\t\tfoo bar baz</code></pre>\n\n",
        'preserve tabs'
    ],
    [
        "\n=pod\n\n\t\tfoo bar baz\n",
        undef,
        "<pre><code>                foo bar baz</code></pre>\n\n",
        'preserve tabs'
    ],
    [
        "\n=pod\n\n\t\tfoo bar baz\n",
        -1,
        "<pre><code>                foo bar baz</code></pre>\n\n",
        'preserve tabs'
    ],
    [
        "\n=pod\n\n\t\tfoo bar baz\n",
        1,
        "<pre><code>  foo bar baz</code></pre>\n\n",
        'tabs are xlate to one space each'
    ],
) {
    my ($pod, $tabs, $xhtml, $desc) = @$spec;
    # Test XHTML output.
    ok my $p = Pod::Simple::XHTML->new, "Construct XHMTL parser to test $desc";
    $p->html_header('');
    $p->html_footer('');
    my $output = '';
    $p->output_string( \$output );
    is $tabs, $p->expand_verbatim_tabs($tabs),
        'Set tab  for XHTML to ' . (defined $tabs ? qq{"$tabs"} : 'undef');
    ok $p->parse_string_document( $pod ), "Parse POD to XHTML for $desc";
    is $output, $xhtml, "Should have expected XHTML output for $desc";
}
