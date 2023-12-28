# Generate one sh*.s file for each architecture defined in sh-opc.h
# This will contain all the instructions valid on that platform

# Pull the architecture inheritance macros out of sh-opc.h
# Pull all the insns out of the sh-opc.h file.
while (<>)
{
    chomp;
    # Handle line continuation
    if (s/\\$//) {
	$_ .= <>;
	redo unless eof();
    }
    # Concat comment line and the next line
    if (/^\s*\/\*
        	(?:\s*\S+){2}
	\s+ ([^*]+?)
	\s* \*\/ $
	/x)
    {
	$_ .= "  ";
	$_ .= <>;
	redo unless eof();
    }
    if (/#define\s+arch_([^ ]*)_up\s*\(([^)]*)\)/)
    {
	($arches[$archcount] = $1) =~ tr/_/-/;
	($descendents[$archcount] = $2) =~ tr/_/-/;
	$archcount += 1;
	next;
    }
    # Special case: Match the repeat pseudo op
    if (/^\s*\/\*
	\s* repeat
	\s+ start\s+end
	\s+ ([^*]+?)		# instruction operand
	\s* \*\/
	\s* \{
	    ((?:[^\}]+\}){2})	# 2 brace pairs (operands and nibbles)
	\s* ,
	\s* arch_(\S+)_up	# architecture name
	\s* \}
	/x)
    {
	$insns[$insncount] = "repeat 10 20 ".$1;
	$insns_context[$insncount] = $_;
	($insns_arch[$insncount] = $3) =~ tr/_/-/;
	$insncount += 1;
	next;
    }
    # Match all 32 bit opcodes
    if (/^\s*\/\*
            (?:\s*\S+){2}
	\s+ ([^*]+?)		# instruction operand
	\s* \*\/
	\s* \{
	    ((?:[^\}]+\}){2})	# 2 brace pairs (operands and nibbles)
	\s* ,
	\s* arch_(\S+)_up	# architecture name
	\s* \|
	\s* arch_op32
	\s* \}
	/x)
    {
	$insns[$insncount] = $1;
	$insns_context[$insncount] = $_;
	($insns_arch[$insncount] = $3) =~ tr/_/-/;
	$insncount += 1;
	next;
    }
    # Match all 16 bit opcodes
    if (/^\s*\/\*
        \s* \S+
	\s+ ([^*]+?)		# instruction operand
	\s* \*\/
	\s* \{
	    ((?:[^\}]+\}){2})	# 2 brace pairs (operands and nibbles)
	\s* ,
	\s* arch_(\S+)_up	# architecture name
	\s* \}
	/x)
    {
	$insns[$insncount] = $1;
	$insns_context[$insncount] = $_;
	($insns_arch[$insncount] = $3) =~ tr/_/-/;
	$insncount += 1;
	next;
    }
    # Match all remaining possible instructions (error detection)
    if (/^\s*\/\*
	    (?:[^*]*(?:\*[^\/])?)+	# match contents of comment allowing *
	\*\/
	\s* \{
	    (?:[^\}]+\}){2}		# 2 brace pairs (operands and nibbles)
	\s* ,
	[^\}]*
	arch
	[^\}]*
	\}
	/x)
    {
	print ("Found something that looks like an instruction",
	       " but cannot be decoded:\n", "\t", $_);
	next;
    }
}

#print $insncount, "\n";
print $archcount, "\n";

# Munge the insns such that they will assemble
# Each instruction in sh-opc.h has an example format
# with placeholders for the parameters. These placeholders
# need to be replaced with real registers and constants
# as appropriate in order to assemble correctly.

