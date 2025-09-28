# tell parser the source POD has already been decoded from bytes to chars
# =encoding line should be ignored
# utf8 characters should come through unscathed

BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }

    use Config;
    if ($Config::Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 5 };

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;

my $parser = Pod::Simple::XMLOutStream->new;
$parser->parse_characters(1);
my $output = '';
$parser->output_string( \$output );
$parser->parse_string_document(qq{

=encoding bogocode

=head1 DESCRIPTION

Confirm that if we tell the parser to expect character data, it avoids all
the code paths that might attempt to decode the source from bytes to chars.

The r\x{101}in in \x{15E}pain \x{FB02}oods the plain

});

ok(1); # parsed without exception

if($output =~ /POD ERRORS/) {
  ok(0);
}
else {
  ok(1); # no errors
}

$output =~ s{&#(\d+);}{chr($1)}eg;

if($output =~ /The r\x{101}in in \x{15E}pain \x{FB02}oods the plain/) {
  ok(1); # data was not messed up
}
else {
  ok(0);
}

##############################################################################
# Test multiple =encoding declarations.
$parser = Pod::Simple::XMLOutStream->new;
$output = '';
$parser->output_string( \$output );
$parser->parse_string_document(qq{

=pod

=encoding UTF-8

=encoding UTF-8

=head1 DESCRIPTION

Confirm that the parser detects multiple encodings and complains.
});

# Should have an error.
ok($output =~ /POD ERRORS/);
ok($output =~ /Cannot have multiple =encoding directives/);

exit;
