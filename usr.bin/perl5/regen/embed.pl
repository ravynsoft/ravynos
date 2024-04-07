#!/usr/bin/perl -w
#
# Regenerate (overwriting only if changed):
#
#    embed.h
#    embedvar.h
#    proto.h
#
# from information stored in
#
#    embed.fnc
#    intrpvar.h
#    perlvars.h
#    regen/opcodes
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.

require 5.004;  # keep this compatible, an old perl is all we may have before
                # we build the new one

use strict;

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
    require './regen/embed_lib.pl';
}

my $unflagged_pointers;
my @az = ('a'..'z');

#
# See database of global and static function prototypes in embed.fnc
# This is used to generate prototype headers under various configurations,
# export symbols lists for different platforms, and macros to provide an
# implicit interpreter context argument.
#

my $error_count = 0;
sub die_at_end ($) { # Keeps going for now, but makes sure the regen doesn't
                     # succeed.
    warn shift;
    $error_count++;
}

sub full_name ($$) { # Returns the function name with potentially the
                     # prefixes 'S_' or 'Perl_'
    my ($func, $flags) = @_;

    return "Perl_$func" if $flags =~ /[ps]/;
    return "S_$func" if $flags =~ /[SIi]/;
    return $func;
}

sub open_print_header {
    my ($file, $quote) = @_;

    return open_new($file, '>',
                    { file => $file, style => '*', by => 'regen/embed.pl',
                      from => [
                               'embed.fnc',
                               'intrpvar.h',
                               'perlvars.h',
                               'regen/opcodes',
                               'regen/embed.pl',
                               'regen/embed_lib.pl',
                               'regen/HeaderParser.pm',
                           ],
                      final => "\nEdit those files and run 'make regen_headers' to effect changes.\n",
                      copyright => [1993 .. 2022],
                      quote => $quote });
}


sub open_buf_out {
    $_[0] //= "";
    open my $fh,">", \$_[0]
        or die "Failed to open buffer: $!";
    return $fh;
}

