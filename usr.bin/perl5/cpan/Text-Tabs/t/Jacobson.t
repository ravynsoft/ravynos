use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

print "1..1\n";

$Text::Wrap::huge='overflow';
$Text::Wrap::columns=9;
$Text::Wrap::break=".(?<=[,.])";
eval {
$a=$a=wrap('','',
"mmmm,n,ooo,ppp.qqqq.rrrrr,sssssssssssss,ttttttttt,uu,vvv wwwwwwwww####\n");
};

ok( !$@ ) or diag( $@ );
