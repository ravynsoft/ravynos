#!/usr/bin/perl -w

# Try opening libperl.a with nm, and verifying it has the kind of
# symbols we expect, and no symbols we should avoid.
#
# Fail softly, expect things only on known platforms:
# - linux, x86 only (ppc linux has odd symbol tables)
# - darwin (OS X), both x86 and ppc
# - freebsd
# and on other platforms, and if things seem odd, just give up (skip_all).
#
# Symbol types for LTO builds don't seem to match their final section, so
# skip on LTO builds too.
#
# Debugging tip: nm output (this script's input) can be faked by
# giving one command line argument for this script: it should be
# either the filename to read, or "-" for STDIN.  You can also append
# "@style" (where style is a supported nm style, like "gnu" or "darwin")
# to this filename for "cross-parsing".
#
# Some terminology:
# - "text" symbols are code
# - "data" symbols are data (duh), with subdivisions:
#   - "bss": (Block-Started-by-Symbol: originally from IBM assembler...),
#     uninitialized data, which often even doesn't exist in the object
#     file as such, only its size does, which is then created on demand
#     by the loader
#  - "const": initialized read-only data, like string literals
#  - "common": uninitialized data unless initialized...
#    (the full story is too long for here, see "man nm")
#  - "data": initialized read-write data
#    (somewhat confusingly below: "data data", but it makes code simpler)
#  - "undefined": external symbol referred to by an object,
#    most likely a text symbol.  Can be either a symbol defined by
#    a Perl object file but referred to by other Perl object files,
#    or a completely external symbol from libc, or other system libraries.

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
}

use strict;

use Config;

if ($Config{cc} =~ /g\+\+/) {
    # XXX Could use c++filt, maybe.
    skip_all "on g++";
}

# ccname is gcc for both gcc and clang
if ($Config{ccname} eq "gcc" && $Config{ccflags} =~ /-flto\b/) {
    # If we compile with gcc nm marks PL_no_mem as "D" (normal data) rather than a R (read only)
    # but the symbol still ends up in the .rodata section of the image on linking.
    # If we compile with clang 14, nm marks PL_no_mem as "T" (text, aka code) rather than R
    # but the symbol still ends up in the .rodata section on linking.
    skip_all "LTO libperl.a flags don't match the final linker sections";
}

my $libperl_a;

for my $f (qw(../libperl.a libperl.a)) {
  if (-f $f) {
    $libperl_a = $f;
    last;
  }
}

unless (defined $libperl_a) {
  skip_all "no libperl.a";
}

print "# \$^O = $^O\n";
print "# \$Config{archname} = $Config{archname}\n";
print "# \$Config{cc} = $Config{cc}\n";
print "# libperl = $libperl_a\n";

my $nm;
my $nm_opt = '';
my $nm_style;
my $nm_fh;
my $nm_err_tmp = "libperl$$";

END {
    # this is still executed when we skip_all above, avoid a warning
    unlink $nm_err_tmp if $nm_err_tmp;
}

my $fake_input;
my $fake_style;

if (@ARGV == 1) {
    $fake_input = shift @ARGV;
    print "# Faking nm output from $fake_input\n";
    if ($fake_input =~ s/\@(.+)$//) {
        $fake_style = $1;
        print "# Faking nm style from $fake_style\n";
        if ($fake_style eq 'gnu' ||
            $fake_style eq 'linux' ||
            $fake_style eq 'freebsd') {
            $nm_style = 'gnu'
        } elsif ($fake_style eq 'darwin' || $fake_style eq 'osx') {
            $nm_style = 'darwin'
        } else {
            die "$0: Unknown explicit nm style '$fake_style'\n";
        }
    }
}

unless (defined $nm_style) {
    if ($^O eq 'linux') {
        # The 'gnu' style could be equally well be called 'bsd' style,
        # since the output format of the GNU binutils nm is really BSD.
        $nm_style = 'gnu';
    } elsif ($^O eq 'freebsd') {
        $nm_style = 'gnu';
    } elsif ($^O eq 'darwin') {
        $nm_style = 'darwin';
    }
}

if (defined $nm_style) {
    if ($nm_style eq 'gnu') {
        $nm = '/usr/bin/nm';
    } elsif ($nm_style eq 'darwin') {
        $nm = '/usr/bin/nm';
        # With the -m option we get better information than the BSD-like
        # default: with the default, a lot of symbols get dumped into 'S'
        # or 's', for example one cannot tell the difference between const
        # and non-const data symbols.
        $nm_opt = '-m';
    } else {
        die "$0: Unexpected nm style '$nm_style'\n";
    }
}

