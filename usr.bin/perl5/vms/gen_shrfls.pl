# Create global symbol declarations, transfer vector, and
# linker options files for PerlShr.
#
# Processes the output of makedef.pl.
#
# Input:
#    $cc_cmd - compiler command
#    $objsuffix - file type (including '.') used for object files.
#    $libperl - Perl object library.
#    $extnames - package names for static extensions (used to generate
#        linker options file entries for boot functions)
#    $rtlopt - name of options file specifying RTLs to which PerlShr.Exe
#        must be linked
#
# Output:
#    PerlShr_Attr.Opt - linker options file which specifies that global vars
#        be placed in NOSHR,WRT psects.  Use when linking any object files
#        against PerlShr.Exe, since cc places global vars in SHR,WRT psects
#        by default.
#    PerlShr_Bld.Opt - declares universal symbols for PerlShr.Exe
#
# To do:
#   - figure out a good way to collect global vars in one psect, given that
#     we can't use globaldef because of gcc.
#   - then, check for existing files and preserve symbol and transfer vector
#     order for upward compatibility
#   - then, add GSMATCH to options file - but how do we insure that new
#     library has everything old one did
#     (i.e. /Define=DEBUGGING,EMBED,MULTIPLICITY)?
#
# Author: Charles Bailey  bailey@newman.upenn.edu

use strict;
require 5.000;

my $debug = $ENV{'GEN_SHRFLS_DEBUG'};

print "gen_shrfls.pl Rev. 8-Jul-2011\n" if $debug;

if ($ARGV[0] eq '-f') {
  open(INP,'<',$ARGV[1]) or die "Can't read input file $ARGV[1]: $!\n";
  print "Input taken from file $ARGV[1]\n" if $debug;
  @ARGV = ();
  while (<INP>) {
    chomp;
    push(@ARGV,split(/\|/,$_));
  }
  close INP;
  print "Read input data | ",join(' | ',@ARGV)," |\n" if $debug > 1;
}

my $cc_cmd = shift @ARGV; # no longer used to run the preprocessor

print "Input \$cc_cmd: \\$cc_cmd\\\n" if $debug;
my $docc = ($cc_cmd !~ /^~~/);
print "\$docc = $docc\n" if $debug;

my ( $use_threads, $use_mymalloc, $care_about_case, $shorten_symbols,
     $debugging_enabled, $hide_mymalloc, $use_perlio, $dir )
   = ( 0, 0, 0, 0, 0, 0, 0 );

if (-f 'perl.h') { $dir = '[]'; }
elsif (-f '[-]perl.h') { $dir = '[-]'; }
else { die "$0: Can't find perl.h\n"; }

# Go see what is enabled in config.sh
my $config = $dir . "config.sh";
open CONFIG, '<', $config;
while(<CONFIG>) {
    $use_threads++ if /usethreads='(define|yes|true|t|y|1)'/i;
    $use_mymalloc++ if /usemymalloc='(define|yes|true|t|y|1)'/i;
    $care_about_case++ if /d_vms_case_sensitive_symbols='(define|yes|true|t|y|1)'/i;
    $shorten_symbols++ if /d_vms_shorten_long_symbols='(define|yes|true|t|y|1)'/i;
    $debugging_enabled++ if /usedebugging_perl='(define|yes|true|t|y|1)'/i;
    $hide_mymalloc++ if /embedmymalloc='(define|yes|true|t|y|1)'/i;
    $use_perlio++ if /useperlio='(define|yes|true|t|y|1)'/i;
}
close CONFIG;
  
# put quotes back onto defines - they were removed by DCL on the way in
if (my ($prefix,$defines,$suffix) =
         ($cc_cmd =~ m#(.*)/Define=(.*?)([/\s].*)#i)) {
  $defines =~ s/^\((.*)\)$/$1/;
  $debugging_enabled ||= $defines =~ /\bDEBUGGING\b/;
  my @defines = split(/,/,$defines);
  $cc_cmd = "$prefix/Define=(" . join(',',grep($_ = "\"$_\"",@defines)) 
              . ')' . $suffix;
}
print "Filtered \$cc_cmd: \\$cc_cmd\\\n" if $debug;

print "\$debugging_enabled: $debugging_enabled\n" if $debug;

my $objsuffix = shift @ARGV;
print "\$objsuffix: \\$objsuffix\\\n" if $debug;
my $dbgprefix = shift @ARGV;
print "\$dbgprefix: \\$dbgprefix\\\n" if $debug;
my $olbsuffix = shift @ARGV;
print "\$olbsuffix: \\$olbsuffix\\\n" if $debug;
my $libperl = "${dbgprefix}libperl$olbsuffix";
my $extnames = shift @ARGV;
print "\$extnames: \\$extnames\\\n" if $debug;
my $rtlopt = shift @ARGV;
print "\$rtlopt: \\$rtlopt\\\n" if $debug;

my (%vars, %fcns);

open my $makedefs, '<', $dir . 'makedef.lis' or die "Unable to open makedef.lis: $!";

while (my $line = <$makedefs>) {
  chomp $line;
  $line = shorten_symbol($line, $care_about_case) if $shorten_symbols;
  # makedef.pl loses distinction between vars and funcs, so
  # use the start of the name to guess and add specific
  # exceptions when we know about them.
  if ($line =~ m/^(PL_|MallocCfg)/
      || $line eq 'PerlIO_perlio'
      || $line eq 'PerlIO_pending') {
    $vars{$line}++;
  }
  else {
    $fcns{$line}++;
  }
}

foreach (split /\s+/, $extnames) {
  my($pkgname) = $_;
  $pkgname =~ s/::/__/g;
  $fcns{"boot_$pkgname"}++;
  print "Adding boot_$pkgname to \%fcns (for extension $_)\n" if $debug;
}

# Eventually, we'll check against existing copies here, so we can add new
# symbols to an existing options file in an upwardly-compatible manner.

my $marord = 1;
open(OPTBLD,'>', "${dir}${dbgprefix}perlshr_bld.opt")
  or die "$0: Can't write to ${dir}${dbgprefix}perlshr_bld.opt: $!\n";


print OPTBLD "PSECT_ATTR=LIB\$INITIALIZE,GBL,NOEXE,NOWRT,NOSHR,LONG\n";
print OPTBLD "case_sensitive=yes\n" if $care_about_case;
my $count = 0;
foreach my $var (sort (keys %vars)) {
  print OPTBLD "SYMBOL_VECTOR=($var=DATA)\n";
}

foreach my $func (sort keys %fcns) {
  print OPTBLD "SYMBOL_VECTOR=($func=PROCEDURE)\n";
}

open(OPTATTR, '>', "${dir}perlshr_attr.opt")
  or die "$0: Can't write to ${dir}perlshr_attr.opt: $!\n";

print OPTATTR "! No additional linker directives are needed when using DECC\n";
close OPTATTR;

my $incstr = 'perl,globals';
my (@symfiles, $drvrname);

# Initial hack to permit building of compatible shareable images for a
# given version of Perl.
if ($ENV{PERLSHR_USE_GSMATCH}) {
  if ($ENV{PERLSHR_USE_GSMATCH} eq 'INCLUDE_COMPILE_OPTIONS') {
    # Build up a major ID. Since on Alpha it can only be 8 bits, we encode
    # the version number in the top 6 bits and use the bottom 2 for build
    # options most likely to cause incompatibilities.  Breaks at Perl 5.64.
    my ($ver, $sub) = $] =~ /\.(\d\d\d)(\d\d\d)/;
    $ver += 0; $sub += 0;
    my $gsmatch = ($ver % 2 == 1) ? "EQUAL" : "LEQUAL"; # Force an equal match for
						  # dev, but be more forgiving
						  # for releases

    $ver <<= 2;
    $ver += 1 if $debugging_enabled;	# If DEBUGGING is set
    $ver += 2 if $use_threads;		# if we're threaded
    print OPTBLD "GSMATCH=$gsmatch,$ver,$sub\n";
  }
  else {
    my $major = int($] * 1000)                        & 0xFF;  # range 0..255
    my $minor = int(($] * 1000 - $major) * 100 + 0.5) & 0xFF;  # range 0..255
    print OPTBLD "GSMATCH=LEQUAL,$major,$minor\n";
  }
}
elsif (@symfiles) { $incstr .= ',' . join(',',@symfiles); }
# Include object modules and RTLs in options file
# Linker wants /Include and /Library on different lines
print OPTBLD "$libperl/Include=($incstr)\n";
print OPTBLD "$libperl/Library\n";
open(RTLOPT,'<',$rtlopt) or die "$0: Can't read options file $rtlopt: $!\n";
while (<RTLOPT>) { print OPTBLD; }
close RTLOPT;
close OPTBLD;


# Symbol shortening Copyright (c) 2012 Craig A. Berry
#
# Released under the same terms as Perl itself.
#
# This code provides shortening of long symbols (> 31 characters) using the
# same mechanism as the OpenVMS C compiler.  The basic procedure is to compute
# an AUTODIN II checksum of the entire symbol, encode the checksum in base32,
# and glue together a shortened symbol from the first 23 characters of the
# original symbol plus the encoded checksum appended.  The output format is
# the same used in the name mangler database, stored by default in
# [.CXX_REPOSITORY]CXX$DEMANGLER_DB.

sub crc32 {
    use constant autodin_ii_table => [
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
    ];

    my $input_string = shift;
    my $crc = 0xFFFFFFFF;

    for my $byte (unpack 'C*', $input_string) {
        $crc = ($crc >> 8) ^ autodin_ii_table->[($crc ^ $byte) & 0xff];
    }
    return ~$crc;
}

sub base32 {
    my $input = shift;
    my $output = '';
    use constant base32hex_table => [
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v'
    ];

    # Grab lowest 5 bits and look up conversion in table.  Lather, rinse,
    # repeat for a total of 7, 5-bit chunks to accommodate 32 bits of input.

    for (0..6) {
        $output  = base32hex_table->[$input & 0x1f] . $output;
        $input >>= 5;     # position to look at next 5
    }
    $output .= '$';       #  It's DEC, so use '$' not '=' to pad.

    return $output;
}

sub shorten_symbol {
    my $input_symbol = shift;
    my $as_is_flag = shift;
    my $symbol = $input_symbol;

    return $symbol unless length($input_symbol) > 31;

    $symbol = uc($symbol) unless $as_is_flag;
    my $crc = crc32($symbol);
    $crc = ~$crc;  # Compiler uses non-inverted form.
    my $b32 = base32($crc);
    $b32 = uc($b32) unless $as_is_flag;

    return substr($symbol, 0, 23) . $b32;
}

__END__

