package Unicode::Collate;

use 5.006;
use strict;
use warnings;
use Carp;
use File::Spec;

no warnings 'utf8';

our $VERSION = '1.31';
our $PACKAGE = __PACKAGE__;

### begin XS only ###
use XSLoader ();
XSLoader::load('Unicode::Collate', $VERSION);
### end XS only ###

my @Path = qw(Unicode Collate);
my $KeyFile = 'allkeys.txt';

# Perl's boolean
use constant TRUE  => 1;
use constant FALSE => "";
use constant NOMATCHPOS => -1;

# A coderef to get combining class imported from Unicode::Normalize
# (i.e. \&Unicode::Normalize::getCombinClass).
# This is also used as a HAS_UNICODE_NORMALIZE flag.
my $CVgetCombinClass;

# Supported Levels
use constant MinLevel => 1;
use constant MaxLevel => 4;

# Minimum weights at level 2 and 3, respectively
use constant Min2Wt => 0x20;
use constant Min3Wt => 0x02;

# Shifted weight at 4th level
use constant Shift4Wt => 0xFFFF;

# A boolean for Variable and 16-bit weights at 4 levels of Collation Element
use constant VCE_TEMPLATE => 'Cn4';

# A sort key: 16-bit weights
use constant KEY_TEMPLATE => 'n*';

# The tie-breaking: 32-bit weights
use constant TIE_TEMPLATE => 'N*';

# Level separator in a sort key:
# i.e. pack(KEY_TEMPLATE, 0)
use constant LEVEL_SEP => "\0\0";

# As Unicode code point separator for hash keys.
# A joined code point string (denoted by JCPS below)
# like "65;768" is used for internal processing
# instead of Perl's Unicode string like "\x41\x{300}",
# as the native code point is different from the Unicode code point
# on EBCDIC platform.
# This character must not be included in any stringified
# representation of an integer.
use constant CODE_SEP => ';';
	# NOTE: in regex /;/ is used for $jcps!

# boolean values of variable weights
use constant NON_VAR => 0; # Non-Variable character
use constant VAR     => 1; # Variable character

# specific code points
use constant Hangul_SIni   => 0xAC00;
use constant Hangul_SFin   => 0xD7A3;

# Logical_Order_Exception in PropList.txt
my $DefaultRearrange = [ 0x0E40..0x0E44, 0x0EC0..0x0EC4 ];

# for highestFFFF and minimalFFFE
my $HighestVCE = pack(VCE_TEMPLATE, 0, 0xFFFE, 0x20, 0x5, 0xFFFF);
my $minimalVCE = pack(VCE_TEMPLATE, 0,      1, 0x20, 0x5, 0xFFFE);

sub UCA_Version { '43' }

sub Base_Unicode_Version { '13.0.0' }

######

my $native_to_unicode = ($::IS_ASCII || $] < 5.008)
	? sub { return shift }
	: sub { utf8::native_to_unicode(shift) };

my $unicode_to_native = ($::IS_ASCII || $] < 5.008)
	? sub { return shift }
	: sub { utf8::unicode_to_native(shift) };

# pack_U() should get Unicode code points.
sub pack_U {
    return pack('U*', map $unicode_to_native->($_), @_);
}

# unpack_U() should return Unicode code points.
sub unpack_U {
    return map $native_to_unicode->($_), unpack('U*', shift(@_).pack('U*'));
}
# for older perl version, pack('U*') generates empty string with utf8 flag.

######

my (%VariableOK);
@VariableOK{ qw/
    blanked  non-ignorable  shifted  shift-trimmed
  / } = (); # keys lowercased

our @ChangeOK = qw/
    alternate backwards level normalization rearrange
    katakana_before_hiragana upper_before_lower ignore_level2
    overrideCJK overrideHangul overrideOut preprocess UCA_Version
    hangul_terminator variable identical highestFFFF minimalFFFE
    long_contraction
  /;

our @ChangeNG = qw/
    entry mapping table maxlength contraction
    ignoreChar ignoreName undefChar undefName rewrite
    versionTable alternateTable backwardsTable forwardsTable
    rearrangeTable variableTable
    derivCode normCode rearrangeHash backwardsFlag
    suppress suppressHash
    __useXS /; ### XS only
# The hash key 'ignored' was deleted at v 0.21.
# The hash key 'isShift' was deleted at v 0.23.
# The hash key 'combining' was deleted at v 0.24.
# The hash key 'entries' was deleted at v 0.30.
# The hash key 'L3_ignorable' was deleted at v 0.40.

sub version {
    my $self = shift;
    return $self->{versionTable} || 'unknown';
}

my (%ChangeOK, %ChangeNG);
@ChangeOK{ @ChangeOK } = ();
@ChangeNG{ @ChangeNG } = ();

sub change {
    my $self = shift;
    my %hash = @_;
    my %old;
    if (exists $hash{alternate}) {
	if (exists $hash{variable}) {
	    delete $hash{alternate};
	} else {
	    $hash{variable} = $hash{alternate};
	}
    }
    foreach my $k (keys %hash) {
	if (exists $ChangeOK{$k}) {
	    $old{$k} = $self->{$k};
	    $self->{$k} = $hash{$k};
	} elsif (exists $ChangeNG{$k}) {
	    croak "change of $k via change() is not allowed!";
	}
	# else => ignored
    }
    $self->checkCollator();
    return wantarray ? %old : $self;
}

sub _checkLevel {
    my $level = shift;
    my $key   = shift; # 'level' or 'backwards'
    MinLevel <= $level or croak sprintf
	"Illegal level %d (in value for key '%s') lower than %d.",
	    $level, $key, MinLevel;
    $level <= MaxLevel or croak sprintf
	"Unsupported level %d (in value for key '%s') higher than %d.",
	    $level, $key, MaxLevel;
}

my %DerivCode = (
    8 => \&_derivCE_8,
    9 => \&_derivCE_9,
   11 => \&_derivCE_9, # 11 == 9
   14 => \&_derivCE_14,
   16 => \&_derivCE_14, # 16 == 14
   18 => \&_derivCE_18,
   20 => \&_derivCE_20,
   22 => \&_derivCE_22,
   24 => \&_derivCE_24,
   26 => \&_derivCE_24, # 26 == 24
   28 => \&_derivCE_24, # 28 == 24
   30 => \&_derivCE_24, # 30 == 24
   32 => \&_derivCE_32,
   34 => \&_derivCE_34,
   36 => \&_derivCE_36,
   38 => \&_derivCE_38,
   40 => \&_derivCE_40,
   41 => \&_derivCE_40, # 41 == 40
   43 => \&_derivCE_43,
);

sub checkCollator {
    my $self = shift;
    _checkLevel($self->{level}, 'level');

    $self->{derivCode} = $DerivCode{ $self->{UCA_Version} }
	or croak "Illegal UCA version (passed $self->{UCA_Version}).";

    $self->{variable} ||= $self->{alternate} || $self->{variableTable} ||
				$self->{alternateTable} || 'shifted';
    $self->{variable} = $self->{alternate} = lc($self->{variable});
    exists $VariableOK{ $self->{variable} }
	or croak "$PACKAGE unknown variable parameter name: $self->{variable}";

    if (! defined $self->{backwards}) {
	$self->{backwardsFlag} = 0;
    } elsif (! ref $self->{backwards}) {
	_checkLevel($self->{backwards}, 'backwards');
	$self->{backwardsFlag} = 1 << $self->{backwards};
    } else {
	my %level;
	$self->{backwardsFlag} = 0;
	for my $b (@{ $self->{backwards} }) {
	    _checkLevel($b, 'backwards');
	    $level{$b} = 1;
	}
	for my $v (sort keys %level) {
	    $self->{backwardsFlag} += 1 << $v;
	}
    }

    defined $self->{rearrange} or $self->{rearrange} = [];
    ref $self->{rearrange}
	or croak "$PACKAGE: list for rearrangement must be store in ARRAYREF";

    # keys of $self->{rearrangeHash} are $self->{rearrange}.
    $self->{rearrangeHash} = undef;

    if (@{ $self->{rearrange} }) {
	@{ $self->{rearrangeHash} }{ @{ $self->{rearrange} } } = ();
    }

    $self->{normCode} = undef;

    if (defined $self->{normalization}) {
	eval { require Unicode::Normalize };
	$@ and croak "Unicode::Normalize is required to normalize strings";

	$CVgetCombinClass ||= \&Unicode::Normalize::getCombinClass;

	if ($self->{normalization} =~ /^(?:NF)D\z/) { # tweak for default
	    $self->{normCode} = \&Unicode::Normalize::NFD;
	}
	elsif ($self->{normalization} ne 'prenormalized') {
	    my $norm = $self->{normalization};
	    $self->{normCode} = sub {
		Unicode::Normalize::normalize($norm, shift);
	    };
	    eval { $self->{normCode}->("") }; # try
	    $@ and croak "$PACKAGE unknown normalization form name: $norm";
	}
    }
    return;
}

