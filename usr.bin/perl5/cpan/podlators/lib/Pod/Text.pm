# Convert POD data to formatted text.
#
# This module converts POD to formatted text.  It replaces the old Pod::Text
# module that came with versions of Perl prior to 5.6.0 and attempts to match
# its output except for some specific circumstances where other decisions
# seemed to produce better output.  It uses Pod::Parser and is designed to be
# very easy to subclass.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

##############################################################################
# Modules and declarations
##############################################################################

package Pod::Text;

use 5.010;
use strict;
use warnings;

use Carp qw(carp croak);
use Encode qw(encode);
use Exporter ();
use Pod::Simple ();

our @ISA = qw(Pod::Simple Exporter);
our $VERSION = '5.01';

# We have to export pod2text for backward compatibility.
our @EXPORT = qw(pod2text);

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

##############################################################################
# Initialization
##############################################################################

# This function handles code blocks.  It's registered as a callback to
# Pod::Simple and therefore doesn't work as a regular method call, but all it
# does is call output_code with the line.
sub handle_code {
    my ($line, $number, $parser) = @_;
    $parser->output_code ($line . "\n");
}

# Initialize the object and set various Pod::Simple options that we need.
# Here, we also process any additional options passed to the constructor or
# set up defaults if none were given.  Note that all internal object keys are
# in all-caps, reserving all lower-case object keys for Pod::Simple and user
# arguments.
sub new {
    my $class = shift;
    my $self = $class->SUPER::new;

    # Tell Pod::Simple to keep whitespace whenever possible.
    if ($self->can ('preserve_whitespace')) {
        $self->preserve_whitespace (1);
    } else {
        $self->fullstop_space_harden (1);
    }

    # The =for and =begin targets that we accept.
    $self->accept_targets (qw/text TEXT/);

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

    # Backwards-compatibility support for the stderr option.
    if ($$self{opt_stderr} and not $$self{opt_errors}) {
        $$self{opt_errors} = 'stderr';
    }
    delete $$self{opt_stderr};

    # Backwards-compatibility support for the utf8 option.
    if ($$self{opt_utf8} && !$$self{opt_encoding}) {
        $$self{opt_encoding} = 'UTF-8';
    }
    delete $$self{opt_utf8};

    # Validate the errors parameter and act on it.
    $$self{opt_errors} //= 'pod';
    if ($$self{opt_errors} eq 'stderr' || $$self{opt_errors} eq 'die') {
        $self->no_errata_section (1);
        $self->complain_stderr (1);
        if ($$self{opt_errors} eq 'die') {
            $$self{complain_die} = 1;
        }
    } elsif ($$self{opt_errors} eq 'pod') {
        $self->no_errata_section (0);
        $self->complain_stderr (0);
    } elsif ($$self{opt_errors} eq 'none') {
        $self->no_errata_section (1);
        $self->no_whining (1);
    } else {
        croak (qq(Invalid errors setting: "$$self{errors}"));
    }
    delete $$self{errors};

    # Initialize various things from our parameters.
    $$self{opt_alt}      //= 0;
    $$self{opt_indent}   //= 4;
    $$self{opt_margin}   //= 0;
    $$self{opt_loose}    //= 0;
    $$self{opt_sentence} //= 0;
    $$self{opt_width}    //= 76;

    # Figure out what quotes we'll be using for C<> text.
    $$self{opt_quotes} ||= '"';
    if ($$self{opt_quotes} eq 'none') {
        $$self{LQUOTE} = $$self{RQUOTE} = '';
    } elsif (length ($$self{opt_quotes}) == 1) {
        $$self{LQUOTE} = $$self{RQUOTE} = $$self{opt_quotes};
    } elsif (length ($$self{opt_quotes}) % 2 == 0) {
        my $length = length ($$self{opt_quotes}) / 2;
        $$self{LQUOTE} = substr ($$self{opt_quotes}, 0, $length);
        $$self{RQUOTE} = substr ($$self{opt_quotes}, $length);
    } else {
        croak qq(Invalid quote specification "$$self{opt_quotes}");
    }

    # Configure guesswork based on options.
    my $guesswork = $self->{opt_guesswork} || q{};
    my %guesswork = map { $_ => 1 } split(m{,}xms, $guesswork);
    if (!%guesswork || $guesswork{all}) {
        $$self{GUESSWORK} = {quoting => 1};
    } elsif ($guesswork{none}) {
        $$self{GUESSWORK} = {};
    } else {
        $$self{GUESSWORK} = {%guesswork};
    }

    # If requested, do something with the non-POD text.
    $self->code_handler (\&handle_code) if $$self{opt_code};

    # Return the created object.
    return $self;
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
# represented by a tuple of the attributes hash for the tag and the contents
# of the tag.

# Add a block of text to the contents of the current node, formatting it
# according to the current formatting instructions as we do.
sub _handle_text {
    my ($self, $text) = @_;
    my $tag = $$self{PENDING}[-1];
    $$tag[1] .= $text;
}

# Given an element name, get the corresponding method name.
sub method_for_element {
    my ($self, $element) = @_;
    $element =~ tr/-/_/;
    $element =~ tr/A-Z/a-z/;
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
    # tag before calling it.
    if ($self->can ("cmd_$method")) {
        push (@{ $$self{PENDING} }, [ $attrs, '' ]);
    } elsif ($self->can ("start_$method")) {
        my $method = 'start_' . $method;
        $self->$method ($attrs, '');
    }
}

# Handle the end of an element.  If we had a cmd_ method for this element,
# this is where we pass along the text that we've accumulated.  Otherwise, if
# we have an end_ method for the element, call that.
sub _handle_element_end {
    my ($self, $element) = @_;
    my $method = $self->method_for_element ($element);

    # If we have a command handler, pull off the pending text and pass it to
    # the handler along with the saved attribute hash.
    if ($self->can ("cmd_$method")) {
        my $tag = pop @{ $$self{PENDING} };
        my $method = 'cmd_' . $method;
        my $text = $self->$method (@$tag);
        if (defined $text) {
            if (@{ $$self{PENDING} } > 1) {
                $$self{PENDING}[-1][1] .= $text;
            } else {
                $self->output ($text);
            }
        }
    } elsif ($self->can ("end_$method")) {
        my $method = 'end_' . $method;
        $self->$method ();
    }
}

##############################################################################
# Output formatting
##############################################################################

# Wrap a line, indenting by the current left margin.  We can't use Text::Wrap
# because it plays games with tabs.  We can't use formline, even though we'd
# really like to, because it screws up non-printing characters.  So we have to
# do the wrapping ourselves.
sub wrap {
    my $self = shift;
    local $_ = shift;
    my $output = '';
    my $spaces = ' ' x $$self{MARGIN};
    my $width = $$self{opt_width} - $$self{MARGIN};
    while (length > $width) {
        if (s/^([^\n]{0,$width})[ \t\n]+// || s/^([^\n]{$width})//) {
            $output .= $spaces . $1 . "\n";
        } else {
            last;
        }
    }
    $output .= $spaces . $_;
    $output =~ s/\s+$/\n\n/;
    return $output;
}

# Reformat a paragraph of text for the current margin.  Takes the text to
# reformat and returns the formatted text.
sub reformat {
    my $self = shift;
    local $_ = shift;

    # If we're trying to preserve two spaces after sentences, do some munging
    # to support that.  Otherwise, smash all repeated whitespace.  Be careful
    # not to use \s here, which in Unicode input may match non-breaking spaces
    # that we don't want to smash.
    if ($$self{opt_sentence}) {
        s/ +$//mg;
        s/\.\n/. \n/g;
        s/\n/ /g;
        s/   +/  /g;
    } else {
        s/[ \t\n]+/ /g;
    }
    return $self->wrap ($_);
}

# Output text to the output device.  Replace non-breaking spaces with spaces
# and soft hyphens with nothing, and then determine the output encoding.
sub output {
    my ($self, @text) = @_;
    my $text = join ('', @text);
    if ($NBSP) {
        $text =~ s/$NBSP/ /g;
    }
    if ($SHY) {
        $text =~ s/$SHY//g;
    }

    # The logic used here is described in the POD documentation.  Prefer the
    # configured encoding, then the pass-through option of using the same
    # encoding as the input, and then UTF-8, but commit to an encoding for the
    # document.
    #
    # ENCODE says whether to encode or not and is turned off if there is a
    # PerlIO encoding layer (in start_document).  ENCODING is the encoding
    # that we previously committed to and is cleared at the start of each
    # document.
    if ($$self{ENCODE}) {
        my $encoding = $$self{ENCODING};
        if (!$encoding) {
            $encoding = $self->encoding();
            if (!$encoding && ASCII && $text =~ /[^\x00-\x7F]/) {
                $encoding = 'UTF-8';
            }
            if ($encoding) {
                $$self{ENCODING} = $encoding;
            }
        }
        if ($encoding) {
            my $check = sub {
                my ($char) = @_;
                my $display = '"\x{' . hex($char) . '}"';
                my $error = "$display does not map to $$self{ENCODING}";
                $self->whine ($self->line_count(), $error);
                return Encode::encode ($$self{ENCODING}, chr($char));
            };
            print { $$self{output_fh} } encode ($encoding, $text, $check);
        } else {
            print { $$self{output_fh} } $text;
        }
    } else {
        print { $$self{output_fh} } $text;
    }
}

# Output a block of code (something that isn't part of the POD text).  Called
# by preprocess_paragraph only if we were given the code option.  Exists here
# only so that it can be overridden by subclasses.
sub output_code { $_[0]->output ($_[1]) }

##############################################################################
# Document initialization
##############################################################################

# Set up various things that have to be initialized on a per-document basis.
sub start_document {
    my ($self, $attrs) = @_;
    if ($$attrs{contentless} && !$$self{ALWAYS_EMIT_SOMETHING}) {
        $$self{CONTENTLESS} = 1;
    } else {
        delete $$self{CONTENTLESS};
    }
    my $margin = $$self{opt_indent} + $$self{opt_margin};

    # Initialize a few per-document variables.
    $$self{INDENTS} = [];       # Stack of indentations.
    $$self{MARGIN}  = $margin;  # Default left margin.
    $$self{PENDING} = [[]];     # Pending output.

    # We have to redo encoding handling for each document.  Check whether the
    # output file handle already has a PerlIO encoding layer set and, if so,
    # disable encoding.
    $$self{ENCODE} = 1;
    eval {
        my @options = (output => 1, details => 1);
        my $flag = (PerlIO::get_layers ($$self{output_fh}, @options))[-1];
        if ($flag && ($flag & PerlIO::F_UTF8 ())) {
            $$self{ENCODE} = 0;
        }
    };
    $$self{ENCODING} = $$self{opt_encoding};

    return '';
}

# Handle the end of the document.  The only thing we do is handle dying on POD
# errors, since Pod::Parser currently doesn't.
sub end_document {
    my ($self) = @_;
    if ($$self{complain_die} && $self->errors_seen) {
        croak ("POD document had syntax errors");
    }
}

##############################################################################
# Text blocks
##############################################################################

# Intended for subclasses to override, this method returns text with any
# non-printing formatting codes stripped out so that length() correctly
# returns the length of the text.  For basic Pod::Text, it does nothing.
sub strip_format {
    my ($self, $string) = @_;
    return $string;
}

# This method is called whenever an =item command is complete (in other words,
# we've seen its associated paragraph or know for certain that it doesn't have
# one).  It gets the paragraph associated with the item as an argument.  If
# that argument is empty, just output the item tag; if it contains a newline,
# output the item tag followed by the newline.  Otherwise, see if there's
# enough room for us to output the item tag in the margin of the text or if we
# have to put it on a separate line.
sub item {
    my ($self, $text) = @_;
    my $tag = $$self{ITEM};
    unless (defined $tag) {
        carp "Item called without tag";
        return;
    }
    undef $$self{ITEM};

    # Calculate the indentation and margin.  $fits is set to true if the tag
    # will fit into the margin of the paragraph given our indentation level.
    my $indent = $$self{INDENTS}[-1] // $$self{opt_indent};
    my $margin = ' ' x $$self{opt_margin};
    my $tag_length = length ($self->strip_format ($tag));
    my $fits = ($$self{MARGIN} - $indent >= $tag_length + 1);

    # If the tag doesn't fit, or if we have no associated text, print out the
    # tag separately.  Otherwise, put the tag in the margin of the paragraph.
    if (!$text || $text =~ /^\s+$/ || !$fits) {
        my $realindent = $$self{MARGIN};
        $$self{MARGIN} = $indent;
        my $output = $self->reformat ($tag);
        $output =~ s/^$margin /$margin:/ if ($$self{opt_alt} && $indent > 0);
        $output =~ s/\n*$/\n/;

        # If the text is just whitespace, we have an empty item paragraph;
        # this can result from =over/=item/=back without any intermixed
        # paragraphs.  Insert some whitespace to keep the =item from merging
        # into the next paragraph.
        $output .= "\n" if $text && $text =~ /^\s*$/;

        $self->output ($output);
        $$self{MARGIN} = $realindent;
        $self->output ($self->reformat ($text)) if ($text && $text =~ /\S/);
    } else {
        my $space = ' ' x $indent;
        $space =~ s/^$margin /$margin:/ if $$self{opt_alt};
        $text = $self->reformat ($text);
        $text =~ s/^$margin /$margin:/ if ($$self{opt_alt} && $indent > 0);
        my $tagspace = ' ' x $tag_length;
        $text =~ s/^($space)$tagspace/$1$tag/ or warn "Bizarre space in item";
        $self->output ($text);
    }
}

# Handle a basic block of text.  The only tricky thing here is that if there
# is a pending item tag, we need to format this as an item paragraph.
sub cmd_para {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\s+$/\n/;
    if (defined $$self{ITEM}) {
        $self->item ($text . "\n");
    } else {
        $self->output ($self->reformat ($text . "\n"));
    }
    return '';
}

# Handle a verbatim paragraph.  Just print it out, but indent it according to
# our margin.
sub cmd_verbatim {
    my ($self, $attrs, $text) = @_;
    $self->item if defined $$self{ITEM};
    return if $text =~ /^\s*$/;
    $text =~ s/^(\n*)([ \t]*\S+)/$1 . (' ' x $$self{MARGIN}) . $2/gme;
    $text =~ s/\s*$/\n\n/;
    $self->output ($text);
    return '';
}

# Handle literal text (produced by =for and similar constructs).  Just output
# it with the minimum of changes.
sub cmd_data {
    my ($self, $attrs, $text) = @_;
    $text =~ s/^\n+//;
    $text =~ s/\n{0,2}$/\n/;
    $self->output ($text);
    return '';
}

##############################################################################
# Headings
##############################################################################

# The common code for handling all headers.  Takes the header text, the
# indentation, and the surrounding marker for the alt formatting method.
sub heading {
    my ($self, $text, $indent, $marker) = @_;
    $self->item ("\n\n") if defined $$self{ITEM};
    $text =~ s/\s+$//;
    if ($$self{opt_alt}) {
        my $closemark = reverse (split (//, $marker));
        my $margin = ' ' x $$self{opt_margin};
        $self->output ("\n" . "$margin$marker $text $closemark" . "\n\n");
    } else {
        $text .= "\n" if $$self{opt_loose};
        my $margin = ' ' x ($$self{opt_margin} + $indent);
        $self->output ($margin . $text . "\n");
    }
    return '';
}

# First level heading.
sub cmd_head1 {
    my ($self, $attrs, $text) = @_;
    $self->heading ($text, 0, '====');
}

# Second level heading.
sub cmd_head2 {
    my ($self, $attrs, $text) = @_;
    $self->heading ($text, $$self{opt_indent} / 2, '==  ');
}

# Third level heading.
sub cmd_head3 {
    my ($self, $attrs, $text) = @_;
    $self->heading ($text, $$self{opt_indent} * 2 / 3 + 0.5, '=   ');
}

# Fourth level heading.
sub cmd_head4 {
    my ($self, $attrs, $text) = @_;
    $self->heading ($text, $$self{opt_indent} * 3 / 4 + 0.5, '-   ');
}

##############################################################################
# List handling
##############################################################################

# Handle the beginning of an =over block.  Takes the type of the block as the
# first argument, and then the attr hash.  This is called by the handlers for
# the four different types of lists (bullet, number, text, and block).
sub over_common_start {
    my ($self, $attrs) = @_;
    $self->item ("\n\n") if defined $$self{ITEM};

    # Find the indentation level.
    my $indent = $$attrs{indent};
    unless (defined ($indent) && $indent =~ /^\s*[-+]?\d{1,4}\s*$/) {
        $indent = $$self{opt_indent};
    }

    # Add this to our stack of indents and increase our current margin.
    push (@{ $$self{INDENTS} }, $$self{MARGIN});
    $$self{MARGIN} += ($indent + 0);
    return '';
}

# End an =over block.  Takes no options other than the class pointer.  Output
# any pending items and then pop one level of indentation.
sub over_common_end {
    my ($self) = @_;
    $self->item ("\n\n") if defined $$self{ITEM};
    $$self{MARGIN} = pop @{ $$self{INDENTS} };
    return '';
}

# Dispatch the start and end calls as appropriate.
sub start_over_bullet { $_[0]->over_common_start ($_[1]) }
sub start_over_number { $_[0]->over_common_start ($_[1]) }
sub start_over_text   { $_[0]->over_common_start ($_[1]) }
sub start_over_block  { $_[0]->over_common_start ($_[1]) }
sub end_over_bullet { $_[0]->over_common_end }
sub end_over_number { $_[0]->over_common_end }
sub end_over_text   { $_[0]->over_common_end }
sub end_over_block  { $_[0]->over_common_end }

# The common handler for all item commands.  Takes the type of the item, the
# attributes, and then the text of the item.
sub item_common {
    my ($self, $type, $attrs, $text) = @_;
    $self->item if defined $$self{ITEM};

    # Clean up the text.  We want to end up with two variables, one ($text)
    # which contains any body text after taking out the item portion, and
    # another ($item) which contains the actual item text.  Note the use of
    # the internal Pod::Simple attribute here; that's a potential land mine.
    $text =~ s/\s+$//;
    my ($item, $index);
    if ($type eq 'bullet') {
        $item = '*';
    } elsif ($type eq 'number') {
        $item = $$attrs{'~orig_content'};
    } else {
        $item = $text;
        $item =~ s/\s*\n\s*/ /g;
        $text = '';
    }
    $$self{ITEM} = $item;

    # If body text for this item was included, go ahead and output that now.
    if ($text) {
        $text =~ s/\s*$/\n/;
        $self->item ($text);
    }
    return '';
}

# Dispatch the item commands to the appropriate place.
sub cmd_item_bullet { my $self = shift; $self->item_common ('bullet', @_) }
sub cmd_item_number { my $self = shift; $self->item_common ('number', @_) }
sub cmd_item_text   { my $self = shift; $self->item_common ('text',   @_) }
sub cmd_item_block  { my $self = shift; $self->item_common ('block',  @_) }

##############################################################################
# Formatting codes
##############################################################################

# The simple ones.
sub cmd_b { return $_[0]{alt} ? "``$_[2]''" : $_[2] }
sub cmd_f { return $_[0]{alt} ? "\"$_[2]\"" : $_[2] }
sub cmd_i { return '*' . $_[2] . '*' }
sub cmd_x { return '' }

# Convert all internal whitespace to $NBSP.
sub cmd_s {
    my ($self, $attrs, $text) = @_;
    $text =~ s{ \s }{$NBSP}xmsg;
    return $text;
}

# Apply a whole bunch of messy heuristics to not quote things that don't
# benefit from being quoted.  These originally come from Barrie Slaymaker and
# largely duplicate code in Pod::Man.
sub cmd_c {
    my ($self, $attrs, $text) = @_;

    # A regex that matches the portion of a variable reference that's the
    # array or hash index, separated out just because we want to use it in
    # several places in the following regex.
    my $index = '(?: \[[^]]+\] | \{[^}]+\} )?';

    # Check for things that we don't want to quote, and if we find any of
    # them, return the string with just a font change and no quoting.
    #
    # Traditionally, Pod::Text has not quoted Perl variables, functions,
    # numbers, or hex constants, but this is not always desirable.  Make this
    # optional on the quoting guesswork flag.
    my $extra = qr{(?!)}xms;    # never matches
    if ($$self{GUESSWORK}{quoting}) {
        $extra = qr{
             \$+ [\#^]? \S $index            # special ($^F, $")
           | [\$\@%&*]+ \#? [:\'\w]+ $index  # plain var or func
           | [\$\@%&*]* [:\'\w]+
             (?: -> )? \(\s*[^\s,\)]*\s*\)   # 0/1-arg func call
           | [+-]? ( \d[\d.]* | \.\d+ )
             (?: [eE][+-]?\d+ )?             # a number
           | 0x [a-fA-F\d]+                  # a hex constant
         }xms;
    }
    $text =~ m{
      ^\s*
      (?:
         ( [\'\`\"] ) .* \1                  # already quoted
       | \` .* \'                            # `quoted'
       | $extra
      )
      \s*\z
     }xms and return $text;

    # If we didn't return, go ahead and quote the text.
    return $$self{opt_alt}
        ? "``$text''"
        : "$$self{LQUOTE}$text$$self{RQUOTE}";
}

# Links reduce to the text that we're given, wrapped in angle brackets if it's
# a URL.
sub cmd_l {
    my ($self, $attrs, $text) = @_;
    if ($$attrs{type} eq 'url') {
        if (not defined($$attrs{to}) or $$attrs{to} eq $text) {
            return "<$text>";
        } elsif ($$self{opt_nourls}) {
            return $text;
        } else {
            return "$text <$$attrs{to}>";
        }
    } else {
        return $text;
    }
}

##############################################################################
# Backwards compatibility
##############################################################################

# The old Pod::Text module did everything in a pod2text() function.  This
# tries to provide the same interface for legacy applications.
sub pod2text {
    my @args;

    # This is really ugly; I hate doing option parsing in the middle of a
    # module.  But the old Pod::Text module supported passing flags to its
    # entry function, so handle -a and -<number>.
    while ($_[0] =~ /^-/) {
        my $flag = shift;
        if    ($flag eq '-a')       { push (@args, alt => 1)    }
        elsif ($flag =~ /^-(\d+)$/) { push (@args, width => $1) }
        else {
            unshift (@_, $flag);
            last;
        }
    }

    # Now that we know what arguments we're using, create the parser.
    my $parser = Pod::Text->new (@args);

    # If two arguments were given, the second argument is going to be a file
    # handle.  That means we want to call parse_from_filehandle(), which means
    # we need to turn the first argument into a file handle.  Magic open will
    # handle the <&STDIN case automagically.
    if (defined $_[1]) {
        my @fhs = @_;
        local *IN;
        unless (open (IN, $fhs[0])) {
            croak ("Can't open $fhs[0] for reading: $!\n");
            return;
        }
        $fhs[0] = \*IN;
        $parser->output_fh ($fhs[1]);
        my $retval = $parser->parse_file ($fhs[0]);
        my $fh = $parser->output_fh ();
        close $fh;
        return $retval;
    } else {
        $parser->output_fh (\*STDOUT);
        return $parser->parse_file (@_);
    }
}

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
    my $retval = $self->Pod::Simple::parse_from_file (@_);

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
    $self->parse_from_file (@_);
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
# Module return value and documentation
##############################################################################

1;
__END__

=for stopwords
alt stderr Allbery Sean Burke's Christiansen UTF-8 pre-Unicode utf8 nourls
parsers EBCDIC autodetecting superset unrepresentable FH NNN

=head1 NAME

Pod::Text - Convert POD data to formatted text

=head1 SYNOPSIS

    use Pod::Text;
    my $parser = Pod::Text->new (sentence => 1, width => 78);

    # Read POD from STDIN and write to STDOUT.
    $parser->parse_from_filehandle;

    # Read POD from file.pod and write to file.txt.
    $parser->parse_from_file ('file.pod', 'file.txt');

=head1 DESCRIPTION

Pod::Text is a module that can convert documentation in the POD format (the
preferred language for documenting Perl) into formatted text.  It uses no
special formatting controls or codes, and its output is therefore suitable for
nearly any device.

=head2 Encoding

Pod::Text uses the following logic to choose an output encoding, in order:

=over 4

=item 1.

If a PerlIO encoding layer is set on the output file handle, do not do any
output encoding and will instead rely on the PerlIO encoding layer.

=item 2.

If the C<encoding> or C<utf8> options are set, use the output encoding
specified by those options.

=item 3.

If the input encoding of the POD source file was explicitly specified (using
C<=encoding>) or automatically detected by Pod::Simple, use that as the output
encoding as well.

=item 4.

Otherwise, if running on a non-EBCDIC system, use UTF-8 as the output
encoding.  Since this is a superset of ASCII, this will result in ASCII output
unless the POD input contains non-ASCII characters without declaring or
autodetecting an encoding (usually via EZ<><> escapes).

=item 5.

Otherwise, for EBCDIC systems, output without doing any encoding and hope
this works.

=back

One caveat: Pod::Text has to commit to an output encoding the first time it
outputs a non-ASCII character, and then has to stick with it for consistency.
However, C<=encoding> commands don't have to be at the beginning of a POD
document.  If someone uses a non-ASCII character early in a document with an
escape, such as EZ<><0xEF>, and then puts C<=encoding iso-8859-1> later,
ideally Pod::Text would follow rule 3 and output the entire document as ISO
8859-1.  Instead, it will commit to UTF-8 following rule 4 as soon as it sees
that escape, and then stick with that encoding for the rest of the document.

Unfortunately, there's no universally good choice for an output encoding.
Each choice will be incorrect in some circumstances.  This approach was chosen
primarily for backwards compatibility.  Callers should consider forcing the
output encoding via C<encoding> if they have any knowledge about what encoding
the user may expect.

In particular, consider importing the L<Encode::Locale> module, if available,
and setting C<encoding> to C<locale> to use an output encoding appropriate to
the user's locale.  But be aware that if the user is not using locales or is
using a locale of C<C>, Encode::Locale will set the output encoding to
US-ASCII.  This will cause all non-ASCII characters will be replaced with C<?>
and produce a flurry of warnings about unsupported characters, which may or
may not be what you want.

=head1 CLASS METHODS

=over 4

=item new(ARGS)

Create a new Pod::Text object.  ARGS should be a list of key/value pairs,
where the keys are chosen from the following.  Each option is annotated with
the version of Pod::Text in which that option was added with its current
meaning.

=over 4

=item alt

[2.00] If set to a true value, selects an alternate output format that, among
other things, uses a different heading style and marks C<=item> entries with a
colon in the left margin.  Defaults to false.

=item code

[2.13] If set to a true value, the non-POD parts of the input file will be
included in the output.  Useful for viewing code documented with POD blocks
with the POD rendered and the code left intact.

=item encoding

[5.00] Specifies the encoding of the output.  The value must be an encoding
recognized by the L<Encode> module (see L<Encode::Supported>).  If the output
contains characters that cannot be represented in this encoding, that is an
error that will be reported as configured by the C<errors> option.  If error
handling is other than C<die>, the unrepresentable character will be replaced
with the Encode substitution character (normally C<?>).

If the output file handle has a PerlIO encoding layer set, this parameter will
be ignored and no encoding will be done by Pod::Man.  It will instead rely on
the encoding layer to make whatever output encoding transformations are
desired.

WARNING: The input encoding of the POD source is independent from the output
encoding, and setting this option does not affect the interpretation of the
POD input.  Unless your POD source is US-ASCII, its encoding should be
declared with the C<=encoding> command in the source, as near to the top of
the file as possible.  If this is not done, Pod::Simple will will attempt to
guess the encoding and may be successful if it's Latin-1 or UTF-8, but it will
produce warnings.  See L<perlpod(1)> for more information.

=item errors

[3.17] How to report errors.  C<die> says to throw an exception on any POD
formatting error.  C<stderr> says to report errors on standard error, but not
to throw an exception.  C<pod> says to include a POD ERRORS section in the
resulting documentation summarizing the errors.  C<none> ignores POD errors
entirely, as much as possible.

The default is C<pod>.

=item guesswork

[5.01] By default, Pod::Text applies some default formatting rules based on
guesswork and regular expressions that are intended to make writing Perl
documentation easier and require less explicit markup.  These rules may not
always be appropriate, particularly for documentation that isn't about Perl.
This option allows turning all or some of it off.

The special value C<all> enables all guesswork.  This is also the default for
backward compatibility reasons.  The special value C<none> disables all
guesswork.  Otherwise, the value of this option should be a comma-separated
list of one or more of the following keywords:

=over 4

=item quoting

If no guesswork is enabled, any text enclosed in CZ<><> is surrounded by
double quotes in nroff (terminal) output unless the contents are already
quoted.  When this guesswork is enabled, quote marks will also be suppressed
for Perl variables, function names, function calls, numbers, and hex
constants.

=back

Any unknown guesswork name is silently ignored (for potential future
compatibility), so be careful about spelling.

=item indent

[2.00] The number of spaces to indent regular text, and the default
indentation for C<=over> blocks.  Defaults to 4.

=item loose

[2.00] If set to a true value, a blank line is printed after a C<=head1>
heading.  If set to false (the default), no blank line is printed after
C<=head1>, although one is still printed after C<=head2>.  This is the default
because it's the expected formatting for manual pages; if you're formatting
arbitrary text documents, setting this to true may result in more pleasing
output.

=item margin

[2.21] The width of the left margin in spaces.  Defaults to 0.  This is the
margin for all text, including headings, not the amount by which regular text
is indented; for the latter, see the I<indent> option.  To set the right
margin, see the I<width> option.

=item nourls

[3.17] Normally, LZ<><> formatting codes with a URL but anchor text are
formatted to show both the anchor text and the URL.  In other words:

    L<foo|http://example.com/>

is formatted as:

    foo <http://example.com/>

This option, if set to a true value, suppresses the URL when anchor text is
given, so this example would be formatted as just C<foo>.  This can produce
less cluttered output in cases where the URLs are not particularly important.

=item quotes

[4.00] Sets the quote marks used to surround CE<lt>> text.  If the value is a
single character, it is used as both the left and right quote.  Otherwise, it
is split in half, and the first half of the string is used as the left quote
and the second is used as the right quote.

This may also be set to the special value C<none>, in which case no quote
marks are added around CE<lt>> text.

=item sentence

[3.00] If set to a true value, Pod::Text will assume that each sentence ends
in two spaces, and will try to preserve that spacing.  If set to false, all
consecutive whitespace in non-verbatim paragraphs is compressed into a single
space.  Defaults to false.

=item stderr

[3.10] Send error messages about invalid POD to standard error instead of
appending a POD ERRORS section to the generated output.  This is equivalent to
setting C<errors> to C<stderr> if C<errors> is not already set.  It is
supported for backward compatibility.

=item utf8

[3.12] If this option is set to a true value, the output encoding is set to
UTF-8.  This is equivalent to setting C<encoding> to C<UTF-8> if C<encoding>
is not already set.  It is supported for backward compatibility.

=item width

[2.00] The column at which to wrap text on the right-hand side.  Defaults to
76.

=back

=back

=head1 INSTANCE METHODS

As a derived class from Pod::Simple, Pod::Text supports the same methods and
interfaces.  See L<Pod::Simple> for all the details.  This section summarizes
the most-frequently-used methods and the ones added by Pod::Text.

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

Be aware that the output in that variable will already be encoded (see
L</Encoding>).

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

=head1 FUNCTIONS

Pod::Text exports one function for backward compatibility with older versions.
This function is deprecated; instead, use the object-oriented interface
described above.

=over 4

=item pod2text([[-a,] [-NNN,]] INPUT[, OUTPUT])

Convert the POD source from INPUT to text and write it to OUTPUT.  If OUTPUT
is not given, defaults to C<STDOUT>.  INPUT can be any expression supported as
the second argument to two-argument open().

If C<-a> is given as an initial argument, pass the C<alt> option to the
Pod::Text constructor.  This enables alternative formatting.

If C<-NNN> is given as an initial argument, pass the C<width> option to the
Pod::Text constructor with the number C<NNN> as its argument.  This sets the
wrap line width to NNN.

=back

=head1 DIAGNOSTICS

=over 4

=item Bizarre space in item

=item Item called without tag

(W) Something has gone wrong in internal C<=item> processing.  These
messages indicate a bug in Pod::Text; you should never see them.

=item Can't open %s for reading: %s

(F) Pod::Text was invoked via the compatibility mode pod2text() interface
and the input file it was given could not be opened.

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

=head1 COMPATIBILITY

Pod::Text 2.03 (based on L<Pod::Parser>) was the first version of this module
included with Perl, in Perl 5.6.0.  Earlier versions of Perl had a different
Pod::Text module, with a different API.

The current API based on L<Pod::Simple> was added in Pod::Text 3.00.
Pod::Text 3.01 was included in Perl 5.9.3, the first version of Perl to
incorporate those changes.  This is the first version that correctly supports
all modern POD syntax.  The parse_from_filehandle() method was re-added for
backward compatibility in Pod::Text 3.07, included in Perl 5.9.4.

Pod::Text 3.12, included in Perl 5.10.1, first implemented the current
practice of attempting to match the default output encoding with the input
encoding of the POD source, unless overridden by the C<utf8> option or (added
later) the C<encoding> option.

Support for anchor text in LZ<><> links of type URL was added in Pod::Text
3.14, included in Perl 5.11.5.

parse_lines(), parse_string_document(), and parse_file() set a default output
file handle of C<STDOUT> if one was not already set as of Pod::Text 3.18,
included in Perl 5.19.5.

Pod::Text 4.00, included in Perl 5.23.7, aligned the module version and the
version of the podlators distribution.  All modules included in podlators, and
the podlators distribution itself, share the same version number from this
point forward.

Pod::Text 4.09, included in Perl 5.25.7, fixed a serious bug on EBCDIC
systems, present in all versions back to 3.00, that would cause opening
brackets to disappear.

Pod::Text 5.00 now defaults, on non-EBCDIC systems, to UTF-8 encoding if it
sees a non-ASCII character in the input and the input encoding is not
specified.  It also commits to an encoding with the first non-ASCII character
and does not change the output encoding if the input encoding changes.  The
L<Encode> module is now used for all output encoding rather than PerlIO
layers, which fixes earlier problems with output to scalars.

=head1 AUTHOR

Russ Allbery <rra@cpan.org>, based I<very> heavily on the original Pod::Text
by Tom Christiansen <tchrist@mox.perl.com> and its conversion to Pod::Parser
by Brad Appleton <bradapp@enteract.com>.  Sean Burke's initial conversion of
Pod::Man to use Pod::Simple provided much-needed guidance on how to use
Pod::Simple.

=head1 COPYRIGHT AND LICENSE

Copyright 1999-2002, 2004, 2006, 2008-2009, 2012-2016, 2018-2019, 2022 Russ
Allbery <rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Encode::Locale>, L<Encode::Supproted>, L<Pod::Simple>,
L<Pod::Text::Termcap>, L<perlpod(1)>, L<pod2text(1)>

The current version of this module is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
