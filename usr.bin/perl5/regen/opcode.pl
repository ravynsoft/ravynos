#!/usr/bin/perl -w
# 
# Regenerate (overwriting only if changed):
#
#    opcode.h
#    opnames.h
#    pp_proto.h
#    lib/B/Op_private.pm
#
# from:
#  * information stored in regen/opcodes;
#  * information stored in regen/op_private (which is actually perl code);
#  * the values hardcoded into this script in @raw_alias.
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.

use v5.26;
use warnings;

sub generate_opcode_h;
sub generate_opcode_h_epilogue;
sub generate_opcode_h_prologue;
sub generate_opcode_h_defines;
sub generate_opcode_h_opnames;
sub generate_opcode_h_pl_check;
sub generate_opcode_h_pl_opargs;
sub generate_opcode_h_pl_ppaddr;

sub generate_opnames_h;
sub generate_opnames_h_opcode_enum;
sub generate_opnames_h_epilogue;
sub generate_opnames_h_opcode_predicates;

sub generate_pp_proto_h;

sub generate_b_op_private_pm;

my $restrict_to_core = "if defined(PERL_CORE) || defined(PERL_EXT)";

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
}

# Read 'opcodes' data.

my %seen;
my (@ops, %desc, %check, %ckname, %flags, %args, %opnum);

open OPS, '<', 'regen/opcodes' or die $!;

while (<OPS>) {
    chop;
    next unless $_;
    next if /^#/;
    my ($key, $desc, $check, $flags, $args) = split(/\t+/, $_, 5);
    $args = '' unless defined $args;

    warn qq[Description "$desc" duplicates $seen{$desc}\n]
     if $seen{$desc} and $key !~ "concat|transr|(?:intro|clone)cv|lvref";
    die qq[Opcode "$key" duplicates $seen{$key}\n] if $seen{$key};
    die qq[Opcode "freed" is reserved for the slab allocator\n]
        if $key eq 'freed';
    $seen{$desc} = qq[description of opcode "$key"];
    $seen{$key} = qq[opcode "$key"];

    push(@ops, $key);
    $opnum{$key} = $#ops;
    $desc{$key} = $desc;
    $check{$key} = $check;
    $ckname{$check}++;
    $flags{$key} = $flags;
    $args{$key} = $args;
}

# Set up aliases

my %alias;

# Format is "this function" => "does these op names"
my @raw_alias = (
                 Perl_do_kv => [qw( keys values )],
                 Perl_unimplemented_op => [qw(padany custom)],
                 # All the ops with a body of { return NORMAL; }
                 Perl_pp_null => [qw(scalar regcmaybe lineseq scope)],

                 Perl_pp_goto => ['dump'],
                 Perl_pp_require => ['dofile'],
                 Perl_pp_untie => ['dbmclose'],
                 Perl_pp_sysread => {read => '', recv => '#ifdef HAS_SOCKET'},
                 Perl_pp_sysseek => ['seek'],
                 Perl_pp_ioctl => ['fcntl'],
                 Perl_pp_ssockopt => {gsockopt => '#ifdef HAS_SOCKET'},
                 Perl_pp_getpeername => {getsockname => '#ifdef HAS_SOCKET'},
                 Perl_pp_stat => ['lstat'],
                 Perl_pp_ftrowned => [qw(fteowned ftzero ftsock ftchr ftblk
                                         ftfile ftdir ftpipe ftsuid ftsgid
                                         ftsvtx)],
                 Perl_pp_fttext => ['ftbinary'],
                 Perl_pp_gmtime => ['localtime'],
                 Perl_pp_semget => [qw(shmget msgget)],
                 Perl_pp_semctl => [qw(shmctl msgctl)],
                 Perl_pp_ghostent => [qw(ghbyname ghbyaddr)],
                 Perl_pp_gnetent => [qw(gnbyname gnbyaddr)],
                 Perl_pp_gprotoent => [qw(gpbyname gpbynumber)],
                 Perl_pp_gservent => [qw(gsbyname gsbyport)],
                 Perl_pp_gpwent => [qw(gpwnam gpwuid)],
                 Perl_pp_ggrent => [qw(ggrnam ggrgid)],
                 Perl_pp_ftis => [qw(ftsize ftmtime ftatime ftctime)],
                 Perl_pp_chown => [qw(unlink chmod utime kill)],
                 Perl_pp_link => ['symlink'],
                 Perl_pp_ftrread => [qw(ftrwrite ftrexec fteread ftewrite
                                        fteexec)],
                 Perl_pp_shmwrite => [qw(shmread msgsnd msgrcv semop)],
                 Perl_pp_syswrite => {send => '#ifdef HAS_SOCKET'},
                 Perl_pp_defined => [qw(dor dorassign)],
                 Perl_pp_and => ['andassign'],
                 Perl_pp_or => ['orassign'],
                 Perl_pp_ucfirst => ['lcfirst'],
                 Perl_pp_sle => [qw(slt sgt sge)],
                 Perl_pp_print => ['say'],
                 Perl_pp_index => ['rindex'],
                 Perl_pp_oct => ['hex'],
                 Perl_pp_shift => ['pop'],
                 Perl_pp_sin => [qw(cos exp log sqrt)],
                 Perl_pp_bit_or => ['bit_xor'],
                 Perl_pp_nbit_or => ['nbit_xor'],
                 Perl_pp_sbit_or => ['sbit_xor'],
                 Perl_pp_rv2av => ['rv2hv'],
                 Perl_pp_akeys => ['avalues'],
                 Perl_pp_trans => [qw(trans transr)],
                 Perl_pp_chop => [qw(chop chomp)],
                 Perl_pp_schop => [qw(schop schomp)],
                 Perl_pp_bind => {connect => '#ifdef HAS_SOCKET'},
                 Perl_pp_preinc => ['i_preinc'],
                 Perl_pp_predec => ['i_predec'],
                 Perl_pp_postinc => ['i_postinc'],
                 Perl_pp_postdec => ['i_postdec'],
                 Perl_pp_ehostent => [qw(enetent eprotoent eservent
                                         spwent epwent sgrent egrent)],
                 Perl_pp_shostent => [qw(snetent sprotoent sservent)],
                 Perl_pp_aelemfast => ['aelemfast_lex'],
                 Perl_pp_grepstart => ['mapstart'],
                );