sub new
{
    my $class = shift;
    my $self = bless { @_ }, $class;

### begin XS only ###
    if (! exists $self->{table}     && !defined $self->{rewrite} &&
	!defined $self->{undefName} && !defined $self->{ignoreName} &&
	!defined $self->{undefChar} && !defined $self->{ignoreChar}) {
	$self->{__useXS} = \&_fetch_simple;
    } else {
	$self->{__useXS} = undef;
    }
### end XS only ###

    # keys of $self->{suppressHash} are $self->{suppress}.
    if ($self->{suppress} && @{ $self->{suppress} }) {
	@{ $self->{suppressHash} }{ @{ $self->{suppress} } } = ();
    } # before read_table()

    # If undef is passed explicitly, no file is read.
    $self->{table} = $KeyFile if ! exists $self->{table};
    $self->read_table() if defined $self->{table};

    if ($self->{entry}) {
	while ($self->{entry} =~ /([^\n]+)/g) {
	    $self->parseEntry($1, TRUE);
	}
    }

    # only in new(), not in change()
    $self->{level} ||= MaxLevel;
    $self->{UCA_Version} ||= UCA_Version();

    $self->{overrideHangul} = FALSE
	if ! exists $self->{overrideHangul};
    $self->{overrideCJK} = FALSE
	if ! exists $self->{overrideCJK};
    $self->{normalization} = 'NFD'
	if ! exists $self->{normalization};
    $self->{rearrange} = $self->{rearrangeTable} ||
	($self->{UCA_Version} <= 11 ? $DefaultRearrange : [])
	if ! exists $self->{rearrange};
    $self->{backwards} = $self->{backwardsTable}
	if ! exists $self->{backwards};
    exists $self->{long_contraction} or $self->{long_contraction}
	= 22 <= $self->{UCA_Version} && $self->{UCA_Version} <= 24;

    # checkCollator() will be called in change()
    $self->checkCollator();

    return $self;
}

sub parseAtmark {
    my $self = shift;
    my $line = shift; # after s/^\s*\@//

    if ($line =~ /^version\s*(\S*)/) {
	$self->{versionTable} ||= $1;
    }
    elsif ($line =~ /^variable\s+(\S*)/) { # since UTS #10-9
	$self->{variableTable} ||= $1;
    }
    elsif ($line =~ /^alternate\s+(\S*)/) { # till UTS #10-8
	$self->{alternateTable} ||= $1;
    }
    elsif ($line =~ /^backwards\s+(\S*)/) {
	push @{ $self->{backwardsTable} }, $1;
    }
    elsif ($line =~ /^forwards\s+(\S*)/) { # perhaps no use
	push @{ $self->{forwardsTable} }, $1;
    }
    elsif ($line =~ /^rearrange\s+(.*)/) { # (\S*) is NG
	push @{ $self->{rearrangeTable} }, _getHexArray($1);
    }
}

sub read_table {
    my $self = shift;

### begin XS only ###
    if ($self->{__useXS}) {
	my @rest = _fetch_rest(); # complex matter need to parse
	for my $line (@rest) {
	    next if $line =~ /^\s*#/;

	    if ($line =~ s/^\s*\@//) {
		$self->parseAtmark($line);
	    } else {
		$self->parseEntry($line);
	    }
	}
	return;
    }
### end XS only ###

    my($f, $fh);
    foreach my $d (@INC) {
	$f = File::Spec->catfile($d, @Path, $self->{table});
	last if open($fh, $f);
	$f = undef;
    }
    if (!defined $f) {
	$f = File::Spec->catfile(@Path, $self->{table});
	croak("$PACKAGE: Can't locate $f in \@INC (\@INC contains: @INC)");
    }

    while (my $line = <$fh>) {
	next if $line =~ /^\s*#/;

	if ($line =~ s/^\s*\@//) {
	    $self->parseAtmark($line);
	} else {
	    $self->parseEntry($line);
	}
    }
    close $fh;
}


