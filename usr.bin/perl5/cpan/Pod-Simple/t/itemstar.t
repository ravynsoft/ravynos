BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 6 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import;
}
#my $d;
#use Pod::Simple::Debug (3);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

my $x = 'Pod::Simple::XMLOutStream';

print "##### Tests for '=item * Foo' tolerance via class $x\n";

$Pod::Simple::XMLOutStream::ATTR_PAD   = ' ';
$Pod::Simple::XMLOutStream::SORT_ATTRS = 1; # for predictably testable output


print "#\n# Tests for simple =item *'s\n";
ok( $x->_out("\n=over\n\n=item * Stuff\n\n=item * Bar I<baz>!\n\n=back\n\n"),
    '<Document><over-bullet indent="4"><item-bullet>Stuff</item-bullet><item-bullet>Bar <I>baz</I>!</item-bullet></over-bullet></Document>'
);
ok( $x->_out("\n=over\n\n=item * Stuff\n\n=cut\n\nStuff\n\n=item *\n\nBar I<baz>!\n\n=back\n\n"),
    '<Document><over-bullet indent="4"><item-bullet>Stuff</item-bullet><item-bullet>Bar <I>baz</I>!</item-bullet></over-bullet></Document>'
);
ok( $x->_out("\n=over 10\n\n=item * Stuff\n\n=cut\n\nStuff\n\n=item *\n\nBar I<baz>!\n\n=back\n\n"),
    '<Document><over-bullet indent="10"><item-bullet>Stuff</item-bullet><item-bullet>Bar <I>baz</I>!</item-bullet></over-bullet></Document>'
);
ok( $x->_out("\n=over\n\n=item * Stuff I<things\num> hoo!\n=cut\nStuff\n\n=item *\n\nBar I<baz>!\n\n=back"),
    '<Document><over-bullet indent="4"><item-bullet>Stuff <I>things um</I> hoo!</item-bullet><item-bullet>Bar <I>baz</I>!</item-bullet></over-bullet></Document>'
);




print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";