if ($^O eq 'linux' && $Config{archname} !~ /^(?:x|i6)86/) {
    # For example in ppc most (but not all!) code symbols are placed
    # in 'D' (data), not in ' T '.  We cannot work under such conditions.
    skip_all "linux but archname $Config{archname} not x86*";
}

unless (defined $nm) {
  skip_all "no nm";
}

unless (defined $nm_style) {
  skip_all "no nm style";
}

print "# nm = $nm\n";
print "# nm_style = $nm_style\n";
print "# nm_opt = $nm_opt\n";

unless (-x $nm) {
    skip_all "no executable nm $nm";
}

if ($nm_style eq 'gnu' && !defined $fake_style) {
    open(my $gnu_verify, "$nm --version|") or
        skip_all "nm failed: $!";
    my $gnu_verified;
    while (<$gnu_verify>) {
        if (/^GNU nm/) {
            $gnu_verified = 1;
            last;
        }
    }
    unless ($gnu_verified) {
        skip_all "no GNU nm";
    }
}

if (defined $fake_input) {
    if ($fake_input eq '-') {
        open($nm_fh, "<&STDIN") or
            skip_all "Duping STDIN failed: $!";
    } else {
        open($nm_fh, "<", $fake_input) or
            skip_all "Opening '$fake_input' failed: $!";
    }
    undef $nm_err_tmp; # In this case there will be no nm errors.
} else {
    print qq{# command: "$nm $nm_opt $libperl_a 2>$nm_err_tmp |"\n};
    open($nm_fh, "$nm $nm_opt $libperl_a 2>$nm_err_tmp |") or
        skip_all "$nm $nm_opt $libperl_a failed: $!";
}

sub is_perlish_symbol {
    $_[0] =~ /^(?:PL_|Perl|PerlIO)/;
}

# XXX Implement "internal test" for this script (option -t?)
# to verify that the parsing does what it's intended to.

sub nm_parse_gnu {
    my $symbols = shift;
    my $line = $_;
    if (m{^(\w+\.o):$}) {
        # object file name
        $symbols->{obj}{$1}++;
        $symbols->{o} = $1;
        return;
    } else {
        die "$0: undefined current object: $line"
            unless defined $symbols->{o};
        # 64-bit systems have 16 hexdigits, 32-bit systems have 8.
        if (s/^[0-9a-f]{8}(?:[0-9a-f]{8})? //) {
            if (/^[Rr] (\w+)$/) {
                # R: read only (const)
                $symbols->{data}{const}{$1}{$symbols->{o}}++;
            } elsif (/^r .+$/) {
                # Skip local const (read only).
            } elsif (/^([Tti]) (\w+)(\..+)?$/) {
                $symbols->{text}{$2}{$symbols->{o}}{$1}++;
            } elsif (/^C (\w+)$/) {
                $symbols->{data}{common}{$1}{$symbols->{o}}++;
            } elsif (/^[BbSs] (\w+)(\.\d+)?$/) {
                # Bb: uninitialized data (bss)
                # Ss: uninitialized data "for small objects"
                $symbols->{data}{bss}{$1}{$symbols->{o}}++;
            } elsif (/^D _LIB_VERSION$/) {
                # Skip the _LIB_VERSION (not ours, probably libm)
            } elsif (/^[DdGg] (\w+)$/) {
                # Dd: initialized data
                # Gg: initialized "for small objects"
                $symbols->{data}{data}{$1}{$symbols->{o}}++;
            } elsif (/^. \.?(\w+)$/) {
                # Skip the unknown types.
                print "# Unknown type: $line ($symbols->{o})\n";
            }
            return;
        } elsif (/^ {8}(?: {8})? U _?(\w+)$/) {
            my ($symbol) = $1;
            return if is_perlish_symbol($symbol);
            $symbols->{undef}{$symbol}{$symbols->{o}}++;
            return;
	}
    }
    print "# Unexpected nm output '$line' ($symbols->{o})\n";
}

sub nm_parse_darwin {
    my $symbols = shift;
    my $line = $_;
    if (m{^(?:.+)?libperl\.a\((\w+\.o)\):$}) {
        # object file name
        $symbols->{obj}{$1}++;
        $symbols->{o} = $1;
        return;
    } else {
        if ($^V < v5.39 && !defined $symbols->{o}) {
            skip_all "nm parser requires an update on Darwin";
        }

        die "$0: undefined current object: $line" unless defined $symbols->{o};
        # 64-bit systems have 16 hexdigits, 32-bit systems have 8.
        if (s/^[0-9a-f]{8}(?:[0-9a-f]{8})? //) {
            # String literals can live in different sections
            # depending on the compiler and os release, assumedly
            # also linker flags.
            if (/^\(__TEXT,__(?:const|(?:asan_)?cstring|literal\d+)\) (?:non-)?external _?(\w+)(\.\w+)?$/) {
                my ($symbol, $suffix) = ($1, $2);
                # Ignore function-local constants like
                # _Perl_av_extend_guts.oom_array_extend
                return if defined $suffix && /__TEXT,__const/;
                # Ignore the cstring unnamed strings.
                return if $symbol =~ /^L\.str\d+$/;
                $symbols->{data}{const}{$symbol}{$symbols->{o}}++;
            } elsif (/^\(__TEXT,__text\) ((?:non-)?external) _(\w+)$/) {
                my ($exp, $sym) = ($1, $2);
                $symbols->{text}{$sym}{$symbols->{o}}{$exp =~ /^non/ ? 't' : 'T'}++;
            } elsif (/^\(__DATA,__\w*?(const|data|bss|common)\w*\) (?:non-)?external _?(\w+)(\.\w+)?$/) {
                my ($dtype, $symbol, $suffix) = ($1, $2, $3);
                # Ignore function-local constants like
                # _Perl_pp_gmtime.dayname
                return if defined $suffix;
                $symbols->{data}{$dtype}{$symbol}{$symbols->{o}}++;
            } elsif (/^\(__DATA,__const\) non-external _\.memset_pattern\d*$/) {
                # Skip this, whatever it is (some inlined leakage from
                # darwin libc?)
            } elsif (/^\(__TEXT,__eh_frame/) {
                # Skip the eh_frame (exception handling) symbols.
                return;
            } elsif (/^\(__\w+,__\w+\) /) {
                # Skip the unknown types.
                print "# Unknown type: $line ($symbols->{o})\n";
            }
            return;
        } elsif (/^ {8}(?: {8})? \(undefined(?: \[lazy bound\])?\) external _?(.+)/) {
            # darwin/ppc marks most undefined text symbols
            # as "[lazy bound]".
            my ($symbol) = $1 =~ s/\$UNIX2003\z//r;
            return if is_perlish_symbol($symbol);
            $symbols->{undef}{$symbol}{$symbols->{o}}++;
            return;
        }
    }
    print "# Unexpected nm output '$line' ($symbols->{o})\n";
}

my $nm_parse;

if ($nm_style eq 'gnu') {
    $nm_parse = \&nm_parse_gnu;
} elsif ($nm_style eq 'darwin') {
    $nm_parse = \&nm_parse_darwin;
}

unless (defined $nm_parse) {
    skip_all "no nm parser ($nm_style $nm_style, \$^O $^O)";
}

my %symbols;

while (<$nm_fh>) {
    next if /^$/;
    chomp;
    $nm_parse->(\%symbols);
}

# use Data::Dumper; print Dumper(\%symbols);

# Something went awfully wrong.  Wrong nm?  Wrong options?
unless (keys %symbols) {
    skip_all "no symbols\n";
}
unless (exists $symbols{text}) {
    skip_all "no text symbols\n";
}

# These should always be true for everyone.

ok($symbols{obj}{'util.o'}, "has object util.o");
ok($symbols{text}{'Perl_croak'}{'util.o'}, "has text Perl_croak in util.o");
ok(exists $symbols{data}{const}, "has data const symbols");
ok($symbols{data}{const}{PL_no_mem}{'globals.o'}, "has PL_no_mem");

my $nocommon = $Config{ccflags} =~ /-fno-common/ ? 1 : 0;

print "# nocommon = $nocommon\n";

my %data_symbols;

for my $dtype (sort keys %{$symbols{data}}) {
    for my $symbol (sort keys %{$symbols{data}{$dtype}}) {
        $data_symbols{$symbol}++;
    }
}

if ( !$symbols{data}{common} ) {
    # This is likely because Perl was compiled with
    # -Accflags="-fno-common"
    $symbols{data}{common} = $symbols{data}{bss};
}

ok($symbols{data}{common}{PL_hash_seed_w}{'globals.o'}, "has PL_hash_seed_w");
ok($symbols{data}{data}{PL_ppaddr}{'globals.o'}, "has PL_ppaddr");

# See the comments in the beginning for what "undefined symbols"
# really means.  We *should* have many of those, that is a good thing.
ok(keys %{$symbols{undef}}, "has undefined symbols");

# There are certain symbols we expect to see.

# chmod, socket, getenv, sigaction, exp, time are system/library
# calls that should each see at least one use. exp can be expl
# if so configured.
my %expected = (
    chmod  => undef, # There is no Configure symbol for chmod.
    socket => 'd_socket',
    getenv => undef, # There is no Configure symbol for getenv,
    sigaction => 'd_sigaction',
    time   => 'd_time',
    );

if ($Config{uselongdouble} && $Config{longdblsize} > $Config{doublesize}) {
    $expected{expl} = undef; # There is no Configure symbol for expl.
} elsif ($Config{usequadmath}) {
    $expected{expq} = undef; # There is no Configure symbol for expq.
} else {
    $expected{exp} = undef; # There is no Configure symbol for exp.
}

# DynaLoader will use dlopen, unless we are building static,
# and it is used in the platforms we are supporting in this test.
if ($Config{usedl} ) {
    $expected{dlopen} = 'd_dlopen';
}

for my $symbol (sort keys %expected) {
    if (defined $expected{$symbol} && !$Config{$expected{$symbol}}) {
      SKIP: {
        skip("no $symbol");
      }
      next;
    }
    my @o = exists $symbols{undef}{$symbol} ?
        sort keys %{ $symbols{undef}{$symbol} } : ();
    ok(@o, "uses $symbol (@o)");
}

# There are certain symbols we expect NOT to see.
#
# gets is horribly unsafe.
#
# fgets should not be used (Perl has its own API, sv_gets),
# even without perlio.
#
# tmpfile is unsafe.
#
# strcat, strcpy, strncat, strncpy are unsafe.
#
# sprintf and vsprintf should not be used because
# Perl has its own safer and more portable implementations.
# (One exception: for certain floating point outputs
# the native sprintf is still used in some platforms, see below.)
#
# atoi has unsafe and undefined failure modes, and is affected by locale.
# Its cousins include atol and atoll.
#
# strtol and strtoul are affected by locale.
# Cousins include strtoq.
#
# system should not be used, use pp_system or my_popen.
#

my %unexpected;

for my $str (qw(system)) {
    $unexpected{$str} = "d_$str";
}

for my $stdio (qw(gets fgets tmpfile sprintf vsprintf)) {
    $unexpected{$stdio} = undef; # No Configure symbol for these.
}
for my $str (qw(strcat strcpy strncat strncpy)) {
    $unexpected{$str} = undef; # No Configure symbol for these.
}

$unexpected{atoi} = undef; # No Configure symbol for atoi.
$unexpected{atol} = undef; # No Configure symbol for atol.

for my $str (qw(atoll strtol strtoul strtoq)) {
    $unexpected{$str} = "d_$str";
}

for my $symbol (sort keys %unexpected) {
    if (defined $unexpected{$symbol} && !$Config{$unexpected{$symbol}}) {
      SKIP: {
        skip("no $symbol");
      }
      next;
    }
    my @o = exists $symbols{undef}{$symbol} ?
        sort keys %{ $symbols{undef}{$symbol} } : ();
    # While sprintf() is bad in the general case,
    # some platforms implement Gconvert via sprintf, in sv.o.
    if ($symbol eq 'sprintf' &&
        $Config{d_Gconvert} =~ /^sprintf/ &&
        @o == 1 && $o[0] eq 'sv.o') {
      SKIP: {
        skip("uses sprintf for Gconvert in sv.o");
      }
    } else {
        is(@o, 0, "uses no $symbol (@o)");
    }
}

# Check that any text symbols named S_ are not exported.
my $export_S_prefix = 0;
for my $t (sort grep { /^S_/ } keys %{$symbols{text}}) {
    for my $o (sort keys %{$symbols{text}{$t}}) {
        if (exists $symbols{text}{$t}{$o}{T}) {
            fail($t, "$t exported from $o");
            $export_S_prefix++;
        }
    }
}
is($export_S_prefix, 0, "no S_ exports");

if (defined $nm_err_tmp) {
    if (open(my $nm_err_fh, $nm_err_tmp)) {
        my $error;
        while (<$nm_err_fh>) {
            # OS X has weird error where nm warns about
            # "no name list" but then outputs fine.
            # llvm-nm may also complain about 'no symbols'. In some
            # versions this is exactly the string "no symbols\n" but in later
            # versions becomes a string followed by ": no symbols\n". For this
            # test it is typically "../libperl.a:perlapi.o: no symbols\n"
            if ( $^O eq 'darwin' ) {
                if (/nm: no name list/ || /^(.*: )?no symbols$/ ) {
                    print "# $^O ignoring $nm output: $_";
                    next;
                }
            }
            warn "$0: Unexpected $nm error: $_";
            $error++;
        }
        die "$0: Unexpected $nm errors\n" if $error;
    } else {
        warn "Failed to open '$nm_err_tmp': $!\n";
    }
}

done_testing();
