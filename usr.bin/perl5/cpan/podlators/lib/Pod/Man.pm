# Convert POD data to formatted *roff input.
#
# This module translates POD documentation into *roff markup using the man
# macro set, and is intended for converting POD documents written as Unix
# manual pages to manual pages that can be read by the man(1) command.  It is
# a replacement for the pod2man command distributed with versions of Perl
# prior to 5.6.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

##############################################################################
# Modules and declarations
##############################################################################

package Pod::Man;

use 5.010;
use strict;
use warnings;

use Carp qw(carp croak);
use Pod::Simple ();

# Conditionally import Encode and set $HAS_ENCODE if it is available.  This is
# required to support building as part of Perl core, since podlators is built
# before Encode is.
my $HAS_ENCODE;
BEGIN {
    $HAS_ENCODE = eval { require Encode };
}

our @ISA = qw(Pod::Simple);
our $VERSION = '5.01';

# Ensure that $Pod::Simple::nbsp and $Pod::Simple::shy are available.  Code
# taken from Pod::Simple 3.32, but was only added in 3.30.
my ($NBSP, $SHY);
if ($Pod::Simple::VERSION ge 3.30) {
    $NBSP = $Pod::Simple::nbsp;
    $SHY  = $Pod::Simple::shy;
} else {
    $NBSP = chr utf8::unicode_to_native(0xA0);
    $SHY  = chr utf8::unicode_to_native(0xAD);
}

# Import the ASCII constant from Pod::Simple.  This is true iff we're in an
# ASCII-based universe (including such things as ISO 8859-1 and UTF-8), and is
# generally only false for EBCDIC.
BEGIN { *ASCII = \&Pod::Simple::ASCII }

# Formatting instructions for various types of blocks.  cleanup makes hyphens
# hard, adds spaces between consecutive underscores, and escapes backslashes.
# convert translates characters into escapes.  guesswork means to apply the
# transformations done by the guesswork sub (if enabled).  literal says to
# protect literal quotes from being turned into UTF-8 quotes.  By default, all
# transformations are on except literal, but some elements override.
#
# DEFAULT specifies the default settings.  All other elements should list only
# those settings that they are overriding.  Data indicates =for roff blocks,
# which should be passed along completely verbatim.
#
# Formatting inherits negatively, in the sense that if the parent has turned
# off guesswork, all child elements should leave it off.
my %FORMATTING = (
    DEFAULT  => { cleanup => 1, convert => 1, guesswork => 1, literal => 0 },
    Data     => { cleanup => 0, convert => 0, guesswork => 0, literal => 0 },
    Verbatim => {                             guesswork => 0, literal => 1 },
    C        => {                             guesswork => 0, literal => 1 },
    X        => { cleanup => 0,               guesswork => 0               },
);

# Try to map an encoding as understood by Perl Encode to an encoding
# understood by groff's preconv.  Encode doesn't care about hyphens or
# capitalization, but preconv does.  The key is the canonicalized Encode
# encoding, and the value is something preconv might understand.
#
# FreeBSD mandoc only understands utf-8 and iso-latin-1 as of 2022-09-24.
# groff preconv prefers iso-8859-1, but also understands iso-latin-1, so
# convert ISO-8859-1 to iso-latin-1 for FreeBSD.
my %ENCODINGS = (
    ascii     => 'us-ascii',
    big5      => 'big5',
    big5eten  => 'big5',
    cp950     => 'big5',
    cp1047    => 'cp1047',
    euccn     => 'gb2312',
    eucjp     => 'euc-jp',
    euckr     => 'euc-kr',
    gb2312    => 'gb2312',
    gb2312raw => 'gb2312',
    iso88591  => 'iso-latin-1',
    iso88592  => 'iso-8859-2',
    iso88595  => 'iso-8859-5',
    iso88597  => 'iso-8859-7',
    iso88599  => 'iso-8859-9',
    iso885913 => 'iso-8859-13',
    iso885915 => 'iso-8859-15',
    koi8r     => 'koi8-r',
    latin1    => 'iso-8859-1',
    usascii   => 'us-ascii',
    utf8      => 'utf-8',
    utf16     => 'utf-16',
    utf16be   => 'utf-16be',
    utf16le   => 'utf-16le',
);

##############################################################################
# Translation tables
##############################################################################

# The following table is adapted from Tom Christiansen's pod2man.  It is only
# used with roff output.  It assumes that the standard preamble has already
# been printed, since that's what defines all of the accent marks.  We really
# want to do something better than this when *roff actually supports other
# character sets itself, since these results are pretty poor.
#
# This only works in an ASCII world.  What to do in a non-ASCII world is very
# unclear, so we just output what we get and hope for the best.
my %ESCAPES;
@ESCAPES{0xA0 .. 0xFF} = (
    $NBSP, undef, undef, undef,            undef, undef, undef, undef,
    undef, undef, undef, undef,            undef, $SHY,  undef, undef,

    undef, undef, undef, undef,            undef, undef, undef, undef,
    undef, undef, undef, undef,            undef, undef, undef, undef,

    "A\\*`",  "A\\*'", "A\\*^", "A\\*~",   "A\\*:", "A\\*o", "\\*(Ae", "C\\*,",
    "E\\*`",  "E\\*'", "E\\*^", "E\\*:",   "I\\*`", "I\\*'", "I\\*^",  "I\\*:",

    "\\*(D-", "N\\*~", "O\\*`", "O\\*'",   "O\\*^", "O\\*~", "O\\*:",  undef,
    "O\\*/",  "U\\*`", "U\\*'", "U\\*^",   "U\\*:", "Y\\*'", "\\*(Th", "\\*8",

    "a\\*`",  "a\\*'", "a\\*^", "a\\*~",   "a\\*:", "a\\*o", "\\*(ae", "c\\*,",
    "e\\*`",  "e\\*'", "e\\*^", "e\\*:",   "i\\*`", "i\\*'", "i\\*^",  "i\\*:",

    "\\*(d-", "n\\*~", "o\\*`", "o\\*'",   "o\\*^", "o\\*~", "o\\*:",  undef,
    "o\\*/" , "u\\*`", "u\\*'", "u\\*^",   "u\\*:", "y\\*'", "\\*(th", "y\\*:",
) if ASCII;

##############################################################################
# Utility functions
##############################################################################

