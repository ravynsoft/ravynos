# Testing the =item directive
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 4 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}

my $d;
#use Pod::Simple::Debug (\$d,0);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

my $x = 'Pod::Simple::XMLOutStream';

print "##### Tests for =item directives via class $x\n";

$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output


print "#\n# Tests for =item [number] that are icky...\n";
ok( $x->_out(sub { $_[0]->no_errata_section(1) },
  "\n=over\n\n=item 5\n\nStuff\n\n=cut\n\nCrunk\nZorp\n\n=item 4\n\nQuux\n\n=back\n\n"),
    '<Document><over-text indent="4"><item-text>5</item-text><Para>Stuff</Para><item-text>4</item-text><Para>Quux</Para></over-text></Document>'
);

ok( $x->_out(sub { $_[0]->no_errata_section(1) },
  "\n=over\n\n=item 5.\n\nStuff\n\n=cut\n\nCrunk\nZorp\n\n=item 4.\n\nQuux\n\n=back\n\n"),
    '<Document><over-text indent="4"><item-text>5.</item-text><Para>Stuff</Para><item-text>4.</item-text><Para>Quux</Para></over-text></Document>'
);


print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

