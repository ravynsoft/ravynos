#!/usr/local/bin/perl

use strict;
use warnings;

use Config;
use File::Basename qw(basename dirname);
use Cwd;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.
# Wanted:  $archlibexp

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir dirname($0);
my $file = basename($0, '.PL');
$file .= '.com' if $^O eq 'VMS';

open OUT, '>', $file or die "Can't create $file: $!";

print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

print OUT <<"!GROK!THIS!";
$Config{startperl}
    eval 'exec $Config{perlpath} -S \$0 \${1+"\$@"}'
	if 0; # ^ Run only under a shell
!GROK!THIS!

# In the following, perl variables are not expanded during extraction.

print OUT <<'!NO!SUBS!';

BEGIN { pop @INC if $INC[-1] eq '.' }

use strict;

use Config;
use File::Path qw(mkpath);
use Getopt::Std;

# Make sure read permissions for all are set:
if (defined umask && (umask() & 0444)) {
    umask (umask() & ~0444);
}

getopts('Dd:rlhaQe');
use vars qw($opt_D $opt_d $opt_r $opt_l $opt_h $opt_a $opt_Q $opt_e);
die "-r and -a options are mutually exclusive\n" if ($opt_r and $opt_a);
my @inc_dirs = inc_dirs() if $opt_a;

my $Exit = 0;

my $Dest_dir = $opt_d || $Config{installsitearch};
die "Destination directory $Dest_dir doesn't exist or isn't a directory\n"
    unless -d $Dest_dir;

my @isatype = qw(
	char	uchar	u_char
	short	ushort	u_short
	int	uint	u_int
	long	ulong	u_long
	FILE	key_t	caddr_t
	float	double	size_t
);

my %isatype;
@isatype{@isatype} = (1) x @isatype;
my $inif = 0;
my %Is_converted;
my %bad_file = ();

@ARGV = ('-') unless @ARGV;

build_preamble_if_necessary();

sub reindent($) {
    my($text) = shift;
    $text =~ s/\n/\n    /g;
    $text =~ s/        /\t/g;
    $text;
}

my ($t, $tab, %curargs, $new, $eval_index, $dir, $name, $args, $outfile);
my ($incl, $incl_type, $incl_quote, $next);
while (defined (my $file = next_file())) {
    if (-l $file and -d $file) {
        link_if_possible($file) if ($opt_l);
        next;
    }

    # Recover from header files with unbalanced cpp directives
    $t = '';
    $tab = 0;

    # $eval_index goes into '#line' directives, to help locate syntax errors:
    $eval_index = 1;

    if ($file eq '-') {
	open(IN, "-");
	open(OUT, ">-");
    } else {
	($outfile = $file) =~ s/\.h$/.ph/ || next;
	print "$file -> $outfile\n" unless $opt_Q;
	if ($file =~ m|^(.*)/|) {
	    $dir = $1;
	    mkpath "$Dest_dir/$dir";
	}

	if ($opt_a) { # automagic mode:  locate header file in @inc_dirs
	    foreach (@inc_dirs) {
		chdir $_;
		last if -f $file;
	    }
	}

	open(IN, "<", "$file") || (($Exit = 1),(warn "Can't open $file: $!\n"),next);
	open(OUT, ">", "$Dest_dir/$outfile") || die "Can't create $outfile: $!\n";
    }

    print OUT
        "require '_h2ph_pre.ph';\n\n",
        "no warnings qw(redefine misc);\n\n";

    while (defined (local $_ = next_line($file))) {
	if (s/^\s*\#\s*//) {
	    if (s/^define\s+(\w+)//) {
		$name = $1;
		$new = '';
		s/\s+$//;
		s/\(\w+\s*\(\*\)\s*\(\w*\)\)\s*(-?\d+)/$1/; # (int (*)(foo_t))0
		if (s/^\(([\w,\s]*)\)//) {
		    $args = $1;
		    my $proto = '() ';
		    if ($args ne '') {
			$proto = '';
			foreach my $arg (split(/,\s*/,$args)) {
			    $arg =~ s/^\s*([^\s].*[^\s])\s*$/$1/;
			    $curargs{$arg} = 1;
			}
			$args =~ s/\b(\w)/\$$1/g;
			$args = "my($args) = \@_;\n$t    ";
		    }
		    s/^\s+//;
		    expr();
		    $new =~ s/(["\\])/\\$1/g;       #"]);
		    EMIT($proto);
		} else {
		    s/^\s+//;
		    expr();

		    $new = 1 if $new eq '';

		    # Shunt around such directives as '#define FOO FOO':
		    next if $new =~ /^\s*&\Q$name\E\s*\z/;

		    $new = reindent($new);
		    $args = reindent($args);
		    $new =~ s/(['\\])/\\$1/g;        #']);

	    	    print OUT $t, 'eval ';
		    if ($opt_h) {
			print OUT "\"\\n#line $eval_index $outfile\\n\" . ";
			$eval_index++;
		    }
		    print OUT "'sub $name () {$new;}' unless defined(&$name);\n";
		}
	    } elsif (/^(include|import|include_next)\s*([<\"])(.*)[>\"]/) {
                $incl_type = $1;
                $incl_quote = $2;
                $incl = $3;
                if (($incl_type eq 'include_next') ||
                    ($opt_e && exists($bad_file{$incl}))) {
                    $incl =~ s/\.h$/.ph/;
		print OUT ($t,
			   "eval {\n");
                $tab += 4;
                $t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
                    print OUT ($t, "my(\@REM);\n");
                    if ($incl_type eq 'include_next') {
		print OUT ($t,
			   "my(\%INCD) = map { \$INC{\$_} => 1 } ",
			           "(grep { \$_ eq \"$incl\" } ",
                                   "keys(\%INC));\n");
		print OUT ($t,
			           "\@REM = map { \"\$_/$incl\" } ",
			   "(grep { not exists(\$INCD{\"\$_/$incl\"})",
			           " and -f \"\$_/$incl\" } \@INC);\n");
                    } else {
                        print OUT ($t,
                                   "\@REM = map { \"\$_/$incl\" } ",
                                   "(grep {-r \"\$_/$incl\" } \@INC);\n");
                    }
		print OUT ($t,
			   "require \"\$REM[0]\" if \@REM;\n");
                $tab -= 4;
                $t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
                print OUT ($t,
			   "};\n");
		print OUT ($t,
			   "warn(\$\@) if \$\@;\n");
                } else {
                    $incl =~ s/\.h$/.ph/;
                    # copy the prefix in the quote syntax (#include "x.h") case
                    if ($incl !~ m|/| && $incl_quote eq q{"} && $file =~ m|^(.*)/|) {
                        $incl = "$1/$incl";
                    }
		    print OUT $t,"require '$incl';\n";
                }
	    } elsif (/^ifdef\s+(\w+)/) {
		print OUT $t,"if(defined(&$1)) {\n";
		$tab += 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
	    } elsif (/^ifndef\s+(\w+)/) {
		print OUT $t,"unless(defined(&$1)) {\n";
		$tab += 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
	    } elsif (s/^if\s+//) {
		$new = '';
		$inif = 1;
		expr();
		$inif = 0;
		print OUT $t,"if($new) {\n";
		$tab += 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
	    } elsif (s/^elif\s+//) {
		$new = '';
		$inif = 1;
		expr();
		$inif = 0;
		$tab -= 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
		print OUT $t,"}\n elsif($new) {\n";
		$tab += 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
	    } elsif (/^else/) {
		$tab -= 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
		print OUT $t,"} else {\n";
		$tab += 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
	    } elsif (/^endif/) {
		$tab -= 4;
		$t = "\t" x ($tab / 8) . ' ' x ($tab % 8);
		print OUT $t,"}\n";
	    } elsif(/^undef\s+(\w+)/) {
		print OUT $t, "undef(&$1) if defined(&$1);\n";
	    } elsif(/^error\s+(".*")/) {
		print OUT $t, "die($1);\n";
	    } elsif(/^error\s+(.*)/) {
		print OUT $t, "die(\"", quotemeta($1), "\");\n";
	    } elsif(/^warning\s+(.*)/) {
		print OUT $t, "warn(\"", quotemeta($1), "\");\n";
	    } elsif(/^ident\s+(.*)/) {
		print OUT $t, "# $1\n";
	    }
	} elsif (/^\s*(typedef\s*)?enum\s*(\s+[a-zA-Z_]\w*\s*)?/) { # { for vi
	    until(/\{[^}]*\}.*;/ || /;/) {
		last unless defined ($next = next_line($file));
		chomp $next;
		# drop "#define FOO FOO" in enums
		$next =~ s/^\s*#\s*define\s+(\w+)\s+\1\s*$//;
		# #defines in enums (aliases)
		$next =~ s/^\s*#\s*define\s+(\w+)\s+(\w+)\s*$/$1 = $2,/;
		$_ .= $next;
		print OUT "# $next\n" if $opt_D;
	    }
	    s/#\s*if.*?#\s*endif//g; # drop #ifdefs
	    s@/\*.*?\*/@@g;
	    s/\s+/ /g;
	    next unless /^\s?(typedef\s?)?enum\s?([a-zA-Z_]\w*)?\s?\{(.*)\}\s?([a-zA-Z_]\w*)?\s?;/;
	    (my $enum_subs = $3) =~ s/\s//g;
	    my @enum_subs = split(/,/, $enum_subs);
	    my $enum_val = -1;
	    foreach my $enum (@enum_subs) {
		my ($enum_name, $enum_value) = $enum =~ /^([a-zA-Z_]\w*)(=.+)?$/;
		$enum_name or next;
		$enum_value =~ s/^=//;
		$enum_val = (length($enum_value) ? $enum_value : $enum_val + 1);
		if ($opt_h) {
		    print OUT ($t,
			       "eval(\"\\n#line $eval_index $outfile\\n",
			       "sub $enum_name () \{ $enum_val; \}\") ",
			       "unless defined(\&$enum_name);\n");
		    ++ $eval_index;
		} else {
		    print OUT ($t,
			       "eval(\"sub $enum_name () \{ $enum_val; \}\") ",
			       "unless defined(\&$enum_name);\n");
		}
	    }
	} elsif (/^(?:__extension__\s+)?(?:extern|static)\s+(?:__)?inline(?:__)?\s+/
	    and !/;\s*$/ and !/{\s*}\s*$/)
	{ # { for vi
	    # This is a hack to parse the inline functions in the glibc headers.
	    # Warning: massive kludge ahead. We suppose inline functions
	    # are mainly constructed like macros.
	    while (1) {
		last unless defined ($next = next_line($file));
		chomp $next;
		undef $_, last if $next =~ /__THROW\s*;/
			       or $next =~ /^(__extension__|extern|static)\b/;
		$_ .= " $next";
		print OUT "# $next\n" if $opt_D;
		last if $next =~ /^}|^{.*}\s*$/;
	    }
	    next if not defined; # because it's only a prototype
	    s/\b(__extension__|extern|static|(?:__)?inline(?:__)?)\b//g;
	    # violently drop #ifdefs
	    s/#\s*if.*?#\s*endif//g
		and print OUT "# some #ifdef were dropped here -- fill in the blanks\n";
	    if (s/^(?:\w|\s|\*)*\s(\w+)\s*//) {
		$name = $1;
	    } else {
		warn "name not found"; next; # shouldn't occur...
	    }
	    my @args;
	    if (s/^\(([^()]*)\)\s*(\w+\s*)*//) {
		for my $arg (split /,/, $1) {
		    if ($arg =~ /(\w+)\s*$/) {
			$curargs{$1} = 1;
			push @args, $1;
		    }
		}
	    }
	    $args = (
		@args
		? "my(" . (join ',', map "\$$_", @args) . ") = \@_;\n$t    "
		: ""
	    );
	    my $proto = @args ? '' : '() ';
	    $new = '';
	    s/\breturn\b//g; # "return" doesn't occur in macros usually...
	    expr();
	    # try to find and perlify local C variables
	    our @local_variables = (); # needs to be a our(): (?{...}) bug workaround
	    {
		use re "eval";
		my $typelist = join '|', keys %isatype;
		$new =~ s['
		  (?:(?:__)?const(?:__)?\s+)?
		  (?:(?:un)?signed\s+)?
		  (?:long\s+)?
		  (?:$typelist)\s+
		  (\w+)
		  (?{ push @local_variables, $1 })
		  ']
		 [my \$$1]gx;
		$new =~ s['
		  (?:(?:__)?const(?:__)?\s+)?
		  (?:(?:un)?signed\s+)?
		  (?:long\s+)?
		  (?:$typelist)\s+
		  ' \s+ &(\w+) \s* ;
		  (?{ push @local_variables, $1 })
		  ]
		 [my \$$1;]gx;
	     }
	    $new =~ s/&$_\b/\$$_/g for @local_variables;
	    $new =~ s/(["\\])/\\$1/g;       #"]);
	    # now that's almost like a macro (we hope)
	    EMIT($proto);
	}
    }
    $Is_converted{$file} = 1;
    if ($opt_e && exists($bad_file{$file})) {
        unlink($Dest_dir . '/' . $outfile);
        $next = '';
    } else {
        print OUT "1;\n";
	queue_includes_from($file) if $opt_a;
    }
}

if ($opt_e && (scalar(keys %bad_file) > 0)) {
    warn "Was unable to convert the following files:\n";
    warn "\t" . join("\n\t",sort(keys %bad_file)) . "\n";
}

exit $Exit;

sub EMIT {
    my $proto = shift;

    $new = reindent($new);
    $args = reindent($args);
    if ($t ne '') {
    $new =~ s/(['\\])/\\$1/g;   #']);
    if ($opt_h) {
        print OUT $t,
                    "eval \"\\n#line $eval_index $outfile\\n\" . 'sub $name $proto\{\n$t    ${args}eval q($new);\n$t}' unless defined(\&$name);\n";
                    $eval_index++;
    } else {
        print OUT $t,
                    "eval 'sub $name $proto\{\n$t    ${args}eval q($new);\n$t}' unless defined(\&$name);\n";
    }
    } else {
              print OUT "unless(defined(\&$name)) {\n    sub $name $proto\{\n\t${args}eval q($new);\n    }\n}\n";
    }
    %curargs = ();
    return;
}

sub expr {
    if (/\b__asm__\b/) {	# freak out
	$new = '"(assembly code)"';
	return
    }
    my $joined_args;
    if(keys(%curargs)) {
	$joined_args = join('|', keys(%curargs));
    }
    while ($_ ne '') {
	s/^\&\&// && do { $new .= " &&"; next;}; # handle && operator
	s/^\&([\(a-z\)]+)/$1/i;	# hack for things that take the address of
	s/^(\s+)//		&& do {$new .= ' '; next;};
	s/^0X([0-9A-F]+)[UL]*//i
	    && do {my $hex = $1;
		   $hex =~ s/^0+//;
		   if (length $hex > 8 && !$Config{use64bitint}) {
		       # Croak if nv_preserves_uv_bits < 64 ?
		       $new .=         hex(substr($hex, -8)) +
			       2**32 * hex(substr($hex,  0, -8));
		       # The above will produce "erroneous" code
		       # if the hex constant was e.g. inside UINT64_C
		       # macro, but then again, h2ph is an approximation.
		   } else {
		       $new .= lc("0x$hex");
		   }
		   next;};
	s/^(-?\d+\.\d+E[-+]?\d+)[FL]?//i	&& do {$new .= $1; next;};
	s/^(\d+)\s*[LU]*//i	&& do {$new .= $1; next;};
	s/^("(\\"|[^"])*")//	&& do {$new .= $1; next;};
	s/^'((\\"|[^"])*)'//	&& do {
	    if ($curargs{$1}) {
		$new .= "ord('\$$1')";
	    } else {
		$new .= "ord('$1')";
	    }
	    next;
	};
        # replace "sizeof(foo)" with "{foo}"
        # also, remove * (C dereference operator) to avoid perl syntax
        # problems.  Where the %sizeof array comes from is anyone's
        # guess (c2ph?), but this at least avoids fatal syntax errors.
        # Behavior is undefined if sizeof() delimiters are unbalanced.
        # This code was modified to able to handle constructs like this:
        #   sizeof(*(p)), which appear in the HP-UX 10.01 header files.
        s/^sizeof\s*\(// && do {
            $new .= '$sizeof';
            my $lvl = 1;  # already saw one open paren
            # tack { on the front, and skip it in the loop
            $_ = "{" . "$_";
            my $index = 1;
            # find balanced closing paren
            while ($index <= length($_) && $lvl > 0) {
                $lvl++ if substr($_, $index, 1) eq "(";
                $lvl-- if substr($_, $index, 1) eq ")";
                $index++;
            }
            # tack } on the end, replacing )
            substr($_, $index - 1, 1) = "}";
            # remove pesky * operators within the sizeof argument
            substr($_, 0, $index - 1) =~ s/\*//g;
            next;
        };
	# Eliminate typedefs
	/\(([\w\s]+)[\*\s]*\)\s*[\w\(]/ && do {
	    my $doit = 1;
	    foreach (split /\s+/, $1) {  # Make sure all the words are types,
	        unless($isatype{$_} or $_ eq 'struct' or $_ eq 'union'){
		    $doit = 0;
		    last;
		}
	    }
	    if( $doit ){
		s/\([\w\s]+[\*\s]*\)// && next;      # then eliminate them.
	    }
	};
	# struct/union member, including arrays:
	s/^([_A-Z]\w*(\[[^\]]+\])?((\.|->)[_A-Z]\w*(\[[^\]]+\])?)+)//i && do {
	    my $id = $1;
	    $id =~ s/(\.|(->))([^\.\-]*)/->\{$3\}/g;
	    $id =~ s/\b([^\$])($joined_args)/$1\$$2/g if length($joined_args);
	    while($id =~ /\[\s*([^\$\&\d\]]+)\]/) {
		my($index) = $1;
		$index =~ s/\s//g;
		if(exists($curargs{$index})) {
		    $index = "\$$index";
		} else {
		    $index = "&$index";
		}
		$id =~ s/\[\s*([^\$\&\d\]]+)\]/[$index]/;
	    }
	    $new .= " (\$$id)";
	};
	s/^([_a-zA-Z]\w*)//	&& do {
	    my $id = $1;
	    if ($id eq 'struct' || $id eq 'union') {
		s/^\s+(\w+)//;
		$id .= ' ' . $1;
		$isatype{$id} = 1;
	    } elsif ($id =~ /^((un)?signed)|(long)|(short)$/) {
		while (s/^\s+(\w+)//) { $id .= ' ' . $1; }
		$isatype{$id} = 1;
	    }
	    if ($curargs{$id}) {
		$new .= "\$$id";
		$new .= '->' if /^[\[\{]/;
	    } elsif ($id eq 'defined') {
		$new .= 'defined';
	    } elsif (/^\s*\(/) {
		s/^\s*\((\w),/("$1",/ if $id =~ /^_IO[WR]*$/i;	# cheat
		$new .= " &$id";
	    } elsif ($isatype{$id}) {
		if ($new =~ /\{\s*$/) {
		    $new .= "'$id'";
		} elsif ($new =~ /\(\s*$/ && /^[\s*]*\)/) {
		    $new =~ s/\(\s*$//;
		    s/^[\s*]*\)//;
		} else {
		    $new .= q(').$id.q(');
		}
	    } else {
		if ($inif) {
		    if ($new =~ /defined\s*$/) {
			$new .= '(&' . $id . ')';
		    } elsif ($new =~ /defined\s*\($/) {
			$new .= '&' . $id;
		    } else {
			$new .= '(defined(&' . $id . ') ? &' . $id . ' : undef)';
		    }
		} elsif (/^\[/) {
		    $new .= " \$$id";
		} else {
		    $new .= ' &' . $id;
		}
	    }
	    next;
	};
	s/^(.)// && do { if ($1 ne '#') { $new .= $1; } next;};
    }
}


sub next_line
{
    my $file = shift;
    my ($in, $out);
    my $pre_sub_tri_graphs = 1;

    READ: while (not eof IN) {
        $in  .= <IN>;
        chomp $in;
        next unless length $in;

        while (length $in) {
            if ($pre_sub_tri_graphs) {
                # Preprocess all tri-graphs
                # including things stuck in quoted string constants.
                $in =~ s/\?\?=/#/g;                         # | ??=|  #|
                $in =~ s/\?\?\!/|/g;                        # | ??!|  ||
                $in =~ s/\?\?'/^/g;                         # | ??'|  ^|
                $in =~ s/\?\?\(/[/g;                        # | ??(|  [|
                $in =~ s/\?\?\)/]/g;                        # | ??)|  ]|
                $in =~ s/\?\?\-/~/g;                        # | ??-|  ~|
                $in =~ s/\?\?\//\\/g;                       # | ??/|  \|
                $in =~ s/\?\?</{/g;                         # | ??<|  {|
                $in =~ s/\?\?>/}/g;                         # | ??>|  }|
            }
	    if ($in =~ /^\#ifdef __LANGUAGE_PASCAL__/) {
		# Tru64 disassembler.h evilness: mixed C and Pascal.
		while (<IN>) {
		    last if /^\#endif/;
		}
		$in = "";
		next READ;
	    }
	    if ($in =~ /^extern inline / && # Inlined assembler.
		$^O eq 'linux' && $file =~ m!(?:^|/)asm/[^/]+\.h$!) {
		while (<IN>) {
		    last if /^}/;
		}
		$in = "";
		next READ;
	    }
            if ($in =~ s/\\$//) {                           # \-newline
                $out    .= ' ';
                next READ;
            } elsif ($in =~ s/^([^"'\\\/]+)//) {            # Passthrough
                $out    .= $1;
            } elsif ($in =~ s/^(\\.)//) {                   # \...
                $out    .= $1;
            } elsif ($in =~ /^'/) {                         # '...
                if ($in =~ s/^('(\\.|[^'\\])*')//) {
                    $out    .= $1;
                } else {
                    next READ;
                }
            } elsif ($in =~ /^"/) {                         # "...
                if ($in =~ s/^("(\\.|[^"\\])*")//) {
                    $out    .= $1;
                } else {
                    next READ;
                }
            } elsif ($in =~ s/^\/\/.*//) {                  # //...
                # fall through
            } elsif ($in =~ m/^\/\*/) {                     # /*...
                # C comment removal adapted from perlfaq6:
                if ($in =~ s/^\/\*[^*]*\*+([^\/*][^*]*\*+)*\///) {
                    $out    .= ' ';
                } else {                                    # Incomplete /* */
                    next READ;
                }
            } elsif ($in =~ s/^(\/)//) {                    # /...
                $out    .= $1;
            } elsif ($in =~ s/^([^\'\"\\\/]+)//) {
                $out    .= $1;
            } elsif ($^O eq 'linux' &&
                     $file =~ m!(?:^|/)linux/byteorder/pdp_endian\.h$! &&
                     $in   =~ s!\'T KNOW!!) {
                $out    =~ s!I DON$!I_DO_NOT_KNOW!;
            } else {
                if ($opt_e) {
                    warn "Cannot parse $file:\n$in\n";
                    $bad_file{$file} = 1;
                    $in = '';
                    $out = undef;
                    last READ;
                } else {
		die "Cannot parse:\n$in\n";
                }
            }
        }

        last READ if $out =~ /\S/;
    }

    return $out;
}


# Handle recursive subdirectories without getting a grotesquely big stack.
# Could this be implemented using File::Find?
sub next_file
{
    my $file;

    while (@ARGV) {
        $file = shift @ARGV;

        if ($file eq '-' or -f $file or -l $file) {
            return $file;
        } elsif (-d $file) {
            if ($opt_r) {
                expand_glob($file);
            } else {
                print STDERR "Skipping directory '$file'\n";
            }
        } elsif ($opt_a) {
            return $file;
        } else {
            print STDERR "Skipping '$file':  not a file or directory\n";
        }
    }

    return undef;
}


# Put all the files in $directory into @ARGV for processing.
sub expand_glob
{
    my ($directory)  = @_;

    $directory =~ s:/$::;

    opendir DIR, $directory;
        foreach (readdir DIR) {
            next if ($_ eq '.' or $_ eq '..');

            # expand_glob() is going to be called until $ARGV[0] isn't a
            # directory; so push directories, and unshift everything else.
            if (-d "$directory/$_") { push    @ARGV, "$directory/$_" }
            else                    { unshift @ARGV, "$directory/$_" }
        }
    closedir DIR;
}


# Given $file, a symbolic link to a directory in the C include directory,
# make an equivalent symbolic link in $Dest_dir, if we can figure out how.
# Otherwise, just duplicate the file or directory.
sub link_if_possible
{
    my ($dirlink)  = @_;
    my $target  = eval 'readlink($dirlink)';

    if ($target =~ m:^\.\./: or $target =~ m:^/:) {
        # The target of a parent or absolute link could leave the $Dest_dir
        # hierarchy, so let's put all of the contents of $dirlink (actually,
        # the contents of $target) into @ARGV; as a side effect down the
        # line, $dirlink will get created as an _actual_ directory.
        expand_glob($dirlink);
    } else {
        if (-l "$Dest_dir/$dirlink") {
            unlink "$Dest_dir/$dirlink" or
                print STDERR "Could not remove link $Dest_dir/$dirlink:  $!\n";
        }

        if (eval 'symlink($target, "$Dest_dir/$dirlink")') {
            print "Linking $target -> $Dest_dir/$dirlink\n";

            # Make sure that the link _links_ to something:
            if (! -e "$Dest_dir/$target") {
                mkpath("$Dest_dir/$target", 0755) or
                    print STDERR "Could not create $Dest_dir/$target/\n";
            }
        } else {
            print STDERR "Could not symlink $target -> $Dest_dir/$dirlink:  $!\n";
        }
    }
}


# Push all #included files in $file onto our stack, except for STDIN
# and files we've already processed.
sub queue_includes_from
{
    my ($file)    = @_;
    my $line;

    return if ($file eq "-");

    open HEADER, "<", $file or return;
        while (defined($line = <HEADER>)) {
            while (/\\$/) { # Handle continuation lines
                chop $line;
                $line .= <HEADER>;
            }

            if ($line =~ /^#\s*include\s+([<"])(.*?)[>"]/) {
                my ($delimiter, $new_file) = ($1, $2);
                # copy the prefix in the quote syntax (#include "x.h") case
                if ($delimiter eq q{"} && $file =~ m|^(.*)/|) {
                    $new_file = "$1/$new_file";
                }
                push(@ARGV, $new_file) unless $Is_converted{$new_file};
            }
        }
    close HEADER;
}


# Determine include directories; $Config{usrinc} should be enough for (all
# non-GCC?) C compilers, but gcc uses additional include directories.
sub inc_dirs
{
    my $from_gcc   = `LC_ALL=C $Config{cc} -v -E - < /dev/null 2>&1 | awk '/^#include/, /^End of search list/' | grep '^ '`;
    length($from_gcc) ? (split(' ', $from_gcc), $Config{usrinc}) : ($Config{usrinc});
}


# Create "_h2ph_pre.ph", if it doesn't exist or was built by a different
# version of h2ph.
sub build_preamble_if_necessary
{
    # Increment $VERSION every time this function is modified:
    my $VERSION     = 4;
    my $preamble    = "$Dest_dir/_h2ph_pre.ph";

    # Can we skip building the preamble file?
    if (-r $preamble) {
        # Extract version number from first line of preamble:
        open  PREAMBLE, "<", $preamble or die "Cannot open $preamble:  $!";
            my $line = <PREAMBLE>;
            $line =~ /(\b\d+\b)/;
        close PREAMBLE            or die "Cannot close $preamble:  $!";

        # Don't build preamble if a compatible preamble exists:
        return if $1 == $VERSION;
    }

    my (%define) = _extract_cc_defines();

    open  PREAMBLE, ">", $preamble or die "Cannot open $preamble:  $!";
	print PREAMBLE "# This file was created by h2ph version $VERSION\n";
        # Prevent non-portable hex constants from warning.
        #
        # We still produce an overflow warning if we can't represent
        # a hex constant as an integer.
        print PREAMBLE "no warnings qw(portable);\n";

	foreach (sort keys %define) {
	    if ($opt_D) {
		print PREAMBLE "# $_=$define{$_}\n";
	    }
	    if ($define{$_} =~ /^\((.*)\)$/) {
		# parenthesized value:  d=(v)
		$define{$_} = $1;
	    }
	    if (/^(\w+)\((\w)\)$/) {
		my($macro, $arg) = ($1, $2);
		my $def = $define{$_};
		$def =~ s/$arg/\$\{$arg\}/g;
		print PREAMBLE <<DEFINE;
unless (defined &$macro) { sub $macro(\$) { my (\$$arg) = \@_; \"$def\" } }

DEFINE
	    } elsif
		($define{$_} =~ /^([+-]?(\d+)?\.\d+([eE][+-]?\d+)?)[FL]?$/) {
		# float:
		print PREAMBLE
		    "unless (defined &$_) { sub $_() { $1 } }\n\n";
	    } elsif ($define{$_} =~ /^([+-]?\d+)U?L{0,2}$/i) {
		# integer:
		print PREAMBLE
		    "unless (defined &$_) { sub $_() { $1 } }\n\n";
            } elsif ($define{$_} =~ /^([+-]?0x[\da-f]+)U?L{0,2}$/i) {
                # hex integer
                # Special cased, since perl warns on hex integers
                # that can't be represented in a UV.
                #
                # This way we get the warning at time of use, so the user
                # only gets the warning if they happen to use this
                # platform-specific definition.
                my $code = $1;
                $code = "hex('$code')" if length $code > 10;
                print PREAMBLE
                    "unless (defined &$_) { sub $_() { $code } }\n\n";
	    } elsif ($define{$_} =~ /^\w+$/) {
		my $def = $define{$_};
		if ($isatype{$def}) {
		  print PREAMBLE
		    "unless (defined &$_) { sub $_() { \"$def\" } }\n\n";
		} else {
		  print PREAMBLE
		    "unless (defined &$_) { sub $_() { &$def } }\n\n";
	        }
	    } else {
		print PREAMBLE
		    "unless (defined &$_) { sub $_() { \"",
		    quotemeta($define{$_}), "\" } }\n\n";
	    }
	}
	print PREAMBLE "\n1;\n";  # avoid 'did not return a true value' when empty
    close PREAMBLE               or die "Cannot close $preamble:  $!";
}


# %Config contains information on macros that are pre-defined by the
# system's compiler.  We need this information to make the .ph files
# function with perl as the .h files do with cc.
sub _extract_cc_defines
{
    my %define;
    my $allsymbols  = join " ",
	@Config{'ccsymbols', 'cppsymbols', 'cppccsymbols'};

    # Split compiler pre-definitions into 'key=value' pairs:
    while ($allsymbols =~ /([^\s]+)=((\\\s|[^\s])+)/g) {
	$define{$1} = $2;
	if ($opt_D) {
	    print STDERR "$_:  $1 -> $2\n";
	}
    }

    return %define;
}


1;

##############################################################################
__END__

=head1 NAME

h2ph - convert .h C header files to .ph Perl header files

=head1 SYNOPSIS

B<h2ph [-d destination directory] [-r | -a] [-l] [-h] [-e] [-D] [-Q]
[headerfiles]>

=head1 DESCRIPTION

I<h2ph>
converts any C header files specified to the corresponding Perl header file
format.
It is most easily run while in /usr/include:

	cd /usr/include; h2ph * sys/*

or

	cd /usr/include; h2ph * sys/* arpa/* netinet/*

or

	cd /usr/include; h2ph -r -l .

The output files are placed in the hierarchy rooted at Perl's
architecture dependent library directory.  You can specify a different
hierarchy with a B<-d> switch.

If run with no arguments, filters standard input to standard output.

=head1 OPTIONS

=over 4

=item -d destination_dir

Put the resulting B<.ph> files beneath B<destination_dir>, instead of
beneath the default Perl library location (C<$Config{'installsitearch'}>).

=item -r

Run recursively; if any of B<headerfiles> are directories, then run I<h2ph>
on all files in those directories (and their subdirectories, etc.).  B<-r>
and B<-a> are mutually exclusive.

=item -a

Run automagically; convert B<headerfiles>, as well as any B<.h> files
which they include.  This option will search for B<.h> files in all
directories which your C compiler ordinarily uses.  B<-a> and B<-r> are
mutually exclusive.

=item -l

Symbolic links will be replicated in the destination directory.  If B<-l>
is not specified, then links are skipped over.

=item -h

Put 'hints' in the .ph files which will help in locating problems with
I<h2ph>.  In those cases when you B<require> a B<.ph> file containing syntax
errors, instead of the cryptic

	[ some error condition ] at (eval mmm) line nnn

you will see the slightly more helpful

	[ some error condition ] at filename.ph line nnn

However, the B<.ph> files almost double in size when built using B<-h>.

=item -e

If an error is encountered during conversion, output file will be removed and
a warning emitted instead of terminating the conversion immediately.

=item -D

Include the code from the B<.h> file as a comment in the B<.ph> file.
This is primarily used for debugging I<h2ph>.

=item -Q

'Quiet' mode; don't print out the names of the files being converted.

=back

=head1 ENVIRONMENT

No environment variables are used.

=head1 FILES

 /usr/include/*.h
 /usr/include/sys/*.h

etc.

=head1 AUTHOR

Larry Wall

=head1 SEE ALSO

perl(1)

=head1 DIAGNOSTICS

The usual warnings if it can't read or write the files involved.

=head1 BUGS

Doesn't construct the %sizeof array for you.

It doesn't handle all C constructs, but it does attempt to isolate
definitions inside evals so that you can get at the definitions
that it can translate.

It's only intended as a rough tool.
You may need to dicker with the files produced.

You have to run this program by hand; it's not run as part of the Perl
installation.

Doesn't handle complicated expressions built piecemeal, a la:

    enum {
	FIRST_VALUE,
	SECOND_VALUE,
    #ifdef ABC
	THIRD_VALUE
    #endif
    };

Doesn't necessarily locate all of your C compiler's internally-defined
symbols.

=cut

!NO!SUBS!

close OUT or die "Can't close $file: $!";
chmod 0755, $file or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir $origdir;
