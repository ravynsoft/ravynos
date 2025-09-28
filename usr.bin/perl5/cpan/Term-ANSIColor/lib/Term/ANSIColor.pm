# Color screen output using ANSI escape sequences.
#
# This module provides utility functions (in two different forms) for coloring
# output with ANSI escape sequences.
#
# This module is sometimes used in low-memory environments, so avoid use of
# \d, \w, [:upper:], and similar constructs in the most important functions
# (color, colored, AUTOLOAD, and the generated constant functions) since
# loading the Unicode attribute files consumes a lot of memory.
#
# Ah, September, when the sysadmins turn colors and fall off the trees....
#                               -- Dave Van Domelen
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

##############################################################################
# Modules and declarations
##############################################################################

package Term::ANSIColor;

use 5.008;
use strict;
use warnings;

# Also uses Carp but loads it on demand to reduce memory usage.

use Exporter;

# use Exporter plus @ISA instead of use base to reduce memory usage.
## no critic (ClassHierarchies::ProhibitExplicitISA)

# Declare variables that should be set in BEGIN for robustness.
## no critic (Modules::ProhibitAutomaticExportation)
our (@EXPORT, @EXPORT_OK, %EXPORT_TAGS, @ISA, $VERSION);

# We use autoloading, which sets this variable to the name of the called sub.
our $AUTOLOAD;

# Set $VERSION and everything export-related in a BEGIN block for robustness
# against circular module loading (not that we load any modules, but
# consistency is good).
BEGIN {
    $VERSION = '5.01';

    # All of the basic supported constants, used in %EXPORT_TAGS.
    my @colorlist = qw(
      CLEAR           RESET             BOLD            DARK
      FAINT           ITALIC            UNDERLINE       UNDERSCORE
      BLINK           REVERSE           CONCEALED

      BLACK           RED               GREEN           YELLOW
      BLUE            MAGENTA           CYAN            WHITE
      ON_BLACK        ON_RED            ON_GREEN        ON_YELLOW
      ON_BLUE         ON_MAGENTA        ON_CYAN         ON_WHITE

      BRIGHT_BLACK    BRIGHT_RED        BRIGHT_GREEN    BRIGHT_YELLOW
      BRIGHT_BLUE     BRIGHT_MAGENTA    BRIGHT_CYAN     BRIGHT_WHITE
      ON_BRIGHT_BLACK ON_BRIGHT_RED     ON_BRIGHT_GREEN ON_BRIGHT_YELLOW
      ON_BRIGHT_BLUE  ON_BRIGHT_MAGENTA ON_BRIGHT_CYAN  ON_BRIGHT_WHITE
    );

    # 256-color constants, used in %EXPORT_TAGS.
    my @colorlist256 = (
        (map { ("ANSI$_", "ON_ANSI$_") } 0 .. 255),
        (map { ("GREY$_", "ON_GREY$_") } 0 .. 23),
    );
    for my $r (0 .. 5) {
        for my $g (0 .. 5) {
            push(@colorlist256, map { ("RGB$r$g$_", "ON_RGB$r$g$_") } 0 .. 5);
        }
    }

    # Exported symbol configuration.
    @ISA         = qw(Exporter);
    @EXPORT      = qw(color colored);
    @EXPORT_OK   = qw(uncolor colorstrip colorvalid coloralias);
    %EXPORT_TAGS = (
        constants    => \@colorlist,
        constants256 => \@colorlist256,
        pushpop      => [@colorlist, qw(PUSHCOLOR POPCOLOR LOCALCOLOR)],
    );
    Exporter::export_ok_tags('pushpop', 'constants256');
}

##############################################################################
# Package variables
##############################################################################

# If this is set, any color changes will implicitly push the current color
# onto the stack and then pop it at the end of the constant sequence, just as
# if LOCALCOLOR were used.
our $AUTOLOCAL;

# Caller sets this to force a reset at the end of each constant sequence.
our $AUTORESET;

# Caller sets this to force colors to be reset at the end of each line.
our $EACHLINE;

##############################################################################
# Internal data structures
##############################################################################

# This module does quite a bit of initialization at the time it is first
# loaded, primarily to set up the package-global %ATTRIBUTES hash.  The
# entries for 256-color names are easier to handle programmatically, and
# custom colors are also imported from the environment if any are set.

# All basic supported attributes, including aliases.
#<<<
our %ATTRIBUTES = (
    'clear'          => 0,
    'reset'          => 0,
    'bold'           => 1,
    'dark'           => 2,
    'faint'          => 2,
    'italic'         => 3,
    'underline'      => 4,
    'underscore'     => 4,
    'blink'          => 5,
    'reverse'        => 7,
    'concealed'      => 8,

    'black'          => 30,   'on_black'          => 40,
    'red'            => 31,   'on_red'            => 41,
    'green'          => 32,   'on_green'          => 42,
    'yellow'         => 33,   'on_yellow'         => 43,
    'blue'           => 34,   'on_blue'           => 44,
    'magenta'        => 35,   'on_magenta'        => 45,
    'cyan'           => 36,   'on_cyan'           => 46,
    'white'          => 37,   'on_white'          => 47,

    'bright_black'   => 90,   'on_bright_black'   => 100,
    'bright_red'     => 91,   'on_bright_red'     => 101,
    'bright_green'   => 92,   'on_bright_green'   => 102,
    'bright_yellow'  => 93,   'on_bright_yellow'  => 103,
    'bright_blue'    => 94,   'on_bright_blue'    => 104,
    'bright_magenta' => 95,   'on_bright_magenta' => 105,
    'bright_cyan'    => 96,   'on_bright_cyan'    => 106,
    'bright_white'   => 97,   'on_bright_white'   => 107,
);
#>>>

# Generating the 256-color codes involves a lot of codes and offsets that are
# not helped by turning them into constants.

# The first 16 256-color codes are duplicates of the 16 ANSI colors.  The rest
# are RBG and greyscale values.
for my $code (0 .. 15) {
    $ATTRIBUTES{"ansi$code"}    = "38;5;$code";
    $ATTRIBUTES{"on_ansi$code"} = "48;5;$code";
}

# 256-color RGB colors.  Red, green, and blue can each be values 0 through 5,
# and the resulting 216 colors start with color 16.
for my $r (0 .. 5) {
    for my $g (0 .. 5) {
        for my $b (0 .. 5) {
            my $code = 16 + (6 * 6 * $r) + (6 * $g) + $b;
            $ATTRIBUTES{"rgb$r$g$b"}    = "38;5;$code";
            $ATTRIBUTES{"on_rgb$r$g$b"} = "48;5;$code";
        }
    }
}

# The last 256-color codes are 24 shades of grey.
for my $n (0 .. 23) {
    my $code = $n + 232;
    $ATTRIBUTES{"grey$n"}    = "38;5;$code";
    $ATTRIBUTES{"on_grey$n"} = "48;5;$code";
}

