#!/usr/bin/perl -w

# t/rtf_utf8.t - Check that RTF works with UTF-8 input

BEGIN {
    chdir 't' if -d 't';
}

my $expected = join "", <DATA>;

use strict;
use warnings;
use lib '../lib';
use Test::More;
use File::Spec;

if ($] < 5.008) {
    plan skip_all => "Doesn't work before 5.8";
}
else {
    plan tests => 5;
}

for my $format (qw(RTF)) {
    my $class = "Pod::Simple::RTF";
    use_ok $class or next;
    ok my $parser = $class->new, "Construct RTF parser";

    my $output = '';
    ok $parser->output_string(\$output), "Set RTF output string";
    ok $parser->parse_file(File::Spec->catfile(qw(corpus polish_utf8.txt))),
        "Parse to RTF via parse_file()";
    $output =~ s/\\info.*?author \[see doc\]\}/VARIANT TEXT DELETED/s;
    $output =~ s/$/\n/;

    my $msg = "got expected output";
    if ($output eq $expected) {
        pass($msg);
    }
    elsif ($ENV{PERL_TEST_DIFF}) {
        fail($msg);
        require File::Temp;
        my $orig_file = File::Temp->new();
        local $/ = "\n";
        chomp $expected;
        print $orig_file $expected, "\n";
        close $orig_file || die "Can't close orig_file: $!";

        chomp $output;
        my $parsed_file = File::Temp->new();
        print $parsed_file $output, "\n";
        close $parsed_file || die "Can't close parsed_file";

        my $diff = File::Temp->new();
        system("$ENV{PERL_TEST_DIFF} $orig_file $parsed_file > $diff");

        open my $fh, "<", $diff || die "Can't open $diff";
        my @diffs = <$fh>;
        diag(@diffs);
    }
    else {
        eval { require Text::Diff; };
        if ($@) {
            is($output, $expected, $msg);
            diag("Set environment variable PERL_TEST_DIFF=diff_tool or install"
            . " Text::Diff to see just the differences.");
        }
        else {
            fail($msg);
            diag Text::Diff::diff(\$expected, \$output, { STYLE => 'Unified' });
        }
    }
}

