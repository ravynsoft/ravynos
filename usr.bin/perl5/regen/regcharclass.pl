#!perl
package CharClass::Matcher;
use strict;
use 5.008;
use warnings;
use warnings FATAL => 'all';
use Data::Dumper;
$Data::Dumper::Useqq= 1;

sub DEBUG () { 0 }
$|=1 if DEBUG;

require './regen/regen_lib.pl';
require './regen/charset_translations.pl';
require "./regen/regcharclass_multi_char_folds.pl";

=head1 NAME

CharClass::Matcher -- Generate C macros that match character classes efficiently

=head1 SYNOPSIS

    perl regen/regcharclass.pl

=head1 DESCRIPTION

Dynamically generates macros for detecting special charclasses
in latin-1, utf8, and codepoint forms. Macros can be set to return
the length (in bytes) of the matched codepoint, and/or the codepoint itself.

To regenerate F<regcharclass.h>, run this script from perl-root. No arguments
are necessary.

Using WHATEVER as an example the following macros can be produced, depending
on the input parameters (how to get each is described by internal comments at
the C<__DATA__> line):

=over 4

=item C<is_WHATEVER(s,is_utf8)>

=item C<is_WHATEVER_safe(s,e,is_utf8)>

Do a lookup as appropriate based on the C<is_utf8> flag. When possible
comparisons involving octet<128 are done before checking the C<is_utf8>
flag, hopefully saving time.

The version without the C<_safe> suffix should be used only when the input is
known to be well-formed.

=item C<is_WHATEVER_utf8(s)>

=item C<is_WHATEVER_utf8_safe(s,e)>

Do a lookup assuming the string is encoded in (normalized) UTF8.

The version without the C<_safe> suffix should be used only when the input is
known to be well-formed.

=item C<is_WHATEVER_latin1(s)>

=item C<is_WHATEVER_latin1_safe(s,e)>

Do a lookup assuming the string is encoded in latin-1 (aka plan octets).

The version without the C<_safe> suffix should be used only when it is known
that C<s> contains at least one character.

=item C<is_WHATEVER_cp(cp)>

Check to see if the string matches a given codepoint (hypothetically a
U32). The condition is constructed as to "break out" as early as
possible if the codepoint is out of range of the condition.

IOW:

  (cp==X || (cp>X && (cp==Y || (cp>Y && ...))))

Thus if the character is X+1 only two comparisons will be done. Making
matching lookups slower, but non-matching faster.

=item C<what_len_WHATEVER_FOO(arg1, ..., len)>

A variant form of each of the macro types described above can be generated, in
which the code point is returned by the macro, and an extra parameter (in the
final position) is added, which is a pointer for the macro to set the byte
length of the returned code point.

These forms all have a C<what_len> prefix instead of the C<is_>, for example
C<what_len_WHATEVER_safe(s,e,is_utf8,len)> and
C<what_len_WHATEVER_utf8(s,len)>.

These forms should not be used I<except> on small sets of mostly widely
separated code points; otherwise the code generated is inefficient.  For these
cases, it is best to use the C<is_> forms, and then find the code point with
C<utf8_to_uvchr_buf>().  This program can fail with a "deep recursion"
message on the worst of the inappropriate sets.  Examine the generated macro
to see if it is acceptable.

=item C<what_WHATEVER_FOO(arg1, ...)>

A variant form of each of the C<is_> macro types described above can be generated, in
which the code point and not the length is returned by the macro.  These have
the same caveat as L</what_len_WHATEVER_FOO(arg1, ..., len)>, plus they should
not be used where the set contains a NULL, as 0 is returned for two different
cases: a) the set doesn't include the input code point; b) the set does
include it, and it is a NULL.

=back

The above isn't quite complete, as for specialized purposes one can get a
macro like C<is_WHATEVER_utf8_no_length_checks(s)>, which assumes that it is
already known that there is enough space to hold the character starting at
C<s>, but otherwise checks that it is well-formed.  In other words, this is
intermediary in checking between C<is_WHATEVER_utf8(s)> and
C<is_WHATEVER_utf8_safe(s,e)>.

=head2 CODE FORMAT

perltidy  -st -bt=1 -bbt=0 -pt=0 -sbt=1 -ce -nwls== "%f"


=head1 AUTHOR

Author: Yves Orton (demerphq) 2007.  Maintained by Perl5 Porters.

=head1 BUGS

No tests directly here (although the regex engine will fail tests
if this code is broken). Insufficient documentation and no Getopts
handler for using the module as a script.

=head1 LICENSE

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

=cut

# Sub naming convention:
# __func : private subroutine, can not be called as a method
# _func  : private method, not meant for external use
# func   : public method.

# private subs
#-------------------------------------------------------------------------------
#
# ($cp,$n,$l,$u)=__uni_latin($str);
#
# Return a list of arrays, each of which when interpreted correctly
# represent the string in some given encoding with specific conditions.
#
# $cp - list of codepoints that make up the string.
# $n  - list of octets that make up the string if all codepoints are invariant
#       regardless of if the string is in UTF-8 or not.
# $l  - list of octets that make up the string in latin1 encoding if all
#       codepoints < 256, and at least one codepoint is UTF-8 variant.
# $u  - list of octets that make up the string in utf8 if any codepoint is
#       UTF-8 variant
#
#   High CP | Defined
#-----------+----------
#   0 - 127 : $n            (127/128 are the values for ASCII platforms)
# 128 - 255 : $l, $u
# 256 - ... : $u
#

sub __uni_latin1 {
    my $charset = shift;
    my $a2n= shift;
    my $str= shift;
    my $max= 0;
    my @cp;
    my @cp_high;
    my $only_has_invariants = 1;
    for my $ch ( split //, $str ) {
        my $cp= ord $ch;
        $max= $cp if $max < $cp;
        if ($cp > 255) {
            push @cp, $cp;
            push @cp_high, $cp;
        }
        else {
            push @cp, $a2n->[$cp];
        }
    }
    my ( $n, $l, $u );
    $only_has_invariants = ($charset =~ /ascii/i) ? $max < 128 : $max < 160;
    if ($only_has_invariants) {
        $n= [@cp];
    } else {
        $l= [@cp] if $max && $max < 256;

        my @u;
        for my $ch ( split //, $str ) {
            push @u, map { ord } split //, cp_2_utfbytes(ord $ch, $charset);
        }
        $u = \@u;
    }
    return ( \@cp, \@cp_high, $n, $l, $u );
}

#
# $clean= __clean($expr);
#
# Cleanup a ternary expression, removing unnecessary parens and apply some
# simplifications using regexes.
#

sub __clean {
    my ( $expr )= @_;

    #return $expr;

    our $parens;
    $parens= qr/ (?> \( (?> (?: (?> [^()]+ ) | (??{ $parens }) )* ) \) ) /x;

    ## remove redundant parens
    1 while $expr =~ s/ \( \s* ( $parens ) \s* \) /$1/gx;


    # repeatedly simplify conditions like
    #       ( (cond1) ? ( (cond2) ? X : Y ) : Y )
    # into
    #       ( ( (cond1) && (cond2) ) ? X : Y )
    # Also similarly handles expressions like:
    #       : (cond1) ? ( (cond2) ? X : Y ) : Y )
    # Note the inclusion of the close paren in ([:()]) and the open paren in
    # ([()]) is purely to ensure we have a balanced set of parens in the
    # expression which makes it easier to understand the pattern in an editor
    # that understands paren's, we do not expect either of these cases to
    # actually fire. - Yves
    1 while $expr =~ s/
        ([:()])  \s*
            ($parens) \s*
            \? \s*
                \( \s* ($parens) \s*
                    \? \s* ($parens|[^()?:\s]+?) \s*
                    :  \s* ($parens|[^()?:\s]+?) \s*
                \) \s*
            : \s* \5 \s*
        ([()])
    /$1 ( $2 && $3 ) ? $4 : $5 $6/gx;
    #$expr=~s/\(\(U8\*\)s\)\[(\d+)\]/S$1/g if length $expr > 8000;
    #$expr=~s/\s+//g if length $expr > 8000;

    die "Expression too long" if length $expr > 8000;

    return $expr;
}

#
# $text= __macro(@args);
# Join args together by newlines, and then neatly add backslashes to the end
# of every  line as expected by the C pre-processor for #define's.
#

