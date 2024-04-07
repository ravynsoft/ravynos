#!./perl -T

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bDevel\/Peek\b/) {
        print "1..0 # Skip: Devel::Peek was not built\n";
        exit 0;
    }
    {
    package t;
        my $core = !!$ENV{PERL_CORE};
        if ($core) {
            require '../../t/test.pl';
            require '../../t/charset_tools.pl';
        }
        else {
            require './t/test.pl';
            require './t/charset_tools.pl';
        }
    }
}

use Test::More;

BEGIN {
    use_ok 'Devel::Peek';
}
require Tie::Hash;

our $DEBUG = 0;
open(SAVERR, ">&STDERR") or die "Can't dup STDERR: $!";

# If I reference any lexicals in this, I get the entire outer subroutine (or
# MAIN) dumped too, which isn't really what I want, as it's a lot of faff to
# maintain that.
format PIE =
Pie     @<<<<<
$::type
Good    @>>>>>
$::mmmm
.

use constant thr => $Config{useithreads};

sub do_test {
    my $todo = $_[3];
    my $repeat_todo = $_[4];
    my $pattern = $_[2];
    my $do_eval = $_[5];
    if (open(OUT,'>', "peek$$")) {
        my $setup_stderr = sub { open(STDERR, ">&OUT") or die "Can't dup OUT: $!" };
        if ($do_eval) {
            my $sub = eval "sub { Dump $_[1] }";
            die $@ if $@;
            $setup_stderr->();
            $sub->();
            print STDERR "*****\n";
            # second dump to compare with the first to make sure nothing
            # changed.
            $sub->();
        }
        else {
            $setup_stderr->();
            Dump($_[1]);
            print STDERR "*****\n";
            # second dump to compare with the first to make sure nothing
            # changed.
            Dump($_[1]);
        }
	open(STDERR, ">&SAVERR") or die "Can't restore STDERR: $!";
	close(OUT);
	if (open(IN, '<', "peek$$")) {
	    local $/;
	    $pattern =~ s/\$ADDR/0x[[:xdigit:]]+/g;
	    $pattern =~ s/\$FLOAT/(?:\\d*\\.\\d+(?:e[-+]\\d+)?|\\d+)/g;
	    # handle DEBUG_LEAKING_SCALARS prefix
	    $pattern =~ s/^(\s*)(SV =.* at )/(?:$1ALLOCATED at .*?\n)?$1$2/mg;

	    # Need some clear generic mechanism to eliminate (or add) lines
	    # of dump output dependant on perl version. The (previous) use of
	    # things like $IVNV gave the illusion that the string passed in was
	    # a regexp into which variables were interpolated, but this wasn't
	    # actually true as those 'variables' actually also ate the
	    # whitespace on the line. So it seems better to mark lines that
	    # need to be eliminated. I considered (?# ... ) and (?{ ... }),
	    # but whilst embedded code or comment syntax would keep it as a
	    # legitimate regexp, it still isn't true. Seems easier and clearer
	    # things that look like comments.

	    # Could do this is in a s///mge but seems clearer like this:
	    $pattern = join '', map {
		# If we identify the version condition, take *it* out whatever
		s/\s*# (\$\].*)$//
		    ? (eval $1 ? $_ : '')
		    : $_ # Didn't match, so this line is in
	    } split /^/, $pattern;
	    
	    $pattern =~ s/\$PADMY,/
		$] < 5.012005 ? 'PADMY,' : '';
	    /mge;
	    $pattern =~ s/\$RV/
		($] < 5.011) ? 'RV' : 'IV';
	    /mge;
	    $pattern =~ s/^\h+COW_REFCNT = .*\n//mg
		if $Config{ccflags} =~
			/-DPERL_(?:OLD_COPY_ON_WRITE|NO_COW)\b/
			    || $] < 5.019003;
            if ($Config::Config{ccflags} =~ /-DNODEFAULT_SHAREKEYS\b/) {
                $pattern =~ s/,SHAREKEYS\b//g;
                $pattern =~ s/\bSHAREKEYS,//g;
                $pattern =~ s/\bSHAREKEYS\b//g;
            }
	    print $pattern, "\n" if $DEBUG;
	    my ($dump, $dump2) = split m/\*\*\*\*\*\n/, scalar <IN>;
	    print $dump, "\n"    if $DEBUG;
	    like( $dump, qr/\A$pattern\Z/ms, $_[0])
	      or note("line " . (caller)[2]);

            local $TODO = $repeat_todo;
            is($dump2, $dump, "$_[0] (unchanged by dump)")
	      or note("line " . (caller)[2]);

	    close(IN);

            return $1;
	} else {
	    die "$0: failed to open peek$$: !\n";
	}
    } else {
	die "$0: failed to create peek$$: $!\n";
    }
}

our   $a;
our   $b;
my    $c;
local $d = 0;

END {
    1 while unlink("peek$$");
}

