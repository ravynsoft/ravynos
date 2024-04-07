#!perl -w
use strict;
use FindExt;

# take a semicolon separated path list and turn it into a quoted
# list of paths that Text::Parsewords will grok
sub mungepath {
    my $p = shift;
    # remove leading/trailing semis/spaces
    $p =~ s/^[ ;]+//;
    $p =~ s/[ ;]+$//;
    $p =~ s/'/"/g;
    my @p = map { $_ = "\"$_\"" if /\s/ and !/^".*"$/; $_ } split /;/, $p;
    return wantarray ? @p : join(' ', @p);
}

# check that the directories in the provided var exist
sub check_path {
    my $opt = shift;
    my $p = shift;
    my @paths = mungepath($p);
    my $header_seen = 0;
    foreach my $path (@paths) {
        $path =~ s/^"(.*)"$/$1/;
        next, if -d $path;

        if (not $header_seen++) {
            print STDERR "'$opt' contains the following non-existing paths:\n";
        }
        print STDERR "\t$path\n";
    }
    if ($header_seen) {
        print STDERR <<EOF;
Did you provide a correct value for the 'CCHOME' option?

(This check can be skipped by using the SKIP_CCHOME_CHECK=1 option)
EOF
        exit 1;
    }
}

# generate an array of option strings from command-line args
# or an option file
#    -- added by BKS, 10-17-1999 to fix command-line overflow problems
sub loadopts {
    if ($ARGV[0] =~ /--cfgsh-option-file/) {
	shift @ARGV;
	my $optfile = shift @ARGV;
	local (*OPTF);
	open OPTF, '<', $optfile or die "Can't open $optfile: $!\n";
	my @opts;
	chomp(my $line = <OPTF>);
	my @vars = split(/\t+~\t+/, $line);
	for (@vars) {
	    push(@opts, $_) unless (/^\s*$/);
	}
	close OPTF;
	return \@opts;
    }
    else {
	return \@ARGV;
    }
}

my $prebuilt; # are we making the prebuilt config used to bootstrap?
if (@ARGV && $ARGV[0] eq '--prebuilt') {
    ++$prebuilt;
    shift;
}

my %opt;

my $optref = loadopts();
while (@{$optref} && $optref->[0] =~ /^([\w_]+)=(.*)$/) {
    $opt{$1}=$2;
    shift(@{$optref});
}

FindExt::scan_ext("../cpan");
FindExt::scan_ext("../dist");
FindExt::scan_ext("../ext");
FindExt::set_static_extensions(split ' ', $opt{static_ext});

$opt{nonxs_ext}        = join(' ',FindExt::nonxs_ext()) || ' ';
$opt{static_ext}       = join(' ',FindExt::static_ext()) || ' ';
$opt{dynamic_ext}      = join(' ',FindExt::dynamic_ext()) || ' ';
$opt{extensions}       = join(' ',FindExt::extensions()) || ' ';
$opt{known_extensions} = join(' ',FindExt::known_extensions()) || ' ';

my $pl_h = '../patchlevel.h';

if (-e $pl_h) {
    open PL, "<", $pl_h or die "Can't open $pl_h: $!";
    while (<PL>) {
	if (/^#\s*define\s+(PERL_\w+)\s+([\d.]+)/) {
	    $opt{$1} = $2;
	}
    }
    close PL;
}
else {
    die "Can't find $pl_h: $!";
}

my $patch_file = '../.patch';

if (-e $patch_file) {
    open my $fh, "<", $patch_file or die "Can't open $patch_file: $!";
    chomp($opt{PERL_PATCHLEVEL} = <$fh>);
    close $fh;
}

$opt{version} = "$opt{PERL_REVISION}.$opt{PERL_VERSION}.$opt{PERL_SUBVERSION}";
$opt{version_patchlevel_string} = "version $opt{PERL_VERSION} subversion $opt{PERL_SUBVERSION}";
$opt{version_patchlevel_string} .= " patch $opt{PERL_PATCHLEVEL}" if exists $opt{PERL_PATCHLEVEL};