##
## get $line, parse it, and write an entry in $self
##
sub parseEntry
{
    my $self = shift;
    my $line = shift;
    my $tailoring = shift;
    my($name, $entry, @uv, @key);

    if (defined $self->{rewrite}) {
	$line = $self->{rewrite}->($line);
    }

    return if $line !~ /^\s*[0-9A-Fa-f]/;

    # removes comment and gets name
    $name = $1
	if $line =~ s/[#%]\s*(.*)//;
    return if defined $self->{undefName} && $name =~ /$self->{undefName}/;

    # gets element
    my($e, $k) = split /;/, $line;
    croak "Wrong Entry: <charList> must be separated by ';' from <collElement>"
	if ! $k;

    @uv = _getHexArray($e);
    return if !@uv;
    return if @uv > 1 && $self->{suppressHash} && !$tailoring &&
		  exists $self->{suppressHash}{$uv[0]};
    $entry = join(CODE_SEP, @uv); # in JCPS

    if (defined $self->{undefChar} || defined $self->{ignoreChar}) {
	my $ele = pack_U(@uv);

	# regarded as if it were not stored in the table
	return
	    if defined $self->{undefChar} && $ele =~ /$self->{undefChar}/;

	# replaced as completely ignorable
	$k = '[.0000.0000.0000.0000]'
	    if defined $self->{ignoreChar} && $ele =~ /$self->{ignoreChar}/;
    }

    # replaced as completely ignorable
    $k = '[.0000.0000.0000.0000]'
	if defined $self->{ignoreName} && $name =~ /$self->{ignoreName}/;

    my $is_L3_ignorable = TRUE;

    foreach my $arr ($k =~ /\[([^\[\]]+)\]/g) { # SPACEs allowed
	my $var = $arr =~ /\*/; # exactly /^\*/ but be lenient.
	my @wt = _getHexArray($arr);
	push @key, pack(VCE_TEMPLATE, $var, @wt);
	$is_L3_ignorable = FALSE
	    if $wt[0] || $wt[1] || $wt[2];
	# Conformance Test for 3.1.1 and 4.0.0 shows Level 3 ignorable
	# is completely ignorable.
	# For expansion, an entry $is_L3_ignorable
	# if and only if "all" CEs are [.0000.0000.0000].
    }

    # mapping: be an array ref or not exists (any false value is disallowed)
    $self->{mapping}{$entry} = $is_L3_ignorable ? [] : \@key;

    # maxlength: be more than 1 or not exists (any false value is disallowed)
    if (@uv > 1) {
	if (!$self->{maxlength}{$uv[0]} || $self->{maxlength}{$uv[0]} < @uv) {
	    $self->{maxlength}{$uv[0]} = @uv;
	}
    }

    # contraction: be 1 or not exists (any false value is disallowed)
    while (@uv > 2) {
	pop @uv;
	my $fake_entry = join(CODE_SEP, @uv); # in JCPS
	$self->{contraction}{$fake_entry} = 1;
    }
}


sub viewSortKey
{
    my $self = shift;
    my $str  = shift;
    $self->visualizeSortKey($self->getSortKey($str));
}


sub process
{
    my $self = shift;
    my $str  = shift;
    my $prep = $self->{preprocess};
    my $norm = $self->{normCode};

    $str = &$prep($str) if ref $prep;
    $str = &$norm($str) if ref $norm;
    return $str;
}

##
## arrayref of JCPS   = splitEnt(string to be collated)
## arrayref of arrayref[JCPS, ini_pos, fin_pos] = splitEnt(string, TRUE)
##
sub splitEnt
{
    my $self = shift;
    my $str  = shift;
    my $wLen = shift; # with Length

    my $map  = $self->{mapping};
    my $max  = $self->{maxlength};
    my $reH  = $self->{rearrangeHash};
    my $vers = $self->{UCA_Version};
    my $ver9 = $vers >= 9 && $vers <= 11;
    my $long = $self->{long_contraction};
    my $uXS  = $self->{__useXS}; ### XS only

    my @buf;

    # get array of Unicode code point of string.
    my @src = unpack_U($str);

    # rearrangement:
    # Character positions are not kept if rearranged,
    # then neglected if $wLen is true.
    if ($reH && ! $wLen) {
	for (my $i = 0; $i < @src; $i++) {
	    if (exists $reH->{ $src[$i] } && $i + 1 < @src) {
		($src[$i], $src[$i+1]) = ($src[$i+1], $src[$i]);
		$i++;
	    }
	}
    }

    # remove a code point marked as a completely ignorable.
    for (my $i = 0; $i < @src; $i++) {
	if ($vers <= 20 && _isIllegal($src[$i])) {
	    $src[$i] = undef;
	} elsif ($ver9) {
	    $src[$i] = undef if exists $map->{ $src[$i] }
			   ? @{ $map->{ $src[$i] } } == 0
			   : $uXS && _ignorable_simple($src[$i]); ### XS only
	}
    }

    for (my $i = 0; $i < @src; $i++) {
	my $jcps = $src[$i];

	# skip removed code point
	if (! defined $jcps) {
	    if ($wLen && @buf) {
		$buf[-1][2] = $i + 1;
	    }
	    next;
	}

	my $i_orig = $i;

	# find contraction
	if (exists $max->{$jcps}) {
	    my $temp_jcps = $jcps;
	    my $jcpsLen = 1;
	    my $maxLen = $max->{$jcps};

	    for (my $p = $i + 1; $jcpsLen < $maxLen && $p < @src; $p++) {
		next if ! defined $src[$p];
		$temp_jcps .= CODE_SEP . $src[$p];
		$jcpsLen++;
		if (exists $map->{$temp_jcps}) {
		    $jcps = $temp_jcps;
		    $i = $p;
		}
	    }

	# discontiguous contraction with Combining Char (cf. UTS#10, S2.1).
	# This process requires Unicode::Normalize.
	# If "normalization" is undef, here should be skipped *always*
	# (in spite of bool value of $CVgetCombinClass),
	# since canonical ordering cannot be expected.
	# Blocked combining character should not be contracted.

	    # $self->{normCode} is false in the case of "prenormalized".
	    if ($self->{normalization}) {
		my $cont = $self->{contraction};
		my $preCC = 0;
		my $preCC_uc = 0;
		my $jcps_uc = $jcps;
		my(@out, @out_uc);

		for (my $p = $i + 1; $p < @src; $p++) {
		    next if ! defined $src[$p];
		    my $curCC = $CVgetCombinClass->($src[$p]);
		    last unless $curCC;
		    my $tail = CODE_SEP . $src[$p];

		    if ($preCC != $curCC && exists $map->{$jcps.$tail}) {
			$jcps .= $tail;
			push @out, $p;
		    } else {
			$preCC = $curCC;
		    }

		    next if !$long;

		    if ($preCC_uc != $curCC &&
			    (exists $map->{$jcps_uc.$tail} ||
			    exists $cont->{$jcps_uc.$tail})) {
			$jcps_uc .= $tail;
			push @out_uc, $p;
		    } else {
			$preCC_uc = $curCC;
		    }
		}

		if (@out_uc && exists $map->{$jcps_uc}) {
		    $jcps = $jcps_uc;
		    $src[$_] = undef for @out_uc;
		} else {
		    $src[$_] = undef for @out;
		}
	    }
	}

	# skip completely ignorable
	if (exists $map->{$jcps} ? @{ $map->{$jcps} } == 0 :
	    $uXS && $jcps !~ /;/ && _ignorable_simple($jcps)) { ### XS only
	    if ($wLen && @buf) {
		$buf[-1][2] = $i + 1;
	    }
	    next;
	}

	push @buf, $wLen ? [$jcps, $i_orig, $i + 1] : $jcps;
    }
    return \@buf;
}

##
## VCE = _pack_override(input, codepoint, derivCode)
##
sub _pack_override ($$$) {
    my $r = shift;
    my $u = shift;
    my $der = shift;

    if (ref $r) {
	return pack(VCE_TEMPLATE, NON_VAR, @$r);
    } elsif (defined $r) {
	return pack(VCE_TEMPLATE, NON_VAR, $r, Min2Wt, Min3Wt, $u);
    } else {
	$u = 0xFFFD if 0x10FFFF < $u;
	return $der->($u);
    }
}

##
## list of VCE = getWt(JCPS)
##
sub getWt
{
    my $self = shift;
    my $u    = shift;
    my $map  = $self->{mapping};
    my $der  = $self->{derivCode};
    my $out  = $self->{overrideOut};
    my $uXS  = $self->{__useXS}; ### XS only

    return if !defined $u;
    return $self->varCE($HighestVCE) if $u eq 0xFFFF && $self->{highestFFFF};
    return $self->varCE($minimalVCE) if $u eq 0xFFFE && $self->{minimalFFFE};
    $u = 0xFFFD if $u !~ /;/ && 0x10FFFF < $u && !$out;

    my @ce;
    if (exists $map->{$u}) {
	@ce = @{ $map->{$u} }; # $u may be a contraction
### begin XS only ###
    } elsif ($uXS && _exists_simple($u)) {
	@ce = _fetch_simple($u);
### end XS only ###
    } elsif (Hangul_SIni <= $u && $u <= Hangul_SFin) {
	my $hang = $self->{overrideHangul};
	if ($hang) {
	    @ce = map _pack_override($_, $u, $der), $hang->($u);
	} elsif (!defined $hang) {
	    @ce = $der->($u);
	} else {
	    my $max  = $self->{maxlength};
	    my @decH = _decompHangul($u);

	    if (@decH == 2) {
		my $contract = join(CODE_SEP, @decH);
		@decH = ($contract) if exists $map->{$contract};
	    } else { # must be <@decH == 3>
		if (exists $max->{$decH[0]}) {
		    my $contract = join(CODE_SEP, @decH);
		    if (exists $map->{$contract}) {
			@decH = ($contract);
		    } else {
			$contract = join(CODE_SEP, @decH[0,1]);
			exists $map->{$contract} and @decH = ($contract, $decH[2]);
		    }
		    # even if V's ignorable, LT contraction is not supported.
		    # If such a situation were required, NFD should be used.
		}
		if (@decH == 3 && exists $max->{$decH[1]}) {
		    my $contract = join(CODE_SEP, @decH[1,2]);
		    exists $map->{$contract} and @decH = ($decH[0], $contract);
		}
	    }

	    @ce = map({
		    exists $map->{$_} ? @{ $map->{$_} } :
		$uXS && _exists_simple($_) ? _fetch_simple($_) : ### XS only
		    $der->($_);
		} @decH);
	}
    } elsif ($out && 0x10FFFF < $u) {
	@ce = map _pack_override($_, $u, $der), $out->($u);
    } else {
	my $cjk  = $self->{overrideCJK};
	my $vers = $self->{UCA_Version};
	if ($cjk && _isUIdeo($u, $vers)) {
	    @ce = map _pack_override($_, $u, $der), $cjk->($u);
	} elsif ($vers == 8 && defined $cjk && _isUIdeo($u, 0)) {
	    @ce = _uideoCE_8($u);
	} else {
	    @ce = $der->($u);
	}
    }
    return map $self->varCE($_), @ce;
}


##
## string sortkey = getSortKey(string arg)
##
sub getSortKey
{
    my $self = shift;
    my $orig = shift;
    my $str  = $self->process($orig);
    my $rEnt = $self->splitEnt($str); # get an arrayref of JCPS
    my $vers = $self->{UCA_Version};
    my $term = $self->{hangul_terminator};
    my $lev  = $self->{level};
    my $iden = $self->{identical};

    my @buf; # weight arrays
    if ($term) {
	my $preHST = '';
	my $termCE = $self->varCE(pack(VCE_TEMPLATE, NON_VAR, $term, 0,0,0));
	foreach my $jcps (@$rEnt) {
	    # weird things like VL, TL-contraction are not considered!
	    my $curHST = join '', map getHST($_, $vers), split /;/, $jcps;
	    if ($preHST && !$curHST || # hangul before non-hangul
		$preHST =~ /L\z/ && $curHST =~ /^T/ ||
		$preHST =~ /V\z/ && $curHST =~ /^L/ ||
		$preHST =~ /T\z/ && $curHST =~ /^[LV]/) {
		push @buf, $termCE;
	    }
	    $preHST = $curHST;
	    push @buf, $self->getWt($jcps);
	}
	push @buf, $termCE if $preHST; # end at hangul
    } else {
	foreach my $jcps (@$rEnt) {
	    push @buf, $self->getWt($jcps);
	}
    }

    my $rkey = $self->mk_SortKey(\@buf); ### XS only

    if ($iden || $vers >= 26 && $lev == MaxLevel) {
	$rkey .= LEVEL_SEP;
	$rkey .= pack(TIE_TEMPLATE, unpack_U($str)) if $iden;
    }
    return $rkey;
}


##
## int compare = cmp(string a, string b)
##
sub cmp { $_[0]->getSortKey($_[1]) cmp $_[0]->getSortKey($_[2]) }
sub eq  { $_[0]->getSortKey($_[1]) eq  $_[0]->getSortKey($_[2]) }
sub ne  { $_[0]->getSortKey($_[1]) ne  $_[0]->getSortKey($_[2]) }
sub lt  { $_[0]->getSortKey($_[1]) lt  $_[0]->getSortKey($_[2]) }
sub le  { $_[0]->getSortKey($_[1]) le  $_[0]->getSortKey($_[2]) }
sub gt  { $_[0]->getSortKey($_[1]) gt  $_[0]->getSortKey($_[2]) }
sub ge  { $_[0]->getSortKey($_[1]) ge  $_[0]->getSortKey($_[2]) }

##
## list[strings] sorted = sort(list[strings] arg)
##
sub sort {
    my $obj = shift;
    return
	map { $_->[1] }
	    sort{ $a->[0] cmp $b->[0] }
		map [ $obj->getSortKey($_), $_ ], @_;
}


##
## bool _nonIgnorAtLevel(arrayref weights, int level)
##
sub _nonIgnorAtLevel($$)
{
    my $wt = shift;
    return if ! defined $wt;
    my $lv = shift;
    return grep($wt->[$_-1] != 0, MinLevel..$lv) ? TRUE : FALSE;
}

##
## bool _eqArray(
##    arrayref of arrayref[weights] source,
##    arrayref of arrayref[weights] substr,
##    int level)
## * comparison of graphemes vs graphemes.
##   @$source >= @$substr must be true (check it before call this);
##
sub _eqArray($$$)
{
    my $source = shift;
    my $substr = shift;
    my $lev = shift;

    for my $g (0..@$substr-1){
	# Do the $g'th graphemes have the same number of AV weights?
	return if @{ $source->[$g] } != @{ $substr->[$g] };

	for my $w (0..@{ $substr->[$g] }-1) {
	    for my $v (0..$lev-1) {
		return if $source->[$g][$w][$v] != $substr->[$g][$w][$v];
	    }
	}
    }
    return 1;
}

##
## (int position, int length)
## int position = index(string, substring, position, [undoc'ed global])
##
## With "global" (only for the list context),
##  returns list of arrayref[position, length].
##
sub index
{
    my $self = shift;
    $self->{preprocess} and
	croak "Don't use Preprocess with index(), match(), etc.";
    $self->{normCode} and
	croak "Don't use Normalization with index(), match(), etc.";

    my $str  = shift;
    my $len  = length($str);
    my $sub  = shift;
    my $subE = $self->splitEnt($sub);
    my $pos  = @_ ? shift : 0;
       $pos  = 0 if $pos < 0;
    my $glob = shift;

    my $lev  = $self->{level};
    my $v2i  = $self->{UCA_Version} >= 9 &&
		$self->{variable} ne 'non-ignorable';

    if (! @$subE) {
	my $temp = $pos <= 0 ? 0 : $len <= $pos ? $len : $pos;
	return $glob
	    ? map([$_, 0], $temp..$len)
	    : wantarray ? ($temp,0) : $temp;
    }
    $len < $pos
	and return wantarray ? () : NOMATCHPOS;
    my $strE = $self->splitEnt($pos ? substr($str, $pos) : $str, TRUE);
    @$strE
	or return wantarray ? () : NOMATCHPOS;

    my(@strWt, @iniPos, @finPos, @subWt, @g_ret);

    my $last_is_variable;
    for my $vwt (map $self->getWt($_), @$subE) {
	my($var, @wt) = unpack(VCE_TEMPLATE, $vwt);
	my $to_be_pushed = _nonIgnorAtLevel(\@wt,$lev);

	# "Ignorable (L1, L2) after Variable" since track. v. 9
	if ($v2i) {
	    if ($var) {
		$last_is_variable = TRUE;
	    }
	    elsif (!$wt[0]) { # ignorable
		$to_be_pushed = FALSE if $last_is_variable;
	    }
	    else {
		$last_is_variable = FALSE;
	    }
	}

	if (@subWt && !$var && !$wt[0]) {
	    push @{ $subWt[-1] }, \@wt if $to_be_pushed;
	} elsif ($to_be_pushed) {
	    push @subWt, [ \@wt ];
	}
	# else ===> skipped
    }

    my $count = 0;
    my $end = @$strE - 1;

    $last_is_variable = FALSE; # reuse
    for (my $i = 0; $i <= $end; ) { # no $i++
	my $found_base = 0;

	# fetch a grapheme
	while ($i <= $end && $found_base == 0) {
	    for my $vwt ($self->getWt($strE->[$i][0])) {
		my($var, @wt) = unpack(VCE_TEMPLATE, $vwt);
		my $to_be_pushed = _nonIgnorAtLevel(\@wt,$lev);

		# "Ignorable (L1, L2) after Variable" since track. v. 9
		if ($v2i) {
		    if ($var) {
			$last_is_variable = TRUE;
		    }
		    elsif (!$wt[0]) { # ignorable
			$to_be_pushed = FALSE if $last_is_variable;
		    }
		    else {
			$last_is_variable = FALSE;
		    }
		}

		if (@strWt && !$var && !$wt[0]) {
		    push @{ $strWt[-1] }, \@wt if $to_be_pushed;
		    $finPos[-1] = $strE->[$i][2];
		} elsif ($to_be_pushed) {
		    push @strWt, [ \@wt ];
		    push @iniPos, $found_base ? NOMATCHPOS : $strE->[$i][1];
		    $finPos[-1] = NOMATCHPOS if $found_base;
		    push @finPos, $strE->[$i][2];
		    $found_base++;
		}
		# else ===> no-op
	    }
	    $i++;
	}

	# try to match
	while ( @strWt > @subWt || (@strWt == @subWt && $i > $end) ) {
	    if ($iniPos[0] != NOMATCHPOS &&
		    $finPos[$#subWt] != NOMATCHPOS &&
			_eqArray(\@strWt, \@subWt, $lev)) {
		my $temp = $iniPos[0] + $pos;

		if ($glob) {
		    push @g_ret, [$temp, $finPos[$#subWt] - $iniPos[0]];
		    splice @strWt,  0, $#subWt;
		    splice @iniPos, 0, $#subWt;
		    splice @finPos, 0, $#subWt;
		}
		else {
		    return wantarray
			? ($temp, $finPos[$#subWt] - $iniPos[0])
			:  $temp;
		}
	    }
	    shift @strWt;
	    shift @iniPos;
	    shift @finPos;
	}
    }

    return $glob
	? @g_ret
	: wantarray ? () : NOMATCHPOS;
}

##
## scalarref to matching part = match(string, substring)
##
sub match
{
    my $self = shift;
    if (my($pos,$len) = $self->index($_[0], $_[1])) {
	my $temp = substr($_[0], $pos, $len);
	return wantarray ? $temp : \$temp;
	# An lvalue ref \substr should be avoided,
	# since its value is affected by modification of its referent.
    }
    else {
	return;
    }
}

##
## arrayref matching parts = gmatch(string, substring)
##
sub gmatch
{
    my $self = shift;
    my $str  = shift;
    my $sub  = shift;
    return map substr($str, $_->[0], $_->[1]),
		$self->index($str, $sub, 0, 'g');
}

##
## bool subst'ed = subst(string, substring, replace)
##
sub subst
{
    my $self = shift;
    my $code = ref $_[2] eq 'CODE' ? $_[2] : FALSE;

    if (my($pos,$len) = $self->index($_[0], $_[1])) {
	if ($code) {
	    my $mat = substr($_[0], $pos, $len);
	    substr($_[0], $pos, $len, $code->($mat));
	} else {
	    substr($_[0], $pos, $len, $_[2]);
	}
	return TRUE;
    }
    else {
	return FALSE;
    }
}

##
## int count = gsubst(string, substring, replace)
##
sub gsubst
{
    my $self = shift;
    my $code = ref $_[2] eq 'CODE' ? $_[2] : FALSE;
    my $cnt = 0;

    # Replacement is carried out from the end, then use reverse.
    for my $pos_len (reverse $self->index($_[0], $_[1], 0, 'g')) {
	if ($code) {
	    my $mat = substr($_[0], $pos_len->[0], $pos_len->[1]);
	    substr($_[0], $pos_len->[0], $pos_len->[1], $code->($mat));
	} else {
	    substr($_[0], $pos_len->[0], $pos_len->[1], $_[2]);
	}
	$cnt++;
    }
    return $cnt;
}

1;
__END__

=head1 NAME

Unicode::Collate - Unicode Collation Algorithm

=head1 SYNOPSIS

  use Unicode::Collate;

  #construct
  $Collator = Unicode::Collate->new(%tailoring);

  #sort
  @sorted = $Collator->sort(@not_sorted);

  #compare
  $result = $Collator->cmp($a, $b); # returns 1, 0, or -1.

B<Note:> Strings in C<@not_sorted>, C<$a> and C<$b> are interpreted
according to Perl's Unicode support. See L<perlunicode>,
L<perluniintro>, L<perlunitut>, L<perlunifaq>, L<utf8>.
Otherwise you can use C<preprocess> or should decode them before.

=head1 DESCRIPTION

This module is an implementation of Unicode Technical Standard #10
(a.k.a. UTS #10) - Unicode Collation Algorithm (a.k.a. UCA).

=head2 Constructor and Tailoring

The C<new> method returns a collator object. If new() is called
with no parameters, the collator should do the default collation.

   $Collator = Unicode::Collate->new(
      UCA_Version => $UCA_Version,
      alternate => $alternate, # alias for 'variable'
      backwards => $levelNumber, # or \@levelNumbers
      entry => $element,
      hangul_terminator => $term_primary_weight,
      highestFFFF => $bool,
      identical => $bool,
      ignoreName => qr/$ignoreName/,
      ignoreChar => qr/$ignoreChar/,
      ignore_level2 => $bool,
      katakana_before_hiragana => $bool,
      level => $collationLevel,
      long_contraction => $bool,
      minimalFFFE => $bool,
      normalization  => $normalization_form,
      overrideCJK => \&overrideCJK,
      overrideHangul => \&overrideHangul,
      preprocess => \&preprocess,
      rearrange => \@charList,
      rewrite => \&rewrite,
      suppress => \@charList,
      table => $filename,
      undefName => qr/$undefName/,
      undefChar => qr/$undefChar/,
      upper_before_lower => $bool,
      variable => $variable,
   );

=over 4

=item UCA_Version

If the revision (previously "tracking version") number of UCA is given,
behavior of that revision is emulated on collating.
If omitted, the return value of C<UCA_Version()> is used.

The following revisions are supported.  The default is 43.

     UCA       Unicode Standard         DUCET (@version)
   -------------------------------------------------------
      8              3.1                3.0.1 (3.0.1d9)
      9     3.1 with Corrigendum 3      3.1.1
     11             4.0.0
     14             4.1.0
     16             5.0.0
     18             5.1.0
     20             5.2.0
     22             6.0.0
     24             6.1.0
     26             6.2.0
     28             6.3.0
     30             7.0.0
     32             8.0.0
     34             9.0.0
     36            10.0.0
     38            11.0.0
     40            12.0.0
     41            12.1.0
     43            13.0.0

* See below for C<long_contraction> with C<UCA_Version> 22 and 24.

* Noncharacters (e.g. U+FFFF) are not ignored, and can be overridden
since C<UCA_Version> 22.

* Out-of-range codepoints (greater than U+10FFFF) are not ignored,
and can be overridden since C<UCA_Version> 22.

* Fully ignorable characters were ignored, and would not interrupt
contractions with C<UCA_Version> 9 and 11.

* Treatment of ignorables after variables and some behaviors
were changed at C<UCA_Version> 9.

* Characters regarded as CJK unified ideographs (cf. C<overrideCJK>)
depend on C<UCA_Version>.

* Many hangul jamo are assigned at C<UCA_Version> 20, that will affect
C<hangul_terminator>.

=item alternate

-- see 3.2.2 Alternate Weighting, version 8 of UTS #10

For backward compatibility, C<alternate> (old name) can be used
as an alias for C<variable>.

=item backwards

-- see 3.4 Backward Accents, UTS #10.

     backwards => $levelNumber or \@levelNumbers

Weights in reverse order; ex. level 2 (diacritic ordering) in French.
If omitted (or C<$levelNumber> is C<undef> or C<\@levelNumbers> is C<[]>),
forwards at all the levels.

=item entry

-- see 5 Tailoring; 9.1 Allkeys File Format, UTS #10.

If the same character (or a sequence of characters) exists
in the collation element table through C<table>,
mapping to collation elements is overridden.
If it does not exist, the mapping is defined additionally.

    entry => <<'ENTRY', # for DUCET v4.0.0 (allkeys-4.0.0.txt)
0063 0068 ; [.0E6A.0020.0002.0063] # ch
0043 0068 ; [.0E6A.0020.0007.0043] # Ch
0043 0048 ; [.0E6A.0020.0008.0043] # CH
006C 006C ; [.0F4C.0020.0002.006C] # ll
004C 006C ; [.0F4C.0020.0007.004C] # Ll
004C 004C ; [.0F4C.0020.0008.004C] # LL
00F1      ; [.0F7B.0020.0002.00F1] # n-tilde
006E 0303 ; [.0F7B.0020.0002.00F1] # n-tilde
00D1      ; [.0F7B.0020.0008.00D1] # N-tilde
004E 0303 ; [.0F7B.0020.0008.00D1] # N-tilde
ENTRY

    entry => <<'ENTRY', # for DUCET v4.0.0 (allkeys-4.0.0.txt)
00E6 ; [.0E33.0020.0002.00E6][.0E8B.0020.0002.00E6] # ae ligature as <a><e>
00C6 ; [.0E33.0020.0008.00C6][.0E8B.0020.0008.00C6] # AE ligature as <A><E>
ENTRY

B<NOTE:> The code point in the UCA file format (before C<';'>)
B<must> be a Unicode code point (defined as hexadecimal),
but not a native code point.
So C<0063> must always denote C<U+0063>,
but not a character of C<"\x63">.

Weighting may vary depending on collation element table.
So ensure the weights defined in C<entry> will be consistent with
those in the collation element table loaded via C<table>.

In DUCET v4.0.0, primary weight of C<C> is C<0E60>
and that of C<D> is C<0E6D>. So setting primary weight of C<CH> to C<0E6A>
(as a value between C<0E60> and C<0E6D>)
makes ordering as C<C E<lt> CH E<lt> D>.
Exactly speaking DUCET already has some characters between C<C> and C<D>:
C<small capital C> (C<U+1D04>) with primary weight C<0E64>,
C<c-hook/C-hook> (C<U+0188/U+0187>) with C<0E65>,
and C<c-curl> (C<U+0255>) with C<0E69>.
Then primary weight C<0E6A> for C<CH> makes C<CH>
ordered between C<c-curl> and C<D>.

=item hangul_terminator

-- see 7.1.4 Trailing Weights, UTS #10.

If a true value is given (non-zero but should be positive),
it will be added as a terminator primary weight to the end of
every standard Hangul syllable. Secondary and any higher weights
for terminator are set to zero.
If the value is false or C<hangul_terminator> key does not exist,
insertion of terminator weights will not be performed.

Boundaries of Hangul syllables are determined
according to conjoining Jamo behavior in F<the Unicode Standard>
and F<HangulSyllableType.txt>.

B<Implementation Note:>
(1) For expansion mapping (Unicode character mapped
to a sequence of collation elements), a terminator will not be added
between collation elements, even if Hangul syllable boundary exists there.
Addition of terminator is restricted to the next position
to the last collation element.

(2) Non-conjoining Hangul letters
(Compatibility Jamo, halfwidth Jamo, and enclosed letters) are not
automatically terminated with a terminator primary weight.
These characters may need terminator included in a collation element
table beforehand.

=item highestFFFF

-- see 2.4 Tailored noncharacter weights, UTS #35 (LDML) Part 5: Collation.

If the parameter is made true, C<U+FFFF> has a highest primary weight.
When a boolean of C<$coll-E<gt>ge($str, "abc")> and
C<$coll-E<gt>le($str, "abc\x{FFFF}")> is true, it is expected that C<$str>
begins with C<"abc">, or another primary equivalent.
C<$str> may be C<"abcd">, C<"abc012">, but should not include C<U+FFFF>
such as C<"abc\x{FFFF}xyz">.

C<$coll-E<gt>le($str, "abc\x{FFFF}")> works like C<$coll-E<gt>lt($str, "abd")>
almost, but the latter has a problem that you should know which letter is
next to C<c>. For a certain language where C<ch> as the next letter,
C<"abch"> is greater than C<"abc\x{FFFF}">, but less than C<"abd">.

Note:
This is equivalent to C<(entry =E<gt> 'FFFF ; [.FFFE.0020.0005.FFFF]')>.
Any other character than C<U+FFFF> can be tailored by C<entry>.

=item identical

-- see A.3 Deterministic Comparison, UTS #10.

By default, strings whose weights are equal should be equal,
even though their code points are not equal.
Completely ignorable characters are ignored.

If the parameter is made true, a final, tie-breaking level is used.
If no difference of weights is found after the comparison through
all the level specified by C<level>, the comparison with code points
will be performed.
For the tie-breaking comparison, the sort key has code points
of the original string appended.
Completely ignorable characters are not ignored.

If C<preprocess> and/or C<normalization> is applied, the code points
of the string after them (in NFD by default) are used.

=item ignoreChar

=item ignoreName

-- see 3.6 Variable Weighting, UTS #10.

Makes the entry in the table completely ignorable;
i.e. as if the weights were zero at all level.

Through C<ignoreChar>, any character matching C<qr/$ignoreChar/>
will be ignored. Through C<ignoreName>, any character whose name
(given in the C<table> file as a comment) matches C<qr/$ignoreName/>
will be ignored.

E.g. when 'a' and 'e' are ignorable,
'element' is equal to 'lament' (or 'lmnt').

=item ignore_level2

-- see 5.1 Parametric Tailoring, UTS #10.

By default, case-sensitive comparison (that is level 3 difference)
won't ignore accents (that is level 2 difference).

If the parameter is made true, accents (and other primary ignorable
characters) are ignored, even though cases are taken into account.

B<NOTE>: C<level> should be 3 or greater.

=item katakana_before_hiragana

-- see 7.2 Tertiary Weight Table, UTS #10.

By default, hiragana is before katakana.
If the parameter is made true, this is reversed.

B<NOTE>: This parameter simplemindedly assumes that any hiragana/katakana
distinctions must occur in level 3, and their weights at level 3 must be
same as those mentioned in 7.3.1, UTS #10.
If you define your collation elements which violate this requirement,
this parameter does not work validly.

=item level

-- see 4.3 Form Sort Key, UTS #10.

Set the maximum level.
Any higher levels than the specified one are ignored.

  Level 1: alphabetic ordering
  Level 2: diacritic ordering
  Level 3: case ordering
  Level 4: tie-breaking (e.g. in the case when variable is 'shifted')

  ex.level => 2,

If omitted, the maximum is the 4th.

B<NOTE:> The DUCET includes weights over 0xFFFF at the 4th level.
But this module only uses weights within 0xFFFF.
When C<variable> is 'blanked' or 'non-ignorable' (other than 'shifted'
and 'shift-trimmed'), the level 4 may be unreliable.

See also C<identical>.

=item long_contraction

-- see 3.8.2 Well-Formedness of the DUCET, 4.2 Produce Array, UTS #10.

If the parameter is made true, for a contraction with three or more
characters (here nicknamed "long contraction"), initial substrings
will be handled.
For example, a contraction ABC, where A is a starter, and B and C
are non-starters (character with non-zero combining character class),
will be detected even if there is not AB as a contraction.

B<Default:> Usually false.
If C<UCA_Version> is 22 or 24, and the value of C<long_contraction>
is not specified in C<new()>, a true value is set implicitly.
This is a workaround to pass Conformance Tests for Unicode 6.0.0 and 6.1.0.

C<change()> handles C<long_contraction> explicitly only.
If C<long_contraction> is not specified in C<change()>, even though
C<UCA_Version> is changed, C<long_contraction> will not be changed.

B<Limitation:> Scanning non-starters is one-way (no back tracking).
If AB is found but not ABC is not found, other long contraction where
the first character is A and the second is not B may not be found.

Under C<(normalization =E<gt> undef)>, detection step of discontiguous
contractions will be skipped.

B<Note:> The following contractions in DUCET are not considered
in steps S2.1.1 to S2.1.3, where they are discontiguous.

    0FB2 0F71 0F80 (TIBETAN VOWEL SIGN VOCALIC RR)
    0FB3 0F71 0F80 (TIBETAN VOWEL SIGN VOCALIC LL)

For example C<TIBETAN VOWEL SIGN VOCALIC RR> with C<COMBINING TILDE OVERLAY>
(C<U+0344>) is C<0FB2 0344 0F71 0F80> in NFD.
In this case C<0FB2 0F80> (C<TIBETAN VOWEL SIGN VOCALIC R>) is detected,
instead of C<0FB2 0F71 0F80>.
Inserted C<0344> makes C<0FB2 0F71 0F80> discontiguous and lack of
contraction C<0FB2 0F71> prohibits C<0FB2 0F71 0F80> from being detected.

=item minimalFFFE

-- see 1.1.1 U+FFFE, UTS #35 (LDML) Part 5: Collation.

If the parameter is made true, C<U+FFFE> has a minimal primary weight.
The comparison between C<"$a1\x{FFFE}$a2"> and C<"$b1\x{FFFE}$b2">
first compares C<$a1> and C<$b1> at level 1, and
then C<$a2> and C<$b2> at level 1, as followed.

        "ab\x{FFFE}a"
        "Ab\x{FFFE}a"
        "ab\x{FFFE}c"
        "Ab\x{FFFE}c"
        "ab\x{FFFE}xyz"
        "abc\x{FFFE}def"
        "abc\x{FFFE}xYz"
        "aBc\x{FFFE}xyz"
        "abcX\x{FFFE}def"
        "abcx\x{FFFE}xyz"
        "b\x{FFFE}aaa"
        "bbb\x{FFFE}a"

Note:
This is equivalent to C<(entry =E<gt> 'FFFE ; [.0001.0020.0005.FFFE]')>.
Any other character than C<U+FFFE> can be tailored by C<entry>.

=item normalization

-- see 4.1 Normalize, UTS #10.

If specified, strings are normalized before preparation of sort keys
(the normalization is executed after preprocess).

A form name C<Unicode::Normalize::normalize()> accepts will be applied
as C<$normalization_form>.
Acceptable names include C<'NFD'>, C<'NFC'>, C<'NFKD'>, and C<'NFKC'>.
See C<Unicode::Normalize::normalize()> for detail.
If omitted, C<'NFD'> is used.

C<normalization> is performed after C<preprocess> (if defined).

Furthermore, special values, C<undef> and C<"prenormalized">, can be used,
though they are not concerned with C<Unicode::Normalize::normalize()>.

If C<undef> (not a string C<"undef">) is passed explicitly
as the value for this key,
any normalization is not carried out (this may make tailoring easier
if any normalization is not desired). Under C<(normalization =E<gt> undef)>,
only contiguous contractions are resolved;
e.g. even if C<A-ring> (and C<A-ring-cedilla>) is ordered after C<Z>,
C<A-cedilla-ring> would be primary equal to C<A>.
In this point,
C<(normalization =E<gt> undef, preprocess =E<gt> sub { NFD(shift) })>
B<is not> equivalent to C<(normalization =E<gt> 'NFD')>.

In the case of C<(normalization =E<gt> "prenormalized")>,
any normalization is not performed, but
discontiguous contractions with combining characters are performed.
Therefore
C<(normalization =E<gt> 'prenormalized', preprocess =E<gt> sub { NFD(shift) })>
B<is> equivalent to C<(normalization =E<gt> 'NFD')>.
If source strings are finely prenormalized,
C<(normalization =E<gt> 'prenormalized')> may save time for normalization.

Except C<(normalization =E<gt> undef)>,
B<Unicode::Normalize> is required (see also B<CAVEAT>).

=item overrideCJK

-- see 7.1 Derived Collation Elements, UTS #10.

By default, CJK unified ideographs are ordered in Unicode codepoint
order, but those in the CJK Unified Ideographs block are less than
those in the CJK Unified Ideographs Extension A etc.

    In the CJK Unified Ideographs block:
    U+4E00..U+9FA5 if UCA_Version is 8, 9 or 11.
    U+4E00..U+9FBB if UCA_Version is 14 or 16.
    U+4E00..U+9FC3 if UCA_Version is 18.
    U+4E00..U+9FCB if UCA_Version is 20 or 22.
    U+4E00..U+9FCC if UCA_Version is 24 to 30.
    U+4E00..U+9FD5 if UCA_Version is 32 or 34.
    U+4E00..U+9FEA if UCA_Version is 36.
    U+4E00..U+9FEF if UCA_Version is 38, 40 or 41.
    U+4E00..U+9FFC if UCA_Version is 43.

    In the CJK Unified Ideographs Extension blocks:
    Ext.A (U+3400..U+4DB5)   if UCA_Version is  8 to 41.
    Ext.A (U+3400..U+4DBF)   if UCA_Version is 43.
    Ext.B (U+20000..U+2A6D6) if UCA_Version is  8 to 41.
    Ext.B (U+20000..U+2A6DD) if UCA_Version is 43.
    Ext.C (U+2A700..U+2B734) if UCA_Version is 20 or later.
    Ext.D (U+2B740..U+2B81D) if UCA_Version is 22 or later.
    Ext.E (U+2B820..U+2CEA1) if UCA_Version is 32 or later.
    Ext.F (U+2CEB0..U+2EBE0) if UCA_Version is 36 or later.
    Ext.G (U+30000..U+3134A) if UCA_Version is 43.

Through C<overrideCJK>, ordering of CJK unified ideographs (including
extensions) can be overridden.

ex. CJK unified ideographs in the JIS code point order.

  overrideCJK => sub {
      my $u = shift;             # get a Unicode codepoint
      my $b = pack('n', $u);     # to UTF-16BE
      my $s = your_unicode_to_sjis_converter($b); # convert
      my $n = unpack('n', $s);   # convert sjis to short
      [ $n, 0x20, 0x2, $u ];     # return the collation element
  },

The return value may be an arrayref of 1st to 4th weights as shown
above. The return value may be an integer as the primary weight
as shown below.  If C<undef> is returned, the default derived
collation element will be used.

  overrideCJK => sub {
      my $u = shift;             # get a Unicode codepoint
      my $b = pack('n', $u);     # to UTF-16BE
      my $s = your_unicode_to_sjis_converter($b); # convert
      my $n = unpack('n', $s);   # convert sjis to short
      return $n;                 # return the primary weight
  },

The return value may be a list containing zero or more of
an arrayref, an integer, or C<undef>.

ex. ignores all CJK unified ideographs.

  overrideCJK => sub {()}, # CODEREF returning empty list

   # where ->eq("Pe\x{4E00}rl", "Perl") is true
   # as U+4E00 is a CJK unified ideograph and to be ignorable.

If a false value (including C<undef>) is passed, C<overrideCJK>
has no effect.
C<$Collator-E<gt>change(overrideCJK =E<gt> 0)> resets the old one.

But assignment of weight for CJK unified ideographs
in C<table> or C<entry> is still valid.
If C<undef> is passed explicitly as the value for this key,
weights for CJK unified ideographs are treated as undefined.
However when C<UCA_Version> E<gt> 8, C<(overrideCJK =E<gt> undef)>
has no special meaning.

B<Note:> In addition to them, 12 CJK compatibility ideographs (C<U+FA0E>,
C<U+FA0F>, C<U+FA11>, C<U+FA13>, C<U+FA14>, C<U+FA1F>, C<U+FA21>, C<U+FA23>,
C<U+FA24>, C<U+FA27>, C<U+FA28>, C<U+FA29>) are also treated as CJK unified
ideographs. But they can't be overridden via C<overrideCJK> when you use
DUCET, as the table includes weights for them. C<table> or C<entry> has
priority over C<overrideCJK>.

=item overrideHangul

-- see 7.1 Derived Collation Elements, UTS #10.

By default, Hangul syllables are decomposed into Hangul Jamo,
even if C<(normalization =E<gt> undef)>.
But the mapping of Hangul syllables may be overridden.

This parameter works like C<overrideCJK>, so see there for examples.

If you want to override the mapping of Hangul syllables,
NFD and NFKD are not appropriate, since NFD and NFKD will decompose
Hangul syllables before overriding. FCD may decompose Hangul syllables
as the case may be.

If a false value (but not C<undef>) is passed, C<overrideHangul>
has no effect.
C<$Collator-E<gt>change(overrideHangul =E<gt> 0)> resets the old one.

If C<undef> is passed explicitly as the value for this key,
weight for Hangul syllables is treated as undefined
without decomposition into Hangul Jamo.
But definition of weight for Hangul syllables
in C<table> or C<entry> is still valid.

=item overrideOut

-- see 7.1.1 Handling Ill-Formed Code Unit Sequences, UTS #10.

Perl seems to allow out-of-range values (greater than 0x10FFFF).
By default, out-of-range values are replaced with C<U+FFFD>
(REPLACEMENT CHARACTER) when C<UCA_Version> E<gt>= 22,
or ignored when C<UCA_Version> E<lt>= 20.

When C<UCA_Version> E<gt>= 22, the weights of out-of-range values
can be overridden. Though C<table> or C<entry> are available for them,
out-of-range values are too many.

C<overrideOut> can perform it algorithmically.
This parameter works like C<overrideCJK>, so see there for examples.

ex. ignores all out-of-range values.

  overrideOut => sub {()}, # CODEREF returning empty list

If a false value (including C<undef>) is passed, C<overrideOut>
has no effect.
C<$Collator-E<gt>change(overrideOut =E<gt> 0)> resets the old one.

B<NOTE ABOUT U+FFFD:>

UCA recommends that out-of-range values should not be ignored for security
reasons. Say, C<"pe\x{110000}rl"> should not be equal to C<"perl">.
However, C<U+FFFD> is wrongly mapped to a variable collation element
in DUCET for Unicode 6.0.0 to 6.2.0, that means out-of-range values will be
ignored when C<variable> isn't C<Non-ignorable>.

The mapping of C<U+FFFD> is corrected in Unicode 6.3.0.
see L<http://www.unicode.org/reports/tr10/tr10-28.html#Trailing_Weights>
(7.1.4 Trailing Weights). Such a correction is reproduced by this.

  overrideOut => sub { 0xFFFD }, # CODEREF returning a very large integer

This workaround is unnecessary since Unicode 6.3.0.

=item preprocess

-- see 5.4 Preprocessing, UTS #10.

If specified, the coderef is used to preprocess each string
before the formation of sort keys.

ex. dropping English articles, such as "a" or "the".
Then, "the pen" is before "a pencil".

     preprocess => sub {
           my $str = shift;
           $str =~ s/\b(?:an?|the)\s+//gi;
           return $str;
        },

C<preprocess> is performed before C<normalization> (if defined).

ex. decoding strings in a legacy encoding such as shift-jis:

    $sjis_collator = Unicode::Collate->new(
        preprocess => \&your_shiftjis_to_unicode_decoder,
    );
    @result = $sjis_collator->sort(@shiftjis_strings);

B<Note:> Strings returned from the coderef will be interpreted
according to Perl's Unicode support. See L<perlunicode>,
L<perluniintro>, L<perlunitut>, L<perlunifaq>, L<utf8>.

=item rearrange

-- see 3.5 Rearrangement, UTS #10.

Characters that are not coded in logical order and to be rearranged.
If C<UCA_Version> is equal to or less than 11, default is:

    rearrange => [ 0x0E40..0x0E44, 0x0EC0..0x0EC4 ],

If you want to disallow any rearrangement, pass C<undef> or C<[]>
(a reference to empty list) as the value for this key.

If C<UCA_Version> is equal to or greater than 14, default is C<[]>
(i.e. no rearrangement).

B<According to the version 9 of UCA, this parameter shall not be used;
but it is not warned at present.>

=item rewrite

If specified, the coderef is used to rewrite lines in C<table> or C<entry>.
The coderef will get each line, and then should return a rewritten line
according to the UCA file format.
If the coderef returns an empty line, the line will be skipped.

e.g. any primary ignorable characters into tertiary ignorable:

    rewrite => sub {
        my $line = shift;
        $line =~ s/\[\.0000\..{4}\..{4}\./[.0000.0000.0000./g;
        return $line;
    },

This example shows rewriting weights. C<rewrite> is allowed to
affect code points, weights, and the name.

B<NOTE>: C<table> is available to use another table file;
preparing a modified table once would be more efficient than
rewriting lines on reading an unmodified table every time.

=item suppress

-- see 3.12 Special-Purpose Commands, UTS #35 (LDML) Part 5: Collation.

Contractions beginning with the specified characters are suppressed,
even if those contractions are defined in C<table>.

An example for Russian and some languages using the Cyrillic script:

    suppress => [0x0400..0x0417, 0x041A..0x0437, 0x043A..0x045F],

where 0x0400 stands for C<U+0400>, CYRILLIC CAPITAL LETTER IE WITH GRAVE.

B<NOTE>: Contractions via C<entry> will not be suppressed.

=item table

-- see 3.8 Default Unicode Collation Element Table, UTS #10.

You can use another collation element table if desired.

The table file should locate in the F<Unicode/Collate> directory
on C<@INC>. Say, if the filename is F<Foo.txt>,
the table file is searched as F<Unicode/Collate/Foo.txt> in C<@INC>.

By default, F<allkeys.txt> (as the filename of DUCET) is used.
If you will prepare your own table file, any name other than F<allkeys.txt>
may be better to avoid namespace conflict.

B<NOTE>: When XSUB is used, the DUCET is compiled on building this
module, and it may save time at the run time.
Explicit saying C<(table =E<gt> 'allkeys.txt')>, or using another table,
or using C<ignoreChar>, C<ignoreName>, C<undefChar>, C<undefName> or
C<rewrite> will prevent this module from using the compiled DUCET.

If C<undef> is passed explicitly as the value for this key,
no file is read (but you can define collation elements via C<entry>).

A typical way to define a collation element table
without any file of table:

   $onlyABC = Unicode::Collate->new(
       table => undef,
       entry => << 'ENTRIES',
0061 ; [.0101.0020.0002.0061] # LATIN SMALL LETTER A
0041 ; [.0101.0020.0008.0041] # LATIN CAPITAL LETTER A
0062 ; [.0102.0020.0002.0062] # LATIN SMALL LETTER B
0042 ; [.0102.0020.0008.0042] # LATIN CAPITAL LETTER B
0063 ; [.0103.0020.0002.0063] # LATIN SMALL LETTER C
0043 ; [.0103.0020.0008.0043] # LATIN CAPITAL LETTER C
ENTRIES
    );

If C<ignoreName> or C<undefName> is used, character names should be
specified as a comment (following C<#>) on each line.

=item undefChar

=item undefName

-- see 6.3.3 Reducing the Repertoire, UTS #10.

Undefines the collation element as if it were unassigned in the C<table>.
This reduces the size of the table.
If an unassigned character appears in the string to be collated,
the sort key is made from its codepoint
as a single-character collation element,
as it is greater than any other assigned collation elements
(in the codepoint order among the unassigned characters).
But, it'd be better to ignore characters
unfamiliar to you and maybe never used.

Through C<undefChar>, any character matching C<qr/$undefChar/>
will be undefined. Through C<undefName>, any character whose name
(given in the C<table> file as a comment) matches C<qr/$undefName/>
will be undefined.

ex. Collation weights for beyond-BMP characters are not stored in object:

    undefChar => qr/[^\0-\x{fffd}]/,

=item upper_before_lower

-- see 6.6 Case Comparisons, UTS #10.

By default, lowercase is before uppercase.
If the parameter is made true, this is reversed.

B<NOTE>: This parameter simplemindedly assumes that any lowercase/uppercase
distinctions must occur in level 3, and their weights at level 3 must be
same as those mentioned in 7.3.1, UTS #10.
If you define your collation elements which differs from this requirement,
this parameter doesn't work validly.

=item variable

-- see 3.6 Variable Weighting, UTS #10.

This key allows for variable weighting of variable collation elements,
which are marked with an ASTERISK in the table
(NOTE: Many punctuation marks and symbols are variable in F<allkeys.txt>).

   variable => 'blanked', 'non-ignorable', 'shifted', or 'shift-trimmed'.

These names are case-insensitive.
By default (if specification is omitted), 'shifted' is adopted.

   'Blanked'        Variable elements are made ignorable at levels 1 through 3;
                    considered at the 4th level.

   'Non-Ignorable'  Variable elements are not reset to ignorable.

   'Shifted'        Variable elements are made ignorable at levels 1 through 3
                    their level 4 weight is replaced by the old level 1 weight.
                    Level 4 weight for Non-Variable elements is 0xFFFF.

   'Shift-Trimmed'  Same as 'shifted', but all FFFF's at the 4th level
                    are trimmed.

=back

=head2 Methods for Collation

=over 4

=item C<@sorted = $Collator-E<gt>sort(@not_sorted)>

Sorts a list of strings.

=item C<$result = $Collator-E<gt>cmp($a, $b)>

Returns 1 (when C<$a> is greater than C<$b>)
or 0 (when C<$a> is equal to C<$b>)
or -1 (when C<$a> is less than C<$b>).

=item C<$result = $Collator-E<gt>eq($a, $b)>

=item C<$result = $Collator-E<gt>ne($a, $b)>

=item C<$result = $Collator-E<gt>lt($a, $b)>

=item C<$result = $Collator-E<gt>le($a, $b)>

=item C<$result = $Collator-E<gt>gt($a, $b)>

=item C<$result = $Collator-E<gt>ge($a, $b)>

They works like the same name operators as theirs.

   eq : whether $a is equal to $b.
   ne : whether $a is not equal to $b.
   lt : whether $a is less than $b.
   le : whether $a is less than $b or equal to $b.
   gt : whether $a is greater than $b.
   ge : whether $a is greater than $b or equal to $b.

=item C<$sortKey = $Collator-E<gt>getSortKey($string)>

-- see 4.3 Form Sort Key, UTS #10.

Returns a sort key.

You compare the sort keys using a binary comparison
and get the result of the comparison of the strings using UCA.

   $Collator->getSortKey($a) cmp $Collator->getSortKey($b)

      is equivalent to

   $Collator->cmp($a, $b)

=item C<$sortKeyForm = $Collator-E<gt>viewSortKey($string)>

Converts a sorting key into its representation form.
If C<UCA_Version> is 8, the output is slightly different.

   use Unicode::Collate;
   my $c = Unicode::Collate->new();
   print $c->viewSortKey("Perl"),"\n";

   # output:
   # [0B67 0A65 0B7F 0B03 | 0020 0020 0020 0020 | 0008 0002 0002 0002 | FFFF FFFF FFFF FFFF]
   #  Level 1               Level 2               Level 3               Level 4

=back

=head2 Methods for Searching

The C<match>, C<gmatch>, C<subst>, C<gsubst> methods work
like C<m//>, C<m//g>, C<s///>, C<s///g>, respectively,
but they are not aware of any pattern, but only a literal substring.

B<DISCLAIMER:> If C<preprocess> or C<normalization> parameter is true
for C<$Collator>, calling these methods (C<index>, C<match>, C<gmatch>,
C<subst>, C<gsubst>) is croaked, as the position and the length might
differ from those on the specified string.

C<rearrange> and C<hangul_terminator> parameters are neglected.
C<katakana_before_hiragana> and C<upper_before_lower> don't affect
matching and searching, as it doesn't matter whether greater or less.

=over 4

=item C<$position = $Collator-E<gt>index($string, $substring[, $position])>

=item C<($position, $length) = $Collator-E<gt>index($string, $substring[, $position])>

If C<$substring> matches a part of C<$string>, returns
the position of the first occurrence of the matching part in scalar context;
in list context, returns a two-element list of
the position and the length of the matching part.

If C<$substring> does not match any part of C<$string>,
returns C<-1> in scalar context and
an empty list in list context.

e.g. when the content of C<$str> is C<"Ich mu>E<szlig>C< studieren Perl.">,
you say the following where C<$sub> is C<"M>E<uuml>C<SS">,

  my $Collator = Unicode::Collate->new( normalization => undef, level => 1 );
                                     # (normalization => undef) is REQUIRED.
  my $match;
  if (my($pos,$len) = $Collator->index($str, $sub)) {
      $match = substr($str, $pos, $len);
  }

and get C<"mu>E<szlig>C<"> in C<$match>, since C<"mu>E<szlig>C<">
is primary equal to C<"M>E<uuml>C<SS">.

=item C<$match_ref = $Collator-E<gt>match($string, $substring)>

=item C<($match)   = $Collator-E<gt>match($string, $substring)>

If C<$substring> matches a part of C<$string>, in scalar context, returns
B<a reference to> the first occurrence of the matching part
(C<$match_ref> is always true if matches,
since every reference is B<true>);
in list context, returns the first occurrence of the matching part.

If C<$substring> does not match any part of C<$string>,
returns C<undef> in scalar context and
an empty list in list context.

e.g.

    if ($match_ref = $Collator->match($str, $sub)) { # scalar context
	print "matches [$$match_ref].\n";
    } else {
	print "doesn't match.\n";
    }

     or

    if (($match) = $Collator->match($str, $sub)) { # list context
	print "matches [$match].\n";
    } else {
	print "doesn't match.\n";
    }

=item C<@match = $Collator-E<gt>gmatch($string, $substring)>

If C<$substring> matches a part of C<$string>, returns
all the matching parts (or matching count in scalar context).

If C<$substring> does not match any part of C<$string>,
returns an empty list.

=item C<$count = $Collator-E<gt>subst($string, $substring, $replacement)>

If C<$substring> matches a part of C<$string>,
the first occurrence of the matching part is replaced by C<$replacement>
(C<$string> is modified) and C<$count> (always equals to C<1>) is returned.

C<$replacement> can be a C<CODEREF>,
taking the matching part as an argument,
and returning a string to replace the matching part
(a bit similar to C<s/(..)/$coderef-E<gt>($1)/e>).

=item C<$count = $Collator-E<gt>gsubst($string, $substring, $replacement)>

If C<$substring> matches a part of C<$string>,
all the occurrences of the matching part are replaced by C<$replacement>
(C<$string> is modified) and C<$count> is returned.

C<$replacement> can be a C<CODEREF>,
taking the matching part as an argument,
and returning a string to replace the matching part
(a bit similar to C<s/(..)/$coderef-E<gt>($1)/eg>).

e.g.

  my $Collator = Unicode::Collate->new( normalization => undef, level => 1 );
                                     # (normalization => undef) is REQUIRED.
  my $str = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l...";
  $Collator->gsubst($str, "camel", sub { "<b>$_[0]</b>" });

  # now $str is "<b>Camel</b> donkey zebra <b>came\x{301}l</b> <b>CAMEL</b> horse <b>cam\0e\0l</b>...";
  # i.e., all the camels are made bold-faced.

   Examples: levels and ignore_level2 - what does camel match?
  ---------------------------------------------------------------------------
   level  ignore_level2  |  camel  Camel  came\x{301}l  c-a-m-e-l  cam\0e\0l
  -----------------------|---------------------------------------------------
     1        false      |   yes    yes      yes          yes        yes
     2        false      |   yes    yes      no           yes        yes
     3        false      |   yes    no       no           yes        yes
     4        false      |   yes    no       no           no         yes
  -----------------------|---------------------------------------------------
     1        true       |   yes    yes      yes          yes        yes
     2        true       |   yes    yes      yes          yes        yes
     3        true       |   yes    no       yes          yes        yes
     4        true       |   yes    no       yes          no         yes
  ---------------------------------------------------------------------------
   note: if variable => non-ignorable, camel doesn't match c-a-m-e-l
         at any level.

=back

=head2 Other Methods

=over 4

=item C<%old_tailoring = $Collator-E<gt>change(%new_tailoring)>

=item C<$modified_collator = $Collator-E<gt>change(%new_tailoring)>

Changes the value of specified keys and returns the changed part.

    $Collator = Unicode::Collate->new(level => 4);

    $Collator->eq("perl", "PERL"); # false

    %old = $Collator->change(level => 2); # returns (level => 4).

    $Collator->eq("perl", "PERL"); # true

    $Collator->change(%old); # returns (level => 2).

    $Collator->eq("perl", "PERL"); # false

Not all C<(key,value)>s are allowed to be changed.
See also C<@Unicode::Collate::ChangeOK> and C<@Unicode::Collate::ChangeNG>.

In the scalar context, returns the modified collator
(but it is B<not> a clone from the original).

    $Collator->change(level => 2)->eq("perl", "PERL"); # true

    $Collator->eq("perl", "PERL"); # true; now max level is 2nd.

    $Collator->change(level => 4)->eq("perl", "PERL"); # false

=item C<$version = $Collator-E<gt>version()>

Returns the version number (a string) of the Unicode Standard
which the C<table> file used by the collator object is based on.
If the table does not include a version line (starting with C<@version>),
returns C<"unknown">.

=item C<UCA_Version()>

Returns the revision number of UTS #10 this module consults,
that should correspond with the DUCET incorporated.

=item C<Base_Unicode_Version()>

Returns the version number of UTS #10 this module consults,
that should correspond with the DUCET incorporated.

=back

=head1 EXPORT

No method will be exported.

=head1 INSTALL

Though this module can be used without any C<table> file,
to use this module easily, it is recommended to install a table file
in the UCA format, by copying it under the directory
<a place in @INC>/Unicode/Collate.

The most preferable one is "The Default Unicode Collation Element Table"
(aka DUCET), available from the Unicode Consortium's website:

   http://www.unicode.org/Public/UCA/

   http://www.unicode.org/Public/UCA/latest/allkeys.txt
   (latest version)

If DUCET is not installed, it is recommended to copy the file
from http://www.unicode.org/Public/UCA/latest/allkeys.txt
to <a place in @INC>/Unicode/Collate/allkeys.txt
manually.

=head1 CAVEATS

=over 4

=item Normalization

Use of the C<normalization> parameter requires the B<Unicode::Normalize>
module (see L<Unicode::Normalize>).

If you need not it (say, in the case when you need not
handle any combining characters),
assign C<(normalization =E<gt> undef)> explicitly.

-- see 6.5 Avoiding Normalization, UTS #10.

=item Conformance Test

The Conformance Test for the UCA is available
under L<http://www.unicode.org/Public/UCA/>.

For F<CollationTest_SHIFTED.txt>,
a collator via C<Unicode::Collate-E<gt>new( )> should be used;
for F<CollationTest_NON_IGNORABLE.txt>, a collator via
C<Unicode::Collate-E<gt>new(variable =E<gt> "non-ignorable", level =E<gt> 3)>.

If C<UCA_Version> is 26 or later, the C<identical> level is preferred;
C<Unicode::Collate-E<gt>new(identical =E<gt> 1)> and
C<Unicode::Collate-E<gt>new(identical =E<gt> 1,>
C<variable =E<gt> "non-ignorable", level =E<gt> 3)> should be used.

B<Unicode::Normalize is required to try The Conformance Test.>

B<EBCDIC-SUPPORT IS EXPERIMENTAL.>

=back

=head1 AUTHOR, COPYRIGHT AND LICENSE

The Unicode::Collate module for perl was written by SADAHIRO Tomoyuki,
<SADAHIRO@cpan.org>. This module is Copyright(C) 2001-2021,
SADAHIRO Tomoyuki. Japan. All rights reserved.

This module is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

The file Unicode/Collate/allkeys.txt was copied verbatim
from L<http://www.unicode.org/Public/UCA/13.0.0/allkeys.txt>.
For this file, Copyright (c) 2020 Unicode, Inc.; distributed
under the Terms of Use in L<http://www.unicode.org/terms_of_use.html>

=head1 SEE ALSO

=over 4

=item Unicode Collation Algorithm - UTS #10

L<http://www.unicode.org/reports/tr10/>

=item The Default Unicode Collation Element Table (DUCET)

L<http://www.unicode.org/Public/UCA/latest/allkeys.txt>

=item The conformance test for the UCA

L<http://www.unicode.org/Public/UCA/latest/CollationTest.html>

L<http://www.unicode.org/Public/UCA/latest/CollationTest.zip>

=item Hangul Syllable Type

L<http://www.unicode.org/Public/UNIDATA/HangulSyllableType.txt>

=item Unicode Normalization Forms - UAX #15

L<http://www.unicode.org/reports/tr15/>

=item Unicode Locale Data Markup Language (LDML) - UTS #35

L<http://www.unicode.org/reports/tr35/>

=back

=cut
