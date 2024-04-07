#!/usr/bin/perl -w
#
# Regenerate (overwriting only if changed):
#
#    overload.h
#    overload.inc
#    lib/overload/numbers.pm
#
# from information stored in the DATA section of this file.
#
# This allows the order of overloading constants to be changed.
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
}

use strict;

my (@enums, @names);
while (<DATA>) {
  next if /^#/;
  next if /^$/;
  my ($enum, $name) = /^(\S+)\s+(\S+)/ or die "Can't parse $_";
  push @enums, $enum;
  push @names, $name;
}

my ($c, $h) = map {
    open_new($_, '>',
             { by => 'regen/overload.pl', file => $_, style => '*',
               copyright => [1997, 1998, 2000, 2001, 2005 .. 2007, 2011] });
} 'overload.inc', 'overload.h';

mkdir("lib/overload", 0777) unless -d 'lib/overload';
my $p = open_new('lib/overload/numbers.pm', '>',
                 { by => 'regen/overload.pl',
                   file => 'lib/overload/numbers.pm', copyright => [2008] });

{
local $" = "\n    ";
print $p <<"EOF";
package overload::numbers;

our \@names = qw#
    @names
#;

our \@enums = qw#
    @enums
#;

{ my \$i = 0; our %names = map { \$_ => \$i++ } \@names }

{ my \$i = 0; our %enums = map { \$_ => \$i++ } \@enums }
EOF
}

print $h "enum {\n";

for (0..$#enums) {
    my $op = $names[$_];
    $op = 'fallback' if $op eq '()';
    $op =~ s/^\(//;
    die if $op =~ m{\*/};
    my $l =   3 - int((length($enums[$_]) + 9) / 8);
    $l = 1 if $l < 1;
    printf $h "    %s_amg,%s/* 0x%02x %-8s */\n", $enums[$_],
        ("\t" x $l), $_, $op;
}

print $h <<'EOF';
    max_amg_code
    /* Do not leave a trailing comma here.  C9X allows it, C89 doesn't. */
};

#define NofAMmeth max_amg_code
EOF

print $c <<'EOF';
#define AMG_id2name(id) (PL_AMG_names[id]+1)
#define AMG_id2namelen(id) (PL_AMG_namelens[id]-1)

static const U8 PL_AMG_namelens[NofAMmeth] = {
EOF

my $last = pop @names;

print $c map { "    " . (length $_) . ",\n" } @names;

my $lastlen = length $last;
print $c <<"EOT";
    $lastlen
};

static const char * const PL_AMG_names[NofAMmeth] = {
  /* Names kept in the symbol table.  fallback => "()", the rest has
     "(" prepended.  The only other place in perl which knows about
     this convention is AMG_id2name (used for debugging output and
     'nomethod' only), the only other place which has it hardwired is
     overload.pm.  */
EOT

for (0..$#names) {
    my $n = $names[$_];
    $n =~ s/(["\\])/\\$1/g;
    my $l =   3 - int((length($n) + 7) / 8);
    $l = 1 if $l < 1;
    printf $c "    \"%s\",%s/* %-10s */\n", $n, ("\t" x $l), $enums[$_];
}

print $c <<"EOT";
    "$last"
};
EOT

foreach ($h, $c, $p) {
    read_only_bottom_close_and_rename($_);
}
__DATA__
# Fallback should be the first
fallback	()

# These 5 are the most common in the fallback switch statement in amagic_call
to_sv		(${}
to_av		(@{}
to_hv		(%{}
to_gv		(*{}
to_cv		(&{}

# These have non-default cases in that switch statement
inc		(++
dec		(--
bool_		(bool
numer		(0+
string		(""
not		(!
copy		(=
abs		(abs
neg		(neg
iter		(<>
int		(int

# These 12 feature in the next switch statement
lt		(<
le		(<=
gt		(>
ge		(>=
eq		(==
ne		(!=
slt		(lt
sle		(le
sgt		(gt
sge		(ge
seq		(eq
sne		(ne

nomethod	(nomethod
add		(+
add_ass		(+=
subtr		(-
subtr_ass	(-=
mult		(*
mult_ass	(*=
div		(/
div_ass		(/=
modulo		(%
modulo_ass	(%=
pow		(**
pow_ass		(**=
lshift		(<<
lshift_ass	(<<=
rshift		(>>
rshift_ass	(>>=
band		(&
band_ass	(&=
sband		(&.
sband_ass	(&.=
bor		(|
bor_ass		(|=
sbor		(|.
sbor_ass	(|.=
bxor		(^
bxor_ass	(^=
sbxor		(^.
sbxor_ass	(^.=
ncmp		(<=>
scmp		(cmp
compl		(~
scompl		(~.
atan2		(atan2
cos		(cos
sin		(sin
exp		(exp
log		(log
sqrt		(sqrt
repeat		(x
repeat_ass	(x=
concat		(.
concat_ass	(.=
smart		(~~
ftest           (-X
regexp          (qr