# Reverse lookup.  Alphabetically first name for a sequence is preferred.
our %ATTRIBUTES_R;
for my $attr (reverse(sort(keys(%ATTRIBUTES)))) {
    $ATTRIBUTES_R{ $ATTRIBUTES{$attr} } = $attr;
}

# Provide ansiN names for all 256 characters to provide a convenient flat
# namespace if one doesn't want to mess with the RGB and greyscale naming.  Do
# this after creating %ATTRIBUTES_R since we want to use the canonical names
# when reversing a color.
for my $code (16 .. 255) {
    $ATTRIBUTES{"ansi$code"}    = "38;5;$code";
    $ATTRIBUTES{"on_ansi$code"} = "48;5;$code";
}

# Import any custom colors set in the environment.
our %ALIASES;
if (exists($ENV{ANSI_COLORS_ALIASES})) {
    my $spec = $ENV{ANSI_COLORS_ALIASES};
    $spec =~ s{ \A \s+ }{}xms;
    $spec =~ s{ \s+ \z }{}xms;

    # Error reporting here is an interesting question.  Use warn rather than
    # carp because carp would report the line of the use or require, which
    # doesn't help anyone understand what's going on, whereas seeing this code
    # will be more helpful.
    ## no critic (ErrorHandling::RequireCarping)
    for my $definition (split(m{\s*,\s*}xms, $spec)) {
        my ($new, $old) = split(m{\s*=\s*}xms, $definition, 2);
        if (!$new || !$old) {
            warn qq{Bad color mapping "$definition"};
        } else {
            my $result = eval { coloralias($new, $old) };
            if (!$result) {
                my $error = $@;
                $error =~ s{ [ ] at [ ] .* }{}xms;
                warn qq{$error in "$definition"};
            }
        }
    }
}

# Stores the current color stack maintained by PUSHCOLOR and POPCOLOR.  This
# is global and therefore not threadsafe.
our @COLORSTACK;

##############################################################################
# Helper functions
##############################################################################

# Stub to load the Carp module on demand.
sub croak {
    my (@args) = @_;
    require Carp;
    Carp::croak(@args);
}

##############################################################################
# Implementation (constant form)
##############################################################################

# Time to have fun!  We now want to define the constant subs, which are named
# the same as the attributes above but in all caps.  Each constant sub needs
# to act differently depending on whether $AUTORESET is set.  Without
# autoreset:
#
#     BLUE "text\n"  ==>  "\e[34mtext\n"
#
# If $AUTORESET is set, we should instead get:
#
#     BLUE "text\n"  ==>  "\e[34mtext\n\e[0m"
#
# The sub also needs to handle the case where it has no arguments correctly.
# Maintaining all of this as separate subs would be a major nightmare, as well
# as duplicate the %ATTRIBUTES hash, so instead we define an AUTOLOAD sub to
# define the constant subs on demand.  To do that, we check the name of the
# called sub against the list of attributes, and if it's an all-caps version
# of one of them, we define the sub on the fly and then run it.
#
# If the environment variable ANSI_COLORS_DISABLED is set to a true value, or
# if the variable NO_COLOR is set, just return the arguments without adding
# any escape sequences.  This is to make it easier to write scripts that also
# work on systems without any ANSI support, like Windows consoles.
#
# Avoid using character classes like [:upper:] and \w here, since they load
# Unicode character tables and consume a ton of memory.  All of our constants
# only use ASCII characters.
#
## no critic (ClassHierarchies::ProhibitAutoloading)
## no critic (Subroutines::RequireArgUnpacking)
## no critic (RegularExpressions::ProhibitEnumeratedClasses)
sub AUTOLOAD {
    my ($sub, $attr) = $AUTOLOAD =~ m{
        \A ( [a-zA-Z0-9:]* :: ([A-Z0-9_]+) ) \z
    }xms;

    # Check if we were called with something that doesn't look like an
    # attribute.
    if (!($attr && defined($ATTRIBUTES{ lc $attr }))) {
        croak("undefined subroutine &$AUTOLOAD called");
    }

    # If colors are disabled, just return the input.  Do this without
    # installing a sub for (marginal, unbenchmarked) speed.
    if ($ENV{ANSI_COLORS_DISABLED} || defined($ENV{NO_COLOR})) {
        return join(q{}, @_);
    }

    # We've untainted the name of the sub.
    $AUTOLOAD = $sub;

    # Figure out the ANSI string to set the desired attribute.
    my $escape = "\e[" . $ATTRIBUTES{ lc $attr } . 'm';

    # Save the current value of $@.  We can't just use local since we want to
    # restore it before dispatching to the newly-created sub.  (The caller may
    # be colorizing output that includes $@.)
    my $eval_err = $@;

    # Generate the constant sub, which should still recognize some of our
    # package variables.  Use string eval to avoid a dependency on
    # Sub::Install, even though it makes it somewhat less readable.
    ## no critic (BuiltinFunctions::ProhibitStringyEval)
    ## no critic (ValuesAndExpressions::ProhibitImplicitNewlines)
    my $eval_result = eval qq{
        sub $AUTOLOAD {
            if (\$ENV{ANSI_COLORS_DISABLED} || defined(\$ENV{NO_COLOR})) {
                return join(q{}, \@_);
            } elsif (\$AUTOLOCAL && \@_) {
                return PUSHCOLOR('$escape') . join(q{}, \@_) . POPCOLOR;
            } elsif (\$AUTORESET && \@_) {
                return '$escape' . join(q{}, \@_) . "\e[0m";
            } else {
                return '$escape' . join(q{}, \@_);
            }
        }
        1;
    };

    # Failure is an internal error, not a problem with the caller.
    ## no critic (ErrorHandling::RequireCarping)
    if (!$eval_result) {
        die "failed to generate constant $attr: $@";
    }

    # Restore $@.
    ## no critic (Variables::RequireLocalizedPunctuationVars)
    $@ = $eval_err;

    # Dispatch to the newly-created sub.
    goto &$AUTOLOAD;
}
## use critic

