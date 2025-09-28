use strict;
use warnings;

# A tiny private library routine which is a helper to several Perl core
# modules, to allow a paradigm to be implemented in a single place.  The name,
# contents, or even the existence of this file may be changed at any time and
# are NOT to be used by anything outside the Perl core.

sub _meta_notation ($) {

    # Returns a copy of the input string with the nonprintable characters
    # below 0x100 changed into printables.  Any ASCII printables or above 0xFF
    # are unchanged.  (XXX Probably above-Latin1 characters should be
    # converted to \X{...})
    #
    # \0 .. \x1F (which are "\c@" .. "\c_") are changed into ^@, ^A, ^B, ...
    # ^Z, ^[, ^\, ^], ^^, ^_
    # \c? is changed into ^?.
    #
    # The above accounts for all the ASCII-range nonprintables.
    #
    # On ASCII platforms, the upper-Latin1-range characters are converted to
    # Meta notation, so that \xC1 becomes 'M-A', \xE2 becomes 'M-b', etc.
    # This is how it always has worked, so is continued that way for backwards
    # compatibility.  The range \x80 .. \x9F becomes M-^@ .. M-^A, M-^B, ...
    # M-^Z, M-^[, M-^\, M-^], M-^, M-^_
    #
    # On EBCDIC platforms, the upper-Latin1-range characters are converted
    # into '\x{...}'  Meta notation doesn't make sense on EBCDIC platforms
    # because the ASCII-range printables are a mixture of upper bit set or
    # not.  [A-Za-Z0-9] all have the upper bit set.  The underscore likely
    # doesn't; and other punctuation may or may not.  There's no simple
    # pattern.

    my $string = shift;

    $string =~ s/([\0-\037])/
               sprintf("^%c",utf8::unicode_to_native(ord($1)^64))/xeg;
    $string =~ s/\c?/^?/g;
    if (ord("A") == 65) {
        $string =~ s/([\200-\237])/sprintf("M-^%c",(ord($1)&0177)^64)/eg;
        $string =~ s/([\240-\377])/sprintf("M-%c"  ,ord($1)&0177)/eg;
    }
    else {
        # Leave alone things above \xff
        $string =~ s/( (?[ [\x00-\xFF] & [:^print:]])) /
                  sprintf("\\x{%X}", ord($1))/xaeg;
    }

    return $string;
}
1
