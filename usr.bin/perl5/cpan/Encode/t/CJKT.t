BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
# should work w/o PerlIO now!
#    unless (PerlIO::Layer->find('perlio')){
#	print "1..0 # Skip: PerlIO required\n";
#	exit 0;
#   }
    $| = 1;
}
use strict;
use Test::More tests => 60;
use Encode;
use File::Basename;
use File::Spec;
use File::Compare qw(compare_text);
our $DEBUG = shift || 0;

my %Charset =
    (
     'big5-eten'  => [qw(big5-eten)],
     'big5-hkscs' => [qw(big5-hkscs)],
     gb2312       => [qw(euc-cn hz)],
     jisx0201     => [qw(euc-jp shiftjis 7bit-jis)],
     jisx0208     => [qw(euc-jp shiftjis 7bit-jis iso-2022-jp iso-2022-jp-1)],
     jisx0212     => [qw(euc-jp 7bit-jis iso-2022-jp-1)],
     ksc5601      => [qw(euc-kr iso-2022-kr johab)],
    );


my $dir = dirname(__FILE__);
my $seq = 1;

for my $charset (sort keys %Charset){
    my ($src, $uni, $dst, $txt);

    my $transcoder = find_encoding($Charset{$charset}[0]) or die;

    my $src_enc = File::Spec->catfile($dir,"$charset.enc");
    my $src_utf = File::Spec->catfile($dir,"$charset.utf");
    my $dst_enc = File::Spec->catfile($dir,"$$.enc");
    my $dst_utf = File::Spec->catfile($dir,"$$.utf");

    open $src, "<$src_enc" or die "$src_enc : $!";
    
    if (PerlIO::Layer->find('perlio')){
    binmode($src, ":bytes"); # needed when :utf8 in default open layer
    }

    $txt = join('',<$src>);
    close($src);
    
    eval { $uni = $transcoder->decode($txt, 1) } or print $@;
    ok(defined($uni),  "decode $charset"); $seq++;
    is(length($txt),0, "decode $charset completely"); $seq++;
    
    open $dst, ">$dst_utf" or die "$dst_utf : $!";
    if (PerlIO::Layer->find('perlio')){
    binmode($dst, ":utf8");
    print $dst $uni;
    }else{ # ugh!
    binmode($dst);
    my $raw = $uni; Encode::_utf8_off($raw);
    print $dst $raw;
    }

    close($dst); 
    is(compare_text($dst_utf, $src_utf), 0, "$dst_utf eq $src_utf")
    or ($DEBUG and rename $dst_utf, "$dst_utf.$seq");
    $seq++;
    
    open $src, "<$src_utf" or die "$src_utf : $!";
    if (PerlIO::Layer->find('perlio')){
    binmode($src, ":utf8");
    $uni = join('', <$src>);
    }else{ # ugh!
    binmode($src);
    $uni = join('', <$src>);
    Encode::_utf8_on($uni);
    }
    close $src;

    my $unisave = $uni;
    eval { $txt = $transcoder->encode($uni,1) } or print $@;
    ok(defined($txt),   "encode $charset"); $seq++;
    is(length($uni), 0, "encode $charset completely");  $seq++;
    $uni = $unisave;

    open $dst,">$dst_enc" or die "$dst_utf : $!";
    binmode($dst);
    print $dst $txt;
    close($dst); 
    is(compare_text($src_enc, $dst_enc), 0 => "$dst_enc eq $src_enc")
    or ($DEBUG and rename $dst_enc, "$dst_enc.$seq");
    $seq++;
    
    unlink($dst_utf, $dst_enc);

    for my $encoding (@{$Charset{$charset}}){
    my $rt = decode($encoding, encode($encoding, $uni));
    is ($rt, $uni, "RT $encoding");
    }
}
