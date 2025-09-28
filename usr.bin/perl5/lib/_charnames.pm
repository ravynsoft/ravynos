# !!!!!!!   INTERNAL PERL USE ONLY   !!!!!!!
# This helper module is for internal use by core Perl only.  This module is
# subject to change or removal at any time without notice.  Don't use it
# directly.  Use the public <charnames> module instead.

package _charnames;
use strict;
use warnings;
our $VERSION = '1.50';
use unicore::Name;    # mktables-generated algorithmically-defined names

use bytes ();          # for $bytes::hint_bits
use re "/aa";          # Everything in here should be ASCII

$Carp::Internal{ (__PACKAGE__) } = 1;

# Translate between Unicode character names and their code points.  This is a
# submodule of package <charnames>, used to allow \N{...} to be autoloaded,
# but it was decided not to autoload the various functions in charnames; the
# splitting allows this behavior.
#
# The official names with their code points are stored in a table in
# lib/unicore/Name.pl which is read in as a large string (almost 3/4 Mb in
# Unicode 6.0).  Each code point sequence appears on a line by itself, with
# its corresponding name occupying the next line in the string.  (Some of the
# CJK and the Hangul syllable names are instead determined algorithmically via
# subroutines stored instead in lib/unicore/Name.pm).  Because of the large
# size of this table, it isn't converted into hashes for faster lookup.
#
# But, user defined aliases are stored in their own hashes, as are Perl
# extensions to the official names.  These are checked first before looking at
# the official table.
#
# Basically, the table is grepped for the input code point (viacode()) or
# name (the other functions), and the corresponding value on the next or
# previous line is returned.  The grepping is done by turning the input into a
# regular expression.  Thus, the same table does double duty, used by both
# name and code point lookup.  (If we were to have hashes, we would need two,
# one for each lookup direction.)
#
# For loose name matching, the logical thing would be to have a table
# with all the ignorable characters squeezed out, and then grep it with the
# similiarly-squeezed input name.  (And this is in fact how the lookups are
# done with the small Perl extension hashes.)  But since we need to be able to
# go from code point to official name, the original table would still need to
# exist.  Due to the large size of the table, it was decided to not read
# another very large string into memory for a second table.  Instead, the
# regular expression of the input name is modified to have optional spaces and
# dashes between characters.  For example, in strict matching, the regular
# expression would be:
#   qr/^DIGIT ONE$/m
# Under loose matching, the blank would be squeezed out, and the re would be:
#   qr/^D[- ]?I[- ]?G[- ]?I[- ]?T[- ]?O[- ]?N[- ]?E$/m
# which matches a blank or dash between any characters in the official table.
#
# This is also how script lookup is done.  Basically the re looks like
#   qr/ (?:LATIN|GREEK|CYRILLIC) (?:SMALL )?LETTER $name/
# where $name is the loose or strict regex for the remainder of the name.

# The hashes are stored as utf8 strings.  This makes it easier to deal with
# sequences.  I (khw) also tried making Name.pl utf8, but it slowed things
# down by a factor of 7.  I then tried making Name.pl store the utf8
# equivalents but not calling them utf8.  That led to similar speed as leaving
# it alone, but since that is harder for a human to parse, I left it as-is.

my %system_aliases = (

    'SINGLE-SHIFT 2'                => chr utf8::unicode_to_native(0x8E),
    'SINGLE-SHIFT 3'                => chr utf8::unicode_to_native(0x8F),
    'PRIVATE USE 1'                 => chr utf8::unicode_to_native(0x91),
    'PRIVATE USE 2'                 => chr utf8::unicode_to_native(0x92),
);

# These are the aliases above that differ under :loose and :full matching
# because the :full versions have blanks or hyphens in them.
#my %loose_system_aliases = (
#);

#my %deprecated_aliases;
#$deprecated_aliases{'BELL'} = chr utf8::unicode_to_native(0x07) if $^V lt v5.17.0;

#my %loose_deprecated_aliases = (
#);

# These are special cased in :loose matching, differing only in a medial
# hyphen
my $HANGUL_JUNGSEONG_O_E_utf8 = chr 0x1180;
my $HANGUL_JUNGSEONG_OE_utf8 = chr 0x116C;


my $txt;  # The table of official character names

