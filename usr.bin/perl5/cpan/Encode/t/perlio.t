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
    unless (PerlIO::Layer->find('perlio')){
        print "1..0 # Skip: PerlIO required\n";
        exit 0;
    }
    $| = 1;
}

use strict;
use File::Basename;
use File::Spec;
use File::Compare qw(compare_text);
use File::Copy;
use FileHandle;

#use Test::More qw(no_plan);
use Test::More tests => 38;

our $DEBUG = 0;

use Encode (":all");
{
    no warnings;
    @ARGV and $DEBUG = shift;
    #require Encode::JP::JIS7;
    #require Encode::KR::2022_KR;
    #$Encode::JP::JIS7::DEBUG = $DEBUG;
}

my $seq = 0;
my $dir = dirname(__FILE__);

my %e = 
    (
     jisx0208 => [ qw/euc-jp shiftjis 7bit-jis iso-2022-jp iso-2022-jp-1/],
     ksc5601  => [ qw/euc-kr/],
     gb2312   => [ qw/euc-cn hz/],
    );

$/ = "\x0a"; # may fix VMS problem for test #28 and #29

for my $src (sort keys %e) {
    my $ufile = File::Spec->catfile($dir,"$src.utf");
    open my $fh, "<:utf8", $ufile or die "$ufile : $!";
    my @uline = <$fh>;
    my $utext = join('' => @uline);
    close $fh;

    for my $e (@{$e{$src}}){
    my $sfile = File::Spec->catfile($dir,"$$.sio");
    my $pfile = File::Spec->catfile($dir,"$$.pio");
    
    # first create a file without perlio
    dump2file($sfile, &encode($e, $utext, 0));
    
    # then create a file via perlio without autoflush

    SKIP:{
        skip "$e: !perlio_ok", 4 unless (perlio_ok($e) or $DEBUG);
        no warnings 'uninitialized';
        open $fh, ">:encoding($e)", $pfile or die "$sfile : $!";
        $fh->autoflush(0);
        print $fh $utext;
        close $fh;
        $seq++;
        is(compare_text($sfile, $pfile), 0 => ">:encoding($e)");
        if ($DEBUG){
        copy $sfile, "$sfile.$seq";
        copy $pfile, "$pfile.$seq";
        }
        
        # this time print line by line.
        # works even for ISO-2022 but not ISO-2022-KR
        open $fh, ">:encoding($e)", $pfile or die "$sfile : $!";
        $fh->autoflush(1);
        for my $l (@uline) {
        print $fh $l;
        }
        close $fh;
        $seq++;
        is(compare_text($sfile, $pfile), 0 => ">:encoding($e) by lines");
        if ($DEBUG){
        copy $sfile, "$sfile.$seq";
        copy $pfile, "$pfile.$seq";
        }
        my $dtext;
        open $fh, "<:encoding($e)", $pfile or die "$pfile : $!";
        $fh->autoflush(0);
        $dtext = join('' => <$fh>);
        close $fh;
        $seq++;
        ok($utext eq $dtext, "<:encoding($e)");
        if ($DEBUG){
        dump2file("$sfile.$seq", $utext);
        dump2file("$pfile.$seq", $dtext);
        }
        if (perlio_ok($e) or $DEBUG){
        $dtext = '';
        open $fh, "<:encoding($e)", $pfile or die "$pfile : $!";
        while(defined(my $l = <$fh>)) {
            $dtext .= $l;
        }
        close $fh;
        }
        $seq++;
        ok($utext eq $dtext,  "<:encoding($e) by lines");
        if ($DEBUG){
        dump2file("$sfile.$seq", $utext);
        dump2file("$pfile.$seq", $dtext);
        }
    }
     if ( ! $DEBUG ) {
            1 while unlink ($sfile);
            1 while unlink ($pfile);
        }
    }
}

# BOM Test

SKIP:{
    my $pev = PerlIO::encoding->VERSION;
    skip "PerlIO::encoding->VERSION = $pev <= 0.07 ", 6
    unless ($pev >= 0.07 or $DEBUG);

    my $file = File::Spec->catfile($dir,"jisx0208.utf");
    open my $fh, "<:utf8", $file or die "$file : $!";
    my $str = join('' => <$fh>);
    close $fh;
    my %bom = (
           'UTF-16BE' => pack('n', 0xFeFF),
           'UTF-16LE' => pack('v', 0xFeFF),
           'UTF-32BE' => pack('N', 0xFeFF),
           'UTF-32LE' => pack('V', 0xFeFF),
          );
    # reading
    for my $utf (sort keys %bom){
    my $bomed = $bom{$utf} . encode($utf, $str);
    my $sfile = File::Spec->catfile($dir,".${utf}_${seq}_$$");
    dump2file($sfile, $bomed);
    my $utf_nobom = $utf; $utf_nobom =~ s/(LE|BE)$//o;
    # reading
    open $fh, "<:encoding($utf_nobom)", $sfile or die "$sfile : $!";
    my $cmp = join '' => <$fh>;
    close $fh;
    is($str, $cmp, "<:encoding($utf_nobom) eq $utf");
    unlink $sfile;  $seq++;
    }
    # writing
    for my $utf_nobom (qw/UTF-16 UTF-32/){
    my $utf = $utf_nobom . 'BE';
    my $sfile = File::Spec->catfile($dir,".${utf_nobom}_${seq}_$$");
    my $bomed = $bom{$utf} . encode($utf, $str);
    open  $fh, ">:encoding($utf_nobom)", $sfile or die "$sfile : $!";
    print $fh $str;
    close $fh;
    open my $fh, "<:bytes", $sfile or die "$sfile : $!";
    read $fh, my $cmp, -s $sfile;
    close $fh;
    use bytes ();
    ok($bomed eq $cmp, ">:encoding($utf_nobom) eq $utf");
    unlink $sfile; $seq++;
    }
}
sub dump2file{
    no warnings;
    open my $fh, ">", $_[0] or die "$_[0]: $!";
    binmode $fh;
    print $fh $_[1];
    close $fh;
}
