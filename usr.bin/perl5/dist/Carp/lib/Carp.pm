package Carp;

{ use 5.006; }
use strict;
use warnings;
BEGIN {
    # Very old versions of warnings.pm load Carp.  This can go wrong due
    # to the circular dependency.  If warnings is invoked before Carp,
    # then warnings starts by loading Carp, then Carp (above) tries to
    # invoke warnings, and gets nothing because warnings is in the process
    # of loading and hasn't defined its import method yet.  If we were
    # only turning on warnings ("use warnings" above) this wouldn't be too
    # bad, because Carp would just gets the state of the -w switch and so
    # might not get some warnings that it wanted.  The real problem is
    # that we then want to turn off Unicode warnings, but "no warnings
    # 'utf8'" won't be effective if we're in this circular-dependency
    # situation.  So, if warnings.pm is an affected version, we turn
    # off all warnings ourselves by directly setting ${^WARNING_BITS}.
    # On unaffected versions, we turn off just Unicode warnings, via
    # the proper API.
    if(!defined($warnings::VERSION) || eval($warnings::VERSION) < 1.06) {
	${^WARNING_BITS} = "";
    } else {
	"warnings"->unimport("utf8");
    }
}

sub _fetch_sub { # fetch sub without autovivifying
    my($pack, $sub) = @_;
    $pack .= '::';
    # only works with top-level packages
    return unless exists($::{$pack});
    for ($::{$pack}) {
	return unless ref \$_ eq 'GLOB' && *$_{HASH} && exists $$_{$sub};
	for ($$_{$sub}) {
	    return ref \$_ eq 'GLOB' ? *$_{CODE} : undef
	}
    }
}

# UTF8_REGEXP_PROBLEM is a compile-time constant indicating whether Carp
# must avoid applying a regular expression to an upgraded (is_utf8)
# string.  There are multiple problems, on different Perl versions,
# that require this to be avoided.  All versions prior to 5.13.8 will
# load utf8_heavy.pl for the swash system, even if the regexp doesn't
# use character classes.  Perl 5.6 and Perls [5.11.2, 5.13.11) exhibit
# specific problems when Carp is being invoked in the aftermath of a
# syntax error.
BEGIN {
    if("$]" < 5.013011) {
	*UTF8_REGEXP_PROBLEM = sub () { 1 };
    } else {
	*UTF8_REGEXP_PROBLEM = sub () { 0 };
    }
}

# is_utf8() is essentially the utf8::is_utf8() function, which indicates
# whether a string is represented in the upgraded form (using UTF-8
# internally).  As utf8::is_utf8() is only available from Perl 5.8
# onwards, extra effort is required here to make it work on Perl 5.6.
BEGIN {
    if(defined(my $sub = _fetch_sub utf8 => 'is_utf8')) {
	*is_utf8 = $sub;
    } else {
	# black magic for perl 5.6
	*is_utf8 = sub { unpack("C", "\xaa".$_[0]) != 170 };
    }
}

# The downgrade() function defined here is to be used for attempts to
# downgrade where it is acceptable to fail.  It must be called with a
# second argument that is a true value.
BEGIN {
    if(defined(my $sub = _fetch_sub utf8 => 'downgrade')) {
	*downgrade = \&{"utf8::downgrade"};
    } else {
	*downgrade = sub {
	    my $r = "";
	    my $l = length($_[0]);
	    for(my $i = 0; $i != $l; $i++) {
		my $o = ord(substr($_[0], $i, 1));
		return if $o > 255;
		$r .= chr($o);
	    }
	    $_[0] = $r;
	};
    }
}

# is_safe_printable_codepoint() indicates whether a character, specified
# by integer codepoint, is OK to output literally in a trace.  Generally
# this is if it is a printable character in the ancestral character set
# (ASCII or EBCDIC).  This is used on some Perls in situations where a
# regexp can't be used.
BEGIN {
    *is_safe_printable_codepoint =
	"$]" >= 5.007_003 ?
	    eval(q(sub ($) {
		my $u = utf8::native_to_unicode($_[0]);
		$u >= 0x20 && $u <= 0x7e;
	    }))
	: ord("A") == 65 ?
	    sub ($) { $_[0] >= 0x20 && $_[0] <= 0x7e }
	:
	    sub ($) {
		# Early EBCDIC
		# 3 EBCDIC code pages supported then;  all controls but one
		# are the code points below SPACE.  The other one is 0x5F on
		# POSIX-BC; FF on the other two.
		# FIXME: there are plenty of unprintable codepoints other
		# than those that this code and the comment above identifies
		# as "controls".
		$_[0] >= ord(" ") && $_[0] <= 0xff &&
		    $_[0] != (ord ("^") == 106 ? 0x5f : 0xff);
	    }
	;
}