do_test('assignment of immediate constant (string)',
	$a = "foo",
'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(POK,(?:IsCOW,)?pPOK\\)
  PV = $ADDR "foo"\\\0
  CUR = 3
  LEN = \\d+
  COW_REFCNT = 1
');

do_test('immediate constant (string)',
        "bar",
'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(.*POK,READONLY,(?:IsCOW,)?pPOK\\)		# $] < 5.021005
  FLAGS = \\(.*POK,(?:IsCOW,)?READONLY,PROTECT,pPOK\\)	# $] >=5.021005
  PV = $ADDR "bar"\\\0
  CUR = 3
  LEN = \\d+
  COW_REFCNT = 0
');

do_test('assignment of immediate constant (integer)',
        $b = 123,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(IOK,pIOK\\)
  IV = 123');

do_test('immediate constant (integer)',
        456,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(.*IOK,READONLY,pIOK\\)		# $] < 5.021005
  FLAGS = \\(.*IOK,READONLY,PROTECT,pIOK\\)	# $] >=5.021005
  IV = 456');

do_test('assignment of immediate constant (integer)',
        $c = 456,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\($PADMY,IOK,pIOK\\)
  IV = 456');

# If perl is built with PERL_PRESERVE_IVUV then maths is done as integers
# where possible and this scalar will be an IV. If NO_PERL_PRESERVE_IVUV then
# maths is done in floating point always, and this scalar will be an NV.
# ([NI]) captures the type, referred to by \1 in this regexp and $type for
# building subsequent regexps.
my $type = do_test('result of addition',
        $c + $d,
'SV = ([NI])V\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(PADTMP,\1OK,p\1OK\\)		# $] < 5.019003
  FLAGS = \\(\1OK,p\1OK\\)			# $] >=5.019003
  \1V = 456');

($d = "789") += 0.1;

do_test('floating point value',
       $d,
       $] < 5.019003
        || $Config{ccflags} =~ /-DPERL_(?:NO_COW|OLD_COPY_ON_WRITE)\b/
       ?
'SV = PVNV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(NOK,pNOK\\)
  IV = \d+
  NV = 789\\.(?:1(?:000+\d+)?|0999+\d+)
  PV = $ADDR "789"\\\0
  CUR = 3
  LEN = \\d+'
       :
'SV = PVNV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(NOK,pNOK\\)
  IV = \d+
  NV = 789\\.(?:1(?:000+\d+)?|0999+\d+)
  PV = 0');

do_test('integer constant',
        0xabcd,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(.*IOK,READONLY,pIOK\\)		# $] < 5.021005
  FLAGS = \\(.*IOK,READONLY,PROTECT,pIOK\\)	# $] >=5.021005
  IV = 43981');

do_test('undef',
        undef,
'SV = NULL\\(0x0\\) at $ADDR
  REFCNT = \d+
  FLAGS = \\(READONLY\\)			# $] < 5.021005
  FLAGS = \\(READONLY,PROTECT\\)		# $] >=5.021005
');

do_test('reference to scalar',
        \$a,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\(POK,(?:IsCOW,)?pPOK\\)
    PV = $ADDR "foo"\\\0
    CUR = 3
    LEN = \\d+
    COW_REFCNT = 1
');

do_test('immediate boolean',
        !!0,
'SV = PVNV\\($ADDR\\) at $ADDR
  REFCNT = \d+
  FLAGS = \\(.*\\)
  IV = 0
  NV = 0
  PV = $ADDR "" \[BOOL PL_No\]
  CUR = 0
  LEN = 0
') if $] >= 5.035004;

do_test('assignment of boolean',
        do { my $tmp = !!1 },
'SV = PVNV\\($ADDR\\) at $ADDR
  REFCNT = \d+
  FLAGS = \\(.*\\)
  IV = 1
  NV = 1
  PV = $ADDR "1" \[BOOL PL_Yes\]
  CUR = 1
  LEN = 0
') if $] >= 5.035004;

my $c_pattern;
if ($type eq 'N') {
  $c_pattern = '
    SV = PVNV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(IOK,NOK,pIOK,pNOK\\)
      IV = 456
      NV = 456
      PV = 0';
} else {
  $c_pattern = '
    SV = IV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(IOK,pIOK\\)
      IV = 456';
}
do_test('reference to array',
       [$b,$c],
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVAV\\($ADDR\\) at $ADDR
    REFCNT = 1
    FLAGS = \\(\\)
    ARRAY = $ADDR
    FILL = 1
    MAX = 1
    FLAGS = \\(REAL\\)
    Elt No. 0
    SV = IV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(IOK,pIOK\\)
      IV = 123
    Elt No. 1' . $c_pattern);

do_test('reference to hash',
       {$b=>$c},
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = [12]
    FLAGS = \\(SHAREKEYS\\)
    ARRAY = $ADDR  \\(0:7, 1:1\\)
    hash quality = 100.0%
    KEYS = 1
    FILL = 1
    MAX = 7
    Elt "123" HASH = $ADDR' . $c_pattern,
	'',
	($] < 5.015) ? 'The hash iterator used in dump.c sets the OOK flag' : undef);

do_test('reference to anon sub with empty prototype',
        sub(){@_},
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVCV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\($PADMY,POK,pPOK,ANON,WEAKOUTSIDE,CVGV_RC\\) # $] < 5.015 || !thr
    FLAGS = \\($PADMY,POK,pPOK,ANON,WEAKOUTSIDE,CVGV_RC,DYNFILE\\) # $] >= 5.015 && thr
    PROTOTYPE = ""
    COMP_STASH = $ADDR\\t"main"
    START = $ADDR ===> \\d+
    ROOT = $ADDR
    GVGV::GV = $ADDR\\t"main" :: "__ANON__[^"]*"
    FILE = ".*\\b(?i:peek\\.t)"
    DEPTH = 0(?:
    MUTEXP = $ADDR
    OWNER = $ADDR)?
    FLAGS = 0x490				# $] < 5.015 || !thr
    FLAGS = 0x1490				# $] >= 5.015 && thr
    OUTSIDE_SEQ = \\d+
    PADLIST = $ADDR
    PADNAME = $ADDR\\($ADDR\\) PAD = $ADDR\\($ADDR\\)
    OUTSIDE = $ADDR \\(MAIN\\)');

do_test('reference to named subroutine without prototype',
        \&do_test,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVCV\\($ADDR\\) at $ADDR
    REFCNT = (3|4)
    FLAGS = \\((?:HASEVAL(?:,NAMED)?)?\\)	# $] < 5.015 || !thr
    FLAGS = \\(DYNFILE(?:,HASEVAL(?:,NAMED)?)?\\) # $] >= 5.015 && thr
    COMP_STASH = $ADDR\\t"main"
    START = $ADDR ===> \\d+
    ROOT = $ADDR
    NAME = "do_test"				# $] >=5.021004
    GVGV::GV = $ADDR\\t"main" :: "do_test"	# $] < 5.021004
    FILE = ".*\\b(?i:peek\\.t)"
    DEPTH = 1(?:
    MUTEXP = $ADDR
    OWNER = $ADDR)?
    FLAGS = 0x(?:[c4]00)?0			# $] < 5.015 || !thr
    FLAGS = 0x[cd145]000			# $] >= 5.015 && thr
    OUTSIDE_SEQ = \\d+
    PADLIST = $ADDR
    PADNAME = $ADDR\\($ADDR\\) PAD = $ADDR\\($ADDR\\)
       \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$todo"
       \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$repeat_todo"
       \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$pattern"
       \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$do_eval"
\s+\\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$setup_stderr"
\s+\\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "&"
      \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$sub"
      \\d+\\. $ADDR<\\d+> FAKE "\\$DEBUG" flags=0x0 index=0
      \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$dump"
      \\d+\\. $ADDR<\\d+> \\(\\d+,\\d+\\) "\\$dump2"
    OUTSIDE = $ADDR \\(MAIN\\)');

# note the conditionals on ENGINE and INTFLAGS were introduced in 5.19.9
do_test('reference to regexp',
        qr(tic),
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = REGEXP\\($ADDR\\) at $ADDR
    REFCNT = 1
    FLAGS = \\(OBJECT,POK,FAKE,pPOK\\)
    PV = $ADDR "\\(\\?\\^:tic\\)"
    CUR = 8
    LEN = 0
    STASH = $ADDR\\s+"Regexp"
    COMPFLAGS = 0x0 \\(\\)
    EXTFLAGS = 0x680000 \\(CHECK_ALL,USE_INTUIT_NOML,USE_INTUIT_ML\\)
    ENGINE = $ADDR \\(STANDARD\\)
    INTFLAGS = 0x0 \\(\\)
    NPARENS = 0
    LOGICAL_NPARENS = 0
    LOGICAL_TO_PARNO = 0x0
    PARNO_TO_LOGICAL = 0x0
    PARNO_TO_LOGICAL_NEXT = 0x0
    LASTPAREN = 0
    LASTCLOSEPAREN = 0
    MINLEN = 3
    MINLENRET = 3
    GOFS = 0
    PRE_PREFIX = 4
    SUBLEN = 0
    SUBOFFSET = 0
    SUBCOFFSET = 0
    SUBBEG = 0x0
    PAREN_NAMES = 0x0
    SUBSTRS = $ADDR
    PPRIVATE = $ADDR
    OFFS = $ADDR
      \\[ 0:0 \\]
    QR_ANONCV = 0x0
    SAVED_COPY = 0x0
    MOTHER_RE = $ADDR
    SV = REGEXP\\($ADDR\\) at $ADDR
      REFCNT = 2
      FLAGS = \\(POK,pPOK\\)
      PV = $ADDR "\\(\\?\\^:tic\\)"
      CUR = 8
      LEN = \\d+
      COMPFLAGS = 0x0 \\(\\)
      EXTFLAGS = 0x680000 \\(CHECK_ALL,USE_INTUIT_NOML,USE_INTUIT_ML\\)
      ENGINE = $ADDR \\(STANDARD\\)
      INTFLAGS = 0x0 \\(\\)
      NPARENS = 0
      LOGICAL_NPARENS = 0
      LOGICAL_TO_PARNO = 0x0
      PARNO_TO_LOGICAL = 0x0
      PARNO_TO_LOGICAL_NEXT = 0x0
      LASTPAREN = 0
      LASTCLOSEPAREN = 0
      MINLEN = 3
      MINLENRET = 3
      GOFS = 0
      PRE_PREFIX = 4
      SUBLEN = 0
      SUBOFFSET = 0
      SUBCOFFSET = 0
      SUBBEG = 0x0
      PAREN_NAMES = 0x0
      SUBSTRS = $ADDR
      PPRIVATE = $ADDR
      OFFS = $ADDR
        \\[ 0:0 \\]
      QR_ANONCV = 0x0
      SAVED_COPY = 0x0
      MOTHER_RE = 0x0
');

do_test('reference to blessed hash',
        (bless {}, "Tac"),
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = [12]
    FLAGS = \\(OBJECT,SHAREKEYS\\)
    STASH = $ADDR\\t"Tac"
    ARRAY = 0x0
    KEYS = 0
    FILL = 0
    MAX = 7', '',
	$] >= 5.015
	     ? undef
	     : 'The hash iterator used in dump.c sets the OOK flag');

do_test('typeglob',
	*a,
'SV = PVGV\\($ADDR\\) at $ADDR
  REFCNT = 5
  FLAGS = \\(MULTI(?:,IN_PAD)?\\)
  NAME = "a"
  NAMELEN = 1
  GvSTASH = $ADDR\\t"main"
  FLAGS = $ADDR					# $] >=5.021004
  GP = $ADDR
    SV = $ADDR
    REFCNT = 1
    IO = 0x0
    FORM = 0x0  
    AV = 0x0
    HV = 0x0
    CV = 0x0
    CVGEN = 0x0
    GPFLAGS = 0x0 \(\)				# $] >= 5.021004
    LINE = \\d+
    FILE = ".*\\b(?i:peek\\.t)"
    FLAGS = $ADDR				# $] < 5.021004
    EGV = $ADDR\\t"a"');

# Get native character set representations for these code points
my $cp100_bytes = t::byte_utf8a_to_utf8n("\xC4\x80");
my $cp0_bytes =   t::byte_utf8a_to_utf8n("\x00");
my $cp200_bytes = t::byte_utf8a_to_utf8n("\xC8\x80");

# Convert to e.g., \\\\xC4
my $prefix = '\\\\x';
foreach my $ref (\$cp100_bytes, \$cp0_bytes, \$cp200_bytes) {
    my $revised = "";
    $$ref =~ s/(.)/sprintf("$prefix%02X", ord $1)/eg;
}

do_test('string with Unicode',
	chr(256).chr(0).chr(512),
'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\((?:PADTMP,)?POK,READONLY,pPOK,UTF8\\)	# $] < 5.019003
  FLAGS = \\((?:PADTMP,)?POK,(?:IsCOW,)?pPOK,UTF8\\)	# $] >=5.019003
  PV = $ADDR "' . $cp100_bytes
                . $cp0_bytes
                . $cp200_bytes
                . '"\\\0 \[UTF8 "\\\x\{100\}\\\x\{0\}\\\x\{200\}"\]
  CUR = 5
  LEN = \\d+
  COW_REFCNT = 1					# $] < 5.019007
');

do_test('reference to hash containing Unicode',
	{chr(256)=>chr(512)},
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = [12]
    FLAGS = \\(SHAREKEYS,HASKFLAGS\\)
    ARRAY = $ADDR  \\(0:7, 1:1\\)
    hash quality = 100.0%
    KEYS = 1
    FILL = 1
    MAX = 7
    Elt "' . $cp100_bytes . '" \[UTF8 "\\\x\{100\}"\] HASH = $ADDR
    SV = PV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(POK,(?:IsCOW,)?pPOK,UTF8\\)
      PV = $ADDR "' . $cp200_bytes . '"\\\0 \[UTF8 "\\\x\{200\}"\]
      CUR = 2
      LEN = \\d+
      COW_REFCNT = 1				# $] < 5.019007
',      '',
	$] >= 5.015
	    ? undef
	    : 'The hash iterator used in dump.c sets the OOK flag');

my $x="";
$x=~/.??/g;
do_test('scalar with pos magic',
        $x,
'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\($PADMY,SMG,POK,(?:IsCOW,)?pPOK\\)
  IV = \d+
  NV = 0
  PV = $ADDR ""\\\0
  CUR = 0
  LEN = \d+
  COW_REFCNT = [12]
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_mglob
    MG_TYPE = PERL_MAGIC_regex_global\\(g\\)
    MG_FLAGS = 0x01					# $] < 5.019003
    MG_FLAGS = 0x41					# $] >=5.019003
      MINMATCH
      BYTES						# $] >=5.019003
');

#
# TAINTEDDIR is not set on: OS2, AMIGAOS, WIN32
# environment variables may be invisibly case-forced, hence the (?i:PATH)
# C<scalar(@ARGV)> is turned into an IV on VMS hence the (?:IV)?
# Perl 5.18 ensures all env vars end up as strings only, hence the (?:,pIOK)?
# Perl 5.18 ensures even magic vars have public OK, hence the (?:,POK)?
# VMS is setting FAKE and READONLY flags.  What VMS uses for storing
# ENV hashes is also not always null terminated.
#
if (${^TAINT}) {
  # Save and restore PATH, since fresh_perl ends up using that in Windows.
  my $path = $ENV{PATH};
  do_test('tainted value in %ENV',
          $ENV{PATH}=@ARGV,  # scalar(@ARGV) is a handy known tainted value
'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(GMG,SMG,RMG(?:,POK)?(?:,pIOK)?,pPOK\\)
  IV = 0
  NV = 0
  PV = $ADDR "0"\\\0
  CUR = 1
  LEN = \d+
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_envelem
    MG_TYPE = PERL_MAGIC_envelem\\(e\\)
(?:    MG_FLAGS = 0x01
      TAINTEDDIR
)?    MG_LEN = -?\d+
    MG_PTR = $ADDR (?:"(?i:PATH)"|=> HEf_SVKEY
    SV = PV(?:IV)?\\($ADDR\\) at $ADDR
      REFCNT = \d+
      FLAGS = \\((?:TEMP,)?POK,(?:FAKE,READONLY,)?pPOK\\)
(?:      IV = 0
)?      PV = $ADDR "(?i:PATH)"(?:\\\0)?
      CUR = \d+
      LEN = \d+)
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_taint
    MG_TYPE = PERL_MAGIC_taint\\(t\\)');
    $ENV{PATH} = $path;
}

do_test('blessed reference',
	bless(\\undef, 'Foobar'),
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVMG\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\(OBJECT,ROK\\)
    IV = -?\d+
    NV = $FLOAT
    RV = $ADDR
    SV = NULL\\(0x0\\) at $ADDR
      REFCNT = \d+
      FLAGS = \\(READONLY\\)			# $] < 5.021005
      FLAGS = \\(READONLY,PROTECT\\)		# $] >=5.021005
    PV = $ADDR ""
    CUR = 0
    LEN = 0
    STASH = $ADDR\s+"Foobar"');

sub const () {
    "Perl rules";
}

do_test('constant subroutine',
	\&const,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVCV\\($ADDR\\) at $ADDR
    REFCNT = (2)
    FLAGS = \\(POK,pPOK,CONST,ISXSUB\\)		# $] < 5.015
    FLAGS = \\(POK,pPOK,CONST,DYNFILE,ISXSUB\\)	# $] >= 5.015
    PROTOTYPE = ""
    COMP_STASH = 0x0				# $] < 5.021004
    COMP_STASH = $ADDR	"main"			# $] >=5.021004
    XSUB = $ADDR
    XSUBANY = $ADDR \\(CONST SV\\)
    SV = PV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(.*POK,READONLY,(?:IsCOW,)?pPOK\\)	   # $] < 5.021005
      FLAGS = \\(.*POK,(?:IsCOW,)?READONLY,PROTECT,pPOK\\) # $] >=5.021005
      PV = $ADDR "Perl rules"\\\0
      CUR = 10
      LEN = \\d+
      COW_REFCNT = 0
    GVGV::GV = $ADDR\\t"main" :: "const"
    FILE = ".*\\b(?i:peek\\.t)"
    DEPTH = 0(?:
    MUTEXP = $ADDR
    OWNER = $ADDR)?
    FLAGS = 0xc00				# $] < 5.013
    FLAGS = 0xc					# $] >= 5.013 && $] < 5.015
    FLAGS = 0x100c				# $] >= 5.015
    OUTSIDE_SEQ = 0
    PADLIST = 0x0				# $] < 5.021006
    HSCXT = $ADDR				# $] >= 5.021006
    OUTSIDE = 0x0 \\(null\\)');	

do_test('isUV should show on PVMG',
	do { my $v = $1; $v = ~0; $v },
'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(IOK,pIOK,IsUV\\)
  UV = \d+
  NV = 0
  PV = 0');

do_test('IO',
	*STDOUT{IO},
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVIO\\($ADDR\\) at $ADDR
    REFCNT = 3
    FLAGS = \\(OBJECT\\)
    IV = 0					# $] < 5.011
    NV = 0					# $] < 5.011
    STASH = $ADDR\s+"IO::File"
    IFP = $ADDR
    OFP = $ADDR
    DIRP = 0x0
    LINES = 0
    PAGE = 0
    PAGE_LEN = 60
    LINES_LEFT = 0
    TOP_GV = 0x0
    FMT_GV = 0x0
    BOTTOM_GV = 0x0
    TYPE = \'>\'
    FLAGS = 0x4');

do_test('FORMAT',
	*PIE{FORMAT},
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVFM\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\(\\)				# $] < 5.015 || !thr
    FLAGS = \\(DYNFILE\\)			# $] >= 5.015 && thr
(?:    PV = 0
)?    COMP_STASH = 0x0
    START = $ADDR ===> \\d+
    ROOT = $ADDR
    GVGV::GV = $ADDR\\t"main" :: "PIE"
    FILE = ".*\\b(?i:peek\\.t)"(?:
    DEPTH = 0)?(?:
    MUTEXP = $ADDR
    OWNER = $ADDR)?
    FLAGS = 0x0					# $] < 5.015 || !thr
    FLAGS = 0x1000				# $] >= 5.015 && thr
    OUTSIDE_SEQ = \\d+
    LINES = 0					# $] < 5.017_003
    PADLIST = $ADDR
    PADNAME = $ADDR\\($ADDR\\) PAD = $ADDR\\($ADDR\\)
    OUTSIDE = $ADDR \\(MAIN\\)');

do_test('blessing to a class with embedded NUL characters',
        (bless {}, "\0::foo::\n::baz::\t::\0"),
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = [12]
    FLAGS = \\(OBJECT,SHAREKEYS\\)
    STASH = $ADDR\\t"\\\\0::foo::\\\\n::baz::\\\\t::\\\\0"
    ARRAY = $ADDR
    KEYS = 0
    FILL = 0
    MAX = 7', '',
	$] >= 5.015
	    ? undef
	    : 'The hash iterator used in dump.c sets the OOK flag');

do_test('ENAME on a stash',
        \%RWOM::,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\(OOK,SHAREKEYS\\)
    AUX_FLAGS = 0                               # $] > 5.019008
    ARRAY = $ADDR
    KEYS = 0
    FILL = 0
    MAX = 7
    RITER = -1
    EITER = 0x0
    RAND = $ADDR
    NAME = "RWOM"
    ENAME = "RWOM"				# $] > 5.012
');

*KLANK:: = \%RWOM::;

do_test('ENAMEs on a stash',
        \%RWOM::,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 3
    FLAGS = \\(OOK,SHAREKEYS\\)
    AUX_FLAGS = 0                               # $] > 5.019008
    ARRAY = $ADDR
    KEYS = 0
    FILL = 0
    MAX = 7
    RITER = -1
    EITER = 0x0
    RAND = $ADDR
    NAME = "RWOM"
    NAMECOUNT = 2				# $] > 5.012
    ENAME = "RWOM", "KLANK"			# $] > 5.012
');

undef %RWOM::;

do_test('ENAMEs on a stash with no NAME',
        \%RWOM::,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 3
    FLAGS = \\(OOK,SHAREKEYS\\)			# $] < 5.017
    FLAGS = \\(OOK,OVERLOAD,SHAREKEYS\\)	# $] >=5.017 && $]<5.021005
    FLAGS = \\(OOK,SHAREKEYS,OVERLOAD\\)	# $] >=5.021005
    AUX_FLAGS = 0                               # $] > 5.019008
    ARRAY = $ADDR
    KEYS = 0
    FILL = 0
    MAX = 7
    RITER = -1
    EITER = 0x0
    RAND = $ADDR
    NAMECOUNT = -3				# $] > 5.012
    ENAME = "RWOM", "KLANK"			# $] > 5.012
');

my %small = ("Perl", "Rules", "Beer", "Foamy");
my $b = %small;
do_test('small hash',
        \%small,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\($PADMY,SHAREKEYS\\)
    ARRAY = $ADDR  \\(0:[67],.*\\)
    hash quality = [0-9.]+%
    KEYS = 2
    FILL = [12]
    MAX = 7
(?:    Elt "(?:Perl|Beer)" HASH = $ADDR
    SV = PV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(POK,(?:IsCOW,)?pPOK\\)
      PV = $ADDR "(?:Rules|Foamy)"\\\0
      CUR = \d+
      LEN = \d+
      COW_REFCNT = 1
){2}');

$b = keys %small;

do_test('small hash after keys',
        \%small,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\($PADMY,OOK,SHAREKEYS\\)
    AUX_FLAGS = 0                               # $] > 5.019008
    ARRAY = $ADDR  \\(0:[67],.*\\)
    hash quality = [0-9.]+%
    KEYS = 2
    FILL = [12]
    MAX = 7
    RITER = -1
    EITER = 0x0
    RAND = $ADDR
(?:    Elt "(?:Perl|Beer)" HASH = $ADDR
    SV = PV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(POK,(?:IsCOW,)?pPOK\\)
      PV = $ADDR "(?:Rules|Foamy)"\\\0
      CUR = \d+
      LEN = \d+
      COW_REFCNT = 1
){2}');

$b = %small;

do_test('small hash after keys and scalar',
        \%small,
'SV = $RV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = PVHV\\($ADDR\\) at $ADDR
    REFCNT = 2
    FLAGS = \\($PADMY,OOK,SHAREKEYS\\)
    AUX_FLAGS = 0                               # $] > 5.019008
    ARRAY = $ADDR  \\(0:[67],.*\\)
    hash quality = [0-9.]+%
    KEYS = 2
    FILL = ([12])
    MAX = 7
    RITER = -1
    EITER = 0x0
    RAND = $ADDR
(?:    Elt "(?:Perl|Beer)" HASH = $ADDR
    SV = PV\\($ADDR\\) at $ADDR
      REFCNT = 1
      FLAGS = \\(POK,(?:IsCOW,)?pPOK\\)
      PV = $ADDR "(?:Rules|Foamy)"\\\0
      CUR = \d+
      LEN = \d+
      COW_REFCNT = 1
){2}');

# Dump with arrays, hashes, and operator return values
@array = 1..3;
do_test('Dump @array', '@array', <<'ARRAY', '', undef, 1);
SV = PVAV\($ADDR\) at $ADDR
  REFCNT = 1
  FLAGS = \(\)
  ARRAY = $ADDR
  FILL = 2
  MAX = 3
  FLAGS = \(REAL\)
  Elt No. 0
  SV = IV\($ADDR\) at $ADDR
    REFCNT = 1
    FLAGS = \(IOK,pIOK\)
    IV = 1
  Elt No. 1
  SV = IV\($ADDR\) at $ADDR
    REFCNT = 1
    FLAGS = \(IOK,pIOK\)
    IV = 2
  Elt No. 2
  SV = IV\($ADDR\) at $ADDR
    REFCNT = 1
    FLAGS = \(IOK,pIOK\)
    IV = 3
ARRAY

do_test('Dump @array,1', '@array,1', <<'ARRAY', '', undef, 1);
SV = PVAV\($ADDR\) at $ADDR
  REFCNT = 1
  FLAGS = \(\)
  ARRAY = $ADDR
  FILL = 2
  MAX = 3
  FLAGS = \(REAL\)
  Elt No. 0
  SV = IV\($ADDR\) at $ADDR
    REFCNT = 1
    FLAGS = \(IOK,pIOK\)
    IV = 1
ARRAY

%hash = 1..2;
do_test('Dump %hash', '%hash', <<'HASH', '', undef, 1);
SV = PVHV\($ADDR\) at $ADDR
  REFCNT = 1
  FLAGS = \(SHAREKEYS\)
  ARRAY = $ADDR  \(0:7, 1:1\)
  hash quality = 100.0%
  KEYS = 1
  FILL = 1
  MAX = 7
  Elt "1" HASH = $ADDR
  SV = IV\($ADDR\) at $ADDR
    REFCNT = 1
    FLAGS = \(IOK,pIOK\)
    IV = 2
HASH

tie %tied, "Tie::StdHash";
do_test('Dump %tied', '%tied', <<'HASH', "", undef, 1);
SV = PVHV\($ADDR\) at $ADDR
  REFCNT = 1
  FLAGS = \(RMG,SHAREKEYS\)
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_pack
    MG_TYPE = PERL_MAGIC_tied\(P\)
    MG_FLAGS = 0x02
      REFCOUNTED
    MG_OBJ = $ADDR
    SV = $RV\($ADDR\) at $ADDR
      REFCNT = 1
      FLAGS = \(ROK\)
      RV = $ADDR
      SV = PVHV\($ADDR\) at $ADDR
        REFCNT = 1
        FLAGS = \(OBJECT,SHAREKEYS\)
        STASH = $ADDR	"Tie::StdHash"
        ARRAY = 0x0
        KEYS = 0
        FILL = 0
        MAX = 7
  ARRAY = 0x0
  KEYS = 0
  FILL = 0
  MAX = 7
HASH

$_ = "hello";
do_test('rvalue substr', 'substr $_, 1, 2', <<'SUBSTR', '', undef, 1);
SV = PV\($ADDR\) at $ADDR
  REFCNT = 1
  FLAGS = \(PADTMP,POK,pPOK\)
  PV = $ADDR "el"\\0
  CUR = 2
  LEN = \d+
SUBSTR

# Dump with no arguments
eval 'Dump';
like $@, qr/^Not enough arguments for Devel::Peek::Dump/, 'Dump;';
eval 'Dump()';
like $@, qr/^Not enough arguments for Devel::Peek::Dump/, 'Dump()';

SKIP: {
    skip "Not built with usemymalloc", 2
      unless $Config{usemymalloc} eq 'y';
    my $x = __PACKAGE__;
    ok eval { fill_mstats($x); 1 }, 'fill_mstats on COW scalar'
     or diag $@;
    my $y;
    ok eval { fill_mstats($y); 1 }, 'fill_mstats on undef scalar';
}

# This is more a test of fbm_compile/pp_study (non) interaction than dumping
# prowess, but short of duplicating all the gubbins of this file, I can't see
# a way to make a better place for it:

use constant {

    # The length of the rhs string must be such that if chr() is applied to it
    # doesn't yield a character with a backslash mnemonic.  For example, if it
    # were 'rules' instead of 'rule', it would have 5 characters, and on
    # EBCDIC, chr(5) is \t.  The dumping code would translate all the 5's in
    # MG_PTR into "\t", and this test code would be expecting \5's, so the
    # tests would fail.  No platform that Perl works on translates chr(4) into
    # a mnemonic.
    perl => 'rule',
    beer => 'foam',
};

unless ($Config{useithreads}) {
    # These end up as copies in pads under ithreads, which rather defeats the
    # point of what we're trying to test here.

    do_test('regular string constant', perl,
'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 5
  FLAGS = \\(PADMY,POK,READONLY,(?:IsCOW,)?pPOK\\)	# $] < 5.021005
  FLAGS = \\(POK,(?:IsCOW,)?READONLY,pPOK\\)		# $] >=5.021005
  PV = $ADDR "rule"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
');

    eval 'index "", perl';

    do_test('string constant now an FBM', perl,
'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 5
  FLAGS = \\($PADMY,SMG,POK,(?:IsCOW,)?READONLY,(?:IsCOW,)?pPOK\\)
  PV = $ADDR "rule"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_regexp
    MG_TYPE = PERL_MAGIC_bm\\(B\\)
    MG_LEN = 256
    MG_PTR = $ADDR "(?:\\\\\d){256}"
  RARE = \d+					# $] < 5.019002
  PREVIOUS = 1					# $] < 5.019002
  USEFUL = 100
');

    is(study perl, '', "Not allowed to study an FBM");

    do_test('string constant still an FBM', perl,
'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 5
  FLAGS = \\($PADMY,SMG,POK,(?:IsCOW,)?READONLY,(?:IsCOW,)?pPOK\\)
  PV = $ADDR "rule"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_regexp
    MG_TYPE = PERL_MAGIC_bm\\(B\\)
    MG_LEN = 256
    MG_PTR = $ADDR "(?:\\\\\d){256}"
  RARE = \d+					# $] < 5.019002
  PREVIOUS = 1					# $] < 5.019002
  USEFUL = 100
');

    do_test('regular string constant', beer,
'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 6
  FLAGS = \\(PADMY,POK,READONLY,(?:IsCOW,)?pPOK\\)	# $] < 5.021005
  FLAGS = \\(POK,(?:IsCOW,)?READONLY,pPOK\\)		# $] >=5.021005
  PV = $ADDR "foam"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
');

    is(study beer, 1, "Our studies were successful");

    do_test('string constant quite unaffected', beer, 'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 6
  FLAGS = \\(PADMY,POK,READONLY,(?:IsCOW,)?pPOK\\)	# $] < 5.021005
  FLAGS = \\(POK,(?:IsCOW,)?READONLY,pPOK\\)		# $] >=5.021005
  PV = $ADDR "foam"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
');

    my $want = 'SV = PVMG\\($ADDR\\) at $ADDR
  REFCNT = 6
  FLAGS = \\($PADMY,SMG,POK,(?:IsCOW,)?READONLY,(?:IsCOW,)?pPOK\\)
  PV = $ADDR "foam"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 0
  MAGIC = $ADDR
    MG_VIRTUAL = &PL_vtbl_regexp
    MG_TYPE = PERL_MAGIC_bm\\(B\\)
    MG_LEN = 256
    MG_PTR = $ADDR "(?:\\\\\d){256}"
  RARE = \d+					# $] < 5.019002
  PREVIOUS = \d+				# $] < 5.019002
  USEFUL = 100
';

    is (eval 'index "not too foamy", beer', 8, 'correct index');

    do_test('string constant now FBMed', beer, $want);

    my $pie = 'good';

    is(study $pie, 1, "Our studies were successful");

    do_test('string constant still FBMed', beer, $want);

    do_test('second string also unaffected', $pie, 'SV = PV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\($PADMY,POK,(?:IsCOW,)?pPOK\\)
  PV = $ADDR "good"\\\0
  CUR = 4
  LEN = \d+
  COW_REFCNT = 1
');
}

# (One block of study tests removed when study was made a no-op.)

{
    open(OUT, '>', "peek$$") or die "Failed to open peek $$: $!";
    open(STDERR, ">&OUT") or die "Can't dup OUT: $!";
    DeadCode();
    open(STDERR, ">&SAVERR") or die "Can't restore STDERR: $!";
    pass "no crash with DeadCode";
    close OUT;
}
# note the conditionals on ENGINE and INTFLAGS were introduced in 5.19.9
do_test('UTF-8 in a regular expression',
        qr/\x{100}/,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = REGEXP\\($ADDR\\) at $ADDR
    REFCNT = 1
    FLAGS = \(OBJECT,POK,FAKE,pPOK,UTF8\)
    PV = $ADDR "\\(\\?\\^u:\\\\\\\\x\\{100\\}\\)" \\[UTF8 "\\(\\?\\^u:\\\\\\\\x\\{100\\}\\)"\\]
    CUR = 13
    LEN = 0
    STASH = $ADDR\\s+"Regexp"
    COMPFLAGS = 0x0 \\(\\)
    EXTFLAGS = $ADDR \\(CHECK_ALL,USE_INTUIT_NOML,USE_INTUIT_ML\\)
(?:    ENGINE = $ADDR \\(STANDARD\\)
)?    INTFLAGS = 0x0(?: \\(\\))?
    NPARENS = 0
    LOGICAL_NPARENS = 0
    LOGICAL_TO_PARNO = 0x0
    PARNO_TO_LOGICAL = 0x0
    PARNO_TO_LOGICAL_NEXT = 0x0
    LASTPAREN = 0
    LASTCLOSEPAREN = 0
    MINLEN = 1
    MINLENRET = 1
    GOFS = 0
    PRE_PREFIX = 5
    SUBLEN = 0
    SUBOFFSET = 0
    SUBCOFFSET = 0
    SUBBEG = 0x0
    PAREN_NAMES = 0x0
    SUBSTRS = $ADDR
    PPRIVATE = $ADDR
    OFFS = $ADDR
      \\[ 0:0 \\]
    QR_ANONCV = 0x0
    SAVED_COPY = 0x0
    MOTHER_RE = $ADDR
    SV = REGEXP\\($ADDR\\) at $ADDR
      REFCNT = 2
      FLAGS = \\(POK,pPOK,UTF8\\)
      PV = $ADDR "\\(\\?\\^u:\\\\\\\\x\\{100\\}\\)" \\[UTF8 "\\(\\?\\^u:\\\\\\\\x\\{100\\}\\)"\\]
      CUR = 13
      LEN = \\d+
      COMPFLAGS = 0x0 \\(\\)
      EXTFLAGS = 0x680100 \\(CHECK_ALL,USE_INTUIT_NOML,USE_INTUIT_ML\\)
      ENGINE = $ADDR \\(STANDARD\\)
      INTFLAGS = 0x0 \\(\\)
      NPARENS = 0
      LOGICAL_NPARENS = 0
      LOGICAL_TO_PARNO = 0x0
      PARNO_TO_LOGICAL = 0x0
      PARNO_TO_LOGICAL_NEXT = 0x0
      LASTPAREN = 0
      LASTCLOSEPAREN = 0
      MINLEN = 1
      MINLENRET = 1
      GOFS = 0
      PRE_PREFIX = 5
      SUBLEN = 0
      SUBOFFSET = 0
      SUBCOFFSET = 0
      SUBBEG = 0x0
      PAREN_NAMES = 0x0
      SUBSTRS = $ADDR
      PPRIVATE = $ADDR
      OFFS = $ADDR
        \\[ 0:0 \\]
      QR_ANONCV = 0x0
      SAVED_COPY = 0x0
      MOTHER_RE = 0x0
');

do_test('Branch Reset regexp',
        qr/(?|(foo)|(bar))(?|(baz)|(bop))/,
'SV = IV\\($ADDR\\) at $ADDR
  REFCNT = 1
  FLAGS = \\(ROK\\)
  RV = $ADDR
  SV = REGEXP\\($ADDR\\) at $ADDR
    REFCNT = 1
    FLAGS = \\(OBJECT,POK,FAKE,pPOK\\)
    PV = $ADDR "\\(\\?\\^:\\(\\?\\|\\(foo\\)\\|\\(bar\\)\\)\\(\\?\\|\\(baz\\)\\|\\(bop\\)\\)\\)"
    CUR = 35
    LEN = 0
    STASH = $ADDR\\s+"Regexp"
    COMPFLAGS = 0x0 \\(\\)
    EXTFLAGS = 0x0 \\(\\)
    ENGINE = $ADDR \\(STANDARD\\)
    INTFLAGS = 0x0 \\(\\)
    NPARENS = 4
    LOGICAL_NPARENS = 2
    LOGICAL_TO_PARNO = $ADDR
      \\{ 0, 1, 3 \\}
    PARNO_TO_LOGICAL = $ADDR
      \\{ 0, 1, 1, 2, 2 \\}
    PARNO_TO_LOGICAL_NEXT = $ADDR
      \\{ 0, 2, 0, 4, 0 \\}
    LASTPAREN = 0
    LASTCLOSEPAREN = 0
    MINLEN = 6
    MINLENRET = 6
    GOFS = 0
    PRE_PREFIX = 4
    SUBLEN = 0
    SUBOFFSET = 0
    SUBCOFFSET = 0
    SUBBEG = 0x0
    PAREN_NAMES = 0x0
    SUBSTRS = $ADDR
    PPRIVATE = $ADDR
    OFFS = $ADDR
      \\[ 0:0, 0:0, 0:0, 0:0, 0:0 \\]
    QR_ANONCV = 0x0
    SAVED_COPY = 0x0
    MOTHER_RE = $ADDR
    SV = REGEXP\\($ADDR\\) at $ADDR
      REFCNT = 2
      FLAGS = \\(POK,pPOK\\)
      PV = $ADDR "\\(\\?\\^:\\(\\?\\|\\(foo\\)\\|\\(bar\\)\\)\\(\\?\\|\\(baz\\)\\|\\(bop\\)\\)\\)"
      CUR = 35
      LEN = \\d+
      COMPFLAGS = 0x0 \\(\\)
      EXTFLAGS = 0x0 \\(\\)
      ENGINE = $ADDR \\(STANDARD\\)
      INTFLAGS = 0x0 \\(\\)
      NPARENS = 4
      LOGICAL_NPARENS = 2
      LOGICAL_TO_PARNO = $ADDR
        \\{ 0, 1, 3 \\}
      PARNO_TO_LOGICAL = $ADDR
        \\{ 0, 1, 1, 2, 2 \\}
      PARNO_TO_LOGICAL_NEXT = $ADDR
        \\{ 0, 2, 0, 4, 0 \\}
      LASTPAREN = 0
      LASTCLOSEPAREN = 0
      MINLEN = 6
      MINLENRET = 6
      GOFS = 0
      PRE_PREFIX = 4
      SUBLEN = 0
      SUBOFFSET = 0
      SUBCOFFSET = 0
      SUBBEG = 0x0
      PAREN_NAMES = 0x0
      SUBSTRS = $ADDR
      PPRIVATE = $ADDR
      OFFS = $ADDR
        \\[ 0:0, 0:0, 0:0, 0:0, 0:0 \\]
      QR_ANONCV = 0x0
      SAVED_COPY = 0x0
      MOTHER_RE = 0x0
');


{ # perl #117793: Extend SvREFCNT* to work on any perl variable type
  my %hash;
  my $base_count = Devel::Peek::SvREFCNT(%hash);
  my $ref = \%hash;
  is(Devel::Peek::SvREFCNT(%hash), $base_count + 1, "SvREFCNT on non-scalar");
  ok(!eval { &Devel::Peek::SvREFCNT(1) }, "requires prototype");
}
{
# utf8 tests
use utf8;

sub _dump {
   open(OUT, '>', "peek$$") or die $!;
   open(STDERR, ">&OUT") or die "Can't dup OUT: $!";
   Dump($_[0]);
   open(STDERR, ">&SAVERR") or die "Can't restore STDERR: $!";
   close(OUT);
   open(IN, '<', "peek$$") or die $!;
   my $dump = do { local $/; <IN> };
   close(IN);
   1 while unlink "peek$$";
   return $dump;
}

sub _get_coderef {
   my $x = $_[0];
   utf8::upgrade($x);
   eval "sub $x {}; 1" or die $@;
   return *{$x}{CODE};
}

like(
   _dump(_get_coderef("\x{df}::\xdf")),
   qr/GVGV::GV = 0x[[:xdigit:]]+\s+\Q"\xdf" :: "\xdf"/,
   "GVGV's are correctly escaped for latin1 :: latin1",
);

like(
   _dump(_get_coderef("\x{30cd}::\x{30cd}")),
   qr/GVGV::GV = 0x[[:xdigit:]]+\s+\Q"\x{30cd}" :: "\x{30cd}"/,
   "GVGV's are correctly escaped for UTF8 :: UTF8",
);

like(
   _dump(_get_coderef("\x{df}::\x{30cd}")),
   qr/GVGV::GV = 0x[[:xdigit:]]+\s+\Q"\xdf" :: "\x{30cd}"/,
   "GVGV's are correctly escaped for latin1 :: UTF8",
);

like(
   _dump(_get_coderef("\x{30cd}::\x{df}")),
   qr/GVGV::GV = 0x[[:xdigit:]]+\s+\Q"\x{30cd}" :: "\xdf"/,
   "GVGV's are correctly escaped for UTF8 :: latin1",
);

like(
   _dump(_get_coderef("\x{30cb}::\x{df}::\x{30cd}")),
   qr/GVGV::GV = 0x[[:xdigit:]]+\s+\Q"\x{30cb}::\x{df}" :: "\x{30cd}"/,
   "GVGV's are correctly escaped for UTF8 :: latin 1 :: UTF8",
);

my $dump = _dump(*{"\x{30cb}::\x{df}::\x{30dc}"});

like(
   $dump,
   qr/NAME = \Q"\x{30dc}"/,
   "NAME is correctly escaped for UTF8 globs",
);

like(
   $dump,
   qr/GvSTASH = 0x[[:xdigit:]]+\s+\Q"\x{30cb}::\x{df}"/,
   "GvSTASH is correctly escaped for UTF8 globs"
);

like(
   $dump,
   qr/EGV = 0x[[:xdigit:]]+\s+\Q"\x{30dc}"/,
   "EGV is correctly escaped for UTF8 globs"
);

$dump = _dump(*{"\x{df}::\x{30cc}"});

like(
   $dump,
   qr/NAME = \Q"\x{30cc}"/,
   "NAME is correctly escaped for UTF8 globs with latin1 stashes",
);

like(
   $dump,
   qr/GvSTASH = 0x[[:xdigit:]]+\s+\Q"\xdf"/,
   "GvSTASH is correctly escaped for UTF8 globs with latin1 stashes"
);

like(
   $dump,
   qr/EGV = 0x[[:xdigit:]]+\s+\Q"\x{30cc}"/,
   "EGV is correctly escaped for UTF8 globs with latin1 stashes"
);

like(
   _dump(bless {}, "\0::\1::\x{30cd}"),
   qr/STASH = 0x[[:xdigit:]]+\s+\Q"\0::\x{01}::\x{30cd}"/,
   "STASH for blessed hashrefs is correct"
);

BEGIN { $::{doof} = "\0\1\x{30cd}" }
like(
   _dump(\&doof),
   qr/PROTOTYPE = \Q"\0\x{01}\x{30cd}"/,
   "PROTOTYPE is escaped correctly"
);

{
    my $coderef = eval <<"EOP";
    use feature 'lexical_subs';
    no warnings 'experimental::lexical_subs';
    my sub bar (\$\x{30cd}) {1}; \\&bar
EOP
    like(
       _dump($coderef),
       qr/PROTOTYPE = "\$\Q\x{30cd}"/,
       "PROTOTYPE works on lexical subs"
    )
}

sub get_outside {
   eval "sub $_[0] { my \$x; \$x++; return sub { eval q{\$x} } } $_[0]()";
}
sub basic { my $x; return eval q{sub { eval q{$x} }} }
like(
    _dump(basic()),
    qr/OUTSIDE = 0x[[:xdigit:]]+\s+\Q(basic)/,
    'OUTSIDE works'
);

like(
    _dump(get_outside("\x{30ce}")),
    qr/OUTSIDE = 0x[[:xdigit:]]+\s+\Q(\x{30ce})/,
    'OUTSIDE + UTF8 works'
);

# TODO AUTOLOAD = stashname, which requires using a XS autoload
# and calling Dump() on the cv



sub test_utf8_stashes {
   my ($stash_name, $test) = @_;

   $dump = _dump(\%{"${stash_name}::"});

   my $format = utf8::is_utf8($stash_name) ? '\x{%2x}' : '\x%2x';
   $escaped_stash_name = join "", map {
         $_ eq ':' ? $_ : sprintf $format, ord $_
   } split //, $stash_name;

   like(
      $dump,
      qr/\QNAME = "$escaped_stash_name"/,
      "NAME is correct escaped for $test"
   );

   like(
      $dump,
      qr/\QENAME = "$escaped_stash_name"/,
      "ENAME is correct escaped for $test"
   );
}

for my $test (
  [ "\x{30cd}", "UTF8 stashes" ],
   [ "\x{df}", "latin 1 stashes" ],
   [ "\x{df}::\x{30cd}", "latin1 + UTF8 stashes" ],
   [ "\x{30cd}::\x{df}", "UTF8 + latin1 stashes" ],
) {
   test_utf8_stashes(@$test);
}

}

my $runperl_args = { switches => ['-Ilib'] };
sub test_DumpProg {
    my ($prog, $expected, $name, $test) = @_;
    $test ||= 'like';

    my $u = 'use Devel::Peek "DumpProg"; DumpProg();';

    # Interface between Test::Builder & test.pl
    my $builder = Test::More->builder();
    t::curr_test($builder->current_test() + 1);

    utf8::encode($prog);
    
    if ( $test eq 'is' ) {
        t::fresh_perl_is($prog . $u, $expected, $runperl_args, $name)
    }
    else {
        t::fresh_perl_like($prog . $u, $expected, $runperl_args, $name)
    }

    $builder->current_test(t::curr_test() - 1);
}

my $threads = $Config{'useithreads'};

for my $test (
[
    "package test;",
    qr/PACKAGE = "test"/,
    "DumpProg() + package declaration"
],
[
    "use utf8; package \x{30cd};",
    qr/PACKAGE = "\\x\Q{30cd}"/,
    "DumpProg() + UTF8 package declaration"
],
[
    "use utf8; sub \x{30cc}::\x{30cd} {1}; \x{30cc}::\x{30cd};",
    ($threads ? qr/PADIX = \d+/ : qr/GV = \Q\x{30cc}::\x{30cd}\E/)
],
[
    "use utf8; \x{30cc}: { last \x{30cc} }",
    qr/LABEL = \Q"\x{30cc}"/
],
)
{
   test_DumpProg(@$test);
}

{
    local $TODO = 'This gets mangled by the current pipe implementation' if $^O eq 'VMS';
    my $e = <<'EODUMP';
dumpindent is 4 at -e line 1.
     
1    leave LISTOP(0xNNN) ===> [0x0]
     PARENT ===> [0x0]
     TARG = 1
     FLAGS = (VOID,KIDS,PARENS,SLABBED)
     PRIVATE = (REFC)
     REFCNT = 1
     |   
2    +--enter OP(0xNNN) ===> 3 [nextstate 0xNNN]
     |   FLAGS = (VOID,SLABBED,MORESIB)
     |   
3    +--nextstate COP(0xNNN) ===> 4 [pushmark 0xNNN]
     |   FLAGS = (VOID,SLABBED,MORESIB)
     |   LINE = 1
     |   PACKAGE = "t"
     |   HINTS = 00000100
     |     |   
5    +--entersub UNOP(0xNNN) ===> 1 [leave 0xNNN]
         TARG = 1
         FLAGS = (VOID,KIDS,STACKED,SLABBED)
         PRIVATE = (TARG)
         |   
6        +--null (ex-list) UNOP(0xNNN) ===> 5 [entersub 0xNNN]
             FLAGS = (UNKNOWN,KIDS,SLABBED)
             |   
4            +--pushmark OP(0xNNN) ===> 7 [gv 0xNNN]
             |   FLAGS = (SCALAR,SLABBED,MORESIB)
             |   
8            +--null (ex-rv2cv) UNOP(0xNNN) ===> 6 [null 0xNNN]
                 FLAGS = (SCALAR,KIDS,SLABBED)
                 PRIVATE = (0x1)
                 |   
7                +--gv SVOP(0xNNN) ===> 5 [entersub 0xNNN]
                     FLAGS = (SCALAR,SLABBED)
                     GV_OR_PADIX
EODUMP

    $e =~ s/GV_OR_PADIX/$threads ? "PADIX = 2" : "GV = t::DumpProg (0xNNN)"/e;
    $e =~ s/SVOP/PADOP/g if $threads;
    my $out = t::runperl
                 switches => ['-Ilib'],
                 prog => 'package t; use Devel::Peek q-DumpProg-; DumpProg();',
                 stderr=>1;
    $out =~ s/ *SEQ = .*\n//;
    $out =~ s/0x[0-9a-f]{2,}\]/${1}0xNNN]/g;
    $out =~ s/\(0x[0-9a-f]{3,}\)/(0xNNN)/g;
    is $out, $e, "DumpProg() has no 'Attempt to free X prematurely' warning";
}

{
    my $epsilon_p = 1.0;
    my $epsilon_n = 1.0;
    if($Config{nvtype} eq 'long double' &&
       $Config{longdblkind} >= 5 && $Config{longdblkind} <= 8) {
      # For this (doubledouble) kind of NV we need to use a separate
      # method for assigning values to $epsilon_p and $epsilon_n. 
      # Theoretically, $epsilon_p should be set to 2 ** -107, and
      # $epsilon_n to 2 ** -110. However, a known possible bug in "%.33g"
      # formatting will render those values inaccurately, thereby
      # incorrectly influencing the results of the "NV 1.0 + epsilon" 
      # and "NV 1.0 - epsilon" tests. So we test for the presence of
      # the bug, and set both of those "epsilon" variables to
      # 2 ** -105 if the bug is detected.
      # See the discussion at https://github.com/Perl/perl5/issues/19585.

      if( sprintf("%.33g", 1.0 + (2 ** -108)) == 1
          &&
          sprintf("%.33g", 1.0 + (2 ** -107)) > 1 ) {

          $epsilon_p = 2 ** -107;
      }
      else { $epsilon_p = 2 ** -105 } # Avoids the formatting bug.

      if( sprintf("%.33g", 1.0 - (2 ** -111)) == 1
          &&
          sprintf("%.33g", 1.0 - (2 ** -110)) < 1 ) {

          $epsilon_n = 2 ** -110;
      }
      else { $epsilon_n = 2 ** -105 } # Avoids the formatting bug.

    }
    else {
        $epsilon_p /= 2 while 1.0 != 1.0 + $epsilon_p / 2;
        $epsilon_n /= 2 while 1.0 != 1.0 - $epsilon_n / 2;
    }

    my $head = 'SV = NV\($ADDR\) at $ADDR
(?:.+
)*  ';
    my $tail = '
(?:.+
)*';

    do_test('NV 1.0', 1.0,
            $head . 'NV = 1' . $tail);
    do_test('NV 1.0 + epsilon', 1.0 + $epsilon_p,
            $head . 'NV = 1\.00000000\d+' . $tail);
    do_test('NV 1.0 - epsilon', 1.0 - $epsilon_n,
            $head . 'NV = 0\.99999999\d+' . $tail);
}

done_testing();
