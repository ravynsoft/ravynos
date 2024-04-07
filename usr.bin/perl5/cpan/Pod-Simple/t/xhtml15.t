#!/usr/bin/perl -w

# t/xhtml15.t - test compatibility between Pod::Simple::XHTML and
# Pod::Simple::HtmlBatch

use strict;
use warnings;
use Test::More tests => 4;

use_ok('Pod::Simple::XHTML') or exit;

my ($parser, $results);

initialize();
my $style = 'http://amazingpants.com/style.css';
$parser->html_css($style);
$parser->parse_string_document( '=head1 Foo' );
like $results, qr/ href="$style" /, 'CSS is correct when link is passed in';

initialize();
my $link = qq{<link rel="stylesheet" href="$style" type="text/css">};
$parser->html_css($link);
$parser->parse_string_document( '=head1 Foo' );
like $results, qr/ href="$style" /, 'CSS is correct when <link> is passed in';

#note('These methods are called when XHTML is used by HtmlBatch');
can_ok $parser, qw/batch_mode_page_object_init html_header_after_title/;

sub initialize {
    $parser = Pod::Simple::XHTML->new;
    $parser->index(1);
    $parser->output_string( \$results );
    $results = '';
}