my %full_names_cache; # Holds already-looked-up names, so don't have to
# re-look them up again.  The previous versions of charnames had scoping
# bugs.  For example if we use script A in one scope and find and cache
# what Z resolves to, we can't use that cache in a different scope that
# uses script B instead of A, as Z might be an entirely different letter
# there; or there might be different aliases in effect in different
# scopes, or :short may be in effect or not effect in different scopes,
# or various combinations thereof.  This was solved in this version
# mostly by moving things to %^H.  But some things couldn't be moved
# there.  One of them was the cache of runtime looked-up names, in part
# because %^H is read-only at runtime.  I (khw) don't know why the cache
# was run-time only in the previous versions: perhaps oversight; perhaps
# that compile time looking doesn't happen in a loop so didn't think it
# was worthwhile; perhaps not wanting to make the cache too large.  But
# I decided to make it compile time as well; this could easily be
# changed.
# Anyway, this hash is not scoped, and is added to at runtime.  It
# doesn't have scoping problems because the data in it is restricted to
# official names, which are always invariant, and we only set it and
# look at it at during :full lookups, so is unaffected by any other
# scoped options.  I put this in to maintain parity with the older
# version.  If desired, a %short_names cache could also be made, as well
# as one for each script, say in %script_names_cache, with each key
# being a hash for a script named in a 'use charnames' statement.  I
# decided not to do that for now, just because it's added complication,
# and because I'm just trying to maintain parity, not extend it.

# Like %full_names_cache, but for use when :loose is in effect.  There needs
# to be two caches because :loose may not be in effect for a scope, and a
# loose name could inappropriately be returned when only exact matching is
# called for.
my %loose_names_cache;

# Designed so that test decimal first, and then hex.  Leading zeros
# imply non-decimal, as do non-[0-9]
my $decimal_qr = qr/^[1-9]\d*$/;

# Returns the hex number in $1.
my $hex_qr = qr/^(?:[Uu]\+|0[xX])?([[:xdigit:]]+)$/;

sub croak
{
  require Carp; goto &Carp::croak;
} # croak

sub carp
{
  require Carp; goto &Carp::carp;
} # carp

sub populate_txt()
{
  return if $txt;

  $txt = do "unicore/Name.pl";
  Internals::SvREADONLY($txt, 1);
}

sub alias (@) # Set up a single alias
{
  my @errors;
  my $nbsp = chr utf8::unicode_to_native(0xA0);

  my $alias = ref $_[0] ? $_[0] : { @_ };
  foreach my $name (sort keys %$alias) {  # Sort only because it helps having
                                          # deterministic output for
                                          # t/lib/charnames/alias
    my $value = $alias->{$name};
    next unless defined $value;          # Omit if screwed up.

    # Is slightly slower to just after this statement see if it is
    # decimal, since we already know it is after having converted from
    # hex, but makes the code easier to maintain, and is called
    # infrequently, only at compile-time
    if ($value !~ $decimal_qr && $value =~ $hex_qr) {
      my $temp = CORE::hex $1;
      $temp = utf8::unicode_to_native($temp) if $value =~ /^[Uu]\+/;
      $value = $temp;
    }
    if ($value =~ $decimal_qr) {
        no warnings qw(non_unicode surrogate nonchar); # Allow any of these
        $^H{charnames_ord_aliases}{$name} = chr $value;

        # Use a canonical form.
        $^H{charnames_inverse_ords}{sprintf("%05X", $value)} = $name;
    }
    else {
        my $ok_portion = "";
        $ok_portion = $1 if $name =~ / ^ (
                                            \p{_Perl_Charname_Begin}
                                            \p{_Perl_Charname_Continue}*
                                         ) /x;

        # If the name was fully correct, the above should have matched all of
        # it.
        if (length $ok_portion < length $name) {
          my $first_bad = substr($name, length($ok_portion), 1);
          push @errors, "Invalid character in charnames alias definition; "
                        . "marked by <-- HERE in '$ok_portion$first_bad<-- HERE "
                        . substr($name, length($ok_portion) + 1)
                        . "'";
        }
        else {
            if ($name =~ / ( .* \s ) ( \s* ) $ /x) {
              push @errors, "charnames alias definitions may not contain "
                            . "trailing white-space; marked by <-- HERE in "
                            . "'$1 <-- HERE " . $2 . "'";
              next;
            }

            # Use '+' instead of '*' in this regex, because any trailing
            # blanks have already been found
            if ($name =~ / ( .*? \s{2} ) ( .+ ) /x) {
              push @errors, "charnames alias definitions may not contain a "
                            . "sequence of multiple spaces; marked by <-- HERE "
                            . "in '$1 <-- HERE " . $2 . "'";
              next;
            }

            $^H{charnames_name_aliases}{$name} = $value;
        }
    }
  }

  # We find and output all errors from this :alias definition, rather than
  # failing on the first one, so fewer runs are needed to get it to compile
  if (@errors) {
    croak join "\n", @errors;
  }

  return;
} # alias

