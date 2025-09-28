#!/usr/bin/perl

use strict;
use warnings;

use Test::More;

BEGIN {
    eval { require Test::Deep; };
    plan skip_all => 'Fails with Can\'t locate object method "print" via package "IO::File" at t/github_issue_79.t line 33' if $] le 5.012005;
    plan skip_all => 'Need Test::Deep to test' if $@;
    Test::Deep->import('cmp_deeply');
}

{
package DumpAsXML::Enh;

use parent 'Pod::Simple::DumpAsXML';

sub new {
    my ( $class ) = @_;
    my $self = $class->SUPER::new();
    $self->code_handler( sub { pop( @_ )->_handle_line( 'code', @_ ); } );
    $self->cut_handler( sub { pop( @_ )->_handle_line( 'cut', @_ ); } );
    $self->pod_handler( sub { pop( @_ )->_handle_line( 'pod', @_ ); } );
    $self->whiteline_handler( sub { pop( @_ )->_handle_line( 'white', @_ ); } );
    return $self;
};

sub _handle_line {
    my ( $self, $elem, $text, $line ) = @_;
    my $fh = $self->{ output_fh };
    $fh->print( '  ' x $self->{ indent }, "<$elem start_line=\"$line\"/>\n" );
};

}

my $output = '';
my $parser = DumpAsXML::Enh->new();
$parser->output_string( \$output );

my $input = [
    '=head1 DESCRIPTION',
    '',
    '    Verbatim paragraph.',
    '',
    '=cut',
];
my $expected_output = [
    '<Document start_line="1">',
    '  <head1 start_line="1">',
    '    DESCRIPTION',
    '  </head1>',
    '  <VerbatimFormatted start_line="3" xml:space="preserve">',
    '        Verbatim paragraph.',
    '  </VerbatimFormatted>',
    '  <cut start_line="5"/>',
    '</Document>',
];

$parser->parse_lines( @$input, undef );

my $actual_output = [ split( "\n", $output ) ];
cmp_deeply( $actual_output, $expected_output ) or do {
    diag( 'actual output:' );
    diag( "|$_" ) for @$actual_output;
    diag( 'expected output:' );
    diag( "|$_" ) for @$expected_output;
};

done_testing;
exit( 0 );