while (my ($func, $names) = splice @raw_alias, 0, 2) {
    if (ref $names eq 'ARRAY') {
        foreach (@$names) {
            $alias{$_} = [$func, ''];
        }
    } else {
        while (my ($opname, $cond) = each %$names) {
            $alias{$opname} = [$func, $cond];
        }
    }
}

foreach my $sock_func (qw(socket bind listen accept shutdown
                          ssockopt getpeername)) {
    $alias{$sock_func} = ["Perl_pp_$sock_func", '#ifdef HAS_SOCKET'],
}



# =================================================================
#
# Functions for processing regen/op_private data.
#
# Put them in a separate package so that croak() does the right thing

package OP_PRIVATE;

use Carp;


# the vars holding the global state built up by all the calls to addbits()


# map OPpLVAL_INTRO => LVINTRO
my %LABELS;


# the numeric values of flags - what will get output as a #define
my %DEFINES;

# %BITFIELDS: the various bit field types. The key is the concatenation of
# all the field values that make up a bit field hash; the values are bit
# field hash refs.  This allows us to de-dup identical bit field defs
# across different ops, and thus make the output tables more compact (esp
# important for the C version)
my %BITFIELDS;

# %FLAGS: the main data structure. Indexed by op name, then bit index:
# single bit flag:
#   $FLAGS{rv2av}{2} = 'OPpSLICEWARNING';
# bit field (bits 5 and 6):
#   $FLAGS{rv2av}{5} = $FLAGS{rv2av}{6} = { .... };
my %FLAGS;


# do, with checking, $LABELS{$name} = $label

sub add_label {
    my ($name, $label) = @_;
    if (exists $LABELS{$name} and $LABELS{$name} ne $label) {
        croak "addbits(): label for flag '$name' redefined:\n"
        .  "  was '$LABELS{$name}', now '$label'";
    }
    $LABELS{$name} = $label;
}

#
# do, with checking, $DEFINES{$name} = $val

sub add_define {
    my ($name, $val) = @_;
    if (exists $DEFINES{$name} && $DEFINES{$name} != $val) {
        croak "addbits(): value for flag '$name' redefined:\n"
        .  "  was $DEFINES{$name}, now $val";
    }
    $DEFINES{$name} = $val;
}


# intended to be called from regen/op_private; see that file for details

