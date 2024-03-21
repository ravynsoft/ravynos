#   Copyright (C) 2021-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

use strict;
package acct;
use vars qw(%Acct $Erp);
my($debug_f, $retVal, $OpenDis, $OpenFsingle, $Read_rules_txt);
my(@Comparison, @hashSample, @acctHeader);
my(%RANGE, %Rules);
my($ERROR_ACCT_MISMATCH, $ERROR_NEGATIVE_TIME, $ERROR_PERL_ERROR,
   $ERROR_DIFF_RANGE, $ERROR_ZERO_METRIC, $ERROR_HIGH_UNKNOWN,
   $ERROR_CALLER_VERIF, $ERROR_SIGNAL_LOST);

BEGIN {
#    use Exporter ();
#    @ISA = 'Exporter';
#    @EXPORT_OK = ('&readAcct', '%Acct');
    $debug_f = $ENV{PERL_DEBUG};
    $retVal = 0;
    $OpenDis = 0;
    $OpenFsingle = 0;
    $#Comparison = -1;
    $Read_rules_txt = 0;
    $Erp = {};
    @hashSample = [];

    %RANGE = (
        Count => {  P_RANGE =>  0,      P_RATE => 0,
                    N_RANGE =>  0,      N_RATE => 0,    FMT => "%d"
                 },
        Total => {  P_RANGE =>  0.20,   P_RATE =>  3,
                    N_RANGE => -0.20,   N_RATE => -3,   FMT => "%6.3f"
                 },
        Cpu   => {  P_RANGE =>  0.5,    P_RATE => 10,
                    N_RANGE => -0.5,   N_RATE => -10,   FMT => "%6.3f"
                    ,P_RANGE_2AVG =>  0.5,    P_RATE_2AVG => 10,
                    N_RANGE_2AVG =>  -0.5,    N_RATE_2AVG => -10
                 },
        Cycles => {  P_RANGE =>  0.5,    P_RATE => 10,
                    N_RANGE => -0.5,   N_RATE => -10,   FMT => "%6.3f"
                    ,P_RANGE_2AVG =>  0.5,    P_RATE_2AVG => 10,
                    N_RANGE_2AVG =>  -0.5,    N_RATE_2AVG => -10
                 },
        Cycles1 => {  P_RANGE =>  0.5,    P_RATE => 10,
                    N_RANGE => -0.5,   N_RATE => -10,   FMT => "%6.3f"
                    ,P_RANGE_2AVG =>  0.5,    P_RATE_2AVG => 10,
                    N_RANGE_2AVG =>  -0.5,    N_RATE_2AVG => -10
                 },
        Sync  => {  P_RANGE =>  0.5,    P_RATE =>  3,
                    N_RANGE => -0.5,    N_RATE => -3,   FMT => "%6.3f"
                 },
        Unkn  => {  P_RANGE =>  0.10,   P_RATE =>  0.5,   FMT => "%6.3f" }
    );

    $ERROR_SIGNAL_LOST = 44;
    $ERROR_DIFF_RANGE = 84;
    $ERROR_HIGH_UNKNOWN = 85;
    $ERROR_PERL_ERROR = 86;
    $ERROR_ACCT_MISMATCH = 87;
    $ERROR_CALLER_VERIF = 88;
    $ERROR_ZERO_METRIC = 94;
    $ERROR_NEGATIVE_TIME = 103;
}

sub debug
{
    my ($lineN, $fmt);
    if ( $debug_f == 0 ) {
        return;
    }
    $lineN = shift @_;
    $fmt = shift @_;
    if ( $debug_f == 2 ) {
        warn "DEBUG:#$lineN:\n";
    }
    warn sprintf($fmt, @_);
}