my $ver = `ver 2>nul`;
$opt{osvers} = $ver =~ /\b(\d+(?:\.\d+)+)\b/ ? $1 : '4.0';

if (exists $opt{cc}) {
    # cl version detection borrowed from Test::Smoke's configsmoke.pl
    if ($opt{cc} =~ /\b(?:cl|icl)/) { #MSVC can come as clarm.exe, icl=Intel C
        my $output = `$opt{cc} 2>&1`;
        $opt{ccversion} = $output =~ /\b(\d+(?:\.\d+)+)\b/ ? $1 : '?';
    }
    elsif ($opt{cc} =~ /\bgcc\b/) {
        chomp($opt{gccversion} = `$opt{cc} -dumpversion`);
    }
}

$opt{cf_by} = $ENV{USERNAME} unless $opt{cf_by};
if (!$opt{cf_email}) {
    my $computername = eval{(gethostbyname('localhost'))[0]};
# gethostbyname might not be implemented in miniperl
    $computername = $ENV{COMPUTERNAME} if $@;    
    $opt{cf_email} = $opt{cf_by} . '@' . $computername;
}
$opt{usemymalloc} = 'y' if $opt{d_mymalloc} eq 'define';

unless ($opt{SKIP_CCHOME_CHECK}) {
    check_path('CCLIBDIR', $opt{libpth});
    check_path('CCINCDIR', $opt{incpath});
}

$opt{libpth} = mungepath($opt{libpth}) if exists $opt{libpth};
$opt{incpath} = mungepath($opt{incpath}) if exists $opt{incpath};

my $int64;
if ($opt{cc} =~ /\b(?:cl|icl)/) {
    $int64  = '__int64';
}
elsif ($opt{cc} =~ /\bgcc\b/) {
    $int64  = 'long long';
}

# set 64-bit options
if ($opt{WIN64} eq 'define') {
    $opt{d_atoll} = 'define';
    $opt{d_strtoll} = 'define';
    $opt{d_strtoull} = 'define';
    $opt{ptrsize} = 8;
    $opt{sizesize} = 8;
    $opt{ssizetype} = $int64;
    $opt{st_ino_size} = 8;
}
else {
    $opt{d_atoll} = 'undef';
    $opt{d_strtoll} = 'undef';
    $opt{d_strtoull} = 'undef';
    $opt{ptrsize} = 4;
    $opt{sizesize} = 4;
    $opt{ssizetype} = 'int';
    $opt{st_ino_size} = 4;
}

# set 64-bit-int options
if ($opt{use64bitint} eq 'define') {
    if ($opt{uselongdouble} eq 'define' || $opt{usequadmath} eq 'define') {
        $opt{d_nv_preserves_uv} = 'define';
        $opt{nv_preserves_uv_bits} = 64;
    }
    else {
        $opt{d_nv_preserves_uv} = 'undef';
        $opt{nv_preserves_uv_bits} = 53;
    }
    $opt{ivdformat} = qq{"I64d"};
    $opt{ivsize} = 8;
    $opt{ivtype} = $int64;
    $opt{sPRIXU64} = qq{"I64X"};
    $opt{sPRId64} = qq{"I64d"};
    $opt{sPRIi64} = qq{"I64i"};
    $opt{sPRIo64} = qq{"I64o"};
    $opt{sPRIu64} = qq{"I64u"};
    $opt{sPRIx64} = qq{"I64x"};
    $opt{uvXUformat} = qq{"I64X"};
    $opt{uvoformat} = qq{"I64o"};
    $opt{uvsize} = 8;
    $opt{uvtype} = qq{unsigned $int64};
    $opt{uvuformat} = qq{"I64u"};
    $opt{uvxformat} = qq{"I64x"};
}
else {
    $opt{d_nv_preserves_uv} = 'define';
    $opt{ivdformat} = '"ld"';
    $opt{ivsize} = 4;
    $opt{ivtype} = 'long';
    $opt{nv_preserves_uv_bits} = 32;
    $opt{sPRIXU64} = '"lX"';
    $opt{sPRId64} = '"ld"';
    $opt{sPRIi64} = '"li"';
    $opt{sPRIo64} = '"lo"';
    $opt{sPRIu64} = '"lu"';
    $opt{sPRIx64} = '"lx"';
    $opt{uvXUformat} = '"lX"';
    $opt{uvoformat} = '"lo"';
    $opt{uvsize} = 4;
    $opt{uvtype} = 'unsigned long';
    $opt{uvuformat} = '"lu"';
    $opt{uvxformat} = '"lx"';
}

