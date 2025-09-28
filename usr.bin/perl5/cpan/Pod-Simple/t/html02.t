# Testing HTML text styles

BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

#use Pod::Simple::Debug (10);

use strict;
use warnings;
use Test;
BEGIN { plan tests => 7};
use Pod::Simple::HTML;

sub x { Pod::Simple::HTML->_out(
  sub{  $_[0]->bare_output(1)  },
  "=pod\n\n$_[0]",
) }

ok 1;

my @pairs = (
[ "I<italicized>"   => qq{\n<p><i>italicized</i></p>\n} ],
[ 'B<bolded>'       => qq{\n<p><b>bolded</b></p>\n}           ],
[ 'C<code>'         => qq{\n<p><code>code</code></p>\n} ],
[ 'F</tmp/foo>'     => qq{\n<p><em>/tmp/foo</em></p>\n} ],
[ 'F</tmp/foo>'     => qq{\n<p><em>/tmp/foo</em></p>\n} ],
);
 
 
foreach( @pairs ) {
  print "# Testing pod source $$_[0] ...\n" unless $_->[0] =~ m/\n/;
  ok( x($_->[0]), $_->[1] )
}
print "# And one for the road...\n";
ok 1;


