# encoding not error 0
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

#use Pod::Simple::Debug (5);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

{
my @output_lines = split m/[\cm\cj]+/, Pod::Simple::XMLOutStream->_out( q{

=encoding koi8-r

=head1 NAME

Bippitty Boppity Boo -- Yormp

=cut

} );


if(grep m/Unknown directive/i, @output_lines ) {
  ok 0;
  print "# I saw an Unknown directive warning here! :\n",
    map("#==> $_\n", @output_lines), "#\n#\n";
} else {
  ok 1;
}

}

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
print "# Now a control group, to make sure that =fishbladder DOES\n",
      "#  cause an 'unknown directive' error...\n";
      
{
my @output_lines = split m/[\cm\cj]+/, Pod::Simple::XMLOutStream->_out( q{

=fishbladder

=head1 NAME

Fet's "When you were reading"

=cut

} );


if(grep m/Unknown directive/i, @output_lines ) {
  ok 1;
} else {
  ok 0;
  print "# But I didn't see an Unknows directive warning here! :\n",
    map("#==> $_\n", @output_lines), "#\n#\n";
}

}



print "#\n# And one for the road...\n";
ok 1;