__DATA__
{\rtf1\ansi\deff0

{\fonttbl
{\f0\froman Times New Roman;}
{\f1\fmodern Courier New;}
{\f2\fswiss Arial;}
}

{\stylesheet
{\snext0 Normal;}
{\*\cs10 \additive Default Paragraph Font;}
{\*\cs16 \additive \i \sbasedon10 pod-I;}
{\*\cs17 \additive \i\lang1024\noproof \sbasedon10 pod-F;}
{\*\cs18 \additive \b \sbasedon10 pod-B;}
{\*\cs19 \additive \f1\lang1024\noproof\sbasedon10 pod-C;}
{\s20\ql \li0\ri0\sa180\widctlpar\f1\fs18\lang1024\noproof\sbasedon0 \snext0 pod-codeblock;}
{\*\cs21 \additive \lang1024\noproof \sbasedon10 pod-computerese;}
{\*\cs22 \additive \i\lang1024\noproof\sbasedon10 pod-L-pod;}
{\*\cs23 \additive \i\lang1024\noproof\sbasedon10 pod-L-url;}
{\*\cs24 \additive \i\lang1024\noproof\sbasedon10 pod-L-man;}

{\*\cs25 \additive \f1\lang1024\noproof\sbasedon0 pod-codelbock-plain;}
{\*\cs26 \additive \f1\lang1024\noproof\sbasedon25 pod-codelbock-ital;}
{\*\cs27 \additive \f1\lang1024\noproof\sbasedon25 pod-codelbock-bold;}
{\*\cs28 \additive \f1\lang1024\noproof\sbasedon25 pod-codelbock-bold-ital;}

{\s31\ql \keepn\sb90\sa180\f2\fs32\ul\sbasedon0 \snext0 pod-head1;}
{\s32\ql \keepn\sb90\sa180\f2\fs28\ul\sbasedon0 \snext0 pod-head2;}
{\s33\ql \keepn\sb90\sa180\f2\fs25\ul\sbasedon0 \snext0 pod-head3;}
{\s34\ql \keepn\sb90\sa180\f2\fs22\ul\sbasedon0 \snext0 pod-head4;}
}

{\colortbl;\red255\green0\blue0;\red0\green0\blue255;}
{VARIANT TEXT DELETED{\company [see doc]}{\operator [see doc]}
}

\deflang1033\plain\lang1033\widowctrl
{\header\pard\qr\plain\f2\fs17
W\uc1\u346?R\'d3D NOCNEJ CISZY \_\_ explicitly utf8 test document in Polish, 
p.\chpgn\par}
\fs25



{\pard\li0\s31\keepn\sb90\sa180\f2\fs32\ul{
NAME
}\par}

{\pard\li0\sa180
W\uc1\u346?R\'d3D NOCNEJ CISZY \_\_ explicitly utf8 test document 
in Polish
\par}

{\pard\li0\s31\keepn\sb90\sa180\f2\fs32\ul{
DESCRIPTION
}\par}

{\pard\li0\sa180
This is a test Pod document in UT\'468. Its content is the lyrics 
to the Polish Christmas carol "W\uc1\u347?r\'f3d nocnej ciszy", except 
it includes a few lines to test RT\'46 specially.
\par}

{\pard\li0\sa180
\uc1\u-1280? is a character in the upper half of Plane 0, so should 
be negative in RT\'46 \uc1\u-10187\u-8904? is a character in Plane 
1, so should be expressed as a surrogate pair in RT\'46
\par}

{\pard\li0\sa180
All the ASCII printables !"#$%&\'5c'()*+,\_./0123456789:;<=>?@ ABCDE\'46GHIJKLMNOPQRSTUVWXYZ[{
\cs21\lang1024\noproof \'5c]^\'5f`} abcdefghijklmnopqrstuvwxyz\'7b|\'7d~
\par}

{\pard\li0\sa180
W\uc1\u347?r\'f3d nocnej ciszy g\uc1\u322?os si\uc1\u281? rozchodzi: 
/ Wsta\uc1\u324?cie, pasterze, B\'f3g si\uc1\u281? nam rodzi! / Czym 
pr\uc1\u281?dzej si\uc1\u281? wybierajcie, / Do Betlejem pospieszajcie 
/ Przywita\uc1\u263? Pana.
\par}

{\pard\li0\sa180
Poszli, znale\uc1\u378?li Dzieci\uc1\u261?tko w \uc1\u380?\uc1\u322?obie 
/ Z wszystkimi znaki danymi sobie. / Jako Bogu cze\uc1\u347?\uc1\u263? 
Mu dali, / A witaj\uc1\u261?c zawo\uc1\u322?ali / Z wielkiej rado\uc1\u347?ci:
\par}

{\pard\li0\sa180
Ach, witaj Zbawco z dawno \uc1\u380?\uc1\u261?dany, / Wiele tysi\uc1\u281?cy 
lat wygl\uc1\u261?dany / Na Ciebie kr\'f3le, prorocy / Czekali, a 
Ty\uc1\u347? tej nocy / Nam si\uc1\u281? objawi\uc1\u322?.
\par}

{\pard\li0\sa180
I my czekamy na Ciebie, Pana, / A skoro przyjdziesz na g\uc1\u322?os 
kap\uc1\u322?ana, / Padniemy na twarz przed Tob\uc1\u261?, / Wierz\uc1\u261?c, 
\uc1\u380?e\uc1\u347? jest pod os\uc1\u322?on\uc1\u261? / Chleba i 
wina.
\par}

{\pard\li0\s32\keepn\sb90\sa180\f2\fs28\ul{
As Verbatim
}\par}

{\pard\li0\sa180
And now as verbatim text:
\par}

{\pard\li0\plain\s20\sa180\f1\fs18\lang1024\noproof
  \uc1\u-1280?  upper half, Plane 0\line
  \uc1\u-10187\u-8904? Plane 1\line
\line
  All the ASCII printables\line
   !"#$%&\'5c'()*+,-./0123456789:;<=>?@\line
  ABCDE\'46GHIJKLMNOPQRSTUVWXYZ[\'5c]^\'5f`\line
  abcdefghijklmnopqrstuvwxyz\'7b|\'7d~\line
\line
  W\uc1\u347?r\'f3d nocnej ciszy g\uc1\u322?os si\uc1\u281? rozchodzi:\line
  Wsta\uc1\u324?cie, pasterze, B\'f3g si\uc1\u281? nam rodzi!\line
  Czym pr\uc1\u281?dzej si\uc1\u281? wybierajcie,\line
  Do Betlejem pospieszajcie\line
  Przywita\uc1\u263? Pana.\line
\line
  Poszli, znale\uc1\u378?li Dzieci\uc1\u261?tko w \uc1\u380?\uc1\u322?obie\line
  Z wszystkimi znaki danymi sobie.\line
  Jako Bogu cze\uc1\u347?\uc1\u263? Mu dali,\line
  A witaj\uc1\u261?c zawo\uc1\u322?ali\line
  Z wielkiej rado\uc1\u347?ci:\line
\line
  Ach, witaj Zbawco z dawno \uc1\u380?\uc1\u261?dany,\line
  Wiele tysi\uc1\u281?cy lat wygl\uc1\u261?dany\line
  Na Ciebie kr\'f3le, prorocy\line
  Czekali, a Ty\uc1\u347? tej nocy\line
  Nam si\uc1\u281? objawi\uc1\u322?.\line
\line
  I my czekamy na Ciebie, Pana,\line
  A skoro przyjdziesz na g\uc1\u322?os kap\uc1\u322?ana,\line
  Padniemy na twarz przed Tob\uc1\u261?,\line
  Wierz\uc1\u261?c, \uc1\u380?e\uc1\u347? jest pod os\uc1\u322?on\uc1\u261?\line
  Chleba i wina.
\par}

{\pard\li0\sa180
[end]
\par}
}
