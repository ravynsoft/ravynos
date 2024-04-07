#!/usr/bin/perl -w

# t/output.t - Check output_string.

BEGIN {
    chdir 't' if -d 't';
}

use strict;
use warnings;
use lib '../lib';
use Test::More tests => 36;
#use Test::More 'no_plan';
use File::Spec;

for my $format (qw(XHTML HTML Text RTF)) {
    my $class = "Pod::Simple::$format";
    use_ok $class or next;
    ok my $parser = $class->new, "Construct $format parser";

    # Try parse_string_document().
    my $output = '';
    ok $parser->output_string(\$output), "Set $format output string";
    ok $parser->parse_string_document( "=head1 Poit!" ),
        "Parse to $format via parse_string_document()";
    like $output, qr{Poit!},
        "Should have $format output from parse_string_document()";

    # Try parse_file().
    ok $parser = $class->new, "Construct another $format parser";
    $output = '';
    ok $parser->output_string(\$output), "Set $format output string again";
    ok $parser->parse_file(File::Spec->catfile(qw(testlib1 zikzik.pod))),
        "Parse to $format via parse_file()";
    like $output, qr{This is just a test file},
        "Should have $format output from parse_file";
}
