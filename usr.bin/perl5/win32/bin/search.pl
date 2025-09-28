#!/usr/local/bin/perl -w
'di';
'ig00';
##############################################################################
##
## search
##
## Jeffrey Friedl (jfriedl@omron.co.jp), Dec 1994.
## Copyright 19.... ah hell, just take it.
##
## BLURB:
## A combo of find and grep -- more or less do a 'grep' on a whole
## directory tree. Fast, with lots of options. Much more powerful than
## the simple "find ... | xargs grep ....". Has a full man page.
## Powerfully customizable.
##
## This file is big, but mostly comments and man page.
##
## See man page for usage info.
## Return value: 2=error, 1=nothing found, 0=something found.
##

$version = "950918.5";
##
## "950918.5";
##	Changed all 'sysread' to 'read' because Linux perl's don't seem
##	to like sysread()
##
## "941227.4";
##	Added -n, -u
##
## "941222.3"
##      Added -nice (due to Lionel Cons <Lionel.Cons@cern.ch>)
##	Removed any leading "./" from name.
##      Added default flags for ~/.search, including TTY, -nice, -list, etc.
##	Program name now has path removed when printed in diagnostics.
##	Added simple tilde-expansion to -dir arg.
##	Added -dskip, etc. Fixed -iregex bug.
##	Changed -dir to be additive, adding -ddir.
##	Now screen out devices, pipes, and sockets.
##	More tidying and lots of expanding of the man page
##
##
## "941217.2";
##	initial release.

$stripped=0;

&init;
if (exists $ENV{'HOME'}) {
    $rc_file = join('/', $ENV{'HOME'}, ".search");
}
else {
    $rc_file = "";
}

&check_args;

## Make sure we've got a regex.
## Don't need one if -find or -showrc was specified.
$!=2, die "expecting regex arguments.\n"
	if $FIND_ONLY == 0 && $showrc == 0 && @ARGV == 0;

&prepare_to_search($rc_file);

&import_program if !defined &dodir; ## BIG key to speed.

## do search while there are directories to be done.
&dodir(shift(@todo)) while @todo;

&clear_message if $VERBOSE && $STDERR_IS_TTY;
exit($retval);
###############################################################################

sub init
{
  ## initialize variables that might be reset by command-line args
  $DOREP=0; 		## set true by -dorep (redo multi-hardlink files)
  $DOREP=1 if $^O eq 'MSWin32';
  $DO_SORT=0;           ## set by -sort (sort files in a dir before checking)
  $FIND_ONLY=0;         ## set by -find (don't search files)
  $LIST_ONLY=0;		## set true by -l (list filenames only)
  $NEWER=0;             ## set by -newer, "-mtime -###"
  $NICE=0;              ## set by -nice (print human-readable output)
  $NOLINKS=0; 		## set true by -nolinks (don't follow symlinks)
  $OLDER=0;             ## set by -older, "-mtime  ###"
  $PREPEND_FILENAME=1;  ## set false by -h (don't prefix lines with filename)
  $REPORT_LINENUM=0;    ## set true by -n (show line numbers)
  $VERBOSE=0;		## set to a value by -v, -vv, etc. (verbose messages)
  $WHY=0;		## set true by -why, -vvv+ (report why skipped)
  $XDEV=0;		## set true by -xdev (stay on one filesystem)
  $all=0;		## set true by -all (don't skip many kinds of files)
  $iflag = '';		## set to 'i' by -i (ignore case);
  $norc=0;              ## set by -norc (don't load rc file)
  $showrc=0;            ## set by -showrc (show what happens with rc file)
  $underlineOK=0;       ## set true by -u (watch for underline stuff)
  $words=0;             ## set true by -w (match whole-words only)
  $DELAY=0;		## inter-file delay (seconds)
  $retval=1;            ## will set to 0 if we find anything.

  ## various elements of stat() that we might access
  $STAT_DEV   = 1;
  $STAT_INODE = 2;
  $STAT_MTIME = 9;

  $VV_PRINT_COUNT = 50;  ## with -vv, print every VV_PRINT_COUNT files, or...
  $VV_SIZE = 1024*1024;  ## ...every VV_SIZE bytes searched
  $vv_print = $vv_size = 0; ## running totals.

  ## set default options, in case the rc file wants them
  $opt{'TTY'}= 1 if -t STDOUT;
  
  ## want to know this for debugging message stuff
  $STDERR_IS_TTY = -t STDERR ? 1 : 0;
  $STDERR_SCREWS_STDOUT = ($STDERR_IS_TTY && -t STDOUT) ? 1 : 0;

  $0 =~ s,.*/,,;  ## clean up $0 for any diagnostics we'll be printing.
}

##
## Check arguments.
##
sub check_args
{
  while (@ARGV && $ARGV[0] =~ m/^-/)
  {
      $arg = shift(@ARGV);

      if ($arg eq '-version' || ($VERBOSE && $arg eq '-help')) {
	  print qq/Jeffrey's file search, version "$version".\n/;
	  exit(0) unless $arg eq '-help';
      }
      if ($arg eq '-help') {
	  print <<INLINE_LITERAL_TEXT;
usage: $0 [options] [-e] [PerlRegex ....]
OPTIONS TELLING *WHERE* TO SEARCH:
  -dir DIR       start search at the named directory (default is current dir).
  -xdev          stay on starting file system.
  -sort          sort the files in each directory before processing.
  -nolinks       don't follow symbolic links.
OPTIONS TELLING WHICH FILES TO EVEN CONSIDER:
  -mtime #       consider files modified > # days ago (-# for < # days old)
  -newer FILE    consider files modified more recently than FILE (also -older)
  -name GLOB     consider files whose name matches pattern (also -regex).
  -skip GLOB     opposite of -name: identifies files to not consider.
  -path GLOB     like -name, but for files whose whole path is described.
  -dpath/-dregex/-dskip versions for selecting or pruning directories.
  -all           don't skip any files marked to be skipped by the startup file.
  -x<SPECIAL>    (see manual, and/or try -showrc).
  -why           report why a file isn't checked (also implied by -vvvv).
OPTIONS TELLING WHAT TO DO WITH FILES THAT WILL BE CONSIDERED:
  -f  | -find    just list files (PerlRegex ignored). Default is to grep them.
  -ff | -ffind   Does a faster -find (implies -find -all -dorep)
OPTIONS CONTROLLING HOW THE SEARCH IS DONE (AND WHAT IS PRINTED):
  -l | -list     only list files with matches, not the lines themselves.
  -nice | -nnice print more "human readable" output.
  -n             prefix each output line with its line number in the file.
  -h             don't prefix output lines with file name.
  -u             also look "inside" manpage-style underlined text
  -i             do case-insensitive searching.
  -w             match words only (as defined by perl's \\b).
OTHER OPTIONS:
  -v, -vv, -vvv  various levels of message verbosity.
  -e             end of options (in case a regex looks like an option).
  -showrc        show what the rc file sets, then exit.
  -norc          don't load the rc file.
  -dorep         check files with multiple hard links multiple times.
INLINE_LITERAL_TEXT
	print "Use -v -help for more verbose help.\n" unless $VERBOSE;
	print "This script file is also a man page.\n" unless $stripped;
	print <<INLINE_LITERAL_TEXT if $VERBOSE;

If -f (or -find) given, PerlRegex is optional and ignored.
Otherwise, will search for files with lines matching any of the given regexes.

Combining things like -name and -mtime implies boolean AND.
However, duplicating things (such as -name '*.c' -name '*.txt') implies OR.

-mtime may be given floating point (i.e. 1.5 is a day and a half).
-iskip/-idskip/-ipath/... etc are case-insensitive versions.

If any letter in -newer/-older is upper case, "or equal" is
inserted into the test.

INLINE_LITERAL_TEXT
	  exit(0);
      }
      $DOREP=1,             next if $arg eq '-dorep';   ## do repeats
      $DO_SORT=1,           next if $arg eq '-sort';    ## sort files
      $NOLINKS=1,           next if $arg eq '-nolinks'; ## no sym. links
      $PREPEND_FILENAME=0,  next if $arg eq '-h';       ## no filename prefix
      $REPORT_LINENUM=1,    next if $arg eq '-n';       ## show line numbers
      $WHY=1,               next if $arg eq '-why';     ## tell why skipped
      $XDEV=1,              next if $arg eq '-xdev';    ## don't leave F.S.
      $all=1,$opt{'-all'}=1,next if $arg eq '-all';     ## don't skip *.Z, etc
      $iflag='i',           next if $arg eq '-i';       ## ignore case
      $norc=1,              next if $arg eq '-norc';    ## don't load rc file
      $showrc=1,            next if $arg eq '-showrc';  ## show rc file
      $underlineOK=1,       next if $arg eq '-u';       ## look through underln.
      $words=1,             next if $arg eq '-w';       ## match "words" only
      &strip                     if $arg eq '-strip';   ## dump this program
      last                       if $arg eq '-e';
      $DELAY=$1,            next if $arg =~ m/-delay(\d+)/;

      $FIND_ONLY=1,         next if $arg =~/^-f(ind)?$/;## do "find" only

      $FIND_ONLY=1, $DOREP=1, $all=1,
                            next if $arg =~/^-ff(ind)?$/;## fast -find
      $LIST_ONLY=1,$opt{'-list'}=1,
		            next if $arg =~/^-l(ist)?$/;## only list files

      if ($arg =~ m/^-(v+)$/) { ## verbosity
	$VERBOSE =length($1);
	foreach $len (1..$VERBOSE) { $opt{'-'.('v' x $len)}=1 }
	next;
      }
      if ($arg =~ m/^-(n+)ice$/) { ## "nice" output
        $NICE =length($1);
	foreach $len (1..$NICE) { $opt{'-'.('n' x $len).'ice'}=1 }
	next;
      }

      if ($arg =~ m/^-(i?)(d?)skip$/) {
	  local($i) = $1 eq 'i';
	  local($d) = $2 eq 'd';
	  $! = 2, die qq/$0: expecting glob arg to -$arg\n/ unless @ARGV;
	  foreach (split(/\s+/, shift @ARGV)) {
	      if ($d) {
		  $idskip{$_}=1 if $i;
		   $dskip{$_}=1;
	      } else {
		  $iskip{$_}=1 if $i;
		   $skip{$_}=1;
	      }
	  }
	  next;
      }


      if ($arg =~ m/^-(i?)(d?)(regex|path|name)$/) {
	  local($i) = $1 eq 'i';
	  $! = 2, die qq/$0: expecting arg to -$arg\n/ unless @ARGV;
	  foreach (split(/\s+/, shift @ARGV)) {
	      $iname{join(',', $arg, $_)}=1 if $i;
	       $name{join(',', $arg, $_)}=1;
	  }
	  next;
      }

      if ($arg =~ m/^-d?dir$/) {
	  $opt{'-dir'}=1;
	  $! = 2, die qq/$0: expecting filename arg to -$arg\n/ unless @ARGV;
	  $start = shift(@ARGV);
	  $start =~ s#^~(/+|$)#$ENV{'HOME'}$1# if defined $ENV{'HOME'};
	  $! = 2, die qq/$0: can't find ${arg}'s "$start"\n/ unless -e $start;
	  $! = 2, die qq/$0: ${arg}'s "$start" not a directory.\n/ unless -d _;
	  undef(@todo), $opt{'-ddir'}=1 if $arg eq '-ddir';
	  push(@todo, $start);
	  next;
      }

      if ($arg =~ m/^-(new|old)er$/i) {
	  $! = 2, die "$0: expecting filename arg to -$arg\n" unless @ARGV;
	  local($file, $time) = shift(@ARGV);
	  $! = 2, die qq/$0: can't stat -${arg}'s "$file"./
		  unless $time = (stat($file))[$STAT_MTIME];
	  local($upper) = $arg =~ tr/A-Z//;
	  if ($arg =~ m/new/i) {
	     $time++ unless $upper;
	     $NEWER = $time if $NEWER < $time;
	  } else {
	     $time-- unless $upper;
	     $OLDER = $time if $OLDER == 0 || $OLDER > $time;
	  }
	  next;
      }

      if ($arg =~ m/-mtime/) {
	  $! = 2, die "$0: expecting numerical arg to -$arg\n" unless @ARGV;
	  local($days) = shift(@ARGV);
	  $! = 2, die qq/$0: inappropriate arg ($days) to $arg\n/ if $days==0;
	  $days *= 3600 * 24;
	  if ($days < 0) {
	      local($time) = $^T + $days;
	      $NEWER = $time if $NEWER < $time;
	  } else {
	      local($time) = $^T - $days;
  	      $OLDER = $time if $OLDER == 0 || $OLDER > $time;
	  }
	  next;
      }

      ## special user options
      if ($arg =~ m/^-x(.+)/) {
	  foreach (split(/[\s,]+/, $1)) {  $user_opt{$_} = $opt{$_}= 1;  }
	  next;
      }

      $! = 2, die "$0: unknown arg [$arg]\n";
  }
}

##
## Given a filename glob, return a regex.
## If the glob has no globbing chars (no * ? or [..]), then
## prepend an effective '*' to it.
##
sub glob_to_regex
{
    local($glob) = @_;
    local(@parts) = $glob =~ m/\\.|[*?]|\[]?[^]]*]|[^[\\*?]+/g;
    local($trueglob)=0;
    foreach (@parts) {
	if ($_ eq '*' || $_ eq '?') {
	    $_ = ".$_";
	    $trueglob=1;  ## * and ? are a real glob
	} elsif (substr($_, 0, 1) eq '[') {
	    $trueglob=1;  ## [..] is a real glob
	} else {
	    s/^\\//;     ## remove any leading backslash;
	    s/\W/\\$&/g; ## now quote anything dangerous;
	}
    }
    unshift(@parts, '.*') unless $trueglob;
    join('', '^', @parts, '$');
}