sub ::addbits {
    my @args = @_;

    croak "too few arguments for addbits()" unless @args >= 3;
    my $op = shift @args;
    croak "invalid op name: '$op'" unless exists $opnum{$op};

    while (@args) {
        my $bits = shift @args;
        if ($bits =~ /^[0-7]$/) {
            # single bit
            croak "addbits(): too few arguments for single bit flag"
                unless @args >= 2;
            my $flag_name   = shift @args;
            my $flag_label  = shift @args;
            add_label($flag_name, $flag_label);
            croak "addbits(): bit $bits of $op already specified ($FLAGS{$op}{$bits})"
                if defined $FLAGS{$op}{$bits};
            $FLAGS{$op}{$bits} = $flag_name;
            add_define($flag_name, (1 << $bits));
        }
        elsif ($bits =~ /^([0-7])\.\.([0-7])$/) {
            # bit range
            my ($bitmin, $bitmax) = ($1,$2);

            croak "addbits(): min bit > max bit in bit range '$bits'"
                unless $bitmin <= $bitmax;
            croak "addbits(): bit field argument missing"
                unless @args >= 1;

            my $arg_hash = shift @args;
            croak "addbits(): arg to $bits must be a hash ref"
                unless defined $arg_hash and ref($arg_hash) =~ /HASH/;

            my %valid_keys;
            @valid_keys{qw(baseshift_def bitcount_def mask_def label enum)} = ();
            for (keys %$arg_hash) {
                croak "addbits(): unrecognised bifield key: '$_'"
                    unless exists $valid_keys{$_};
            }

            my $bitmask = 0;
            $bitmask += (1 << $_) for $bitmin..$bitmax;

            my $enum_id ='';

            if (defined $arg_hash->{enum}) {
                my $enum = $arg_hash->{enum};
                croak "addbits(): arg to enum must be an array ref"
                    unless defined $enum and ref($enum) =~ /ARRAY/;
                croak "addbits(): enum list must be in triplets"
                    unless @$enum % 3 == 0;

                my $max_id = (1 << ($bitmax - $bitmin + 1)) - 1;

                my @e = @$enum;
                while (@e) {
                    my $enum_ix     = shift @e;
                    my $enum_name   = shift @e;
                    my $enum_label  = shift @e;
                    croak "addbits(): enum index must be a number: '$enum_ix'"
                        unless $enum_ix =~ /^\d+$/;
                    croak "addbits(): enum index too big: '$enum_ix'"
                        unless $enum_ix  <= $max_id;
                    add_label($enum_name, $enum_label);
                    add_define($enum_name, $enum_ix << $bitmin);
                    $enum_id .= "($enum_ix:$enum_name:$enum_label)";
                }
            }

            # id is a fingerprint of all the content of the bit field hash
            my $id = join ':', map defined() ? $_ : "-undef-",
                $bitmin, $bitmax,
                $arg_hash->{label},
                $arg_hash->{mask_def},
                $arg_hash->{baseshift_def},
                $arg_hash->{bitcount_def},
                $enum_id;

            unless (defined $BITFIELDS{$id}) {

                if (defined $arg_hash->{mask_def}) {
                    add_define($arg_hash->{mask_def}, $bitmask);
                }

                if (defined $arg_hash->{baseshift_def}) {
                    add_define($arg_hash->{baseshift_def}, $bitmin);
                }

                if (defined $arg_hash->{bitcount_def}) {
                    add_define($arg_hash->{bitcount_def}, $bitmax-$bitmin+1);
                }

                # create deep copy

                my $copy = {};
                for (qw(baseshift_def  bitcount_def mask_def label)) {
                    $copy->{$_} = $arg_hash->{$_} if defined $arg_hash->{$_};
                }
                if (defined $arg_hash->{enum}) {
                    $copy->{enum} = [ @{$arg_hash->{enum}} ];
                }

                # and add some extra fields

                $copy->{bitmask} = $bitmask;
                $copy->{bitmin} = $bitmin;
                $copy->{bitmax} = $bitmax;

                $BITFIELDS{$id} = $copy;
            }

            for my $bit ($bitmin..$bitmax) {
                croak "addbits(): bit $bit of $op already specified ($FLAGS{$op}{$bit})"
                    if defined $FLAGS{$op}{$bit};
                $FLAGS{$op}{$bit} = $BITFIELDS{$id};
            }
        }
        else {
            croak "addbits(): invalid bit specifier '$bits'";
        }
    }
}


# intended to be called from regen/op_private; see that file for details

sub ::ops_with_flag {
    my $flag = shift;
    return grep $flags{$_} =~ /\Q$flag/, sort keys %flags;
}


# intended to be called from regen/op_private; see that file for details

sub ::ops_with_check {
    my $c = shift;
    return grep $check{$_} eq $c, sort keys %check;
}


# intended to be called from regen/op_private; see that file for details

sub ::ops_with_arg {
    my ($i, $arg_type) = @_;
    my @ops;
    for my $op (sort keys %args) {
        my @args = split(' ',$args{$op});
        push @ops, $op if defined $args[$i] and $args[$i] eq $arg_type;
    }
    @ops;
}


# output '#define OPpLVAL_INTRO 0x80' etc

sub print_defines {
    my $fh = shift;

    for (sort { $DEFINES{$a} <=> $DEFINES{$b} || $a cmp $b } keys %DEFINES) {
        printf $fh "#define %-23s 0x%02x\n", $_, $DEFINES{$_};
    }
}


# Generate the content of B::Op_private