sub _univ_mod_loaded {
    return 0 unless exists($::{"UNIVERSAL::"});
    for ($::{"UNIVERSAL::"}) {
	return 0 unless ref \$_ eq "GLOB" && *$_{HASH} && exists $$_{"$_[0]::"};
	for ($$_{"$_[0]::"}) {
	    return 0 unless ref \$_ eq "GLOB" && *$_{HASH} && exists $$_{"VERSION"};
	    for ($$_{"VERSION"}) {
		return 0 unless ref \$_ eq "GLOB";
		return ${*$_{SCALAR}};
	    }
	}
    }
}

# _maybe_isa() is usually the UNIVERSAL::isa function.  We have to avoid
# the latter if the UNIVERSAL::isa module has been loaded, to avoid infi-
# nite recursion; in that case _maybe_isa simply returns true.
my $isa;
BEGIN {
    if (_univ_mod_loaded('isa')) {
        *_maybe_isa = sub { 1 }
    }
    else {
        # Since we have already done the check, record $isa for use below
        # when defining _StrVal.
        *_maybe_isa = $isa = _fetch_sub(UNIVERSAL => "isa");
    }
}


# We need an overload::StrVal or equivalent function, but we must avoid
# loading any modules on demand, as Carp is used from __DIE__ handlers and
# may be invoked after a syntax error.
# We can copy recent implementations of overload::StrVal and use
# overloading.pm, which is the fastest implementation, so long as
# overloading is available.  If it is not available, we use our own pure-
# Perl StrVal.  We never actually use overload::StrVal, for various rea-
# sons described below.
# overload versions are as follows:
#     undef-1.00 (up to perl 5.8.0)   uses bless (avoid!)
#     1.01-1.17  (perl 5.8.1 to 5.14) uses Scalar::Util
#     1.18+      (perl 5.16+)         uses overloading
# The ancient 'bless' implementation (that inspires our pure-Perl version)
# blesses unblessed references and must be avoided.  Those using
# Scalar::Util use refaddr, possibly the pure-Perl implementation, which
# has the same blessing bug, and must be avoided.  Also, Scalar::Util is
# loaded on demand.  Since we avoid the Scalar::Util implementations, we
# end up having to implement our own overloading.pm-based version for perl
# 5.10.1 to 5.14.  Since it also works just as well in more recent ver-
# sions, we use it there, too.
BEGIN {
    if (eval { require "overloading.pm" }) {
        *_StrVal = eval 'sub { no overloading; "$_[0]" }'
    }
    else {
        # Work around the UNIVERSAL::can/isa modules to avoid recursion.

        # _mycan is either UNIVERSAL::can, or, in the presence of an
        # override, overload::mycan.
        *_mycan = _univ_mod_loaded('can')
            ? do { require "overload.pm"; _fetch_sub overload => 'mycan' }
            : \&UNIVERSAL::can;

        # _blessed is either UNIVERSAL::isa(...), or, in the presence of an
        # override, a hideous, but fairly reliable, workaround.
        *_blessed = $isa
            ? sub { &$isa($_[0], "UNIVERSAL") }
            : sub {
                my $probe = "UNIVERSAL::Carp_probe_" . rand;
                no strict 'refs';
                local *$probe = sub { "unlikely string" };
                local $@;
                local $SIG{__DIE__} = sub{};
                (eval { $_[0]->$probe } || '') eq 'unlikely string'
              };

        *_StrVal = sub {
            my $pack = ref $_[0];
            # Perl's overload mechanism uses the presence of a special
            # "method" named "((" or "()" to signal it is in effect.
            # This test seeks to see if it has been set up.  "((" post-
            # dates overloading.pm, so we can skip it.
            return "$_[0]" unless _mycan($pack, "()");
            # Even at this point, the invocant may not be blessed, so
            # check for that.
            return "$_[0]" if not _blessed($_[0]);
            bless $_[0], "Carp";
            my $str = "$_[0]";
            bless $_[0], $pack;
            $pack . substr $str, index $str, "=";
        }
    }
}


our $VERSION = '1.54';
$VERSION =~ tr/_//d;

our $MaxEvalLen = 0;
our $Verbose    = 0;
our $CarpLevel  = 0;
our $MaxArgLen  = 64;    # How much of each argument to print. 0 = all.
our $MaxArgNums = 8;     # How many arguments to print. 0 = all.
our $RefArgFormatter = undef; # allow caller to format reference arguments

require Exporter;
our @ISA       = ('Exporter');
our @EXPORT    = qw(confess croak carp);
our @EXPORT_OK = qw(cluck verbose longmess shortmess);
our @EXPORT_FAIL = qw(verbose);    # hook to enable verbose mode

