#!/usr/local/bin/perl
#
# $Id: rt.pl,v 2.1 2006/05/03 18:24:10 dankogai Exp $
#

BEGIN {
    my $ucmdir  = "ucm";
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
        $ucmdir = "../ext/Encode/ucm";
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
    use strict;
    require Test::More;
    our $DEBUG;
    our @ucm;
    unless(@ARGV){
    use File::Spec;
    Test::More->import(tests => 103);
    opendir my $dh, $ucmdir or die "$ucmdir:$!";
    @ucm = 
        map {File::Spec->catfile($ucmdir, $_) } 
        sort grep {/\.ucm$/o} readdir($dh);
    closedir $dh;
    }else{
    Test::More->import("no_plan");
    $DEBUG = 1;
    @ucm = @ARGV;
    }
}

use strict;
use Encode qw/encode decode/;
our $DEBUG;
our @ucm;

for my $ucm (@ucm){
    my ($name, $nchar, $nrt, $nok) = rttest($ucm);
    $nok += 0;
    ok($nok == 0, "$ucm => $name ($nchar, $nrt, $nok)");
}

sub rttest{
    my $ucm = shift;
    my ($name, $nchar, $nrt, $nok);
    open my $rfh, "<$ucm" or die "$ucm:$!";
    # <U0000> \x00 |0 # <control>
    while(<$rfh>){
    s/#.*//o; /^$/ and next;
    unless ($name){
        /^<code_set_name>\s+"([^\"]+)"/io or next;
        $name = $1 and next;
    }else{
        /^<U([0-9a-f]+)>\s+(\S+)\s+\|(\d)/io or next;
        $nchar++;
        $3 == 0 or next;
        $nrt++;
        my $uni = chr(hex($1));
        my $enc = eval qq{ "$2" };
        decode($name, $enc) eq $uni or $nok++;
        encode($name, $uni) eq $enc or $nok++;
    }
    }
    return($name, $nchar, $nrt, $nok);
}
__END__
