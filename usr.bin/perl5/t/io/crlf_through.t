#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

no warnings 'once';
$main::use_crlf = 1;
do './io/through.t' or die "no kid script";
