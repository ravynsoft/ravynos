BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
    unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
use Test::More tests => 29;
use Encode;

no utf8; # we have raw Chinese encodings here

BEGIN {
    use_ok('Encode::CN');
}

# Since JP.t already tests basic file IO, we will just focus on
# internal encode / decode test here. Unfortunately, to test
# against all the UniHan characters will take a huge disk space,
# not to mention the time it will take, and the fact that Perl
# did not bundle UniHan.txt anyway.

# So, here we just test a typical snippet spanning multiple Unicode
# blocks, and hope it can point out obvious errors.

run_tests('Simplified Chinese only', {
    'utf'	=> (
12298.26131.32463.12299.31532.19968.21350.
24406.26352.65306.
22823.21705.20094.20803.65292.19975.29289.36164.22987.65292.
20035.32479.22825.12290.
20113.34892.38632.26045.65292.21697.29289.27969.24418.12290.
22823.26126.22987.32456.65292.20845.20301.26102.25104.65292.
26102.20056.20845.40857.20197.24481.22825.12290.
20094.36947.21464.21270.65292.21508.27491.24615.21629.65292.
20445.21512.22823.21644.65292.20035.21033.36126.12290.
39318.20986.24246.29289.65292.19975.22269.21688.23425.12290
    ),

    'euc-cn'	=> join('',
'《易经》第一卦',
'彖曰：',
'大哉乾元，万物资始，',
'乃统天。',
'云行雨施，品物流形。',
'大明始终，六位时成，',
'时乘六龙以御天。',
'乾道变化，各正性命，',
'保合大和，乃利贞。',
'首出庶物，万国咸宁。',
    ),

    'gb2312-raw'	=> join('',
'!6RW>-!75ZR;XT',
'ehT;#:',
'4sTUG,T*#,MrNoWJJ<#,',
'DKM3Ll!#',
'TFPPSjJ)#,F7NoAwPN!#',
'4sCwJ<VU#,AyN;J13I#,',
'J13KAyAzRTSyLl!#',
'G,5@1d;/#,8wU}PTC|#,',
'1#:O4s:M#,DK@{Uj!#',
'JW3vJ|No#,Mr9zOLD~!#'
    ), 

    'iso-ir-165'=> join('',
'!6RW>-!75ZR;XT',
'ehT;#:',
'4sTUG,T*#,MrNoWJJ<#,',
'DKM3Ll!#',
'TFPPSjJ)#,F7NoAwPN!#',
'4sCwJ<VU#,AyN;J13I#,',
'J13KAyAzRTSyLl!#',
'G,5@1d;/#,8wU}PTC|#,',
'1#:O4s:M#,DK@{Uj!#',
'JW3vJ|No#,Mr9zOLD~!#'
    ), 
});

run_tests('Simplified Chinese + ASCII', {
    'utf'	=> (
35937.26352.65306.10.
22825.34892.20581.65292.21531.23376.20197.33258.24378.19981.24687.12290.10.
28508.40857.21247.29992.65292.38451.22312.19979.20063.12290.32.
35265.40857.22312.30000.65292.24503.26045.26222.20063.12290.32.
32456.26085.20094.20094.65292.21453.22797.36947.20063.12290.10.
25110.36291.22312.28170.65292.36827.26080.21646.20063.12290.39134.
40857.22312.22825.65292.22823.20154.36896.20063.12290.32.
20130.40857.26377.24724.65292.30408.19981.21487.20037.20063.12290.10.
29992.20061.65292.22825.24503.19981.21487.20026.39318.20063.12290
    ),

    'cp936'	=> join(chr(10),
'象曰：',
'天行健，君子以自强不息。',
'潜龙勿用，阳在下也。 见龙在田，德施普也。 终日乾乾，反复道也。',
'或跃在渊，进无咎也。飞龙在天，大人造也。 亢龙有悔，盈不可久也。',
'用九，天德不可为首也。',
    ),

    'hz'	=> join(chr(10),
'~{OsT;#:~}',
'~{LlPP=!#,>}WSRTWTG?2;O"!#~}',
'~{G1AzNpSC#,QtTZOBR2!#~} ~{<{AzTZLo#,5BJ)FUR2!#~} ~{VUHUG,G,#,74845@R2!#~}',
'~{;rT>TZT(#,=xN^>LR2!#7IAzTZLl#,4sHKTlR2!#~} ~{?:AzSP;Z#,S/2;?I>CR2!#~}',
'~{SC>E#,Ll5B2;?IN*JWR2!#~}',
    ),
});

run_tests('Traditional Chinese', {
    'utf',	=> 20094.65306.20803.12289.20136.12289.21033.12289.35998,
    'gb12345-raw'	=> 'G,#:T*!":`!"@{!"Uj',
    'gbk'	=> '乾：元、亨、利、貞',
});

sub run_tests {
    my ($title, $tests) = @_;
    my $utf = delete $tests->{'utf'};

    # $enc = encoding, $str = content
    foreach my $enc (sort keys %{$tests}) {
    my $str = $tests->{$enc};

    is(Encode::decode($enc, $str), $utf, "[$enc] decode - $title");
    is(Encode::encode($enc, $utf), $str, "[$enc] encode - $title");

    my $str2 = $str;
    my $utf8 = Encode::encode('utf-8', $utf);

    Encode::from_to($str2, $enc, 'utf-8');
    is($str2, $utf8, "[$enc] from_to => utf8 - $title");

    Encode::from_to($utf8, 'utf-8', $enc); # convert $utf8 as $enc
    is($utf8, $str,  "[$enc] utf8 => from_to - $title");
    }
}