sub print_B_Op_private {
    my $fh = shift;

    my $header = <<'EOF';
@=head1 NAME
@
@B::Op_private - OP op_private flag definitions
@
@=head1 SYNOPSIS
@
@    use B::Op_private;
@
@    # flag details for bit 7 of OP_AELEM's op_private:
@    my $name  = $B::Op_private::bits{aelem}{7}; # OPpLVAL_INTRO
@    my $value = $B::Op_private::defines{$name}; # 128
@    my $label = $B::Op_private::labels{$name};  # LVINTRO
@
@    # the bit field at bits 5..6 of OP_AELEM's op_private:
@    my $bf  = $B::Op_private::bits{aelem}{6};
@    my $mask = $bf->{bitmask}; # etc
@
@=head1 DESCRIPTION
@
@This module provides four global hashes:
@
@    %B::Op_private::bits
@    %B::Op_private::defines
@    %B::Op_private::labels
@    %B::Op_private::ops_using
@
@which contain information about the per-op meanings of the bits in the
@op_private field.
@
@=head2 C<%bits>
@
@This is indexed by op name and then bit number (0..7). For single bit flags,
@it returns the name of the define (if any) for that bit:
@
@   $B::Op_private::bits{aelem}{7} eq 'OPpLVAL_INTRO';
@
@For bit fields, it returns a hash ref containing details about the field.
@The same reference will be returned for all bit positions that make
@up the bit field; so for example these both return the same hash ref:
@
@    $bitfield = $B::Op_private::bits{aelem}{5};
@    $bitfield = $B::Op_private::bits{aelem}{6};
@
@The general format of this hash ref is
@
@    {
@        # The bit range and mask; these are always present.
@        bitmin        => 5,
@        bitmax        => 6,
@        bitmask       => 0x60,
@
@        # (The remaining keys are optional)
@
@        # The names of any defines that were requested:
@        mask_def      => 'OPpFOO_MASK',
@        baseshift_def => 'OPpFOO_SHIFT',
@        bitcount_def  => 'OPpFOO_BITS',
@
@        # If present, Concise etc will display the value with a 'FOO='
@        # prefix. If it equals '-', then Concise will treat the bit
@        # field as raw bits and not try to interpret it.
@        label         => 'FOO',
@
@        # If present, specifies the names of some defines and the
@        # display labels that are used to assign meaning to particu-
@        # lar integer values within the bit field; e.g. 3 is dis-
@        # played as 'C'.
@        enum          => [ qw(
@                             1   OPpFOO_A  A
@                             2   OPpFOO_B  B
@                             3   OPpFOO_C  C
@                         )],
@
@    };
@
@
@=head2 C<%defines>
@
@This gives the value of every C<OPp> define, e.g.
@
@    $B::Op_private::defines{OPpLVAL_INTRO} == 128;
@
@=head2 C<%labels>
@
@This gives the short display label for each define, as used by C<B::Concise>
@and C<perl -Dx>, e.g.
@
@    $B::Op_private::labels{OPpLVAL_INTRO} eq 'LVINTRO';
@
@If the label equals '-', then Concise will treat the bit as a raw bit and
@not try to display it symbolically.
@
@=head2 C<%ops_using>
@
@For each define, this gives a reference to an array of op names that use
@the flag.
@
@    @ops_using_lvintro = @{ $B::Op_private::ops_using{OPp_LVAL_INTRO} };
@
@=cut

package B::Op_private;

our %bits;

EOF
    # remove podcheck.t-defeating leading char
    $header =~ s/^\@//gm;
    print $fh $header;
    my $v = (::perl_version())[3];
    print $fh qq{\nour \$VERSION = "$v";\n\n};

    my %ops_using;

    # for each flag/bit combination, find the ops which use it
    my %combos;
    for my $op (sort keys %FLAGS) {
        my $entry = $FLAGS{$op};
        for my $bit (0..7) {
            my $e = $entry->{$bit};
            next unless defined $e;
            next if ref $e; # bit field, not flag
            push @{$combos{$e}{$bit}}, $op;
            push @{$ops_using{$e}}, $op;
        }
    }

    # dump flags used by multiple ops
    for my $flag (sort keys %combos) {
        for my $bit (sort keys %{$combos{$flag}}) {
            my $ops = $combos{$flag}{$bit};
            next unless @$ops > 1;
            my @o = sort @$ops;
            print $fh "\$bits{\$_}{$bit} = '$flag' for qw(@o);\n";
        }
    }

    # dump bit field definitions

    my %bitfield_ix;
    {
        my %bitfields;
        # stringified-ref to ref mapping
        $bitfields{$_} = $_ for values %BITFIELDS;
        my $ix = -1;
        my $s = "\nmy \@bf = (\n";
        for my $bitfield_key (sort keys %BITFIELDS) {
            my $bitfield = $BITFIELDS{$bitfield_key};
            $ix++;
            $bitfield_ix{$bitfield} = $ix;

            $s .= "    {\n";
            for (qw(label mask_def baseshift_def bitcount_def)) {
                next unless defined $bitfield->{$_};
                $s .= sprintf "        %-9s => '%s',\n",
                            $_,  $bitfield->{$_};
            }
            for (qw(bitmin bitmax bitmask)) {
                croak "panic" unless defined $bitfield->{$_};
                $s .= sprintf "        %-9s => %d,\n",
                            $_,  $bitfield->{$_};
            }
            if (defined $bitfield->{enum}) {
                $s .= "        enum      => [\n";
                my @enum = @{$bitfield->{enum}};
                while (@enum) {
                    my $i     = shift @enum;
                    my $name  = shift @enum;
                    my $label = shift @enum;
                    $s .= sprintf "            %d, %-10s, %s,\n",
                            $i, "'$name'", "'$label'";
                }
                $s .= "        ],\n";
            }
            $s .= "    },\n";

        }
        $s .= ");\n";
        print $fh "$s\n";
    }

    # dump bitfields and remaining labels

    for my $op (sort keys %FLAGS) {
        my @indices;
        my @vals;
        my $entry = $FLAGS{$op};
        my $bit;

        for ($bit = 7; $bit >= 0; $bit--) {
            next unless defined $entry->{$bit};
            my $e = $entry->{$bit};
            if (ref $e) {
                my $ix = $bitfield_ix{$e};
                for (reverse $e->{bitmin}..$e->{bitmax}) {
                    push @indices,  $_;
                    push @vals, "\$bf[$ix]";
                }
                $bit = $e->{bitmin};
            }
            else {
                next if @{$combos{$e}{$bit}} > 1;  # already output
                push @indices, $bit;
                push @vals, "'$e'";
            }
        }
        if (@indices) {
            my $s = '';
            $s = '@{' if @indices > 1;
            $s .= "\$bits{$op}";
            $s .= '}' if @indices > 1;
            $s .= '{' . join(',', @indices) . '} = ';
            $s .= '(' if @indices > 1;
            $s .= join ', ', @vals;
            $s .= ')' if @indices > 1;
            print $fh "$s;\n";
        }
    }

    # populate %defines and %labels

    print  $fh "\n\nour %defines = (\n";
    printf $fh "    %-23s  => %3d,\n", $_ , $DEFINES{$_} for sort keys %DEFINES;
    print  $fh ");\n\nour %labels = (\n";
    printf $fh "    %-23s  => '%s',\n", $_ , $LABELS{$_}  for sort keys %LABELS;
    print  $fh ");\n";

    # %ops_using
    print  $fh "\n\nour %ops_using = (\n";
    # Save memory by using the same array wherever possible.
    my %flag_by_op_list;
    my $pending = '';
    for my $flag (sort keys %ops_using) {
        my $op_list = $ops_using{$flag} = "@{$ops_using{$flag}}";
        if (!exists $flag_by_op_list{$op_list}) {
            $flag_by_op_list{$op_list} = $flag;
            printf $fh "    %-23s  => %s,\n", $flag , "[qw($op_list)]"
        }
        else {
            $pending .= "\$ops_using{$flag} = "
                      . "\$ops_using{$flag_by_op_list{$op_list}};\n";
        }
    }
    print  $fh ");\n\n$pending";

}