foreach $i (0 .. $insncount) {
    $out = $insns[$i];
    if ($insns_context[$i] =~ /AY_.{3,4}_N/) {
	$out =~ s/<REG_N>/r6/;
    } else {
	$out =~ s/<REG_N>/r4/;
    }
    $out =~ s/<REG_M>/r5/;
    if ($insns_context[$i] =~ /IMM0_20BY8/) {
	$out =~ s/<imm>/1024/;
    } else {
	$out =~ s/<imm>/4/;
    }
    $out =~ s/<bdisp\d*>/.+8/;
    $out =~ s/<disp12>/2048/;
    $out =~ s/<DISP12>/2048/;
    $out =~ s/<disp\d*>/8/;
    $out =~ s/Rn_BANK/r1_bank/;
    $out =~ s/Rm_BANK/r2_bank/;
    $out =~ s/<F_REG_N>/fr1/;
    $out =~ s/<F_REG_M>/fr2/;
    $out =~ s/<D_REG_N>/dr2/;
    $out =~ s/<D_REG_M>/dr4/;
    $out =~ s/<V_REG_[Nn]>/fv0/;
    $out =~ s/<V_REG_M>/fv4/;
    $out =~ s/<DX_REG_N>/xd2/;
    $out =~ s/<DX_REG_M>/xd4/;
    $out =~ s/XMTRX_M4/xmtrx/;
    $out =~ s/<DSP_REG_X>/x1/;
    $out =~ s/<DSP_REG_Y>/y0/;
    $out =~ s/<DSP_REG_M>/a1/;
    $out =~ s/<DSP_REG_N>/m0/;
    $out =~ s/<REG_Axy>/r1/;
    $out =~ s/<REG_Ayx>/r3/;
    $out =~ s/<DSP_REG_XY>/y1/;
    $out =~ s/<DSP_REG_YX>/y1/;
    $out =~ s/<DSP_REG_AX>/a0/;
    $out =~ s/<DSP_REG_AY>/a0/;
    $out =~ s/Se/x0/;
    $out =~ s/Sf/y0/;
    $out =~ s/Dg/m0/;
    if ($insns_context[$i] =~ /PPIC/) {
	$out = "dct $out";
    }
    if ($insns_context[$i] =~ /i8p4/) {
	$out = ".align 2\n\t$out";
    }
    # Write back the results.
    # print ($out, "\n");
    $insns[$i] = $out;
}

# For each architecture, extract its immediate parents
foreach $a (0 .. $archcount) {
    $s = $descendents[$a];
    $s =~ s/[\s|]+/ /g;
    @list = split(' ', $s);
    while ($word = shift (@list)) {
	if ($word =~ /^arch-(.*)-up$/) {
	    push @{$archtree{$1}}, $arches[$a];
	}
    }
}

# Propagate the inhertances through the list
# Iterate to ensure all inheritances are found (necessary?)
$changesmade = 1;
while ($changesmade) {
    $changesmade = 0;
    foreach $a (@arches) {
	foreach $b (@arches) {
	    # If arch 'a' is a parent of arch 'b' then b inherits from a
	    if (grep {$_ eq $a} @{$archtree{$b}}) {
 		# Only add each arch if it is not already present
		foreach $c (@{$archtree{$a}}) {
		    if ((grep {$_ eq $c} @{$archtree{$b}}) == 0) {
			push @{$archtree{$b}}, $c;
			$changesmade = 1;
		    }
		}
	    }
	}
    }
}

# Generate the assembler file for each architecture
# Also count up how many instructions should be valid for each architecture

foreach $arch (0 .. ($archcount - 1)) {
    print $arches[$arch], "\n";
    $insns_valid{$arches[$arch]} = 0;
    unless (open ($fd, ">$arches[$arch].s")) {
	die "Can't open $arches[$arch].s\n";
    }
    print $fd "! Generated file. DO NOT EDIT.\n";
    print $fd "!\n";
    print $fd "! This file was generated by gas/testsuite/gas/sh/arch/sh-opc-gen-as.pl .\n";
    print $fd "! This file should contain every instruction valid on\n";
    print $fd "! architecture $arches[$arch] but no more.\n";
    print $fd "! If the tests are failing because the expected results have changed then run\n";
    print $fd "!    'cat ../../../../../opcodes/sh-opc.h | perl sh-opc-gen-as.pl'\n";
    print $fd "! in <srcdir>/gas/testsuite/gas/sh/arch to re-generate the files.\n";
    print $fd "! Make sure there are no unexpected or missing instructions.\n";
    print $fd "\n\t.section .text\n";
    ($lab = $arches[$arch]) =~ tr/-/_/;
    print $fd "$lab:\n";
    print $fd "! Instructions introduced into $arches[$arch]\n";
    foreach $i (0 .. $insncount) {
	if ($arches[$arch] eq $insns_arch[$i]) {
	    $context = $insns_context[$i];
	    $context =~ s/,$//;
	    $context =~ s/^\s*\//\//;
	    printf $fd "\t%-25s ;!%s\n", $insns[$i], $context;
	    $insns_valid{$arches[$arch]} += 1;
	}
    }
    print $fd "\n! Instructions inherited from ancestors:";
    foreach $anc (sort @{$archtree{$arches[$arch]}}) {
	print $fd " $anc";
    }
    print $fd "\n";
    foreach $i (0 .. $insncount) {
	if (($arches[$arch] ne $insns_arch[$i])
	    && (grep {$_ eq  $insns_arch[$i]} @{$archtree{$arches[$arch]}})) {
	    $context = $insns_context[$i];
	    $context =~ s/,$//;
	    $context =~ s/^\s*\//\//;
	    printf $fd "\t%-25s ;!%s\n", $insns[$i], $context;
	    $insns_valid{$arches[$arch]} += 1;
	}
    }
    close $fd;
}