# The members of %Internal are packages that are internal to perl.
# Carp will not report errors from within these packages if it
# can.  The members of %CarpInternal are internal to Perl's warning
# system.  Carp will not report errors from within these packages
# either, and will not report calls *to* these packages for carp and
# croak.  They replace $CarpLevel, which is deprecated.    The
# $Max(EvalLen|(Arg(Len|Nums)) variables are used to specify how the eval
# text and function arguments should be formatted when printed.

our %CarpInternal;
our %Internal;

# disable these by default, so they can live w/o require Carp
$CarpInternal{Carp}++;
$CarpInternal{warnings}++;
$Internal{Exporter}++;
$Internal{'Exporter::Heavy'}++;

# if the caller specifies verbose usage ("perl -MCarp=verbose script.pl")
# then the following method will be called by the Exporter which knows
# to do this thanks to @EXPORT_FAIL, above.  $_[1] will contain the word
# 'verbose'.

sub export_fail { shift; $Verbose = shift if $_[0] eq 'verbose'; @_ }

sub _cgc {
    no strict 'refs';
    return \&{"CORE::GLOBAL::caller"} if defined &{"CORE::GLOBAL::caller"};
    return;
}

sub longmess {
    local($!, $^E);
    # Icky backwards compatibility wrapper. :-(
    #
    # The story is that the original implementation hard-coded the
    # number of call levels to go back, so calls to longmess were off
    # by one.  Other code began calling longmess and expecting this
    # behaviour, so the replacement has to emulate that behaviour.
    my $cgc = _cgc();
    my $call_pack = $cgc ? $cgc->() : caller();
    if ( $Internal{$call_pack} or $CarpInternal{$call_pack} ) {
        return longmess_heavy(@_);
    }
    else {
        local $CarpLevel = $CarpLevel + 1;
        return longmess_heavy(@_);
    }
}

our @CARP_NOT;

sub shortmess {
    local($!, $^E);
    my $cgc = _cgc();

    # Icky backwards compatibility wrapper. :-(
    local @CARP_NOT = scalar( $cgc ? $cgc->() : caller() );
    shortmess_heavy(@_);
}

sub croak   { die shortmess @_ }
sub confess { die longmess @_ }
sub carp    { warn shortmess @_ }
sub cluck   { warn longmess @_ }

BEGIN {
    if("$]" >= 5.015002 || ("$]" >= 5.014002 && "$]" < 5.015) ||
	    ("$]" >= 5.012005 && "$]" < 5.013)) {
	*CALLER_OVERRIDE_CHECK_OK = sub () { 1 };
    } else {
	*CALLER_OVERRIDE_CHECK_OK = sub () { 0 };
    }
}

sub caller_info {
    my $i = shift(@_) + 1;
    my %call_info;
    my $cgc = _cgc();
    {
	# Some things override caller() but forget to implement the
	# @DB::args part of it, which we need.  We check for this by
	# pre-populating @DB::args with a sentinel which no-one else
	# has the address of, so that we can detect whether @DB::args
	# has been properly populated.  However, on earlier versions
	# of perl this check tickles a bug in CORE::caller() which
	# leaks memory.  So we only check on fixed perls.
        @DB::args = \$i if CALLER_OVERRIDE_CHECK_OK;
        package DB;
        @call_info{
            qw(pack file line sub has_args wantarray evaltext is_require) }
            = $cgc ? $cgc->($i) : caller($i);
    }

    unless ( defined $call_info{file} ) {
        return ();
    }

    my $sub_name = Carp::get_subname( \%call_info );
    if ( $call_info{has_args} ) {
        # Guard our serialization of the stack from stack refcounting bugs
        # NOTE this is NOT a complete solution, we cannot 100% guard against
        # these bugs.  However in many cases Perl *is* capable of detecting
        # them and throws an error when it does.  Unfortunately serializing
        # the arguments on the stack is a perfect way of finding these bugs,
        # even when they would not affect normal program flow that did not
        # poke around inside the stack.  Inside of Carp.pm it makes little
        # sense reporting these bugs, as Carp's job is to report the callers
        # errors, not the ones it might happen to tickle while doing so.
        # See: https://rt.perl.org/Public/Bug/Display.html?id=131046
        # and: https://rt.perl.org/Public/Bug/Display.html?id=52610
        # for more details and discussion. - Yves
        my @args = map {
                my $arg;
                local $@= $@;
                eval {
                    $arg = $_;
                    1;
                } or do {
                    $arg = '** argument not available anymore **';
                };
                $arg;
            } @DB::args;
        if (CALLER_OVERRIDE_CHECK_OK && @args == 1
            && ref $args[0] eq ref \$i
            && $args[0] == \$i ) {
            @args = ();    # Don't let anyone see the address of $i
            local $@;
            my $where = eval {
                my $func    = $cgc or return '';
                my $gv      =
                    (_fetch_sub B => 'svref_2object' or return '')
                        ->($func)->GV;
                my $package = $gv->STASH->NAME;
                my $subname = $gv->NAME;
                return unless defined $package && defined $subname;

                # returning CORE::GLOBAL::caller isn't useful for tracing the cause:
                return if $package eq 'CORE::GLOBAL' && $subname eq 'caller';
                " in &${package}::$subname";
            } || '';
            @args
                = "** Incomplete caller override detected$where; \@DB::args were not set **";
        }
        else {
            my $overflow;
            if ( $MaxArgNums and @args > $MaxArgNums )
            {    # More than we want to show?
                $#args = $MaxArgNums - 1;
                $overflow = 1;
            }

            @args = map { Carp::format_arg($_) } @args;

            if ($overflow) {
                push @args, '...';
            }
        }

        # Push the args onto the subroutine
        $sub_name .= '(' . join( ', ', @args ) . ')';
    }
    $call_info{sub_name} = $sub_name;
    return wantarray() ? %call_info : \%call_info;
}