sub set_retVal
{
    if ( $retVal == 0 ) {
        $retVal = $_[0];
        if ($retVal != 0 ) {
          my $s = "";
          if ($retVal == $ERROR_DIFF_RANGE) {
            $s = "Difference out of range";
          } elsif ($retVal == $ERROR_HIGH_UNKNOWN) {
            $s = "High unknown detected";
          } elsif ($retVal == $ERROR_ACCT_MISMATCH) {
            $s = "Accounting file mismatch";
          } elsif ($retVal == $ERROR_CALLER_VERIF) {
            $s = "Caller/caller verification failed";
          } elsif ($retVal == $ERROR_ZERO_METRIC) {
            $s = "Unexpected zero metric";
          } elsif ($retVal == $ERROR_NEGATIVE_TIME) {
            $s = "Negative CPU time";
          }
          warn sprintf("DEBUG: retVal=%d %s\n", $retVal, $s);
        }
    }
    return $retVal;
}

sub diffRule
{
    # The format of the comparison rule is:
    #   <Name>, <Column number in *.acct>, <Column number in erprint.out>, <message>
    #   Cpu,   3, 1
    #   Total, 2, 3
    my ($str) = @_;
    my (@arr);

    @arr = split (/,/, $str);
    if ($#arr == 2) {
        # Old version
        push @arr, $arr[0];
    }
    push @Comparison, [@arr];
}