unless ($opt{cc} =~ /\bcl/) {
    if ($opt{WIN64} eq 'define') {
        $opt{longdblsize} = 16;
        $opt{longdblinfbytes} = '0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00';
        $opt{longdblnanbytes} = '0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00';
    }
    else {
        $opt{longdblsize} = 12;
        $opt{longdblinfbytes} = '0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x7f, 0x00, 0x00';
        $opt{longdblnanbytes} = '0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x00, 0x00';
    }
}

# set long double options
if ($opt{uselongdouble} eq 'define') {
    $opt{d_Gconvert} = 'sprintf((b),"%.*""Lg",(n),(x))';
    $opt{d_PRIEUldbl} = 'define';
    $opt{d_PRIFUldbl} = 'define';
    $opt{d_PRIGUldbl} = 'define';
    $opt{d_modflproto} = 'define';
    $opt{d_strtold} = 'define';
    $opt{d_PRIeldbl} = 'define';
    $opt{d_PRIfldbl} = 'define';
    $opt{d_PRIgldbl} = 'define';
    $opt{d_SCNfldbl} = 'define';
    $opt{nvsize} = $opt{longdblsize};
    $opt{nvtype} = 'long double';
    $opt{nv_overflows_integers_at} = '256.0*256.0*256.0*256.0*256.0*256.0*256.0*2.0*2.0*2.0*2.0*2.0*2.0*2.0*2.0';
    $opt{nvEUformat} = '"LE"';
    $opt{nvFUformat} = '"LF"';
    $opt{nvGUformat} = '"LG"';
    $opt{nveformat} = '"Le"';
    $opt{nvfformat} = '"Lf"';
    $opt{nvgformat} = '"Lg"';
    $opt{nvmantbits} = 64;
    $opt{longdblkind} = 3;
    $opt{longdblmantbits} = 64;
}
# set __float128 options
elsif ($opt{usequadmath} eq 'define') {
    $opt{d_Gconvert} = 'sprintf((b),"%.*""Lg",(n),(x))';
    $opt{d_PRIEUldbl} = 'define';
    $opt{d_PRIFUldbl} = 'define';
    $opt{d_PRIGUldbl} = 'define';
    $opt{d_modflproto} = 'define';
    $opt{d_strtold} = 'define';
    $opt{d_PRIeldbl} = 'define';
    $opt{d_PRIfldbl} = 'define';
    $opt{d_PRIgldbl} = 'define';
    $opt{d_SCNfldbl} = 'define';
    $opt{nv_overflows_integers_at} = '256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*2.0';
    $opt{nvsize} = 16;
    $opt{nvtype} = '__float128';
    $opt{nvEUformat} = '"QE"';
    $opt{nvFUformat} = '"QF"';
    $opt{nvGUformat} = '"QG"';
    $opt{nveformat} = '"Qe"';
    $opt{nvfformat} = '"Qf"';
    $opt{nvgformat} = '"Qg"';
    $opt{nvmantbits} = 112;
    $opt{longdblkind} = 3;
    $opt{longdblmantbits} = 64;
    $opt{i_quadmath} = 'define';
}
else {
    $opt{d_Gconvert} = 'sprintf((b),"%.*g",(n),(x))';
    $opt{d_PRIEUldbl} = 'undef';
    $opt{d_PRIFUldbl} = 'undef';
    $opt{d_PRIGUldbl} = 'undef';

    if($opt{cc} =~ /\b(?:cl|icl)/) {
        $opt{d_modflproto} = 'undef';
    }
    else {
        $opt{d_modflproto} = 'define';
    }

    $opt{d_strtold} = 'undef';
    $opt{d_PRIeldbl} = 'undef';
    $opt{d_PRIfldbl} = 'undef';
    $opt{d_PRIgldbl} = 'undef';
    $opt{d_SCNfldbl} = 'undef';
    $opt{nvsize} = 8;
    $opt{nvtype} = 'double';
    $opt{nv_overflows_integers_at} = '256.0*256.0*256.0*256.0*256.0*256.0*2.0*2.0*2.0*2.0*2.0';
    $opt{nvEUformat} = '"E"';
    $opt{nvFUformat} = '"F"';
    $opt{nvGUformat} = '"G"';
    $opt{nveformat} = '"e"';
    $opt{nvfformat} = '"f"';
    $opt{nvgformat} = '"g"';
}

