#!/usr/bin/perl -w
#
#
# Regenerate (overwriting only if changed):
#
#    scope_types.h
#
# from information contained in this file in the
# __DATA_ section below.
#
# To add a new type simply add its name to the list
# below in the correct section (marked by C comments)
# and then regenerate with 'make regen'.
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.

# The style of this file is determined by:
#
# perltidy -w -ple -bbb -bbc -bbs -nolq -l=80 -noll -nola -nwls='=' \
#   -isbc -nolc -otr -kis -ci=4 -se -sot -sct -nsbl -pt=2 -fs  \
#   -fsb='##!' -fse='##.'

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
}

use strict;
use warnings;

my %args= (
    "zero"  => 0,
    "one"   => 1,
    "two"   => 2,
    "three" => 3,
);
my $nargs= 0;
my @arg_num;
my @types;
my $tlen= 0;
my @lines;

foreach my $line (<DATA>) {
    $line =~ s/\s+\z//;
    if ($line =~ /(\w+) arg/) {
        $nargs= $args{$1} // die "panic: Bad arg number '$1'";
    }
    if ($line =~ /^SAVEt/) {
        my $id= 0 + @arg_num;
        $tlen= length($line) if $tlen < length($line);
        push @types,   $line;
        push @arg_num, [ $nargs, $line ];
        push @lines,   [ $line,  $id ];
    }
    else {
        push @lines, $line;
    }
}

my $c_code= "";
foreach my $num (0 .. $#lines) {
    my $line= $lines[$num];
    if (ref $line) {
        my ($type, $id)= @$line;
        $line= sprintf "#define %*s %*d",
            -$tlen, $type, length(0 + @types), $id;
    }
    $c_code .= $line . "\n";
}

$c_code .= <<EOF_C;

static const U8 leave_scope_arg_counts[] = {
EOF_C

foreach my $tuple (@arg_num) {
    my ($nargs, $type)= @$tuple;
    $c_code .= sprintf "    %d%s /* %*s */\n",
        $nargs, $tuple == $arg_num[-1] ? " " : ",",
        -$tlen, $type;
}
my $max_savet= $#arg_num;

$c_code .= <<EOF_C;
};

#define MAX_SAVEt $max_savet
EOF_C

my $final= <<'EOF_FINAL';
The defines and contents of the leave_scope_arg_counts[] array
must match. To add a new type modify the __DATA__ section in
regen/scope_types.pl and run `make regen` to rebuild the file.
EOF_FINAL

my $out= open_new(
    'scope_types.h',
    '>', {
        by        => 'regen/scope_types.pl',
        copyright => [2022],
        final     => $final,
    });
print $out $c_code;
read_only_bottom_close_and_rename($out);

__DATA__
/* zero args */

SAVEt_ALLOC
SAVEt_CLEARPADRANGE
SAVEt_CLEARSV
SAVEt_REGCONTEXT

/* one arg */

SAVEt_TMPSFLOOR
SAVEt_BOOL
SAVEt_COMPILE_WARNINGS
SAVEt_CURCOP_WARNINGS
SAVEt_COMPPAD
SAVEt_FREECOPHH
SAVEt_FREEOP
SAVEt_FREEPV
SAVEt_FREESV
SAVEt_I16
SAVEt_I32_SMALL
SAVEt_I8
SAVEt_INT_SMALL
SAVEt_MORTALIZESV
SAVEt_NSTAB
SAVEt_OP
SAVEt_PARSER
SAVEt_STACK_POS
SAVEt_READONLY_OFF
SAVEt_FREEPADNAME
SAVEt_STRLEN_SMALL
SAVEt_FREERCPV

/* two args */

SAVEt_AV
SAVEt_DESTRUCTOR
SAVEt_DESTRUCTOR_X
SAVEt_GENERIC_PVREF
SAVEt_GENERIC_SVREF
SAVEt_GP
SAVEt_GVSV
SAVEt_HINTS
SAVEt_HPTR
SAVEt_HV
SAVEt_I32
SAVEt_INT
SAVEt_ITEM
SAVEt_IV
SAVEt_LONG
SAVEt_PPTR
SAVEt_SAVESWITCHSTACK
SAVEt_SHARED_PVREF
SAVEt_SPTR
SAVEt_STRLEN
SAVEt_SV
SAVEt_SVREF
SAVEt_VPTR
SAVEt_ADELETE
SAVEt_APTR
SAVEt_RCPV

/* three args */

SAVEt_HELEM
SAVEt_PADSV_AND_MORTALIZE
SAVEt_SET_SVFLAGS
SAVEt_GVSLOT
SAVEt_AELEM
SAVEt_DELETE
SAVEt_HINTS_HH