sub read_rules
{
    my ($name, $rule, $line, @arr);
    return if ( $Read_rules_txt == 1);
    $Read_rules_txt = 1;
    open(FP, "<rules.txt") or return;
    while ($line = <FP>) {
        chomp ($line);
        $line =~ s/\s*//g;   # Remove all blanks
        $line =~ s/\\s/ /g;  # Replace \s with space
        next if ( $line =~ m/^$/ );
        next if ( $line =~ m/^#/ );

        if ( $line =~ m/=/ ) {
            # Set a calculation rule
            ($name, $rule) = split (/=/, $line);
            $Rules{$name} = [split(/\+/, $rule)];
            next;
        }

        # Set a comparison rule
        &diffRule($line);
    }
    close(FP);
}

sub dump_acct()
{
    my ($i, $n, $key, $fmt, @fmt_head);
    printf "dump_acct:\n";
    foreach $i ( @acctHeader ) {
        $fmt = sprintf("%%%ds ", length($i));
        push @fmt_head, $fmt;
        printf $fmt, $i;
    }
    printf "\n";
    foreach $key (sort keys %Acct) {
        $n = 0;
        foreach $i ( @{$Acct{$key}} ) {
            $fmt = $n <= $#fmt_head ? $fmt_head[$n] : " %10s";
            $n++;
            printf $fmt, $i;
        }
        printf "   '%s'", $key;
        if ( exists $Rules{$key} ) {
            printf " := %s", join(" + ", @{$Rules{$key}});
        }
        printf "\n";
    }
}

sub readAcct
{
    # Read the *.acct file into hash $Acct with the function name as key.
    # The format of *.acct is :
    #   X <time1> ... <timeN> <func_name>
    my ($fileName, @checkTime) = @_;
    my ($name, $i, $key, $line, @arr);

    # file *.acct is generated while the test program is running.
    if (!open(FP, "<$fileName")) {
        printf "acct::readAcct: Cannot open '%s'\n\n", $fileName;
        exit($ERROR_ACCT_MISMATCH);
    }
    while ($line = <FP>) {  # Skip the first lines (header)
        last if ( $line =~ m/^X\s+/ );
    }
    @acctHeader = split (/\s+/, $line);
    push @acctHeader, "Comment";
    while ($line = <FP>) {
        chomp($line);
        $line =~ s/^\s*//;   # Delete leading spaces
        next if ( $line =~ m/^$/ );
        @arr = split (/\s+/, $line);
        $name = pop(@arr);
        if (defined $Acct{$name}) {
            for ($i = 1; $i <= $#arr; $i++ ) {
                $Acct{$name}[$i] += $arr[$i];
            }
        } else {
            $Acct{$name} = [ @arr ];
        }

        foreach $i ( @checkTime ) {
            next if ($i > $#arr);
            if ( $arr[$i] < 0 ) {
                &set_retVal($ERROR_NEGATIVE_TIME);
                last;
            }
        }
    }
    close(FP);

    &read_rules;
    # &checkCallersCallees;

    if ( $debug_f != 0 ) {
        printf "\nreadAcct: '%s'\n", $fileName;
        printf "checkTime: ";
        if( $#checkTime == -1 ) {
                printf "<None>\n";
        } else {
            print "[ ", join(", ", @checkTime), " ]\n";
        }
        foreach $i ( @Comparison ) {
            print "Comparison rule: ", join(", ", @{$i}), "\n";
        }
        &dump_acct;
        printf "\n";
    }
}


sub read_er_print_out
{
    my ($fileName, $colName) = @_;
    my ($name, @arr, $head_f, $line, $key, $i);

    $Erp = {};
    $head_f = 1;
    open(FP, "<$fileName") or return;
    while ($line = <FP>) {
        chomp($line);
        $line =~ s/^\s*//;   # Delete leading spaces
        next if ( $line =~ m/^$/ );
        if ($head_f == 1) {
            # Skip the first lines (header)
            next unless ( $line =~ m/^\d/ );
            next unless ( ($line =~ m/<Total>\s*$/) ||
                          ($line =~ m/<Stack-unwind-failed>\s*$/) );
            $head_f = 0;
            if ($colName == -1) {
                @arr = split (/\s+/, $line);
                $colName = $#arr + 1;
            }
        }
        @arr = split (/\s+/, $line, $colName);
        $name = pop(@arr);
        if (defined $Erp->{$name}) {
            for ($i = 0; $i <= $#arr; $i++ ) {
                $Erp->{$name}[$i] += $arr[$i];
            }
        } else {
            $Erp->{$name} = [ @arr ];
        }

        $i = index($name, "(");
        if ($i > 0) {
          my $funcName = substr($name, 0, $i);
          if (defined $Erp->{$funcName}) {
            for ($i = 0; $i <= $#arr; $i++ ) {
              $Erp->{$funcName}[$i] += $arr[$i];
            }
          } else {
            $Erp->{$funcName} = [ @arr ];
          }
        }
    }
    close(FP);

    if ( $debug_f != 0 ) {
        printf "read_er_print_out:\n";
        foreach $key (sort keys %{$Erp}) {
            foreach $i ( @{$Erp->{$key}} ) {
                printf " %10s", $i;
            }
            printf "  %-10s", "'$key'";
            if ( exists $Rules{$key} ) {
                printf " += %s", join(" + ", @{$Rules{$key}});
            }
            printf "\n";
        }
    }
}


sub createKDiff
{
    my ($colSample) = @_;
    my ($key, $str, $i, $head_str);

    open(DIFF_fp, ">diff.out");
    $head_str = "X";
    for $i ( 0..$#Comparison ) {
        $head_str .= &get_head_str($i);
    }
    $head_str .= "   Name";
    printf DIFF_fp "%s\n", $head_str;
    foreach $key (sort keys %Acct) {
        # Restore a hash 'Erp'
        $Erp = $hashSample[$Acct{$key}[$colSample]];
        $str = &doComp($key, $head_str);
        printf DIFF_fp "%s (Sample %d)\n", $str,$Acct{$key}[$colSample];
    }
    close(DIFF_fp);
    &closeDisFile();
}

sub commandToScr1_fp()
{
    my ($str) = @_;
    printf Scr1_fp "#\n#%s\n%s\n", $str, $str;
}

sub openFsingleScr
{
    return if ($OpenFsingle == 1);
    open(Scr1_fp, ">>erp_fsingle.scr");
    $OpenFsingle = 1;
}

sub closeFsingleScr
{
    return if ($OpenFsingle != 1);
    $OpenFsingle = 2;
    close(Scr1_fp);
}

sub openDisFile
{
    &openFsingleScr();
    return if ($OpenDis == 1);
    open(Dis_fp, ">>discrepancy.out");
    $OpenDis = 1;
}

sub closeDisFile
{
    &closeFsingleScr();
    return if ($OpenDis != 1);
    $OpenDis = 2;
    close(Dis_fp);
}

sub with_diff
{
    my ($i) = @_;
    my ($key);

    $key = $Comparison[$i][0];
    if( ! exists $RANGE{$key} ) {
        printf "acct::with_diff: '$key' is a wrong key\n\n";
        exit $ERROR_PERL_ERROR;
    }
    if ($RANGE{$key}->{FMT} !~ m/^%d/) {
        return 1;
    }
    return 0;
}

sub get_head_str()
{
    my ($i) = @_;
    my ($str);
    $str = $Comparison[$i][3];
    while (length($str) < 16) {
        $str = "*" . $str . "*";
    }
    if (with_diff($i)) {
        return sprintf("| %17s %7s %7s %s", $str, "Diff", "%", "x");
    } else {
        return sprintf("| %17s %s", $str, "x");
    }
}

sub doComp
{
    my ($fname, $head_str) = @_;
    my ($key, $R, $r1, $r2, $diff, $rate, $flagX, $x, $i,
        $retStr, $discrepancy, $err_diff_range, $err_zero_metric, $err_acct_mismatch);

    sub setRate
    {
        my ($val, $diff) = @_;
        return sprintf("%6.1f", ($diff/$val)*100) if ( $val != 0 );
        return sprintf("%6.1f", "0.0") if ( $diff >= -0.05 && $diff <= 0.05);
        return sprintf("%6.1f", "100") if ( $diff > 0 );
        return sprintf("%6.1f", "-100");
    }

    $err_diff_range = 0;
    $err_zero_metric = 0;
    $err_acct_mismatch = 0;
    $discrepancy = " ";
    $flagX = " ";
    $retStr = "";
    for $i ( 0..$#Comparison ) {
        $r1 = $Acct{$fname}[$Comparison[$i][1]];
        $r2 = 0;
        if ( ! exists $Rules{$fname} ) {
            if ( exists $Erp->{$fname} ) {
                $r2 = $Erp->{$fname}[$Comparison[$i][2]];
            }
        } else {
            foreach my $key1 ( @{$Rules{$fname}} ) {
                my $sign = 1;
                $key = $key1;
                if (substr($key1, 0, 1) eq '-') {
                    $key = substr($key1, 1);
                    $sign = -1;
                }
                if ( exists $Erp->{$key} ) {
                    $r2 += $sign * $Erp->{$key}[$Comparison[$i][2]];
                }
            }
        }

        $key = $Comparison[$i][0];
        if( ! exists $RANGE{$key} ) {
            printf "acct::doComp: '$key' is a wrong key\n\n";
            exit $ERROR_PERL_ERROR;
        }
        $R = $RANGE{$key};
        $r1 = sprintf($R->{FMT}, $r1);
        $r2 = sprintf($R->{FMT}, $r2);
        $diff = sprintf($R->{FMT}, $r1 - $r2);
        $rate = &setRate($r1, $diff);
        if ((( $diff > $R->{P_RANGE} ) && ( $rate >= $R->{P_RATE} ))
         || ( ( $fname ne '<Unknown>') && ( $diff < $R->{N_RANGE} ) && ( $rate <= $R->{N_RATE} ))) {
            $x = ($Acct{$fname}[0] eq "Y") ? "y" : "x";
            if ( $x ne "y" ) {
                $flagX = "X";
                &openDisFile();
                printf Dis_fp "%s/ %s\n", $fname, $Comparison[$i][3];

                $discrepancy .= " $Comparison[$i][3]";
                if (with_diff($i)) {
                    if ( $r2 > 0 ) {
                        $err_diff_range = $ERROR_DIFF_RANGE;
                    } else {
                    	if (! exists $ENV{ACCT_FILTER}) {
                            $err_zero_metric = $ERROR_ZERO_METRIC;
                        }
                    }
                } else {
                    $err_acct_mismatch = $ERROR_ACCT_MISMATCH;
                }
	    }
        } else {
            $x = " ";
        }

        if (with_diff($i)) {
            $retStr .= sprintf("| %8s %8s %7s %7s %s", $r1, $r2, $diff, $rate, $x);
        } else {
            $retStr .= sprintf("| %8s %8s %s", $r1, $r2, $x);
        }
    }
    $retStr = $flagX . $retStr . sprintf("   %-10s", $fname);
    if ( exists $Rules{$fname} ) {
        $retStr .=  sprintf " := %s", join(" + ", @{$Rules{$fname}});
    }
    if ($discrepancy ne " ") {
        if ($err_acct_mismatch != 0) {
            $retVal = $err_acct_mismatch;
        }
        &set_retVal($err_zero_metric);
        &set_retVal($err_diff_range);
        printf Scr1_fp "#%s\n#%s\n", $head_str, $retStr;
        &commandToScr1_fp(sprintf("%s %s 1", 'fsingle', $fname));
        &commandToScr1_fp(sprintf("%s %s 1", 'csingle', $fname));
    }
    return ($retStr);
}

sub doComp2AVG
{
    my ($fname, $head_str, @avg) = @_;
    my ($key, $R, $r1, $r2, $diff, $rate, $flagX, $x, $i,
        $retStr, $discrepancy, $err_diff_range, $err_zero_metric, $err_acct_mismatch);

    sub setRate
    {
        my ($val, $diff) = @_;
        return sprintf("%6.1f", ($diff/$val)*100) if ( $val != 0 );
        return sprintf("%6.1f", "0.0") if ( $diff >= -0.05 && $diff <= 0.05);
        return sprintf("%6.1f", "100") if ( $diff > 0 );
        return sprintf("%6.1f", "-100");
    }

    $err_diff_range = 0;
    $err_zero_metric = 0;
    $err_acct_mismatch = 0;
    $discrepancy = " ";
    $flagX = " ";
    $retStr = "";
    for $i ( 0..$#Comparison ) {
        $r1 = $avg[$i];
        $r2 = 0;
        if ( ! exists $Rules{$fname} ) {
            if ( exists $Erp->{$fname} ) {
                $r2 = $Erp->{$fname}[$Comparison[$i][2]];
            }
        } else {
            foreach my $key1 ( @{$Rules{$fname}} ) {
                my $sign = 1;
                $key = $key1;
                if (substr($key1, 0, 1) eq '-') {
                    $key = substr($key1, 1);
                    $sign = -1;
                }
                if ( exists $Erp->{$key} ) {
                    $r2 += $sign * $Erp->{$key}[$Comparison[$i][2]];
                }
            }
        }

        $key = $Comparison[$i][0];
        if( ! exists $RANGE{$key} ) {
            printf "acct::doComp: '$key' is a wrong key\n\n";
            exit $ERROR_PERL_ERROR;
        }
        $R = $RANGE{$key};
        $r1 = sprintf($R->{FMT}, $r1);
        $r2 = sprintf($R->{FMT}, $r2);
        $diff = sprintf($R->{FMT}, $r1 - $r2);
        $rate = &setRate($r1, $diff);
        if ((( $diff > $R->{P_RANGE_2AVG} ) && ( $rate >= $R->{P_RATE_2AVG} ))
         || ( ( $fname ne '<Unknown>') && ( $diff < $R->{N_RANGE_2AVG} ) && ( $rate <= $R->{N_RATE_2AVG} ))) {
            $flagX = "X";
            $x = "x";
            $discrepancy .= " $Comparison[$i][3]";
            if (with_diff($i)) {
                if ( $r2 > 0 ) {
                    $err_diff_range = $ERROR_DIFF_RANGE;
                } else {
                    if (! exists $ENV{ACCT_FILTER}) {
                        $err_zero_metric = $ERROR_ZERO_METRIC;
                    }
                }
            } else {
                $err_acct_mismatch = $ERROR_ACCT_MISMATCH;
            }
        } else {
            $x = " ";
        }

        if (with_diff($i)) {
            $retStr .= sprintf("| %8s %8s %7s %7s %s", $r1, $r2, $diff, $rate, $x);
        } else {
            $retStr .= sprintf("| %8s %8s %s", $r1, $r2, $x);
        }
    }
    $retStr = $flagX . $retStr . sprintf("   %-10s", $fname);
    if ( exists $Rules{$fname} ) {
        $retStr .=  sprintf " := %s", join(" + ", @{$Rules{$fname}});
    }
    if ($discrepancy ne " ") {
        if ($err_acct_mismatch != 0) {
            $retVal = $err_acct_mismatch;
        }
        &set_retVal($err_zero_metric);
        &set_retVal($err_diff_range);
        &openDisFile();
        printf Scr1_fp "#%s\n#%s\n", $head_str, $retStr;
        &commandToScr1_fp(sprintf("%s %s 1", 'fsingle', $fname));
        printf Dis_fp "%s/%s\n", $fname, $discrepancy;
    } else {
    }
    return ($retStr);
}


sub checkUnknown()
{
    my ($total, $i, $R);

    sub checkUnknRate()
    {
        my ($name, $N) = @_;
        my ($val, $rate, $fmt);

        $val = $Erp->{$name}[$Comparison[$N][2]];
        $val = sprintf($R->{FMT}, $val);
        $rate = sprintf($R->{FMT},($val / $total) * 100);

	if ((! exists $ENV{ACCT_FILTER}) &&
	    ($val > $R->{'P_RANGE'}) && ($rate > $R->{'P_RATE'})) {
	    &set_retVal($ERROR_HIGH_UNKNOWN);
	    &openFsingleScr();
	    $fmt = "#%-8s %10s %10s %s\n";
	    printf Scr1_fp $fmt, $Comparison[$N][0], '%', '<Total>', $name;
	    printf Scr1_fp $fmt, ' ', $rate, $total, $val;
	    &commandToScr1_fp(sprintf("%s %s 1", 'fsingle', '<Total>'));
	    &commandToScr1_fp(sprintf("%s %s 1", 'csingle', '<Total>'));
	    &commandToScr1_fp(sprintf("%s %s 1", 'fsingle', $name));
	    &commandToScr1_fp(sprintf("%s %s 1", 'csingle', $name));
	    &closeFsingleScr();
	    return 1;
	}
	return 0;
    }

    return if ( ! exists $Erp->{'<Total>'} );
    return if ( $ENV{NOJAVA} );
    $R = $RANGE{'Unkn'};
    for $i ( 0..$#Comparison ) {
        $total = $Erp->{'<Total>'}[$Comparison[$i][2]];
        next if ( $total == 0 );
        $total = sprintf($R->{FMT}, $total);
#        last if &checkUnknRate('<Stack-unwind-failed>', $i);
        last if &checkUnknRate('<Unknown>', $i);
        last if &checkUnknRate('<no', $i);
    }
}

sub createDiff
{
    my ($key, $str, $i, $head_str);

    &checkUnknown();
    open(DIFF_fp, ">diff.out");
    $head_str = " ";
    for $i ( 0..$#Comparison ) {
        printf DIFF_fp "Comparison[%d]: %s,%d,%d\n", $i,
            $Comparison[$i][0], $Comparison[$i][1], $Comparison[$i][2], $Comparison[$i][3];
        $head_str .= &get_head_str($i);
    }
    printf DIFF_fp "\nX| Compare the acct file (first column) with the er_print output (second column):\n";
    $head_str .= "   Name";
    printf DIFF_fp "%s\n", $head_str;
    foreach $key (sort keys %Acct) {
        $str = &doComp($key, $head_str);
        printf DIFF_fp "%s\n", $str;
    }
    &checkCallersCallees;
    close(DIFF_fp);
    &closeDisFile();
    return -s "discrepancy.out"
}

sub createDiff2AVG
{
    my ($key, $str, $i, $n, $head_str, @avg, $temp, $fname);

    &checkUnknown();
    open(DIFF_fp, ">>diff.out");
    printf DIFF_fp "\n==================\n";
    $head_str = " ";
    for $i ( 0..$#Comparison ) {
        printf DIFF_fp "Comparison[%d]: %s,%d\n", $i,
            $Comparison[$i][0], $Comparison[$i][2];
        $head_str .= &get_head_str($i);
    }
    printf DIFF_fp "\n#| Compare the avg value (first column) with the er_print output (second column):\n";
    $head_str .= "   Name";
    printf DIFF_fp "%s\n", $head_str;
    for $i ( 0..$#Comparison ) {
        $avg[$i] = 0;
    }
    $n=0;
    foreach $fname (sort keys %Acct) {
        $n++;
        for $i ( 0..$#Comparison ) {
            if ( ! exists $Rules{$fname} ) {
                if ( exists $Erp->{$fname} ) {
                    $temp = $Erp->{$fname}[$Comparison[$i][2]];
                }
            } else {
                foreach my $key1 ( @{$Rules{$fname}} ) {
                    my $sign = 1;
                    $key = $key1;
                    if (substr($key1, 0, 1) eq '-') {
                        $key = substr($key1, 1);
                        $sign = -1;
                    }
                    if ( exists $Erp->{$key} ) {
                        $temp += $sign * $Erp->{$key}[$Comparison[$i][2]];
                    }
                }
            }
            $avg[$i] += $temp;
        }
    }
    for $i ( 0..$#Comparison ) {
            $avg[$i] /= $n;
    }

    foreach $key (sort keys %Acct) {
        $str = &doComp2AVG($key, $head_str, @avg);
        printf DIFF_fp "%s\n", $str;
    }
    close(DIFF_fp);
    &closeDisFile();
}

sub sumOutlinedCode
{   # Add a time of the outlined code.
    my ($name, $eName);
    foreach $name (keys %Acct) {
        foreach $eName (keys %$Erp) {
            next if ("$eName" !~ m/^($name)\s--/);
            if (defined $Rules{$name}) {
                push @{$Rules{$name}}, $eName;
            } else {
                $Rules{$name} = [$eName];
            }
        }
    }
}

sub checkCallersCallees
{
    my (@arr, $name, $colName, $line, $nline, %Calls);

    open(FP, "<caller_callee.out") or return;
    while ($line = <FP>) {
        last if ( $line =~ m/\s+sec.\s+/ );
    }
    $nline = 0;
    while ($line = <FP>) {
        chomp($line);
        $line =~ s/^\s*//;   # Delete leading spaces
        next if ( $line =~ m/^$/ );
        @arr = split (/\s+/, $line, $colName);
        $name = pop(@arr);
        # New Callers-Callees format does not have * in the Stack Fragment section
        # - translate old format to new format for compatibility
        if ($name eq "*MAIN") { $name = "MAIN"; };
        last if ($name eq "MAIN");
        $nline += 1;
    }
    if ($nline == 0) {
        printf "checkCallersCallees: No Callers of MAIN\n";
        &set_retVal($ERROR_CALLER_VERIF);
        close(FP);
        return;
    }
    while ($line = <FP>) {
        chomp($line);
        $line =~ s/^\s*//;   # Delete leading spaces
        next if ( $line =~ m/^$/ );
        @arr = split (/\s+/, $line, $colName);
        $name = pop(@arr);
        $Calls{$name} = 1;
        if ( $line =~ /Parallel/ ) { #f90synprog M_EXPERT or M_MACHINE
            @arr = split (/\s\s+/, $line, $colName);
            $name = pop(@arr);
            @arr = split (/\s/, $name);
            $Calls{$arr[0]} = 1;
        }
    }
    close(FP);

    foreach $name (sort keys %Acct) {
        next if ( $name eq '<Total>' ) ;
        next if ( $name eq '<Unknown>' ) ;
        next if (defined $Calls{$name}) ;
        printf "checkCallersCallees: '$name' is not inside callees\n";
        &set_retVal($ERROR_CALLER_VERIF);
    }
}


return 1;
END{}

