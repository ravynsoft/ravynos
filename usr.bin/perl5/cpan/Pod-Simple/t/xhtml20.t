#!/usr/bin/perl -w

# t/xhtml20.t - test subclassing of Pod::Simple::XHTML

use strict;
use warnings;
use Test::More tests => 1;

BEGIN {
    package MyXHTML;
    use base 'Pod::Simple::XHTML';

    sub handle_code {
	my($self, $code, $kind) = @_;
	$code = $kind . "[$code]";
	$self->SUPER::handle_code($code);
    }

    sub start_code {
	my($self, $kind) = @_;
	$self->{scratch} .= "<code class=\"$kind\">";
    }

    sub end_code {
	my($self, $kind) = @_;
	$self->{scratch} .= "</code><!-- $kind -->";
    }
}



my ($parser, $results);

initialize();
$parser->parse_string_document(<<'EOT');
=head1 Foo

This is C<$code> and so is:

  my $foo = 1;

Code might even be C<<< nested( B<< C<1> >> ) >>>.
EOT

is($results, <<'EOT');
<h1 id="Foo">Foo</h1>

<p>This is <code class="C">C[$code]</code><!-- C --> and so is:</p>

<pre><code class="Verbatim">Verbatim[  my $foo = 1;]</code><!-- Verbatim --></pre>

<p>Code might even be <code class="C">C[nested( ]<b><code class="C">C[1]</code><!-- C --></b>C[ )]</code><!-- C -->.</p>

EOT


sub initialize {
    $parser = MyXHTML->new;
    $parser->html_header('');
    $parser->html_footer('');
    $parser->output_string( \$results );
    $results = '';
}
