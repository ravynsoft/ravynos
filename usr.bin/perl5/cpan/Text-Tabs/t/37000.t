use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

#Causes Text::Wrap to die...

my $toPrint = "(1) Category\t(2 or greater) New Category\n\n"; 
my $good =    "(1) Category\t(2 or greater) New Category\n"; 

print "1..6\n";

local($Text::Wrap::break) = '\s';
eval { $toPrint = wrap("","",$toPrint); };
ok( !$@ );
ok( $toPrint eq $good );

local($Text::Wrap::break) = '\d';
eval { $toPrint = wrap("","",$toPrint); };
ok( !$@ );
ok( $toPrint eq $good );

local($Text::Wrap::break) = 'a';
eval { $toPrint = wrap("","",$toPrint); };
ok( !$@ );
ok( $toPrint eq $good );

