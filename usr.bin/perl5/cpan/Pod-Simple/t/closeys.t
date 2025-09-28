BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 3 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import('f');
}

my $d;
#use Pod::Simple::Debug (\$d,0);
#use Pod::Simple::Debug (10);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

sub nowhine {
#  $_[0]->{'no_whining'} = 1;
  $_[0]->accept_targets("*");
}

local $Pod::Simple::XMLOutStream::SORT_ATTRS = 1;
&ok(f(
	\&nowhine,
"=begin :foo\n\n=begin :bar\n\nZaz\n\n",
"=begin :foo\n\n=begin :bar\n\nZaz\n\n=end :bar\n\n=end :foo\n\n",
));


print "# Ending ", __FILE__, "\n";
ok 1;

__END__


