use strict;
use warnings;
use Test::More tests => 6;

{
  package Pod::Simple::ErrorFinder;
  use base 'Pod::Simple::DumpAsXML'; # arbitrary choice -- rjbs, 2013-04-16

  sub errors_for_input {
    my ($class, $input, $mutor) = @_;

    my $parser = $class->new;
    my $output = '';
    $parser->output_string( \$output );
    $parser->no_errata_section(1);
    $parser->parse_string_document( $input );

    return $parser->errata_seen();
  }
}

sub errors { Pod::Simple::ErrorFinder->errors_for_input(@_) }

{
  my $errors = errors("=over 4\n\n=item 1\n\nHey\n\n");
  is_deeply(
    $errors,
    { 1 => [ "=over without closing =back" ] },
    "no closing =back",
  );
}

{
  for my $l_code ('L< foo>', 'L< bar>') {
    my $input = "=pod\n\nAmbiguous space: $l_code\n";
    my $errors = errors("$input");
    is_deeply(
      $errors,
      { 3 => [ "L<> starts or ends with whitespace" ] },
      "warning for space in $l_code",
    );
  }
}

{
  my $input = "=pod\n\nAmbiguous slash: L<I/O Operators|op/io>\n";
  my $errors = errors("$input");
  is_deeply(
    $errors,
    { 3 => [ "alternative text 'I/O Operators' contains non-escaped | or /" ] },
    "warning for / in text part of L<>",
  );
}

{
  my $input = "=pod\n\nnested LE<lt>E<sol>E<gt>: L<Nested L<http://foobar>|http://baz>\n";
  my $errors = errors("$input");
  is_deeply(
    $errors,
    { 3 => [ "Nested L<> are illegal.  Pretending inner one is X<...> so can continue looking for other errors." ] },
      "warning for nested L<>",
  );
}
    
{
  my $input = "=pod\n\nLE<lt>E<sol>E<gt> containing only slash: L< / >\n";
  my $errors = errors("$input");
  is_deeply(
    $errors,
    { 3 => [ "L<> contains only '/'" ] },
      "warning for L< / > containing only a slash",
  );
}