# Append a new color to the top of the color stack and return the top of
# the stack.
#
# $text - Any text we're applying colors to, with color escapes prepended
#
# Returns: The text passed in
sub PUSHCOLOR {
    my (@text) = @_;
    my $text = join(q{}, @text);

    # Extract any number of color-setting escape sequences from the start of
    # the string.
    my ($color) = $text =~ m{ \A ( (?:\e\[ [\d;]+ m)+ ) }xms;

    # If we already have a stack, append these escapes to the set from the top
    # of the stack.  This way, each position in the stack stores the complete
    # enabled colors for that stage, at the cost of some potential
    # inefficiency.
    if (@COLORSTACK) {
        $color = $COLORSTACK[-1] . $color;
    }

    # Push the color onto the stack.
    push(@COLORSTACK, $color);
    return $text;
}

# Pop the color stack and return the new top of the stack (or reset, if
# the stack is empty).
#
# @text - Any text we're applying colors to
#
# Returns: The concatenation of @text prepended with the new stack color
sub POPCOLOR {
    my (@text) = @_;
    pop(@COLORSTACK);
    if (@COLORSTACK) {
        return $COLORSTACK[-1] . join(q{}, @text);
    } else {
        return RESET(@text);
    }
}

# Surround arguments with a push and a pop.  The effect will be to reset the
# colors to whatever was on the color stack before this sequence of colors was
# applied.
#
# @text - Any text we're applying colors to
#
# Returns: The concatenation of the text and the proper color reset sequence.
sub LOCALCOLOR {
    my (@text) = @_;
    return PUSHCOLOR(join(q{}, @text)) . POPCOLOR();
}

##############################################################################
# Implementation (attribute string form)
##############################################################################

# Return the escape code for a given set of color attributes.
#
# @codes - A list of possibly space-separated color attributes
#
# Returns: The escape sequence setting those color attributes
#          undef if no escape sequences were given
#  Throws: Text exception for any invalid attribute
sub color {
    my (@codes) = @_;

    # Return the empty string if colors are disabled.
    if ($ENV{ANSI_COLORS_DISABLED} || defined($ENV{NO_COLOR})) {
        return q{};
    }

    # Split on whitespace and expand aliases.
    @codes = map { split } @codes;
    @codes = map { defined($ALIASES{$_}) ? @{ $ALIASES{$_} } : $_ } @codes;

    # Build the attribute string from semicolon-separated numbers.
    ## no critic (RegularExpressions::ProhibitEnumeratedClasses)
    my $attribute = q{};
    for my $code (@codes) {
        $code = lc($code);
        if (defined($ATTRIBUTES{$code})) {
            $attribute .= $ATTRIBUTES{$code} . q{;};
        } elsif ($code =~ m{ \A (on_)? r([0-9]+) g([0-9]+) b([0-9]+) \z }xms) {
            my ($r, $g, $b) = ($2 + 0, $3 + 0, $4 + 0);
            if ($r > 255 || $g > 255 || $b > 255) {
                croak("Invalid attribute name $code");
            }
            my $prefix = $1 ? '48' : '38';
            $attribute .= "$prefix;2;$r;$g;$b;";
        } else {
            croak("Invalid attribute name $code");
        }
    }
    ## use critic

    # We added one too many semicolons for simplicity.  Remove the last one.
    chop($attribute);

    # Return undef if there were no attributes.
    return ($attribute ne q{}) ? "\e[${attribute}m" : undef;
}

# Return a list of named color attributes for a given set of escape codes.
# Escape sequences can be given with or without enclosing "\e[" and "m".  The
# empty escape sequence '' or "\e[m" gives an empty list of attrs.
#
# There is one special case.  256-color codes start with 38 or 48, followed by
# a 5 and then the 256-color code.
#
# @escapes - A list of escape sequences or escape sequence numbers
#
# Returns: An array of attribute names corresponding to those sequences
#  Throws: Text exceptions on invalid escape sequences or unknown colors
sub uncolor {
    my (@escapes) = @_;
    my (@nums, @result);

    # Walk the list of escapes and build a list of attribute numbers.
    for my $escape (@escapes) {
        $escape =~ s{ \A \e\[ }{}xms;
        $escape =~ s{ m \z }   {}xms;
        my ($attrs) = $escape =~ m{ \A ((?:\d+;)* \d*) \z }xms;
        if (!defined($attrs)) {
            croak("Bad escape sequence $escape");
        }

        # Pull off 256-color codes (38;5;n or 48;5;n) and true color codes
        # (38;2;n;n;n or 48;2;n;n;n) as a unit.
        my $regex = qr{
            (
                0*[34]8 ; 0*2 ; \d+ ; \d+ ; \d+
              | 0*[34]8 ; 0*5 ; \d+
              | \d+
            )
            (?: ; | \z )
        }xms;
        push(@nums, $attrs =~ m{$regex}xmsg);
    }

    # Now, walk the list of numbers and convert them to attribute names.
    # Strip leading zeroes from any of the numbers.  (xterm, at least, allows
    # leading zeroes to be added to any number in an escape sequence.)
    for my $num (@nums) {
        if ($num =~ m{ \A 0*([34])8 ; 0*2 ; (\d+) ; (\d+) ; (\d+) \z }xms) {
            my ($r, $g, $b) = ($2 + 0, $3 + 0, $4 + 0);
            if ($r > 255 || $g > 255 || $b > 255) {
                croak("No name for escape sequence $num");
            }
            my $prefix = ($1 == 4) ? 'on_' : q{};
            push(@result, "${prefix}r${r}g${g}b${b}");
        } else {
            $num =~ s{ ( \A | ; ) 0+ (\d) }{$1$2}xmsg;
            my $name = $ATTRIBUTES_R{$num};
            if (!defined($name)) {
                croak("No name for escape sequence $num");
            }
            push(@result, $name);
        }
    }

    # Return the attribute names.
    return @result;
}

# Given a string and a set of attributes, returns the string surrounded by
# escape codes to set those attributes and then clear them at the end of the
# string.  The attributes can be given either as an array ref as the first
# argument or as a list as the second and subsequent arguments.
#
# If $EACHLINE is set, insert a reset before each occurrence of the string
# $EACHLINE and the starting attribute code after the string $EACHLINE, so
# that no attribute crosses line delimiters (this is often desirable if the
# output is to be piped to a pager or some other program).
#
# $first - An anonymous array of attributes or the text to color
# @rest  - The text to color or the list of attributes
#
# Returns: The text, concatenated if necessary, surrounded by escapes to set
#          the desired colors and reset them afterwards
#  Throws: Text exception on invalid attributes
sub colored {
    my ($first, @rest) = @_;
    my ($string, @codes);
    if (ref($first) && ref($first) eq 'ARRAY') {
        @codes  = @{$first};
        $string = join(q{}, @rest);
    } else {
        $string = $first;
        @codes  = @rest;
    }

    # Return the string unmolested if colors are disabled.
    if ($ENV{ANSI_COLORS_DISABLED} || defined($ENV{NO_COLOR})) {
        return $string;
    }

    # Find the attribute string for our colors.
    my $attr = color(@codes);

    # If $EACHLINE is defined, split the string on line boundaries, suppress
    # empty segments, and then colorize each of the line sections.
    if (defined($EACHLINE)) {
        my @text = map { ($_ ne $EACHLINE) ? $attr . $_ . "\e[0m" : $_ }
          grep { length > 0 }
          split(m{ (\Q$EACHLINE\E) }xms, $string);
        return join(q{}, @text);
    } else {
        return $attr . $string . "\e[0m";
    }
}

# Define a new color alias, or return the value of an existing alias.
#
# $alias - The color alias to define
# @color - The color attributes the alias will correspond to (optional)
#
# Returns: The standard color value of the alias as a string (may be multiple
#              attributes separated by spaces)
#          undef if one argument was given and the alias was not recognized
#  Throws: Text exceptions for invalid alias names, attempts to use a
#          standard color name as an alias, or an unknown standard color name
sub coloralias {
    my ($alias, @color) = @_;
    if (!@color) {
        if (exists($ALIASES{$alias})) {
            return join(q{ }, @{ $ALIASES{$alias} });
        } else {
            return;
        }
    }

    # Avoid \w here to not load Unicode character tables, which increases the
    # memory footprint of this module considerably.
    #
    ## no critic (RegularExpressions::ProhibitEnumeratedClasses)
    if ($alias !~ m{ \A [a-zA-Z0-9._-]+ \z }xms) {
        croak(qq{Invalid alias name "$alias"});
    } elsif ($ATTRIBUTES{$alias}) {
        croak(qq{Cannot alias standard color "$alias"});
    }
    ## use critic

    # Split on whitespace and expand aliases.
    @color = map { split } @color;
    @color = map { defined($ALIASES{$_}) ? @{ $ALIASES{$_} } : $_ } @color;

    # Check that all of the attributes are valid.
    for my $attribute (@color) {
        if (!exists($ATTRIBUTES{$attribute})) {
            croak(qq{Invalid attribute name "$attribute"});
        }
    }

    # Set the alias and return.
    $ALIASES{$alias} = [@color];
    return join(q{ }, @color);
}

# Given a string, strip the ANSI color codes out of that string and return the
# result.  This removes only ANSI color codes, not movement codes and other
# escape sequences.
#
# @string - The list of strings to sanitize
#
# Returns: (array)  The strings stripped of ANSI color escape sequences
#          (scalar) The same, concatenated
sub colorstrip {
    my (@string) = @_;
    for my $string (@string) {
        $string =~ s{ \e\[ [\d;]* m }{}xmsg;
    }
    return wantarray ? @string : join(q{}, @string);
}

# Given a list of color attributes (arguments for color, for instance), return
# true if they're all valid or false if any of them are invalid.
#
# @codes - A list of color attributes, possibly space-separated
#
# Returns: True if all the attributes are valid, false otherwise.
sub colorvalid {
    my (@codes) = @_;
    @codes = map { split(q{ }, lc) } @codes;
    for my $code (@codes) {
        next if defined($ATTRIBUTES{$code});
        next if defined($ALIASES{$code});
        if ($code =~ m{ \A (?: on_ )? r (\d+) g (\d+) b (\d+) \z }xms) {
            next if ($1 <= 255 && $2 <= 255 && $3 <= 255);
        }
        return;
    }
    return 1;
}

##############################################################################
# Module return value and documentation
##############################################################################

# Ensure we evaluate to true.
1;
__END__

=head1 NAME

Term::ANSIColor - Color screen output using ANSI escape sequences

=for stopwords
cyan colorize namespace runtime TMTOWTDI cmd.exe cmd.exe. 4nt.exe. 4nt.exe
command.com NT ESC Delvare SSH OpenSSH aixterm ECMA-048 Fraktur overlining
Zenin reimplemented Allbery PUSHCOLOR POPCOLOR LOCALCOLOR openmethods.com
openmethods.com. grey ATTR urxvt mistyped prepending Bareword filehandle
Cygwin Starsinic aterm rxvt CPAN RGB Solarized Whitespace alphanumerics
undef CLICOLOR NNN GGG RRR

=head1 SYNOPSIS

    use Term::ANSIColor;
    print color('bold blue');
    print "This text is bold blue.\n";
    print color('reset');
    print "This text is normal.\n";
    print colored("Yellow on magenta.", 'yellow on_magenta'), "\n";
    print "This text is normal.\n";
    print colored(['yellow on_magenta'], 'Yellow on magenta.', "\n");
    print colored(['red on_bright_yellow'], 'Red on bright yellow.', "\n");
    print colored(['bright_red on_black'], 'Bright red on black.', "\n");
    print "\n";

    # Map escape sequences back to color names.
    use Term::ANSIColor 1.04 qw(uncolor);
    my @names = uncolor('01;31');
    print join(q{ }, @names), "\n";

    # Strip all color escape sequences.
    use Term::ANSIColor 2.01 qw(colorstrip);
    print colorstrip("\e[1mThis is bold\e[0m"), "\n";

    # Determine whether a color is valid.
    use Term::ANSIColor 2.02 qw(colorvalid);
    my $valid = colorvalid('blue bold', 'on_magenta');
    print "Color string is ", $valid ? "valid\n" : "invalid\n";

    # Create new aliases for colors.
    use Term::ANSIColor 4.00 qw(coloralias);
    coloralias('alert', 'red');
    print "Alert is ", coloralias('alert'), "\n";
    print colored("This is in red.", 'alert'), "\n";

    use Term::ANSIColor qw(:constants);
    print BOLD, BLUE, "This text is in bold blue.\n", RESET;

    use Term::ANSIColor qw(:constants);
    {
        local $Term::ANSIColor::AUTORESET = 1;
        print BOLD BLUE "This text is in bold blue.\n";
        print "This text is normal.\n";
    }

    use Term::ANSIColor 2.00 qw(:pushpop);
    print PUSHCOLOR RED ON_GREEN "This text is red on green.\n";
    print PUSHCOLOR BRIGHT_BLUE "This text is bright blue on green.\n";
    print RESET BRIGHT_BLUE "This text is just bright blue.\n";
    print POPCOLOR "Back to red on green.\n";
    print LOCALCOLOR GREEN ON_BLUE "This text is green on blue.\n";
    print "This text is red on green.\n";
    {
        local $Term::ANSIColor::AUTOLOCAL = 1;
        print ON_BLUE "This text is red on blue.\n";
        print "This text is red on green.\n";
    }
    print POPCOLOR "Back to whatever we started as.\n";

=head1 DESCRIPTION

This module has two interfaces, one through color() and colored() and the
other through constants.  It also offers the utility functions uncolor(),
colorstrip(), colorvalid(), and coloralias(), which have to be explicitly
imported to be used (see L</SYNOPSIS>).

If you are using Term::ANSIColor in a console command, consider supporting the
CLICOLOR standard.  See L</"Supporting CLICOLOR"> for more information.

See L</COMPATIBILITY> for the versions of Term::ANSIColor that introduced
particular features and the versions of Perl that included them.

=head2 Supported Colors

Terminal emulators that support color divide into four types: ones that
support only eight colors, ones that support sixteen, ones that support 256,
and ones that support 24-bit color.  This module provides the ANSI escape
codes for all of them.  These colors are referred to as ANSI colors 0 through
7 (normal), 8 through 15 (16-color), 16 through 255 (256-color), and true
color (called direct-color by B<xterm>).

Unfortunately, interpretation of colors 0 through 7 often depends on
whether the emulator supports eight colors or sixteen colors.  Emulators
that only support eight colors (such as the Linux console) will display
colors 0 through 7 with normal brightness and ignore colors 8 through 15,
treating them the same as white.  Emulators that support 16 colors, such
as gnome-terminal, normally display colors 0 through 7 as dim or darker
versions and colors 8 through 15 as normal brightness.  On such emulators,
the "normal" white (color 7) usually is shown as pale grey, requiring
bright white (15) to be used to get a real white color.  Bright black
usually is a dark grey color, although some terminals display it as pure
black.  Some sixteen-color terminal emulators also treat normal yellow
(color 3) as orange or brown, and bright yellow (color 11) as yellow.

Following the normal convention of sixteen-color emulators, this module
provides a pair of attributes for each color.  For every normal color (0
through 7), the corresponding bright color (8 through 15) is obtained by
prepending the string C<bright_> to the normal color name.  For example,
C<red> is color 1 and C<bright_red> is color 9.  The same applies for
background colors: C<on_red> is the normal color and C<on_bright_red> is
the bright color.  Capitalize these strings for the constant interface.

There is unfortunately no way to know whether the current emulator
supports more than eight colors, which makes the choice of colors
difficult.  The most conservative choice is to use only the regular
colors, which are at least displayed on all emulators.  However, they will
appear dark in sixteen-color terminal emulators, including most common
emulators in UNIX X environments.  If you know the display is one of those
emulators, you may wish to use the bright variants instead.  Even better,
offer the user a way to configure the colors for a given application to
fit their terminal emulator.

For 256-color emulators, this module additionally provides C<ansi0>
through C<ansi15>, which are the same as colors 0 through 15 in
sixteen-color emulators but use the 256-color escape syntax, C<grey0>
through C<grey23> ranging from nearly black to nearly white, and a set of
RGB colors.  The RGB colors are of the form C<rgbI<RGB>> where I<R>, I<G>,
and I<B> are numbers from 0 to 5 giving the intensity of red, green, and
blue.  The grey and RGB colors are also available as C<ansi16> through
C<ansi255> if you want simple names for all 256 colors.  C<on_> variants
of all of these colors are also provided.  These colors may be ignored
completely on non-256-color terminals or may be misinterpreted and produce
random behavior.  Additional attributes such as blink, italic, or bold may
not work with the 256-color palette.

For true color emulators, this module supports attributes of the form C<<
rI<NNN>gI<NNN>bI<NNN> >> and C<< on_rI<NNN>gI<NNN>bI<NNN> >> for all values of
I<NNN> between 0 and 255.  These represent foreground and background colors,
respectively, with the RGB values given by the I<NNN> numbers.  These colors
may be ignored completely on non-true-color terminals or may be misinterpreted
and produce random behavior.

=head2 Function Interface

The function interface uses attribute strings to describe the colors and
text attributes to assign to text.  The recognized non-color attributes
are clear, reset, bold, dark, faint, italic, underline, underscore, blink,
reverse, and concealed.  Clear and reset (reset to default attributes),
dark and faint (dim and saturated), and underline and underscore are
equivalent, so use whichever is the most intuitive to you.

Note that not all attributes are supported by all terminal types, and some
terminals may not support any of these sequences.  Dark and faint, italic,
blink, and concealed in particular are frequently not implemented.

The recognized normal foreground color attributes (colors 0 to 7) are:

  black  red  green  yellow  blue  magenta  cyan  white

The corresponding bright foreground color attributes (colors 8 to 15) are:

  bright_black  bright_red      bright_green  bright_yellow
  bright_blue   bright_magenta  bright_cyan   bright_white

The recognized normal background color attributes (colors 0 to 7) are:

  on_black  on_red      on_green  on yellow
  on_blue   on_magenta  on_cyan   on_white

The recognized bright background color attributes (colors 8 to 15) are:

  on_bright_black  on_bright_red      on_bright_green  on_bright_yellow
  on_bright_blue   on_bright_magenta  on_bright_cyan   on_bright_white

For 256-color terminals, the recognized foreground colors are:

  ansi0 .. ansi255
  grey0 .. grey23

plus C<rgbI<RGB>> for I<R>, I<G>, and I<B> values from 0 to 5, such as
C<rgb000> or C<rgb515>.  Similarly, the recognized background colors are:

  on_ansi0 .. on_ansi255
  on_grey0 .. on_grey23

plus C<on_rgbI<RGB>> for I<R>, I<G>, and I<B> values from 0 to 5.

For true color terminals, the recognized foreground colors are C<<
rI<RRR>gI<GGG>bI<BBB> >> for I<RRR>, I<GGG>, and I<BBB> values between 0 and
255.  Similarly, the recognized background colors are C<<
on_rI<RRR>gI<GGG>bI<BBB> >> for I<RRR>, I<GGG>, and I<BBB> values between 0
and 255.

For any of the above listed attributes, case is not significant.

Attributes, once set, last until they are unset (by printing the attribute
C<clear> or C<reset>).  Be careful to do this, or otherwise your attribute
will last after your script is done running, and people get very annoyed
at having their prompt and typing changed to weird colors.

=over 4

=item color(ATTR[, ATTR ...])

color() takes any number of strings as arguments and considers them to be
space-separated lists of attributes.  It then forms and returns the escape
sequence to set those attributes.  It doesn't print it out, just returns
it, so you'll have to print it yourself if you want to.  This is so that
you can save it as a string, pass it to something else, send it to a file
handle, or do anything else with it that you might care to.  color()
throws an exception if given an invalid attribute.

=item colored(STRING, ATTR[, ATTR ...])

=item colored(ATTR-REF, STRING[, STRING...])

As an aid in resetting colors, colored() takes a scalar as the first
argument and any number of attribute strings as the second argument and
returns the scalar wrapped in escape codes so that the attributes will be
set as requested before the string and reset to normal after the string.
Alternately, you can pass a reference to an array as the first argument,
and then the contents of that array will be taken as attributes and color
codes and the remainder of the arguments as text to colorize.

Normally, colored() just puts attribute codes at the beginning and end of
the string, but if you set $Term::ANSIColor::EACHLINE to some string, that
string will be considered the line delimiter and the attribute will be set
at the beginning of each line of the passed string and reset at the end of
each line.  This is often desirable if the output contains newlines and
you're using background colors, since a background color that persists
across a newline is often interpreted by the terminal as providing the
default background color for the next line.  Programs like pagers can also
be confused by attributes that span lines.  Normally you'll want to set
$Term::ANSIColor::EACHLINE to C<"\n"> to use this feature.

Particularly consider setting $Term::ANSIColor::EACHLINE if you are
interleaving output to standard output and standard error and you aren't
flushing standard output (via autoflush() or setting C<$|>).  If you don't,
the code to reset the color may unexpectedly sit in the standard output buffer
rather than going to the display, causing standard error output to appear in
the wrong color.

=item uncolor(ESCAPE)

uncolor() performs the opposite translation as color(), turning escape
sequences into a list of strings corresponding to the attributes being set
by those sequences.  uncolor() will never return C<ansi16> through
C<ansi255>, instead preferring the C<grey> and C<rgb> names (and likewise
for C<on_ansi16> through C<on_ansi255>).

=item colorstrip(STRING[, STRING ...])

colorstrip() removes all color escape sequences from the provided strings,
returning the modified strings separately in array context or joined
together in scalar context.  Its arguments are not modified.

=item colorvalid(ATTR[, ATTR ...])

colorvalid() takes attribute strings the same as color() and returns true
if all attributes are known and false otherwise.

=item coloralias(ALIAS[, ATTR ...])

If ATTR is specified, it is interpreted as a list of space-separated strings
naming attributes or existing aliases.  In this case, coloralias() sets up an
alias of ALIAS for the set of attributes given by ATTR.  From that point
forward, ALIAS can be passed into color(), colored(), and colorvalid() and
will have the same meaning as the sequence of attributes given in ATTR.  One
possible use of this facility is to give more meaningful names to the
256-color RGB colors.  Only ASCII alphanumerics, C<.>, C<_>, and C<-> are
allowed in alias names.

If ATTR includes aliases, those aliases will be expanded at definition time
and their values will be used to define the new alias.  This means that if you
define an alias A in terms of another alias B, and then later redefine alias
B, the value of alias A will not change.

If ATTR is not specified, coloralias() returns the standard attribute or
attributes to which ALIAS is aliased, if any, or undef if ALIAS does not
exist.  If it is aliased to multiple attributes, the return value will be a
single string and the attributes will be separated by spaces.

This is the same facility used by the ANSI_COLORS_ALIASES environment
variable (see L</ENVIRONMENT> below) but can be used at runtime, not just
when the module is loaded.

Later invocations of coloralias() with the same ALIAS will override
earlier aliases.  There is no way to remove an alias.

Aliases have no effect on the return value of uncolor().

B<WARNING>: Aliases are global and affect all callers in the same process.
There is no way to set an alias limited to a particular block of code or a
particular object.

=back

=head2 Constant Interface

Alternately, if you import C<:constants>, you can use the following
constants directly:

  CLEAR           RESET             BOLD            DARK
  FAINT           ITALIC            UNDERLINE       UNDERSCORE
  BLINK           REVERSE           CONCEALED

  BLACK           RED               GREEN           YELLOW
  BLUE            MAGENTA           CYAN            WHITE
  BRIGHT_BLACK    BRIGHT_RED        BRIGHT_GREEN    BRIGHT_YELLOW
  BRIGHT_BLUE     BRIGHT_MAGENTA    BRIGHT_CYAN     BRIGHT_WHITE

  ON_BLACK        ON_RED            ON_GREEN        ON_YELLOW
  ON_BLUE         ON_MAGENTA        ON_CYAN         ON_WHITE
  ON_BRIGHT_BLACK ON_BRIGHT_RED     ON_BRIGHT_GREEN ON_BRIGHT_YELLOW
  ON_BRIGHT_BLUE  ON_BRIGHT_MAGENTA ON_BRIGHT_CYAN  ON_BRIGHT_WHITE

These are the same as color('attribute') and can be used if you prefer
typing:

    print BOLD BLUE ON_WHITE "Text", RESET, "\n";

to

    print colored ("Text", 'bold blue on_white'), "\n";

(Note that the newline is kept separate to avoid confusing the terminal as
described above since a background color is being used.)

If you import C<:constants256>, you can use the following constants
directly:

  ANSI0 .. ANSI255
  GREY0 .. GREY23

  RGBXYZ (for X, Y, and Z values from 0 to 5, like RGB000 or RGB515)

  ON_ANSI0 .. ON_ANSI255
  ON_GREY0 .. ON_GREY23

  ON_RGBXYZ (for X, Y, and Z values from 0 to 5)

Note that C<:constants256> does not include the other constants, so if you
want to mix both, you need to include C<:constants> as well.  You may want
to explicitly import at least C<RESET>, as in:

    use Term::ANSIColor 4.00 qw(RESET :constants256);

True color and aliases are not supported by the constant interface.

When using the constants, if you don't want to have to remember to add the
C<, RESET> at the end of each print line, you can set
$Term::ANSIColor::AUTORESET to a true value.  Then, the display mode will
automatically be reset if there is no comma after the constant.  In other
words, with that variable set:

    print BOLD BLUE "Text\n";

will reset the display mode afterward, whereas:

    print BOLD, BLUE, "Text\n";

will not.  If you are using background colors, you will probably want to
either use say() (in newer versions of Perl) or print the newline with a
separate print statement to avoid confusing the terminal.

If $Term::ANSIColor::AUTOLOCAL is set (see below), it takes precedence
over $Term::ANSIColor::AUTORESET, and the latter is ignored.

The subroutine interface has the advantage over the constants interface in
that only two subroutines are exported into your namespace, versus
thirty-eight in the constants interface, and aliases and true color attributes
are supported.  On the flip side, the constants interface has the advantage of
better compile time error checking, since misspelled names of colors or
attributes in calls to color() and colored() won't be caught until runtime
whereas misspelled names of constants will be caught at compile time.  So,
pollute your namespace with almost two dozen subroutines that you may not even
use that often, or risk a silly bug by mistyping an attribute.  Your choice,
TMTOWTDI after all.

=head2 The Color Stack

You can import C<:pushpop> and maintain a stack of colors using PUSHCOLOR,
POPCOLOR, and LOCALCOLOR.  PUSHCOLOR takes the attribute string that
starts its argument and pushes it onto a stack of attributes.  POPCOLOR
removes the top of the stack and restores the previous attributes set by
the argument of a prior PUSHCOLOR.  LOCALCOLOR surrounds its argument in a
PUSHCOLOR and POPCOLOR so that the color resets afterward.

If $Term::ANSIColor::AUTOLOCAL is set, each sequence of color constants
will be implicitly preceded by LOCALCOLOR.  In other words, the following:

    {
        local $Term::ANSIColor::AUTOLOCAL = 1;
        print BLUE "Text\n";
    }

is equivalent to:

    print LOCALCOLOR BLUE "Text\n";

If $Term::ANSIColor::AUTOLOCAL is set, it takes precedence over
$Term::ANSIColor::AUTORESET, and the latter is ignored.

When using PUSHCOLOR, POPCOLOR, and LOCALCOLOR, it's particularly
important to not put commas between the constants.

    print PUSHCOLOR BLUE "Text\n";

will correctly push BLUE onto the top of the stack.

    print PUSHCOLOR, BLUE, "Text\n";    # wrong!

will not, and a subsequent pop won't restore the correct attributes.
PUSHCOLOR pushes the attributes set by its argument, which is normally a
string of color constants.  It can't ask the terminal what the current
attributes are.

=head2 Supporting CLICOLOR

L<https://bixense.com/clicolors/> proposes a standard for enabling and
disabling color output from console commands using two environment variables,
CLICOLOR and CLICOLOR_FORCE.  Term::ANSIColor cannot automatically support
this standard, since the correct action depends on where the output is going
and Term::ANSIColor may be used in a context where colors should always be
generated even if CLICOLOR is set in the environment.  But you can use the
supported environment variable ANSI_COLORS_DISABLED to implement CLICOLOR in
your own programs with code like this:

    if (exists($ENV{CLICOLOR}) && $ENV{CLICOLOR} == 0) {
        if (!$ENV{CLICOLOR_FORCE}) {
            $ENV{ANSI_COLORS_DISABLED} = 1;
        }
    }

If you are using the constant interface, be sure to include this code before
you use any color constants (such as at the very top of your script), since
this environment variable is only honored the first time a color constant is
seen.

Be aware that this will export ANSI_COLORS_DISABLED to any child processes of
your program as well.

=head1 DIAGNOSTICS

=over 4

=item Bad color mapping %s

(W) The specified color mapping from ANSI_COLORS_ALIASES is not valid and
could not be parsed.  It was ignored.

=item Bad escape sequence %s

(F) You passed an invalid ANSI escape sequence to uncolor().

=item Bareword "%s" not allowed while "strict subs" in use

(F) You probably mistyped a constant color name such as:

    $Foobar = FOOBAR . "This line should be blue\n";

or:

    @Foobar = FOOBAR, "This line should be blue\n";

This will only show up under use strict (another good reason to run under
use strict).

=item Cannot alias standard color %s

(F) The alias name passed to coloralias() matches a standard color name.
Standard color names cannot be aliased.

=item Cannot alias standard color %s in %s

(W) The same, but in ANSI_COLORS_ALIASES.  The color mapping was ignored.

=item Invalid alias name %s

(F) You passed an invalid alias name to coloralias().  Alias names must
consist only of alphanumerics, C<.>, C<->, and C<_>.

=item Invalid alias name %s in %s

(W) You specified an invalid alias name on the left hand of the equal sign
in a color mapping in ANSI_COLORS_ALIASES.  The color mapping was ignored.

=item Invalid attribute name %s

(F) You passed an invalid attribute name to color(), colored(), or
coloralias().

=item Invalid attribute name %s in %s

(W) You specified an invalid attribute name on the right hand of the equal
sign in a color mapping in ANSI_COLORS_ALIASES.  The color mapping was
ignored.

=item Name "%s" used only once: possible typo

(W) You probably mistyped a constant color name such as:

    print FOOBAR "This text is color FOOBAR\n";

It's probably better to always use commas after constant names in order to
force the next error.

=item No comma allowed after filehandle

(F) You probably mistyped a constant color name such as:

    print FOOBAR, "This text is color FOOBAR\n";

Generating this fatal compile error is one of the main advantages of using
the constants interface, since you'll immediately know if you mistype a
color name.

=item No name for escape sequence %s

(F) The ANSI escape sequence passed to uncolor() contains escapes which
aren't recognized and can't be translated to names.

=back

=head1 ENVIRONMENT

=over 4

=item ANSI_COLORS_ALIASES

This environment variable allows the user to specify custom color aliases
that will be understood by color(), colored(), and colorvalid().  None of
the other functions will be affected, and no new color constants will be
created.  The custom colors are aliases for existing color names; no new
escape sequences can be introduced.  Only alphanumerics, C<.>, C<_>, and
C<-> are allowed in alias names.

The format is:

    ANSI_COLORS_ALIASES='newcolor1=oldcolor1,newcolor2=oldcolor2'

Whitespace is ignored.  The alias value can be a single attribute or a
space-separated list of attributes.

For example the L<Solarized|https://ethanschoonover.com/solarized> colors
can be mapped with:

    ANSI_COLORS_ALIASES='\
        base00=bright_yellow, on_base00=on_bright_yellow,\
        base01=bright_green,  on_base01=on_bright_green, \
        base02=black,         on_base02=on_black,        \
        base03=bright_black,  on_base03=on_bright_black, \
        base0=bright_blue,    on_base0=on_bright_blue,   \
        base1=bright_cyan,    on_base1=on_bright_cyan,   \
        base2=white,          on_base2=on_white,         \
        base3=bright_white,   on_base3=on_bright_white,  \
        orange=bright_red,    on_orange=on_bright_red,   \
        violet=bright_magenta,on_violet=on_bright_magenta'

This environment variable is read and applied when the Term::ANSIColor
module is loaded and is then subsequently ignored.  Changes to
ANSI_COLORS_ALIASES after the module is loaded will have no effect.  See
coloralias() for an equivalent facility that can be used at runtime.

=item ANSI_COLORS_DISABLED

If this environment variable is set to a true value, all of the functions
defined by this module (color(), colored(), and all of the constants) will not
output any escape sequences and instead will just return the empty string or
pass through the original text as appropriate.  This is intended to support
easy use of scripts using this module on platforms that don't support ANSI
escape sequences.

=item NO_COLOR

If this environment variable is set to any value, it suppresses generation of
escape sequences the same as if ANSI_COLORS_DISABLED is set to a true value.
This implements the L<https://no-color.org/> informal standard.  Programs that
want to enable color despite NO_COLOR being set will need to unset that
environment variable before any constant or function provided by this module
is used.

=back

=head1 COMPATIBILITY

Term::ANSIColor was first included with Perl in Perl 5.6.0.

The uncolor() function and support for ANSI_COLORS_DISABLED were added in
Term::ANSIColor 1.04, included in Perl 5.8.0.

Support for dark was added in Term::ANSIColor 1.08, included in Perl
5.8.4.

The color stack, including the C<:pushpop> import tag, PUSHCOLOR,
POPCOLOR, LOCALCOLOR, and the $Term::ANSIColor::AUTOLOCAL variable, was
added in Term::ANSIColor 2.00, included in Perl 5.10.1.

colorstrip() was added in Term::ANSIColor 2.01 and colorvalid() was added
in Term::ANSIColor 2.02, both included in Perl 5.11.0.

Support for colors 8 through 15 (the C<bright_> variants) was added in
Term::ANSIColor 3.00, included in Perl 5.13.3.

Support for italic was added in Term::ANSIColor 3.02, included in Perl
5.17.1.

Support for colors 16 through 256 (the C<ansi>, C<rgb>, and C<grey>
colors), the C<:constants256> import tag, the coloralias() function, and
support for the ANSI_COLORS_ALIASES environment variable were added in
Term::ANSIColor 4.00, included in Perl 5.17.8.

$Term::ANSIColor::AUTOLOCAL was changed to take precedence over
$Term::ANSIColor::AUTORESET, rather than the other way around, in
Term::ANSIColor 4.00, included in Perl 5.17.8.

C<ansi16> through C<ansi255>, as aliases for the C<rgb> and C<grey> colors,
and the corresponding C<on_ansi> names and C<ANSI> and C<ON_ANSI> constants
were added in Term::ANSIColor 4.06, included in Perl 5.25.7.

Support for true color (the C<rNNNgNNNbNNN> and C<on_rNNNgNNNbNNN>
attributes), defining aliases in terms of other aliases, and aliases mapping
to multiple attributes instead of only a single attribute was added in
Term::ANSIColor 5.00.

Support for NO_COLOR was added in Term::ANSIColor 5.01.

=head1 RESTRICTIONS

Both colored() and many uses of the color constants will add the reset escape
sequence after a newline.  If a program mixes colored output to standard
output with output to standard error, this can result in the standard error
text having the wrong color because the reset escape sequence hasn't yet been
flushed to the display (since standard output to a terminal is line-buffered
by default).  To avoid this, either set autoflush() on STDOUT or set
$Term::ANSIColor::EACHLINE to C<"\n">.

It would be nice if one could leave off the commas around the constants
entirely and just say:

    print BOLD BLUE ON_WHITE "Text\n" RESET;

but the syntax of Perl doesn't allow this.  You need a comma after the
string.  (Of course, you may consider it a bug that commas between all the
constants aren't required, in which case you may feel free to insert
commas unless you're using $Term::ANSIColor::AUTORESET or
PUSHCOLOR/POPCOLOR.)

For easier debugging, you may prefer to always use the commas when not
setting $Term::ANSIColor::AUTORESET or PUSHCOLOR/POPCOLOR so that you'll
get a fatal compile error rather than a warning.

It's not possible to use this module to embed formatting and color
attributes using Perl formats.  They replace the escape character with a
space (as documented in L<perlform(1)>), resulting in garbled output from
the unrecognized attribute.  Even if there were a way around that problem,
the format doesn't know that the non-printing escape sequence is
zero-length and would incorrectly format the output.  For formatted output
using color or other attributes, either use sprintf() instead or use
formline() and then add the color or other attributes after formatting and
before output.

=head1 NOTES

The codes generated by this module are standard terminal control codes,
complying with ECMA-048 and ISO 6429 (generally referred to as "ANSI
color" for the color codes).  The non-color control codes (bold, dark,
italic, underline, and reverse) are part of the earlier ANSI X3.64
standard for control sequences for video terminals and peripherals.

Note that not all displays are ISO 6429-compliant, or even X3.64-compliant
(or are even attempting to be so).  This module will not work as expected
on displays that do not honor these escape sequences, such as cmd.exe,
4nt.exe, and command.com under either Windows NT or Windows 2000.  They
may just be ignored, or they may display as an ESC character followed by
some apparent garbage.

Jean Delvare provided the following table of different common terminal
emulators and their support for the various attributes and others have
helped me flesh it out:

              clear    bold     faint   under    blink   reverse  conceal
 ------------------------------------------------------------------------
 xterm         yes      yes      no      yes      yes      yes      yes
 linux         yes      yes      yes    bold      yes      yes      no
 rxvt          yes      yes      no      yes  bold/black   yes      no
 dtterm        yes      yes      yes     yes    reverse    yes      yes
 teraterm      yes    reverse    no      yes    rev/red    yes      no
 aixterm      kinda   normal     no      yes      no       yes      yes
 PuTTY         yes     color     no      yes      no       yes      no
 Windows       yes      no       no      no       no       yes      no
 Cygwin SSH    yes      yes      no     color    color    color     yes
 Terminal.app  yes      yes      no      yes      yes      yes      yes

Windows is Windows telnet, Cygwin SSH is the OpenSSH implementation under
Cygwin on Windows NT, and Mac Terminal is the Terminal application in Mac
OS X.  Where the entry is other than yes or no, that emulator displays the
given attribute as something else instead.  Note that on an aixterm, clear
doesn't reset colors; you have to explicitly set the colors back to what
you want.  More entries in this table are welcome.

Support for code 3 (italic) is rare and therefore not mentioned in that
table.  It is not believed to be fully supported by any of the terminals
listed, although it's displayed as green in the Linux console, but it is
reportedly supported by urxvt.

Note that codes 6 (rapid blink) and 9 (strike-through) are specified in ANSI
X3.64 and ECMA-048 but are not commonly supported by most displays and
emulators and therefore aren't supported by this module.  ECMA-048 also
specifies a large number of other attributes, including a sequence of
attributes for font changes, Fraktur characters, double-underlining, framing,
circling, and overlining.  As none of these attributes are widely supported or
useful, they also aren't currently supported by this module.

Most modern X terminal emulators support 256 colors.  Known to not support
those colors are aterm, rxvt, Terminal.app, and TTY/VC.

For information on true color support in various terminal emulators, see
L<True Colour support|https://gist.github.com/XVilka/8346728>.

=head1 AUTHORS

Original idea (using constants) by Zenin, reimplemented using subs by Russ
Allbery <rra@cpan.org>, and then combined with the original idea by
Russ with input from Zenin.  256-color support is based on work by Kurt
Starsinic.  Russ Allbery now maintains this module.

PUSHCOLOR, POPCOLOR, and LOCALCOLOR were contributed by openmethods.com
voice solutions.

=head1 COPYRIGHT AND LICENSE

Copyright 1996-1998, 2000-2002, 2005-2006, 2008-2018, 2020 Russ Allbery
<rra@cpan.org>

Copyright 1996 Zenin

Copyright 2012 Kurt Starsinic <kstarsinic@gmail.com>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

The CPAN module L<Term::ExtendedColor> provides a different and more
comprehensive interface for 256-color emulators that may be more
convenient.  The CPAN module L<Win32::Console::ANSI> provides ANSI color
(and other escape sequence) support in the Win32 Console environment.
The CPAN module L<Term::Chrome> provides a different interface using
objects and operator overloading.

ECMA-048 is available on-line (at least at the time of this writing) at
L<https://www.ecma-international.org/publications/standards/Ecma-048.htm>.

ISO 6429 is available from ISO for a charge; the author of this module
does not own a copy of it.  Since the source material for ISO 6429 was
ECMA-048 and the latter is available for free, there seems little reason
to obtain the ISO standard.

The 256-color control sequences are documented at
L<https://invisible-island.net/xterm/ctlseqs/ctlseqs.html> (search for
256-color).

Information about true color support in various terminal emulators and test
programs you can run to check the true color support in your terminal emulator
are available at L<https://gist.github.com/XVilka/8346728>.

L<CLICOLORS|https://bixense.com/clicolors/> and
L<NO_COLOR|https://no-color.org/> are useful standards to be aware of, and
ideally follow, for any application using color.  Term::ANSIColor complies
with the latter.

The current version of this module is always available from its web site
at L<https://www.eyrie.org/~eagle/software/ansicolor/>.  It is also part
of the Perl core distribution as of 5.6.0.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