sub not_legal_use_bytes_msg {
  my ($name, $utf8) = @_;
  my $return;

  if (length($utf8) == 1) {
    $return = sprintf("Character 0x%04x with name '%s' is", ord $utf8, $name);
  } else {
    $return = sprintf("String with name '%s' (and ordinals %s) contains character(s)", $name, join(" ", map { sprintf "0x%04X", ord $_ } split(//, $utf8)));
  }
  return $return . " above 0xFF with 'use bytes' in effect";
}

sub alias_file ($)  # Reads a file containing alias definitions
{
  require File::Spec;
  my ($arg, $file) = @_;
  if (-f $arg && File::Spec->file_name_is_absolute ($arg)) {
    $file = $arg;
  }
  elsif ($arg =~ m/ ^ \p{_Perl_IDStart} \p{_Perl_IDCont}* $/x) {
    $file = "unicore/${arg}_alias.pl";
  }
  else {
    croak "Charnames alias file names can only have identifier characters";
  }
  if (my @alias = do $file) {
    @alias == 1 && !defined $alias[0] and
      croak "$file cannot be used as alias file for charnames";
    @alias % 2 and
      croak "$file did not return a (valid) list of alias pairs";
    alias (@alias);
    return (1);
  }
  0;
} # alias_file

# For use when don't import anything.  This structure must be kept in
# sync with the one that import() fills up.
my %dummy_H = (
                charnames_stringified_names => "",
                charnames_stringified_ords => "",
                charnames_scripts => "",
                charnames_full => 1,
                charnames_loose => 0,
                charnames_short => 0,
              );


sub lookup_name ($$$;$) {
  my ($name, $wants_ord, $runtime, $regex_loose) = @_;
  $regex_loose //= 0;

  # Lookup the name or sequence $name in the tables.  If $wants_ord is false,
  # returns the string equivalent of $name; if true, returns the ordinal value
  # instead, but in this case $name must not be a sequence; otherwise undef is
  # returned and a warning raised.  $runtime is 0 if compiletime, otherwise
  # gives the number of stack frames to go back to get the application caller
  # info.
  # If $name is not found, returns undef in runtime with no warning; and in
  # compiletime, the Unicode replacement character, with a warning.

  # It looks first in the aliases, then in the large table of official Unicode
  # names.

  my $result;       # The string result
  my $save_input;

  if ($runtime && ! $regex_loose) {

    my $hints_ref = (caller($runtime))[10];

    # If we didn't import anything (which happens with 'use charnames ()',
    # substitute a dummy structure.
    $hints_ref = \%dummy_H if ! defined $hints_ref
                              || (! defined $hints_ref->{charnames_full}
                                  && ! defined $hints_ref->{charnames_loose});

    # At runtime, but currently not at compile time, %^H gets
    # stringified, so un-stringify back to the original data structures.
    # These get thrown away by perl before the next invocation
    # Also fill in the hash with the non-stringified data.
    # N.B.  New fields must be also added to %dummy_H

    %{$^H{charnames_name_aliases}} = split ',',
                                      $hints_ref->{charnames_stringified_names};
    %{$^H{charnames_ord_aliases}} = split ',',
                                      $hints_ref->{charnames_stringified_ords};
    $^H{charnames_scripts} = $hints_ref->{charnames_scripts};
    $^H{charnames_full} = $hints_ref->{charnames_full};
    $^H{charnames_loose} = $hints_ref->{charnames_loose};
    $^H{charnames_short} = $hints_ref->{charnames_short};
  }

  my $loose = $regex_loose || $^H{charnames_loose};
  my $lookup_name;  # Input name suitably modified for grepping for in the
                    # table

  # User alias should be checked first or else can't override ours, and if we
  # were to add any, could conflict with theirs.
  if (! $regex_loose && exists $^H{charnames_ord_aliases}{$name}) {
    $result = $^H{charnames_ord_aliases}{$name};
  }
  elsif (! $regex_loose && exists $^H{charnames_name_aliases}{$name}) {
    $name = $^H{charnames_name_aliases}{$name};
    $save_input = $lookup_name = $name;  # Cache the result for any error
                                         # message
    # The aliases are documented to not match loosely, so change loose match
    # into full.
    if ($loose) {
      $loose = 0;
      $^H{charnames_full} = 1;
    }
  }
  else {

    # Here, not a user alias.  That means that loose matching may be in
    # effect; will have to modify the input name.
    $lookup_name = $name;
    if ($loose) {
      $lookup_name = uc $lookup_name;

      # Squeeze out all underscores
      $lookup_name =~ s/_//g;

      # Remove all medial hyphens
      $lookup_name =~ s/ (?<= \S  ) - (?= \S  )//gx;

      # Squeeze out all spaces
      $lookup_name =~ s/\s//g;
    }

    # Here, $lookup_name has been modified as necessary for looking in the
    # hashes.  Check the system alias files next.  Most of these aliases are
    # the same for both strict and loose matching.  To save space, the ones
    # which differ are in their own separate hash, which is checked if loose
    # matching is selected and the regular match fails.  To save time, the
    # loose hashes could be expanded to include all aliases, and there would
    # only have to be one check.  But if someone specifies :loose, they are
    # interested in convenience over speed, and the time for this second check
    # is miniscule compared to the rest of the routine.
    if (exists $system_aliases{$lookup_name}) {
      $result = $system_aliases{$lookup_name};
    }
    # There are currently no entries in this hash, so don't waste time looking
    # for them.  But the code is retained for the unlikely possibility that
    # some will be added in the future.
#    elsif ($loose && exists $loose_system_aliases{$lookup_name}) {
#      $result = $loose_system_aliases{$lookup_name};
#    }
#    if (exists $deprecated_aliases{$lookup_name}) {
#      require warnings;
#      warnings::warnif('deprecated',
#                       "Unicode character name \"$name\" is deprecated, use \""
#                       . viacode(ord $deprecated_aliases{$lookup_name})
#                       . "\" instead");
#      $result = $deprecated_aliases{$lookup_name};
#    }
    # There are currently no entries in this hash, so don't waste time looking
    # for them.  But the code is retained for the unlikely possibility that
    # some will be added in the future.
#    elsif ($loose && exists $loose_deprecated_aliases{$lookup_name}) {
#      require warnings;
#      warnings::warnif('deprecated',
#                       "Unicode character name \"$name\" is deprecated, use \""
#                       . viacode(ord $loose_deprecated_aliases{$lookup_name})
#                       . "\" instead");
#      $result = $loose_deprecated_aliases{$lookup_name};
#    }
  }

  my @off;  # Offsets into table of pattern match begin and end

  # If haven't found it yet...
  if (! defined $result) {

    # See if has looked this input up earlier.
    if (! $loose && $^H{charnames_full} && exists $full_names_cache{$name}) {
      $result = $full_names_cache{$name};
    }
    elsif ($loose && exists $loose_names_cache{$name}) {
      $result = $loose_names_cache{$name};
    }
    else { # Here, must do a look-up

      # If full or loose matching succeeded, points to where to cache the
      # result
      my $cache_ref;

      ## Suck in the code/name list as a big string.
      ## Entries look like:
      ##     "00052\nLATIN CAPITAL LETTER R\n\n"
      # or
      #      "0052 0303\nLATIN CAPITAL LETTER R WITH TILDE\n\n"
      populate_txt() unless $txt;

      ## @off will hold the index into the code/name string of the start and
      ## end of the name as we find it.

      ## If :loose, look for a loose match; if :full, look for the name
      ## exactly
      # First, see if the name is one which is algorithmically determinable.
      # The subroutine is included in Name.pl.  The table contained in
      # $txt doesn't contain these.  Experiments show that checking
      # for these before checking for the regular names has no
      # noticeable impact on performance for the regular names, but
      # the other way around slows down finding these immensely.
      # Algorithmically determinables are not placed in the cache because
      # that uses up memory, and finding these again is fast.
      if (   ($loose || $^H{charnames_full})
          && (defined (my $ord = charnames::name_to_code_point_special($lookup_name, $loose))))
      {
        $result = chr $ord;
      }
      else {

        # Not algorithmically determinable; look up in the table.  The name
        # will be turned into a regex, so quote any meta characters.
        $lookup_name = quotemeta $lookup_name;

        if ($loose) {

          # For loose matches, $lookup_name has already squeezed out the
          # non-essential characters.  We have to add in code to make the
          # squeezed version match the non-squeezed equivalent in the table.
          # The only remaining hyphens are ones that start or end a word in
          # the original.  They have been quoted in $lookup_name so they look
          # like "\-".  Change all other characters except the backslash
          # quotes for any metacharacters, and the final character, so that
          # e.g., COLON gets transformed into: /C[- ]?O[- ]?L[- ]?O[- ]?N/
          $lookup_name =~ s/ (?! \\ -)    # Don't do this to the \- sequence
                             ( [^-\\] )   # Nor the "-" within that sequence,
                                          # nor the "\" that quotes metachars,
                                          # but otherwise put the char into $1
                             (?=.)        # And don't do it for the final char
                           /$1\[- \]?/gx; # And add an optional blank or
                                          # '-' after each $1 char

          # Those remaining hyphens were originally at the beginning or end of
          # a word, so they can match either a blank before or after, but not
          # both.  (Keep in mind that they have been quoted, so are a '\-'
          # sequence)
          $lookup_name =~ s/\\ -/(?:- | -)/xg;
        }

        # Do the lookup in the full table if asked for, and if succeeds
        # save the offsets and set where to cache the result.
        if (($loose || $^H{charnames_full}) && $txt =~ /^$lookup_name$/m) {
          @off = ($-[0], $+[0]);
          $cache_ref = ($loose) ? \%loose_names_cache : \%full_names_cache;
        }
        elsif ($regex_loose) {
          # Currently don't allow :short when this is set
          return;
        }
        else {

          # Here, didn't look for, or didn't find the name.
          # If :short is allowed, see if input is like "greek:Sigma".
          # Keep in mind that $lookup_name has had the metas quoted.
          my $scripts_trie = "";
          my $name_has_uppercase;
          my @scripts;
          if (($^H{charnames_short})
              && $lookup_name =~ /^ (?: \\ \s)*   # Quoted space
                                    (.+?)         # $1 = the script
                                    (?: \\ \s)*
                                    \\ :          # Quoted colon
                                    (?: \\ \s)*
                                    (.+?)         # $2 = the name
                                    (?: \\ \s)* $
                                  /xs)
          {
              # Even in non-loose matching, the script traditionally has been
              # case insensitive
              $scripts_trie = "\U$1";
              $lookup_name = $2;

              # Use original name to find its input casing, but ignore the
              # script part of that to make the determination.
              $save_input = $name if ! defined $save_input;
              $name =~ s/.*?://;
              $name_has_uppercase = $name =~ /[[:upper:]]/;
          }
          else { # Otherwise look in allowed scripts
              # We want to search first by script name then by letter name, so that
              # if the user imported `use charnames qw(arabic hebrew)` and asked for
              # \N{alef} they get ARABIC LETTER ALEF, and if they imported
              # `... (hebrew arabic)` and ask for \N{alef} they get HEBREW LETTER ALEF.
              # We can't rely on the regex engine to preserve ordering like that, so
              # pick the pipe-seperated string apart so we can iterate over it.
              @scripts = split(/\|/, $^H{charnames_scripts});

              # Use original name to find its input casing
              $name_has_uppercase = $name =~ /[[:upper:]]/;
          }
          my $case = $name_has_uppercase ? "CAPITAL" : "SMALL";

          if(@scripts) {
              SCRIPTS: foreach my $script (@scripts) {
                  if($txt =~ /^ (?: $script ) \ (?:$case\ )? LETTER \ \U$lookup_name $/xm) {
                      @off = ($-[0], $+[0]);
                      last SCRIPTS;
                  }
              }
              return unless(@off);
          }
          else {
            return if (! $scripts_trie || $txt !~
               /^ (?: $scripts_trie ) \ (?:$case\ )? LETTER \ \U$lookup_name $/xm);
            @off = ($-[0], $+[0]);
          }
        }

        # Here, the input name has been found; we haven't set up the output,
        # but we know where in the string
        # the name starts.  The string is set up so that for single characters
        # (and not named sequences), the name is on a line by itself, and the
        # previous line contains precisely 5 hex digits for its code point.
        # Named sequences won't have the 7th preceding character be a \n.
        # (Actually, for the very first entry in the table this isn't strictly
        # true: subtracting 7 will yield -1, and the substr below will
        # therefore yield the very last character in the table, which should
        # also be a \n, so the statement works anyway.)
        if (substr($txt, $off[0] - 7, 1) eq "\n") {
          $result = chr CORE::hex substr($txt, $off[0] - 6, 5);

          # Handle the single loose matching special case, in which two names
          # differ only by a single medial hyphen.  If the original had a
          # hyphen (or more) in the right place, then it is that one.
          $result = $HANGUL_JUNGSEONG_O_E_utf8
                  if $loose
                     && $result eq $HANGUL_JUNGSEONG_OE_utf8
                     && $name =~ m/O \s* - [-\s]* E/ix;
                     # Note that this wouldn't work if there were a 2nd
                     # OE in the name
        }
        else {

          # Here, is a named sequence.  Need to go looking for the beginning,
          # which is just after the \n from the previous entry in the table.
          # The +1 skips past that newline, or, if the rindex() fails, to put
          # us to an offset of zero.
          my $charstart = rindex($txt, "\n", $off[0] - 7) + 1;
          $result = pack("W*", map { CORE::hex }
              split " ", substr($txt, $charstart, $off[0] - $charstart - 1));
        }
      }

      # Cache the input so as to not have to search the large table
      # again, but only if it came from the one search that we cache.
      # (Haven't bothered with the pain of sorting out scoping issues for the
      # scripts searches.)
      $cache_ref->{$name} = $result if defined $cache_ref;
    }
  }

  # Here, have the result character.  If the return is to be an ord, must be
  # any single character.
  if ($wants_ord) {
    return ord($result) if length $result == 1;
  }
  elsif (! utf8::is_utf8($result)) {

    # Here isn't UTF-8.  That's OK if it is all ASCII, or we are being called
    # at compile time where we know we can guarantee that Unicode rules are
    # correctly imposed on the result, or under 'bytes' where we don't want
    # those rules.  But otherwise we have to make it UTF8 to guarantee Unicode
    # rules on the returned string.
    return $result if ! $runtime
                      || (caller $runtime)[8] & $bytes::hint_bits
                      || $result !~ /[[:^ascii:]]/;
    utf8::upgrade($result);
    return $result;
  }
  else {

    # Here, wants string output.  If utf8 is acceptable, just return what
    # we've got; otherwise attempt to convert it to non-utf8 and return that.
    my $in_bytes =     ! $regex_loose   # \p{name=} doesn't currently care if
                                        # in bytes or not
                   && (($runtime)
                       ? (caller $runtime)[8] & $bytes::hint_bits
                       : $^H & $bytes::hint_bits);
    return $result if (! $in_bytes || utf8::downgrade($result, 1)) # The 1 arg
                                                  # means don't die on failure
  }

  # Here, there is an error:  either there are too many characters, or the
  # result string needs to be non-utf8, and at least one character requires
  # utf8.  Prefer any official name over the input one for the error message.
  if (@off) {
    $name = substr($txt, $off[0], $off[1] - $off[0]) if @off;
  }
  else {
    $name = (defined $save_input) ? $save_input : $_[0];
  }

  if ($wants_ord) {
    # Only way to get here in this case is if result too long.  Message
    # assumes that our only caller that requires single char result is
    # vianame.
    carp "charnames::vianame() doesn't handle named sequences ($name).  Use charnames::string_vianame() instead";
    return;
  }

  # Only other possible failure here is from use bytes.
  if ($runtime) {
    carp not_legal_use_bytes_msg($name, $result);
    return;
  } else {
    croak not_legal_use_bytes_msg($name, $result);
  }

} # lookup_name

sub charnames {

  # For \N{...}.  Looks up the character name and returns the string
  # representation of it.

  # The first 0 arg means wants a string returned; the second that we are in
  # compile time
  return lookup_name($_[0], 0, 0);
}

sub _loose_regcomp_lookup {
  # For use only by regcomp.c to compile \p{name=...}
  # khw thinks it best to not do :short matching, and only official names.
  # But that is only a guess, and if demand warrants, could be changed
  return lookup_name($_[0], 0, 1,
                     1  # Always use :loose matching
                    );
}

sub _get_names_info {
  # For use only by regcomp.c to compile \p{name=/.../}
  populate_txt() unless $txt;


  return ( \$txt, \@charnames::code_points_ending_in_code_point );
}

sub import
{
  shift; ## ignore class name

  populate_txt() unless $txt;

  if (not @_) {
    carp("'use charnames' needs explicit imports list");
  }
  $^H{charnames} = \&charnames ;
  $^H{charnames_ord_aliases} = {};
  $^H{charnames_name_aliases} = {};
  $^H{charnames_inverse_ords} = {};
  # New fields must be added to %dummy_H, and the code in lookup_name()
  # that copies fields from the runtime structure

  ##
  ## fill %h keys with our @_ args.
  ##
  my ($promote, %h, @args) = (0);
  while (my $arg = shift) {
    if ($arg eq ":alias") {
      @_ or
        croak ":alias needs an argument in charnames";
      my $alias = shift;
      if (ref $alias) {
        ref $alias eq "HASH" or
          croak "Only HASH reference supported as argument to :alias";
        alias ($alias);
        $promote = 1;
        next;
      }
      if ($alias =~ m{:(\w+)$}) {
        $1 eq "full" || $1 eq "loose" || $1 eq "short" and
          croak ":alias cannot use existing pragma :$1 (reversed order?)";
        alias_file ($1) and $promote = 1;
        next;
      }
      alias_file ($alias) and $promote = 1;
      next;
    }
    if (substr($arg, 0, 1) eq ':'
      and ! ($arg eq ":full" || $arg eq ":short" || $arg eq ":loose"))
    {
      warn "unsupported special '$arg' in charnames";
      next;
    }
    push @args, $arg;
  }

  @args == 0 && $promote and @args = (":full");
  @h{@args} = (1) x @args;

  # Don't leave these undefined as are tested for in lookup_names
  $^H{charnames_full} = delete $h{':full'} || 0;
  $^H{charnames_loose} = delete $h{':loose'} || 0;
  $^H{charnames_short} = delete $h{':short'} || 0;
  my @scripts = map { uc quotemeta } grep { /^[^:]/ } @args;

  ##
  ## If utf8? warnings are enabled, and some scripts were given,
  ## see if at least we can find one letter from each script.
  ##
  if (warnings::enabled('utf8') && @scripts) {
    for my $script (@scripts) {
      if (not $txt =~ m/^$script (?:CAPITAL |SMALL )?LETTER /m) {
        warnings::warn('utf8',  "No such script: '$script'");
        $script = quotemeta $script;  # Escape it, for use in the re.
      }
    }
  }

  # %^H gets stringified, so serialize it ourselves so can extract the
  # real data back later.
  $^H{charnames_stringified_ords} = join ",", %{$^H{charnames_ord_aliases}};
  $^H{charnames_stringified_names} = join ",", %{$^H{charnames_name_aliases}};
  $^H{charnames_stringified_inverse_ords} = join ",", %{$^H{charnames_inverse_ords}};

  # Modify the input script names for loose name matching if that is also
  # specified, similar to the way the base character name is prepared.  They
  # don't (currently, and hopefully never will) have dashes.  These go into a
  # regex, and have already been uppercased and quotemeta'd.  Squeeze out all
  # input underscores, blanks, and dashes.  Then convert so will match a blank
  # between any characters.
  if ($^H{charnames_loose}) {
    for (my $i = 0; $i < @scripts; $i++) {
      $scripts[$i] =~ s/[_ -]//g;
      $scripts[$i] =~ s/ ( [^\\] ) (?= . ) /$1\\ ?/gx;
    }
  }

  my %letters_by_script = map {
      $_ => [
          ($txt =~ m/$_(?: (?:small|capital))? letter (.*)/ig)
      ]
  } @scripts;
  SCRIPTS: foreach my $this_script (@scripts) {
    my @other_scripts = grep { $_ ne $this_script } @scripts;
    my @this_script_letters  = @{$letters_by_script{$this_script}};
    my @other_script_letters = map { @{$letters_by_script{$_}} } @other_scripts;
    foreach my $this_letter (@this_script_letters) {
      if(grep { $_ eq $this_letter } @other_script_letters) {
        warn "charnames: some short character names may clash in [".join(', ', sort @scripts)."], for example $this_letter\n";
        last SCRIPTS;
      }
    }
  }

  $^H{charnames_scripts} = join "|", @scripts;  # Stringifiy them as a trie
} # import

# Cache of already looked-up values.  This is set to only contain
# official values, and user aliases can't override them, so scoping is
# not an issue.
my %viacode;

my $no_name_code_points_re = join "|", map { sprintf("%05X",
                                             utf8::unicode_to_native($_)) }
                                            0x80, 0x81, 0x84, 0x99;
$no_name_code_points_re = qr/$no_name_code_points_re/;

sub viacode {

  # Returns the name of the code point argument

  if (@_ != 1) {
    carp "charnames::viacode() expects one argument";
    return;
  }

  my $arg = shift;

  # This is derived from Unicode::UCD, where it is nearly the same as the
  # function _getcode(), but here it makes sure that even a hex argument
  # has the proper number of leading zeros, which is critical in
  # matching against $txt below
  # Must check if decimal first; see comments at that definition
  my $hex;
  if ($arg =~ $decimal_qr) {
    $hex = sprintf "%05X", $arg;
  } elsif ($arg =~ $hex_qr) {
    $hex = CORE::hex $1;
    $hex = utf8::unicode_to_native($hex) if $arg =~ /^[Uu]\+/;
    # Below is the line that differs from the _getcode() source
    $hex = sprintf "%05X", $hex;
  } else {
    carp("unexpected arg \"$arg\" to charnames::viacode()");
    return;
  }

  return $viacode{$hex} if exists $viacode{$hex};

  my $return;

  # If the code point is above the max in the table, there's no point
  # looking through it.  Checking the length first is slightly faster
  if (length($hex) <= 5 || CORE::hex($hex) <= 0x10FFFF) {
    populate_txt() unless $txt;

    # See if the name is algorithmically determinable.
    my $algorithmic = charnames::code_point_to_name_special(CORE::hex $hex);
    if (defined $algorithmic) {
      $viacode{$hex} = $algorithmic;
      return $algorithmic;
    }

    # Return the official name, if exists.  It's unclear to me (khw) at
    # this juncture if it is better to return a user-defined override, so
    # leaving it as is for now.
    if ($txt =~ m/^$hex\n/m) {

        # The name starts with the next character and goes up to the
        # next new-line.  Using capturing parentheses above instead of
        # @+ more than doubles the execution time in Perl 5.13
        $return = substr($txt, $+[0], index($txt, "\n", $+[0]) - $+[0]);

        # If not one of these 4 code points, return what we've found.
        if ($hex !~ / ^ $no_name_code_points_re $ /x) {
          $viacode{$hex} = $return;
          return $return;
        }

        # For backwards compatibility, we don't return the official name of
        # the 4 code points if there are user-defined aliases for them -- so
        # continue looking.
    }
  }

  # See if there is a user name for it, before giving up completely.
  # First get the scoped aliases, give up if have none.
  my $H_ref = (caller(1))[10];
  return if ! defined $return
              && (! defined $H_ref
                  || ! exists $H_ref->{charnames_stringified_inverse_ords});

  my %code_point_aliases;
  if (defined $H_ref->{charnames_stringified_inverse_ords}) {
    %code_point_aliases = split ',',
                          $H_ref->{charnames_stringified_inverse_ords};
    return $code_point_aliases{$hex} if exists $code_point_aliases{$hex};
  }

  # Here there is no user-defined alias, return any official one.
  return $return if defined $return;

  if (CORE::hex($hex) > 0x10FFFF
      && warnings::enabled('non_unicode'))
  {
      carp "Unicode characters only allocated up to U+10FFFF (you asked for U+$hex)";
  }
  return;

} # viacode

1;

# ex: set ts=8 sts=2 sw=2 et:
