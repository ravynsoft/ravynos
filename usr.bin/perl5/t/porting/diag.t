#!/usr/bin/perl

BEGIN {
  if (-f './TestInit.pm') {
    @INC = '.';
  } elsif (-f '../TestInit.pm') {
    @INC = '..';
  }
}
use TestInit qw(T); # T is chdir to the top level

use warnings;
use strict;
use Config;
use Data::Dumper;
require './t/test.pl';

if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

plan('no_plan');

# --make-exceptions-list outputs the list of strings that don't have
# perldiag.pod entries to STDERR without TAP formatting, so they can
# easily be put in the __DATA__ section of this file.  This was done
# initially so as to not create new test failures upon the initial
# creation of this test file.  You probably shouldn't do it again.
# Just add the documentation instead.
my $make_exceptions_list = ($ARGV[0]||'') eq '--make-exceptions-list'
  and shift;

require './regen/embed_lib.pl';

# Look for functions that look like they could be diagnostic ones.
my @functions;
foreach (@{(setup_embed())[0]}) {
  my $embed= $_->{embed}
    or next;
  next unless $embed->{name}  =~ /warn|(?<!ov)err|(\b|_)die|croak|deprecate/i;
  # Skip some known exceptions
  next if $embed->{name} =~ /croak_kw_unless_class/;
  # The flag p means that this function may have a 'Perl_' prefix
  # The flag S means that this function may have a 'S_' prefix
  push @functions, $embed->{name};
  push @functions, 'Perl_' . $embed->{name} if $embed->{flags} =~ /p/;
  push @functions, 'S_' . $embed->{name} if $embed->{flags} =~ /S/;
};
push @functions, 'Perl_mess';
@functions = sort { length($b) <=> length($a) || $a cmp $b } @functions;
push @functions, 'PERL_DIAG_(?<wrapper>\w+)';

my $regcomp_fail_re = '\b(?:(?:Simple_)?v)?FAIL[2-4]?(?:utf8f)?\b';
my $regcomp_re =
   "(?<routine>ckWARN(?:\\d+)?reg\\w*|vWARN\\d+|$regcomp_fail_re)";
my $function_re = join '|', @functions;
my $source_msg_re =
   "(?<routine>\\bDIE\\b|$function_re)";
my $text_re = '"(?<text>(?:\\\\"|[^"]|"\s*[A-Z_]+\s*")*)"';
my $source_msg_call_re = qr/$source_msg_re(?:_nocontext)? \s*
    \( (?: \s* Perl_form \( )? (?:aTHX_)? \s*
    (?:packWARN\d*\((?<category>.*?)\),)? \s*
    (?:(?<category>WARN_DEPRECATED__\w+)\s*,(?:\s*(?<version_string>"[^"]+")\s*,)?)? \s*
    $text_re /x;
my $bad_version_re = qr{BADVERSION\([^"]*$text_re};
   $regcomp_fail_re = qr/$regcomp_fail_re\([^"]*$text_re/;
my $regcomp_call_re = qr/$regcomp_re.*?$text_re/;

my %entries;
my $data_start_line= 0;
# Get the ignores that are compiled into this file
my $reading_categorical_exceptions;
# reset the DATA point to the top of the file, read until we find __DATA__
# so that $. is "correct" for our purposes.
seek DATA, 0, 0;
while (<DATA>) {
    /^__DATA__/ and last;
}
while (<DATA>) {
  chomp;
  next if /^\s*#/ and !/\S/;
  $entries{$_}{todo} = 1;
  $entries{$_}{todo_line}= $data_start_line + $.;
  $reading_categorical_exceptions and $entries{$_}{cattodo}=1;
  /__CATEGORIES__/ and ++$reading_categorical_exceptions;
}

my $pod = "pod/perldiag.pod";
my $cur_entry;
open my $diagfh, "<", $pod
  or die "Can't open $pod: $!";

my $category_re = qr/ [a-z0-9_:]+?/;    # Note: requires an initial space
my $severity_re = qr/ . (?: \| . )* /x; # A severity is a single char, but can
                                        # be of the form 'S|P|W'
my @same_descr;
my $depth = 0;
while (<$diagfh>) {
  if (m/^=over/) {
    $depth++;
    next;
  }
  if (m/^=back/) {
    $depth--;
    next;
  }

  # Stuff deeper than main level is ignored
  next if $depth != 1;

  if (m/^=item (.*)/) {
    $cur_entry = $1;

    # Allow multi-line headers
    while (<$diagfh>) {
      if (/^\s*$/) {
        last;
      }

      $cur_entry =~ s/ ?\z/ $_/;
    }

    $cur_entry =~ s/\n/ /gs; # Fix multi-line headers if they have \n's
    $cur_entry =~ s/\s+\z//;
    $cur_entry =~ s/E<lt>/</g;
    $cur_entry =~ s/E<gt>/>/g;
    $cur_entry =~ s,E<sol>,/,g;
    $cur_entry =~ s/[BCIFS](?:<<< (.*?) >>>|<< (.*?) >>|<(.*?)>)/$+/g;

    if (exists $entries{$cur_entry} &&  $entries{$cur_entry}{todo}
                                    && !$entries{$cur_entry}{cattodo}) {
        my $data_line= $entries{$cur_entry}{todo_line};
        TODO: {
            local $::TODO = "Remove the TODO entry \"$cur_entry\" from DATA "
                          . "at $0 line $data_line as it is already in $pod near line $.";
            ok($cur_entry);
        }
    }
    # Make sure to init this here, so an actual entry in perldiag
    # overwrites one in DATA.
    # diag("adding '$cur_entry'");
    $entries{$cur_entry}{todo} = 0;
    $entries{$cur_entry}{line_number} = $.;
  }

  next if ! defined $cur_entry;

  if (! $entries{$cur_entry}{severity}) {
    if (/^ \( ( $severity_re )

        # Can have multiple categories separated by commas
        ( $category_re (?: , $category_re)* )? \) /x)
    {
      $entries{$cur_entry}{severity} = $1;
      $entries{$cur_entry}{category} =
        $2 && join ", ", sort split " ", $2 =~ y/,//dr;

      # Record it also for other messages sharing the same description
      @$_{qw<severity category>} =
        @{$entries{$cur_entry}}{qw<severity category>}
       for @same_descr;
    }
    elsif (! $entries{$cur_entry}{first_line} && $_ =~ /\S/) {

      # Keep track of first line of text if doesn't contain a severity, so
      # that can later examine it to determine if that is ok or not
      $entries{$cur_entry}{first_line} = $_;
    }
    if (/\S/) {
      @same_descr = ();
    }
    else {
      push @same_descr, $entries{$cur_entry};
    }
  }
}

if ($depth != 0) {
    diag ("Unbalance =over/=back.  Fix before proceeding; over - back = " . $depth);
    exit(1);
}

foreach my $cur_entry ( keys %entries) {
    next if $entries{$cur_entry}{todo}; # If in this file, won't have a severity
    if (! exists $entries{$cur_entry}{severity}

            # If there is no first line, it was two =items in a row, so the
            # second one is the one with text, not this one.
        && exists $entries{$cur_entry}{first_line}

            # If the first line refers to another message, no need for severity
        && $entries{$cur_entry}{first_line} !~ /^See/)
    {
        fail($cur_entry);
        diag(
            "   $pod entry at line $entries{$cur_entry}{line_number}\n"
          . "       \"$cur_entry\"\n"
          . "   is missing a severity and/or category"
        );
    }
}

# List from perlguts.pod "Formatted Printing of IVs, UVs, and NVs"
# Convert from internal formats to ones that the readers will be familiar
# with, while removing any format modifiers, such as precision, the
# presence of which would just confuse the pod's explanation.
# Note that the 'S' formats get converted into \"%s\" as they inject
# double quotes.
my %specialformats = (IVdf => 'd',
		      UVuf => 'd',
		      UVof => 'o',
		      UVxf => 'x',
		      UVXf => 'X',
		      NVef => 'f',
		      NVff => 'f',
		      NVgf => 'f',
		      HEKf256=>'s',
		      HEKf256_QUOTEDPREFIX => 'S',
		      HEKf => 's',
		      HEKf_QUOTEDPREFIX => 'S',
		      UTF8f=> 's',
		      UTF8f_QUOTEDPREFIX => 'S',
		      SVf256=>'s',
		      SVf32=> 's',
		      SVf  => 's',
		      SVf_QUOTEDPREFIX  => 'S',
                      PVf_QUOTEDPREFIX  => 'S',
		      PNf  => 's',
                      HvNAMEf => 's',
                      HvNAMEf_QUOTEDPREFIX => 'S',
                  );

my $format_modifiers = qr/ [#0\ +-]*              # optional flags
			  (?: [1-9][0-9]* | \* )? # optional field width
			  (?: \. \d* )?           # optional precision
			  (?: h|l )?              # optional length modifier
			/x;

my $specialformats =
 join '|', sort { length($b) <=> length($a) || $a cmp $b } keys %specialformats;
my $specialformats_re = qr/%$format_modifiers"\s*($specialformats)(\s*(?:"|\z))?/;

# We skip the bodies of most XS functions, but not within these files
my @include_xs_files = (
  "builtin.c",
);

if (@ARGV) {
  check_file($_) for @ARGV;
  exit;
}
open my $fh, '<', 'MANIFEST' or die "Can't open MANIFEST: $!";
while (my $file = <$fh>) {
    chomp $file;
    $file =~ s/\s+.*//;
    next unless $file =~ /\.(?:c|cpp|h|xs|y)\z/ or $file =~ /^perly\./;
    # OS/2 extensions have never been migrated to ext/, hence the special case:
    next if $file =~ m!\A(?:ext|dist|cpan|lib|t|os2/OS2)/!
            && $file !~ m!\Aext/DynaLoader/!;
    check_file($file);
}
close $fh or die $!;

# Standardize messages with variants into the form that appears
# in perldiag.pod -- useful for things without a diag_listed_as annotation
sub standardize {
  my ($name) = @_;

  if    ( $name =~ m/^(Invalid strict version format) \([^\)]*\)/ ) {
    $name = "$1 (\%s)";
  }
  elsif ( $name =~ m/^(Invalid version format) \([^\)]*\)/ ) {
    $name = "$1 (\%s)";
  }
  elsif ($name =~ m/^(panic|Usage): /) {
    $name = "$1: \%s";
  }
  else {
    $name =~ s/ (*plb:^Function\ ")
                (\w+)
                (*pla:"\ not\ implemented\ in\ this\ version\ of\ perl\.)
              /%s/gx;
  }

  return $name;
}

sub check_file {
  my ($codefn) = @_;

  print "# Checking $codefn\n";

  open my $codefh, "<", $codefn
    or die "Can't open $codefn: $!";

  my $listed_as;
  my $listed_as_line;
  my $sub = 'top of file';
  while (<$codefh>) {
    chomp;
    my $first_line = $.;
    # Getting too much here isn't a problem; we only use this to skip
    # errors inside of XS modules, which should get documented in the
    # docs for the module.
    if (m<^[^#\s]> and $_ !~ m/^[{}]*$/) {
      $sub = $_;
    }
    next if $sub =~ m/^XS/ and !grep { $_ eq $codefn } @include_xs_files;
    if (m</\*\s*diag_listed_as: (.*?)\s*\*/>) {
      $listed_as = $1;
      $listed_as_line = $.+1;
    }
    elsif (m</\*\s*diag_listed_as: (.*?)\s*\z>) {
      my $new_listed_as = $1;
      while (<$codefh>) {
        if (m<\*/>) {
          $new_listed_as .= $` =~ s/^\s*/ /r =~ s/\s+\z//r;
          $listed_as_line = $.+1;
          $listed_as= $new_listed_as;
          last;
        }
        else {
          $new_listed_as .= s/^\s*/ /r =~ s/\s+\z//r;
        }
      }
    }
    next if /^#/;

    my $multiline = 0;
    # Loop to accumulate the message text all on one line.
    if (m/(?!^)\b(?:$source_msg_re(?:_nocontext)?|$regcomp_re)\s*\((?<tail>(?:[^()]+|\([^()]+\))+\))?/
        and !$+{tail}
    ) {
      while (not m/\);\s*$/) {
        my $nextline = <$codefh>;
        # Means we fell off the end of the file.  Not terribly surprising;
        # this code tries to merge a lot of things that aren't regular C
        # code (preprocessor stuff, long comments).  That's OK; we don't
        # need those anyway.
        last if not defined $nextline;
        chomp $nextline;
        $nextline =~ s/^\s+//;
        $_ =~ s/\\$//;
        # Note that we only want to do this where *both* are true.
        if ($_ =~ m/"\s*$/ and $nextline =~ m/^"/) {
          $_ =~ s/"\s*$//;
          $nextline =~ s/^"//;
        }
        $_ .= $nextline;
        ++$multiline;
      }
    }
    # This should happen *after* unwrapping, or we don't reformat the things
    # in later lines.

    s/$specialformats_re/"%$specialformats{$1}" .  (defined $2 ? '' : '"')/ge;
    s/\%S/\\"%s\\"/g; # convert an %S into a quoted %s.

    # Remove any remaining format modifiers, but not in %%
    s/ (?<!%) % $format_modifiers ( [dioxXucsfeEgGp] ) /%$1/xg;

    # The %"foo" thing needs to happen *before* this regex.
    # diag(">$_<");
    # DIE is just return Perl_die
    my ($name, $category, $routine, $wrapper);
    if (/\b$source_msg_call_re/) {
      my $version_string;
      ($name, $category, $routine, $wrapper, $version_string) =
        ($+{'text'}, $+{'category'}, $+{'routine'}, $+{'wrapper'}, $+{'version_string'});
      if ($wrapper) {
        $category = $wrapper if $wrapper=~/WARN/;
        $routine = "Perl_warner" if $wrapper=~/WARN/;
        $routine = "yyerror" if $wrapper=~/DIE/;
      }
      if ($routine=~/^deprecate/) {
        $name .= " is deprecated";
        if ($version_string) {
            like($version_string, qr/"5\.\d+"/,
                "version string is of the correct form at $codefn line $first_line");
        }
      }
      # diag(Dumper(\%+,{category=>$category, routine=>$routine, name=>$name}));
      # Sometimes the regexp will pick up too much for the category
      # e.g., WARN_UNINITIALIZED), PL_warn_uninit_sv ... up to the next )
      $category && $category =~ s/\).*//s;
      # Special-case yywarn
      /yywarn/ and $category = 'syntax';
      if (/win32_croak_not_implemented\(/) {
        $name .= " not implemented!"
      }
    }
    elsif (/$bad_version_re/) {
      ($name, $category) = ($+{'text'}, undef);
    }
    elsif (/$regcomp_fail_re/) {
      #  FAIL("foo") -> "foo in regex m/%s/"
      # vFAIL("foo") -> "foo in regex; marked by <-- HERE in m/%s/"
      ($name, $category) = ($+{'text'}, undef);
      $name .=
        " in regex" . ("; marked by <-- HERE in" x /vFAIL/) . " m/%s/";
    }
    elsif (/$regcomp_call_re/) {
      # vWARN/ckWARNreg("foo") -> "foo in regex; marked by <-- HERE in m/%s/
      ($name, $category, $routine) = ($+{'text'}, undef, $+{'routine'});
      $name .= " in regex; marked by <-- HERE in m/%s/";
      $category = 'WARN_REGEXP';
      if ($routine =~ /dep/) {
        $category .= ',WARN_DEPRECATED';
      }
    }
    else {
      next;
    }

    # Try to guess what the severity should be.  In the case of
    # Perl_ck_warner and other _ck_ functions, we can tell whether it is
    # a severe/default warning or no by the _d suffix.  In the case of
    # other warn functions we cannot tell, because Perl_warner may be pre-
    # ceded by if(ckWARN) or if(ckWARN_d).
    my $severity = !$routine                   ? '[PFX]'
                 :  $routine =~ /warn.*_d\z/   ? '[DS]'
                 :  $routine =~ /ck_warn/      ?  'W'
                 :  $routine =~ /warner/       ? '[WDS]'
                 :  $routine =~ /warn/         ?  'S'
                 :  $routine =~ /ckWARN.*dep/  ?  'D'
                 :  $routine =~ /ckWARN\d*reg_d/? 'S'
                 :  $routine =~ /ckWARN\d*reg/ ?  'W'
                 :  $routine =~ /vWARN\d/      ? '[WDS]'
                 :  $routine =~ /^deprecate/   ? '[WDS]'
                 :                             '[PFX]';
    my $categories;
    if (defined $category) {
      $category =~ s/__/::/g;
      $categories =
        join ", ",
              sort map {s/^WARN_//; lc $_} split /\s*[|,]\s*/, $category;
    }
    if ($listed_as) {
      $name = $listed_as;
      undef $listed_as;
    } else {
      # The form listed in perldiag ignores most sorts of fancy printf
      # formatting, or makes it more perlish.
      $name =~ s/%%/%/g;
      $name =~ s/%l[ud]/%d/g;
      $name =~ s/%\.(\d+|\*)s/\%s/g;
      $name =~ s/(?:%s){2,}/%s/g;
      $name =~ s/(\\")|("\s*[A-Z_]+\s*")/$1 ? '"' : '%s'/egg;
      $name =~ s/\\t/\t/g;
      $name =~ s/\\n/\n/g;
      $name =~ s/\s+$//;
      $name =~ s/(\\)\\/$1/g;
    }

    # Extra explanatory info on an already-listed error, doesn't
    # need its own listing.
    next if $name =~ m/^\t/;

    # Happens fairly often with PL_no_modify.
    next if $name eq '%s';

    # Special syntax for magic comment, allows ignoring the fact
    # that it isn't listed.  Only use in very special circumstances,
    # like this script failing to notice that the Perl_croak call is
    # inside an #if 0 block.
    next if $name eq 'SKIPME';

    next if $name=~/\[TESTING\]/; # ignore these as they are works in progress

    check_message(standardize($name),$codefn,$severity,$categories);
  }
}

sub check_message {
    my($name,$codefn,$severity,$categories,$partial) = @_;
    my $key = $name =~ y/\n/ /r;
    my $ret;

    # Try to reduce printf() formats to simplest forms
    # Really this should be matching %s, etc like diagnostics.pm does

    # Kill flags
    $key =~ s/%[#0\-+]/%/g;

    # Kill width
    $key =~ s/\%(\d+|\*)/%/g;

    # Kill precision
    $key =~ s/\%\.(\d+|\*)/%/g;

    if (exists $entries{$key} and
          # todo + cattodo means it is not found and it is not in the
          # regular todo list, either
          !$entries{$key}{todo} || !$entries{$key}{cattodo}) {
      $ret = 1;
      if ( $entries{$key}{seen}++ ) {
        # no need to repeat entries we've tested
      } elsif ($entries{$key}{todo}) {
        TODO: {
          no warnings 'once';
          local $::TODO = 'in DATA';
          # diag(Dumper($entries{$key}));
          # There is no listing, but it is in the list of exceptions.  TODO FAIL.
          fail($key);
          diag(
            "    Message '$name'\n    from $codefn line $. is not listed in $pod\n".
            "    (but it wasn't documented in 5.10 either, so marking it TODO)."
          );
        }
      } else {
        # We found an actual valid entry in perldiag.pod for this error.
        pass($key);

        return $ret
          if $entries{$key}{cattodo};

        # Now check the category and severity

        # Cache our severity qr thingies
        use feature 'state';
        state %qrs;
        my $qr = $qrs{$severity} ||= qr/$severity/;

        my $pod_line = $entries{$key}{line_number} // "";

        if ($pod_line) {
            $pod_line = ", at perldiag.pod line $pod_line";
        }

        like($entries{$key}{severity}, $qr,
          ($severity =~ /\[/
            ? "severity is one of $severity"
            : "severity is $severity") . "for '$name' at $codefn line $.$pod_line");

        is($entries{$key}{category}, $categories,
           ($categories ? "categories are [$categories]" : "no category")
             . " for '$name' at $codefn line $.$pod_line");
      }
    } elsif ($partial) {
      # noop
    } else {
      my $ok;
      if ($name =~ /\n/) {
        $ok = 1;
        check_message($_,$codefn,$severity,$categories,1) or $ok = 0, last
          for split /\n/, $name;
      }
      if ($ok) {
        # noop
      } elsif ($make_exceptions_list) {
        # We're making an updated version of the exception list, to
        # stick in the __DATA__ section.  I honestly can't think of
        # a situation where this is the right thing to do, but I'm
        # leaving it here, just in case one of my descendents thinks
        # it's a good idea.
        print STDERR "$key\n";
      } else {
        # No listing found, and no excuse either.
        # Find the correct place in perldiag.pod, and add a stanza beginning =item $name.
        fail($name);
        diag("    Message '$name'\n    from $codefn line $. is not listed in $pod");
      }
      # seen it, so only fail once for this message
      $entries{$name}{seen}++;
    }

    die if $name =~ /%$/;
    return $ret;
}

# The DATA section includes two types of entries, in two sets separated by a
# blank line.
#
# The first set are entries whose text we think is fully explanatory and don't
# need further elaboration in perldiag.  This set should consist of very few
# entries.
#
# The second set, after the blank line, consists of TODO entries.  (There are
# actually two subsets described below.)  This list should basically be just
# those entries that otherwise would have generated an error upon inauguration
# of this program, so we didn't have to go from "meh" to perfect all at once.
# The only valid reason we can think of to add to the list is for cases where
# this program is not smart enough to recognize the message is something that
# actually is in perldiag.  Otherwise, DO NOT ADD TO THIS LIST.  Instead,
# write an entry in pod/perldiag.pod for your new (warning|error).
#
# This second set has a subcategory, after the line marked __CATEGORIES__ .
# These are entries that are in perldiag but fail the severity/category test.

__DATA__
Function "%s" not implemented in this version of perl.
QUITing...
Recompile perl with -DDEBUGGING to use -D switch (did you mean -d ?)
System V IPC is not implemented on this machine
Terminating on signal SIG%s(%d)
This version of OS/2 does not support %s.%s
Usage: %s

Cannot apply "%s" in non-PerlIO perl
Can't find DLL name for the module `%s' by the handle %d, rc=%u=%x
Can't find string terminator %c%s%c anywhere before EOF
Can't get short module name from a handle
Can't load DLL `%s', possible problematic module `%s'
Can't locate %s:   %s
Can't pipe "%s": %s
Can't set type on DOS
Can't spawn: %s
Can't spawn "%s": %s
Can't %s script `%s' with ARGV[0] being `%s'
Can't %s "%s": %s
Can't %s `%s' with ARGV[0] being `%s' (looking for executables only, not found)
Can't use string ("%s"%s) as a subroutine ref while "strict refs" in use
Character(s) in '%c' format wrapped in %s
clear %s
Code missing after '/' in pack
Code missing after '/' in unpack
Could not find version 2.0 of winsock dll
'%c' outside of string in pack
Debug leaking scalars child failed%s with errno %d: %s
detach of a thread which could not start
detach on an already detached thread
detach on a thread with a waiter
Empty array reference given to mod2fname
endhostent not implemented!
endnetent not implemented!
endprotoent not implemented!
endservent not implemented!
Error loading module '%s': %s
Error reading "%s": %s
EVAL without pos change exceeded limit in regex
Filehandle opened only for %sput
Filehandle %s opened only for %sput
file_type not implemented on DOS
filter_del can only delete in reverse order (currently)
fork() not available
fork() not implemented!
YOU HAVEN'T DISABLED SET-ID SCRIPTS IN THE KERNEL YET! FIX YOUR KERNEL, PUT A C WRAPPER AROUND THIS SCRIPT, OR USE -u AND UNDUMP!
free %s
Free to wrong pool %p not %p
get %s %p %p %p
gethostent not implemented!
getnetbyaddr not implemented!
getnetbyname not implemented!
getnetent not implemented!
getprotoent not implemented!
getpwnam returned invalid UIC %o for user "%s"
getservent not implemented!
glob failed (can't start child: %s)
glob failed (child exited with status %d%s)
Got an error from DosAllocMem: %i
Goto undefined subroutine
Goto undefined subroutine &%s
Got signal %d
()-group starts with a count in %s
Illegal octal digit '%c' ignored
INSTALL_PREFIX too long: `%s'
Invalid argument to sv_cat_decode
Invalid range "%c-%c" in transliteration operator
Invalid separator character %c%c%c in PerlIO layer specification %s
ioctl implemented only on sockets
join with a thread with a waiter
Looks like we have no PM; will not load DLL %s without $ENV{PERL_ASIF_PM}
Malformed integer in [] in %s
Malformed %s
Malformed UTF-8 character (fatal)
Missing (suid) fd script name
More than one argument to open
More than one argument to open(,':%s')
No message queue
No %s allowed while running setgid
No %s allowed with (suid) fdscript
Not an XSUB reference
Not a reference given to mod2fname
Not array reference given to mod2fname
Operator or semicolon missing before %c%s
Out of memory during list extend
panic queryaddr
Parse error
POSIX syntax [%c %c] is reserved for future extensions in regex; marked by <-- HERE in m/%s/
ptr wrong %p != %p fl=%x nl=%p e=%p for %d
recursion detected in %s
Reversed %c= operator
%s: Can't parse EXE/DLL name: '%s'
%s(%f) failed
%s: Error stripping dirs from EXE/DLL/INSTALLDIR name
sethostent not implemented!
setnetent not implemented!
setprotoent not implemented!
set %s %p %p %p
setservent not implemented!
%s free() ignored (RMAGIC, PERL_CORE)
%s has too many errors.
SIG%s handler "%s" not defined.
%s in %s
Size magic not implemented
%s: name `%s' too long
%s not implemented!
%s number > %s non-portable
%srealloc() %signored
%s on %s %s
%s: %s
Starting Full Screen process with flag=%d, mytype=%d
Starting PM process with flag=%d, mytype=%d
switching effective gid is not implemented
switching effective uid is not implemented
This perl was compiled without taint support. Cowardly refusing to run with -t or -T flags
Too deeply nested ()-groups in %s
Too many args on %s line of "%s"
U0 mode on a byte string
unable to find VMSPIPE.COM for i/o piping
Unable to locate winsock library!
Unexpected program mode %d when morphing back from PM
Unrecognized character %s; marked by <-- HERE after %s<-- HERE near column %d
Unstable directory path, current directory changed unexpectedly
Unterminated compressed integer in unpack
utf8 "\x%X" does not map to Unicode
Value of logical "%s" too long. Truncating to %i bytes
waitpid: process %x is not a child of process %x
Wide character
Wide character in $/
Within []-length '*' not allowed in %s
Within []-length '%c' not allowed in %s
Wrong size of loadOrdinals array: expected %d, actual %d
Wrong syntax (suid) fd script name "%s"
'X' outside of string in %s
'X' outside of string in unpack

__CATEGORIES__

# This is a warning, but is currently followed immediately by a croak (toke.c)
Illegal character \%o (carriage return)

# Because uses WARN_MISSING as a synonym for WARN_UNINITIALIZED (sv.c)
Missing argument in %s

# This message can be both fatal and non-
False [] range "%s" in regex; marked by <-- HERE in m/%s/