# output the contents of the assorted PL_op_private_*[] tables

sub print_PL_op_private_tables {
    my $fh = shift;

    my $PL_op_private_labels     = '';
    my $PL_op_private_valid      = '';
    my $PL_op_private_bitdef_ix  = '';
    my $PL_op_private_bitdefs    = '';
    my $PL_op_private_bitfields  = '';

    my %label_ix;
    my %bitfield_ix;

    # generate $PL_op_private_labels

    {
        my %labs;
        $labs{$_} = 1 for values %LABELS; # de-duplicate labels
        # add in bit field labels
        for (values %BITFIELDS) {
            next unless defined $_->{label};
            $labs{$_->{label}} = 1;
        }

        my $labels = '';
        for my $lab (sort keys %labs) {
            $label_ix{$lab} = length $labels;
            $labels .= "$lab\0";
            $PL_op_private_labels .=
                  "    "
                . join(',', map("'$_'", split //, $lab))
                . ",'\\0',\n";
        }
    }


    # generate PL_op_private_bitfields

    {
        my %bitfields;
        # stringified-ref to ref mapping
        $bitfields{$_} = $_ for values %BITFIELDS;

        my $ix = 0;
        for my $bitfield_key (sort keys %BITFIELDS) {
            my $bf = $BITFIELDS{$bitfield_key};
            $bitfield_ix{$bf} = $ix;

            my @b;
            push @b, $bf->{bitmin},
                defined $bf->{label} ? $label_ix{$bf->{label}} : -1;
            my $enum = $bf->{enum};
            if (defined $enum) {
                my @enum = @$enum;
                while (@enum) {
                    my $i     = shift @enum;
                    my $name  = shift @enum;
                    my $label = shift @enum;
                    push @b, $i, $label_ix{$label};
                }
            }
            push @b, -1; # terminate enum list

            $PL_op_private_bitfields .= "    " .  join(', ', @b) .",\n";
            $ix += @b;
        }
    }


    # generate PL_op_private_bitdefs, PL_op_private_bitdef_ix

    {
        my $bitdef_count = 0;

        my %not_seen = %FLAGS;
        my @seen_bitdefs;
        my %seen_bitdefs;

        my $opnum = -1;
        for my $op (sort { $opnum{$a} <=> $opnum{$b} } keys %opnum) {
            $opnum++;
            die "panic: opnum misorder: opnum=$opnum opnum{op}=$opnum{$op}"
                unless $opnum == $opnum{$op};
            delete $not_seen{$op};

            my @bitdefs;
            my $entry = $FLAGS{$op};
            my $bit;
            my $index;

            for ($bit = 7; $bit >= 0; $bit--) {
                my $e = $entry->{$bit};
                next unless defined $e;

                my $ix;
                if (ref $e) {
                    $ix = $bitfield_ix{$e};
                    die "panic: \$bit =\= $e->{bitmax}"
                        unless $bit == $e->{bitmax};

                    push @bitdefs, ( ($ix << 5) | ($bit << 2) | 2 );
                    $bit = $e->{bitmin};
                }
                else {
                    $ix = $label_ix{$LABELS{$e}};
                    die "panic: no label ix for '$e'" unless defined $ix;
                    push @bitdefs, ( ($ix << 5) | ($bit << 2));
                }
                if ($ix > 2047) {
                    die "Too many labels or bitfields (ix=$ix): "
                    . "maybe the type of PL_op_private_bitdefs needs "
                    . "expanding from U16 to U32???";
                }
            }
            if (@bitdefs) {
                $bitdefs[-1] |= 1; # stop bit
                my $key = join(', ', map(sprintf("0x%04x", $_), @bitdefs));
                if (!$seen_bitdefs{$key}) {
                    $index = $bitdef_count;
                    $bitdef_count += @bitdefs;
                    push @seen_bitdefs,
                         $seen_bitdefs{$key} = [$index, $key];
                }
                else {
                    $index = $seen_bitdefs{$key}[0];
                }
                push @{$seen_bitdefs{$key}}, $op;
            }
            else {
                $index = -1;
            }
            $PL_op_private_bitdef_ix .= sprintf "    %4d, /* %s */\n", $index, $op;
        }
        if (%not_seen) {
            die "panic: unprocessed ops: ". join(',', keys %not_seen);
        }
        for (@seen_bitdefs) {
            local $" = ", ";
            $PL_op_private_bitdefs .= "    $$_[1], /* @$_[2..$#$_] */\n";
        }
    }


    # generate PL_op_private_valid

    for my $op (@ops) {
        my $last;
        my @flags;
        for my $bit (0..7) {
            next unless exists $FLAGS{$op};
            my $entry = $FLAGS{$op}{$bit};
            next unless defined $entry;
            if (ref $entry) {
                # skip later entries for the same bit field
                next if defined $last and $last == $entry;
                $last = $entry;
                push @flags,
                    defined $entry->{mask_def}
                        ? $entry->{mask_def}
                        : $entry->{bitmask};
            }
            else {
                push @flags, $entry;
            }
        }

        # all bets are off
        @flags = '0xff' if $op eq 'null' or $op eq 'custom';

        $PL_op_private_valid .= sprintf "    /* %-10s */ (%s),\n", uc($op),
                                    @flags ? join('|', @flags): '0';
    }

    print $fh <<EOF;
START_EXTERN_C

#ifndef DOINIT

/* data about the flags in op_private */

EXTCONST I16  PL_op_private_bitdef_ix[];
EXTCONST U16  PL_op_private_bitdefs[];
EXTCONST char PL_op_private_labels[];
EXTCONST I16  PL_op_private_bitfields[];
EXTCONST U8   PL_op_private_valid[];

#else


/* PL_op_private_labels[]: the short descriptions of private flags.
 * All labels are concatenated into a single char array
 * (separated by \\0's) for compactness.
 */

EXTCONST char PL_op_private_labels[] = {
$PL_op_private_labels
};



/* PL_op_private_bitfields[]: details about each bit field type.
 * Each definition consists of the following list of words:
 *    bitmin
 *    label (index into PL_op_private_labels[]; -1 if no label)
 *    repeat for each enum entry (if any):
 *       enum value
 *       enum label (index into PL_op_private_labels[])
 *    -1
 */

EXTCONST I16 PL_op_private_bitfields[] = {
$PL_op_private_bitfields
};


/* PL_op_private_bitdef_ix[]: map an op number to a starting position
 * in PL_op_private_bitdefs.  If -1, the op has no bits defined */

EXTCONST I16  PL_op_private_bitdef_ix[] = {
$PL_op_private_bitdef_ix
};



/* PL_op_private_bitdefs[]: given a starting position in this array (as
 * supplied by PL_op_private_bitdef_ix[]), each word (until a stop bit is
 * seen) defines the meaning of a particular op_private bit for a
 * particular op. Each word consists of:
 *  bit  0:     stop bit: this is the last bit def for the current op
 *  bit  1:     bitfield: if set, this defines a bit field rather than a flag
 *  bits 2..4:  unsigned number in the range 0..7 which is the bit number
 *  bits 5..15: unsigned number in the range 0..2047 which is an index
 *              into PL_op_private_labels[]    (for a flag), or
 *              into PL_op_private_bitfields[] (for a bit field)
 */

EXTCONST U16  PL_op_private_bitdefs[] = {
$PL_op_private_bitdefs
};


/* PL_op_private_valid: for each op, indexed by op_type, indicate which
 * flags bits in op_private are legal */

EXTCONST U8 PL_op_private_valid[] = {
$PL_op_private_valid
};

#endif /* !DOINIT */

END_EXTERN_C


EOF

}


# =================================================================


package main;

# read regen/op_private data
#
# This file contains Perl code that builds up some data structures
# which define what bits in op_private have what meanings for each op.
# It populates %LABELS, %DEFINES, %FLAGS, %BITFIELDS.

require './regen/op_private';

#use Data::Dumper;
#print Dumper \%LABELS, \%DEFINES, \%FLAGS, \%BITFIELDS;

# Emit allowed argument types.

my $ARGBITS = 32;

my %argnum = (
    'S',  1,        # scalar
    'L',  2,        # list
    'A',  3,        # array value
    'H',  4,        # hash value
    'C',  5,        # code value
    'F',  6,        # file value
    'R',  7,        # scalar reference
);

my %opclass = (
    '0',  0,        # baseop
    '1',  1,        # unop
    '2',  2,        # binop
    '|',  3,        # logop
    '@',  4,        # listop
    '/',  5,        # pmop
    '$',  6,        # svop_or_padop
    '#',  7,        # padop
    '"',  8,        # pvop_or_svop
    '{',  9,        # loop
    ';',  10,       # cop
    '%',  11,       # baseop_or_unop
    '-',  12,       # filestatop
    '}',  13,       # loopexop
    '.',  14,       # methop
    '+',  15,       # unop_aux
);

my %opflags = (
    'm' =>   1,     # needs stack mark
    'f' =>   2,     # fold constants
    's' =>   4,     # always produces scalar
    't' =>   8,     # needs target scalar
    'T' =>   8 | 16,    # ... which may be lexical
    'i' =>   0,     # always produces integer (unused since e7311069)
    'I' =>  32,     # has corresponding int op
    'd' =>  64,     # danger, make temp copy in list assignment
    'u' => 128,     # defaults to $_
);

generate_opcode_h;
generate_opnames_h;
generate_pp_proto_h;
generate_b_op_private_pm;

sub gen_op_is_macro {
    my ($op_is, $macname) = @_;
    if (keys %$op_is) {
        
        # get opnames whose numbers are lowest and highest
        my ($first, @rest) = sort {
            $op_is->{$a} <=> $op_is->{$b}
        } keys %$op_is;
        
        my $last = pop @rest;   # @rest slurped, get its last
        die "Invalid range of ops: $first .. $last\n" unless $last;

        print "\n#define $macname(op)   \\\n\t(";

        # verify that op-ct matches 1st..last range (and fencepost)
        # (we know there are no dups)
        if ( $op_is->{$last} - $op_is->{$first} == scalar @rest + 1) {
            
            # contiguous ops -> optimized version
            print "(op) >= OP_" . uc($first)
                . " && (op) <= OP_" . uc($last);
        }
        else {
            print join(" || \\\n\t ",
                           map { "(op) == OP_" . uc() } sort keys %$op_is);
        }
        print ")\n";
    }
}

sub generate_opcode_h {
    my $oc = open_new( 'opcode.h', '>', {
        by        => 'regen/opcode.pl',
        copyright => [1993 .. 2007],
        file      => 'opcode.h',
        from      => 'its data',
        style     => '*',
    });

    my $old = select $oc;

    generate_opcode_h_prologue;
    generate_opcode_h_defines;
    generate_opcode_h_opnames;
    generate_opcode_h_pl_ppaddr;
    generate_opcode_h_pl_check;
    generate_opcode_h_pl_opargs;
    generate_opcode_h_epilogue;

    select $old;
}

my @unimplemented;
sub generate_opcode_h_defines {
    my $last_cond = '';

    sub unimplemented {
        if (@unimplemented) {
            print "#else\n";
            foreach (@unimplemented) {
                print "#define $_ Perl_unimplemented_op\n";
            }
            print "#endif\n";
            @unimplemented = ();
        }

    }

    for (@ops) {
        my ($impl, $cond) = @{$alias{$_} || ["Perl_pp_$_", '']};
        my $op_func = "Perl_pp_$_";

        if ($cond ne $last_cond) {
            # A change in condition. (including to or from no condition)
            unimplemented();
            $last_cond = $cond;
            if ($last_cond) {
                print "$last_cond\n";
            }
        }
        push @unimplemented, $op_func if $last_cond;
        print "#define $op_func $impl\n" if $impl ne $op_func;
    }
    # If the last op was conditional, we need to close it out:
    unimplemented();

    print "\n#endif /* End of $restrict_to_core */\n\n";
}

sub generate_opcode_h_epilogue {
    print "\n\n";
    OP_PRIVATE::print_defines(select);
    OP_PRIVATE::print_PL_op_private_tables(select);
    read_only_bottom_close_and_rename(select);
}

sub generate_opcode_h_prologue {
    print "#$restrict_to_core\n\n";
}

sub generate_opcode_h_opnames {
    # Emit op names and descriptions.
    print <<~'END';
    START_EXTERN_C

    EXTCONST char* const PL_op_name[] INIT({
    END

    for (@ops) {
        print qq(\t"$_",\n);
    }

    print <<~'END';
            "freed",
    });

    EXTCONST char* const PL_op_desc[] INIT({
    END

    for (@ops) {
        my($safe_desc) = $desc{$_};

        # Have to escape double quotes and escape characters.
        $safe_desc =~ s/([\\"])/\\$1/g;

        print qq(\t"$safe_desc",\n);
    }

    print <<~'END';
        "freed op",
    });

    END_EXTERN_C
    END
}

sub generate_opcode_h_pl_check {
    print <<~'END';

    EXT Perl_check_t PL_check[] /* or perlvars.h */
    INIT({
    END

    for (@ops) {
        print "\t", tab(3, "Perl_$check{$_},"), "\t/* $_ */\n";
    }

    print <<~'END';
    });
    END
}

sub generate_opcode_h_pl_opargs {
    my $OCSHIFT = 8;
    my $OASHIFT = 12;

    print <<~'END';

    EXTCONST U32 PL_opargs[] INIT({
    END

    for my $op (@ops) {
        my $argsum = 0;
        my $flags = $flags{$op};
        for my $flag (keys %opflags) {
            if ($flags =~ s/$flag//) {
                die "Flag collision for '$op' ($flags{$op}, $flag)\n"
                    if $argsum & $opflags{$flag};
                $argsum |= $opflags{$flag};
            }
        }
        die qq[Opcode '$op' has no class indicator ($flags{$op} => $flags)\n]
            unless exists $opclass{$flags};
        $argsum |= $opclass{$flags} << $OCSHIFT;
        my $argshift = $OASHIFT;
        for my $arg (split(' ',$args{$op})) {
            if ($arg =~ s/^D//) {
                # handle 1st, just to put D 1st.
            }
            if ($arg =~ /^F/) {
                # record opnums of these opnames
                $arg =~ s/s//;
                $arg =~ s/-//;
                $arg =~ s/\+//;
            } elsif ($arg =~ /^S./) {
                $arg =~ s/<//;
                $arg =~ s/\|//;
            }
            my $argnum = ($arg =~ s/\?//) ? 8 : 0;
            die "op = $op, arg = $arg\n"
                unless exists $argnum{$arg};
            $argnum += $argnum{$arg};
            die "Argument overflow for '$op'\n"
                if $argshift >= $ARGBITS ||
                $argnum > ((1 << ($ARGBITS - $argshift)) - 1);
            $argsum += $argnum << $argshift;
            $argshift += 4;
        }
        $argsum = sprintf("0x%08x", $argsum);
        print "\t", tab(3, "$argsum,"), "/* $op */\n";
    }

    print <<~'END';
    });

    END_EXTERN_C
    END
}

sub generate_opcode_h_pl_ppaddr {
    # Emit ppcode switch array.

    print <<~'END';

    START_EXTERN_C

    EXT Perl_ppaddr_t PL_ppaddr[] /* or perlvars.h */
    INIT({
    END

    for (@ops) {
        my $op_func = "Perl_pp_$_";
        my $name = $alias{$_};
        if ($name && $name->[0] ne $op_func) {
            print "\t$op_func,\t/* implemented by $name->[0] */\n";
        } else {
            print "\t$op_func,\n";
        }
    }

    print <<~'END';
    });
    END
}

sub generate_opnames_h {
    my $on = open_new('opnames.h', '>', {
        by => 'regen/opcode.pl',
        from => 'its data',
        style => '*',
        file => 'opnames.h',
        copyright => [1999 .. 2008],
    });

    my $old = select $on;

    generate_opnames_h_opcode_enum;
    generate_opnames_h_opcode_predicates;
    generate_opnames_h_epilogue;

    select $old;
}

sub generate_opnames_h_opcode_enum {
    print "typedef enum opcode {\n";

    my $i = 0;
    for (@ops) {
        print "\t", tab(3,"OP_\U$_"), " = ", $i++, ",\n";
    }

    print "\t", tab(3,"OP_max"), "\n";
    print "} opcode;\n";
    print "\n#define MAXO ", scalar @ops, "\n";
    print "#define OP_FREED MAXO\n";
}

sub generate_opnames_h_epilogue {
    read_only_bottom_close_and_rename(select);
}

sub generate_opnames_h_opcode_predicates {
    # Emit OP_IS_* macros
    print <<~'EO_OP_IS_COMMENT';

    /* the OP_IS_* macros are optimized to a simple range check because
        all the member OPs are contiguous in regen/opcodes table.
        opcode.pl verifies the range contiguity, or generates an OR-equals
        expression */
    EO_OP_IS_COMMENT

    my %OP_IS_SOCKET;                    # /Fs/
    my %OP_IS_FILETEST;                  # /F-/
    my %OP_IS_FT_ACCESS;                 # /F-+/
    my %OP_IS_NUMCOMPARE;                # /S</
    my %OP_IS_DIRHOP;                    # /Fd/
    my %OP_IS_INFIX_BIT;                 # /S\|/

    for my $op (@ops) {
        for my $arg (split(' ',$args{$op})) {
            if ($arg =~ s/^D//) {
                # handle 1st, just to put D 1st.
                $OP_IS_DIRHOP{$op}   = $opnum{$op};
            }
            if ($arg =~ /^F/) {
                # record opnums of these opnames
                $OP_IS_SOCKET{$op}   = $opnum{$op} if $arg =~ s/s//;
                $OP_IS_FILETEST{$op} = $opnum{$op} if $arg =~ s/-//;
                $OP_IS_FT_ACCESS{$op} = $opnum{$op} if $arg =~ s/\+//;
            } elsif ($arg =~ /^S./) {
                $OP_IS_NUMCOMPARE{$op} = $opnum{$op} if $arg =~ s/<//;
                $OP_IS_INFIX_BIT {$op} = $opnum{$op} if $arg =~ s/\|//;
            }
        }
    }

    gen_op_is_macro( \%OP_IS_SOCKET, 'OP_IS_SOCKET');
    gen_op_is_macro( \%OP_IS_FILETEST, 'OP_IS_FILETEST');
    gen_op_is_macro( \%OP_IS_FT_ACCESS, 'OP_IS_FILETEST_ACCESS');
    gen_op_is_macro( \%OP_IS_NUMCOMPARE, 'OP_IS_NUMCOMPARE');
    gen_op_is_macro( \%OP_IS_DIRHOP, 'OP_IS_DIRHOP');
    gen_op_is_macro( \%OP_IS_INFIX_BIT, 'OP_IS_INFIX_BIT');
}

sub generate_pp_proto_h {
    my $pp = open_new('pp_proto.h', '>', {
        by => 'opcode.pl',
        from => 'its data',
    });

    my $old = select $pp;

    my %funcs;
    for (@ops) {
        my $name = $alias{$_} ? $alias{$_}[0] : "pp_$_";
        $name =~ s/^Perl_//;
        ++$funcs{$name};
    }

    for (sort keys %funcs) {
        print $pp qq{PERL_CALLCONV PP($_) __attribute__visibility__("hidden");\n};
    }

    read_only_bottom_close_and_rename(select);

    select $old;
}

sub generate_b_op_private_pm {
    my $oprivpm = open_new('lib/B/Op_private.pm', '>', {
        by          => 'regen/opcode.pl',
        from        => "data in\nregen/op_private and pod embedded in regen/opcode.pl",
        style       => '#',
        file        => 'lib/B/Op_private.pm',
        copyright   => [2014 .. 2014],
    });

    OP_PRIVATE::print_B_Op_private($oprivpm);

    read_only_bottom_close_and_rename($oprivpm);
}