# Transform an argument to a function into a string.
our $in_recurse;
sub format_arg {
    my $arg = shift;

    if ( my $pack= ref($arg) ) {

         # legitimate, let's not leak it.
        if (!$in_recurse && _maybe_isa( $arg, 'UNIVERSAL' ) &&
	    do {
                local $@;
	        local $in_recurse = 1;
		local $SIG{__DIE__} = sub{};
                eval {$arg->can('CARP_TRACE') }
            })
        {
            return $arg->CARP_TRACE();
        }
        elsif (!$in_recurse &&
	       defined($RefArgFormatter) &&
	       do {
                local $@;
	        local $in_recurse = 1;
		local $SIG{__DIE__} = sub{};
                eval {$arg = $RefArgFormatter->($arg); 1}
                })
        {
            return $arg;
        }
        else
        {
            # Argument may be blessed into a class with overloading, and so
            # might have an overloaded stringification.  We don't want to
            # risk getting the overloaded stringification, so we need to
            # use _StrVal, our overload::StrVal()-equivalent.
            return _StrVal $arg;
        }
    }
    return "undef" if !defined($arg);
    downgrade($arg, 1);
    return $arg if !(UTF8_REGEXP_PROBLEM && is_utf8($arg)) &&
	    $arg =~ /\A-?[0-9]+(?:\.[0-9]*)?(?:[eE][-+]?[0-9]+)?\z/;
    my $suffix = "";
    if ( 2 < $MaxArgLen and $MaxArgLen < length($arg) ) {
        substr ( $arg, $MaxArgLen - 3 ) = "";
	$suffix = "...";
    }
    if(UTF8_REGEXP_PROBLEM && is_utf8($arg)) {
	for(my $i = length($arg); $i--; ) {
	    my $c = substr($arg, $i, 1);
	    my $x = substr($arg, 0, 0);   # work around bug on Perl 5.8.{1,2}
	    if($c eq "\"" || $c eq "\\" || $c eq "\$" || $c eq "\@") {
		substr $arg, $i, 0, "\\";
		next;
	    }
	    my $o = ord($c);
	    substr $arg, $i, 1, sprintf("\\x{%x}", $o)
		unless is_safe_printable_codepoint($o);
	}
    } else {
	$arg =~ s/([\"\\\$\@])/\\$1/g;
        # This is all the ASCII printables spelled-out.  It is portable to all
        # Perl versions and platforms (such as EBCDIC).  There are other more
        # compact ways to do this, but may not work everywhere every version.
        $arg =~ s/([^ !"#\$\%\&'()*+,\-.\/0123456789:;<=>?\@ABCDEFGHIJKLMNOPQRSTUVWXYZ\[\\\]^_`abcdefghijklmnopqrstuvwxyz\{|}~])/sprintf("\\x{%x}",ord($1))/eg;
    }
    downgrade($arg, 1);
    return "\"".$arg."\"".$suffix;
}

sub Regexp::CARP_TRACE {
    my $arg = "$_[0]";
    downgrade($arg, 1);
    if(UTF8_REGEXP_PROBLEM && is_utf8($arg)) {
	for(my $i = length($arg); $i--; ) {
	    my $o = ord(substr($arg, $i, 1));
	    my $x = substr($arg, 0, 0);   # work around bug on Perl 5.8.{1,2}
	    substr $arg, $i, 1, sprintf("\\x{%x}", $o)
		unless is_safe_printable_codepoint($o);
	}
    } else {
        # See comment in format_arg() about this same regex.
        $arg =~ s/([^ !"#\$\%\&'()*+,\-.\/0123456789:;<=>?\@ABCDEFGHIJKLMNOPQRSTUVWXYZ\[\\\]^_`abcdefghijklmnopqrstuvwxyz\{|}~])/sprintf("\\x{%x}",ord($1))/eg;
    }
    downgrade($arg, 1);
    my $suffix = "";
    if($arg =~ /\A\(\?\^?([a-z]*)(?:-[a-z]*)?:(.*)\)\z/s) {
	($suffix, $arg) = ($1, $2);
    }
    if ( 2 < $MaxArgLen and $MaxArgLen < length($arg) ) {
        substr ( $arg, $MaxArgLen - 3 ) = "";
	$suffix = "...".$suffix;
    }
    return "qr($arg)$suffix";
}

# Takes an inheritance cache and a package and returns
# an anon hash of known inheritances and anon array of
# inheritances which consequences have not been figured
# for.
sub get_status {
    my $cache = shift;
    my $pkg   = shift;
    $cache->{$pkg} ||= [ { $pkg => $pkg }, [ trusts_directly($pkg) ] ];
    return @{ $cache->{$pkg} };
}

# Takes the info from caller() and figures out the name of
# the sub/require/eval
sub get_subname {
    my $info = shift;
    if ( defined( $info->{evaltext} ) ) {
        my $eval = $info->{evaltext};
        if ( $info->{is_require} ) {
            return "require $eval";
        }
        else {
            $eval =~ s/([\\\'])/\\$1/g;
            return "eval '" . str_len_trim( $eval, $MaxEvalLen ) . "'";
        }
    }

    # this can happen on older perls when the sub (or the stash containing it)
    # has been deleted
    if ( !defined( $info->{sub} ) ) {
        return '__ANON__::__ANON__';
    }

    return ( $info->{sub} eq '(eval)' ) ? 'eval {...}' : $info->{sub};
}

# Figures out what call (from the point of view of the caller)
# the long error backtrace should start at.
sub long_error_loc {
    my $i;
    my $lvl = $CarpLevel;
    {
        ++$i;
        my $cgc = _cgc();
        my @caller = $cgc ? $cgc->($i) : caller($i);
        my $pkg = $caller[0];
        unless ( defined($pkg) ) {

            # This *shouldn't* happen.
            if (%Internal) {
                local %Internal;
                $i = long_error_loc();
                last;
            }
            elsif (defined $caller[2]) {
                # this can happen when the stash has been deleted
                # in that case, just assume that it's a reasonable place to
                # stop (the file and line data will still be intact in any
                # case) - the only issue is that we can't detect if the
                # deleted package was internal (so don't do that then)
                # -doy
                redo unless 0 > --$lvl;
                last;
            }
            else {
                return 2;
            }
        }
        redo if $CarpInternal{$pkg};
        redo unless 0 > --$lvl;
        redo if $Internal{$pkg};
    }
    return $i - 1;
}

sub longmess_heavy {
    if ( ref( $_[0] ) ) {   # don't break references as exceptions
        return wantarray ? @_ : $_[0];
    }
    my $i = long_error_loc();
    return ret_backtrace( $i, @_ );
}

BEGIN {
    if("$]" >= 5.017004) {
        # The LAST_FH constant is a reference to the variable.
        $Carp::{LAST_FH} = \eval '\${^LAST_FH}';
    } else {
        eval '*LAST_FH = sub () { 0 }';
    }
}

# Returns a full stack backtrace starting from where it is
# told.
sub ret_backtrace {
    my ( $i, @error ) = @_;
    my $mess;
    my $err = join '', @error;
    $i++;

    my $tid_msg = '';
    if ( defined &threads::tid ) {
        my $tid = threads->tid;
        $tid_msg = " thread $tid" if $tid;
    }

    my %i = caller_info($i);
    $mess = "$err at $i{file} line $i{line}$tid_msg";
    if( $. ) {
      # Use ${^LAST_FH} if available.
      if (LAST_FH) {
        if (${+LAST_FH}) {
            $mess .= sprintf ", <%s> %s %d",
                              *${+LAST_FH}{NAME},
                              ($/ eq "\n" ? "line" : "chunk"), $.
        }
      }
      else {
        local $@ = '';
        local $SIG{__DIE__};
        eval {
            CORE::die;
        };
        if($@ =~ /^Died at .*(, <.*?> (?:line|chunk) \d+).$/ ) {
            $mess .= $1;
        }
      }
    }
    $mess .= "\.\n";

    while ( my %i = caller_info( ++$i ) ) {
        $mess .= "\t$i{sub_name} called at $i{file} line $i{line}$tid_msg\n";
    }

    return $mess;
}

sub ret_summary {
    my ( $i, @error ) = @_;
    my $err = join '', @error;
    $i++;

    my $tid_msg = '';
    if ( defined &threads::tid ) {
        my $tid = threads->tid;
        $tid_msg = " thread $tid" if $tid;
    }

    my %i = caller_info($i);
    return "$err at $i{file} line $i{line}$tid_msg\.\n";
}

sub short_error_loc {
    # You have to create your (hash)ref out here, rather than defaulting it
    # inside trusts *on a lexical*, as you want it to persist across calls.
    # (You can default it on $_[2], but that gets messy)
    my $cache = {};
    my $i     = 1;
    my $lvl   = $CarpLevel;
    {
        my $cgc = _cgc();
        my $called = $cgc ? $cgc->($i) : caller($i);
        $i++;
        my $caller = $cgc ? $cgc->($i) : caller($i);

        if (!defined($caller)) {
            my @caller = $cgc ? $cgc->($i) : caller($i);
            if (@caller) {
                # if there's no package but there is other caller info, then
                # the package has been deleted - treat this as a valid package
                # in this case
                redo if defined($called) && $CarpInternal{$called};
                redo unless 0 > --$lvl;
                last;
            }
            else {
                return 0;
            }
        }
        redo if $Internal{$caller};
        redo if $CarpInternal{$caller};
        redo if $CarpInternal{$called};
        redo if trusts( $called, $caller, $cache );
        redo if trusts( $caller, $called, $cache );
        redo unless 0 > --$lvl;
    }
    return $i - 1;
}

sub shortmess_heavy {
    return longmess_heavy(@_) if $Verbose;
    return @_ if ref( $_[0] );    # don't break references as exceptions
    my $i = short_error_loc();
    if ($i) {
        ret_summary( $i, @_ );
    }
    else {
        longmess_heavy(@_);
    }
}

# If a string is too long, trims it with ...
sub str_len_trim {
    my $str = shift;
    my $max = shift || 0;
    if ( 2 < $max and $max < length($str) ) {
        substr( $str, $max - 3 ) = '...';
    }
    return $str;
}

# Takes two packages and an optional cache.  Says whether the
# first inherits from the second.
#
# Recursive versions of this have to work to avoid certain
# possible endless loops, and when following long chains of
# inheritance are less efficient.
sub trusts {
    my $child  = shift;
    my $parent = shift;
    my $cache  = shift;
    my ( $known, $partial ) = get_status( $cache, $child );

    # Figure out consequences until we have an answer
    while ( @$partial and not exists $known->{$parent} ) {
        my $anc = shift @$partial;
        next if exists $known->{$anc};
        $known->{$anc}++;
        my ( $anc_knows, $anc_partial ) = get_status( $cache, $anc );
        my @found = keys %$anc_knows;
        @$known{@found} = ();
        push @$partial, @$anc_partial;
    }
    return exists $known->{$parent};
}

# Takes a package and gives a list of those trusted directly
sub trusts_directly {
    my $class = shift;
    no strict 'refs';
    my $stash = \%{"$class\::"};
    for my $var (qw/ CARP_NOT ISA /) {
        # Don't try using the variable until we know it exists,
        # to avoid polluting the caller's namespace.
        if ( $stash->{$var} && ref \$stash->{$var} eq 'GLOB'
          && *{$stash->{$var}}{ARRAY} && @{$stash->{$var}} ) {
           return @{$stash->{$var}}
        }
    }
    return;
}

if(!defined($warnings::VERSION) ||
	do { no warnings "numeric"; $warnings::VERSION < 1.03 }) {
    # Very old versions of warnings.pm import from Carp.  This can go
    # wrong due to the circular dependency.  If Carp is invoked before
    # warnings, then Carp starts by loading warnings, then warnings
    # tries to import from Carp, and gets nothing because Carp is in
    # the process of loading and hasn't defined its import method yet.
    # So we work around that by manually exporting to warnings here.
    no strict "refs";
    *{"warnings::$_"} = \&$_ foreach @EXPORT;
}

1;

__END__

=head1 NAME

Carp - alternative warn and die for modules

=head1 SYNOPSIS

    use Carp;

    # warn user (from perspective of caller)
    carp "string trimmed to 80 chars";

    # die of errors (from perspective of caller)
    croak "We're outta here!";

    # die of errors with stack backtrace
    confess "not implemented";

    # cluck, longmess and shortmess not exported by default
    use Carp qw(cluck longmess shortmess);
    cluck "This is how we got here!"; # warn with stack backtrace
    my $long_message   = longmess( "message from cluck() or confess()" );
    my $short_message  = shortmess( "message from carp() or croak()" );

=head1 DESCRIPTION

The Carp routines are useful in your own modules because
they act like C<die()> or C<warn()>, but with a message which is more
likely to be useful to a user of your module.  In the case of
C<cluck()> and C<confess()>, that context is a summary of every
call in the call-stack; C<longmess()> returns the contents of the error
message.

For a shorter message you can use C<carp()> or C<croak()> which report the
error as being from where your module was called.  C<shortmess()> returns the
contents of this error message.  There is no guarantee that that is where the
error was, but it is a good educated guess.

C<Carp> takes care not to clobber the status variables C<$!> and C<$^E>
in the course of assembling its error messages.  This means that a
C<$SIG{__DIE__}> or C<$SIG{__WARN__}> handler can capture the error
information held in those variables, if it is required to augment the
error message, and if the code calling C<Carp> left useful values there.
Of course, C<Carp> can't guarantee the latter.

You can also alter the way the output and logic of C<Carp> works, by
changing some global variables in the C<Carp> namespace. See the
section on L</GLOBAL VARIABLES> below.

Here is a more complete description of how C<carp> and C<croak> work.
What they do is search the call-stack for a function call stack where
they have not been told that there shouldn't be an error.  If every
call is marked safe, they give up and give a full stack backtrace
instead.  In other words they presume that the first likely looking
potential suspect is guilty.  Their rules for telling whether
a call shouldn't generate errors work as follows:

=over 4

=item 1.

Any call from a package to itself is safe.

=item 2.

Packages claim that there won't be errors on calls to or from
packages explicitly marked as safe by inclusion in C<@CARP_NOT>, or
(if that array is empty) C<@ISA>.  The ability to override what
@ISA says is new in 5.8.

=item 3.

The trust in item 2 is transitive.  If A trusts B, and B
trusts C, then A trusts C.  So if you do not override C<@ISA>
with C<@CARP_NOT>, then this trust relationship is identical to,
"inherits from".

=item 4.

Any call from an internal Perl module is safe.  (Nothing keeps
user modules from marking themselves as internal to Perl, but
this practice is discouraged.)

=item 5.

Any call to Perl's warning system (eg Carp itself) is safe.
(This rule is what keeps it from reporting the error at the
point where you call C<carp> or C<croak>.)

=item 6.

C<$Carp::CarpLevel> can be set to skip a fixed number of additional
call levels.  Using this is not recommended because it is very
difficult to get it to behave correctly.

=back

=head2 Forcing a Stack Trace

As a debugging aid, you can force Carp to treat a croak as a confess
and a carp as a cluck across I<all> modules. In other words, force a
detailed stack trace to be given.  This can be very helpful when trying
to understand why, or from where, a warning or error is being generated.

This feature is enabled by 'importing' the non-existent symbol
'verbose'. You would typically enable it by saying

    perl -MCarp=verbose script.pl

or by including the string C<-MCarp=verbose> in the PERL5OPT
environment variable.

Alternately, you can set the global variable C<$Carp::Verbose> to true.
See the L</GLOBAL VARIABLES> section below.

=head2 Stack Trace formatting

At each stack level, the subroutine's name is displayed along with
its parameters.  For simple scalars, this is sufficient.  For complex
data types, such as objects and other references, this can simply
display C<'HASH(0x1ab36d8)'>.

Carp gives two ways to control this.

=over 4

=item 1.

For objects, a method, C<CARP_TRACE>, will be called, if it exists.  If
this method doesn't exist, or it recurses into C<Carp>, or it otherwise
throws an exception, this is skipped, and Carp moves on to the next option,
otherwise checking stops and the string returned is used.  It is recommended
that the object's type is part of the string to make debugging easier.

=item 2.

For any type of reference, C<$Carp::RefArgFormatter> is checked (see below).
This variable is expected to be a code reference, and the current parameter
is passed in.  If this function doesn't exist (the variable is undef), or
it recurses into C<Carp>, or it otherwise throws an exception, this is
skipped, and Carp moves on to the next option, otherwise checking stops
and the string returned is used.

=item 3.

Otherwise, if neither C<CARP_TRACE> nor C<$Carp::RefArgFormatter> is
available, stringify the value ignoring any overloading.

=back

=head1 GLOBAL VARIABLES

=head2 $Carp::MaxEvalLen

This variable determines how many characters of a string-eval are to
be shown in the output. Use a value of C<0> to show all text.

Defaults to C<0>.

=head2 $Carp::MaxArgLen

This variable determines how many characters of each argument to a
function to print. Use a value of C<0> to show the full length of the
argument.

Defaults to C<64>.

=head2 $Carp::MaxArgNums

This variable determines how many arguments to each function to show.
Use a false value to show all arguments to a function call.  To suppress all
arguments, use C<-1> or C<'0 but true'>.

Defaults to C<8>.

=head2 $Carp::Verbose

This variable makes C<carp()> and C<croak()> generate stack backtraces
just like C<cluck()> and C<confess()>.  This is how C<use Carp 'verbose'>
is implemented internally.

Defaults to C<0>.

=head2 $Carp::RefArgFormatter

This variable sets a general argument formatter to display references.
Plain scalars and objects that implement C<CARP_TRACE> will not go through
this formatter.  Calling C<Carp> from within this function is not supported.

    local $Carp::RefArgFormatter = sub {
        require Data::Dumper;
        Data::Dumper->Dump($_[0]); # not necessarily safe
    };

=head2 @CARP_NOT

This variable, I<in your package>, says which packages are I<not> to be
considered as the location of an error. The C<carp()> and C<cluck()>
functions will skip over callers when reporting where an error occurred.

NB: This variable must be in the package's symbol table, thus:

    # These work
    our @CARP_NOT; # file scope
    use vars qw(@CARP_NOT); # package scope
    @My::Package::CARP_NOT = ... ; # explicit package variable

    # These don't work
    sub xyz { ... @CARP_NOT = ... } # w/o declarations above
    my @CARP_NOT; # even at top-level

Example of use:

    package My::Carping::Package;
    use Carp;
    our @CARP_NOT;
    sub bar     { .... or _error('Wrong input') }
    sub _error  {
        # temporary control of where'ness, __PACKAGE__ is implicit
        local @CARP_NOT = qw(My::Friendly::Caller);
        carp(@_)
    }

This would make C<Carp> report the error as coming from a caller not
in C<My::Carping::Package>, nor from C<My::Friendly::Caller>.

Also read the L</DESCRIPTION> section above, about how C<Carp> decides
where the error is reported from.

Use C<@CARP_NOT>, instead of C<$Carp::CarpLevel>.

Overrides C<Carp>'s use of C<@ISA>.

=head2 %Carp::Internal

This says what packages are internal to Perl.  C<Carp> will never
report an error as being from a line in a package that is internal to
Perl.  For example:

    $Carp::Internal{ (__PACKAGE__) }++;
    # time passes...
    sub foo { ... or confess("whatever") };

would give a full stack backtrace starting from the first caller
outside of __PACKAGE__.  (Unless that package was also internal to
Perl.)

=head2 %Carp::CarpInternal

This says which packages are internal to Perl's warning system.  For
generating a full stack backtrace this is the same as being internal
to Perl, the stack backtrace will not start inside packages that are
listed in C<%Carp::CarpInternal>.  But it is slightly different for
the summary message generated by C<carp> or C<croak>.  There errors
will not be reported on any lines that are calling packages in
C<%Carp::CarpInternal>.

For example C<Carp> itself is listed in C<%Carp::CarpInternal>.
Therefore the full stack backtrace from C<confess> will not start
inside of C<Carp>, and the short message from calling C<croak> is
not placed on the line where C<croak> was called.

=head2 $Carp::CarpLevel

This variable determines how many additional call frames are to be
skipped that would not otherwise be when reporting where an error
occurred on a call to one of C<Carp>'s functions.  It is fairly easy
to count these call frames on calls that generate a full stack
backtrace.  However it is much harder to do this accounting for calls
that generate a short message.  Usually people skip too many call
frames.  If they are lucky they skip enough that C<Carp> goes all of
the way through the call stack, realizes that something is wrong, and
then generates a full stack backtrace.  If they are unlucky then the
error is reported from somewhere misleading very high in the call
stack.

Therefore it is best to avoid C<$Carp::CarpLevel>.  Instead use
C<@CARP_NOT>, C<%Carp::Internal> and C<%Carp::CarpInternal>.

Defaults to C<0>.

=head1 BUGS

The Carp routines don't handle exception objects currently.
If called with a first argument that is a reference, they simply
call die() or warn(), as appropriate.

=head1 SEE ALSO

L<Carp::Always>,
L<Carp::Clan>

=head1 CONTRIBUTING

L<Carp> is maintained by the perl 5 porters as part of the core perl 5
version control repository. Please see the L<perlhack> perldoc for how to
submit patches and contribute to it.

=head1 AUTHOR

The Carp module first appeared in Larry Wall's perl 5.000 distribution.
Since then it has been modified by several of the perl 5 porters.
Andrew Main (Zefram) <zefram@fysh.org> divested Carp into an independent
distribution.

=head1 COPYRIGHT

Copyright (C) 1994-2013 Larry Wall

Copyright (C) 2011, 2012, 2013 Andrew Main (Zefram) <zefram@fysh.org>

=head1 LICENSE

This module is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.
