#!./perl

use strict;
use Encode;
use Benchmark qw(:all);

my $Count = shift @ARGV;
$Count ||= 16;
my @sizes = @ARGV || (1, 4, 16);

my %utf8_seed;
for my $i (0x00..0xff){
    my $c = chr($i);
    $utf8_seed{BMP} .= ($c =~ /^\p{IsPrint}/o) ? $c : " ";
}
utf8::upgrade($utf8_seed{BMP});

for my $i (0x00..0xff){
    my $c = chr(0x10000+$i);
    $utf8_seed{HIGH} .= ($c =~ /^\p{IsPrint}/o) ? $c : " ";
}
utf8::upgrade($utf8_seed{HIGH});

my %S;
for my $i (@sizes){
    my $sz = 256 * $i;
    for my $cp (qw(BMP HIGH)){
    $S{utf8}{$sz}{$cp}  = $utf8_seed{$cp} x $i;
    $S{utf16}{$sz}{$cp} = encode('UTF-16BE', $S{utf8}{$sz}{$cp});
    }
}

for my $i (@sizes){
    my $sz = $i * 256;
    my $count = $Count * int(256/$i);
    for my $cp (qw(BMP HIGH)){
    for my $op (qw(encode decode)){
        my ($meth, $from, $to) = ($op eq 'encode') ?
        (\&encode, 'utf8', 'utf16') : (\&decode, 'utf16', 'utf8');
        my $XS = sub {
        Encode::Unicode::set_transcoder("xs");  
        $meth->('UTF-16BE', $S{$from}{$sz}{$cp})
             eq $S{$to}{$sz}{$cp} 
             or die "$op,$from,$to,$sz,$cp";
        };
        my $modern = sub {
        Encode::Unicode::set_transcoder("modern");  
        $meth->('UTF-16BE', $S{$from}{$sz}{$cp})
             eq $S{$to}{$sz}{$cp} 
             or die "$op,$from,$to,$sz,$cp";
        };
        my $classic = sub {
        Encode::Unicode::set_transcoder("classic");  
        $meth->('UTF-16BE', $S{$from}{$sz}{$cp})
             eq $S{$to}{$sz}{$cp} or 
             die "$op,$from,$to,$sz,$cp";
        };
        print "---- $op length=$sz/range=$cp ----\n";
        my $r = timethese($count,
             {
              "XS"      => $XS,
              "Modern"  => $modern,
              "Classic" => $classic,
             },
             'none',
            );
        cmpthese($r);
    }
    }
}