sub __macro {
    my $str= join "\n", @_;
    $str =~ s/\s*$//;
    my @lines= map { s/\s+$//; s/\t/        /g; $_ } split /\n/, $str;
    my $last= pop @lines;
    $str= join "\n", ( map { sprintf "%-76s\\", $_ } @lines ), $last;
    1 while $str =~ s/^(\t*) {8}/$1\t/gm;
    return $str . "\n";
}

#
# my $op=__incrdepth($op);
#
# take an 'op' hashref and add one to it and all its childrens depths.
#

sub __incrdepth {
    my $op= shift;
    return unless ref $op;
    $op->{depth} += 1;
    __incrdepth( $op->{yes} );
    __incrdepth( $op->{no} );
    return $op;
}

# join two branches of an opcode together with a condition, incrementing
# the depth on the yes branch when we do so.
# returns the new root opcode of the tree.
sub __cond_join {
    my ( $cond, $yes, $no )= @_;
    if (ref $yes) {
        return {
            test  => $cond,
            yes   => __incrdepth( $yes ),
            no    => $no,
            depth => 0,
        };
    }
    else {
        return {
            test  => $cond,
            yes   => $yes,
            no    => __incrdepth($no),
            depth => 0,
        };
    }
}

my $hex_fmt= "0x%02X";

sub val_fmt
{
    my $self = shift;
    my $arg = shift;
    my $always_hex = shift // 0;    # Use \x{}; don't look for a mnemonic

    # Format 'arg' using the printable character if it has one, or a %x if
    # not, returning a string containing the result

    # Return what always returned for an unexpected argument
    return $hex_fmt unless defined $arg && $arg !~ /\D/;

    # We convert only things inside Latin1
    if (! $always_hex && $arg < 256) {

        # Find the ASCII equivalent of this argument (as the current character
        # set might not be ASCII)
        my $char = chr $self->{n2a}->[$arg];

        # If printable, return it, escaping \ and '
        return "'$char'" if $char =~ /[^\\'[:^print:]]/a;
        return "'\\\\'" if $char eq "\\";
        return "'\''" if $char eq "'";

        # Handle the mnemonic controls
        my $pos = index("\a\b\e\f\n\r\t\cK", $char);
        return "'\\" . substr("abefnrtv", $pos, 1) . "'" if $pos >= 0;
    }

    # Otherwise, just the input, formatted
    return sprintf $hex_fmt, $arg;
}

# Methods

# constructor
#
# my $obj=CLASS->new(op=>'SOMENAME',title=>'blah',txt=>[..]);
#
# Create a new CharClass::Matcher object by parsing the text in
# the txt array. Currently applies the following rules:
#
# Element starts with C<0x>, line is evaled the result treated as
# a number which is passed to chr().
#
# Element starts with C<">, line is evaled and the result treated
# as a string.
#
# Each string is then stored in the 'strs' subhash as a hash record
# made up of the results of __uni_latin1, using the keynames
# 'low', 'latin1', 'utf8', as well as the synthesized 'LATIN1', 'high',
# 'UTF8', and 'backwards_UTF8' which hold a merge of 'low' and their lowercase
# equivalents.
#
# Size data is tracked per type in the 'size' subhash.
#
# Return an object

my %a2n;
my %n2a;        # Inversion of a2n, for each character set
my %I8_2_utf;
my %utf_2_I8;   # Inversion of I8_2_utf, for each EBCDIC character set
my @identity = (0..255);

sub new {
    my $class= shift;
    my %opt= @_;
    my %hash_return;
    for ( qw(op txt) ) {
        die "in " . __PACKAGE__ . " constructor '$_;' is a mandatory field"
          if !exists $opt{$_};
    }

    my $self= bless {
        op    => $opt{op},
        title => $opt{title} || '',
    }, $class;

    my $charset = $opt{charset};
    $a2n{$charset} = get_a2n($charset);

    # We need to construct the maps going the other way if not already done
    unless (defined $n2a{$charset}) {
        for (my $i = 0; $i < 256; $i++) {
            $n2a{$charset}->[$a2n{$charset}->[$i]] = $i;
        }
    }

    if ($charset =~ /ebcdic/i) {
        $I8_2_utf{$charset} = get_I8_2_utf($charset);
        unless (defined $utf_2_I8{$charset}) {
            for (my $i = 0; $i < 256; $i++) {
                $utf_2_I8{$charset}->[$I8_2_utf{$charset}->[$i]] = $i;
            }
        }
    }

    foreach my $txt ( @{ $opt{txt} } ) {
        my $str= $txt;
        if ( $str =~ /^[""]/ ) {
            $str= eval $str;
        } elsif ($str =~ / - /x ) { # A range:  Replace this element on the
                                    # list with its expansion
            my ($lower, $upper) = $str =~ / 0x (.+?) \s* - \s* 0x (.+) /x;
            die "Format must be like '0xDEAD - 0xBEAF'; instead was '$str'"
                                        if ! defined $lower || ! defined $upper;
            foreach my $cp (hex $lower .. hex $upper) {
                push @{$opt{txt}}, sprintf "0x%X", $cp;
            }
            next;
        } elsif ($str =~ s/ ^ N (?= 0x ) //x ) {
            # Otherwise undocumented, a leading N means is already in the
            # native character set; don't convert.
            $str= chr eval $str;
        } elsif ( $str =~ /^0x/ ) {
            $str= eval $str;
            $str = chr $str;
        } elsif ( $str =~ / \s* \\p \{ ( .*? ) \} /x) {
            my $property = $1;
            use Unicode::UCD qw(prop_invlist);

            my @invlist = prop_invlist($property, '_perl_core_internal_ok');
            if (! @invlist) {

                # An empty return could mean an unknown property, or merely
                # that it is empty.  Call in scalar context to differentiate
                my $count = prop_invlist($property, '_perl_core_internal_ok');
                die "$property not found" unless defined $count;
            }

            # Replace this element on the list with the property's expansion
            for (my $i = 0; $i < @invlist; $i += 2) {
                foreach my $cp ($invlist[$i] .. $invlist[$i+1] - 1) {

                    # prop_invlist() returns native values; add leading 'N'
                    # to indicate that.
                    push @{$opt{txt}}, sprintf "N0x%X", $cp;
                }
            }
            next;
        } elsif ($str =~ / ^ do \s+ ( .* ) /x) {
            die "do '$1' failed: $!$@" if ! do $1 or $@;
            next;
        } elsif ($str =~ / ^ & \s* ( .* ) /x) { # user-furnished sub() call
            my @results = eval "$1";
            die "eval '$1' failed: $@" if $@;
            push @{$opt{txt}}, @results;
            next;
        } elsif ($str =~ / ^ % \s* ( .* ) /x) { # user-furnished sub() call
            %hash_return = eval "$1";
            die "eval '$1' failed: $@" if $@;
            push @{$opt{txt}}, keys %hash_return;
            die "Only one multi character expansion currently allowed per rule"
                                                        if  $self->{multi_maps};
            next;
        } else {
            die "Unparsable line: $txt\n";
        }
        my ( $cp, $cp_high, $low, $latin1, $utf8 )
                                    = __uni_latin1($charset, $a2n{$charset}, $str );
        my $from;
        if (defined $hash_return{"\"$str\""}) {
            $from = $hash_return{"\"$str\""};
            $from = $a2n{$charset}->[$from] if $from < 256;
        }
        my $UTF8= $low   || $utf8;
        my $LATIN1= $low || $latin1;
        my $high = (scalar grep { $_ < 256 } @$cp) ? 0 : $utf8;
        #die Dumper($txt,$cp,$low,$latin1,$utf8)
        #    if $txt=~/NEL/ or $utf8 and @$utf8>3;

        @{ $self->{strs}{$str} }{qw( str txt low utf8 latin1 high cp cp_high UTF8 LATIN1 from )}=
          ( $str, $txt, $low, $utf8, $latin1, $high, $cp, $cp_high, $UTF8, $LATIN1, $from );
        my $rec= $self->{strs}{$str};
        foreach my $key ( qw(low utf8 latin1 high cp cp_high UTF8 LATIN1) ) {
            $self->{size}{$key}{ 0 + @{ $self->{strs}{$str}{$key} } }++
              if $self->{strs}{$str}{$key};
        }
        $self->{has_multi} ||= @$cp > 1;
        $self->{has_ascii} ||= $latin1 && @$latin1;
        $self->{has_low}   ||= $low && @$low;
        $self->{has_high}  ||= !$low && !$latin1;
    }
    $self->{n2a} = $n2a{$charset};
    $self->{count}= 0 + keys %{ $self->{strs} };
    return $self;
}

# my $trie = make_trie($type,$maxlen);
#
# using the data stored in the object build a trie of a specific type,
# and with specific maximum depth. The trie is made up the elements of
# the given types array for each string in the object (assuming it is
# not too long.)
#
# returns the trie, or undef if there was no relevant data in the object.
#

sub make_trie {
    my ( $self, $type, $maxlen, $backwards )= @_;

    my $strs= $self->{strs};
    my %trie;
    foreach my $rec ( values %$strs ) {
        die "panic: unknown type '$type'"
          if !exists $rec->{$type};
        my $dat= $rec->{$type};
        next unless $dat;
        next if $maxlen && @$dat > $maxlen;
        my $node= \%trie;
        my @ordered_dat = ($backwards) ? reverse @$dat : @$dat;
        foreach my $elem ( @ordered_dat ) {
            $node->{$elem} ||= {};
            $node= $node->{$elem};
        }
        $node->{''}= $rec->{str};
    }
    return 0 + keys( %trie ) ? \%trie : undef;
}

sub pop_count ($) {
    my $word = shift;

    # This returns a list of the positions of the bits in the input word that
    # are 1.

    my @positions;
    my $position = 0;
    while ($word) {
        push @positions, $position if $word & 1;
        $position++;
        $word >>= 1;
    }
    return @positions;
}

# my $optree= _optree()
#
# recursively convert a trie to an optree where every node represents
# an if else branch.
#
#

sub _optree {
    my ( $self, $trie, $test_type, $ret_type, $else, $depth, $backwards )= @_;
    return unless defined $trie;
    $ret_type ||= 'len';
    $else= 0  unless defined $else;
    $depth= 0 unless defined $depth;

    # if we have an empty string as a key it means we are in an
    # accepting state and unless we can match further on should
    # return the value of the '' key.
    if (exists $trie->{''} ) {
        # we can now update the "else" value, anything failing to match
        # after this point should return the value from this.
        my $prefix = $self->{strs}{ $trie->{''} };
        if ( $ret_type eq 'cp' ) {
            $else= $prefix->{from};
            $else= $self->{strs}{ $trie->{''} }{cp}[0] unless defined $else;
            $else= $self->val_fmt($else) if $else > 9;
        } elsif ( $ret_type eq 'len' ) {
            $else= $depth;
        } elsif ( $ret_type eq 'both') {
            $else= $prefix->{from};
            $else= $self->{strs}{ $trie->{''} }{cp}[0] unless defined $else;
            $else= $self->val_fmt($else) if $else > 9;
            $else= "len=$depth, $else";
        }
    }
    # extract the meaningful keys from the trie, filter out '' as
    # it means we are an accepting state (end of sequence).
    my @conds= sort { $a <=> $b } grep { length $_ } keys %$trie;

    # if we haven't any keys there is no further we can match and we
    # can return the "else" value.
    return $else if !@conds;

    my $test;
    if ($test_type =~ /^cp/) {
        $test = "cp";
    }
    elsif ($backwards) {
        $test = "*((const U8*)s - " . ($depth + 1) . ")";
    }
    else {
        $test = "((const U8*)s)[$depth]";
    }

    # First we loop over the possible keys/conditions and find out what they
    # look like; we group conditions with the same optree together.
    my %dmp_res;
    my @res_order;
    local $Data::Dumper::Sortkeys=1;
    foreach my $cond ( @conds ) {

        # get the optree for this child/condition
        my $res= $self->_optree( $trie->{$cond}, $test_type, $ret_type,
                                                $else, $depth + 1, $backwards );
        # convert it to a string with Dumper
        my $res_code= Dumper( $res );

        push @{$dmp_res{$res_code}{vals}}, $cond;
        if (!$dmp_res{$res_code}{optree}) {
            $dmp_res{$res_code}{optree}= $res;
            push @res_order, $res_code;
        }
    }

    # now that we have deduped the optrees we construct a new optree
    # containing the merged
    # results.
    my %root;
    my $node= \%root;
    foreach my $res_code_idx (0 .. $#res_order) {
        my $res_code= $res_order[$res_code_idx];
        $node->{vals}= $dmp_res{$res_code}{vals};
        $node->{test}= $test;
        $node->{yes}= $dmp_res{$res_code}{optree};
        $node->{depth}= $depth;
        if ($res_code_idx < $#res_order) {
            $node= $node->{no}= {};
        } else {
            $node->{no}= $else;
        }
    }

    # return the optree.
    return \%root;
}

# my $optree= optree(%opts);
#
# Convert a trie to an optree, wrapper for _optree

sub optree {
    my $self= shift;
    my %opt= @_;
    my $trie= $self->make_trie( $opt{type}, $opt{max_depth}, $opt{backwards} );
    $opt{ret_type} ||= 'len';
    my $test_type= $opt{type} =~ /^cp/ ? 'cp' : 'depth';
    return $self->_optree( $trie, $test_type, $opt{ret_type}, $opt{else}, 0,
                                                                    $opt{backwards} );
}

# my $optree= generic_optree(%opts);
#
# build a "generic" optree out of the three 'low', 'latin1', 'utf8'
# sets of strings, including a branch for handling the string type check.
#

sub generic_optree {
    my $self= shift;
    my %opt= @_;

    $opt{ret_type} ||= 'len';
    my $test_type= 'depth';
    my $else= $opt{else} || 0;

    my $latin1= $self->make_trie( 'latin1', $opt{max_depth}, $opt{backwards} );
    my $utf8= $self->make_trie( 'utf8',     $opt{max_depth}, $opt{backwards} );

    $_= $self->_optree( $_, $test_type, $opt{ret_type}, $else, 0, $opt{backwards} )
      for $latin1, $utf8;

    if ( $utf8 ) {
        $else= __cond_join( "( is_utf8 )", $utf8, $latin1 || $else );
    } elsif ( $latin1 ) {
        $else= __cond_join( "!( is_utf8 )", $latin1, $else );
    }
    if ($opt{type} eq 'generic') {
        my $low= $self->make_trie( 'low', $opt{max_depth}, $opt{backwards} );
        if ( $low ) {
            $else= $self->_optree( $low, $test_type, $opt{ret_type}, $else, 0,
                                                                    $opt{backwards} );
        }
    }

    return $else;
}

# length_optree()
#
# create a string length guarded optree.
#

sub length_optree {
    my $self= shift;
    my %opt= @_;
    my $type= $opt{type};

    die "Can't do a length_optree on type 'cp', makes no sense."
      if $type =~ /^cp/;

    my $else= ( $opt{else} ||= 0 );

    return $else if $self->{count} == 0;

    my $method = $type =~ /generic/ ? 'generic_optree' : 'optree';
    if ($method eq 'optree' && scalar keys %{$self->{size}{$type}} == 1) {

        # Here is non-generic output (meaning that we are only generating one
        # type), and all things that match have the same number ('size') of
        # bytes.  The length guard is simply that we have that number of
        # bytes.
        my @size = keys %{$self->{size}{$type}};
        my $cond= "((e) - (s)) >= $size[0]";
        my $optree = $self->$method(%opt);
        $else= __cond_join( $cond, $optree, $else );
    }
    elsif ($self->{has_multi}) {
        my @size;

        # Here, there can be a match of a multiple character string.  We use
        # the traditional method which is to have a branch for each possible
        # size (longest first) and test for the legal values for that size.
        my %sizes= (
            %{ $self->{size}{low}    || {} },
            %{ $self->{size}{latin1} || {} },
            %{ $self->{size}{utf8}   || {} }
        );
        if ($method eq 'generic_optree') {
            @size= sort { $a <=> $b } keys %sizes;
        } else {
            @size= sort { $a <=> $b } keys %{ $self->{size}{$type} };
        }
        for my $size ( @size ) {
            my $optree= $self->$method(%opt, type => $type, max_depth => $size);
            my $cond= "((e)-(s) > " . ( $size - 1 ).")";
            $else= __cond_join( $cond, $optree, $else );
        }
    }
    elsif ($opt{backwards}) {
        my @size= sort { $a <=> $b } keys %{ $self->{size}{$type} };
        for my $size ( @size ) {
            my $optree= $self->$method(%opt, type => $type, max_depth => $size);
            my $cond= "((s) - (e) > " . ( $size - 1 ).")";
            $else= __cond_join( $cond, $optree, $else );
        }
    }
    else {
        my $utf8;

        # Here, has more than one possible size, and only matches a single
        # character.  For non-utf8, the needed length is 1; for utf8, it is
        # found by array lookup 'UTF8SKIP'.

        # If want just the code points above 255, set up to look for those;
        # otherwise assume will be looking for all non-UTF-8-invariant code
        # poiints.
        my $trie_type = ($type eq 'high') ? 'high' : 'utf8';

        # If we do want more than the 0-255 range, find those, and if they
        # exist...
        if (   $opt{type} !~ /latin1/i
            && ($utf8 = $self->make_trie($trie_type, 0, $opt{backwards})))
        {

            # ... get them into an optree, and set them up as the 'else' clause
            $utf8 = $self->_optree( $utf8, 'depth', $opt{ret_type}, 0, 0,
                                                                    $opt{backwards} );

            # We could make this
            #   UTF8_IS_START(*s) && ((e) - (s)) >= UTF8SKIP(s))";
            # to avoid doing the UTF8SKIP and subsequent branches for invariants
            # that don't match.  But the current macros that get generated
            # have only a few things that can match past this, so I (khw)
            # don't think it is worth it.  (Even better would be to use
            # calculate_mask(keys %$utf8) instead of UTF8_IS_START, and use it
            # if it saves a bunch.  We assume that input text likely to be
            # well-formed .
            my $cond = "LIKELY(((e) - (s)) >= UTF8SKIP(s))";
            $else = __cond_join($cond, $utf8, $else);

            # For 'generic', we also will want the latin1 UTF-8 variants for
            # the case where the input isn't UTF-8.
            my $latin1;
            if ($method eq 'generic_optree') {
                $latin1 = $self->make_trie( 'latin1', 1, $opt{backwards});
                $latin1= $self->_optree($latin1, 'depth', $opt{ret_type}, 0, 0,
                                                                    $opt{backwards});
            }

            # If we want the UTF-8 invariants, get those.
            my $low;
            if ($opt{type} !~ /non_low|high/
                && ($low= $self->make_trie( 'low', 1, 0)))
            {
                $low= $self->_optree( $low, 'depth', $opt{ret_type}, 0, 0,
                                                                    $opt{backwards} );

                # Expand out the UTF-8 invariants as a string so that we
                # can use them as the conditional
                $low = $self->_cond_as_str( $low, 0, \%opt);

                # If there are Latin1 variants, add a test for them.
                if ($latin1) {
                    $else = __cond_join("(! is_utf8 )", $latin1, $else);
                }
                elsif ($method eq 'generic_optree') {

                    # Otherwise for 'generic' only we know that what
                    # follows must be valid for just UTF-8 strings,
                    $else->{test} = "( is_utf8 && $else->{test} )";
                }

                # If the invariants match, we are done; otherwise we have
                # to go to the 'else' clause.
                $else = __cond_join($low, 1, $else);
            }
            elsif ($latin1) {   # Here, didn't want or didn't have invariants,
                                # but we do have latin variants
                $else = __cond_join("(! is_utf8)", $latin1, $else);
            }

            # We need at least one byte available to start off the tests
            $else = __cond_join("LIKELY((e) > (s))", $else, 0);
        }
        else {  # Here, we don't want or there aren't any variants.  A single
                # byte available is enough.
            my $cond= "((e) > (s))";
            my $optree = $self->$method(%opt);
            $else= __cond_join( $cond, $optree, $else );
        }
    }

    return $else;
}

sub calculate_mask(@) {
    # Look at the input list of byte values.  This routine returns an array of
    # mask/base pairs to generate that list.

    my @list = @_;
    my $list_count = @list;

    # Consider a set of byte values, A, B, C ....  If we want to determine if
    # <c> is one of them, we can write c==A || c==B || c==C ....  If the
    # values are consecutive, we can shorten that to inRANGE(c, 'A', 'Z'),
    # which uses far fewer branches.  If only some of them are consecutive we
    # can still save some branches by creating range tests for just those that
    # are consecutive. _cond_as_str() does this work for looking for ranges.
    #
    # Another approach is to look at the bit patterns for A, B, C .... and see
    # if they have some commonalities.  That's what this function does.  For
    # example, consider a set consisting of the bytes
    # 0x42, 0x43, 0x62, and 0x63.  We could write:
    #   inRANGE(c, 0x42, 0x43) || inRANGE(c, 0x62, 0x63)
    # which through the magic of casting has not 4, but 2 tests.  But the
    # following mask/compare also works, and has just one test:
    #   (c & 0xDE) == 0x42
    # The reason it works is that the set consists of exactly the 4 bit
    # patterns which have either 0 or 1 in the two bit positions that are 0 in
    # the mask.  They have the same value in each bit position where the mask
    # is 1.  The comparison makes sure that the result matches all bytes which
    # match those six 1 bits exactly.  This can be applied to bytes that
    # differ in 1 through all 8 bit positions.  In order to be a candidate for
    # this optimization, the number of bytes in the set must be a power of 2.
    #
    # It may be that the bytes needing to be matched can't be done with a
    # single mask.  But it may be possible to have two (or more) sets, each
    # with a separate mask.  This function attempts to find some way to save
    # some branches using the mask technique.  If not, it returns an empty
    # list; if so, it returns a list consisting of
    #   [ [compare1, mask1], [compare2, mask2], ...
    #     [compare_n, undef], [compare_m, undef], ...
    #   ]
    # The <mask> is undef in the above for those bytes that must be tested
    # for individually.
    #
    # This function does not attempt to find the optimal set.  To do so would
    # probably require testing all possible combinations, and keeping track of
    # the current best one.
    #
    # There are probably much better algorithms, but this is the one I (khw)
    # came up with.  We start with doing a bit-wise compare of every byte in
    # the set with every other byte.  The results are sorted into arrays of
    # all those that differ by the same bit positions.  These are stored in a
    # hash with the each key being the bits they differ in.  Here is the hash
    # for the 0x53, 0x54, 0x73, 0x74 set:
    # {
    #    4 => {
    #            "0,1,2,5" => [
    #                            83,
    #                            116,
    #                            84,
    #                            115
    #                        ]
    #        },
    #    3 => {
    #            "0,1,2" => [
    #                        83,
    #                        84,
    #                        115,
    #                        116
    #                        ]
    #        }
    #    1 => {
    #            5 => [
    #                    83,
    #                    115,
    #                    84,
    #                    116
    #                ]
    #        },
    # }
    #
    # The set consisting of values which differ in the 4 bit positions 0, 1,
    # 2, and 5 from some other value in the set consists of all 4 values.
    # Likewise all 4 values differ from some other value in the 3 bit
    # positions 0, 1, and 2; and all 4 values differ from some other value in
    # the single bit position 5.  The keys at the uppermost level in the above
    # hash, 1, 3, and 4, give the number of bit positions that each sub-key
    # below it has.  For example, the 4 key could have as its value an array
    # consisting of "0,1,2,5", "0,1,2,6", and "3,4,6,7", if the inputs were
    # such.  The best optimization will group the most values into a single
    # mask.  The most values will be the ones that differ in the most
    # positions, the ones with the largest value for the topmost key.  These
    # keys, are thus just for convenience of sorting by that number, and do
    # not have any bearing on the core of the algorithm.
    #
    # We start with an element from largest number of differing bits.  The
    # largest in this case is 4 bits, and there is only one situation in this
    # set which has 4 differing bits, "0,1,2,5".  We look for any subset of
    # this set which has 16 values that differ in these 4 bits.  There aren't
    # any, because there are only 4 values in the entire set.  We then look at
    # the next possible thing, which is 3 bits differing in positions "0,1,2".
    # We look for a subset that has 8 values that differ in these 3 bits.
    # Again there are none.  So we go to look for the next possible thing,
    # which is a subset of 2**1 values that differ only in bit position 5.  83
    # and 115 do, so we calculate a mask and base for those and remove them
    # from every set.  Since there is only the one set remaining, we remove
    # them from just this one.  We then look to see if there is another set of
    # 2 values that differ in bit position 5.  84 and 116 do, so we calculate
    # a mask and base for those and remove them from every set (again only
    # this set remains in this example).  The set is now empty, and there are
    # no more sets to look at, so we are done.

    if ($list_count == 256) {   # All 256 is trivially masked
        return (0, 0);
    }

    my %hash;

    # Generate bits-differing lists for each element compared against each
    # other element
    for my $i (0 .. $list_count - 2) {
        for my $j ($i + 1 .. $list_count - 1) {
            my @bits_that_differ = pop_count($list[$i] ^ $list[$j]);
            my $differ_count = @bits_that_differ;
            my $key = join ",", @bits_that_differ;
            push @{$hash{$differ_count}{$key}}, $list[$i]
                unless grep { $_ == $list[$i] } @{$hash{$differ_count}{$key}};
            push @{$hash{$differ_count}{$key}}, $list[$j];
        }
    }

    print STDERR __LINE__, ": calculate_mask() called:  List of values grouped",
                                " by differing bits: ", Dumper \%hash if DEBUG;

    my @final_results;
    foreach my $count (reverse sort { $a <=> $b } keys %hash) {
        my $need = 2 ** $count;     # Need 8 values for 3 differing bits, etc
        foreach my $bits (sort keys $hash{$count}->%*) {

            print STDERR __LINE__, ": For $count bit(s) difference ($bits),",
            " need $need; have ", scalar @{$hash{$count}{$bits}}, "\n" if DEBUG;

            # Look only as long as there are at least as many elements in the
            # subset as are needed
            while ((my $cur_count = @{$hash{$count}{$bits}}) >= $need) {

                print STDERR __LINE__, ": Looking at bit positions ($bits): ",
                                          Dumper $hash{$count}{$bits} if DEBUG;

                # Start with the first element in it
                my $try_base = $hash{$count}{$bits}[0];
                my @subset = $try_base;

                # If it succeeds, we return a mask and a base to compare
                # against the masked value.  That base will be the AND of
                # every element in the subset.  Initialize to the one element
                # we have so far.
                my $compare = $try_base;

                # We are trying to find a subset of this that has <need>
                # elements that differ in the bit positions given by the
                # string $bits, which is comma separated.
                my @bits = split ",", $bits;

                TRY: # Look through the remainder of the list for other
                     # elements that differ only by these bit positions.

                for (my $i = 1; $i < $cur_count; $i++) {
                    my $try_this = $hash{$count}{$bits}[$i];
                    my @positions = pop_count($try_base ^ $try_this);

                    print STDERR __LINE__, ": $try_base vs $try_this: is (",
                      join(',', @positions), ") a subset of ($bits)?" if DEBUG;

                    foreach my $pos (@positions) {
                        unless (grep { $pos == $_ } @bits) {
                            print STDERR "  No\n" if DEBUG;
                            my $remaining = $cur_count - $i - 1;
                            if ($remaining && @subset + $remaining < $need) {
                                print STDERR __LINE__, ": Can stop trying",
                                    " $try_base, because even if all the",
                                    " remaining $remaining values work, they",
                                    " wouldn't add up to the needed $need when",
                                    " combined with the existing ",
                                            scalar @subset, " ones\n" if DEBUG;
                                last TRY;
                            }
                            next TRY;
                        }
                    }

                    print STDERR "  Yes\n" if DEBUG;
                    push @subset, $try_this;

                    # Add this to the mask base, in case it ultimately
                    # succeeds,
                    $compare &= $try_this;
                }

                print STDERR __LINE__, ": subset (", join(", ", @subset),
                 ") has ", scalar @subset, " elements; needs $need\n" if DEBUG;

                if (@subset < $need) {
                    shift @{$hash{$count}{$bits}};
                    next;   # Try with next value
                }

                # Create the mask
                my $mask = 0;
                foreach my $position (@bits) {
                    $mask |= 1 << $position;
                }
                $mask = ~$mask & 0xFF;
                push @final_results, [$compare, $mask];

                printf STDERR "%d: Got it: compare=%d=0x%X; mask=%X\n",
                                __LINE__, $compare, $compare, $mask if DEBUG;

                # These values are now spoken for.  Remove them from future
                # consideration
                foreach my $remove_count (sort keys %hash) {
                    foreach my $bits (sort keys %{$hash{$remove_count}}) {
                        foreach my $to_remove (@subset) {
                            @{$hash{$remove_count}{$bits}}
                                    = grep { $_ != $to_remove }
                                                @{$hash{$remove_count}{$bits}};
                        }
                    }
                }
            }
        }
    }

    # Any values that remain in the list are ones that have to be tested for
    # individually.
    my @individuals;
    foreach my $count (reverse sort { $a <=> $b } keys %hash) {
        foreach my $bits (sort keys $hash{$count}->%*) {
            foreach my $remaining (@{$hash{$count}{$bits}}) {

                # If we already know about this value, just ignore it.
                next if grep { $remaining == $_ } @individuals;

                # Otherwise it needs to be returned as something to match
                # individually
                push @final_results, [$remaining, undef];
                push @individuals, $remaining;
            }
        }
    }

    # Sort by increasing numeric value
    @final_results = sort { $a->[0] <=> $b->[0] } @final_results;

    print STDERR __LINE__, ": Final return: ", Dumper \@final_results if DEBUG;

    return @final_results;
}

# _cond_as_str
# turn a list of conditions into a text expression
# - merges ranges of conditions, and joins the result with ||
sub _cond_as_str {
    my ( $self, $op, $combine, $opts_ref )= @_;
    my @cond = ();
    @cond = $op->{vals}->@* if defined $op->{vals};
    my $test= $op->{test};
    my $is_cp_ret = $opts_ref->{ret_type} eq "cp";
    my $charset = $opts_ref->{charset};
    return "( $test )" unless @cond;

    my (@ranges, @native_ranges);
    my @native_conds;

    # rangify the list.  As we encounter a new value, it is placed in a new
    # subarray by itself.  If the next value is adjacent to it, the end point
    # of the subarray is merely incremented; and so on.  When the next value
    # that isn't adjacent to the previous one is encountered, Update() is
    # called to hoist any single-element subarray to be a scalar.
    my $Update= sub {
        # We skip this if there are optimizations that
        # we can apply (below) to the individual ranges
        if ( ($is_cp_ret || $combine) && @ranges && ref $ranges[-1]) {
            $ranges[-1] = $ranges[-1][0] if $ranges[-1][0] == $ranges[-1][1];
        }
    };

    # Parse things twice, using different approaches for representing things,
    # afterwards choosing the alternative with the fewest branches
    for my $i (0, 1) {

        # Should we avoid using mnemonics for code points?
        my $always_hex = 0;

        # The second pass is all about using a transformation to see if it
        # creates contiguous blocks that lead to fewer ranges or masking.  But
        # single element ranges don't have any benefit, and so the transform
        # is just extra work for them.  '$range_test' includes the transform
        # for multi-element ranges, and '$original' maps a byte back to what
        # it was without being transformed.  Thus we use '$range_test' and the
        # transormed bytes on multi-element ranges, and plain '$test' and
        # '$original' on single ones.  In the first pass these are effectively
        # no-ops.
        my $range_test = $test;
        my $original = \@identity;

        if ($i) {   # 2nd pass
            # The second pass is only for non-ascii character sets, to see if
            # a transform to Unicode/ASCII saves anything.
            last if $charset =~ /ascii/i;

            # If the first pass came up with a single range, we won't be able
            # to do better than that, so don't try.
            last if @ranges == 1;

            # We calculated the native values the first iteration
            @native_ranges = @ranges;
            @native_conds = @cond;

            # Start fresh
            undef @ranges;
            undef @cond;

            # Determine the translation function, to/from UTF-8 or Latin1, and
            # the corresponding transform of the condition to match
            my $lookup;
            if ($opts_ref->{type} =~ / ^ (?: utf8 | high ) $ /xi) {
                $lookup = $utf_2_I8{$charset};
                $original = $I8_2_utf{$charset};
                $range_test = "NATIVE_UTF8_TO_I8($test)";
            }
            else {
                $lookup = $n2a{$charset};
                $original = $a2n{$charset};
                $range_test = "NATIVE_TO_LATIN1($test)";
            }

            # Translate the native conditions (bytes) into the Unicode ones
            for my $condition (@native_conds) {
                push @cond, $lookup->[$condition];
            }

            # 'f' won't be the expected 'f' on this box
            $always_hex = 1;
        }

        # Go through the code points (@cond) and collapse them as much as
        # possible into ranges
        for my $condition ( @cond ) {
            if ( !@ranges || $condition != $ranges[-1][1] + 1 ) {
                # Not adjacent to the existing range.  Remove that from being a
                # range if only a single value;
                $Update->();
                push @ranges, [ $condition, $condition ];
            } else {    # Adjacent to the existing range; add to the range
                $ranges[-1][1]++;
            }
        }
        $Update->();

        # _combine is used for cp type matching.  By having it here return, no
        # second pass is done.  It could conceivably be restructured to have a
        # second pass, but no current uses of script would actually gain any
        # advantage by doing so, so the work hasn't been further considered.
        return $self->_combine( $test, @ranges ) if $combine;

        # If the input set has certain characteristics, we can optimize tests
        # for it.

        # If all bytes match, is trivially true; we don't need a 2nd pass
        return 1 if @cond == 256;

        # If this is a single UTF-8 range which includes all possible
        # continuation bytes, and we aren't checking for well-formedness, this
        # is trivially true.
        #
        # (In EBCDIC, this won't happen until the 2nd pass transforms the
        # disjoint continuation byte ranges into a single I8 one.)
        if (     @ranges == 1
            && ! $opts_ref->{safe}
            && ! $opts_ref->{no_length_checks}
            &&   $opts_ref->{type} =~ / ^ (?: utf8 | high ) $ /xi
            &&   $ranges[0]->[1] == 0xBF
            &&   $ranges[0]->[0] == (($charset =~ /ascii/i)
                                        ? 0x80 : 0xA0))
        {
            return 1;
        }

        my $loop_start = 0;
        if (ref $ranges[0] && $ranges[0]->[0] == 0) {

            # If the first range matches all 256 possible bytes, it is
            # trivially true.
            if ($ranges[0]->[1] == 0xFF) {
                die "Range spanning all bytes must be the only one"
                                                                if @ranges > 1;
                return 1;
            }

            # Here, the first range starts at 0, but doesn't match everything.
            # But the condition doesn't have to worry about being < 0
            $ranges[0] = "( $test <= "
                        . $self->val_fmt($ranges[0]->[1], $always_hex) . " )";
            $loop_start++;
        }

        my $loop_end = @ranges;
        if (   @ranges
            && ref $ranges[-1]
            && $ranges[-1]->[1] == 0xFF
            && $ranges[-1]->[0] != 0xFF)
        {
            # If the final range consists of more than one byte ending with
            # the highest possible one, the condition doesn't have to worry
            # about being > FF
            $ranges[-1] = "( $test >= "
                        . $self->val_fmt($ranges[-1]->[0], $always_hex) . " )";
            $loop_end--;
        }

        # Look at each range to see if there any optimizations.  The
        # formatting may be thrown away, so might be wasted effort; and khw
        # supposes this could be restructured to delay that until the final
        # method is chosen.  But that would be more coding work than
        # warranted, as this is executed not that many times during a
        # development cycle.
        for (my $i = $loop_start; $i < $loop_end; $i++) {
            if (! ref $ranges[$i]) {    # Trivial case: no range
                $ranges[$i] =
                    $self->val_fmt($original->[$ranges[$i]], $always_hex)
                  . " == $test";
            }
            elsif ($ranges[$i]->[0] == $ranges[$i]->[1]) {
                $ranges[$i] =           # Trivial case: single element range
                     $self->val_fmt($original->[$ranges[$i]->[0]], $always_hex)
                   . " == $test";
            }
            else {
                $ranges[$i] = "inRANGE_helper_(U8, $range_test, "
                        . $self->val_fmt($ranges[$i]->[0], $always_hex) .", "
                        . $self->val_fmt($ranges[$i]->[1], $always_hex) . ")";
            }
        }

        # Here, have collapsed the matched code points into ranges.  This code
        # also sees if some of those different ranges have bit patterns which
        # causes them to be combinable by ANDing with a mask.  There's no need
        # to do this if we are already down to a single range.
        next unless @ranges > 1;

        my @masks = calculate_mask(@cond);

        # Stringify the output of calculate_mask()
        if (@masks) {
            my @masked;
            foreach my $mask_ref (@masks) {
                if (defined $mask_ref->[1]) {
                    push @masked, "( ( $range_test & "
                        . $self->val_fmt($mask_ref->[1], $always_hex) . " ) == "
                        . $self->val_fmt($mask_ref->[0], $always_hex) . " )";
                }
                else {  # An undefined mask means to use the value as-is
                    push @masked, "$test == "
                    . $self->val_fmt($original->[$mask_ref->[0]], $always_hex);
                }
            }

            # The best possible case below for specifying this set of values via
            # ranges is 1 branch per range.  If our mask method yielded better
            # results, there is no sense trying something that is bound to be
            # worse.
            if (@masked < @ranges) {
                @ranges = @masked;
                next;
            }

            @masks = @masked;
        }

        # If we found some mask possibilities, and they have fewer
        # conditionals in them than the plain range method, convert to use the
        # masks.
        @ranges = @masks if @masks && @masks < @ranges;
    }  # End of both passes

    # If the two passes came up with two sets, use the one with the fewest
    # conditionals (the number of ranges is a proxy for that).  If both have
    # the same number, prefer the native, as that omits transformations.
    if (@native_ranges && @native_ranges <= @ranges) {
        @ranges = @native_ranges;
        @cond = @native_conds;
    }

    return "( " . join( " || ", @ranges) . " )";
}

# _combine
# recursively turn a list of conditions into a fast break-out condition
# used by _cond_as_str() for 'cp' type macros.
sub _combine {
    my ( $self, $test, @cond )= @_;
    return if !@cond;
    my $item= shift @cond;
    my ( $cstr, $gtv );
    if ( ref $item ) {  # @item should be a 2-element array giving range start
                        # and end
        if ($item->[0] == 0) {  # UV's are never negative, so skip "0 <= "
                                # test which could generate a compiler warning
                                # that test is always true
            $cstr= "$test <= " . $self->val_fmt($item->[1]);
        }
        else {
            $cstr = "inRANGE_helper_(UV, $test, "
                  . $self->val_fmt($item->[0]) . ", "
                  . $self->val_fmt($item->[1]) . ")";
        }
        $gtv= $self->val_fmt($item->[1]);
    } else {
        $cstr= $self->val_fmt($item) . " == $test";
        $gtv= $self->val_fmt($item)
    }
    if ( @cond ) {
        my $combine= $self->_combine( $test, @cond );
        if (@cond >1) {
            return "( $cstr || ( $gtv < $test &&\n"
                   . $combine . " ) )";
        } else {
            return "( $cstr || $combine )";
        }
    } else {
        return $cstr;
    }
}

# _render()
# recursively convert an optree to text with reasonably neat formatting
sub _render {
    my ( $self, $op, $combine, $brace, $opts_ref, $def, $submacros )= @_;
    return 0 if ! defined $op;  # The set is empty
    if ( !ref $op ) {
        return $op;
    }
    my $cond= $self->_cond_as_str( $op, $combine, $opts_ref );
    #no warnings 'recursion';   # This would allow really really inefficient
                                # code to be generated.  See pod
    my $yes= $self->_render( $op->{yes}, $combine, 1, $opts_ref, $def,
                                                                    $submacros);
    return $yes if $cond eq '1';

    my $no= $self->_render( $op->{no},   $combine, 0, $opts_ref, $def,
                                                                    $submacros);
    return "( $cond )" if $yes eq '1' and $no eq '0';
    my ( $lb, $rb )= $brace ? ( "( ", " )" ) : ( "", "" );
    return "$lb$cond ? $yes : $no$rb"
      if !ref( $op->{yes} ) && !ref( $op->{no} );
    my $ind1= " " x 4;
    my $ind= "\n" . ( $ind1 x $op->{depth} );

    if ( ref $op->{yes} ) {
        $yes= $ind . $ind1 . $yes;
    } else {
        $yes= " " . $yes;
    }

    my $str= "$lb$cond ?$yes$ind: $no$rb";
    if (length $str > 6000) {
        push @$submacros, sprintf "#define $def\n( %s )", "_part"
                                  . (my $yes_idx= 0+@$submacros) . "_", $yes;
        push @$submacros, sprintf "#define $def\n( %s )", "_part"
                                  . (my $no_idx= 0+@$submacros) . "_", $no;
        return sprintf "%s%s ? $def : $def%s", $lb, $cond,
                                    "_part${yes_idx}_", "_part${no_idx}_", $rb;
    }
    return $str;
}

# $expr=render($op,$combine)
#
# convert an optree to text with reasonably neat formatting. If $combine
# is true then the condition is created using "fast breakouts" which
# produce uglier expressions that are more efficient for common case,
# longer lists such as that resulting from type 'cp' output.
# Currently only used for type 'cp' macros.
sub render {
    my ( $self, $op, $combine, $opts_ref, $def_fmt )= @_;

    my @submacros;
    my $macro= sprintf "#define $def_fmt\n( %s )", "",
                       $self->_render( $op, $combine, 0, $opts_ref, $def_fmt,
                                                                 \@submacros);

    return join "\n\n",
            map { "/*** GENERATED CODE ***/\n" . __macro( __clean( $_ ) ) }
                                                            @submacros, $macro;
}

# make_macro
# make a macro of a given type.
# calls into make_trie and (generic_|length_)optree as needed
# Opts are:
# type             : 'cp', 'cp_high', 'generic', 'high', 'low', 'latin1',
#                    'utf8', 'LATIN1', 'UTF8' 'backwards_UTF8'
# ret_type         : 'cp' or 'len'
# safe             : don't assume is well-formed UTF-8, so don't skip any range
#                    checks, and add length guards to macro
# no_length_checks : like safe, but don't add length guards.
#
# type defaults to 'generic', and ret_type to 'len' unless type is 'cp'
# in which case it defaults to 'cp' as well.
#
# It is illegal to do a type 'cp' macro on a pattern with multi-codepoint
# sequences in it, as the generated macro will accept only a single codepoint
# as an argument.
#
# It is also illegal to do a non-safe macro on a pattern with multi-codepoint
# sequences in it, as even if it is known to be well-formed, we need to not
# run off the end of the buffer when, say, the buffer ends with the first two
# characters, but three are looked at by the macro.
#
# returns the macro.


sub make_macro {
    my $self= shift;
    my %opts= @_;
    my $type= $opts{type} || 'generic';
    if ($self->{has_multi}) {
        if ($type =~ /^cp/) {
            die "Can't do a 'cp' on multi-codepoint character class"
              . " '$self->{op}'"
        }
        elsif (! $opts{safe}) {
            die "'safe' is required on multi-codepoint character class"
               ." '$self->{op}'"
        }
    }
    my $ret_type= $opts{ret_type} || ( $opts{type} =~ /^cp/ ? 'cp' : 'len' );
    my $method;
    if ( $opts{safe} ) {
        $method= 'length_optree';
    } elsif ( $type =~ /generic/ ) {
        $method= 'generic_optree';
    } else {
        $method= 'optree';
    }
    my @args= $type =~ /^cp/ ? 'cp' : 's';
    push @args, "e" if $opts{safe};
    push @args, "is_utf8" if $type =~ /generic/;
    push @args, "len" if $ret_type eq 'both';
    my $pfx= $ret_type eq 'both'    ? 'what_len_' :
             $ret_type eq 'cp'      ? 'what_'     : 'is_';
    my $ext= $type     =~ /generic/ ? ''          : '_' . lc( $type );
    $ext .= '_non_low' if $type eq 'generic_non_low';
    $ext .= "_safe" if $opts{safe};
    $ext .= "_no_length_checks" if $opts{no_length_checks};
    $ext .= "_backwards" if $opts{backwards};
    my $argstr= join ",", @args;
    my $def_fmt="$pfx$self->{op}$ext%s($argstr)";
    my $optree= $self->$method( %opts, type => $type, ret_type => $ret_type );
    return $self->render( $optree, ($type =~ /^cp/) ? 1 : 0, \%opts, $def_fmt );
}

# if we aren't being used as a module (highly likely) then process
# the __DATA__ below and produce macros in regcharclass.h
# if an argument is provided to the script then it is assumed to
# be the path of the file to output to, if the arg is '-' outputs
# to STDOUT.
if ( !caller ) {
    $|++;
    my $path= shift @ARGV || "regcharclass.h";
    my $out_fh;
    if ( $path eq '-' ) {
        $out_fh= \*STDOUT;
    } else {
        $out_fh = open_new( $path );
    }
    print $out_fh read_only_top( lang => 'C', by => $0,
                                 file => 'regcharclass.h', style => '*',
                                 copyright => [2007, 2011],
                                 final => <<EOF,
WARNING: These macros are for internal Perl core use only, and may be
changed or removed without notice.
EOF
    );
    print $out_fh "\n#ifndef PERL_REGCHARCLASS_H_ /* Guard against nested",
                  " #includes */\n#define PERL_REGCHARCLASS_H_\n";

    my ( $op, $title, @txt, @types, %mods );
    my $doit= sub ($) {
        return unless $op;

        my $charset = shift;

        # Skip if to compile on a different platform.
        return if delete $mods{only_ascii_platform} && $charset !~ /ascii/i;
        return if delete $mods{only_ebcdic_platform} && $charset !~ /ebcdic/i;

        print $out_fh "/*\n\t$op: $title\n\n";
        print $out_fh join "\n", ( map { "\t$_" } @txt ), "*/", "";
        my $obj= __PACKAGE__->new( op => $op, title => $title, txt => \@txt,
                                                        charset => $charset);

        #die Dumper(\@types,\%mods);

        my @mods;
        push @mods, 'safe' if delete $mods{safe};
        push @mods, 'no_length_checks' if delete $mods{no_length_checks};

        # Default to 'fast' do this one first, as traditional
        unshift @mods, 'fast' if delete $mods{fast} || ! @mods;
        if (%mods) {
            die "Unknown modifiers: ", join ", ", map { "'$_'" } sort keys %mods;
        }

        foreach my $type_spec ( @types ) {
            my ( $type, $ret )= split /-/, $type_spec;
            $ret ||= 'len';

            my $backwards = 0;
            if ($type eq 'backwards_UTF8') {
                $type = 'UTF8';
                $backwards = 1;
            }

            foreach my $mod ( @mods ) {

                # 'safe' is irrelevant with code point macros, so skip if
                # there is also a 'fast', but don't skip if this is the only
                # way a cp macro will get generated.  Below we convert 'safe'
                # to 'fast' in this instance
                next if $type =~ /^cp/
                        && ($mod eq 'safe' || $mod eq 'no_length_checks')
                        && grep { 'fast' =~ $_ } @mods;
                delete $mods{$mod};
                my $macro= $obj->make_macro(
                    type     => $type,
                    ret_type => $ret,
                    safe     => $mod eq 'safe' && $type !~ /^cp/,
                    charset  => $charset,
                    no_length_checks => $mod eq 'no_length_checks'
                                     && $type !~ /^cp/,
                    backwards => $backwards,
                );
                print $out_fh $macro, "\n";
            }
        }
    };

    my @data = <DATA>;
    foreach my $charset (get_supported_code_pages()) {
        my $first_time = 1;
        undef $op;
        undef $title;
        undef @txt;
        undef @types;
        undef %mods;
        print $out_fh "\n", get_conditional_compile_line_start($charset);
        my @data_copy = @data;
        for (@data_copy) {
            s/^ \s* (?: \# .* ) ? $ //x;    # squeeze out comment and blanks
            next unless /\S/;
            chomp;
            if ( /^[A-Z]/ ) {
                $doit->($charset) unless $first_time;  # This starts a new
                                                       # definition; do the
                                                       # previous one
                $first_time = 0;
                ( $op, $title )= split /\s*:\s*/, $_, 2;
                @txt= ();
            } elsif ( s/^=>// ) {
                my ( $type, $modifier )= split /:/, $_;
                @types= split ' ', $type;
                undef %mods;
                map { $mods{$_} = 1 } split ' ',  $modifier;
            } else {
                push @txt, "$_";
            }
        }
        $doit->($charset);
        print $out_fh get_conditional_compile_line_end();
    }

    print $out_fh "\n#endif /* PERL_REGCHARCLASS_H_ */\n";

    if($path eq '-') {
        print $out_fh "/* ex: set ro: */\n";
    } else {
        # Some of the sources for these macros come from Unicode tables
        my $sources_list = "lib/unicore/mktables.lst";
        my @sources = ($0, qw(lib/unicore/mktables
                              lib/Unicode/UCD.pm
                              regen/regcharclass_multi_char_folds.pl
                              regen/charset_translations.pl
                             ));
        {
            # Depend on mktables’ own sources.  It’s a shorter list of files than
            # those that Unicode::UCD uses.
            if (! open my $mktables_list, '<', $sources_list) {

                # This should force a rebuild once $sources_list exists
                push @sources, $sources_list;
            }
            else {
                while(<$mktables_list>) {
                    last if /===/;
                    chomp;
                    push @sources, "lib/unicore/$_" if /^[^#]/;
                }
            }
        }
        read_only_bottom_close_and_rename($out_fh, \@sources)
    }
}

# The form of the input is a series of definitions to make macros for.
# The first line gives the base name of the macro, followed by a colon, and
# then text to be used in comments associated with the macro that are its
# title or description.  In all cases the first (perhaps only) parameter to
# the macro is a pointer to the first byte of the code point it is to test to
# see if it is in the class determined by the macro.  In the case of non-UTF8,
# the code point consists only of a single byte.
#
# The second line must begin with a '=>' and be followed by the types of
# macro(s) to be generated; these are specified below.  A colon follows the
# types, followed by the modifiers, also specified below.  At least one
# modifier is required.
#
# The subsequent lines give what code points go into the class defined by the
# macro.  Multiple characters may be specified via a string like "\x0D\x0A",
# enclosed in quotes.  Otherwise the lines consist of one of:
#   1)  a single Unicode code point, prefaced by 0x
#   2)  a single range of Unicode code points separated by a minus (and
#       optional space)
#   3)  a single Unicode property specified in the standard Perl form
#       "\p{...}"
#   4)  a line like 'do path'.  This will do a 'do' on the file given by
#       'path'.  It is assumed that this does nothing but load subroutines
#       (See item 5 below).  The reason 'require path' is not used instead is
#       because 'do' doesn't assume that path is in @INC.
#   5)  a subroutine call
#           &pkg::foo(arg1, ...)
#       where pkg::foo was loaded by a 'do' line (item 4).  The subroutine
#       returns an array of entries of forms like items 1-3 above.  This
#       allows more complex inputs than achievable from the other input types.
#
# A blank line or one whose first non-blank character is '#' is a comment.
# The definition of the macro is terminated by a line unlike those described.
#
# Valid types:
#   low         generate a macro whose name is 'is_BASE_low' and defines a
#               class that includes only ASCII-range chars.  (BASE is the
#               input macro base name.)
#   latin1      generate a macro whose name is 'is_BASE_latin1' and defines a
#               class that includes only upper-Latin1-range chars.  It is not
#               designed to take a UTF-8 input parameter.
#   high        generate a macro whose name is 'is_BASE_high' and defines a
#               class that includes all relevant code points that are above
#               the Latin1 range.  This is for very specialized uses only.
#               It is designed to take only an input UTF-8 parameter.
#   utf8        generate a macro whose name is 'is_BASE_utf8' and defines a
#               class that includes all relevant characters that aren't ASCII.
#               It is designed to take only an input UTF-8 parameter.
#   LATIN1      generate a macro whose name is 'is_BASE_latin1' and defines a
#               class that includes both ASCII and upper-Latin1-range chars.
#               It is not designed to take a UTF-8 input parameter.
#   UTF8        generate a macro whose name is 'is_BASE_utf8' and defines a
#               class that can include any code point, adding the 'low' ones
#               to what 'utf8' works on.  It is designed to take only an input
#               UTF-8 parameter.
#   backwards_UTF8  like 'UTF8', but designed to match backwards, so that the
#               second parameter to the function is earlier in the string than
#               the first.
#   generic     generate a macro whose name is 'is_BASE".  It has a 2nd,
#               boolean, parameter which indicates if the first one points to
#               a UTF-8 string or not.  Thus it works in all circumstances.
#   generic_non_low generate a macro whose name is 'is_BASE_non_low".  It has
#               a 2nd, boolean, parameter which indicates if the first one
#               points to a UTF-8 string or not.  It excludes any ASCII-range
#               matches, but otherwise it works in all circumstances.
#   cp          generate a macro whose name is 'is_BASE_cp' and defines a
#               class that returns true if the UV parameter is a member of the
#               class; false if not.
#   cp_high     like cp, but it is assumed that it is known that the UV
#               parameter is above Latin1.  The name of the generated macro is
#               'is_BASE_cp_high'.  This is different from high-cp, derived
#               below.
# A macro of the given type is generated for each type listed in the input.
# The default return value is the number of octets read to generate the match.
# Append "-cp" to the type to have it instead return the matched codepoint.
#               The macro name is changed to 'what_BASE...'.  See pod for
#               caveats
# Appending '-both" instead adds an extra parameter to the end of the argument
#               list, which is a pointer as to where to store the number of
#               bytes matched, while also returning the code point.  The macro
#               name is changed to 'what_len_BASE...'.  See pod for caveats
#
# Valid modifiers:
#   safe        The input string is not necessarily valid UTF-8.  In
#               particular an extra parameter (always the 2nd) to the macro is
#               required, which points to one beyond the end of the string.
#               The macro will make sure not to read off the end of the
#               string.  In the case of non-UTF8, it makes sure that the
#               string has at least one byte in it.  The macro name has
#               '_safe' appended to it.
#   no_length_checks  The input string is not necessarily valid UTF-8, but it
#               is to be assumed that the length has already been checked and
#               found to be valid
#   fast        The input string is valid UTF-8.  No bounds checking is done,
#               and the macro can make assumptions that lead to faster
#               execution.
#   only_ascii_platform   Skip this definition if the character set is for
#               a non-ASCII platform.
#   only_ebcdic_platform  Skip this definition if the character set is for
#               a non-EBCDIC platform.
# No modifier need be specified; fast is assumed for this case.  If both
# 'fast', and 'safe' are specified, two macros will be created for each
# 'type'.
#
# If run on a non-ASCII platform will automatically convert the Unicode input
# to native.  The documentation above is slightly wrong in this case.  'low'
# actually refers to code points whose UTF-8 representation is the same as the
# non-UTF-8 version (invariants); and 'latin1' refers to all the rest of the
# code points less than 256.

1; # in the unlikely case we are being used as a module

__DATA__
# This is no longer used, but retained in case it is needed some day.
# TRICKYFOLD: Problematic fold case letters.  When adding to this list, also should add them to regcomp.c and fold_grind.t
# => generic cp generic-cp generic-both :fast safe
# 0x00DF	# LATIN SMALL LETTER SHARP S
# 0x0390	# GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
# 0x03B0	# GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
# 0x1E9E  # LATIN CAPITAL LETTER SHARP S, because maps to same as 00DF
# 0x1FD3  # GREEK SMALL LETTER IOTA WITH DIALYTIKA AND OXIA; maps same as 0390
# 0x1FE3  # GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND OXIA; maps same as 03B0

LNBREAK: Line Break: \R
=> generic UTF8 LATIN1 : safe
"\x0D\x0A"      # CRLF - Network (Windows) line ending
\p{VertSpace}

HORIZWS: Horizontal Whitespace: \h \H
=> high cp_high : fast
\p{HorizSpace}

VERTWS: Vertical Whitespace: \v \V
=> high cp_high : fast
\p{VertSpace}

XDIGIT: Hexadecimal digits
=> high cp_high : fast
\p{XDigit}

XPERLSPACE: \p{XPerlSpace}
=> high cp_high : fast
\p{XPerlSpace}

SPACE: Backwards \p{XPerlSpace}
=> backwards_UTF8 : safe
\p{XPerlSpace}

NONCHAR: Non character code points
=> UTF8 :safe
\p{_Perl_Nchar}

SHORTER_NON_CHARS:  # 3 bytes
=> UTF8 :only_ascii_platform fast
0xFDD0 - 0xFDEF
0xFFFE - 0xFFFF

LARGER_NON_CHARS:   # 4 bytes
=> UTF8 :only_ascii_platform fast
0x1FFFE - 0x1FFFF
0x2FFFE - 0x2FFFF
0x3FFFE - 0x3FFFF
0x4FFFE - 0x4FFFF
0x5FFFE - 0x5FFFF
0x6FFFE - 0x6FFFF
0x7FFFE - 0x7FFFF
0x8FFFE - 0x8FFFF
0x9FFFE - 0x9FFFF
0xAFFFE - 0xAFFFF
0xBFFFE - 0xBFFFF
0xCFFFE - 0xCFFFF
0xDFFFE - 0xDFFFF
0xEFFFE - 0xEFFFF
0xFFFFE - 0xFFFFF
0x10FFFE - 0x10FFFF

SHORTER_NON_CHARS:  # 4 bytes
=> UTF8 :only_ebcdic_platform fast
0xFDD0 - 0xFDEF
0xFFFE - 0xFFFF
0x1FFFE - 0x1FFFF
0x2FFFE - 0x2FFFF
0x3FFFE - 0x3FFFF

LARGER_NON_CHARS:   # 5 bytes
=> UTF8 :only_ebcdic_platform fast
0x4FFFE - 0x4FFFF
0x5FFFE - 0x5FFFF
0x6FFFE - 0x6FFFF
0x7FFFE - 0x7FFFF
0x8FFFE - 0x8FFFF
0x9FFFE - 0x9FFFF
0xAFFFE - 0xAFFFF
0xBFFFE - 0xBFFFF
0xCFFFE - 0xCFFFF
0xDFFFE - 0xDFFFF
0xEFFFE - 0xEFFFF
0xFFFFE - 0xFFFFF
0x10FFFE - 0x10FFFF

# Note that code in utf8.c is counting on the 'fast' version to look at no
# more than two bytes
SURROGATE: Surrogate code points
=> UTF8 :safe fast
\p{_Perl_Surrogate}

QUOTEMETA: Meta-characters that \Q should quote
=> high :fast
\p{_Perl_Quotemeta}

MULTI_CHAR_FOLD: multi-char strings that are folded to by a single character
=> UTF8 UTF8-cp :safe
%regcharclass_multi_char_folds::multi_char_folds('u', 'a')

MULTI_CHAR_FOLD: multi-char strings that are folded to by a single character
=> LATIN1 LATIN1-cp : safe
%regcharclass_multi_char_folds::multi_char_folds('l', 'a')

THREE_CHAR_FOLD: A three-character multi-char fold
=> UTF8 :safe
%regcharclass_multi_char_folds::multi_char_folds('u', '3')

THREE_CHAR_FOLD: A three-character multi-char fold
=> LATIN1 :safe
%regcharclass_multi_char_folds::multi_char_folds('l', '3')

THREE_CHAR_FOLD_HEAD: The first two of three-character multi-char folds
=> UTF8 :safe
%regcharclass_multi_char_folds::multi_char_folds('u', 'h')

THREE_CHAR_FOLD_HEAD: The first two of three-character multi-char folds
=> LATIN1 :safe
%regcharclass_multi_char_folds::multi_char_folds('l', 'h')
#
#THREE_CHAR_FOLD_NON_FINAL: The first or middle character of multi-char folds
#=> UTF8 :safe
#%regcharclass_multi_char_folds::multi_char_folds('u', 'fm')
#
#THREE_CHAR_FOLD_NON_FINAL: The first or middle character of multi-char folds
#=> LATIN1 :safe
#%regcharclass_multi_char_folds::multi_char_folds('l', 'fm')

FOLDS_TO_MULTI: characters that fold to multi-char strings
=> UTF8 :fast
\p{_Perl_Folds_To_Multi_Char}

PROBLEMATIC_LOCALE_FOLD : characters whose fold is problematic under locale
=> UTF8 cp :fast
\p{_Perl_Problematic_Locale_Folds}

PROBLEMATIC_LOCALE_FOLDEDS_START : The first folded character of folds which are problematic under locale
=> UTF8 cp :fast
\p{_Perl_Problematic_Locale_Foldeds_Start}

PATWS: pattern white space
=> generic : safe
\p{_Perl_PatWS}

HANGUL_ED: Hangul syllables whose first UTF-8 byte is \xED
=> UTF8 :only_ascii_platform safe
0xD000 - 0xD7FF

HANGUL_ED: Hangul syllables whose first UTF-8 byte is \xED
=> UTF8 :only_ebcdic_platform safe
0x1 - 0x0
# Alows fails on EBCDIC; there are no ED Hanguls there
