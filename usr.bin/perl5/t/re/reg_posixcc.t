#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;
plan "no_plan";

my @pats=(
            "\\w",
	    "\\W",
	    "\\s",
	    "\\S",
	    "\\d",
	    "\\D",
            "\\h",
	    "\\H",
            "\\v",
	    "\\V",
	    "[:alnum:]",
	    "[:^alnum:]",
	    "[:alpha:]",
	    "[:^alpha:]",
	    "[:ascii:]",
	    "[:^ascii:]",
	    "[:cntrl:]",
	    "[:^cntrl:]",
	    "[:graph:]",
	    "[:^graph:]",
	    "[:lower:]",
	    "[:^lower:]",
	    "[:print:]",
	    "[:^print:]",
	    "[:punct:]",
	    "[:^punct:]",
	    "[:upper:]",
	    "[:^upper:]",
	    "[:xdigit:]",
	    "[:^xdigit:]",
	    "[:space:]",
	    "[:^space:]",
	    "[:blank:]",
	    "[:^blank:]" );

sub rangify {
    my $ary= shift;
    my $fmt= shift || '%d';
    my $sep= shift || ' ';
    my $rng= shift || '..';
    
    
    my $first= $ary->[0];
    my $last= $ary->[0];
    my $ret= sprintf $fmt, $first;
    for my $idx (1..$#$ary) {
        if ( $ary->[$idx] != $last + 1) {
            if ($last!=$first) {
                $ret.=sprintf "%s$fmt",$rng, $last;
            }             
            $first= $last= $ary->[$idx];
            $ret.=sprintf "%s$fmt",$sep,$first;
         } else {
            $last= $ary->[$idx];
         }
    }
    if ( $last != $first) {
        $ret.=sprintf "%s$fmt",$rng, $last;
    }
    return $ret;
}

# The bug is only fixed for /u
use feature 'unicode_strings';

my $description = "";
while (@pats) {
    my ($yes,$no)= splice @pats,0,2;
    
    my %err_by_type;
    my %singles;
    my %complements;
    foreach my $b (0..255) {
        my %got;
        my $display_b = sprintf("0x%02X", $b);
        for my $type ('utf8','not-utf8') {
            my $str=chr($b).chr($b);
            if ($type eq 'utf8') {
                $str.=chr(256);
                chop $str;
            }
            if ($str=~/[$yes][$no]/){
                unlike($str,qr/[$yes][$no]/,
                    "chr($display_b) X 2 =~/[$yes][$no]/ should not match under $type");
                push @{$err_by_type{$type}},$b;
            }
            $got{"[$yes]"}{$type} = $str=~/[$yes]/ ? 1 : 0;
            $got{"[$no]"}{$type} = $str=~/[$no]/ ? 1 : 0;
            $got{"[^$yes]"}{$type} = $str=~/[^$yes]/ ? 1 : 0;
            $got{"[^$no]"}{$type} = $str=~/[^$no]/ ? 1 : 0;

            # For \w, \s, and \d, \h, \v, also test without being in character
            # classes.
            next if $yes =~ /\[/;

            # The rest of this .t was written when there were many test
            # failures, so it goes to some lengths to summarize things.  Now
            # those are fixed, so these missing tests just do standard
            # procedures

            my $chr = chr($b);
            utf8::upgrade $chr if $type eq 'utf8';
            ok (($chr =~ /$yes/) != ($chr =~ /$no/),
                "$type: chr($display_b) isn't both $yes and $no");
        }
        foreach my $which ("[$yes]","[$no]","[^$yes]","[^$no]") {
            if ($got{$which}{'utf8'} != $got{$which}{'not-utf8'}){
                is($got{$which}{'utf8'},$got{$which}{'not-utf8'},
                    "chr($display_b) X 2=~ /$which/ should have the same results regardless of internal string encoding");
                push @{$singles{$which}},$b;
            }
        }
        foreach my $which ($yes,$no) {
            foreach my $strtype ('utf8','not-utf8') {
                if ($got{"[$which]"}{$strtype} == $got{"[^$which]"}{$strtype}) {
                    isnt($got{"[$which]"}{$strtype},$got{"[^$which]"}{$strtype},
                        "chr($display_b) X 2 =~ /[$which]/ should not have the same result as chr($display_b)=~/[^$which]/");
                    push @{$complements{$which}{$strtype}},$b;
                }
            }
        }
    }
    
    
    if (%err_by_type || %singles || %complements) {
        $description||=" Error:\n";
        $description .= "/[$yes][$no]/\n";
        if (%err_by_type) {
            foreach my $type (sort keys %err_by_type) {
                $description .= "\tmatches $type codepoints:\t";
                $description .= rangify($err_by_type{$type});
                $description .= "\n";
            }
            $description .= "\n";
        }
        if (%singles) {
            $description .= "Unicode/Nonunicode mismatches:\n";
            foreach my $type (sort keys %singles) {
                $description .= "\t$type:\t";
                $description .= rangify($singles{$type});
                $description .= "\n";
            }
            $description .= "\n";
        }
        if (%complements) {
            foreach my $class (sort keys %complements) {
                foreach my $strtype (sort keys %{$complements{$class}}) {
                    $description .= "\t$class has complement failures under $strtype for:\t";
                    $description .= rangify($complements{$class}{$strtype});
                    $description .= "\n";
                }
            }
        }
    }
}
__DATA__