# generate proto.h
sub generate_proto_h {
    my ($all)= @_;
    my $pr = open_buf_out(my $proto_buffer);
    my $ret;

    foreach (@$all) {
        if ($_->{type} ne "content") {
            print $pr "$_->{line}";
            next;
        }
        my $embed= $_->{embed}
            or next;

        my $level= $_->{level};
        my $ind= $level ? " " : "";
        $ind .= "  " x ($level-1) if $level>1;
        my $inner_ind= $ind ? "  " : " ";

        my ($flags,$retval,$plain_func,$args) = @{$embed}{qw(flags return_type name args)};
        if ($flags =~ / ( [^AabCDdEefFGhIiMmNnOoPpRrSsTUuWXx;] ) /x) {
            die_at_end "flag $1 is not legal (for function $plain_func)";
        }
        my @nonnull;
        my $args_assert_line = ( $flags !~ /[Gm]/ );
        my $has_depth = ( $flags =~ /W/ );
        my $has_context = ( $flags !~ /T/ );
        my $never_returns = ( $flags =~ /r/ );
        my $binarycompat = ( $flags =~ /b/ );
        my $commented_out = ( $flags =~ /m/ );
        my $is_malloc = ( $flags =~ /a/ );
        my $can_ignore = ( $flags !~ /R/ ) && ( $flags !~ /P/ ) && !$is_malloc;
        my @names_of_nn;
        my $func;

        if (! $can_ignore && $retval eq 'void') {
            warn "It is nonsensical to require the return value of a void function ($plain_func) to be checked";
        }

        die_at_end "$plain_func: S and p flags are mutually exclusive"
                                            if $flags =~ /S/ && $flags =~ /p/;
        die_at_end "$plain_func: m and $1 flags are mutually exclusive"
                                        if $flags =~ /m/ && $flags =~ /([pS])/;

        die_at_end "$plain_func: u flag only usable with m" if $flags =~ /u/
                                                            && $flags !~ /m/;

        my ($static_flag, @extra_static_flags)= $flags =~/([SsIi])/g;

        if (@extra_static_flags) {
            my $flags_str = join ", ", $static_flag, @extra_static_flags;
            $flags_str =~ s/, (\w)\z/ and $1/;
            die_at_end "$plain_func: flags $flags_str are mutually exclusive\n";
        }

        my $static_inline = 0;
        if ($static_flag) {
            my $type;
            if ($never_returns) {
                $type = {
                    'S' => 'PERL_STATIC_NO_RET',
                    's' => 'PERL_STATIC_NO_RET',
                    'i' => 'PERL_STATIC_INLINE_NO_RET',
                    'I' => 'PERL_STATIC_FORCE_INLINE_NO_RET'
                }->{$static_flag};
            }
            else {
                $type = {
                    'S' => 'STATIC',
                    's' => 'STATIC',
                    'i' => 'PERL_STATIC_INLINE',
                    'I' => 'PERL_STATIC_FORCE_INLINE'
                }->{$static_flag};
            }
            $retval = "$type $retval";
            die_at_end "Don't declare static function '$plain_func' pure" if $flags =~ /P/;
            $static_inline = $type =~ /^PERL_STATIC(?:_FORCE)?_INLINE/;
        }
        else {
            if ($never_returns) {
                $retval = "PERL_CALLCONV_NO_RET $retval";
            }
            else {
                $retval = "PERL_CALLCONV $retval";
            }
        }

        $func = full_name($plain_func, $flags);

        die_at_end "For '$plain_func', M flag requires p flag"
                                            if $flags =~ /M/ && $flags !~ /p/;
        my $C_required_flags = '[pIimbs]';
        die_at_end
            "For '$plain_func', C flag requires one of $C_required_flags] flags"
                                                if $flags =~ /C/
                                                && ($flags !~ /$C_required_flags/

                                                   # Notwithstanding the
                                                   # above, if the name won't
                                                   # clash with a user name,
                                                   # it's ok.
                                                && $plain_func !~ /^[Pp]erl/);

        die_at_end "For '$plain_func', X flag requires one of [Iip] flags"
                                            if $flags =~ /X/ && $flags !~ /[Iip]/;
        die_at_end "For '$plain_func', X and m flags are mutually exclusive"
                                            if $flags =~ /X/ && $flags =~ /m/;
        die_at_end "For '$plain_func', [Ii] with [ACX] requires p flag"
                        if $flags =~ /[Ii]/ && $flags =~ /[ACX]/ && $flags !~ /p/;
        die_at_end "For '$plain_func', b and m flags are mutually exclusive"
                 . " (try M flag)" if $flags =~ /b/ && $flags =~ /m/;
        die_at_end "For '$plain_func', b flag without M flag requires D flag"
                            if $flags =~ /b/ && $flags !~ /M/ && $flags !~ /D/;
        die_at_end "For '$plain_func', I and i flags are mutually exclusive"
                                            if $flags =~ /I/ && $flags =~ /i/;

        $ret = "";
        $ret .= "$retval\n";
        $ret .= "$func(";
        if ( $has_context ) {
            $ret .= @$args ? "pTHX_ " : "pTHX";
        }
        if (@$args) {
            die_at_end "n flag is contradicted by having arguments"
                                                                if $flags =~ /n/;
            my $n;
            for my $arg ( @$args ) {
                ++$n;
                if ($arg =~ / ^ " (.+) " $ /x) {    # Handle literal string
                    my $name = $1;

                    # Make the string a legal C identifier; 'p' is arbitrary,
                    # and is because C reserves leading underscores
                    $name =~ s/^\W/p/a;
                    $name =~ s/\W/_/ag;

                    $arg = "const char * const $name";
                    die_at_end 'm flag required for "literal" argument'
                                                            unless $flags =~ /m/;
                }
                elsif (   $args_assert_line
                       && $arg =~ /\*/
                       && $arg !~ /\b(NN|NULLOK)\b/ )
                {
                    warn "$func: $arg needs NN or NULLOK\n";
                    ++$unflagged_pointers;
                }
                my $nn = ( $arg =~ s/\s*\bNN\b\s+// );
                push( @nonnull, $n ) if $nn;
                my $nz = ( $arg =~ s/\s*\bNZ\b\s+// );

                my $nullok = ( $arg =~ s/\s*\bNULLOK\b\s+// ); # strip NULLOK with no effect

                # Make sure each arg has at least a type and a var name.
                # An arg of "int" is valid C, but want it to be "int foo".
                my $temp_arg = $arg;
                $temp_arg =~ s/\*//g;
                $temp_arg =~ s/\s*\bstruct\b\s*/ /g;
                if ( ($temp_arg ne "...")
                     && ($temp_arg !~ /\w+\s+(\w+)(?:\[\d+\])?\s*$/) ) {
                    die_at_end "$func: $arg ($n) doesn't have a name\n";
                }
                if (defined $1 && ($nn||$nz) && !($commented_out && !$binarycompat)) {
                    push @names_of_nn, $1;
                }
            }
            $ret .= join ", ", @$args;
        }
        else {
            $ret .= "void" if !$has_context;
        }
        $ret .= " comma_pDEPTH" if $has_depth;
        $ret .= ")";
        my @attrs;
        if ( $flags =~ /r/ ) {
            push @attrs, "__attribute__noreturn__";
        }
        if ( $flags =~ /D/ ) {
            push @attrs, "__attribute__deprecated__";
        }
        if ( $is_malloc ) {
            push @attrs, "__attribute__malloc__";
        }
        if ( !$can_ignore ) {
            push @attrs, "__attribute__warn_unused_result__";
        }
        if ( $flags =~ /P/ ) {
            push @attrs, "__attribute__pure__";
        }
        if ( $flags =~ /I/ ) {
            push @attrs, "__attribute__always_inline__";
        }
        # roughly the inverse of the rules used in makedef.pl
        if ( $flags !~ /[AbCeIimSX]/ ) {
            push @attrs, '__attribute__visibility__("hidden")'
        }
        if( $flags =~ /f/ ) {
            my $prefix  = $has_context ? 'pTHX_' : '';
            my ($argc, $pat);
            if (!defined $args->[1]) {
                use Data::Dumper;
                die Dumper($_);
            }
            if ($args->[-1] eq '...') {
                $argc   = scalar @$args;
                $pat    = $argc - 1;
                $argc   = $prefix . $argc;
            }
            else {
                # don't check args, and guess which arg is the pattern
                # (one of 'fmt', 'pat', 'f'),
                $argc = 0;
                my @fmts = grep $args->[$_] =~ /\b(f|pat|fmt)$/, 0..$#$args;
                if (@fmts != 1) {
                    die "embed.pl: '$plain_func': can't determine pattern arg\n";
                }
                $pat = $fmts[0] + 1;
            }
            my $macro   = grep($_ == $pat, @nonnull)
                                ? '__attribute__format__'
                                : '__attribute__format__null_ok__';
            if ($plain_func =~ /strftime/) {
                push @attrs, sprintf "%s(__strftime__,%s1,0)", $macro, $prefix;
            }
            else {
                push @attrs, sprintf "%s(__printf__,%s%d,%s)", $macro,
                                    $prefix, $pat, $argc;
            }
        }
        elsif ((grep { $_ eq '...' } @$args) && $flags !~ /F/) {
            die_at_end "$plain_func: Function with '...' arguments must have"
                     . " f or F flag";
        }
        if ( @attrs ) {
            $ret .= "\n";
            $ret .= join( "\n", map { (" " x 8) . $_ } @attrs );
        }
        $ret .= ";";
        $ret = "/* $ret */" if $commented_out;

        if ($args_assert_line || @names_of_nn) {
            $ret .= "\n#${ind}define PERL_ARGS_ASSERT_\U$plain_func\E";
            if (@names_of_nn) {
                $ret .= " \\\n";
                my $def = " " x 8;
                foreach my $ix (0..$#names_of_nn) {
                    $def .= "assert($names_of_nn[$ix])";
                    if ($ix == $#names_of_nn) {
                        $def .= "\n";
                    } elsif (length $def > 70) {
                        $ret .= $def . "; \\\n";
                        $def = " " x 8;
                    } else {
                        $def .= "; ";
                    }
                }
                $ret .= $def;
            }
        }
        $ret .= "\n";

        $ret = "#${ind}ifndef PERL_NO_INLINE_FUNCTIONS\n$ret\n#${ind}endif"
            if $static_inline;
        $ret = "#${ind}ifndef NO_MATHOMS\n$ret\n#${ind}endif"
            if $binarycompat;

        $ret .= @attrs ? "\n\n" : "\n";

        print $pr $ret;
    }


    close $pr;

    my $clean= normalize_group_content($proto_buffer);

    my $fh = open_print_header("proto.h");
    print $fh <<~"EOF";
    START_EXTERN_C
    $clean
    #ifdef PERL_CORE
    #  include "pp_proto.h"
    #endif
    END_EXTERN_C
    EOF

    read_only_bottom_close_and_rename($fh) if ! $error_count;
}

{
    my $hp= HeaderParser->new();
    sub normalize_group_content {
        open my $in, "<", \$_[0]
            or die "Failed to open buffer: $!";
        $hp->parse_fh($in);
        my $ppc= sub {
            my ($self, $line_data)= @_;
            # re-align defines so that the definitions line up at the 48th col
            # as much as possible.
            if ($line_data->{sub_type} eq "#define") {
                $line_data->{line}=~s/^(\s*#\s*define\s+\S+?(?:\([^()]*\))?\s)(\s*)(\S+)/
                    sprintf "%-48s%s", $1, $3/e;
            }
        };
        my $clean= $hp->lines_as_str($hp->group_content(),$ppc);
        return $clean;
    }
}

sub normalize_and_print {
    my ($file, $buffer)= @_;
    my $fh = open_print_header($file);
    print $fh normalize_group_content($buffer);
    read_only_bottom_close_and_rename($fh);
}


sub readvars {
    my ($file, $pre) = @_;
    my $hp= HeaderParser->new()->read_file($file);
    my %seen;
    foreach my $line_data (@{$hp->lines}) {
        #next unless $line_data->is_content;
        my $line= $line_data->line;
        if ($line=~m/^\s*PERLVARA?I?C?\(\s*$pre\s*,\s*(\w+)/){
            $seen{$1}++
                and
                die_at_end "duplicate symbol $1 while processing $file line "
                       . ($line_data->start_line_num) . "\n"
        }
    }
    my @keys= sort { lc($a) cmp lc($b) ||
                        $a  cmp    $b }
              keys %seen;
    return @keys;
}

sub add_indent {
    #my ($ret, $add, $width)= @_;
    my $width= $_[2] || 48;
    $_[0] .= " " x ($width-length($_[0])) if length($_[0])<$width;
    $_[0] .= " " unless $_[0]=~/\s\z/;
    if (defined $_[1]) {
        $_[0] .= $_[1];
    }
    return $_[0];
}

sub indent_define {
    my ($from, $to, $indent, $width) = @_;
    $indent = '' unless defined $indent;
    my $ret= "#${indent}define $from";
    add_indent($ret,"$to\n",$width);
}

sub multon {
    my ($sym,$pre,$ptr,$ind) = @_;
    $ind//="";
    indent_define("PL_$sym", "($ptr$pre$sym)", $ind);
}

sub embed_h {
    my ($em, $guard, $funcs) = @_;

    my $lines;
    foreach (@$funcs) {
        if ($_->{type} ne "content") {
            $lines .= $_->{line};
            next;
        }
        my $level= $_->{level};
        my $embed= $_->{embed} or next;
        my ($flags,$retval,$func,$args) = @{$embed}{qw(flags return_type name args)};
        my $ret = "";
        my $ind= $level ? " " : "";
        $ind .= "  " x ($level-1) if $level>1;
        my $inner_ind= $ind ? "  " : " ";
        unless ($flags =~ /[omM]/) {
            my $argc = scalar @$args;
            if ($flags =~ /T/) {
                my $full_name = full_name($func, $flags);
                next if $full_name eq $func;    # Don't output a no-op.
                $ret = indent_define($func, $full_name, $ind);
            }
            else {
                my $use_va_list = $argc && $args->[-1] =~ /\.\.\./;

                if($use_va_list) {
                    # CPP has trouble with empty __VA_ARGS__ and comma joining,
                    # so we'll have to eat an extra params here.
                    if($argc < 2) {
                        die "Cannot use ... as the only parameter to a macro ($func)\n";
                    }
                    $argc -= 2;
                }

                my $paramlist   = join(",", @az[0..$argc-1],
                    $use_va_list ? ("...") : ());
                my $replacelist = join(",", @az[0..$argc-1],
                    $use_va_list ? ("__VA_ARGS__") : ());
                $ret = "#${ind}define $func($paramlist) ";
                add_indent($ret,full_name($func, $flags) . "(aTHX");
                $ret .= "_ " if $replacelist;
                $ret .= $replacelist;
                if ($flags =~ /W/) {
                    if ($replacelist) {
                        $ret .= " comma_aDEPTH";
                    } else {
                        die "Can't use W without other args (currently)";
                    }
                }
                $ret .= ")\n";
                if($use_va_list) {
                    # Make them available to !MULTIPLICITY or PERL_CORE
                    $ret = "#${ind}if !defined(MULTIPLICITY) || defined(PERL_CORE)\n" .
                           $ret .
                           "#${ind}endif\n";
                }
            }
            $ret = "#${ind}ifndef NO_MATHOMS\n$ret#${ind}endif\n" if $flags =~ /b/;
        }
        $lines .= $ret;
    }
    # remove empty blocks
    1 while $lines =~ s/^#\s*if.*\n#\s*endif.*\n//mg
         or $lines =~ s/^(#\s*if)\s+(.*)\n#else.*\n/$1 !($2)\n/mg;
    if ($guard) {
        print $em "$guard /* guard */\n";
        $lines=~s/^#(\s*)/"#".(length($1)?"  ":" ").$1/mge;
    }
    print $em $lines;
    print $em "#endif\n" if $guard;
}

sub generate_embed_h {
    my ($all, $api, $ext, $core)= @_;

    my $em= open_buf_out(my $embed_buffer);

    print $em <<~'END';
    /* (Doing namespace management portably in C is really gross.) */

    /* By defining PERL_NO_SHORT_NAMES (not done by default) the short forms
     * (like warn instead of Perl_warn) for the API are not defined.
     * Not defining the short forms is a good thing for cleaner embedding.
     * BEWARE that a bunch of macros don't have long names, so either must be
     * added or don't use them if you define this symbol */

    #ifndef PERL_NO_SHORT_NAMES

    /* Hide global symbols */

    END

    embed_h($em, '', $api);
    embed_h($em, '#if defined(PERL_CORE) || defined(PERL_EXT)', $ext);
    embed_h($em, '#if defined(PERL_CORE)', $core);

    print $em <<~'END';

    #endif      /* #ifndef PERL_NO_SHORT_NAMES */

    #if !defined(PERL_CORE)
    /* Compatibility stubs.  Compile extensions with -DPERL_NOCOMPAT to
     * disable them.
     */
    #  define sv_setptrobj(rv,ptr,name) sv_setref_iv(rv,name,PTR2IV(ptr))
    #  define sv_setptrref(rv,ptr)              sv_setref_iv(rv,NULL,PTR2IV(ptr))
    #endif

    #if !defined(PERL_CORE) && !defined(PERL_NOCOMPAT)

    /* Compatibility for various misnamed functions.  All functions
       in the API that begin with "perl_" (not "Perl_") take an explicit
       interpreter context pointer.
       The following are not like that, but since they had a "perl_"
       prefix in previous versions, we provide compatibility macros.
     */
    #  define perl_atexit(a,b)          call_atexit(a,b)
    END

    foreach (@$all) {
        my $embed= $_->{embed} or next;
        my ($flags, $retval, $func, $args) = @{$embed}{qw(flags return_type name args)};
        next unless $flags =~ /O/;

        my $alist = join ",", @az[0..$#$args];
        my $ret = "#  define perl_$func($alist) ";
        print $em add_indent($ret,"$func($alist)\n");
    }

    my @nocontext;
    {
        my (%has_va, %has_nocontext);
        foreach (@$all) {
            my $embed= $_->{embed}
                or next;
            ++$has_va{$embed->{name}} if @{$embed->{args}} and $embed->{args}[-1] =~ /\.\.\./;
            ++$has_nocontext{$1} if $embed->{name} =~ /(.*)_nocontext/;
        }

        @nocontext = sort grep {
            $has_nocontext{$_}
                && !/printf/ # Not clear to me why these are skipped but they are.
        } keys %has_va;
    }

    print $em <<~'END';

    /* varargs functions can't be handled with CPP macros. :-(
       This provides a set of compatibility functions that don't take
       an extra argument but grab the context pointer using the macro
       dTHX.
     */
    #if defined(MULTIPLICITY) && !defined(PERL_NO_SHORT_NAMES)
    END

    foreach (@nocontext) {
        print $em indent_define($_, "Perl_${_}_nocontext", "  ");
    }

    print $em <<~'END';
    #endif

    #endif /* !defined(PERL_CORE) && !defined(PERL_NOCOMPAT) */

    #if !defined(MULTIPLICITY)
    /* undefined symbols, point them back at the usual ones */
    END

    foreach (@nocontext) {
        print $em indent_define("Perl_${_}_nocontext", "Perl_$_", "  ");
    }

    print $em "#endif\n";
    close $em;

    normalize_and_print('embed.h',$embed_buffer)
        unless $error_count;
}

sub generate_embedvar_h {
    my $em = open_buf_out(my $embedvar_buffer);

    print $em "#if defined(MULTIPLICITY)\n",
              indent_define("vTHX","aTHX"," ");


    my @intrp = readvars 'intrpvar.h','I';
    #my @globvar = readvars 'perlvars.h','G';


    for my $sym (@intrp) {
        my $ind = " ";
        if ($sym eq 'sawampersand') {
            print $em "# if !defined(PL_sawampersand)\n";
            $ind = "   ";
        }
        my $line = multon($sym, 'I', 'vTHX->', $ind);
        print $em $line;
        if ($sym eq 'sawampersand') {
            print $em "# endif /* !defined(PL_sawampersand) */\n";
        }
    }

    print $em "#endif       /* MULTIPLICITY */\n";
    close $em;

    normalize_and_print('embedvar.h',$embedvar_buffer)
        unless $error_count;
}

sub update_headers {
    my ($all, $api, $ext, $core) = setup_embed(); # see regen/embed_lib.pl
    generate_proto_h($all);
    die_at_end "$unflagged_pointers pointer arguments to clean up\n" if $unflagged_pointers;
    generate_embed_h($all, $api, $ext, $core);
    generate_embedvar_h();
    die "$error_count errors found" if $error_count;
}

update_headers() unless caller;

# ex: set ts=8 sts=4 sw=4 noet:
