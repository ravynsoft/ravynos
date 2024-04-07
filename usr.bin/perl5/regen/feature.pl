#!/usr/bin/perl
# 
# Regenerate (overwriting only if changed):
#
#    lib/feature.pm
#    feature.h
#
# from information hardcoded into this script and from two #defines
# in perl.h.
#
# This script is normally invoked from regen.pl.

BEGIN {
    push @INC, './lib';
    require './regen/regen_lib.pl';
    require './regen/HeaderParser.pm';
}

use strict;
use warnings;

###########################################################################
# Hand-editable data

# (feature name) => (internal name, used in %^H and macro names)
my %feature = (
    say                     => 'say',
    state                   => 'state',
    switch                  => 'switch',
    bitwise                 => 'bitwise',
    evalbytes               => 'evalbytes',
    current_sub             => '__SUB__',
    refaliasing             => 'refaliasing',
    postderef_qq            => 'postderef_qq',
    unicode_eval            => 'unieval',
    declared_refs           => 'myref',
    unicode_strings         => 'unicode',
    fc                      => 'fc',
    signatures              => 'signatures',
    isa                     => 'isa',
    indirect                => 'indirect',
    multidimensional        => 'multidimensional',
    bareword_filehandles    => 'bareword_filehandles',
    try                     => 'try',
    defer                   => 'defer',
    extra_paired_delimiters => 'more_delims',
    module_true             => 'module_true',
    class                   => 'class',
);

# NOTE: If a feature is ever enabled in a non-contiguous range of Perl
#       versions, any code below that uses %BundleRanges will have to
#       be changed to account.

# 5.odd implies the next 5.even, but an explicit 5.even can override it.

# features bundles
use constant V5_9_5 => sort qw{say state switch indirect multidimensional bareword_filehandles};
use constant V5_11  => sort ( +V5_9_5, qw{unicode_strings} );
use constant V5_15  => sort ( +V5_11, qw{unicode_eval evalbytes current_sub fc} );
use constant V5_23  => sort ( +V5_15, qw{postderef_qq} );
use constant V5_27  => sort ( +V5_23, qw{bitwise} );

use constant V5_35  => sort grep {; $_ ne 'switch'
                                 && $_ ne 'indirect'
                                 && $_ ne 'multidimensional' } +V5_27, qw{isa signatures};

use constant V5_37  => sort grep {; $_ ne 'bareword_filehandles' } +V5_35, qw{module_true};

#
# when updating features please also update the Pod entry for L</"FEATURES CHEAT SHEET">
#
my %feature_bundle = (
    all     => [ sort keys %feature ],
    default => [ qw{indirect multidimensional bareword_filehandles} ],
    # using 5.9.5 features bundle
    "5.9.5" => [ +V5_9_5 ],
    "5.10"  => [ +V5_9_5 ],
    # using 5.11 features bundle
    "5.11"  => [ +V5_11 ],
    "5.13"  => [ +V5_11 ],
    # using 5.15 features bundle
    "5.15"  => [ +V5_15 ],
    "5.17"  => [ +V5_15 ],
    "5.19"  => [ +V5_15 ],
    "5.21"  => [ +V5_15 ],
    # using 5.23 features bundle
    "5.23"  => [ +V5_23 ],
    "5.25"  => [ +V5_23 ],
    # using 5.27 features bundle
    "5.27"  => [ +V5_27 ],
    "5.29"  => [ +V5_27 ],
    "5.31"  => [ +V5_27 ],
    "5.33"  => [ +V5_27 ],
    # using 5.35 features bundle
    "5.35"  => [ +V5_35 ],
    # using 5.37 features bundle
    "5.37"  => [ +V5_37 ],
);

my @noops = qw( postderef lexical_subs );
my @removed = qw( array_base );


###########################################################################
# More data generated from the above

if (keys %feature > 32) {
    die "cop_features only has room for 32 features";
}

my %feature_bits;
my $mask = 1;
for my $feature (sort keys %feature) {
    $feature_bits{$feature} = $mask;
    $mask <<= 1;
}

for (keys %feature_bundle) {
    next unless /^5\.(\d*[13579])\z/;
    $feature_bundle{"5.".($1+1)} ||= $feature_bundle{$_};
}

my %UniqueBundles; # "say state switch" => 5.10
my %Aliases;       #  5.12 => 5.11
for( sort keys %feature_bundle ) {
    my $value = join(' ', sort @{$feature_bundle{$_}});
    if (exists $UniqueBundles{$value}) {
	$Aliases{$_} = $UniqueBundles{$value};
    }
    else {
	$UniqueBundles{$value} = $_;
    }
}
			   # start   end
my %BundleRanges; # say => ['5.10', '5.15'] # unique bundles for values
for my $bund (
    sort { $a eq 'default' ? -1 : $b eq 'default' ? 1 : $a cmp $b }
         values %UniqueBundles
) {
    next if $bund =~ /[^\d.]/ and $bund ne 'default';
    for (@{$feature_bundle{$bund}}) {
	if (@{$BundleRanges{$_} ||= []} == 2) {
	    $BundleRanges{$_}[1] = $bund
	}
	else {
	    push @{$BundleRanges{$_}}, $bund;
	}
    }
}

my $HintShift;
my $HintMask;
my $Uni8Bit;
my $hp = HeaderParser->new()->read_file("perl.h");

foreach my $line_data (@{$hp->lines}) {
    next unless $line_data->{type} eq "content"
            and $line_data->{sub_type} eq "#define";
    my $line = $line_data->{line};
    next unless $line=~/^\s*#\s*define\s+(HINT_FEATURE_MASK|HINT_UNI_8_BIT)/;
    my $is_u8b = $1 =~ 8;
    $line=~/(0x[A-Fa-f0-9]+)/ or die "No hex number in:\n\n$line\n ";
    if ($is_u8b) {
	$Uni8Bit = $1;
    }
    else {
	my $hex = $HintMask = $1;
	my $bits = sprintf "%b", oct $1;
	$bits =~ /^0*1+(0*)\z/
         or die "Non-contiguous bits in $bits (binary for $hex):\n\n$line\n ";
	$HintShift = length $1;
	my $bits_needed =
	    length sprintf "%b", scalar keys %UniqueBundles;
	$bits =~ /1{$bits_needed}/
	    or die "Not enough bits (need $bits_needed)"
                 . " in $bits (binary for $hex):\n\n$line\n ";
    }
    if ($Uni8Bit && $HintMask) { last }
}
die "No HINT_FEATURE_MASK defined in perl.h" unless $HintMask;
die "No HINT_UNI_8_BIT defined in perl.h"    unless $Uni8Bit;

my @HintedBundles =
    ('default', grep !/[^\d.]/, sort values %UniqueBundles);


###########################################################################
# Open files to be generated

my ($pm, $h) = map {
    open_new($_, '>', { by => 'regen/feature.pl' });
} 'lib/feature.pm', 'feature.h';


###########################################################################
# Generate lib/feature.pm

while (<DATA>) {
    last if /^FEATURES$/ ;
    print $pm $_ ;
}

sub longest {
    my $long;
    for(@_) {
	if (!defined $long or length $long < length) {
	    $long = $_;
	}
    }
    $long;
}

print $pm "our %feature = (\n";
my $width = length longest keys %feature;
for(sort { length $a <=> length $b || $a cmp $b } keys %feature) {
    print $pm "    $_" . " "x($width-length)
	    . " => 'feature_$feature{$_}',\n";
}
print $pm ");\n\n";

print $pm "our %feature_bundle = (\n";
my $bund_width = length longest values %UniqueBundles;
for( sort { $UniqueBundles{$a} cmp $UniqueBundles{$b} }
          keys %UniqueBundles ) {
    my $bund = $UniqueBundles{$_};
    print $pm qq'    "$bund"' . " "x($bund_width-length $bund)
	    . qq' => [qw($_)],\n';
}
print $pm ");\n\n";

for (sort keys %Aliases) {
    print $pm
	qq'\$feature_bundle{"$_"} = \$feature_bundle{"$Aliases{$_}"};\n';
};

print $pm "my \%noops = (\n";
print $pm "    $_ => 1,\n", for @noops;
print $pm ");\n";

print $pm "my \%removed = (\n";
print $pm "    $_ => 1,\n", for @removed;
print $pm ");\n";

print $pm <<EOPM;

our \$hint_shift   = $HintShift;
our \$hint_mask    = $HintMask;
our \@hint_bundles = qw( @HintedBundles );

# This gets set (for now) in \$^H as well as in %^H,
# for runtime speed of the uc/lc/ucfirst/lcfirst functions.
# See HINT_UNI_8_BIT in perl.h.
our \$hint_uni8bit = $Uni8Bit;
EOPM


while (<DATA>) {
    last if /^PODTURES$/ ;
    print $pm $_ ;
}

select +(select($pm), $~ = 'PODTURES')[0];
format PODTURES =
  ^<<<<<<<< ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<~~
$::bundle, $::feature
.

for ('default', sort grep /\.\d[02468]/, keys %feature_bundle) {
    $::bundle = ":$_";
    $::feature = join ' ', @{$feature_bundle{$_}};
    write $pm;
    print $pm "\n";
}

while (<DATA>) {
    print $pm $_ ;
}

read_only_bottom_close_and_rename($pm);


###########################################################################
# Generate feature.h

print $h <<EOH;

#ifndef PERL_FEATURE_H_
#define PERL_FEATURE_H_

#if defined(PERL_CORE) || defined (PERL_EXT)

#define HINT_FEATURE_SHIFT	$HintShift

EOH

for (sort keys %feature_bits) {
    printf $h "#define FEATURE_%s_BIT%*s %#06x\n", uc($feature{$_}),
      $width-length($feature{$_}), "", $feature_bits{$_};
}
print $h "\n";

my $count;
for (@HintedBundles) {
    (my $key = uc) =~ y/.//d;
    print $h "#define FEATURE_BUNDLE_$key	", $count++, "\n";
}

print $h <<'EOH';
#define FEATURE_BUNDLE_CUSTOM	(HINT_FEATURE_MASK >> HINT_FEATURE_SHIFT)

/* this is preserved for testing and asserts */
#define OLD_CURRENT_HINTS \
    (PL_curcop == &PL_compiling ? PL_hints : PL_curcop->cop_hints)
/* this is the same thing, but simpler (no if) as PL_hints expands
   to PL_compiling.cop_hints */
#define CURRENT_HINTS \
    PL_curcop->cop_hints
#define CURRENT_FEATURE_BUNDLE \
    ((CURRENT_HINTS & HINT_FEATURE_MASK) >> HINT_FEATURE_SHIFT)

#define FEATURE_IS_ENABLED_MASK(mask)                   \
  ((CURRENT_HINTS & HINT_LOCALIZE_HH)                \
    ? (PL_curcop->cop_features & (mask)) : FALSE)

/* The longest string we pass in.  */
EOH

my $longest_internal_feature_name = longest values %feature;
print $h <<EOL;
#define MAX_FEATURE_LEN (sizeof("$longest_internal_feature_name")-1)

EOL

for (
    sort { length $a <=> length $b || $a cmp $b } keys %feature
) {
    my($first,$last) =
	map { (my $__ = uc) =~ y/.//d; $__ } @{$BundleRanges{$_}};
    my $name = $feature{$_};
    my $NAME = uc $name;
    if ($last && $first eq 'DEFAULT') { #  '>= DEFAULT' warns
	print $h <<EOI;
#define FEATURE_${NAME}_IS_ENABLED \\
    ( \\
	CURRENT_FEATURE_BUNDLE <= FEATURE_BUNDLE_$last \\
     || (CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_CUSTOM && \\
	 FEATURE_IS_ENABLED_MASK(FEATURE_${NAME}_BIT)) \\
    )

EOI
    }
    elsif ($last) {
	print $h <<EOH3;
#define FEATURE_${NAME}_IS_ENABLED \\
    ( \\
	(CURRENT_FEATURE_BUNDLE >= FEATURE_BUNDLE_$first && \\
	 CURRENT_FEATURE_BUNDLE <= FEATURE_BUNDLE_$last) \\
     || (CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_CUSTOM && \\
	 FEATURE_IS_ENABLED_MASK(FEATURE_${NAME}_BIT)) \\
    )

EOH3
    }
    elsif ($first) {
	print $h <<EOH4;
#define FEATURE_${NAME}_IS_ENABLED \\
    ( \\
	CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_$first \\
     || (CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_CUSTOM && \\
	 FEATURE_IS_ENABLED_MASK(FEATURE_${NAME}_BIT)) \\
    )

EOH4
    }
    else {
	print $h <<EOH5;
#define FEATURE_${NAME}_IS_ENABLED \\
    ( \\
	CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_CUSTOM && \\
	 FEATURE_IS_ENABLED_MASK(FEATURE_${NAME}_BIT) \\
    )

EOH5
    }
}

print $h <<EOH;

#define SAVEFEATUREBITS() SAVEI32(PL_compiling.cop_features)

#define CLEARFEATUREBITS() (PL_compiling.cop_features = 0)

#define STOREFEATUREBITSHH(hh) \\
  (hv_stores((hh), "feature/bits", newSVuv(PL_compiling.cop_features)))

#define FETCHFEATUREBITSHH(hh)                              \\
  STMT_START {                                              \\
      SV **fbsv = hv_fetchs((hh), "feature/bits", FALSE);   \\
      PL_compiling.cop_features = fbsv ? SvUV(*fbsv) : 0;   \\
  } STMT_END

#endif /* PERL_CORE or PERL_EXT */

#ifdef PERL_IN_OP_C
PERL_STATIC_INLINE void
S_enable_feature_bundle(pTHX_ SV *ver)
{
    SV *comp_ver = sv_newmortal();
    PL_hints = (PL_hints &~ HINT_FEATURE_MASK)
	     | (
EOH

for (reverse @HintedBundles[1..$#HintedBundles]) { # skip default
    my $numver = $_;
    if ($numver eq '5.10') { $numver = '5.009005' } # special case
    else		   { $numver =~ s/\./.0/  } # 5.11 => 5.011
    (my $macrover = $_) =~ y/.//d;
    print $h <<"    EOK";
		  (sv_setnv(comp_ver, $numver),
		   vcmp(ver, upg_version(comp_ver, FALSE)) >= 0)
			? FEATURE_BUNDLE_$macrover :
    EOK
}

print $h <<EOJ;
			  FEATURE_BUNDLE_DEFAULT
	       ) << HINT_FEATURE_SHIFT;
    /* special case */
    assert(PL_curcop == &PL_compiling);
    if (FEATURE_UNICODE_IS_ENABLED) PL_hints |=  HINT_UNI_8_BIT;
    else			    PL_hints &= ~HINT_UNI_8_BIT;
}
#endif /* PERL_IN_OP_C */

#ifdef PERL_IN_MG_C

#define magic_sethint_feature(keysv, keypv, keylen, valsv, valbool) \\
    S_magic_sethint_feature(aTHX_ (keysv), (keypv), (keylen), (valsv), (valbool))
PERL_STATIC_INLINE void
S_magic_sethint_feature(pTHX_ SV *keysv, const char *keypv, STRLEN keylen,
                        SV *valsv, bool valbool) {
    if (keysv)
      keypv = SvPV_const(keysv, keylen);

    if (memBEGINs(keypv, keylen, "feature_")) {
        const char *subf = keypv + (sizeof("feature_")-1);
        U32 mask = 0;
        switch (*subf) {
EOJ

my %pref;
for my $key (sort values %feature) {
    push @{$pref{substr($key, 0, 1)}}, $key;
}

for my $pref (sort keys %pref) {
    print $h <<EOS;
        case '$pref':
EOS
    my $first = 1;
    for my $subkey (@{$pref{$pref}}) {
        my $rest = substr($subkey, 1);
        my $if = $first ? "if" : "else if";
        print $h <<EOJ;
            $if (keylen == sizeof("feature_$subkey")-1
                 && memcmp(subf+1, "$rest", keylen - sizeof("feature_")) == 0) {
                mask = FEATURE_\U${subkey}\E_BIT;
                break;
            }
EOJ

        $first = 0;
    }
    print $h <<EOS;
            return;

EOS
}

print $h <<EOJ;
        default:
            return;
        }
        if (valsv ? SvTRUE(valsv) : valbool)
            PL_compiling.cop_features |= mask;
        else
            PL_compiling.cop_features &= ~mask;
    }
}
#endif /* PERL_IN_MG_C */

#endif /* PERL_FEATURE_H_ */
EOJ

read_only_bottom_close_and_rename($h);


###########################################################################
# Template for feature.pm

__END__
package feature;
our $VERSION = '1.82';

FEATURES

# TODO:
# - think about versioned features (use feature switch => 2)

=encoding utf8

=head1 NAME

feature - Perl pragma to enable new features

=head1 SYNOPSIS

    use feature qw(fc say);

    # Without the "use feature" above, this code would not be able to find
    # the built-ins "say" or "fc":
    say "The case-folded version of $x is: " . fc $x;


    # set features to match the :5.36 bundle, which may turn off or on
    # multiple features (see "FEATURE BUNDLES" below)
    use feature ':5.36';


    # implicitly loads :5.36 feature bundle
    use v5.36;

=head1 DESCRIPTION

It is usually impossible to add new syntax to Perl without breaking
some existing programs.  This pragma provides a way to minimize that
risk. New syntactic constructs, or new semantic meanings to older
constructs, can be enabled by C<use feature 'foo'>, and will be parsed
only when the appropriate feature pragma is in scope.  (Nevertheless, the
C<CORE::> prefix provides access to all Perl keywords, regardless of this
pragma.)

=head2 Lexical effect

Like other pragmas (C<use strict>, for example), features have a lexical
effect.  C<use feature qw(foo)> will only make the feature "foo" available
from that point to the end of the enclosing block.

    {
        use feature 'say';
        say "say is available here";
    }
    print "But not here.\n";

=head2 C<no feature>

Features can also be turned off by using C<no feature "foo">.  This too
has lexical effect.

    use feature 'say';
    say "say is available here";
    {
        no feature 'say';
        print "But not here.\n";
    }
    say "Yet it is here.";

C<no feature> with no features specified will reset to the default group.  To
disable I<all> features (an unusual request!) use C<no feature ':all'>.

=head1 AVAILABLE FEATURES

Read L</"FEATURE BUNDLES"> for the feature cheat sheet summary.

=head2 The 'say' feature

C<use feature 'say'> tells the compiler to enable the Raku-inspired
C<say> function.

See L<perlfunc/say> for details.

This feature is available starting with Perl 5.10.

=head2 The 'state' feature

C<use feature 'state'> tells the compiler to enable C<state>
variables.

See L<perlsub/"Persistent Private Variables"> for details.

This feature is available starting with Perl 5.10.

=head2 The 'switch' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::smartmatch";

C<use feature 'switch'> tells the compiler to enable the Raku
given/when construct.

See L<perlsyn/"Switch Statements"> for details.

This feature is available starting with Perl 5.10.
It is deprecated starting with Perl 5.38, and using
C<given>, C<when> or smartmatch will throw a warning.
It will be removed in Perl 5.42.

=head2 The 'unicode_strings' feature

C<use feature 'unicode_strings'> tells the compiler to use Unicode rules
in all string operations executed within its scope (unless they are also
within the scope of either C<use locale> or C<use bytes>).  The same applies
to all regular expressions compiled within the scope, even if executed outside
it.  It does not change the internal representation of strings, but only how
they are interpreted.

C<no feature 'unicode_strings'> tells the compiler to use the traditional
Perl rules wherein the native character set rules is used unless it is
clear to Perl that Unicode is desired.  This can lead to some surprises
when the behavior suddenly changes.  (See
L<perlunicode/The "Unicode Bug"> for details.)  For this reason, if you are
potentially using Unicode in your program, the
C<use feature 'unicode_strings'> subpragma is B<strongly> recommended.

This feature is available starting with Perl 5.12; was almost fully
implemented in Perl 5.14; and extended in Perl 5.16 to cover C<quotemeta>;
was extended further in Perl 5.26 to cover L<the range
operator|perlop/Range Operators>; and was extended again in Perl 5.28 to
cover L<special-cased whitespace splitting|perlfunc/split>.

=head2 The 'unicode_eval' and 'evalbytes' features

Together, these two features are intended to replace the legacy string
C<eval> function, which behaves problematically in some instances.  They are
available starting with Perl 5.16, and are enabled by default by a
S<C<use 5.16>> or higher declaration.

C<unicode_eval> changes the behavior of plain string C<eval> to work more
consistently, especially in the Unicode world.  Certain (mis)behaviors
couldn't be changed without breaking some things that had come to rely on
them, so the feature can be enabled and disabled.  Details are at
L<perlfunc/Under the "unicode_eval" feature>.

C<evalbytes> is like string C<eval>, but it treats its argument as a byte
string. Details are at L<perlfunc/evalbytes EXPR>.  Without a
S<C<use feature 'evalbytes'>> nor a S<C<use v5.16>> (or higher) declaration in
the current scope, you can still access it by instead writing
C<CORE::evalbytes>.

=head2 The 'current_sub' feature

This provides the C<__SUB__> token that returns a reference to the current
subroutine or C<undef> outside of a subroutine.

This feature is available starting with Perl 5.16.

=head2 The 'array_base' feature

This feature supported the legacy C<$[> variable.  See L<perlvar/$[>.
It was on by default but disabled under C<use v5.16> (see
L</IMPLICIT LOADING>, below) and unavailable since perl 5.30.

This feature is available under this name starting with Perl 5.16.  In
previous versions, it was simply on all the time, and this pragma knew
nothing about it.

=head2 The 'fc' feature

C<use feature 'fc'> tells the compiler to enable the C<fc> function,
which implements Unicode casefolding.

See L<perlfunc/fc> for details.

This feature is available from Perl 5.16 onwards.

=head2 The 'lexical_subs' feature

In Perl versions prior to 5.26, this feature enabled
declaration of subroutines via C<my sub foo>, C<state sub foo>
and C<our sub foo> syntax.  See L<perlsub/Lexical Subroutines> for details.

This feature is available from Perl 5.18 onwards.  From Perl 5.18 to 5.24,
it was classed as experimental, and Perl emitted a warning for its
usage, except when explicitly disabled:

  no warnings "experimental::lexical_subs";

As of Perl 5.26, use of this feature no longer triggers a warning, though
the C<experimental::lexical_subs> warning category still exists (for
compatibility with code that disables it).  In addition, this syntax is
not only no longer experimental, but it is enabled for all Perl code,
regardless of what feature declarations are in scope.

=head2 The 'postderef' and 'postderef_qq' features

The 'postderef_qq' feature extends the applicability of L<postfix
dereference syntax|perlref/Postfix Dereference Syntax> so that
postfix array dereference, postfix scalar dereference, and
postfix array highest index access are available in double-quotish interpolations.
For example, it makes the following two statements equivalent:

  my $s = "[@{ $h->{a} }]";
  my $s = "[$h->{a}->@*]";

This feature is available from Perl 5.20 onwards. In Perl 5.20 and 5.22, it
was classed as experimental, and Perl emitted a warning for its
usage, except when explicitly disabled:

  no warnings "experimental::postderef";

As of Perl 5.24, use of this feature no longer triggers a warning, though
the C<experimental::postderef> warning category still exists (for
compatibility with code that disables it).

The 'postderef' feature was used in Perl 5.20 and Perl 5.22 to enable
postfix dereference syntax outside double-quotish interpolations. In those
versions, using it triggered the C<experimental::postderef> warning in the
same way as the 'postderef_qq' feature did. As of Perl 5.24, this syntax is
not only no longer experimental, but it is enabled for all Perl code,
regardless of what feature declarations are in scope.

=head2 The 'signatures' feature

This enables syntax for declaring subroutine arguments as lexical variables.
For example, for this subroutine:

    sub foo ($left, $right) {
        return $left + $right;
    }

Calling C<foo(3, 7)> will assign C<3> into C<$left> and C<7> into C<$right>.

See L<perlsub/Signatures> for details.

This feature is available from Perl 5.20 onwards. From Perl 5.20 to 5.34,
it was classed as experimental, and Perl emitted a warning for its usage,
except when explicitly disabled:

  no warnings "experimental::signatures";

As of Perl 5.36, use of this feature no longer triggers a warning, though the
C<experimental::signatures> warning category still exists (for compatibility
with code that disables it). This feature is now considered stable, and is
enabled automatically by C<use v5.36> (or higher).

=head2 The 'refaliasing' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::refaliasing";

This enables aliasing via assignment to references:

    \$a = \$b; # $a and $b now point to the same scalar
    \@a = \@b; #                     to the same array
    \%a = \%b;
    \&a = \&b;
    foreach \%hash (@array_of_hash_refs) {
        ...
    }

See L<perlref/Assigning to References> for details.

This feature is available from Perl 5.22 onwards.

=head2 The 'bitwise' feature

This makes the four standard bitwise operators (C<& | ^ ~>) treat their
operands consistently as numbers, and introduces four new dotted operators
(C<&. |. ^. ~.>) that treat their operands consistently as strings.  The
same applies to the assignment variants (C<&= |= ^= &.= |.= ^.=>).

See L<perlop/Bitwise String Operators> for details.

This feature is available from Perl 5.22 onwards.  Starting in Perl 5.28,
C<use v5.28> will enable the feature.  Before 5.28, it was still
experimental and would emit a warning in the "experimental::bitwise"
category.

=head2 The 'declared_refs' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::declared_refs";

This allows a reference to a variable to be declared with C<my>, C<state>,
or C<our>, or localized with C<local>.  It is intended mainly for use in
conjunction with the "refaliasing" feature.  See L<perlref/Declaring a
Reference to a Variable> for examples.

This feature is available from Perl 5.26 onwards.

=head2 The 'isa' feature

This allows the use of the C<isa> infix operator, which tests whether the
scalar given by the left operand is an object of the class given by the
right operand. See L<perlop/Class Instance Operator> for more details.

This feature is available from Perl 5.32 onwards.  From Perl 5.32 to 5.34,
it was classed as experimental, and Perl emitted a warning for its usage,
except when explicitly disabled:

    no warnings "experimental::isa";

As of Perl 5.36, use of this feature no longer triggers a warning (though the
C<experimental::isa> warning category stilll exists for compatibility with
code that disables it). This feature is now considered stable, and is enabled
automatically by C<use v5.36> (or higher).

=head2 The 'indirect' feature

This feature allows the use of L<indirect object
syntax|perlobj/Indirect Object Syntax> for method calls, e.g.  C<new
Foo 1, 2;>. It is enabled by default, but can be turned off to
disallow indirect object syntax.

This feature is available under this name from Perl 5.32 onwards. In
previous versions, it was simply on all the time.  To disallow (or
warn on) indirect object syntax on older Perls, see the L<indirect>
CPAN module.

=head2 The 'multidimensional' feature

This feature enables multidimensional array emulation, a perl 4 (or
earlier) feature that was used to emulate multidimensional arrays with
hashes.  This works by converting code like C<< $foo{$x, $y} >> into
C<< $foo{join($;, $x, $y)} >>.  It is enabled by default, but can be
turned off to disable multidimensional array emulation.

When this feature is disabled the syntax that is normally replaced
will report a compilation error.

This feature is available under this name from Perl 5.34 onwards. In
previous versions, it was simply on all the time.

You can use the L<multidimensional> module on CPAN to disable
multidimensional array emulation for older versions of Perl.

=head2 The 'bareword_filehandles' feature

This feature enables bareword filehandles for builtin functions
operations, a generally discouraged practice.  It is enabled by
default, but can be turned off to disable bareword filehandles, except
for the exceptions listed below.

The perl built-in filehandles C<STDIN>, C<STDOUT>, C<STDERR>, C<DATA>,
C<ARGV>, C<ARGVOUT> and the special C<_> are always enabled.

This feature is enabled under this name from Perl 5.34 onwards.  In
previous versions it was simply on all the time.

You can use the L<bareword::filehandles> module on CPAN to disable
bareword filehandles for older versions of perl.

=head2 The 'try' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::try";

This feature enables the C<try> and C<catch> syntax, which allows exception
handling, where exceptions thrown from the body of the block introduced with
C<try> are caught by executing the body of the C<catch> block.

For more information, see L<perlsyn/"Try Catch Exception Handling">.

=head2 The 'defer' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::defer";

This feature enables the C<defer> block syntax, which allows a block of code
to be deferred until when the flow of control leaves the block which contained
it. For more details, see L<perlsyn/defer>.

=head2 The 'extra_paired_delimiters' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::extra_paired_delimiters";

This feature enables the use of more paired string delimiters than the
traditional four, S<C<< <  > >>>, S<C<( )>>, S<C<{ }>>, and S<C<[ ]>>.  When
this feature is on, for example, you can say S<C<qrE<171>patE<187>>>.

As with any usage of non-ASCII delimiters in a UTF-8-encoded source file, you
will want to ensure the parser will decode the source code from UTF-8 bytes
with a declaration such as C<use utf8>.

This feature is available starting in Perl 5.36.

The complete list of accepted paired delimiters as of Unicode 14.0 is:

 (  )    U+0028, U+0029   LEFT/RIGHT PARENTHESIS
 <  >    U+003C, U+003E   LESS-THAN/GREATER-THAN SIGN
 [  ]    U+005B, U+005D   LEFT/RIGHT SQUARE BRACKET
 {  }    U+007B, U+007D   LEFT/RIGHT CURLY BRACKET
 «  »    U+00AB, U+00BB   LEFT/RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
 »  «    U+00BB, U+00AB   RIGHT/LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
 ܆  ܇    U+0706, U+0707   SYRIAC COLON SKEWED LEFT/RIGHT
 ༺  ༻    U+0F3A, U+0F3B   TIBETAN MARK GUG RTAGS GYON,  TIBETAN MARK GUG
                          RTAGS GYAS
 ༼  ༽    U+0F3C, U+0F3D   TIBETAN MARK ANG KHANG GYON,  TIBETAN MARK ANG
                          KHANG GYAS
 ᚛  ᚜    U+169B, U+169C   OGHAM FEATHER MARK,  OGHAM REVERSED FEATHER MARK
 ‘  ’    U+2018, U+2019   LEFT/RIGHT SINGLE QUOTATION MARK
 ’  ‘    U+2019, U+2018   RIGHT/LEFT SINGLE QUOTATION MARK
 “  ”    U+201C, U+201D   LEFT/RIGHT DOUBLE QUOTATION MARK
 ”  “    U+201D, U+201C   RIGHT/LEFT DOUBLE QUOTATION MARK
 ‵  ′    U+2035, U+2032   REVERSED PRIME,  PRIME
 ‶  ″    U+2036, U+2033   REVERSED DOUBLE PRIME,  DOUBLE PRIME
 ‷  ‴    U+2037, U+2034   REVERSED TRIPLE PRIME,  TRIPLE PRIME
 ‹  ›    U+2039, U+203A   SINGLE LEFT/RIGHT-POINTING ANGLE QUOTATION MARK
 ›  ‹    U+203A, U+2039   SINGLE RIGHT/LEFT-POINTING ANGLE QUOTATION MARK
 ⁅  ⁆    U+2045, U+2046   LEFT/RIGHT SQUARE BRACKET WITH QUILL
 ⁍  ⁌    U+204D, U+204C   BLACK RIGHT/LEFTWARDS BULLET
 ⁽  ⁾    U+207D, U+207E   SUPERSCRIPT LEFT/RIGHT PARENTHESIS
 ₍  ₎    U+208D, U+208E   SUBSCRIPT LEFT/RIGHT PARENTHESIS
 →  ←    U+2192, U+2190   RIGHT/LEFTWARDS ARROW
 ↛  ↚    U+219B, U+219A   RIGHT/LEFTWARDS ARROW WITH STROKE
 ↝  ↜    U+219D, U+219C   RIGHT/LEFTWARDS WAVE ARROW
 ↠  ↞    U+21A0, U+219E   RIGHT/LEFTWARDS TWO HEADED ARROW
 ↣  ↢    U+21A3, U+21A2   RIGHT/LEFTWARDS ARROW WITH TAIL
 ↦  ↤    U+21A6, U+21A4   RIGHT/LEFTWARDS ARROW FROM BAR
 ↪  ↩    U+21AA, U+21A9   RIGHT/LEFTWARDS ARROW WITH HOOK
 ↬  ↫    U+21AC, U+21AB   RIGHT/LEFTWARDS ARROW WITH LOOP
 ↱  ↰    U+21B1, U+21B0   UPWARDS ARROW WITH TIP RIGHT/LEFTWARDS
 ↳  ↲    U+21B3, U+21B2   DOWNWARDS ARROW WITH TIP RIGHT/LEFTWARDS
 ⇀  ↼    U+21C0, U+21BC   RIGHT/LEFTWARDS HARPOON WITH BARB UPWARDS
 ⇁  ↽    U+21C1, U+21BD   RIGHT/LEFTWARDS HARPOON WITH BARB DOWNWARDS
 ⇉  ⇇    U+21C9, U+21C7   RIGHT/LEFTWARDS PAIRED ARROWS
 ⇏  ⇍    U+21CF, U+21CD   RIGHT/LEFTWARDS DOUBLE ARROW WITH STROKE
 ⇒  ⇐    U+21D2, U+21D0   RIGHT/LEFTWARDS DOUBLE ARROW
 ⇛  ⇚    U+21DB, U+21DA   RIGHT/LEFTWARDS TRIPLE ARROW
 ⇝  ⇜    U+21DD, U+21DC   RIGHT/LEFTWARDS SQUIGGLE ARROW
 ⇢  ⇠    U+21E2, U+21E0   RIGHT/LEFTWARDS DASHED ARROW
 ⇥  ⇤    U+21E5, U+21E4   RIGHT/LEFTWARDS ARROW TO BAR
 ⇨  ⇦    U+21E8, U+21E6   RIGHT/LEFTWARDS WHITE ARROW
 ⇴  ⬰    U+21F4, U+2B30   RIGHT/LEFT ARROW WITH SMALL CIRCLE
 ⇶  ⬱    U+21F6, U+2B31   THREE RIGHT/LEFTWARDS ARROWS
 ⇸  ⇷    U+21F8, U+21F7   RIGHT/LEFTWARDS ARROW WITH VERTICAL STROKE
 ⇻  ⇺    U+21FB, U+21FA   RIGHT/LEFTWARDS ARROW WITH DOUBLE VERTICAL
                          STROKE
 ⇾  ⇽    U+21FE, U+21FD   RIGHT/LEFTWARDS OPEN-HEADED ARROW
 ∈  ∋    U+2208, U+220B   ELEMENT OF,  CONTAINS AS MEMBER
 ∉  ∌    U+2209, U+220C   NOT AN ELEMENT OF,  DOES NOT CONTAIN AS MEMBER
 ∊  ∍    U+220A, U+220D   SMALL ELEMENT OF,  SMALL CONTAINS AS MEMBER
 ≤  ≥    U+2264, U+2265   LESS-THAN/GREATER-THAN OR EQUAL TO
 ≦  ≧    U+2266, U+2267   LESS-THAN/GREATER-THAN OVER EQUAL TO
 ≨  ≩    U+2268, U+2269   LESS-THAN/GREATER-THAN BUT NOT EQUAL TO
 ≪  ≫    U+226A, U+226B   MUCH LESS-THAN/GREATER-THAN
 ≮  ≯    U+226E, U+226F   NOT LESS-THAN/GREATER-THAN
 ≰  ≱    U+2270, U+2271   NEITHER LESS-THAN/GREATER-THAN NOR EQUAL TO
 ≲  ≳    U+2272, U+2273   LESS-THAN/GREATER-THAN OR EQUIVALENT TO
 ≴  ≵    U+2274, U+2275   NEITHER LESS-THAN/GREATER-THAN NOR EQUIVALENT TO
 ≺  ≻    U+227A, U+227B   PRECEDES/SUCCEEDS
 ≼  ≽    U+227C, U+227D   PRECEDES/SUCCEEDS OR EQUAL TO
 ≾  ≿    U+227E, U+227F   PRECEDES/SUCCEEDS OR EQUIVALENT TO
 ⊀  ⊁    U+2280, U+2281   DOES NOT PRECEDE/SUCCEED
 ⊂  ⊃    U+2282, U+2283   SUBSET/SUPERSET OF
 ⊄  ⊅    U+2284, U+2285   NOT A SUBSET/SUPERSET OF
 ⊆  ⊇    U+2286, U+2287   SUBSET/SUPERSET OF OR EQUAL TO
 ⊈  ⊉    U+2288, U+2289   NEITHER A SUBSET/SUPERSET OF NOR EQUAL TO
 ⊊  ⊋    U+228A, U+228B   SUBSET/SUPERSET OF WITH NOT EQUAL TO
 ⊣  ⊢    U+22A3, U+22A2   LEFT/RIGHT TACK
 ⊦  ⫞    U+22A6, U+2ADE   ASSERTION,  SHORT LEFT TACK
 ⊨  ⫤    U+22A8, U+2AE4   TRUE,  VERTICAL BAR DOUBLE LEFT TURNSTILE
 ⊩  ⫣    U+22A9, U+2AE3   FORCES,  DOUBLE VERTICAL BAR LEFT TURNSTILE
 ⊰  ⊱    U+22B0, U+22B1   PRECEDES/SUCCEEDS UNDER RELATION
 ⋐  ⋑    U+22D0, U+22D1   DOUBLE SUBSET/SUPERSET
 ⋖  ⋗    U+22D6, U+22D7   LESS-THAN/GREATER-THAN WITH DOT
 ⋘  ⋙    U+22D8, U+22D9   VERY MUCH LESS-THAN/GREATER-THAN
 ⋜  ⋝    U+22DC, U+22DD   EQUAL TO OR LESS-THAN/GREATER-THAN
 ⋞  ⋟    U+22DE, U+22DF   EQUAL TO OR PRECEDES/SUCCEEDS
 ⋠  ⋡    U+22E0, U+22E1   DOES NOT PRECEDE/SUCCEED OR EQUAL
 ⋦  ⋧    U+22E6, U+22E7   LESS-THAN/GREATER-THAN BUT NOT EQUIVALENT TO
 ⋨  ⋩    U+22E8, U+22E9   PRECEDES/SUCCEEDS BUT NOT EQUIVALENT TO
 ⋲  ⋺    U+22F2, U+22FA   ELEMENT OF/CONTAINS WITH LONG HORIZONTAL STROKE
 ⋳  ⋻    U+22F3, U+22FB   ELEMENT OF/CONTAINS WITH VERTICAL BAR AT END OF
                          HORIZONTAL STROKE
 ⋴  ⋼    U+22F4, U+22FC   SMALL ELEMENT OF/CONTAINS WITH VERTICAL BAR AT
                          END OF HORIZONTAL STROKE
 ⋶  ⋽    U+22F6, U+22FD   ELEMENT OF/CONTAINS WITH OVERBAR
 ⋷  ⋾    U+22F7, U+22FE   SMALL ELEMENT OF/CONTAINS WITH OVERBAR
 ⌈  ⌉    U+2308, U+2309   LEFT/RIGHT CEILING
 ⌊  ⌋    U+230A, U+230B   LEFT/RIGHT FLOOR
 ⌦  ⌫    U+2326, U+232B   ERASE TO THE RIGHT/LEFT
 〈 〉   U+2329, U+232A   LEFT/RIGHT-POINTING ANGLE BRACKET
 ⍈  ⍇    U+2348, U+2347   APL FUNCTIONAL SYMBOL QUAD RIGHT/LEFTWARDS ARROW
 ⏩ ⏪   U+23E9, U+23EA   BLACK RIGHT/LEFT-POINTING DOUBLE TRIANGLE
 ⏭  ⏮    U+23ED, U+23EE   BLACK RIGHT/LEFT-POINTING DOUBLE TRIANGLE WITH
                          VERTICAL BAR
 ☛  ☚    U+261B, U+261A   BLACK RIGHT/LEFT POINTING INDEX
 ☞  ☜    U+261E, U+261C   WHITE RIGHT/LEFT POINTING INDEX
 ⚞  ⚟    U+269E, U+269F   THREE LINES CONVERGING RIGHT/LEFT
 ❨  ❩    U+2768, U+2769   MEDIUM LEFT/RIGHT PARENTHESIS ORNAMENT
 ❪  ❫    U+276A, U+276B   MEDIUM FLATTENED LEFT/RIGHT PARENTHESIS ORNAMENT
 ❬  ❭    U+276C, U+276D   MEDIUM LEFT/RIGHT-POINTING ANGLE BRACKET
                          ORNAMENT
 ❮  ❯    U+276E, U+276F   HEAVY LEFT/RIGHT-POINTING ANGLE QUOTATION MARK
                          ORNAMENT
 ❰  ❱    U+2770, U+2771   HEAVY LEFT/RIGHT-POINTING ANGLE BRACKET ORNAMENT
 ❲  ❳    U+2772, U+2773   LIGHT LEFT/RIGHT TORTOISE SHELL BRACKET ORNAMENT
 ❴  ❵    U+2774, U+2775   MEDIUM LEFT/RIGHT CURLY BRACKET ORNAMENT
 ⟃  ⟄    U+27C3, U+27C4   OPEN SUBSET/SUPERSET
 ⟅  ⟆    U+27C5, U+27C6   LEFT/RIGHT S-SHAPED BAG DELIMITER
 ⟈  ⟉    U+27C8, U+27C9   REVERSE SOLIDUS PRECEDING SUBSET,  SUPERSET
                          PRECEDING SOLIDUS
 ⟞  ⟝    U+27DE, U+27DD   LONG LEFT/RIGHT TACK
 ⟦  ⟧    U+27E6, U+27E7   MATHEMATICAL LEFT/RIGHT WHITE SQUARE BRACKET
 ⟨  ⟩    U+27E8, U+27E9   MATHEMATICAL LEFT/RIGHT ANGLE BRACKET
 ⟪  ⟫    U+27EA, U+27EB   MATHEMATICAL LEFT/RIGHT DOUBLE ANGLE BRACKET
 ⟬  ⟭    U+27EC, U+27ED   MATHEMATICAL LEFT/RIGHT WHITE TORTOISE SHELL
                          BRACKET
 ⟮  ⟯    U+27EE, U+27EF   MATHEMATICAL LEFT/RIGHT FLATTENED PARENTHESIS
 ⟴  ⬲    U+27F4, U+2B32   RIGHT/LEFT ARROW WITH CIRCLED PLUS
 ⟶  ⟵    U+27F6, U+27F5   LONG RIGHT/LEFTWARDS ARROW
 ⟹  ⟸    U+27F9, U+27F8   LONG RIGHT/LEFTWARDS DOUBLE ARROW
 ⟼  ⟻    U+27FC, U+27FB   LONG RIGHT/LEFTWARDS ARROW FROM BAR
 ⟾  ⟽    U+27FE, U+27FD   LONG RIGHT/LEFTWARDS DOUBLE ARROW FROM BAR
 ⟿  ⬳    U+27FF, U+2B33   LONG RIGHT/LEFTWARDS SQUIGGLE ARROW
 ⤀  ⬴    U+2900, U+2B34   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH VERTICAL
                          STROKE
 ⤁  ⬵    U+2901, U+2B35   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH DOUBLE
                          VERTICAL STROKE
 ⤃  ⤂    U+2903, U+2902   RIGHT/LEFTWARDS DOUBLE ARROW WITH VERTICAL
                          STROKE
 ⤅  ⬶    U+2905, U+2B36   RIGHT/LEFTWARDS TWO-HEADED ARROW FROM BAR
 ⤇  ⤆    U+2907, U+2906   RIGHT/LEFTWARDS DOUBLE ARROW FROM BAR
 ⤍  ⤌    U+290D, U+290C   RIGHT/LEFTWARDS DOUBLE DASH ARROW
 ⤏  ⤎    U+290F, U+290E   RIGHT/LEFTWARDS TRIPLE DASH ARROW
 ⤐  ⬷    U+2910, U+2B37   RIGHT/LEFTWARDS TWO-HEADED TRIPLE DASH ARROW
 ⤑  ⬸    U+2911, U+2B38   RIGHT/LEFTWARDS ARROW WITH DOTTED STEM
 ⤔  ⬹    U+2914, U+2B39   RIGHT/LEFTWARDS ARROW WITH TAIL WITH VERTICAL
                          STROKE
 ⤕  ⬺    U+2915, U+2B3A   RIGHT/LEFTWARDS ARROW WITH TAIL WITH DOUBLE
                          VERTICAL STROKE
 ⤖  ⬻    U+2916, U+2B3B   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH TAIL
 ⤗  ⬼    U+2917, U+2B3C   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH TAIL WITH
                          VERTICAL STROKE
 ⤘  ⬽    U+2918, U+2B3D   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH TAIL WITH
                          DOUBLE VERTICAL STROKE
 ⤚  ⤙    U+291A, U+2919   RIGHT/LEFTWARDS ARROW-TAIL
 ⤜  ⤛    U+291C, U+291B   RIGHT/LEFTWARDS DOUBLE ARROW-TAIL
 ⤞  ⤝    U+291E, U+291D   RIGHT/LEFTWARDS ARROW TO BLACK DIAMOND
 ⤠  ⤟    U+2920, U+291F   RIGHT/LEFTWARDS ARROW FROM BAR TO BLACK DIAMOND
 ⤳  ⬿    U+2933, U+2B3F   WAVE ARROW POINTING DIRECTLY RIGHT/LEFT
 ⤷  ⤶    U+2937, U+2936   ARROW POINTING DOWNWARDS THEN CURVING RIGHT/
                          LEFTWARDS
 ⥅  ⥆    U+2945, U+2946   RIGHT/LEFTWARDS ARROW WITH PLUS BELOW
 ⥇  ⬾    U+2947, U+2B3E   RIGHT/LEFTWARDS ARROW THROUGH X
 ⥓  ⥒    U+2953, U+2952   RIGHT/LEFTWARDS HARPOON WITH BARB UP TO BAR
 ⥗  ⥖    U+2957, U+2956   RIGHT/LEFTWARDS HARPOON WITH BARB DOWN TO BAR
 ⥛  ⥚    U+295B, U+295A   RIGHT/LEFTWARDS HARPOON WITH BARB UP FROM BAR
 ⥟  ⥞    U+295F, U+295E   RIGHT/LEFTWARDS HARPOON WITH BARB DOWN FROM BAR
 ⥤  ⥢    U+2964, U+2962   RIGHT/LEFTWARDS HARPOON WITH BARB UP ABOVE
                          RIGHT/LEFTWARDS HARPOON WITH BARB DOWN
 ⥬  ⥪    U+296C, U+296A   RIGHT/LEFTWARDS HARPOON WITH BARB UP ABOVE LONG
                          DASH
 ⥭  ⥫    U+296D, U+296B   RIGHT/LEFTWARDS HARPOON WITH BARB DOWN BELOW
                          LONG DASH
 ⥱  ⭀    U+2971, U+2B40   EQUALS SIGN ABOVE RIGHT/LEFTWARDS ARROW
 ⥲  ⭁    U+2972, U+2B41   TILDE OPERATOR ABOVE RIGHTWARDS ARROW,  REVERSE
                          TILDE OPERATOR ABOVE LEFTWARDS ARROW
 ⥴  ⭋    U+2974, U+2B4B   RIGHTWARDS ARROW ABOVE TILDE OPERATOR,
                          LEFTWARDS ARROW ABOVE REVERSE TILDE OPERATOR
 ⥵  ⭂    U+2975, U+2B42   RIGHTWARDS ARROW ABOVE ALMOST EQUAL TO,
                          LEFTWARDS ARROW ABOVE REVERSE ALMOST EQUAL TO
 ⥹  ⥻    U+2979, U+297B   SUBSET/SUPERSET ABOVE RIGHT/LEFTWARDS ARROW
 ⦃  ⦄    U+2983, U+2984   LEFT/RIGHT WHITE CURLY BRACKET
 ⦅  ⦆    U+2985, U+2986   LEFT/RIGHT WHITE PARENTHESIS
 ⦇  ⦈    U+2987, U+2988   Z NOTATION LEFT/RIGHT IMAGE BRACKET
 ⦉  ⦊    U+2989, U+298A   Z NOTATION LEFT/RIGHT BINDING BRACKET
 ⦋  ⦌    U+298B, U+298C   LEFT/RIGHT SQUARE BRACKET WITH UNDERBAR
 ⦍  ⦐    U+298D, U+2990   LEFT/RIGHT SQUARE BRACKET WITH TICK IN TOP
                          CORNER
 ⦏  ⦎    U+298F, U+298E   LEFT/RIGHT SQUARE BRACKET WITH TICK IN BOTTOM
                          CORNER
 ⦑  ⦒    U+2991, U+2992   LEFT/RIGHT ANGLE BRACKET WITH DOT
 ⦓  ⦔    U+2993, U+2994   LEFT/RIGHT ARC LESS-THAN/GREATER-THAN BRACKET
 ⦕  ⦖    U+2995, U+2996   DOUBLE LEFT/RIGHT ARC GREATER-THAN/LESS-THAN
                          BRACKET
 ⦗  ⦘    U+2997, U+2998   LEFT/RIGHT BLACK TORTOISE SHELL BRACKET
 ⦨  ⦩    U+29A8, U+29A9   MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW
                          POINTING UP AND RIGHT/LEFT
 ⦪  ⦫    U+29AA, U+29AB   MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW
                          POINTING DOWN AND RIGHT/LEFT
 ⦳  ⦴    U+29B3, U+29B4   EMPTY SET WITH RIGHT/LEFT ARROW ABOVE
 ⧀  ⧁    U+29C0, U+29C1   CIRCLED LESS-THAN/GREATER-THAN
 ⧘  ⧙    U+29D8, U+29D9   LEFT/RIGHT WIGGLY FENCE
 ⧚  ⧛    U+29DA, U+29DB   LEFT/RIGHT DOUBLE WIGGLY FENCE
 ⧼  ⧽    U+29FC, U+29FD   LEFT/RIGHT-POINTING CURVED ANGLE BRACKET
 ⩹  ⩺    U+2A79, U+2A7A   LESS-THAN/GREATER-THAN WITH CIRCLE INSIDE
 ⩻  ⩼    U+2A7B, U+2A7C   LESS-THAN/GREATER-THAN WITH QUESTION MARK ABOVE
 ⩽  ⩾    U+2A7D, U+2A7E   LESS-THAN/GREATER-THAN OR SLANTED EQUAL TO
 ⩿  ⪀    U+2A7F, U+2A80   LESS-THAN/GREATER-THAN OR SLANTED EQUAL TO WITH
                          DOT INSIDE
 ⪁  ⪂    U+2A81, U+2A82   LESS-THAN/GREATER-THAN OR SLANTED EQUAL TO WITH
                          DOT ABOVE
 ⪃  ⪄    U+2A83, U+2A84   LESS-THAN/GREATER-THAN OR SLANTED EQUAL TO WITH
                          DOT ABOVE RIGHT/LEFT
 ⪅  ⪆    U+2A85, U+2A86   LESS-THAN/GREATER-THAN OR APPROXIMATE
 ⪇  ⪈    U+2A87, U+2A88   LESS-THAN/GREATER-THAN AND SINGLE-LINE NOT
                          EQUAL TO
 ⪉  ⪊    U+2A89, U+2A8A   LESS-THAN/GREATER-THAN AND NOT APPROXIMATE
 ⪍  ⪎    U+2A8D, U+2A8E   LESS-THAN/GREATER-THAN ABOVE SIMILAR OR EQUAL
 ⪕  ⪖    U+2A95, U+2A96   SLANTED EQUAL TO OR LESS-THAN/GREATER-THAN
 ⪗  ⪘    U+2A97, U+2A98   SLANTED EQUAL TO OR LESS-THAN/GREATER-THAN WITH
                          DOT INSIDE
 ⪙  ⪚    U+2A99, U+2A9A   DOUBLE-LINE EQUAL TO OR LESS-THAN/GREATER-THAN
 ⪛  ⪜    U+2A9B, U+2A9C   DOUBLE-LINE SLANTED EQUAL TO OR LESS-THAN/
                          GREATER-THAN
 ⪝  ⪞    U+2A9D, U+2A9E   SIMILAR OR LESS-THAN/GREATER-THAN
 ⪟  ⪠    U+2A9F, U+2AA0   SIMILAR ABOVE LESS-THAN/GREATER-THAN ABOVE
                          EQUALS SIGN
 ⪡  ⪢    U+2AA1, U+2AA2   DOUBLE NESTED LESS-THAN/GREATER-THAN
 ⪦  ⪧    U+2AA6, U+2AA7   LESS-THAN/GREATER-THAN CLOSED BY CURVE
 ⪨  ⪩    U+2AA8, U+2AA9   LESS-THAN/GREATER-THAN CLOSED BY CURVE ABOVE
                          SLANTED EQUAL
 ⪪  ⪫    U+2AAA, U+2AAB   SMALLER THAN/LARGER THAN
 ⪬  ⪭    U+2AAC, U+2AAD   SMALLER THAN/LARGER THAN OR EQUAL TO
 ⪯  ⪰    U+2AAF, U+2AB0   PRECEDES/SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN
 ⪱  ⪲    U+2AB1, U+2AB2   PRECEDES/SUCCEEDS ABOVE SINGLE-LINE NOT EQUAL TO
 ⪳  ⪴    U+2AB3, U+2AB4   PRECEDES/SUCCEEDS ABOVE EQUALS SIGN
 ⪵  ⪶    U+2AB5, U+2AB6   PRECEDES/SUCCEEDS ABOVE NOT EQUAL TO
 ⪷  ⪸    U+2AB7, U+2AB8   PRECEDES/SUCCEEDS ABOVE ALMOST EQUAL TO
 ⪹  ⪺    U+2AB9, U+2ABA   PRECEDES/SUCCEEDS ABOVE NOT ALMOST EQUAL TO
 ⪻  ⪼    U+2ABB, U+2ABC   DOUBLE PRECEDES/SUCCEEDS
 ⪽  ⪾    U+2ABD, U+2ABE   SUBSET/SUPERSET WITH DOT
 ⪿  ⫀    U+2ABF, U+2AC0   SUBSET/SUPERSET WITH PLUS SIGN BELOW
 ⫁  ⫂    U+2AC1, U+2AC2   SUBSET/SUPERSET WITH MULTIPLICATION SIGN BELOW
 ⫃  ⫄    U+2AC3, U+2AC4   SUBSET/SUPERSET OF OR EQUAL TO WITH DOT ABOVE
 ⫅  ⫆    U+2AC5, U+2AC6   SUBSET/SUPERSET OF ABOVE EQUALS SIGN
 ⫇  ⫈    U+2AC7, U+2AC8   SUBSET/SUPERSET OF ABOVE TILDE OPERATOR
 ⫉  ⫊    U+2AC9, U+2ACA   SUBSET/SUPERSET OF ABOVE ALMOST EQUAL TO
 ⫋  ⫌    U+2ACB, U+2ACC   SUBSET/SUPERSET OF ABOVE NOT EQUAL TO
 ⫏  ⫐    U+2ACF, U+2AD0   CLOSED SUBSET/SUPERSET
 ⫑  ⫒    U+2AD1, U+2AD2   CLOSED SUBSET/SUPERSET OR EQUAL TO
 ⫕  ⫖    U+2AD5, U+2AD6   SUBSET/SUPERSET ABOVE SUBSET/SUPERSET
 ⫥  ⊫    U+2AE5, U+22AB   DOUBLE VERTICAL BAR DOUBLE LEFT/RIGHT TURNSTILE
 ⫷  ⫸    U+2AF7, U+2AF8   TRIPLE NESTED LESS-THAN/GREATER-THAN
 ⫹  ⫺    U+2AF9, U+2AFA   DOUBLE-LINE SLANTED LESS-THAN/GREATER-THAN OR
                          EQUAL TO
 ⭆  ⭅    U+2B46, U+2B45   RIGHT/LEFTWARDS QUADRUPLE ARROW
 ⭇  ⭉    U+2B47, U+2B49   REVERSE TILDE OPERATOR ABOVE RIGHTWARDS ARROW,
                          TILDE OPERATOR ABOVE LEFTWARDS ARROW
 ⭈  ⭊    U+2B48, U+2B4A   RIGHTWARDS ARROW ABOVE REVERSE ALMOST EQUAL
                          TO,  LEFTWARDS ARROW ABOVE ALMOST EQUAL TO
 ⭌  ⥳    U+2B4C, U+2973   RIGHTWARDS ARROW ABOVE REVERSE TILDE OPERATOR,
                          LEFTWARDS ARROW ABOVE TILDE OPERATOR
 ⭢  ⭠    U+2B62, U+2B60   RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW
 ⭬  ⭪    U+2B6C, U+2B6A   RIGHT/LEFTWARDS TRIANGLE-HEADED DASHED ARROW
 ⭲  ⭰    U+2B72, U+2B70   RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW TO BAR
 ⭼  ⭺    U+2B7C, U+2B7A   RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH
                          DOUBLE VERTICAL STROKE
 ⮆  ⮄    U+2B86, U+2B84   RIGHT/LEFTWARDS TRIANGLE-HEADED PAIRED ARROWS
 ⮊  ⮈    U+2B8A, U+2B88   RIGHT/LEFTWARDS BLACK CIRCLED WHITE ARROW
 ⮕  ⬅    U+2B95, U+2B05   RIGHT/LEFTWARDS BLACK ARROW
 ⮚  ⮘    U+2B9A, U+2B98   THREE-D TOP-LIGHTED RIGHT/LEFTWARDS EQUILATERAL
                          ARROWHEAD
 ⮞  ⮜    U+2B9E, U+2B9C   BLACK RIGHT/LEFTWARDS EQUILATERAL ARROWHEAD
 ⮡  ⮠    U+2BA1, U+2BA0   DOWNWARDS TRIANGLE-HEADED ARROW WITH LONG TIP
                          RIGHT/LEFTWARDS
 ⮣  ⮢    U+2BA3, U+2BA2   UPWARDS TRIANGLE-HEADED ARROW WITH LONG TIP
                          RIGHT/LEFTWARDS
 ⮩  ⮨    U+2BA9, U+2BA8   BLACK CURVED DOWNWARDS AND RIGHT/LEFTWARDS ARROW
 ⮫  ⮪    U+2BAB, U+2BAA   BLACK CURVED UPWARDS AND RIGHT/LEFTWARDS ARROW
 ⮱  ⮰    U+2BB1, U+2BB0   RIBBON ARROW DOWN RIGHT/LEFT
 ⮳  ⮲    U+2BB3, U+2BB2   RIBBON ARROW UP RIGHT/LEFT
 ⯮  ⯬    U+2BEE, U+2BEC   RIGHT/LEFTWARDS TWO-HEADED ARROW WITH TRIANGLE
                          ARROWHEADS
 ⸂  ⸃    U+2E02, U+2E03   LEFT/RIGHT SUBSTITUTION BRACKET
 ⸃  ⸂    U+2E03, U+2E02   RIGHT/LEFT SUBSTITUTION BRACKET
 ⸄  ⸅    U+2E04, U+2E05   LEFT/RIGHT DOTTED SUBSTITUTION BRACKET
 ⸅  ⸄    U+2E05, U+2E04   RIGHT/LEFT DOTTED SUBSTITUTION BRACKET
 ⸉  ⸊    U+2E09, U+2E0A   LEFT/RIGHT TRANSPOSITION BRACKET
 ⸊  ⸉    U+2E0A, U+2E09   RIGHT/LEFT TRANSPOSITION BRACKET
 ⸌  ⸍    U+2E0C, U+2E0D   LEFT/RIGHT RAISED OMISSION BRACKET
 ⸍  ⸌    U+2E0D, U+2E0C   RIGHT/LEFT RAISED OMISSION BRACKET
 ⸑  ⸐    U+2E11, U+2E10   REVERSED FORKED PARAGRAPHOS,  FORKED PARAGRAPHOS
 ⸜  ⸝    U+2E1C, U+2E1D   LEFT/RIGHT LOW PARAPHRASE BRACKET
 ⸝  ⸜    U+2E1D, U+2E1C   RIGHT/LEFT LOW PARAPHRASE BRACKET
 ⸠  ⸡    U+2E20, U+2E21   LEFT/RIGHT VERTICAL BAR WITH QUILL
 ⸡  ⸠    U+2E21, U+2E20   RIGHT/LEFT VERTICAL BAR WITH QUILL
 ⸢  ⸣    U+2E22, U+2E23   TOP LEFT/RIGHT HALF BRACKET
 ⸤  ⸥    U+2E24, U+2E25   BOTTOM LEFT/RIGHT HALF BRACKET
 ⸦  ⸧    U+2E26, U+2E27   LEFT/RIGHT SIDEWAYS U BRACKET
 ⸨  ⸩    U+2E28, U+2E29   LEFT/RIGHT DOUBLE PARENTHESIS
 ⸶  ⸷    U+2E36, U+2E37   DAGGER WITH LEFT/RIGHT GUARD
 ⹂  „    U+2E42, U+201E   DOUBLE LOW-REVERSED-9 QUOTATION MARK,  DOUBLE
                          LOW-9 QUOTATION MARK
 ⹕  ⹖    U+2E55, U+2E56   LEFT/RIGHT SQUARE BRACKET WITH STROKE
 ⹗  ⹘    U+2E57, U+2E58   LEFT/RIGHT SQUARE BRACKET WITH DOUBLE STROKE
 ⹙  ⹚    U+2E59, U+2E5A   TOP HALF LEFT/RIGHT PARENTHESIS
 ⹛  ⹜    U+2E5B, U+2E5C   BOTTOM HALF LEFT/RIGHT PARENTHESIS
 〈 〉   U+3008, U+3009   LEFT/RIGHT ANGLE BRACKET
 《 》   U+300A, U+300B   LEFT/RIGHT DOUBLE ANGLE BRACKET
 「 」   U+300C, U+300D   LEFT/RIGHT CORNER BRACKET
 『 』   U+300E, U+300F   LEFT/RIGHT WHITE CORNER BRACKET
 【 】   U+3010, U+3011   LEFT/RIGHT BLACK LENTICULAR BRACKET
 〔 〕   U+3014, U+3015   LEFT/RIGHT TORTOISE SHELL BRACKET
 〖 〗   U+3016, U+3017   LEFT/RIGHT WHITE LENTICULAR BRACKET
 〘 〙   U+3018, U+3019   LEFT/RIGHT WHITE TORTOISE SHELL BRACKET
 〚 〛   U+301A, U+301B   LEFT/RIGHT WHITE SQUARE BRACKET
 〝 〞   U+301D, U+301E   REVERSED DOUBLE PRIME QUOTATION MARK,  DOUBLE
                          PRIME QUOTATION MARK
 ꧁  ꧂    U+A9C1, U+A9C2   JAVANESE LEFT/RIGHT RERENGGAN
 ﴾  ﴿    U+FD3E, U+FD3F   ORNATE LEFT/RIGHT PARENTHESIS
 ﹙ ﹚   U+FE59, U+FE5A   SMALL LEFT/RIGHT PARENTHESIS
 ﹛ ﹜   U+FE5B, U+FE5C   SMALL LEFT/RIGHT CURLY BRACKET
 ﹝ ﹞   U+FE5D, U+FE5E   SMALL LEFT/RIGHT TORTOISE SHELL BRACKET
 ﹤ ﹥   U+FE64, U+FE65   SMALL LESS-THAN/GREATER-THAN SIGN
 （ ）   U+FF08, U+FF09   FULLWIDTH LEFT/RIGHT PARENTHESIS
 ＜ ＞   U+FF1C, U+FF1E   FULLWIDTH LESS-THAN/GREATER-THAN SIGN
 ［ ］   U+FF3B, U+FF3D   FULLWIDTH LEFT/RIGHT SQUARE BRACKET
 ｛ ｝   U+FF5B, U+FF5D   FULLWIDTH LEFT/RIGHT CURLY BRACKET
 ｟ ｠   U+FF5F, U+FF60   FULLWIDTH LEFT/RIGHT WHITE PARENTHESIS
 ｢  ｣    U+FF62, U+FF63   HALFWIDTH LEFT/RIGHT CORNER BRACKET
 ￫  ￩    U+FFEB, U+FFE9   HALFWIDTH RIGHT/LEFTWARDS ARROW
 𝄃  𝄂    U+1D103, U+1D102 MUSICAL SYMBOL REVERSE FINAL BARLINE,  MUSICAL
                          SYMBOL FINAL BARLINE
 𝄆  𝄇    U+1D106, U+1D107 MUSICAL SYMBOL LEFT/RIGHT REPEAT SIGN
 👉 👈   U+1F449, U+1F448 WHITE RIGHT/LEFT POINTING BACKHAND INDEX
 🔈 🕨    U+1F508, U+1F568 SPEAKER,  RIGHT SPEAKER
 🔉 🕩    U+1F509, U+1F569 SPEAKER WITH ONE SOUND WAVE,  RIGHT SPEAKER WITH
                          ONE SOUND WAVE
 🔊 🕪    U+1F50A, U+1F56A SPEAKER WITH THREE SOUND WAVES,  RIGHT SPEAKER
                          WITH THREE SOUND WAVES
 🕻  🕽    U+1F57B, U+1F57D LEFT/RIGHT HAND TELEPHONE RECEIVER
 🖙  🖘    U+1F599, U+1F598 SIDEWAYS WHITE RIGHT/LEFT POINTING INDEX
 🖛  🖚    U+1F59B, U+1F59A SIDEWAYS BLACK RIGHT/LEFT POINTING INDEX
 🖝  🖜    U+1F59D, U+1F59C BLACK RIGHT/LEFT POINTING BACKHAND INDEX
 🗦  🗧    U+1F5E6, U+1F5E7 THREE RAYS LEFT/RIGHT
 🠂  🠀    U+1F802, U+1F800 RIGHT/LEFTWARDS ARROW WITH SMALL TRIANGLE
                          ARROWHEAD
 🠆  🠄    U+1F806, U+1F804 RIGHT/LEFTWARDS ARROW WITH MEDIUM TRIANGLE
                          ARROWHEAD
 🠊  🠈    U+1F80A, U+1F808 RIGHT/LEFTWARDS ARROW WITH LARGE TRIANGLE
                          ARROWHEAD
 🠒  🠐    U+1F812, U+1F810 RIGHT/LEFTWARDS ARROW WITH SMALL EQUILATERAL
                          ARROWHEAD
 🠖  🠔    U+1F816, U+1F814 RIGHT/LEFTWARDS ARROW WITH EQUILATERAL ARROWHEAD
 🠚  🠘    U+1F81A, U+1F818 HEAVY RIGHT/LEFTWARDS ARROW WITH EQUILATERAL
                          ARROWHEAD
 🠞  🠜    U+1F81E, U+1F81C HEAVY RIGHT/LEFTWARDS ARROW WITH LARGE
                          EQUILATERAL ARROWHEAD
 🠢  🠠    U+1F822, U+1F820 RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH
                          NARROW SHAFT
 🠦  🠤    U+1F826, U+1F824 RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH
                          MEDIUM SHAFT
 🠪  🠨    U+1F82A, U+1F828 RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH BOLD
                          SHAFT
 🠮  🠬    U+1F82E, U+1F82C RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH
                          HEAVY SHAFT
 🠲  🠰    U+1F832, U+1F830 RIGHT/LEFTWARDS TRIANGLE-HEADED ARROW WITH VERY
                          HEAVY SHAFT
 🠶  🠴    U+1F836, U+1F834 RIGHT/LEFTWARDS FINGER-POST ARROW
 🠺  🠸    U+1F83A, U+1F838 RIGHT/LEFTWARDS SQUARED ARROW
 🠾  🠼    U+1F83E, U+1F83C RIGHT/LEFTWARDS COMPRESSED ARROW
 🡂  🡀    U+1F842, U+1F840 RIGHT/LEFTWARDS HEAVY COMPRESSED ARROW
 🡆  🡄    U+1F846, U+1F844 RIGHT/LEFTWARDS HEAVY ARROW
 🡒  🡐    U+1F852, U+1F850 RIGHT/LEFTWARDS SANS-SERIF ARROW
 🡢  🡠    U+1F862, U+1F860 WIDE-HEADED RIGHT/LEFTWARDS LIGHT BARB ARROW
 🡪  🡨    U+1F86A, U+1F868 WIDE-HEADED RIGHT/LEFTWARDS BARB ARROW
 🡲  🡰    U+1F872, U+1F870 WIDE-HEADED RIGHT/LEFTWARDS MEDIUM BARB ARROW
 🡺  🡸    U+1F87A, U+1F878 WIDE-HEADED RIGHT/LEFTWARDS HEAVY BARB ARROW
 🢂  🢀    U+1F882, U+1F880 WIDE-HEADED RIGHT/LEFTWARDS VERY HEAVY BARB
                          ARROW
 🢒  🢐    U+1F892, U+1F890 RIGHT/LEFTWARDS TRIANGLE ARROWHEAD
 🢖  🢔    U+1F896, U+1F894 RIGHT/LEFTWARDS WHITE ARROW WITHIN TRIANGLE
                          ARROWHEAD
 🢚  🢘    U+1F89A, U+1F898 RIGHT/LEFTWARDS ARROW WITH NOTCHED TAIL
 🢡  🢠    U+1F8A1, U+1F8A0 RIGHTWARDS BOTTOM SHADED WHITE ARROW,
                          LEFTWARDS BOTTOM-SHADED WHITE ARROW
 🢣  🢢    U+1F8A3, U+1F8A2 RIGHT/LEFTWARDS TOP SHADED WHITE ARROW
 🢥  🢦    U+1F8A5, U+1F8A6 RIGHT/LEFTWARDS RIGHT-SHADED WHITE ARROW
 🢧  🢤    U+1F8A7, U+1F8A4 RIGHT/LEFTWARDS LEFT-SHADED WHITE ARROW
 🢩  🢨    U+1F8A9, U+1F8A8 RIGHT/LEFTWARDS BACK-TILTED SHADOWED WHITE ARROW
 🢫  🢪    U+1F8AB, U+1F8AA RIGHT/LEFTWARDS FRONT-TILTED SHADOWED WHITE
                          ARROW

=head2 The 'module_true' feature

This feature removes the need to return a true value at the end of a module
loaded with C<require> or C<use>. Any errors during compilation will cause
failures, but reaching the end of the module when this feature is in effect
will prevent C<perl> from throwing an exception that the module "did not return
a true value".

=head2 The 'class' feature

B<WARNING>: This feature is still experimental and the implementation may
change or be removed in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the warning:

    no warnings "experimental::class";

This feature enables the C<class> block syntax and other associated keywords
which implement the "new" object system, previously codenamed "Corinna".

=head1 FEATURE BUNDLES

It's possible to load multiple features together, using
a I<feature bundle>.  The name of a feature bundle is prefixed with
a colon, to distinguish it from an actual feature.

  use feature ":5.10";

The following feature bundles are available:

  bundle    features included
  --------- -----------------
PODTURES
The C<:default> bundle represents the feature set that is enabled before
any C<use feature> or C<no feature> declaration.

Specifying sub-versions such as the C<0> in C<5.14.0> in feature bundles has
no effect.  Feature bundles are guaranteed to be the same for all sub-versions.

  use feature ":5.14.0";    # same as ":5.14"
  use feature ":5.14.1";    # same as ":5.14"

=head1 IMPLICIT LOADING

Instead of loading feature bundles by name, it is easier to let Perl do
implicit loading of a feature bundle for you.

There are two ways to load the C<feature> pragma implicitly:

=over 4

=item *

By using the C<-E> switch on the Perl command-line instead of C<-e>.
That will enable the feature bundle for that version of Perl in the
main compilation unit (that is, the one-liner that follows C<-E>).

=item *

By explicitly requiring a minimum Perl version number for your program, with
the C<use VERSION> construct.  That is,

    use v5.36.0;

will do an implicit

    no feature ':all';
    use feature ':5.36';

and so on.  Note how the trailing sub-version
is automatically stripped from the
version.

But to avoid portability warnings (see L<perlfunc/use>), you may prefer:

    use 5.036;

with the same effect.

If the required version is older than Perl 5.10, the ":default" feature
bundle is automatically loaded instead.

Unlike C<use feature ":5.12">, saying C<use v5.12> (or any higher version)
also does the equivalent of C<use strict>; see L<perlfunc/use> for details.

=back

=head1 CHECKING FEATURES

C<feature> provides some simple APIs to check which features are enabled.

These functions cannot be imported and must be called by their fully
qualified names.  If you don't otherwise need to set a feature you will
need to ensure C<feature> is loaded with:

  use feature ();

=over

=item feature_enabled($feature)

=item feature_enabled($feature, $depth)

  package MyStandardEnforcer;
  use feature ();
  use Carp "croak";
  sub import {
    croak "disable indirect!" if feature::feature_enabled("indirect");
  }

Test whether a named feature is enabled at a given level in the call
stack, returning a true value if it is.  C<$depth> defaults to 1,
which checks the scope that called the scope calling
feature::feature_enabled().

croaks for an unknown feature name.

=item features_enabled()

=item features_enabled($depth)

  package ReportEnabledFeatures;
  use feature "say";
  sub import {
    say STDERR join " ", feature::features_enabled();
  }

Returns a list of the features enabled at a given level in the call
stack.  C<$depth> defaults to 1, which checks the scope that called
the scope calling feature::features_enabled().

=item feature_bundle()

=item feature_bundle($depth)

Returns the feature bundle, if any, selected at a given level in the
call stack.  C<$depth> defaults to 1, which checks the scope that called
the scope calling feature::feature_bundle().

Returns an undefined value if no feature bundle is selected in the
scope.

The bundle name returned will be for the earliest bundle matching the
selected bundle, so:

  use feature ();
  use v5.12;
  BEGIN { print feature::feature_bundle(0); }

will print C<5.11>.

This returns internal state, at this point C<use v5.12;> sets the
feature bundle, but C< use feature ":5.12"; > does not set the feature
bundle.  This may change in a future release of perl.

=back

=cut

sub import {
    shift;

    if (!@_) {
        croak("No features specified");
    }

    __common(1, @_);
}

sub unimport {
    shift;

    # A bare C<no feature> should reset to the default bundle
    if (!@_) {
	$^H &= ~($hint_uni8bit|$hint_mask);
	return;
    }

    __common(0, @_);
}


sub __common {
    my $import = shift;
    my $bundle_number = $^H & $hint_mask;
    my $features = $bundle_number != $hint_mask
      && $feature_bundle{$hint_bundles[$bundle_number >> $hint_shift]};
    if ($features) {
	# Features are enabled implicitly via bundle hints.
	# Delete any keys that may be left over from last time.
	delete @^H{ values(%feature) };
	$^H |= $hint_mask;
	for (@$features) {
	    $^H{$feature{$_}} = 1;
	    $^H |= $hint_uni8bit if $_ eq 'unicode_strings';
	}
    }
    while (@_) {
        my $name = shift;
        if (substr($name, 0, 1) eq ":") {
            my $v = substr($name, 1);
            if (!exists $feature_bundle{$v}) {
                $v =~ s/^([0-9]+)\.([0-9]+).[0-9]+$/$1.$2/;
                if (!exists $feature_bundle{$v}) {
                    unknown_feature_bundle(substr($name, 1));
                }
            }
            unshift @_, @{$feature_bundle{$v}};
            next;
        }
        if (!exists $feature{$name}) {
            if (exists $noops{$name}) {
                next;
            }
            if (!$import && exists $removed{$name}) {
                next;
            }
            unknown_feature($name);
        }
	if ($import) {
	    $^H{$feature{$name}} = 1;
	    $^H |= $hint_uni8bit if $name eq 'unicode_strings';
	} else {
            delete $^H{$feature{$name}};
            $^H &= ~ $hint_uni8bit if $name eq 'unicode_strings';
        }
    }
}

sub unknown_feature {
    my $feature = shift;
    croak(sprintf('Feature "%s" is not supported by Perl %vd',
            $feature, $^V));
}

sub unknown_feature_bundle {
    my $feature = shift;
    croak(sprintf('Feature bundle "%s" is not supported by Perl %vd',
            $feature, $^V));
}

sub croak {
    require Carp;
    Carp::croak(@_);
}

sub features_enabled {
    my ($depth) = @_;

    $depth //= 1;
    my @frame = caller($depth+1)
      or return;
    my ($hints, $hinthash) = @frame[8, 10];

    my $bundle_number = $hints & $hint_mask;
    if ($bundle_number != $hint_mask) {
        return $feature_bundle{$hint_bundles[$bundle_number >> $hint_shift]}->@*;
    }
    else {
        my @features;
        for my $feature (sort keys %feature) {
            if ($hinthash->{$feature{$feature}}) {
                push @features, $feature;
            }
        }
        return @features;
    }
}

sub feature_enabled {
    my ($feature, $depth) = @_;

    $depth //= 1;
    my @frame = caller($depth+1)
      or return;
    my ($hints, $hinthash) = @frame[8, 10];

    my $hint_feature = $feature{$feature}
      or croak "Unknown feature $feature";
    my $bundle_number = $hints & $hint_mask;
    if ($bundle_number != $hint_mask) {
        my $bundle = $hint_bundles[$bundle_number >> $hint_shift];
        for my $bundle_feature ($feature_bundle{$bundle}->@*) {
            return 1 if $bundle_feature eq $feature;
        }
        return 0;
    }
    else {
        return $hinthash->{$hint_feature} // 0;
    }
}

sub feature_bundle {
    my $depth = shift;

    $depth //= 1;
    my @frame = caller($depth+1)
      or return;
    my $bundle_number = $frame[8] & $hint_mask;
    if ($bundle_number != $hint_mask) {
        return $hint_bundles[$bundle_number >> $hint_shift];
    }
    else {
        return undef;
    }
}

1;