# change some configuration variables based on compiler version
if ($opt{cc} =~ /\bcl/ and $opt{ccversion} =~ /^(\d+)/) {
    my $ccversion = $1;
    if ($ccversion >= 14) { # VC8+
	$opt{sGMTIME_max} = 32535291599;
	$opt{sLOCALTIME_max} = 32535244799;
    }
    if ($ccversion >= 16 && !$prebuilt) { # VC10+
	$opt{i_stdint} = 'define';
    }
    if ($ccversion >= 19) { # VC14+
	$opt{stdio_base} = 'PERLIO_FILE_base(fp)';
	$opt{stdio_bufsiz} = '(PERLIO_FILE_cnt(fp) + PERLIO_FILE_ptr(fp) - PERLIO_FILE_base(fp))';
	$opt{stdio_cnt} = 'PERLIO_FILE_cnt(fp)';
	$opt{stdio_ptr} = 'PERLIO_FILE_ptr(fp)';
	$opt{i_stdbool} = 'define' unless $prebuilt;
    }
}
# find out which MSVC this ICC is using
elsif ($opt{cc} =~ /\bicl/) {
    my $output = `cl 2>&1`;
    my $num_ver = $output =~ /\b(\d+(?:\.\d+)+)\b/ ? $1 : '?';
    if ($num_ver =~ /^(\d+)/ && $1 >= 14) { # VC8+
	$opt{sGMTIME_max} = 32535291599;
	$opt{sLOCALTIME_max} = 32535244799;
    }
    if ($num_ver =~ /^(\d+)/ && $1 >= 16) { # VC10+
	$opt{i_stdint} = 'define';
    }
    if ($num_ver =~ /^(\d+)/ && $1 >= 19) { # VC14+
	$opt{stdio_base} = 'PERLIO_FILE_base(fp)';
	$opt{stdio_bufsiz} = '(PERLIO_FILE_cnt(fp) + PERLIO_FILE_ptr(fp) - PERLIO_FILE_base(fp))';
	$opt{stdio_cnt} = 'PERLIO_FILE_cnt(fp)';
	$opt{stdio_ptr} = 'PERLIO_FILE_ptr(fp)';
	$opt{i_stdbool} = 'define';
    }
    $opt{ar} ='xilib';
}

if ($opt{useithreads} eq 'define' && $opt{ccflags} =~ /-DPERL_IMPLICIT_SYS\b/) {
    $opt{d_pseudofork} = 'define';
}

if ($opt{usecplusplus} eq 'define') {
    $opt{d_cplusplus} = 'define';
    $opt{extern_C} = 'extern "C"';
}

#if the fields above are defined, they override the defaults in the premade
#config file
while (<>) {
    s/~([\w_]+)~/exists $opt{$1} ? $opt{$1} : ''/eg;
    if (/^([\w_]+)=(.*)$/) {
	my($k,$v) = ($1,$2);
	# this depends on cf_time being empty in the template (or we'll
	# get a loop)
	if ($k eq 'cf_time') {
	    $_ = "$k='" . localtime(time) . "'\n" if $v =~ /^\s*'\s*'/;
	}
	elsif (exists $opt{$k}) {
	    $_ = "$k='$opt{$k}'\n";
	}
    }
    print;
}