# Quote an argument to a macro.
#
# $arg - Intended argument to the macro
#
# Returns: $arg suitably escaped and quoted
sub _quote_macro_argument {
    my ($arg) = @_;
    if (length($arg) > 0 && $arg !~ m{ [\s\"] }xms) {
        return $arg;
    }
    $arg =~ s{ \" }{""}xmsg;
    return qq("$arg");
}

# Returns whether the given encoding needs a call to Encode::encode.
sub _needs_encode {
    my ($encoding) = @_;
    return $encoding ne 'roff' && $encoding ne 'groff';
}

##############################################################################
# Object initialization
##############################################################################

# Initialize the object and set various Pod::Simple options that we need.
# Here, we also process any additional options passed to the constructor or
# set up defaults if none were given.  Note that all internal object keys are
# in all-caps, reserving all lower-case object keys for Pod::Simple and user
# arguments.
sub new {
    my $class = shift;
    my $self = $class->SUPER::new;

    # Tell Pod::Simple to keep whitespace whenever possible.
    if (my $preserve_whitespace = $self->can ('preserve_whitespace')) {
        $self->$preserve_whitespace (1);
    } else {
        $self->fullstop_space_harden (1);
    }

    # The =for and =begin targets that we accept.
    $self->accept_targets (qw/man MAN roff ROFF/);

    # Ensure that contiguous blocks of code are merged together.  Otherwise,
    # some of the guesswork heuristics don't work right.
    $self->merge_text (1);

    # Pod::Simple doesn't do anything useful with our arguments, but we want
    # to put them in our object as hash keys and values.  This could cause
    # problems if we ever clash with Pod::Simple's own internal class
    # variables.
    my %opts = @_;
    my @opts = map { ("opt_$_", $opts{$_}) } keys %opts;
    %$self = (%$self, @opts);

    # Pod::Simple uses encoding internally, so we need to store it as
    # ENCODING.  Set the default to UTF-8 if not specified.
    #
    # Degrade to the old roff encoding if Encode is not available.
    #
    # Suppress the warning message when PERL_CORE is set, indicating this is
    # running as part of the core Perl build.  Perl builds podlators (and all
    # pure Perl modules) before Encode and other XS modules, so Encode won't
    # yet be available.  Rely on the Perl core build to generate man pages
    # later, after all the modules are available, so that UTF-8 handling will
    # be correct.
    my %options = @_;
    if (defined $self->{opt_encoding}) {
        $$self{ENCODING} = $self->{opt_encoding};
    } elsif (ASCII) {
        $$self{ENCODING} = 'UTF-8';
    } else {
        $$self{ENCODING} = 'groff';
    }
    if (_needs_encode($$self{ENCODING}) && !$HAS_ENCODE) {
        if (!$ENV{PERL_CORE}) {
            carp ('encoding requested but Encode module not available,'
                    . ' falling back to groff escapes');
        }
        $$self{ENCODING} = 'groff';
    }

    # Send errors to stderr if requested.
    if ($self->{opt_stderr} and not $self->{opt_errors}) {
        $self->{opt_errors} = 'stderr';
    }
    delete $self->{opt_stderr};

    # Validate the errors parameter and act on it.
    $self->{opt_errors} //= 'pod';
    if ($self->{opt_errors} eq 'stderr' || $self->{opt_errors} eq 'die') {
        $self->no_errata_section (1);
        $self->complain_stderr (1);
        if ($self->{opt_errors} eq 'die') {
            $self->{complain_die} = 1;
        }
    } elsif ($self->{opt_errors} eq 'pod') {
        $self->no_errata_section (0);
        $self->complain_stderr (0);
    } elsif ($self->{opt_errors} eq 'none') {
        $self->no_errata_section (1);
        $self->no_whining (1);
    } else {
        croak (qq(Invalid errors setting: "$self->{opt_errors}"));
    }
    delete $self->{opt_errors};

    # Initialize various other internal constants based on our arguments.
    $self->init_fonts;
    $self->init_quotes;
    $self->init_page;

    # Configure guesswork based on options.
    my $guesswork = $self->{opt_guesswork} || q{};
    my %guesswork = map { $_ => 1 } split(m{,}xms, $guesswork);
    if (!%guesswork || $guesswork{all}) {
        #<<<
        $$self{GUESSWORK} = {
            functions => 1,
            manref    => 1,
            quoting   => 1,
            variables => 1,
        };
        #>>>
    } elsif ($guesswork{none}) {
        $$self{GUESSWORK} = {};
    } else {
        $$self{GUESSWORK} = {%guesswork};
    }

    return $self;
}

# Translate a font string into an escape.
sub toescape { (length ($_[0]) > 1 ? '\f(' : '\f') . $_[0] }

# Determine which fonts the user wishes to use and store them in the object.
# Regular, italic, bold, and bold-italic are constants, but the fixed width
# fonts may be set by the user.  Sets the internal hash key FONTS which is
# used to map our internal font escapes to actual *roff sequences later.
sub init_fonts {
    my ($self) = @_;

    # Figure out the fixed-width font.  If user-supplied, make sure that they
    # are the right length.
    for (qw(fixed fixedbold fixeditalic fixedbolditalic)) {
        my $font = $self->{"opt_$_"};
        if (defined($font) && (length($font) < 1 || length($font) > 2)) {
            croak(qq(roff font should be 1 or 2 chars, not "$font"));
        }
    }

    # Set the default fonts.  We can't be sure portably across different
    # implementations what fixed bold-italic may be called (if it's even
    # available), so default to just bold.
    #<<<
    $self->{opt_fixed}           ||= 'CW';
    $self->{opt_fixedbold}       ||= 'CB';
    $self->{opt_fixeditalic}     ||= 'CI';
    $self->{opt_fixedbolditalic} ||= 'CB';
    #>>>

    # Set up a table of font escapes.  First number is fixed-width, second is
    # bold, third is italic.
    $self->{FONTS} = {
        '000' => '\fR',
        '001' => '\fI',
        '010' => '\fB',
        '011' => '\f(BI',
        '100' => toescape($self->{opt_fixed}),
        '101' => toescape($self->{opt_fixeditalic}),
        '110' => toescape($self->{opt_fixedbold}),
        '111' => toescape($self->{opt_fixedbolditalic}),
    };

    # Precalculate a regex that matches all fixed-width fonts, which will be
    # used later by switchquotes.
    my @fixedpat = map { quotemeta($self->{FONTS}{$_}) } qw(100 101 110 111);
    my $fixedpat = join('|', @fixedpat);
    $self->{FIXEDPAT} = qr{ $fixedpat }xms;
}

# Initialize the quotes that we'll be using for C<> text.  This requires some
# special handling, both to parse the user parameters if given and to make
# sure that the quotes will be safe against *roff.  Sets the internal hash
# keys LQUOTE and RQUOTE.
sub init_quotes {
    my ($self) = (@_);

    # Handle the quotes option first, which sets both quotes at once.
    $self->{opt_quotes} ||= '"';
    if ($self->{opt_quotes} eq 'none') {
        $$self{LQUOTE} = $$self{RQUOTE} = '';
    } elsif (length ($self->{opt_quotes}) == 1) {
        $$self{LQUOTE} = $$self{RQUOTE} = $self->{opt_quotes};
    } elsif (length ($self->{opt_quotes}) % 2 == 0) {
        my $length = length ($self->{opt_quotes}) / 2;
        $$self{LQUOTE} = substr ($self->{opt_quotes}, 0, $length);
        $$self{RQUOTE} = substr ($self->{opt_quotes}, $length);
    } else {
        croak(qq(Invalid quote specification "$self->{opt_quotes}"))
    }

    # Now handle the lquote and rquote options.
    if (defined($self->{opt_lquote})) {
        $self->{opt_lquote} = q{} if $self->{opt_lquote} eq 'none';
        $$self{LQUOTE} = $self->{opt_lquote};
    }
    if (defined $self->{opt_rquote}) {
        $self->{opt_rquote} = q{} if $self->{opt_rquote} eq 'none';
        $$self{RQUOTE} = $self->{opt_rquote};
    }
}

# Initialize the page title information and indentation from our arguments.
sub init_page {
    my ($self) = @_;

    # Get the version from the running Perl.
    my @version = ($] =~ /^(\d+)\.(\d{3})(\d+)$/);
    for (@version) { $_ += 0 }
    my $version = join ('.', @version);

    # Set the defaults for page titles and indentation if the user didn't
    # override anything.
    $self->{opt_center}  //= 'User Contributed Perl Documentation';
    $self->{opt_release} //= 'perl v' . $version;
    $self->{opt_indent}  //= 4;
}

##############################################################################
# Core parsing
##############################################################################

# This is the glue that connects the code below with Pod::Simple itself.  The
# goal is to convert the event stream coming from the POD parser into method
# calls to handlers once the complete content of a tag has been seen.  Each
# paragraph or POD command will have textual content associated with it, and
# as soon as all of a paragraph or POD command has been seen, that content
# will be passed in to the corresponding method for handling that type of
# object.  The exceptions are handlers for lists, which have opening tag
# handlers and closing tag handlers that will be called right away.
#
# The internal hash key PENDING is used to store the contents of a tag until
# all of it has been seen.  It holds a stack of open tags, each one
# represented by a tuple of the attributes hash for the tag, formatting
# options for the tag (which are inherited), and the contents of the tag.

# Add a block of text to the contents of the current node, formatting it
# according to the current formatting instructions as we do.
sub _handle_text {
    my ($self, $text) = @_;
    my $tag = $$self{PENDING}[-1];
    $$tag[2] .= $self->format_text ($$tag[1], $text);
}

# Given an element name, get the corresponding method name.
sub method_for_element {
    my ($self, $element) = @_;
    $element =~ tr/A-Z-/a-z_/;
    $element =~ tr/_a-z0-9//cd;
    return $element;
}

# Handle the start of a new element.  If cmd_element is defined, assume that
# we need to collect the entire tree for this element before passing it to the
# element method, and create a new tree into which we'll collect blocks of
# text and nested elements.  Otherwise, if start_element is defined, call it.
sub _handle_element_start {
    my ($self, $element, $attrs) = @_;
    my $method = $self->method_for_element ($element);

    # If we have a command handler, we need to accumulate the contents of the
    # tag before calling it.  Turn off IN_NAME for any command other than
    # <Para> and the formatting codes so that IN_NAME isn't still set for the
    # first heading after the NAME heading.
    if ($self->can ("cmd_$method")) {
        $$self{IN_NAME} = 0 if ($element ne 'Para' && length ($element) > 1);

        # How we're going to format embedded text blocks depends on the tag
        # and also depends on our parent tags.  Thankfully, inside tags that
        # turn off guesswork and reformatting, nothing else can turn it back
        # on, so this can be strictly inherited.
        my $formatting = {
            %{ $$self{PENDING}[-1][1] || $FORMATTING{DEFAULT} },
            %{ $FORMATTING{$element} || {} },
        };
        push (@{ $$self{PENDING} }, [ $attrs, $formatting, '' ]);
    } elsif (my $start_method = $self->can ("start_$method")) {
        $self->$start_method ($attrs, '');
    }
}

# Handle the end of an element.  If we had a cmd_ method for this element,
# this is where we pass along the tree that we built.  Otherwise, if we have
# an end_ method for the element, call that.
sub _handle_element_end {
    my ($self, $element) = @_;
    my $method = $self->method_for_element ($element);

    # If we have a command handler, pull off the pending text and pass it to
    # the handler along with the saved attribute hash.
    if (my $cmd_method = $self->can ("cmd_$method")) {
        my $tag = pop @{ $$self{PENDING} };
        my $text = $self->$cmd_method ($$tag[0], $$tag[2]);
        if (defined $text) {
            if (@{ $$self{PENDING} } > 1) {
                $$self{PENDING}[-1][2] .= $text;
            } else {
                $self->output ($text);
            }
        }
    } elsif (my $end_method = $self->can ("end_$method")) {
        $self->$end_method ();
    }
}

##############################################################################
# General formatting
##############################################################################

# Format a text block.  Takes a hash of formatting options and the text to
# format.  Currently, the only formatting options are guesswork, cleanup, and
# convert, all of which are boolean.
sub format_text {
    my ($self, $options, $text) = @_;
    my $guesswork = $$options{guesswork} && !$$self{IN_NAME};
    my $cleanup = $$options{cleanup};
    my $convert = $$options{convert};
    my $literal = $$options{literal};

    # Cleanup just tidies up a few things, telling *roff that the hyphens are
    # hard, putting a bit of space between consecutive underscores, escaping
    # backslashes, and converting zero-width spaces to zero-width break
    # points.
    if ($cleanup) {
        $text =~ s/\\/\\e/g;
        $text =~ s/-/\\-/g;
        $text =~ s/_(?=_)/_\\|/g;
        $text =~ s/\x{200B}/\\:/g;
    }

    # Except in <Data> blocks, if groff or roff encoding is requested and
    # we're in an ASCII environment, do the encoding.  For EBCDIC, we just
    # write what we get and hope for the best.  Leave non-breaking spaces and
    # soft hyphens alone; we'll convert those at the last minute.
    if ($convert) {
        if (ASCII) {
            if ($$self{ENCODING} eq 'groff') {
                $text =~ s{ ([^\x00-\x7F\xA0\xAD]) }{
                    '\\[u' . sprintf('%04X', ord($1)) . ']'
                }xmsge;
            } elsif ($$self{ENCODING} eq 'roff') {
                $text =~ s/([^\x00-\x7F\xA0\xAD])/$ESCAPES{ord ($1)} || "X"/eg;
            }
        }
    }

    # Ensure that *roff doesn't convert literal quotes to UTF-8 single quotes,
    # but don't mess up accent escapes.
    if ($literal) {
        $text =~ s/(?<!\\\*)\'/\\*\(Aq/g;
        $text =~ s/(?<!\\\*)\`/\\\`/g;
    }

    # If guesswork is is viable for this block, do that.
    if ($guesswork) {
        $text = $self->guesswork ($text);
    }

    return $text;
}

# Handles C<> text, deciding whether to put \*C` around it or not.  This is a
# whole bunch of messy heuristics to try to avoid overquoting, originally from
# Barrie Slaymaker.  This largely duplicates similar code in Pod::Text.
sub quote_literal {
    my $self = shift;
    local $_ = shift;

    # If in NAME section, just return an ASCII quoted string to avoid
    # confusing tools like whatis.
    if ($$self{IN_NAME}) {
        return $self->{LQUOTE} . $_ . $self->{RQUOTE};
    }

    # A regex that matches the portion of a variable reference that's the
    # array or hash index, separated out just because we want to use it in
    # several places in the following regex.
    my $index = '(?: \[[^]]+\] | \{[^}]+\} )?';

    # Check for things that we don't want to quote, and if we find any of
    # them, return the string with just a font change and no quoting.
    #
    # Traditionally, Pod::Man has not quoted Perl variables, functions,
    # numbers, or hex constants, but this is not always desirable.  Make this
    # optional on the quoting guesswork flag.
    my $extra = qr{(?!)}xms;    # never matches
    if ($$self{GUESSWORK}{quoting}) {
        $extra = qr{
             \$+ [\#^]? \S $index                    # special ($^F, $")
           | [\$\@%&*]+ \#? [:\'\w]+ $index          # plain var or func
           | [\$\@%&*]* [:\'\w]+
             (?: \\-> )? \(\s*[^\s,\)]*\s*\)         # 0/1-arg func call
           | (?: [+] || \\- )? ( \d[\d.]* | \.\d+ )
             (?: [eE] (?: [+] || \\- )? \d+ )?       # a number
           | 0x [a-fA-F\d]+                          # a hex constant
         }xms;
    }
    m{
      ^\s*
      (?:
         ( [\'\"] ) .* \1                    # already quoted
       | \\\*\(Aq .* \\\*\(Aq                # quoted and escaped
       | \\?\` .* ( \' | \\?\` | \\\*\(Aq )  # `quoted' or `quoted`
       | $extra
      )
      \s*\z
     }xms and return '\f(FS' . $_ . '\f(FE';

    # If we didn't return, go ahead and quote the text.
    return '\f(FS\*(C`' . $_ . "\\*(C'\\f(FE";
}

# Takes a text block to perform guesswork on.  Returns the text block with
# formatting codes added.  This is the code that marks up various Perl
# constructs and things commonly used in man pages without requiring the user
# to add any explicit markup, and is applied to all non-literal text.  Note
# that the inserted font sequences must be treated later with mapfonts.
#
# This method is very fragile, both in the regular expressions it uses and in
# the ordering of those modifications.  Care and testing is required when
# modifying it.
sub guesswork {
    my $self = shift;
    local $_ = shift;

    # By the time we reach this point, all hyphens will be escaped by adding a
    # backslash.  We want to undo that escaping if they're part of regular
    # words and there's only a single dash, since that's a real hyphen that
    # *roff gets to consider a possible break point.  Make sure that a dash
    # after the first character of a word stays non-breaking, however.
    #
    # Note that this is not user-controllable; we pretty much have to do this
    # transformation or *roff will mangle the output in unacceptable ways.
    s{
        ( (?:\G|^|\s|$NBSP) [\(\"]* [a-zA-Z] ) ( \\- )?
        ( (?: [a-zA-Z\']+ \\-)+ )
        ( [a-zA-Z\']+ ) (?= [\)\".?!,;:]* (?:\s|$NBSP|\Z|\\\ ) )
        \b
    } {
        my ($prefix, $hyphen, $main, $suffix) = ($1, $2, $3, $4);
        $hyphen ||= '';
        $main =~ s/\\-/-/g;
        $prefix . $hyphen . $main . $suffix;
    }egx;

    # Embolden functions in the form func(), including functions that are in
    # all capitals, but don't embolden if there's anything inside the parens.
    # The function must start with an alphabetic character or underscore and
    # then consist of word characters or colons.
    if ($$self{GUESSWORK}{functions}) {
        s{
            (?<! \\ )
            \b
            ( [A-Za-z_] [:\w]+ \(\) )
        } {
            '\f(BS' . $1 . '\f(BE'
        }egx;
    }

    # Change references to manual pages to put the page name in bold but
    # the number in the regular font, with a thin space between the name and
    # the number.  Only recognize func(n) where func starts with an alphabetic
    # character or underscore and contains only word characters, periods (for
    # configuration file man pages), or colons, and n is a single digit,
    # optionally followed by some number of lowercase letters.  Note that this
    # does not recognize man page references like perl(l) or socket(3SOCKET).
    if ($$self{GUESSWORK}{manref}) {
        s{
            \b
            (?<! \\ )                                   # rule out \e0(1)
            ( [A-Za-z_] (?:[.:\w] | \\-)+ )
            ( \( \d [a-z]* \) )
        } {
            '\f(BS' . $1 . '\f(BE\|' . $2
        }egx;
    }

    # Convert simple Perl variable references to a fixed-width font.  Be
    # careful not to convert functions, though; there are too many subtleties
    # with them to want to perform this transformation.
    if ($$self{GUESSWORK}{variables}) {
        s{
           ( ^ | \s+ )
           ( [\$\@%] [\w:]+ )
           (?! \( )
        } {
            $1 . '\f(FS' . $2 . '\f(FE'
        }egx;
    }

    # Done.
    return $_;
}

##############################################################################
# Output
##############################################################################

# When building up the *roff code, we don't use real *roff fonts.  Instead, we
# embed font codes of the form \f(<font>[SE] where <font> is one of B, I, or
# F, S stands for start, and E stands for end.  This method turns these into
# the right start and end codes.
#
# We add this level of complexity because the old pod2man didn't get code like
# B<< someI<thing> else>> right.  After I<> it switched back to normal text
# rather than bold.  We take care of this by using variables that state
# whether bold, italic, or fixed are turned on as a combined pointer to our
# current font sequence, and set each to the number of current nestings of
# start tags for that font.
#
# The base font must be either \fP or \fR.  \fP changes to the previous font,
# but only one previous font is kept.  Unfortunately, there is a bug in
# Solaris 2.6 nroff (not present in GNU groff) where the sequence
# \fB\fP\f(CW\fP leaves the font set to B rather than R, presumably because
# \f(CW doesn't actually do a font change.  Because of this, we prefer to use
# \fR where possible.
#
# Unfortunately, this isn't possible for arguments to heading macros, since
# there we don't know what the outside level font is.  In that case, arrange
# things so that the outside font is always the "previous" font and end with
# \fP instead of \fR.  Idea from Zack Weinberg.
#
# This function used to be much simpler outside of macro arguments because it
# went directly from \fB to \f(CW and relied on \f(CW clearing bold since it
# wasn't \f(CB.  Unfortunately, while this works for mandoc, this is not how
# groff works; \fBfoo\f(CWbar still prints bar in bold.  Therefore, we force
# the font back to the base font before each font change.
sub mapfonts {
    my ($self, $text, $base) = @_;

    # The closure used to process each font escape, expected to be called from
    # the right-hand side of an s/// expression.
    my ($fixed, $bold, $italic) = (0, 0, 0);
    my %magic = (F => \$fixed, B => \$bold, I => \$italic);
    my $last = '\fR';
    my $process = sub {
        my ($style, $start_stop) = @_;
        my $sequence = ($last ne '\fR') ? $base : q{};
        ${ $magic{$style} } += ($start_stop eq 'S') ? 1 : -1;
        my $f = $self->{FONTS}{($fixed && 1) . ($bold && 1) . ($italic && 1)};
        return q{} if ($f eq $last);
        if ($f ne '\fR') {
            $sequence .= $f;
        }
        $last = $f;
        return $sequence;
    };

    # Now, do the actual work.
    $text =~ s{ \\f\((.)(.) }{$process->($1, $2)}xmsge;

    # We can do a bit of cleanup by collapsing sequences like \fR\fB\fR\fI
    # into just \fI.
    $text =~ s{ (?: \\fR )? (?: \\f (.|\(..) \\fR )+ }{\\fR}xms;

    return $text;
}

# Given a command and a single argument that may or may not contain double
# quotes and fixed-width text, handle double-quote formatting for it.  If
# there is no fixed-width text, just return the command followed by the
# argument with proper quoting.  If there is fixed-width text, work around a
# Solaris nroff bug with fixed-width fonts by converting fixed-width to
# regular fonts (nroff sees no difference).
sub switchquotes {
    my ($self, $command, $text, $extra) = @_;

    # Separate troff from nroff if there are any fixed-width fonts in use to
    # work around problems with Solaris nroff.
    if ($text =~ $self->{FIXEDPAT}) {
        my $nroff = $text;
        my $troff = $text;

        # Work around the Solaris nroff bug where \f(CW\fP leaves the font set
        # to Roman rather than the actual previous font when used in headings.
        # troff output may still be broken, but at least we can fix nroff by
        # just switching the font changes to the non-fixed versions.
        my $font_end = qr{ (?: \\f[PR] | \Q$self->{FONTS}{100}\E ) }xms;
        $nroff =~ s{\Q$self->{FONTS}{100}\E(.*?)\\f([PR])}{$1}xmsg;
        $nroff =~ s{\Q$self->{FONTS}{101}\E}{\\fI}xmsg;
        $nroff =~ s{\Q$self->{FONTS}{110}\E}{\\fB}xmsg;
        $nroff =~ s{\Q$self->{FONTS}{111}\E}{\\f\(BI}xmsg;

        # We have to deal with \*C` and \*C', which are used to add the quotes
        # around C<> text, since they may expand to " and if they do this
        # confuses the .SH macros and the like no end.  Expand them ourselves.
        my $c_is_quote = index("$self->{LQUOTE}$self->{RQUOTE}", qq(\")) != -1;
        if ($c_is_quote && $text =~ m{ \\[*]\(C[\'\`] }xms) {
            $nroff =~ s{ \\[*]\(C\` }{$self->{LQUOTE}}xmsg;
            $nroff =~ s{ \\[*]\(C\' }{$self->{RQUOTE}}xmsg;
            $troff =~ s{ \\[*]\(C[\'\`] }{}xmsg;
        }

        # Now finally output the command.  Bother with .ie only if the nroff
        # and troff output aren't the same.
        $nroff = _quote_macro_argument($nroff) . ($extra ? " $extra" : '');
        $troff = _quote_macro_argument($troff) . ($extra ? " $extra" : '');
        if ($nroff ne $troff) {
            return ".ie n $command $nroff\n.el $command $troff\n";
        } else {
            return "$command $nroff\n";
        }
    } else {
        $text = _quote_macro_argument($text) . ($extra ? " $extra" : '');
        return "$command $text\n";
    }
}

# Protect leading quotes and periods against interpretation as commands.  Also
# protect anything starting with a backslash, since it could expand or hide
# something that *roff would interpret as a command.  This is overkill, but
# it's much simpler than trying to parse *roff here.
sub protect {
    my ($self, $text) = @_;
    $text =~ s/^([.\'\\])/\\&$1/mg;
    return $text;
}

# Make vertical whitespace if NEEDSPACE is set, appropriate to the indentation
# level the situation.  This function is needed since in *roff one has to
# create vertical whitespace after paragraphs and between some things, but
# other macros create their own whitespace.  Also close out a sequence of
# repeated =items, since calling makespace means we're about to begin the item
# body.
sub makespace {
    my ($self) = @_;
    $self->output (".PD\n") if $$self{ITEMS} > 1;
    $$self{ITEMS} = 0;
    $self->output ($$self{INDENT} > 0 ? ".Sp\n" : ".PP\n")
        if $$self{NEEDSPACE};
}

# Output any pending index entries, and optionally an index entry given as an
# argument.  Support multiple index entries in X<> separated by slashes, and
# strip special escapes from index entries.
sub outindex {
    my ($self, $section, $index) = @_;
    my @entries = map { split m%\s*/\s*% } @{ $$self{INDEX} };
    return unless ($section || @entries);

    # We're about to output all pending entries, so clear our pending queue.
    $$self{INDEX} = [];

    # Build the output.  Regular index entries are marked Xref, and headings
    # pass in their own section.  Undo some *roff formatting on headings.
    my @output;
    if (@entries) {
        push @output, [ 'Xref', join (' ', @entries) ];
    }
    if ($section) {
        $index =~ s/\\-/-/g;
        $index =~ s/\\(?:s-?\d|.\(..|.)//g;
        push @output, [ $section, $index ];
    }

    # Print out the .IX commands.
    for (@output) {
        my ($type, $entry) = @$_;
        $entry =~ s/\s+/ /g;
        $entry =~ s/\"/\"\"/g;
        $entry =~ s/\\/\\\\/g;
        $self->output (".IX $type " . '"' . $entry . '"' . "\n");
    }
}

# Output some text, without any additional changes.
sub output {
    my ($self, @text) = @_;
    my $text = join('', @text);
    $text =~ s{$NBSP}{\\ }g;
    $text =~ s{$SHY}{\\%}g;

    if ($$self{ENCODE} && _needs_encode($$self{ENCODING})) {
        my $check = sub {
            my ($char) = @_;
            my $display = '"\x{' . hex($char) . '}"';
            my $error = "$display does not map to $$self{ENCODING}";
            $self->whine ($self->line_count(), $error);
            return Encode::encode ($$self{ENCODING}, chr($char));
        };
        my $output = Encode::encode ($$self{ENCODING}, $text, $check);
        print { $$self{output_fh} } $output;
    } else {
        print { $$self{output_fh} } $text;
    }
}

##############################################################################
# Document initialization
##############################################################################

# Handle the start of the document.  Here we handle empty documents, as well
# as setting up our basic macros in a preamble and building the page title.
sub start_document {
    my ($self, $attrs) = @_;
    if ($$attrs{contentless} && !$$self{ALWAYS_EMIT_SOMETHING}) {
        $$self{CONTENTLESS} = 1;
    } else {
        delete $$self{CONTENTLESS};
    }

    # When an encoding is requested, check whether our output file handle
    # already has a PerlIO encoding layer set.  If it does not, we'll need to
    # encode our output before printing it (handled in the output() sub).
    # Wrap the check in an eval to handle versions of Perl without PerlIO.
    #
    # PerlIO::get_layers still requires its argument be a glob, so coerce the
    # file handle to a glob.
    $$self{ENCODE} = 0;
    if ($$self{ENCODING}) {
        $$self{ENCODE} = 1;
        eval {
            my @options = (output => 1, details => 1);
            my @layers = PerlIO::get_layers (*{$$self{output_fh}}, @options);
            if ($layers[-1] && ($layers[-1] & PerlIO::F_UTF8 ())) {
                $$self{ENCODE} = 0;
            }
        }
    }

    # Determine information for the preamble and then output it unless the
    # document was content-free.
    if (!$$self{CONTENTLESS}) {
        my ($name, $section);
        if (defined $self->{opt_name}) {
            $name = $self->{opt_name};
            $section = $self->{opt_section} || 1;
        } else {
            ($name, $section) = $self->devise_title;
        }
        my $date = $self->{opt_date} // $self->devise_date();
        $self->preamble ($name, $section, $date)
            unless $self->bare_output;
    }

    # Initialize a few per-document variables.
    $$self{INDENT}    = 0;      # Current indentation level.
    $$self{INDENTS}   = [];     # Stack of indentations.
    $$self{INDEX}     = [];     # Index keys waiting to be printed.
    $$self{IN_NAME}   = 0;      # Whether processing the NAME section.
    $$self{ITEMS}     = 0;      # The number of consecutive =items.
    $$self{ITEMTYPES} = [];     # Stack of =item types, one per list.
    $$self{SHIFTWAIT} = 0;      # Whether there is a shift waiting.
    $$self{SHIFTS}    = [];     # Stack of .RS shifts.
    $$self{PENDING}   = [[]];   # Pending output.
}

# Handle the end of the document.  This handles dying on POD errors, since
# Pod::Parser currently doesn't.  Otherwise, does nothing but print out a
# final comment at the end of the document under debugging.
sub end_document {
    my ($self) = @_;
    if ($$self{complain_die} && $self->errors_seen) {
        croak ("POD document had syntax errors");
    }
    return if $self->bare_output;
    return if ($$self{CONTENTLESS} && !$$self{ALWAYS_EMIT_SOMETHING});
}

# Try to figure out the name and section from the file name and return them as
# a list, returning an empty name and section 1 if we can't find any better
# information.  Uses File::Basename and File::Spec as necessary.
sub devise_title {
    my ($self) = @_;
    my $name = $self->source_filename || '';
    my $section = $self->{opt_section} || 1;
    $section = 3 if (!$self->{opt_section} && $name =~ /\.pm\z/i);
    $name =~ s/\.p(od|[lm])\z//i;

    # If Pod::Parser gave us an IO::File reference as the source file name,
    # convert that to the empty string as well.  Then, if we don't have a
    # valid name, convert it to STDIN.
    #
    # In podlators 4.00 through 4.07, this also produced a warning, but that
    # was surprising to a lot of programs that had expected to be able to pipe
    # POD through pod2man without specifying the name.  In the name of
    # backward compatibility, just quietly set STDIN as the page title.
    if ($name =~ /^IO::File(?:=\w+)\(0x[\da-f]+\)$/i) {
        $name = '';
    }
    if ($name eq '') {
        $name = 'STDIN';
    }

    # If the section isn't 3, then the name defaults to just the basename of
    # the file.
    if ($section !~ /^3/) {
        require File::Basename;
        $name = uc File::Basename::basename ($name);
    } else {
        require File::Spec;
        my ($volume, $dirs, $file) = File::Spec->splitpath ($name);

        # Otherwise, assume we're dealing with a module.  We want to figure
        # out the full module name from the path to the file, but we don't
        # want to include too much of the path into the module name.  Lose
        # anything up to the first of:
        #
        #     */lib/*perl*/         standard or site_perl module
        #     */*perl*/lib/         from -Dprefix=/opt/perl
        #     */*perl*/             random module hierarchy
        #
        # Also strip off a leading site, site_perl, or vendor_perl component,
        # any OS-specific component, and any version number component, and
        # strip off an initial component of "lib" or "blib/lib" since that's
        # what ExtUtils::MakeMaker creates.
        #
        # splitdir requires at least File::Spec 0.8.
        my @dirs = File::Spec->splitdir ($dirs);
        if (@dirs) {
            my $cut = 0;
            my $i;
            for ($i = 0; $i < @dirs; $i++) {
                if ($dirs[$i] =~ /perl/) {
                    $cut = $i + 1;
                    $cut++ if ($dirs[$i + 1] && $dirs[$i + 1] eq 'lib');
                    last;
                }
            }
            if ($cut > 0) {
                splice (@dirs, 0, $cut);
                shift @dirs if ($dirs[0] =~ /^(site|vendor)(_perl)?$/);
                shift @dirs if ($dirs[0] =~ /^[\d.]+$/);
                shift @dirs if ($dirs[0] =~ /^(.*-$^O|$^O-.*|$^O)$/);
            }
            shift @dirs if $dirs[0] eq 'lib';
            splice (@dirs, 0, 2) if ($dirs[0] eq 'blib' && $dirs[1] eq 'lib');
        }

        # Remove empty directories when building the module name; they
        # occur too easily on Unix by doubling slashes.
        $name = join ('::', (grep { $_ ? $_ : () } @dirs), $file);
    }
    return ($name, $section);
}

# Determine the modification date and return that, properly formatted in ISO
# format.
#
# If POD_MAN_DATE is set, that overrides anything else.  This can be used for
# reproducible generation of the same file even if the input file timestamps
# are unpredictable or the POD comes from standard input.
#
# Otherwise, if SOURCE_DATE_EPOCH is set and can be parsed as seconds since
# the UNIX epoch, base the timestamp on that.  See
# <https://reproducible-builds.org/specs/source-date-epoch/>
#
# Otherwise, use the modification date of the input if we can stat it.  Be
# aware that Pod::Simple returns the stringification of the file handle as
# source_filename for input from a file handle, so we'll stat some random ref
# string in that case.  If that fails, instead use the current time.
#
# $self - Pod::Man object, used to get the source file
#
# Returns: YYYY-MM-DD date suitable for the left-hand footer
sub devise_date {
    my ($self) = @_;

    # If POD_MAN_DATE is set, always use it.
    if (defined($ENV{POD_MAN_DATE})) {
        return $ENV{POD_MAN_DATE};
    }

    # If SOURCE_DATE_EPOCH is set and can be parsed, use that.
    my $time;
    if (defined($ENV{SOURCE_DATE_EPOCH}) && $ENV{SOURCE_DATE_EPOCH} !~ /\D/) {
        $time = $ENV{SOURCE_DATE_EPOCH};
    }

    # Otherwise, get the input filename and try to stat it.  If that fails,
    # use the current time.
    if (!defined $time) {
        my $input = $self->source_filename;
        if ($input) {
            $time = (stat($input))[9] || time();
        } else {
            $time = time();
        }
    }

    # Can't use POSIX::strftime(), which uses Fcntl, because MakeMaker uses
    # this and it has to work in the core which can't load dynamic libraries.
    # Use gmtime instead of localtime so that the generated man page does not
    # depend on the local time zone setting and is more reproducible
    my ($year, $month, $day) = (gmtime($time))[5,4,3];
    return sprintf("%04d-%02d-%02d", $year + 1900, $month + 1, $day);
}

# Print out the preamble and the title.  The meaning of the arguments to .TH
# unfortunately vary by system; some systems consider the fourth argument to
# be a "source" and others use it as a version number.  Generally it's just
# presented as the left-side footer, though, so it doesn't matter too much if
# a particular system gives it another interpretation.
#
# The order of date and release used to be reversed in older versions of this
# module, but this order is correct for both Solaris and Linux.
sub preamble {
    my ($self, $name, $section, $date) = @_;
    my $preamble = $self->preamble_template();

    # groff's preconv script will use this line to correctly determine the
    # input encoding if the encoding is one of the ones it recognizes.  It
    # must be the first or second line.
    #
    # If the output encoding is some version of Unicode, we could also add a
    # Unicode Byte Order Mark to the start of the file, but the BOM is now
    # deprecated and I am concerned that may break a *roff implementation that
    # might otherwise cope with Unicode.  Revisit this if someone files a bug
    # report about it.
    if (_needs_encode($$self{ENCODING})) {
        my $normalized = lc($$self{ENCODING});
        $normalized =~ s{-}{}g;
        my $coding = $ENCODINGS{$normalized} || lc($$self{ENCODING});
        if ($coding ne 'us-ascii') {
            $self->output(qq{.\\\" -*- mode: troff; coding: $coding -*-\n});
        }
    }

    # Substitute into the preamble the configuration options.  Because it's
    # used as the argument to defining a string, any leading double quote (but
    # no other double quotes) in LQUOTE and RQUOTE has to be doubled.
    $preamble =~ s{ [@] CFONT [@] }{$self->{opt_fixed}}xms;
    my $lquote = $self->{LQUOTE};
    my $rquote = $self->{RQUOTE};
    $lquote =~ s{ \A \" }{""}xms;
    $rquote =~ s{ \A \" }{""}xms;
    $preamble =~ s{ [@] LQUOTE [@] }{$lquote}xms;
    $preamble =~ s{ [@] RQUOTE [@] }{$rquote}xms;
    chomp($preamble);

    # Get the version information.
    my $version = $self->version_report();

    # Build the index line and make sure that it will be syntactically valid.
    my $index = _quote_macro_argument("$name $section");

    # Quote the arguments to the .TH macro.  (Section should never require
    # this, but we may as well be cautious.)
    $name = _quote_macro_argument($name);
    $section = _quote_macro_argument($section);
    $date = _quote_macro_argument($date);
    my $center = _quote_macro_argument($self->{opt_center});
    my $release = _quote_macro_argument($self->{opt_release});

    # Output the majority of the preamble.
    $self->output (<<"----END OF HEADER----");
.\\" Automatically generated by $version
.\\"
.\\" Standard preamble:
.\\" ========================================================================
$preamble
.\\" ========================================================================
.\\"
.IX Title $index
.TH $name $section $date $release $center
.\\" For nroff, turn off justification.  Always turn off hyphenation; it makes
.\\" way too many mistakes in technical documents.
.if n .ad l
.nh
----END OF HEADER----

    # If the language was specified, output the language configuration.
    if ($self->{opt_language}) {
        $self->output(".mso $self->{opt_language}.tmac\n");
        $self->output(".hla $self->{opt_language}\n");
    }
}

##############################################################################
# Text blocks
##############################################################################

# Handle a basic block of text.  The only tricky part of this is if this is
# the first paragraph of text after an =over, in which case we have to change
# indentations for *roff.
sub cmd_para {
    my ($self, $attrs, $text) = @_;
    my $line = $$attrs{start_line};

    # Output the paragraph.  We also have to handle =over without =item.  If
    # there's an =over without =item, SHIFTWAIT will be set, and we need to
    # handle creation of the indent here.  Add the shift to SHIFTS so that it
    # will be cleaned up on =back.
    $self->makespace;
    if ($$self{SHIFTWAIT}) {
        $self->output (".RS $$self{INDENT}\n");
        push (@{ $$self{SHIFTS} }, $$self{INDENT});
        $$self{SHIFTWAIT} = 0;
    }

    # Force exactly one newline at the end and strip unwanted trailing
    # whitespace at the end, but leave "\ " backslashed space from an S< > at
    # the end of a line.  Reverse the text first, to avoid having to scan the
    # entire paragraph.
    $text = reverse $text;
    $text =~ s/\A\s*?(?= \\|\S|\z)/\n/;
    $text = reverse $text;

    # Output the paragraph.
    $self->output($self->protect($self->mapfonts($text, '\fR')));
    $self->outindex();
    $$self{NEEDSPACE} = 1;
    return '';
}

# Handle a verbatim paragraph.  Put a null token at the beginning of each line
# to protect against commands and wrap in .Vb/.Ve (which we define in our
# prelude).
sub cmd_verbatim {
    my ($self, $attrs, $text) = @_;

    # Ignore an empty verbatim paragraph.
    return if $text !~ m{ \S }xms;

    # Force exactly one newline at the end and strip unwanted trailing
    # whitespace at the end.
    $text =~ s{ \s* \z }{\n}xms;

    # Get a count of the number of lines before the first blank line, which
    # we'll pass to .Vb as its parameter.  This tells *roff to keep that many
    # lines together.  We don't want to tell *roff to keep huge blocks
    # together.
    my @lines = split (m{ \n }xms, $text);
    my $unbroken = 0;
    for my $line (@lines) {
        last if $line =~ m{ \A \s* \z }xms;
        $unbroken++;
    }
    if ($unbroken > 12) {
        $unbroken = 10;
    }

    # Prepend a null token to each line to preserve indentation.
    $text =~ s{ ^ }{\\&}xmsg;

    # Output the results.
    $self->makespace();
    $self->output(".Vb $unbroken\n$text.Ve\n");
    $$self{NEEDSPACE} = 1;
    return q{};
}

# Handle literal text (produced by =for and similar constructs).  Just output
# it with the minimum of changes.
sub cmd_data {
    my ($self, $attrs, $text) = @_;
    $text =~ s{ \A \n+ }{}xms;
    $text =~ s{ \n{0,2} \z }{\n}xms;
    $self->output($text);
    return q{};
}

##############################################################################
# Headings
##############################################################################

# Common code for all headings.  This is called before the actual heading is
# output.  It returns the cleaned up heading text (putting the heading all on
# one line) and may do other things, like closing bad =item blocks.
sub heading_common {
    my ($self, $text, $line) = @_;
    $text =~ s/\s+$//;
    $text =~ s/\s*\n\s*/ /g;

    # This should never happen; it means that we have a heading after =item
    # without an intervening =back.  But just in case, handle it anyway.
    if ($$self{ITEMS} > 1) {
        $$self{ITEMS} = 0;
        $self->output (".PD\n");
    }

    return $text;
}

# First level heading.  We can't output .IX in the NAME section due to a bug
# in some versions of catman, so don't output a .IX for that section.  .SH
# already uses small caps, so remove \s0 and \s-1.  Maintain IN_NAME as
# appropriate.
sub cmd_head1 {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\\s-?\d//g;
    $text = $self->heading_common ($text, $$attrs{start_line});
    my $isname = ($text eq 'NAME' || $text =~ /\(NAME\)/);
    $self->output($self->switchquotes('.SH', $self->mapfonts($text, '\fP')));
    $self->outindex ('Header', $text) unless $isname;
    $$self{NEEDSPACE} = 0;
    $$self{IN_NAME} = $isname;
    return '';
}

# Second level heading.
sub cmd_head2 {
    my ($self, $attrs, $text) = @_;
    $text = $self->heading_common ($text, $$attrs{start_line});
    $self->output($self->switchquotes('.SS', $self->mapfonts($text, '\fP')));
    $self->outindex ('Subsection', $text);
    $$self{NEEDSPACE} = 0;
    return '';
}

# Third level heading.  *roff doesn't have this concept, so just put the
# heading in italics as a normal paragraph.
sub cmd_head3 {
    my ($self, $attrs, $text) = @_;
    $text = $self->heading_common ($text, $$attrs{start_line});
    $self->makespace;
    $self->output($self->mapfonts('\f(IS' . $text . '\f(IE', '\fR') . "\n");
    $self->outindex ('Subsection', $text);
    $$self{NEEDSPACE} = 1;
    return '';
}

# Fourth level heading.  *roff doesn't have this concept, so just put the
# heading as a normal paragraph.
sub cmd_head4 {
    my ($self, $attrs, $text) = @_;
    $text = $self->heading_common ($text, $$attrs{start_line});
    $self->makespace;
    $self->output($self->mapfonts($text, '\fR') . "\n");
    $self->outindex ('Subsection', $text);
    $$self{NEEDSPACE} = 1;
    return '';
}

##############################################################################
# Formatting codes
##############################################################################

# All of the formatting codes that aren't handled internally by the parser,
# other than L<> and X<>.
sub cmd_b { return $_[0]->{IN_NAME} ? $_[2] : '\f(BS' . $_[2] . '\f(BE' }
sub cmd_i { return $_[0]->{IN_NAME} ? $_[2] : '\f(IS' . $_[2] . '\f(IE' }
sub cmd_f { return $_[0]->{IN_NAME} ? $_[2] : '\f(IS' . $_[2] . '\f(IE' }
sub cmd_c { return $_[0]->quote_literal ($_[2]) }

# Convert all internal whitespace to $NBSP.
sub cmd_s {
    my ($self, $attrs, $text) = @_;
    $text =~ s{ \s }{$NBSP}xmsg;
    return $text;
}

# Index entries are just added to the pending entries.
sub cmd_x {
    my ($self, $attrs, $text) = @_;
    push (@{ $$self{INDEX} }, $text);
    return '';
}

# Links reduce to the text that we're given, wrapped in angle brackets if it's
# a URL, followed by the URL.  We take an option to suppress the URL if anchor
# text is given.  We need to format the "to" value of the link before
# comparing it to the text since we may escape hyphens.
sub cmd_l {
    my ($self, $attrs, $text) = @_;
    if ($$attrs{type} eq 'url') {
        my $to = $$attrs{to};
        if (defined $to) {
            my $tag = $$self{PENDING}[-1];
            $to = $self->format_text ($$tag[1], $to);
        }
        if (not defined ($to) or $to eq $text) {
            return "<$text>";
        } elsif ($self->{opt_nourls}) {
            return $text;
        } else {
            return "$text <$$attrs{to}>";
        }
    } else {
        return $text;
    }
}

##############################################################################
# List handling
##############################################################################

# Handle the beginning of an =over block.  Takes the type of the block as the
# first argument, and then the attr hash.  This is called by the handlers for
# the four different types of lists (bullet, number, text, and block).
sub over_common_start {
    my ($self, $type, $attrs) = @_;
    my $line = $$attrs{start_line};
    my $indent = $$attrs{indent};

    # Find the indentation level.
    unless (defined ($indent) && $indent =~ /^[-+]?\d{1,4}\s*$/) {
        $indent = $self->{opt_indent};
    }

    # If we've gotten multiple indentations in a row, we need to emit the
    # pending indentation for the last level that we saw and haven't acted on
    # yet.  SHIFTS is the stack of indentations that we've actually emitted
    # code for.
    if (@{ $$self{SHIFTS} } < @{ $$self{INDENTS} }) {
        $self->output (".RS $$self{INDENT}\n");
        push (@{ $$self{SHIFTS} }, $$self{INDENT});
    }

    # Now, do record-keeping.  INDENTS is a stack of indentations that we've
    # seen so far, and INDENT is the current level of indentation.  ITEMTYPES
    # is a stack of list types that we've seen.
    push (@{ $$self{INDENTS} }, $$self{INDENT});
    push (@{ $$self{ITEMTYPES} }, $type);
    $$self{INDENT} = $indent + 0;
    $$self{SHIFTWAIT} = 1;
}

# End an =over block.  Takes no options other than the class pointer.
# Normally, once we close a block and therefore remove something from INDENTS,
# INDENTS will now be longer than SHIFTS, indicating that we also need to emit
# *roff code to close the indent.  This isn't *always* true, depending on the
# circumstance.  If we're still inside an indentation, we need to emit another
# .RE and then a new .RS to unconfuse *roff.
sub over_common_end {
    my ($self) = @_;
    $$self{INDENT} = pop @{ $$self{INDENTS} };
    pop @{ $$self{ITEMTYPES} };

    # If we emitted code for that indentation, end it.
    if (@{ $$self{SHIFTS} } > @{ $$self{INDENTS} }) {
        $self->output (".RE\n");
        pop @{ $$self{SHIFTS} };
    }

    # If we're still in an indentation, *roff will have now lost track of the
    # right depth of that indentation, so fix that.
    if (@{ $$self{INDENTS} } > 0) {
        $self->output (".RE\n");
        $self->output (".RS $$self{INDENT}\n");
    }
    $$self{NEEDSPACE} = 1;
    $$self{SHIFTWAIT} = 0;
}

# Dispatch the start and end calls as appropriate.
sub start_over_bullet { my $s = shift; $s->over_common_start ('bullet', @_) }
sub start_over_number { my $s = shift; $s->over_common_start ('number', @_) }
sub start_over_text   { my $s = shift; $s->over_common_start ('text',   @_) }
sub start_over_block  { my $s = shift; $s->over_common_start ('block',  @_) }
sub end_over_bullet { $_[0]->over_common_end }
sub end_over_number { $_[0]->over_common_end }
sub end_over_text   { $_[0]->over_common_end }
sub end_over_block  { $_[0]->over_common_end }

# The common handler for all item commands.  Takes the type of the item, the
# attributes, and then the text of the item.
#
# Emit an index entry for anything that's interesting, but don't emit index
# entries for things like bullets and numbers.  Newlines in an item title are
# turned into spaces since *roff can't handle them embedded.
sub item_common {
    my ($self, $type, $attrs, $text) = @_;
    my $line = $$attrs{start_line};

    # Clean up the text.  We want to end up with two variables, one ($text)
    # which contains any body text after taking out the item portion, and
    # another ($item) which contains the actual item text.
    $text =~ s/\s+$//;
    my ($item, $index);
    if ($type eq 'bullet') {
        $item = "\\\(bu";
        $text =~ s/\n*$/\n/;
    } elsif ($type eq 'number') {
        $item = $$attrs{number} . '.';
    } else {
        $item = $text;
        $item =~ s/\s*\n\s*/ /g;
        $text = '';
        $index = $item if ($item =~ /\w/);
    }

    # Take care of the indentation.  If shifts and indents are equal, close
    # the top shift, since we're about to create an indentation with .IP.
    # Also output .PD 0 to turn off spacing between items if this item is
    # directly following another one.  We only have to do that once for a
    # whole chain of items so do it for the second item in the change.  Note
    # that makespace is what undoes this.
    if (@{ $$self{SHIFTS} } == @{ $$self{INDENTS} }) {
        $self->output (".RE\n");
        pop @{ $$self{SHIFTS} };
    }
    $self->output (".PD 0\n") if ($$self{ITEMS} == 1);

    # Now, output the item tag itself.
    $item = $self->mapfonts($item, '\fR');
    $self->output($self->switchquotes('.IP', $item, $$self{INDENT}));
    $$self{NEEDSPACE} = 0;
    $$self{ITEMS}++;
    $$self{SHIFTWAIT} = 0;

    # If body text for this item was included, go ahead and output that now.
    if ($text) {
        $text =~ s/\s*$/\n/;
        $self->makespace;
        $self->output($self->protect($self->mapfonts($text, '\fR')));
        $$self{NEEDSPACE} = 1;
    }
    $self->outindex ($index ? ('Item', $index) : ());
}

# Dispatch the item commands to the appropriate place.
sub cmd_item_bullet { my $self = shift; $self->item_common ('bullet', @_) }
sub cmd_item_number { my $self = shift; $self->item_common ('number', @_) }
sub cmd_item_text   { my $self = shift; $self->item_common ('text',   @_) }
sub cmd_item_block  { my $self = shift; $self->item_common ('block',  @_) }

##############################################################################
# Backward compatibility
##############################################################################

# Reset the underlying Pod::Simple object between calls to parse_from_file so
# that the same object can be reused to convert multiple pages.
sub parse_from_file {
    my $self = shift;
    $self->reinit;

    # Fake the old cutting option to Pod::Parser.  This fiddles with internal
    # Pod::Simple state and is quite ugly; we need a better approach.
    if (ref ($_[0]) eq 'HASH') {
        my $opts = shift @_;
        if (defined ($$opts{-cutting}) && !$$opts{-cutting}) {
            $$self{in_pod} = 1;
            $$self{last_was_blank} = 1;
        }
    }

    # Do the work.
    my $retval = $self->SUPER::parse_from_file (@_);

    # Flush output, since Pod::Simple doesn't do this.  Ideally we should also
    # close the file descriptor if we had to open one, but we can't easily
    # figure this out.
    my $fh = $self->output_fh ();
    my $oldfh = select $fh;
    my $oldflush = $|;
    $| = 1;
    print $fh '';
    $| = $oldflush;
    select $oldfh;
    return $retval;
}

# Pod::Simple failed to provide this backward compatibility function, so
# implement it ourselves.  File handles are one of the inputs that
# parse_from_file supports.
sub parse_from_filehandle {
    my $self = shift;
    return $self->parse_from_file (@_);
}

# Pod::Simple's parse_file doesn't set output_fh.  Wrap the call and do so
# ourself unless it was already set by the caller, since our documentation has
# always said that this should work.
sub parse_file {
    my ($self, $in) = @_;
    unless (defined $$self{output_fh}) {
        $self->output_fh (\*STDOUT);
    }
    return $self->SUPER::parse_file ($in);
}

# Do the same for parse_lines, just to be polite.  Pod::Simple's man page
# implies that the caller is responsible for setting this, but I don't see any
# reason not to set a default.
sub parse_lines {
    my ($self, @lines) = @_;
    unless (defined $$self{output_fh}) {
        $self->output_fh (\*STDOUT);
    }
    return $self->SUPER::parse_lines (@lines);
}

# Likewise for parse_string_document.
sub parse_string_document {
    my ($self, $doc) = @_;
    unless (defined $$self{output_fh}) {
        $self->output_fh (\*STDOUT);
    }
    return $self->SUPER::parse_string_document ($doc);
}

##############################################################################
# Premable
##############################################################################

# The preamble which starts all *roff output we generate.  Most is static
# except for the font to use as a fixed-width font (designed by @CFONT@), and
# the left and right quotes to use for C<> text (designated by @LQOUTE@ and
# @RQUOTE@).  Accent marks are only defined if the output encoding is roff.
sub preamble_template {
    my ($self) = @_;
    my $preamble = <<'----END OF PREAMBLE----';
.de Sp \" Vertical space (when we can't use .PP)
.if t .sp .5v
.if n .sp
..
.de Vb \" Begin verbatim text
.ft @CFONT@
.nf
.ne \\$1
..
.de Ve \" End verbatim text
.ft R
.fi
..
.\" \*(C` and \*(C' are quotes in nroff, nothing in troff, for use with C<>.
.ie n \{\
.    ds C` @LQUOTE@
.    ds C' @RQUOTE@
'br\}
.el\{\
.    ds C`
.    ds C'
'br\}
.\"
.\" Escape single quotes in literal strings from groff's Unicode transform.
.ie \n(.g .ds Aq \(aq
.el       .ds Aq '
.\"
.\" If the F register is >0, we'll generate index entries on stderr for
.\" titles (.TH), headers (.SH), subsections (.SS), items (.Ip), and index
.\" entries marked with X<> in POD.  Of course, you'll have to process the
.\" output yourself in some meaningful fashion.
.\"
.\" Avoid warning from groff about undefined register 'F'.
.de IX
..
.nr rF 0
.if \n(.g .if rF .nr rF 1
.if (\n(rF:(\n(.g==0)) \{\
.    if \nF \{\
.        de IX
.        tm Index:\\$1\t\\n%\t"\\$2"
..
.        if !\nF==2 \{\
.            nr % 0
.            nr F 2
.        \}
.    \}
.\}
.rr rF
----END OF PREAMBLE----
#'# for cperl-mode

    if ($$self{ENCODING} eq 'roff') {
        $preamble .= <<'----END OF PREAMBLE----'
.\"
.\" Accent mark definitions (@(#)ms.acc 1.5 88/02/08 SMI; from UCB 4.2).
.\" Fear.  Run.  Save yourself.  No user-serviceable parts.
.    \" fudge factors for nroff and troff
.if n \{\
.    ds #H 0
.    ds #V .8m
.    ds #F .3m
.    ds #[ \f1
.    ds #] \fP
.\}
.if t \{\
.    ds #H ((1u-(\\\\n(.fu%2u))*.13m)
.    ds #V .6m
.    ds #F 0
.    ds #[ \&
.    ds #] \&
.\}
.    \" simple accents for nroff and troff
.if n \{\
.    ds ' \&
.    ds ` \&
.    ds ^ \&
.    ds , \&
.    ds ~ ~
.    ds /
.\}
.if t \{\
.    ds ' \\k:\h'-(\\n(.wu*8/10-\*(#H)'\'\h'|\\n:u'
.    ds ` \\k:\h'-(\\n(.wu*8/10-\*(#H)'\`\h'|\\n:u'
.    ds ^ \\k:\h'-(\\n(.wu*10/11-\*(#H)'^\h'|\\n:u'
.    ds , \\k:\h'-(\\n(.wu*8/10)',\h'|\\n:u'
.    ds ~ \\k:\h'-(\\n(.wu-\*(#H-.1m)'~\h'|\\n:u'
.    ds / \\k:\h'-(\\n(.wu*8/10-\*(#H)'\z\(sl\h'|\\n:u'
.\}
.    \" troff and (daisy-wheel) nroff accents
.ds : \\k:\h'-(\\n(.wu*8/10-\*(#H+.1m+\*(#F)'\v'-\*(#V'\z.\h'.2m+\*(#F'.\h'|\\n:u'\v'\*(#V'
.ds 8 \h'\*(#H'\(*b\h'-\*(#H'
.ds o \\k:\h'-(\\n(.wu+\w'\(de'u-\*(#H)/2u'\v'-.3n'\*(#[\z\(de\v'.3n'\h'|\\n:u'\*(#]
.ds d- \h'\*(#H'\(pd\h'-\w'~'u'\v'-.25m'\f2\(hy\fP\v'.25m'\h'-\*(#H'
.ds D- D\\k:\h'-\w'D'u'\v'-.11m'\z\(hy\v'.11m'\h'|\\n:u'
.ds th \*(#[\v'.3m'\s+1I\s-1\v'-.3m'\h'-(\w'I'u*2/3)'\s-1o\s+1\*(#]
.ds Th \*(#[\s+2I\s-2\h'-\w'I'u*3/5'\v'-.3m'o\v'.3m'\*(#]
.ds ae a\h'-(\w'a'u*4/10)'e
.ds Ae A\h'-(\w'A'u*4/10)'E
.    \" corrections for vroff
.if v .ds ~ \\k:\h'-(\\n(.wu*9/10-\*(#H)'\s-2\u~\d\s+2\h'|\\n:u'
.if v .ds ^ \\k:\h'-(\\n(.wu*10/11-\*(#H)'\v'-.4m'^\v'.4m'\h'|\\n:u'
.    \" for low resolution devices (crt and lpr)
.if \n(.H>23 .if \n(.V>19 \
\{\
.    ds : e
.    ds 8 ss
.    ds o a
.    ds d- d\h'-1'\(ga
.    ds D- D\h'-1'\(hy
.    ds th \o'bp'
.    ds Th \o'LP'
.    ds ae ae
.    ds Ae AE
.\}
.rm #[ #] #H #V #F C
----END OF PREAMBLE----
#`# for cperl-mode
    }
    return $preamble;
}

##############################################################################
# Module return value and documentation
##############################################################################

1;
__END__

=encoding UTF-8

=for stopwords
en em ALLCAPS teeny fixedbold fixeditalic fixedbolditalic stderr utf8 UTF-8
Allbery Sean Burke Ossanna Solaris formatters troff uppercased Christiansen
nourls parsers Kernighan lquote rquote unrepresentable mandoc NetBSD PostScript
SMP macOS EBCDIC fallbacks manref reflowed reflowing FH overridable

=head1 NAME

Pod::Man - Convert POD data to formatted *roff input

=head1 SYNOPSIS

    use Pod::Man;
    my $parser = Pod::Man->new (release => $VERSION, section => 8);

    # Read POD from STDIN and write to STDOUT.
    $parser->parse_file (\*STDIN);

    # Read POD from file.pod and write to file.1.
    $parser->parse_from_file ('file.pod', 'file.1');

=head1 DESCRIPTION

Pod::Man is a module to convert documentation in the POD format (the
preferred language for documenting Perl) into *roff input using the man
macro set.  The resulting *roff code is suitable for display on a terminal
using L<nroff(1)>, normally via L<man(1)>, or printing using L<troff(1)>.
It is conventionally invoked using the driver script B<pod2man>, but it can
also be used directly.

By default (on non-EBCDIC systems), Pod::Man outputs UTF-8.  Its output should
work with the B<man> program on systems that use B<groff> (most Linux
distributions) or B<mandoc> (most BSD variants), but may result in mangled
output on older UNIX systems.  To choose a different, possibly more
backward-compatible output mangling on such systems, set the C<encoding>
option to C<roff> (the default in earlier Pod::Man versions).  See the
C<encoding> option and L</ENCODING> for more details.

See L</COMPATIBILTY> for the versions of Pod::Man with significant
backward-incompatible changes (other than constructor options, whose versions
are documented below), and the versions of Perl that included them.

=head1 CLASS METHODS

=over 4

=item new(ARGS)

Create a new Pod::Man object.  ARGS should be a list of key/value pairs, where
the keys are chosen from the following.  Each option is annotated with the
version of Pod::Man in which that option was added with its current meaning.

=over 4

=item center

[1.00] Sets the centered page header for the C<.TH> macro.  The default, if
this option is not specified, is C<User Contributed Perl Documentation>.

=item date

[4.00] Sets the left-hand footer for the C<.TH> macro.  If this option is not
set, the contents of the environment variable POD_MAN_DATE, if set, will be
used.  Failing that, the value of SOURCE_DATE_EPOCH, the modification date of
the input file, or the current time if stat() can't find that file (which will
be the case if the input is from C<STDIN>) will be used.  If taken from any
source other than POD_MAN_DATE (which is used verbatim), the date will be
formatted as C<YYYY-MM-DD> and will be based on UTC (so that the output will
be reproducible regardless of local time zone).

=item encoding

[5.00] Specifies the encoding of the output.  The value must be an encoding
recognized by the L<Encode> module (see L<Encode::Supported>), or the special
values C<roff> or C<groff>.  The default on non-EBCDIC systems is UTF-8.

If the output contains characters that cannot be represented in this encoding,
that is an error that will be reported as configured by the C<errors> option.
If error handling is other than C<die>, the unrepresentable character will be
replaced with the Encode substitution character (normally C<?>).

If the C<encoding> option is set to the special value C<groff> (the default on
EBCDIC systems), or if the Encode module is not available and the encoding is
set to anything other than C<roff>, Pod::Man will translate all non-ASCII
characters to C<\[uNNNN]> Unicode escapes.  These are not traditionally part
of the *roff language, but are supported by B<groff> and B<mandoc> and thus by
the majority of manual page processors in use today.

If the C<encoding> option is set to the special value C<roff>, Pod::Man will
do its historic transformation of (some) ISO 8859-1 characters into *roff
escapes that may be adequate in troff and may be readable (if ugly) in nroff.
This was the default behavior of versions of Pod::Man before 5.00.  With this
encoding, all other non-ASCII characters will be replaced with C<X>.  It may
be required for very old troff and nroff implementations that do not support
UTF-8, but its representation of any non-ASCII character is very poor and
often specific to European languages.

If the output file handle has a PerlIO encoding layer set, setting C<encoding>
to anything other than C<groff> or C<roff> will be ignored and no encoding
will be done by Pod::Man.  It will instead rely on the encoding layer to make
whatever output encoding transformations are desired.

WARNING: The input encoding of the POD source is independent from the output
encoding, and setting this option does not affect the interpretation of the
POD input.  Unless your POD source is US-ASCII, its encoding should be
declared with the C<=encoding> command in the source.  If this is not done,
Pod::Simple will will attempt to guess the encoding and may be successful if
it's Latin-1 or UTF-8, but it will produce warnings.  See L<perlpod(1)> for
more information.

=item errors

[2.27] How to report errors.  C<die> says to throw an exception on any POD
formatting error.  C<stderr> says to report errors on standard error, but not
to throw an exception.  C<pod> says to include a POD ERRORS section in the
resulting documentation summarizing the errors.  C<none> ignores POD errors
entirely, as much as possible.

The default is C<pod>.

=item fixed

[1.00] The fixed-width font to use for verbatim text and code.  Defaults to
C<CW>.  Some systems prefer C<CR> instead.  Only matters for B<troff> output.

=item fixedbold

[1.00] Bold version of the fixed-width font.  Defaults to C<CB>.  Only matters
for B<troff> output.

=item fixeditalic

[1.00] Italic version of the fixed-width font (something of a misnomer, since
most fixed-width fonts only have an oblique version, not an italic version).
Defaults to C<CI>.  Only matters for B<troff> output.

=item fixedbolditalic

[1.00] Bold italic (in theory, probably oblique in practice) version of the
fixed-width font.  Pod::Man doesn't assume you have this, and defaults to
C<CB>.  Some systems (such as Solaris) have this font available as C<CX>.
Only matters for B<troff> output.

=item guesswork

[5.00] By default, Pod::Man applies some default formatting rules based on
guesswork and regular expressions that are intended to make writing Perl
documentation easier and require less explicit markup.  These rules may not
always be appropriate, particularly for documentation that isn't about Perl.
This option allows turning all or some of it off.

The special value C<all> enables all guesswork.  This is also the default for
backward compatibility reasons.  The special value C<none> disables all
guesswork.  Otherwise, the value of this option should be a comma-separated
list of one or more of the following keywords:

=over 4

=item functions

Convert function references like C<foo()> to bold even if they have no markup.
The function name accepts valid Perl characters for function names (including
C<:>), and the trailing parentheses must be present and empty.

=item manref

Make the first part (before the parentheses) of manual page references like
C<foo(1)> bold even if they have no markup.  The section must be a single
number optionally followed by lowercase letters.

=item quoting

If no guesswork is enabled, any text enclosed in CZ<><> is surrounded by
double quotes in nroff (terminal) output unless the contents are already
quoted.  When this guesswork is enabled, quote marks will also be suppressed
for Perl variables, function names, function calls, numbers, and hex
constants.

=item variables

Convert Perl variable names to a fixed-width font even if they have no markup.
This transformation will only be apparent in troff output, or some other
output format (unlike nroff terminal output) that supports fixed-width fonts.

=back

Any unknown guesswork name is silently ignored (for potential future
compatibility), so be careful about spelling.

=item language

[5.00] Add commands telling B<groff> that the input file is in the given
language.  The value of this setting must be a language abbreviation for which
B<groff> provides supplemental configuration, such as C<ja> (for Japanese) or
C<zh> (for Chinese).

Specifically, this adds:

    .mso <language>.tmac
    .hla <language>

to the start of the file, which configure correct line breaking for the
specified language.  Without these commands, groff may not know how to add
proper line breaks for Chinese and Japanese text if the manual page is
installed into the normal manual page directory, such as F</usr/share/man>.

On many systems, this will be done automatically if the manual page is
installed into a language-specific manual page directory, such as
F</usr/share/man/zh_CN>.  In that case, this option is not required.

Unfortunately, the commands added with this option are specific to B<groff>
and will not work with other B<troff> and B<nroff> implementations.

=item lquote

=item rquote

[4.08] Sets the quote marks used to surround CE<lt>> text.  C<lquote> sets the
left quote mark and C<rquote> sets the right quote mark.  Either may also be
set to the special value C<none>, in which case no quote mark is added on that
side of CE<lt>> text (but the font is still changed for troff output).

Also see the C<quotes> option, which can be used to set both quotes at once.
If both C<quotes> and one of the other options is set, C<lquote> or C<rquote>
overrides C<quotes>.

=item name

[4.08] Set the name of the manual page for the C<.TH> macro.  Without this
option, the manual name is set to the uppercased base name of the file being
converted unless the manual section is 3, in which case the path is parsed to
see if it is a Perl module path.  If it is, a path like C<.../lib/Pod/Man.pm>
is converted into a name like C<Pod::Man>.  This option, if given, overrides
any automatic determination of the name.

If generating a manual page from standard input, the name will be set to
C<STDIN> if this option is not provided.  In this case, providing this option
is strongly recommended to set a meaningful manual page name.

=item nourls

[2.27] Normally, LZ<><> formatting codes with a URL but anchor text are
formatted to show both the anchor text and the URL.  In other words:

    L<foo|http://example.com/>

is formatted as:

    foo <http://example.com/>

This option, if set to a true value, suppresses the URL when anchor text
is given, so this example would be formatted as just C<foo>.  This can
produce less cluttered output in cases where the URLs are not particularly
important.

=item quotes

[4.00] Sets the quote marks used to surround CE<lt>> text.  If the value is a
single character, it is used as both the left and right quote.  Otherwise, it
is split in half, and the first half of the string is used as the left quote
and the second is used as the right quote.

This may also be set to the special value C<none>, in which case no quote
marks are added around CE<lt>> text (but the font is still changed for troff
output).

Also see the C<lquote> and C<rquote> options, which can be used to set the
left and right quotes independently.  If both C<quotes> and one of the other
options is set, C<lquote> or C<rquote> overrides C<quotes>.

=item release

[1.00] Set the centered footer for the C<.TH> macro.  By default, this is set
to the version of Perl you run Pod::Man under.  Setting this to the empty
string will cause some *roff implementations to use the system default value.

Note that some system C<an> macro sets assume that the centered footer will be
a modification date and will prepend something like C<Last modified: >.  If
this is the case for your target system, you may want to set C<release> to the
last modified date and C<date> to the version number.

=item section

[1.00] Set the section for the C<.TH> macro.  The standard section numbering
convention is to use 1 for user commands, 2 for system calls, 3 for functions,
4 for devices, 5 for file formats, 6 for games, 7 for miscellaneous
information, and 8 for administrator commands.  There is a lot of variation
here, however; some systems (like Solaris) use 4 for file formats, 5 for
miscellaneous information, and 7 for devices.  Still others use 1m instead of
8, or some mix of both.  About the only section numbers that are reliably
consistent are 1, 2, and 3.

By default, section 1 will be used unless the file ends in C<.pm> in which
case section 3 will be selected.

=item stderr

[2.19] If set to a true value, send error messages about invalid POD to
standard error instead of appending a POD ERRORS section to the generated
*roff output.  This is equivalent to setting C<errors> to C<stderr> if
C<errors> is not already set.

This option is for backward compatibility with Pod::Man versions that did not
support C<errors>.  Normally, the C<errors> option should be used instead.

=item utf8

[2.21] This option used to set the output encoding to UTF-8.  Since this is
now the default, it is ignored and does nothing.

=back

=back

=head1 INSTANCE METHODS

As a derived class from Pod::Simple, Pod::Man supports the same methods and
interfaces.  See L<Pod::Simple> for all the details.  This section summarizes
the most-frequently-used methods and the ones added by Pod::Man.

=over 4

=item output_fh(FH)

Direct the output from parse_file(), parse_lines(), or parse_string_document()
to the file handle FH instead of C<STDOUT>.

=item output_string(REF)

Direct the output from parse_file(), parse_lines(), or parse_string_document()
to the scalar variable pointed to by REF, rather than C<STDOUT>.  For example:

    my $man = Pod::Man->new();
    my $output;
    $man->output_string(\$output);
    $man->parse_file('/some/input/file');

Be aware that the output in that variable will already be encoded in UTF-8.

=item parse_file(PATH)

Read the POD source from PATH and format it.  By default, the output is sent
to C<STDOUT>, but this can be changed with the output_fh() or output_string()
methods.

=item parse_from_file(INPUT, OUTPUT)

=item parse_from_filehandle(FH, OUTPUT)

Read the POD source from INPUT, format it, and output the results to OUTPUT.

parse_from_filehandle() is provided for backward compatibility with older
versions of Pod::Man.  parse_from_file() should be used instead.

=item parse_lines(LINES[, ...[, undef]])

Parse the provided lines as POD source, writing the output to either C<STDOUT>
or the file handle set with the output_fh() or output_string() methods.  This
method can be called repeatedly to provide more input lines.  An explicit
C<undef> should be passed to indicate the end of input.

This method expects raw bytes, not decoded characters.

=item parse_string_document(INPUT)

Parse the provided scalar variable as POD source, writing the output to either
C<STDOUT> or the file handle set with the output_fh() or output_string()
methods.

This method expects raw bytes, not decoded characters.

=back

=head1 ENCODING

As of Pod::Man 5.00, the default output encoding for Pod::Man is UTF-8.  This
should work correctly on any modern system that uses either B<groff> (most
Linux distributions) or B<mandoc> (Alpine Linux and most BSD variants,
including macOS).

The user will probably have to use a UTF-8 locale to see correct output.  This
may be done by default; if not, set the LANG or LC_CTYPE environment variables
to an appropriate local.  The locale C<C.UTF-8> is available on most systems
if one wants correct output without changing the other things locales affect,
such as collation.

The backward-compatible output format used in Pod::Man versions before 5.00 is
available by setting the C<encoding> option to C<roff>.  This may produce
marginally nicer results on older UNIX versions that do not use B<groff> or
B<mandoc>, but none of the available options will correctly render Unicode
characters on those systems.

Below are some additional details about how this choice was made and some
discussion of alternatives.

=head2 History

The default output encoding for Pod::Man has been a long-standing problem.
B<troff> and B<nroff> predate Unicode by a significant margin, and their
implementations for many UNIX systems reflect that legacy.  It's common for
Unicode to not be supported in any form.

Because of this, versions of Pod::Man prior to 5.00 maintained the highly
conservative output of the original pod2man, which output pure ASCII with
complex macros to simulate common western European accented characters when
processed with troff.  The nroff output was awkward and sometimes incorrect,
and characters not used in western European scripts were replaced with C<X>.
This choice maximized backwards compatibility with B<man> and
B<nroff>/B<troff> implementations at the cost of incorrect rendering of many
POD documents, particularly those containing people's names.

The modern implementations, B<groff> (used in most Linux distributions) and
B<mandoc> (used by most BSD variants), do now support Unicode.  Other UNIX
systems often do not, but they're now a tiny minority of the systems people
use on a daily basis.  It's increasingly common (for very good reasons) to use
Unicode characters for POD documents rather than using ASCII conversions of
people's names or avoiding non-English text, making the limitations in the old
output format more apparent.

Four options have been proposed to fix this:

=over 2

=item * 

Optionally support UTF-8 output but don't change the default.  This is the
approach taken since Pod::Man 2.1.0, which added the C<utf8> option.  Some
Pod::Man users use this option for better output on platforms known to support
Unicode, but since the defaults have not changed, people continued to
encounter (and file bug reports about) the poor default rendering.

=item *

Convert characters to troff C<\(xx> escapes.  This requires maintaining a
large translation table and addresses only a tiny part of the problem, since
many Unicode characters have no standard troff name.  B<groff> has the largest
list, but if one is willing to assume B<groff> is the formatter, the next
option is better.

=item *

Convert characters to groff C<\[uNNNN]> escapes.  This is implemented as the
C<groff> encoding for those who want to use it, and is supported by both
B<groff> and B<mandoc>.  However, it is no better than UTF-8 output for
portability to other implementations.  See L</Testing results> for more
details.

=item *

Change the default output format to UTF-8 and ask those who want maximum
backward compatibility to explicitly select the old encoding.  This fixes the
issue for most users at the cost of backwards compatibility.  While the
rendering of non-ASCII characters is different on older systems that don't
support UTF-8, it's not always worse than the old output.

=back

Pod::Man 5.00 and later makes the last choice.  This arguably produces worse
output when manual pages are formatted with B<troff> into PostScript or PDF,
but doing this is rare and normally manual, so the encoding can be changed in
those cases.  The older output encoding is available by setting C<encoding> to
C<roff>.

=head2 Testing results

Here is the results of testing C<encoding> values of C<utf-8> and C<groff> on
various operating systems.  The testing methodology was to create F<man/man1>
in the current directory, copy F<encoding.utf8> or F<encoding.groff> from the
podlators 5.00 distribution to F<man/man1/encoding.1>, and then run:

    LANG=C.UTF-8 MANPATH=$(pwd)/man man 1 encoding

If the locale is not explicitly set to one that includes UTF-8, the Unicode
characters were usually converted to ASCII (by, for example, dropping an
accent) or deleted or replaced with C<< <?> >> if there was no conversion.

Tested on 2022-09-25.  Many thanks to the GCC Compile Farm project for access
to testing hosts.

    OS                   UTF-8      groff
    ------------------   -------    -------
    AIX 7.1              no [1]     no [2]
    Alpine 3.15.0        yes        yes
    CentOS 7.9           yes        yes
    Debian 7             yes        yes
    FreeBSD 13.0         yes        yes
    NetBSD 9.2           yes        yes
    OpenBSD 7.1          yes        yes
    openSUSE Leap 15.4   yes        yes
    Solaris 10           yes        no [2]
    Solaris 11           no [3]     no [3]

I did not have access to a macOS system for testing, but since it uses
B<mandoc>, it's behavior is probably the same as the BSD hosts.

Notes:

=over 4

=item [1]

Unicode characters were converted to one or two random ASCII characters
unrelated to the original character.

=item [2]

Unicode characters were shown as the body of the groff escape rather than the
indicated character (in other words, text like C<[u00EF]>).

=item [3]

Unicode characters were deleted entirely, as if they weren't there.  Using
C<nroff -man> instead of B<man> to format the page showed the same results as
Solaris 10.  Using C<groff -k -man -Tutf8> to format the page produced the
correct output.

=back

PostScript and PDF output using groff on a Debian 12 system do not support
combining accent marks or SMP characters due to a lack of support in the
default output font.

Testing on additional platforms is welcome.  Please let the author know if you
have additional results.

=head1 DIAGNOSTICS

=over 4

=item roff font should be 1 or 2 chars, not "%s"

(F) You specified a *roff font (using C<fixed>, C<fixedbold>, etc.) that
wasn't either one or two characters.  Pod::Man doesn't support *roff fonts
longer than two characters, although some *roff extensions do (the
canonical versions of B<nroff> and B<troff> don't either).

=item Invalid errors setting "%s"

(F) The C<errors> parameter to the constructor was set to an unknown value.

=item Invalid quote specification "%s"

(F) The quote specification given (the C<quotes> option to the
constructor) was invalid.  A quote specification must be either one
character long or an even number (greater than one) characters long.

=item POD document had syntax errors

(F) The POD document being formatted had syntax errors and the C<errors>
option was set to C<die>.

=back

=head1 ENVIRONMENT

=over 4

=item PERL_CORE

If set and Encode is not available, silently fall back to an encoding of
C<groff> without complaining to standard error.  This environment variable is
set during Perl core builds, which build Encode after podlators.  Encode is
expected to not (yet) be available in that case.

=item POD_MAN_DATE

If set, this will be used as the value of the left-hand footer unless the
C<date> option is explicitly set, overriding the timestamp of the input
file or the current time.  This is primarily useful to ensure reproducible
builds of the same output file given the same source and Pod::Man version,
even when file timestamps may not be consistent.

=item SOURCE_DATE_EPOCH

If set, and POD_MAN_DATE and the C<date> options are not set, this will be
used as the modification time of the source file, overriding the timestamp of
the input file or the current time.  It should be set to the desired time in
seconds since UNIX epoch.  This is primarily useful to ensure reproducible
builds of the same output file given the same source and Pod::Man version,
even when file timestamps may not be consistent.  See
L<https://reproducible-builds.org/specs/source-date-epoch/> for the full
specification.

(Arguably, according to the specification, this variable should be used only
if the timestamp of the input file is not available and Pod::Man uses the
current time.  However, for reproducible builds in Debian, results were more
reliable if this variable overrode the timestamp of the input file.)

=back

=head1 COMPATIBILITY

Pod::Man 1.02 (based on L<Pod::Parser>) was the first version included with
Perl, in Perl 5.6.0.

The current API based on L<Pod::Simple> was added in Pod::Man 2.00.  Pod::Man
2.04 was included in Perl 5.9.3, the first version of Perl to incorporate
those changes.  This is the first version that correctly supports all modern
POD syntax.  The parse_from_filehandle() method was re-added for backward
compatibility in Pod::Man 2.09, included in Perl 5.9.4.

Support for anchor text in LZ<><> links of type URL was added in Pod::Man
2.23, included in Perl 5.11.5.

parse_lines(), parse_string_document(), and parse_file() set a default output
file handle of C<STDOUT> if one was not already set as of Pod::Man 2.28,
included in Perl 5.19.5.

Support for SOURCE_DATE_EPOCH and POD_MAN_DATE was added in Pod::Man 4.00,
included in Perl 5.23.7, and generated dates were changed to use UTC instead
of the local time zone.  This is also the first release that aligned the
module version and the version of the podlators distribution.  All modules
included in podlators, and the podlators distribution itself, share the same
version number from this point forward.

Pod::Man 4.10, included in Perl 5.27.8, changed the formatting for manual page
references and function names to bold instead of italic, following the current
Linux manual page standard.

Pod::Man 5.00 changed the default output encoding to UTF-8, overridable with
the new C<encoding> option.  It also fixed problems with bold or italic
extending too far when used with CZ<><> escapes, and began converting Unicode
zero-width spaces (U+200B) to the C<\:> *roff escape.  It also dropped
attempts to add subtle formatting corrections in the output that would only be
visible when typeset with B<troff>, which had previously been a significant
source of bugs.

=head1 BUGS

There are numerous bugs and language-specific assumptions in the nroff
fallbacks for accented characters in the C<roff> encoding.  Since the point of
this encoding is backward compatibility with the output from earlier versions
of Pod::Man, and it is deprecated except when necessary to support old
systems, those bugs are unlikely to ever be fixed.

Pod::Man doesn't handle font names longer than two characters.  Neither do
most B<troff> implementations, but groff does as an extension.  It would be
nice to support as an option for those who want to use it.

=head1 CAVEATS

=head2 Sentence spacing

Pod::Man copies the input spacing verbatim to the output *roff document.  This
means your output will be affected by how B<nroff> generally handles sentence
spacing.

B<nroff> dates from an era in which it was standard to use two spaces after
sentences, and will always add two spaces after a line-ending period (or
similar punctuation) when reflowing text.  For example, the following input:

    =pod

    One sentence.
    Another sentence.

will result in two spaces after the period when the text is reflowed.  If you
use two spaces after sentences anyway, this will be consistent, although you
will have to be careful to not end a line with an abbreviation such as C<e.g.>
or C<Ms.>.  Output will also be consistent if you use the *roff style guide
(and L<XKCD 1285|https://xkcd.com/1285/>) recommendation of putting a line
break after each sentence, although that will consistently produce two spaces
after each sentence, which may not be what you want.

If you prefer one space after sentences (which is the more modern style), you
will unfortunately need to ensure that no line in the middle of a paragraph
ends in a period or similar sentence-ending paragraph.  Otherwise, B<nroff>
will add a two spaces after that sentence when reflowing, and your output
document will have inconsistent spacing.

=head2 Hyphens

The handling of hyphens versus dashes is somewhat fragile, and one may get a
the wrong one under some circumstances.  This will normally only matter for
line breaking and possibly for troff output.

=head1 AUTHOR

Written by Russ Allbery <rra@cpan.org>, based on the original B<pod2man> by
Tom Christiansen <tchrist@mox.perl.com>.

The modifications to work with Pod::Simple instead of Pod::Parser were
contributed by Sean Burke <sburke@cpan.org>, but I've since hacked them beyond
recognition and all bugs are mine.

=head1 COPYRIGHT AND LICENSE

Copyright 1999-2010, 2012-2020, 2022 Russ Allbery <rra@cpan.org>

Substantial contributions by Sean Burke <sburke@cpan.org>.

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Encode::Supported>, L<Pod::Simple>, L<perlpod(1)>, L<pod2man(1)>,
L<nroff(1)>, L<troff(1)>, L<man(1)>, L<man(7)>

Ossanna, Joseph F., and Brian W. Kernighan.  "Troff User's Manual,"
Computing Science Technical Report No. 54, AT&T Bell Laboratories.  This is
the best documentation of standard B<nroff> and B<troff>.  At the time of
this writing, it's available at L<http://www.troff.org/54.pdf>.

The manual page documenting the man macro set may be L<man(5)> instead of
L<man(7)> on your system.

See L<perlpodstyle(1)> for documentation on writing manual pages in POD if
you've not done it before and aren't familiar with the conventions.

The current version of this module is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