sub prepare_to_search
{
  local($rc_file) = @_;

  $HEADER_BYTES=0;          ## Might be set nonzero in &read_rc;
  $last_message_length = 0; ## For &message and &clear_message.

  &read_rc($rc_file, $showrc) unless $norc;
  exit(0) if $showrc;

  $NEXT_DIR_ENTRY = $DO_SORT ? 'shift @files' : 'readdir(DIR)';
  $WHY = 1 if $VERBOSE > 3; ## Arg -vvvv or above implies  -why.
  @todo = ('.') if @todo == 0; ## Where we'll start looking

  ## see if any user options were specified that weren't accounted for
  foreach $opt (keys %user_opt) {
      next if defined $seen_opt{$opt};
      warn "warning: -x$opt never considered.\n";
  }

  die "$0: multiple time constraints exclude all possible files.\n"
      if ($NEWER && $OLDER) && ($NEWER > $OLDER);

  ##
  ## Process any -skip/-iskip args that had been given
  ##
  local(@skip_test);
  foreach $glob (keys %skip) {
      $i = defined($iskip{$glob}) ? 'i': '';
      push(@skip_test, '$name =~ m/'. &glob_to_regex($glob). "/$i");
  }
  if (@skip_test) {
      $SKIP_TEST = join('||',@skip_test);
      $DO_SKIP_TEST = 1;
  } else {
      $DO_SKIP_TEST = $SKIP_TEST = 0;
  }

  ##
  ## Process any -dskip/-idskip args that had been given
  ##
  local(@dskip_test);
  foreach $glob (keys %dskip) {
      $i = defined($idskip{$glob}) ? 'i': '';
      push(@dskip_test, '$name =~ m/'. &glob_to_regex($glob). "/$i");
  }
  if (@dskip_test) {
      $DSKIP_TEST = join('||',@dskip_test);
      $DO_DSKIP_TEST = 1;
  } else {
      $DO_DSKIP_TEST = $DSKIP_TEST = 0;
  }


  ##
  ## Process any -name, -path, -regex, etc. args that had been given.
  ##
  undef @name_test;
  undef @dname_test;
  foreach $key (keys %name) {
      local($type, $pat) = split(/,/, $key, 2);
      local($i) = defined($iname{$key}) ? 'i' : '';
      if ($type =~ /regex/) {
	  $pat =~ s/!/\\!/g;
	  $test = "\$name =~ m!^$pat\$!$i";
      } else {
	  local($var) = $type eq 'name' ? '$name' : '$file';
	  $test = "$var =~ m/". &glob_to_regex($pat). "/$i";
      }
      if ($type =~ m/^-i?d/) {
	  push(@dname_test, $test);
      } else {
	  push(@name_test, $test);
      }
  }
  if (@name_test) {
      $GLOB_TESTS = join('||', @name_test);

      $DO_GLOB_TESTS = 1;
  } else {
      $GLOB_TESTS = $DO_GLOB_TESTS = 0;
  }
  if (@dname_test) {
      $DGLOB_TESTS = join('||', @dname_test);
      $DO_DGLOB_TESTS = 1;
  } else {
      $DGLOB_TESTS = $DO_DGLOB_TESTS = 0;
  }


  ##
  ## Process any 'magic' things from the startup file.
  ##
  if (@magic_tests && $HEADER_BYTES) {
      ## the $magic' one is for when &dodir is not inlined
      $tests = join('||',@magic_tests);
      $MAGIC_TESTS = " { package magic; \$val = ($tests) }";
      $DO_MAGIC_TESTS = 1;
  } else {
      $MAGIC_TESTS = 1;
      $DO_MAGIC_TESTS = 0;
  }

  ##
  ## Prepare regular expressions.
  ##
  {
      local(@regex_tests);

      if ($LIST_ONLY) {
	 $mflag = '';
	 ## need to have $* set, but perl5 just won''t shut up about it.
	 if ($] >= 5) {
	      $mflag = 'm';
	 } else {
	      eval ' $* = 1 ';
	 }
      }

      ##
      ## Until I figure out a better way to deal with it,
      ## We have to worry about a regex like [^xyz] when doing $LIST_ONLY.
      ## Such a regex *will* match \n, and if I'm pulling in multiple
      ## lines, it can allow lines to match that would otherwise not match.
      ##
      ## Therefore, if there is a '[^' in a regex, we can NOT take a chance
      ## an use the fast listonly.
      ##
      $CAN_USE_FAST_LISTONLY = $LIST_ONLY;

      local(@extra);
      local($underline_glue) = ($] >= 5) ? '(:?_\cH)?' : '(_\cH)?';
      while (@ARGV) {
          $regex = shift(@ARGV);
	  ##
	  ## If watching for underlined things too, add another regex.
	  ##
	  if ($underlineOK) {
	     if ($regex =~ m/[?*+{}()\\.|^\$[]/) {
		warn "$0: warning, can't underline-safe '$regex'.\n";
	     } else {
		$regex = join($underline_glue, split(//, $regex));
	     }
	  }

	  ## If nothing special in the regex, just use index...
	  ## is quite a bit faster.
	  if (($iflag eq '') && ($words == 0) &&
			$regex !~ m/[?*+{}()\\.|^\$[]/)
	  {
	      push(@regex_tests, "(index(\$_, q+$regex+)>=0)");

	  } else {
	      $regex =~ s#[\$\@\/]\w#\\$&#;
	      if ($words) {
		  if ($regex =~ m/\|/) {
		      ## could be dangerous -- see if we can wrap in parens.
		      if ($regex =~ m/\\\d/) {
			  warn "warning: -w and a | in a regex is dangerous.\n"
		      } else {
			  $regex = join($regex, '(', ')');
		      }
		  }
		  $regex = join($regex, '\b', '\b');
	      }
	      $CAN_USE_FAST_LISTONLY = 0 if substr($regex, "[^") >= 0;
	      push(@regex_tests, "m/$regex/$iflag$mflag");
	  }

	  ## If we're done, but still have @extra to do, get set for that.
	  if (@ARGV == 0 && @extra) {
	      @ARGV = @extra;   ## now deal with the extra stuff.
	      $underlineOK = 0; ## but no more of this.
	      undef @extra;     ## or this.
	  }
      }
      if (@regex_tests) {
	  $REGEX_TEST = join('||', @regex_tests);
	  ## print STDERR $REGEX_TEST, "\n"; exit;
      } else {
	  ## must be doing -find -- just give something syntactically correct.
	  $REGEX_TEST = 1;
      }
  }

  ##
  ## Make sure we can read the first item(s).
  ##
  foreach $start (@todo) {
      $! = 2, die qq/$0: can't stat "$start"\n/
	  unless ($dev,$inode) = (stat($start))[$STAT_DEV,$STAT_INODE];

      if (defined $dir_done{"$dev,$inode"}) {
	  ## ignore the repeat.
	  warn(qq/ignoring "$start" (same as "$dir_done{"$dev,$inode"}").\n/)
		if $VERBOSE;
	  next;
      }

      ## if -xdev was given, remember the device.
      $xdev{$dev} = 1 if $XDEV;

      ## Note that we won't want to do it again
      $dir_done{"$dev,$inode"} = $start;
  }
}


##
## See the comment above the __END__ above the 'sub dodir' below.
##
sub import_program
{
    sub bad {
	print STDERR "$0: internal error (@_)\n";
	exit 2;
    }

    ## Read from data, up to next __END__. This will be &dodir.
    local($/) = "\n__END__";
    $prog = <DATA>;
    close(DATA);

    $prog =~ s/\beval\b//g;       ## remove any 'eval'

    ## Inline uppercase $-variables by their current values.
    if ($] >= 5) {
	$prog =~ s/\$([A-Z][A-Z0-9_]{2,}\b)/
		    &bad($1) if !defined ${$main::{$1}}; ${$main::{$1}};/eg;
    } else {
	$prog =~ s/\$([A-Z][A-Z0-9_]{2,}\b)/local(*VAR) = $_main{$1};
		    &bad($1) if !defined $VAR; $VAR;/eg;
    }

    eval $prog;  ## now do it. This will define &dodir;
    $!=2, die "$0 internal error: $@\n" if $@;
}

###########################################################################

##
## Read the .search file:
##    Blank lines and lines that are only #-comments ignored.
##    Newlines may be escaped to create long lines
##    Other lines are directives.
##
##    A directive may begin with an optional tag in the form <...>
##    Things inside the <...> are evaluated as with:
##	   <(this || that) && must>
##    will be true if
##       -xmust -xthis   or   -xmust -xthat
##    were specified on the command line (order doesn't matter, though)
##    A directive is not done if there is a tag and it's false.
##    Any characters but whitespace and &|()>,! may appear after an -x
##    (although "-xdev" is special).  -xmust,this is the same as -xmust -xthis.
##    Something like -x~ would make <~> true, and <!~> false.
##
##    Directives are in the form:
##      option: STRING
##	magic : NUMBYTES : EXPR
##
##    With option:
##      The STRING is parsed like a Bourne shell command line, and the
##      options are used as if given on the command line.
##      No comments are allowed on 'option' lines.
##	Examples:
##	    # skip objects and libraries
##	    option: -skip '.o .a'
##	    # skip emacs *~ and *# files, unless -x~ given:
##	    <!~> option: -skip '~ #'
##
##    With magic:
##	EXPR can be pretty much any perl (comments allowed!).
##      If it evaluates to true for any particular file, it is skipped.
##      The only info you'll have about a file is the variable $H, which
##      will have at least the first NUMBYTES of the file (less if the file
##      is shorter than that, of course, and maybe more). You'll also have
##      any variables you set in previous 'magic' lines.
##	Examples:
##	    magic: 6 : ($x6 = substr($H, 0, 6)) eq 'GIF87a'
##	    magic: 6 :  $x6                     eq 'GIF89a'
##
##          magic: 6 : (($x6 = substr($H, 0, 6)) eq 'GIF87a' ## old gif \
##		                         || $x6  eq 'GIF89a' ## new gif
##	(the above two sets are the same)
##	    ## Check the first 32 bytes for "binarish" looking bytes.
##	    ## Don't blindly dump on any high-bit set, as non-ASCII text
##	    ## often has them set. \x80 and \xff seem to be special, though.
##	    ## Require two in a row to not get things like perl's $^T.
##	    ## This is known to get *.Z, *.gz, pkzip, *.elc and about any
##	    ## executable you'll find.
##	    magic: 32 : $H =~ m/[\x00-\x06\x10-\x1a\x1c-\x1f\x80\xff]{2}/
##
sub read_rc
{
    local($file, $show) = @_;
    local($line_num, $ln, $tag) = 0;
    local($use_default, @default) = 0;

    { package magic; $^W= 0; } ## turn off warnings for when we run EXPR's

    unless (open(RC, '<', $file)) {
	$use_default=1;
	$file = "<internal default startup file>";
	## no RC file -- use this default.
	@default = split(/\n/,<<'--------INLINE_LITERAL_TEXT');
            magic: 32 : $H =~ m/[\x00-\x06\x10-\x1a\x1c-\x1f\x80\xff]{2}/
	    option: -skip '.a .elc .gz .o .pbm .xbm .dvi'
	    option: -iskip '.com .exe .lib .pdb .tarz .zip .z .lzh .jpg .jpeg .gif .uu'
	    <!~> option: -skip '~ #'
--------INLINE_LITERAL_TEXT
    }

    ##
    ## Make an eval error pretty.
    ##
    sub clean_eval_error {
	local($_) = @_;
	s/ in file \(eval\) at line \d+,//g; ## perl4-style error
	s/ at \(eval \d+\) line \d+,//g;     ## perl5-style error
	$_ = $` if m/\n/;                    ## remove all but first line
	"$_\n";
    }

    print "reading RC file: $file\n" if $show;

    while (defined($_ = ($use_default ? shift(@default) : <RC>))) {
	$ln = ++$line_num;			     ## note starting line num.
        $_ .= <RC>, $line_num++ while s/\\\n?$/\n/;  ## allow continuations
	next if /^\s*(#.*)?$/;          ## skip blank or comment-only lines.
        $do = '';
	
	## look for an initial <...> tag.
	if (s/^\s*<([^>]*)>//) {
	    ## This simple s// will make the tag ready to eval.
	    ($tag = $msg = $1) =~
		s/[^\s&|(!)]+/
			$seen_opt{$&}=1;         ## note seen option
			"defined(\$opt{q>$&>})"  ## (q>> is safe quoting here)
		/eg;
	    
	    ## see if the tag is true or not, abort this line if not.
	    $dothis = (eval $tag);
	    $!=2, die "$file $ln <$msg>: $_".&clean_eval_error($@) if $@;

	    if ($show) {
	        $msg =~ s/[^\s&|(!)]+/-x$&/;
	        $msg =~ s/\s*!\s*/ no /g;
	        $msg =~ s/\s*&&\s*/ and /g;
	        $msg =~ s/\s*\|\|\s*/ or /g;
		$msg =~ s/^\s+//; $msg =~ s/\s+$//;
		$do = $dothis ? "(doing because $msg)" :
				"(do if $msg)";
	    } elsif (!$dothis) {
	        next;
	    }
	}

	if (m/^\s*option\s*:\s*/) {
	    next if $all && !$show; ## -all turns off these checks;
	    local($_) = $';
            s/\n$//;
	    local($orig) = $_;
	    print " $do option: $_\n" if $show;
	    local($0) = "$0 ($file)"; ## for any error message.
	    local(@ARGV);
	    local($this);
	    ##
	    ## Parse $_ as a Bourne shell line -- fill @ARGV
	    ##
	    while (length) {
		if (s/^\s+//) {
		    push(@ARGV, $this) if defined $this;
		    undef $this;
		    next;
		}
		$this = '' if !defined $this;
		$this .= $1 while s/^'([^']*)'// ||
				  s/^"([^"]*)"// ||
				  s/^([^'"\s\\]+)//||
				  s/^(\\[\D\d])//;
		die "$file $ln: error parsing $orig at $_\n" if m/^\S/;
	    }
	    push(@ARGV, $this) if defined $this;
	    &check_args;
	    die qq/$file $ln: unused arg "@ARGV".\n/ if @ARGV;
	    next;
	}

	if (m/^\s*magic\s*:\s*(\d+)\s*:\s*/) {
	    next if $all && !$show; ## -all turns off these checks;
	    local($bytes, $check) = ($1, $');

	    if ($show) {
		$check =~ s/\n?$/\n/;
		print " $do contents: $check";
	    }
	    ## Check to make sure the thing at least compiles.
	    eval  "package magic; (\$H = '1'x \$main'bytes) && (\n$check\n)\n";
	    $! = 2, die "$file $ln: ".&clean_eval_error($@) if $@;

	    $HEADER_BYTES = $bytes if $bytes > $HEADER_BYTES;
	    push(@magic_tests, "(\n$check\n)");
	    next;
	}
	$! = 2, die "$file $ln: unknown command\n";
    }
    close(RC);
}

sub message
{
    if (!$STDERR_IS_TTY) {
	print STDERR $_[0], "\n";
    } else {
	local($text) = @_;
	$thislength = length($text);
	if ($thislength >= $last_message_length) {
	    print STDERR $text, "\r";
	} else {
	    print STDERR $text, ' 'x ($last_message_length-$thislength),"\r";
	}	
	$last_message_length = $thislength;
    }
}

sub clear_message
{
    print STDERR ' ' x $last_message_length, "\r" if $last_message_length;
    $vv_print = $vv_size = $last_message_length = 0;
}

##
## Output a copy of this program with comments, extra whitespace, and
## the trailing man page removed. On an ultra slow machine, such a copy
## might load faster (but I can't tell any difference on my machine).
##
sub strip {
    seek(DATA, 0, 0) || die "$0: can't reset internal pointer.\n";
    while(<DATA>) {
      print, next if /INLINE_LITERAL_TEXT/.../INLINE_LITERAL_TEXT/;
      ## must mention INLINE_LITERAL_TEXT on this line!
      s/\#\#.*|^\s+|\s+$//; ## remove cruft
      last if $_ eq '.00;';
      next if ($_ eq '') || ($_ eq "'di'") || ($_ eq "'ig00'");
      s/\$stripped=0;/\$stripped=1;/;
      s/\s\s+/ /;  ## squish multiple whitespaces down to one.
      print $_, "\n";
    }
    exit(0);
}

##
## Just to shut up -w. Never executed.
##
sub dummy {

    1 || &dummy || &dir_done || &bad || &message || $NEXT_DIR_ENTRY ||
    $DELAY || $VV_SIZE || $VV_PRINT_COUNT || $STDERR_SCREWS_STDOUT ||
    @files || @files || $magic'H || $magic'H || $xdev{''} || &clear_message;

}

##
## If the following __END__ is in place, what follows will be
## inlined when the program first starts up. Any $ variable name
## all in upper case, specifically, any string matching
##	\$([A-Z][A-Z0-9_]{2,}\b
## will have the true value for that variable inlined. Also, any 'eval' is
## removed
##
## The idea is that when the whole thing is then eval'ed to define &dodir,
## the perl optimizer will make all the decisions that are based upon
## command-line options (such as $VERBOSE), since they'll be inlined as
## constants
##
## Also, and here's the big win, the tests for matching the regex, and a
## few others, are all inlined. Should be blinding speed here.
##
## See the read from <DATA> above for where all this takes place.
## But all-in-all, you *want* the __END__ here. Comment it out only for
## debugging....
##

__END__

##
## Given a directory, check all "appropriate" files in it.
## Shove any subdirectories into the global @todo, so they'll be done
## later.
##
## Be careful about adding any upper-case variables, as they are subject
## to being inlined. See comments above the __END__ above.
##
sub dodir
{
  local($dir) = @_;
  $dir =~ s,/+$,,; ## remove any trailing slash.
  unless (opendir(DIR, "$dir/.")) {
      &clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
      warn qq($0: can't opendir "$dir/".\n);
      return;
  }

  if ($VERBOSE) {
      &message($dir);
      $vv_print = $vv_size = 0;
  }

  @files = sort readdir(DIR) if $DO_SORT;

  while (defined($name = eval $NEXT_DIR_ENTRY))
  {
    next if $name eq '.' || $name eq '..'; ## never follow these.

    ## create full relative pathname.
    $file = $dir eq '.' ? $name : "$dir/$name";

    ## if link and skipping them, do so.
    if ($NOLINKS && -l $file) {
	warn qq/skip (symlink): $file\n/ if $WHY;
	next;
    }

    ## skip things unless files or directories
    unless (-f $file || -d _) {
	if ($WHY) {
	    $why = (-S _ && "socket")       ||
		   (-p _ && "pipe")         ||
		   (-b _ && "block special")||
		   (-c _ && "char special") || "somekinda special";
	    warn qq/skip ($why): $file\n/;
	}
	next;
    }

    ## skip things we can't read
    unless (-r _) {
	if ($WHY) {
	    $why = (-l $file) ? "follow" : "read";
	    warn qq/skip (can't $why): $file\n/;
	}
	next;
    }

    ## skip things that are empty
    unless (-s _ || -d _) {
	warn qq/skip (empty): $file\n/ if $WHY;
	next;
    }

    ## Note file device & inode. If -xdev, skip if appropriate.
    ($dev, $inode) = (stat(_))[$STAT_DEV, $STAT_INODE];
    if ($XDEV && defined $xdev{$dev}) {
	warn qq/skip (other device): $file\n/ if $WHY;
	next;
    }
    $id = "$dev,$inode";

    ## special work for a directory
    if (-d _) {
	## Do checks for directory file endings.
	if ($DO_DSKIP_TEST && (eval $DSKIP_TEST)) {
	    warn qq/skip (-dskip): $file\n/ if $WHY;
	    next;
	}
	## do checks for -name/-regex/-path tests
	if ($DO_DGLOB_TESTS && !(eval $DGLOB_TESTS)) {
	    warn qq/skip (dirname): $file\n/ if $WHY;
	    next;
	}

	## _never_ redo a directory
	if (defined $dir_done{$id} and $^O ne 'MSWin32') {
	    warn qq/skip (did as "$dir_done{$id}"): $file\n/ if $WHY;
	    next;
	}
	$dir_done{$id} = $file;     ## mark it done.
	unshift(@todo, $file);	    ## add to the list to do.
	next;
    }
    if ($WHY == 0  && $VERBOSE > 1) {
      if ($VERBOSE>2||$vv_print++>$VV_PRINT_COUNT||($vv_size+=-s _)>$VV_SIZE){
	  &message($file);
	  $vv_print = $vv_size = 0;
      }
    }

    ## do time-related tests
    if ($NEWER || $OLDER) {
	$_ = (stat(_))[$STAT_MTIME];
	if ($NEWER && $_ < $NEWER) {
	    warn qq/skip (too old): $file\n/ if $WHY;
	    next;
	}
	if ($OLDER && $_ > $OLDER) {
	    warn qq/skip (too new): $file\n/ if $WHY;
	    next;
	}
    }

    ## do checks for file endings
    if ($DO_SKIP_TEST && (eval $SKIP_TEST)) {
	warn qq/skip (-skip): $file\n/ if $WHY;
	next;
    }

    ## do checks for -name/-regex/-path tests
    if ($DO_GLOB_TESTS && !(eval $GLOB_TESTS)) {
	warn qq/skip (filename): $file\n/ if $WHY;
	next;
    }


    ## If we're not repeating files,
    ##	skip this one if we've done it, or note we're doing it.
    unless ($DOREP) {
	if (defined $file_done{$id}) {
	    warn qq/skip (did as "$file_done{$id}"): $file\n/ if $WHY;
	    next;
	}
	$file_done{$id} = $file;
    }

    if ($DO_MAGIC_TESTS) {
	if (!open(FILE_IN, '<', $file)) {
	    &clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
	    warn qq/$0: can't open: $file\n/;
	    next;
	}
	unless (read(FILE_IN, $magic'H, $HEADER_BYTES)) {
	    &clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
	    warn qq/$0: can't read from "$file"\n"/;
	    close(FILE_IN);
	    next;
	}

	eval $MAGIC_TESTS;
	if ($magic'val) {
	    close(FILE_IN);
	    warn qq/skip (magic): $file\n/ if $WHY;
	    next;
	}
	seek(FILE_IN, 0, 0);  ## reset for later <FILE_IN>
    }

    if ($WHY != 0  && $VERBOSE > 1) {
      if ($VERBOSE>2||$vv_print++>$VV_PRINT_COUNT||($vv_size+=-s _)>$VV_SIZE){
	  &message($file);
	  $vv_print = $vv_size = 0;
      }
    }

    if ($DELAY) {
	sleep($DELAY);
    }

    if ($FIND_ONLY) {
	&clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
	print $file, "\n";
	$retval=0; ## we've found something
	close(FILE_IN) if $DO_MAGIC_TESTS;
	next;
    } else {
	## if we weren't doing magic tests, file won't be open yet...
	if (!$DO_MAGIC_TESTS && !open(FILE_IN, '<', $file)) {
	    &clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
	    warn qq/$0: can't open: $file\n/;
	    next;
	}
	if ($LIST_ONLY && $CAN_USE_FAST_LISTONLY) {
	    ##
	    ## This is rather complex, but buys us a LOT when we're just
	    ## listing files and not the individual internal lines.
	    ##
	    local($size) = 4096;  ## block-size in which to do reads
	    local($nl);           ## will point to $_'s ending newline.
	    local($read);	  ## will be how many bytes read.
	    local($_) = '';       ## Starts out empty
	    local($hold);	  ## (see below)

	    while (($read = read(FILE_IN,$_,$size,length($_)))||length($_))
	    {
		undef @parts;
		## if read a full block, but no newline, need to read more.
		while ($read == $size && ($nl = rindex($_, "\n")) < 0) {
		    push(@parts, $_);                    ## save that part
		    $read = read(FILE_IN, $_, $size); ## keep trying
		}

		##
		## If we had to save parts, must now combine them together.
		## adjusting $nl to reflect the now-larger $_. This should
		## be a lot more efficient than using any kind of .= in the
		## loop above.
		##
		if (@parts) {
		    local($lastlen) = length($_); #only need if $nl >= 0
		    $_ = join('', @parts, $_);
		    $nl = length($_) - ($lastlen - $nl) if $nl >= 0;
		}

		##
		## If we're at the end of the file, then we can use $_ as
		## is.  Otherwise, we need to remove the final partial-line
		## and save it so that it'll be at the beginning of the
		## next read (where the rest of the line will be layed in
		## right after it).  $hold will be what we should save
		## until next time.
		##
		if ($read != $size || $nl < 0) {
		    $hold = '';
		} else {
		    $hold = substr($_, $nl + 1);
		    substr($_, $nl + 1) = '';
		}

		##
		## Now have a bunch of full lines in $_. Use it.
		##
		if (eval $REGEX_TEST) {
		    &clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
		    print $file, "\n";
		    $retval=0; ## we've found something

		    last;
		}

		## Prepare for next read....
		$_ = $hold;
	    }

	} else {  ## else not using faster block scanning.....

            $lines_printed = 0 if $NICE;
	    while (<FILE_IN>) {
		study;
		next unless (eval $REGEX_TEST);

		##
		## We found a matching line.
		##
		$retval=0;
		&clear_message if $VERBOSE && $STDERR_SCREWS_STDOUT;
		if ($LIST_ONLY) {
		    print $file, "\n";
		    last;
		} else {
		    ## prepare to print line.
		    if ($NICE && $lines_printed++ == 0) {
			print '-' x 70, "\n" if $NICE > 1;
			print $file, ":\n";
		    }

		    ##
		    ## Print all the prelim stuff. This looks less efficient
		    ## than it needs to be, but that's so that when the eval
		    ## is compiled (and the tests are optimized away), the
		    ## result will be less actual PRINTs than the more natural
		    ## way of doing these tests....
		    ##
		    if ($NICE) {
			if ($REPORT_LINENUM) {
			    print " line $.:  ";
			} else {
			    print "  ";
			}
		    } elsif ($REPORT_LINENUM && $PREPEND_FILENAME) {
			print "$file,:$.: ";
		    } elsif ($PREPEND_FILENAME) {
			print "$file: ";
		    } elsif ($REPORT_LINENUM) {
			print "$.: ";
		    }
		    print $_;
		    print "\n" unless m/\n$/;
		}
	    }
	    print "\n" if ($NICE > 1) && $lines_printed;
	}
	close(FILE_IN);
    }
  }
  closedir(DIR);
}

__END__
.00;			## finish .ig
 
'di			\" finish diversion--previous line must be blank
.nr nl 0-1		\" fake up transition to first page again
.nr % 0			\" start at page 1
.\"__________________NORMAL_MAN_PAGE_BELOW_________________
.ll+10n
.TH search 1 "Dec 17, 1994"
.SH SEARCH
search \- search files (a'la grep) in a whole directory tree.
.SH SYNOPSIS
search [ grep-like and find-like options] [regex ....]
.SH DESCRIPTION
.I Search
is more or less a combo of 'find' and 'grep' (although the regular
expression flavor is that of the perl being used, which is closer to
egrep's than grep's).

.I Search
does generally the same kind of thing that
.nf
   find <blah blah> | xargs egrep <blah blah>
.fi
does, but is
.I much
more powerful and efficient (and intuitive, I think).

This manual describes
.I search
as of version "941227.4".

.SH "QUICK EXAMPLE"
Basic use is simple:
.nf
    % search jeff
.fi
will search files in the current directory, and all sub directories, for
files that have "jeff" in them. The lines will be listed with the
containing file's name prepended.
.PP
If you list more than one regex, such as with
.nf
    % search jeff Larry Randal+ 'Stoc?k' 'C.*son'
.fi
then a line containing any of the regexes will be listed.
This makes it effectively the same as
.nf
    % search 'jeff|Larry|Randal+|Stoc?k|C.*son'
.fi
However, listing them separately is much more efficient (and is easier
to type).
.PP
Note that in the case of these examples, the
.B \-w
(list whole-words only) option would be useful.
.PP
Normally, various kinds of files are automatically removed from consideration.
If it has a certain ending (such as ".tar", ".Z", ".o", .etc), or if
the beginning of the file looks like a binary, it'll be excluded.
You can control exactly how this works -- see below. One quick way to
override this is to use the
.B \-all
option, which means to consider all the files that would normally be
automatically excluded.
Or, if you're curious, you can use
.B \-why
to have notes about what files are skipped (and why) printed to stderr.

.SH "BASIC OVERVIEW"
Normally, the search starts in the current directory, considering files in
all subdirectories.

You can use the
.I ~/.search
file to control ways to automatically exclude files.
If you don't have this file, a default one will kick in, which automatically
add
.nf
    -skip .o .Z .gif
.fi
(among others) to exclude those kinds of files (which you probably want to
skip when searching for text, as is normal).
Files that look to be binary will also be excluded.

Files ending with "#" and "~" will also be excluded unless the
.B -x~
option is given. 

You can use
.B -showrc
to show what kinds of files will normally be skipped.
See the section on the startup file
for more info.

You can use the
.B -all
option to indicate you want to consider all files that would otherwise be
skipped by the startup file.

Based upon various other flags (see "WHICH FILES TO CONSIDER" below),
more files might be removed from consideration. For example
.nf
    -mtime 3
.fi
will exclude files that aren't at least three days old (change the 3 to -3
to exclude files that are more than three days old), while
.nf
    -skip .*
.fi
would exclude any file beginning with a dot (of course, '.' and '..'  are
special and always excluded).

If you'd like to see what files are being excluded, and why, you can get the
list via the
.B \-why
option.

If a file makes it past all the checks, it is then "considered".
This usually means it is greped for the regular expressions you gave
on the command line.

If any of the regexes match a line, the line is printed.
However, if
.B -list
is given, just the filename is printed. Or, if
.B -nice
is given, a somewhat more (human-)readable output is generated.

If you're searching a huge tree and want to keep informed about how
the search is progressing,
.B -v
will print (to stderr) the current directory being searched.
Using
.B -vv
will also print the current file "every so often", which could be useful
if a directory is huge. Using
.B -vvv
will print the update with every file.

Below is the full listing of options.

.SH "OPTIONS TELLING *WHERE* TO SEARCH"
.TP
.BI -dir " DIR"
Start searching at the named directory instead of the current directory.
If multiple
.B -dir
arguments are given, multiple trees will be searched.
.TP
.BI -ddir " DIR"
Like
.B -dir
except it flushes any previous
.B -dir
directories (i.e. "-dir A -dir B -dir C" will search A, B, and C, while
"-dir A -ddir B -dir C" will search only B and C. This might be of use
in the startup file (see that section below).
.TP
.B -xdev
Stay on the same filesystem as the starting directory/directories.
.TP
.B -sort
Sort the items in a directory before processing them.
Normally they are processed in whatever order they happen to be read from
the directory.
.TP
.B -nolinks
Don't follow symbolic links. Normally they're followed.

.SH "OPTIONS CONTROLLING WHICH FILES TO CONSIDER AND EXCLUDE"
.TP
.BI -mtime " NUM"
Only consider files that were last changed more than
.I NUM
days ago
(less than
.I NUM
days if
.I NUM
has '-' prepended, i.e. "-mtime -2.5" means to consider files that
have been changed in the last two and a half days).
.TP
.B -older FILE
Only consider files that have not changed since
.I FILE
was last changed.
If there is any upper case in the "-older", "or equal" is added to the sense
of the test.  Therefore, "search -older ./file regex" will never consider
"./file", while "search -Older ./file regex" will.

If a file is a symbolic link, the time used is that of the file and not the
link.
.TP
.BI -newer " FILE"
Opposite of
.BR -older .
.TP
.BI -name " GLOB"
Only consider files that match the shell filename pattern
.IR GLOB .
The check is only done on a file's name (use
.B -path
to check the whole path, and use
.B -dname
to check directory names).

Multiple specifications can be given by separating them with spaces, a'la
.nf
    -name '*.c *.h'
.fi
to consider C source and header files.
If
.I GLOB
doesn't contain any special pattern characters, a '*' is prepended.
This last example could have been given as
.nf
   -name '.c .h'
.fi
It could also be given as
.nf
    -name .c -name .h
.fi
or
.nf
    -name '*.c' -name '*.h'
.fi
or
.nf
    -name '*.[ch]'
.fi
(among others)
but in this last case, you have to be sure to supply the leading '*'.
.TP
.BI -path " GLOB"
Like
.B -name
except the entire path is checked against the pattern.
.TP
.B -regex " REGEX"
Considers files whose names (not paths) match the given perl regex
exactly.
.TP
.BI -iname " GLOB"
Case-insensitive version of
.BR -name .
.TP
.BI -ipath " GLOB"
Case-insensitive version of
.BR -path .
.TP
.BI -iregex " REGEX"
Case-insensitive version of
.BR -regex .

.TP
.BI -dpath " GLOB"
Only search down directories whose path matches the given pattern (this
doesn't apply to the initial directory given by
.BI -dir ,
of course).
Something like
.nf
    -dir /usr/man -dpath /usr/man/man*
.fi
would completely skip
"/usr/man/cat1", "/usr/man/cat2", etc.
.TP
.BI -dskip " GLOB"
Skips directories whose name (not path) matches the given pattern.
Something like
.nf
    -dir /usr/man -dskip cat*
.fi
would completely skip any directory in the tree whose name begins with "cat"
(including "/usr/man/cat1", "/usr/man/cat2", etc.).
.TP
.BI -dregex " REGEX"
Like
.BI -dpath ,
but the pattern is a full perl regex. Note that this quite different
from
.B -regex
which considers only file names (not paths). This option considers
full directory paths (not just names). It's much more useful this way.
Sorry if it's confusing.
.TP
.BI -dpath " GLOB"
This option exists, but is probably not very useful. It probably wants to
be like the '-below' or something I mention in the "TODO" section.
.TP
.BI -idpath " GLOB"
Case-insensitive version of
.BR -dpath .
.TP
.BI -idskip " GLOB"
Case-insensitive version of
.BR -dskip .
.TP
.BI -idregex " REGEX"
Case-insensitive version of
.BR -dregex .
.TP
.B -all
Ignore any 'magic' or 'option' lines in the startup file.
The effect is that all files that would otherwise be automatically
excluded are considered.
.TP
.BI -x SPECIAL
Arguments starting with
.B -x
(except
.BR -xdev ,
explained elsewhere) do special interaction with the
.I ~/.search
startup file. Something like
.nf
	-xflag1 -xflag2
.fi
will turn on "flag1" and "flag2" in the startup file (and is
the same as "-xflag1,flag2"). You can use this to write your own
rules for what kinds of files are to be considered.

For example, the internal-default startup file contains the line
.nf
	<!~> option: -skip '~ #'
.fi
This means that if the
.B -x~
flag is
.I not
seen, the option
.nf
    -skip '~ #'
.fi
should be done.
The effect is that emacs temp and backup files are not normally
considered, but you can included them with the -x~ flag.

You can write your own rules to customize
.I search
in powerful ways. See the STARTUP FILE section below.
.TP
.B -why
Print a message (to stderr) when and why a file is not considered.

.SH "OPTIONS TELLING WHAT TO DO WITH FILES THAT WILL BE CONSIDERED"
.TP
.B -find
(you can use
.B -f
as well).
This option changes the basic action of
.IR search .

Normally, if a file is considered, it is searched
for the regular expressions as described earlier. However, if this option
is given, the filename is printed and no searching takes place. This turns
.I search
into a 'find' of some sorts.

In this case, no regular expressions are needed on the command line
(any that are there are silently ignored).

This is not intended to be a replacement for the 'find' program,
but to aid
you in understanding just what files are getting past the exclusion checks.
If you really want to use it as a sort of replacement for the 'find' program,
you might want to use
.B -all
so that it doesn't waste time checking to see if the file is binary, etc
(unless you really want that, of course).

If you use
.BR -find ,
none of the "GREP-LIKE OPTIONS" (below) matter.

As a replacement for 'find',
.I search
is probably a bit slower (or in the case of GNU find, a lot slower --
GNU find is
.I unbelievably
fast).
However, "search -ffind"
might be more useful than 'find' when options such as
.B -skip
are used (at least until 'find' gets such functionality).
.TP
.B -ffind
(or
.BR -ff )
A faster more 'find'-like find. Does
.nf
    -find  -all -dorep
.fi
.SH "GREP-LIKE OPTIONS"
These options control how a searched file is accessed,
and how things are printed.
.TP
.B -i
Ignore letter case when matching.
.TP
.B -w
Consider only whole-word matches ("whole word" as defined by perl's "\\b"
regex).
.TP
.B -u
If the regex(es) is/are simple, try to modify them so that they'll work
in manpage-like underlined text (i.e. like _^Ht_^Hh_^Hi_^Hs).
This is very rudimentary at the moment.
.TP
.B -list
(you can use
.B -l
too).
Don't print matching lines, but the names of files that contain matching
lines. This will likely be *much* faster, as special optimizations are
made -- particularly with large files.
.TP
.B -n
Pepfix each line by its line number.
.TP
.B -nice
Not a grep-like option, but similar to
.BR -list ,
so included here.
.B -nice
will have the output be a bit more human-readable, with matching lines printed
slightly indented after the filename, a'la
.nf

   % search foo
   somedir/somefile: line with foo in it
   somedir/somefile: some food for thought
   anotherdir/x: don't be a buffoon!
   %

.fi
will become
.nf

   % search -nice foo
   somedir/somefile:
     line with foo in it
     some food for thought
   anotherdir/x:
     don't be a buffoon!
   %

.fi
This option due to Lionel Cons.
.TP
.B -nnice
Be a bit nicer than
.BR -nice .
Prefix each file's output by a rule line, and follow with an extra blank line.
.TP
.B -h
Don't prepend each output line with the name of the file
(meaningless when
.B -find
or
.B -l
are given).

.SH "OTHER OPTIONS"
.TP
.B -help
Print the usage information.
.TP
.B -version
Print the version information and quit.
.TP
.B -v
Set the level of message verbosity.
.B -v
will print a note whenever a new directory is entered.
.B -vv
will also print a note "every so often". This can be useful to see
what's happening when searching huge directories.
.B -vvv
will print a new with every file.
.B -vvvv
is
-vvv
plus
.BR -why .
.TP
.B -e
This ends the options, and can be useful if the regex begins with '-'.
.TP
.B -showrc
Shows what is being considered in the startup file, then exits.
.TP
.B -dorep
Normally, an identical file won't be checked twice (even with multiple
hard or symbolic links). If you're just trying to do a fast
.BR -find ,
the bookkeeping to remember which files have been seen is not desirable,
so you can eliminate the bookkeeping with this flag.

.SH "STARTUP FILE"
When
.I search
starts up, it processes the directives in
.IR ~/.search .
If no such file exists, a default
internal version is used.

The internal version looks like:
.nf

   magic: 32 : $H =~ m/[\ex00-\ex06\ex10-\ex1a\ex1c-\ex1f\ex80\exff]{2}/
   option: -skip '.a .COM .elc .EXE .gz .o .pbm .xbm .dvi'
   option: -iskip '.tarz .zip .z .lzh .jpg .jpeg .gif .uu'
   <!~> option: -skip '~ #'

.fi
If you wish to create your own "~/.search",
you might consider copying the above, and then working from there.

There are two kinds of directives in a startup file: "magic" and "option".
.RS 0n
.TP
OPTION
Option lines will automatically do the command-line options given.
For example, the line
.nf
	option: -v
.fi
in you startup file will turn on -v every time, without needing to type it
on the command line.

The text on the line after the "option:" directive is processed
like the Bourne shell, so make sure to pay attention to quoting.
.nf
	option: -skip .exe .com
.fi
will give an error (".com" by itself isn't a valid option), while
.nf
	option: -skip ".exe .com"
.fi
will properly include it as part of -skip's argument.

.TP
MAGIC
Magic lines are used to determine if a file should be considered a binary
or not (the term "magic" refers to checking a file's magic number).  These
are described in more detail below.
.RE

Blank lines and comments (lines beginning with '#') are allowed.

If a line begins with  <...>, then it's a check to see if the
directive on the line should be done or not. The stuff inside the <...>
can contain perl's && (and), || (or), ! (not), and parens for grouping,
along with "flags" that might be indicated by the user with
.BI -x flag
options.

For example, using "-xfoo" will cause "foo" to be true inside the <...>
blocks. Therefore, a line beginning with "<foo>" would be done only when
"-xfoo" had been specified, while a line beginning with "<!foo>" would be
done only when "-xfoo" is not specified (of course, a line without any <...>
is done in either case).

A realistic example might be
.nf
	<!v> -vv
.fi
This will cause -vv messages to be the default, but allow "-xv" to override.

There are a few flags that are set automatically:
.RS
.TP
.B TTY
true if the output is to the screen (as opposed to being redirected to a file).
You can force this (as with all the other automatic flags) with -xTTY.
.TP
.B -v
True if -v was specified. If -vv was specified, both 
.B -v
and
.B -vv
flags are true (and so on).
.TP
.B -nice
True if -nice was specified. Same thing about -nnice as for -vv.
.PP
.TP
.B -list
true if -list (or -l) was given.
.TP
.B -dir
true if -dir was given.
.RE

Using this info, you might change the last example to
.nf

    <!v && !-v> option: -vv

.fi
The added "&& !-v" means "and if the '-v' option not given".
This will allow you to use "-v" alone on the command line, and not
have this directive add the more verbose "-vv" automatically.

.RS 0
Some other examples:
.TP
<!-dir && !here> option: -dir ~/
Effectively make the default directory your home directory (instead of the
current directory). Using -dir or -xhere will undo this.
.TP
<tex> option: -name .tex -dir ~/pub
Create '-xtex' to search only "*.tex" files in your ~/pub directory tree.
Actually, this could be made a bit better. If you combine '-xtex' and '-dir'
on the command line, this directive will add ~/pub to the list, when you
probably want to use the -dir directory only. You could do
.nf

   <tex> option: -name .tex
   <tex && !-dir> option: -dir ~/pub
.fi

to will allow '-xtex' to work as before, but allow a command-line "-dir"
to take precedence with respect to ~/pub.
.TP
<fluff> option: -nnice -sort -i -vvv
Combine a few user-friendly options into one '-xfluff' option.
.TP
<man> option: -ddir /usr/man -v -w
When the '-xman' option is given, search "/usr/man" for whole-words
(of whatever regex or regexes are given on the command line), with -v.
.RE

The lines in the startup file are executed from top to bottom, so something
like
.nf

   <both> option: -xflag1 -xflag2
   <flag1> option: ...whatever...
   <flag2> option: ...whatever...

.fi
will allow '-xboth' to be the same as '-xflag1 -xflag2' (or '-xflag1,flag2'
for that matter). However, if you put the "<both>" line below the others,
they will not be true when encountered, so the result would be different
(and probably undesired).

The "magic" directives are used to determine if a file looks to be binary
or not. The form of a magic line is
.nf
    magic: \fISIZE\fP : \fIPERLCODE\fP
.fi
where
.I SIZE
is the number of bytes of the file you need to check, and
.I PERLCODE
is the code to do the check. Within
.IR PERLCODE ,
the variable $H will hold at least the first
.I SIZE
bytes of the file (unless the file is shorter than that, of course).
It might hold more bytes. The perl should evaluate to true if the file
should be considered a binary.

An example might be
.nf
    magic: 6 : substr($H, 0, 6) eq 'GIF87a'
.fi
to test for a GIF ("-iskip .gif" is better, but this might be useful
if you have images in files without the ".gif" extension).

Since the startup file is checked from top to bottom, you can be a bit
efficient:
.nf
    magic: 6 : ($x6 = substr($H, 0, 6)) eq 'GIF87a'
    magic: 6 :  $x6                     eq 'GIF89a'
.fi
You could also write the same thing as
.nf
  magic: 6 : (($x6 = substr($H, 0, 6)) eq 'GIF87a') || ## an old gif, or.. \e
	       $x6                     eq 'GIF89a'     ## .. a new one.
.fi
since newlines may be escaped.

The default internal startup file includes
.nf
   magic: 32 : $H =~ m/[\ex00-\ex06\ex10-\ex1a\ex1c-\ex1f\ex80\exff]{2}/
.fi
which checks for certain non-printable characters, and catches a large
number of binary files, including most system's executables, linkable
objects, compressed, tarred, and otherwise folded, spindled, and mutilated
files.

Another example might be
.nf
    ## an archive library
    magic: 17 : substr($H, 0, 17) eq "!<arch>\en__.SYMDEF"
.fi

.SH "RETURN VALUE"
.I Search
returns zero if lines (or files, if appropriate) were found,
or if no work was requested (such as with
.BR -help ).
Returns 1 if no lines (or files) were found.
Returns 2 on error.

.SH TODO
Things I'd like to add some day:
.nf
  + show surrounding lines (context).
  + highlight matched portions of lines.
  + add '-and', which can go between regexes to override
    the default logical or of the regexes.
  + add something like
      -below GLOB
    which will examine a tree and only consider files that
    lie in a directory deeper than one named by the pattern.
  + add 'warning' and 'error' directives.
  + add 'help' directive.
.fi
.SH BUGS
If -xdev and multiple -dir arguments are given, any file in any of the
target filesystems are allowed. It would be better to allow each filesystem
for each separate tree.

Multiple -dir args might also cause some confusing effects. Doing
.nf
   -dir some/dir -dir other
.fi
will search "some/dir" completely, then search "other" completely. This
is good. However, something like
.nf
   -dir some/dir -dir some/dir/more/specific
.fi
will search "some/dir" completely *except for* "some/dir/more/specific",
after which it will return and be searched. Not really a bug, but just sort
of odd.

File times (for -newer, etc.) of symbolic links are for the file, not the
link. This could cause some misunderstandings.

Probably more. Please let me know.
.SH AUTHOR
Jeffrey Friedl, Omron Corp (jfriedl@omron.co.jp)
.br
http://www.wg.omron.co.jp/cgi-bin/j-e/jfriedl.html

.SH "LATEST SOURCE"
See http://www.wg.omron.co.jp/~jfriedl/perl/index.html
