use 5.006;  # we use some open(X, "<", $y) syntax

package Pod::Perldoc;
use strict;
use warnings;
use Config '%Config';

use Fcntl;    # for sysopen
use File::Basename qw(basename);
use File::Spec::Functions qw(catfile catdir splitdir);

use vars qw($VERSION @Pagers $Bindir $Pod2man
  $Temp_Files_Created $Temp_File_Lifetime
);
$VERSION = '3.2801';

#..........................................................................

BEGIN {  # Make a DEBUG constant very first thing...
  unless(defined &DEBUG) {
    if(($ENV{'PERLDOCDEBUG'} || '') =~ m/^(\d+)/) { # untaint
      eval("sub DEBUG () {$1}");
      die "WHAT? Couldn't eval-up a DEBUG constant!? $@" if $@;
    } else {
      *DEBUG = sub () {0};
    }
  }
}

use Pod::Perldoc::GetOptsOO; # uses the DEBUG.
use Carp qw(croak carp);

# these are also in BaseTo, which I don't want to inherit
sub debugging {
	my $self = shift;

    ( defined(&Pod::Perldoc::DEBUG) and &Pod::Perldoc::DEBUG() )
	}

sub debug {
	my( $self, @messages ) = @_;
	return unless $self->debugging;
	print STDERR map { "DEBUG : $_" } @messages;
	}

sub warn {
  my( $self, @messages ) = @_;

  carp( join "\n", @messages, '' );
  }

sub die {
  my( $self, @messages ) = @_;

  croak( join "\n", @messages, '' );
  }

#..........................................................................

sub TRUE  () {1}
sub FALSE () {return}
sub BE_LENIENT () {1}

BEGIN {
 *is_vms     = $^O eq 'VMS'     ? \&TRUE : \&FALSE unless defined &is_vms;
 *is_mswin32 = $^O eq 'MSWin32' ? \&TRUE : \&FALSE unless defined &is_mswin32;
 *is_dos     = $^O eq 'dos'     ? \&TRUE : \&FALSE unless defined &is_dos;
 *is_os2     = $^O eq 'os2'     ? \&TRUE : \&FALSE unless defined &is_os2;
 *is_cygwin  = $^O eq 'cygwin'  ? \&TRUE : \&FALSE unless defined &is_cygwin;
 *is_linux   = $^O eq 'linux'   ? \&TRUE : \&FALSE unless defined &is_linux;
 *is_hpux    = $^O =~ m/hpux/   ? \&TRUE : \&FALSE unless defined &is_hpux;
 *is_amigaos = $^O eq 'amigaos' ? \&TRUE : \&FALSE unless defined &is_amigaos;
}

$Temp_File_Lifetime ||= 60 * 60 * 24 * 5;
  # If it's older than five days, it's quite unlikely
  #  that anyone's still looking at it!!
  # (Currently used only by the MSWin cleanup routine)


#..........................................................................
{ my $pager = $Config{'pager'};
  push @Pagers, $pager if -x (split /\s+/, $pager)[0] or __PACKAGE__->is_vms;
}
$Bindir  = $Config{'scriptdirexp'};
$Pod2man = "pod2man" . ( $Config{'versiononly'} ? $Config{'version'} : '' );

# End of class-init stuff
#
###########################################################################
#
# Option accessors...

foreach my $subname (map "opt_$_", split '', q{mhlDriFfXqnTdULva}) {
  no strict 'refs';
  *$subname = do{ use strict 'refs';  sub () { shift->_elem($subname, @_) } };
}

# And these are so that GetOptsOO knows they take options:
sub opt_a_with { shift->_elem('opt_a', @_) }
sub opt_f_with { shift->_elem('opt_f', @_) }
sub opt_q_with { shift->_elem('opt_q', @_) }
sub opt_d_with { shift->_elem('opt_d', @_) }
sub opt_L_with { shift->_elem('opt_L', @_) }
sub opt_v_with { shift->_elem('opt_v', @_) }

sub opt_w_with { # Specify an option for the formatter subclass
  my($self, $value) = @_;
  if($value =~ m/^([-_a-zA-Z][-_a-zA-Z0-9]*)(?:[=\:](.*?))?$/s) {
    my $option = $1;
    my $option_value = defined($2) ? $2 : "TRUE";
    $option =~ tr/\-/_/s;  # tolerate "foo-bar" for "foo_bar"
    $self->add_formatter_option( $option, $option_value );
  } else {
    $self->warn( qq("$value" isn't a good formatter option name.  I'm ignoring it!\n ) );
  }
  return;
}

sub opt_M_with { # specify formatter class name(s)
  my($self, $classes) = @_;
  return unless defined $classes and length $classes;
  DEBUG > 4 and print "Considering new formatter classes -M$classes\n";
  my @classes_to_add;
  foreach my $classname (split m/[,;]+/s, $classes) {
    next unless $classname =~ m/\S/;
    if( $classname =~ m/^(\w+(::\w+)+)$/s ) {
      # A mildly restrictive concept of what modulenames are valid.
      push @classes_to_add, $1; # untaint
    } else {
      $self->warn(  qq("$classname" isn't a valid classname.  Ignoring.\n) );
    }
  }

  unshift @{ $self->{'formatter_classes'} }, @classes_to_add;

  DEBUG > 3 and print(
    "Adding @classes_to_add to the list of formatter classes, "
    . "making them @{ $self->{'formatter_classes'} }.\n"
  );

  return;
}

sub opt_V { # report version and exit
  print join '',
    "Perldoc v$VERSION, under perl v$] for $^O",

    (defined(&Win32::BuildNumber) and defined &Win32::BuildNumber())
     ? (" (win32 build ", &Win32::BuildNumber(), ")") : (),

    (chr(65) eq 'A') ? () : " (non-ASCII)",

    "\n",
  ;
  exit;
}

sub opt_t { # choose plaintext as output format
  my $self = shift;
  $self->opt_o_with('text')  if @_ and $_[0];
  return $self->_elem('opt_t', @_);
}

sub opt_u { # choose raw pod as output format
  my $self = shift;
  $self->opt_o_with('pod')  if @_ and $_[0];
  return $self->_elem('opt_u', @_);
}

sub opt_n_with {
  # choose man as the output format, and specify the proggy to run
  my $self = shift;
  $self->opt_o_with('man')  if @_ and $_[0];
  $self->_elem('opt_n', @_);
}

sub opt_o_with { # "o" for output format
  my($self, $rest) = @_;
  return unless defined $rest and length $rest;
  if($rest =~ m/^(\w+)$/s) {
    $rest = $1; #untaint
  } else {
    $self->warn( qq("$rest" isn't a valid output format.  Skipping.\n") );
    return;
  }

  $self->aside("Noting \"$rest\" as desired output format...\n");

  # Figure out what class(es) that could actually mean...

  my @classes;
  foreach my $prefix ("Pod::Perldoc::To", "Pod::Simple::", "Pod::") {
    # Messy but smart:
    foreach my $stem (
      $rest,  # Yes, try it first with the given capitalization
      "\L$rest", "\L\u$rest", "\U$rest" # And then try variations

    ) {
      $self->aside("Considering $prefix$stem\n");
      push @classes, $prefix . $stem;
    }

    # Tidier, but misses too much:
    #push @classes, $prefix . ucfirst(lc($rest));
  }
  $self->opt_M_with( join ";", @classes );
  return;
}

###########################################################################
# % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %

sub run {  # to be called by the "perldoc" executable
  my $class = shift;
  if(DEBUG > 3) {
    print "Parameters to $class\->run:\n";
    my @x = @_;
    while(@x) {
      $x[1] = '<undef>'  unless defined $x[1];
      $x[1] = "@{$x[1]}" if ref( $x[1] ) eq 'ARRAY';
      print "  [$x[0]] => [$x[1]]\n";
      splice @x,0,2;
    }
    print "\n";
  }
  return $class -> new(@_) -> process() || 0;
}

# % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %
###########################################################################

sub new {  # yeah, nothing fancy
  my $class = shift;
  my $new = bless {@_}, (ref($class) || $class);
  DEBUG > 1 and print "New $class object $new\n";
  $new->init();
  $new;
}

#..........................................................................

sub aside {  # If we're in -D or DEBUG mode, say this.
  my $self = shift;
  if( DEBUG or $self->opt_D ) {
    my $out = join( '',
      DEBUG ? do {
        my $callsub = (caller(1))[3];
        my $package = quotemeta(__PACKAGE__ . '::');
        $callsub =~ s/^$package/'/os;
         # the o is justified, as $package really won't change.
        $callsub . ": ";
      } : '',
      @_,
    );
    if(DEBUG) { print $out } else { print STDERR $out }
  }
  return;
}

#..........................................................................

sub usage {
  my $self = shift;
  $self->warn( "@_\n" ) if @_;

  # Erase evidence of previous errors (if any), so exit status is simple.
  $! = 0;

  CORE::die( <<EOF );
perldoc [options] PageName|ModuleName|ProgramName|URL...
perldoc [options] -f BuiltinFunction
perldoc [options] -q FAQRegex
perldoc [options] -v PerlVariable

Options:
    -h   Display this help message
    -V   Report version
    -r   Recursive search (slow)
    -i   Ignore case
    -t   Display pod using pod2text instead of Pod::Man and groff
             (-t is the default on win32 unless -n is specified)
    -u   Display unformatted pod text
    -m   Display module's file in its entirety
    -n   Specify replacement for groff
    -l   Display the module's file name
    -U   Don't attempt to drop privs for security
    -F   Arguments are file names, not modules (implies -U)
    -D   Verbosely describe what's going on
    -T   Send output to STDOUT without any pager
    -d output_filename_to_send_to
    -o output_format_name
    -M FormatterModuleNameToUse
    -w formatter_option:option_value
    -L translation_code   Choose doc translation (if any)
    -X   Use index if present (looks for pod.idx at $Config{archlib})
    -q   Search the text of questions (not answers) in perlfaq[1-9]
    -f   Search Perl built-in functions
    -a   Search Perl API
    -v   Search predefined Perl variables

PageName|ModuleName|ProgramName|URL...
         is the name of a piece of documentation that you want to look at. You
         may either give a descriptive name of the page (as in the case of
         `perlfunc') the name of a module, either like `Term::Info' or like
         `Term/Info', or the name of a program, like `perldoc', or a URL
         starting with http(s).

BuiltinFunction
         is the name of a perl function.  Will extract documentation from
         `perlfunc' or `perlop'.

FAQRegex
         is a regex. Will search perlfaq[1-9] for and extract any
         questions that match.

Any switches in the PERLDOC environment variable will be used before the
command line arguments.  The optional pod index file contains a list of
filenames, one per line.
                                                       [Perldoc v$VERSION]
EOF

}

#..........................................................................

sub program_name {
  my( $self ) = @_;

  if( my $link = readlink( $0 ) ) {
    $self->debug( "The value in $0 is a symbolic link to $link\n" );
    }

  my $basename = basename( $0 );

  $self->debug( "\$0 is [$0]\nbasename is [$basename]\n" );
  # possible name forms
  #   perldoc
  #   perldoc-v5.14
  #   perldoc-5.14
  #   perldoc-5.14.2
  #   perlvar         # an alias mentioned in Camel 3
  {
  my( $untainted ) = $basename =~ m/(
    \A
    perl
      (?: doc | func | faq | help | op | toc | var # Camel 3
      ) 
    (?: -? v? \d+ \. \d+ (?:\. \d+)? )? # possible version
    (?: \. (?: bat | exe | com ) )?    # possible extension
    \z
    )
    /x;

  $self->debug($untainted);
  return $untainted if $untainted;
  }

  $self->warn(<<"HERE");
You called the perldoc command with a name that I didn't recognize.
This might mean that someone is tricking you into running a
program you don't intend to use, but it also might mean that you
created your own link to perldoc. I think your program name is
[$basename].

I'll allow this if the filename only has [a-zA-Z0-9._-].
HERE

  {
  my( $untainted ) = $basename =~ m/(
    \A [a-zA-Z0-9._-]+ \z
    )/x;

  $self->debug($untainted);
  return $untainted if $untainted;
  }

  $self->die(<<"HERE");
I think that your name for perldoc is potentially unsafe, so I'm
going to disallow it. I'd rather you be safe than sorry. If you
intended to use the name I'm disallowing, please tell the maintainers
about it. Write to:

    Pod-Perldoc\@rt.cpan.org

HERE
}

#..........................................................................

sub usage_brief {
  my $self = shift;
  my $program_name = $self->program_name;

  CORE::die( <<"EOUSAGE" );
Usage: $program_name [-hVriDtumUFXlT] [-n nroffer_program]
    [-d output_filename] [-o output_format] [-M FormatterModule]
    [-w formatter_option:option_value] [-L translation_code]
    PageName|ModuleName|ProgramName

Examples:

    $program_name -f PerlFunc
    $program_name -q FAQKeywords
    $program_name -v PerlVar
    $program_name -a PerlAPI

The -h option prints more help.  Also try "$program_name perldoc" to get
acquainted with the system.                        [Perldoc v$VERSION]
EOUSAGE

}

#..........................................................................

sub pagers { @{ shift->{'pagers'} } }

#..........................................................................

sub _elem {  # handy scalar meta-accessor: shift->_elem("foo", @_)
  if(@_ > 2) { return  $_[0]{ $_[1] } = $_[2]  }
  else       { return  $_[0]{ $_[1] }          }
}
#..........................................................................
###########################################################################
#
# Init formatter switches, and start it off with __bindir and all that
# other stuff that ToMan.pm needs.
#

sub init {
  my $self = shift;

  # Make sure creat()s are neither too much nor too little
  eval { umask(0077) };   # doubtless someone has no mask

  if ( $] < 5.008 ) {
      $self->aside("Your old perl doesn't have proper unicode support.");
    }
  else {
      # http://www.perl.com/pub/2012/04/perlunicookbook-decode-argv-as-utf8.html
      # Decode command line arguments as UTF-8. See RT#98906 for example problem.
      use Encode qw(decode_utf8);
      @ARGV = map { decode_utf8($_, 1) } @ARGV;
    }

  $self->{'args'}              ||= \@ARGV;
  $self->{'found'}             ||= [];
  $self->{'temp_file_list'}    ||= [];


  $self->{'target'} = undef;

  $self->init_formatter_class_list;

  $self->{'pagers' } = [@Pagers] unless exists $self->{'pagers'};
  $self->{'bindir' } = $Bindir   unless exists $self->{'bindir'};
  $self->{'pod2man'} = $Pod2man  unless exists $self->{'pod2man'};
  $self->{'search_path'} = [ ]   unless exists $self->{'search_path'};

  push @{ $self->{'formatter_switches'} = [] }, (
   # Yeah, we could use a hashref, but maybe there's some class where options
   # have to be ordered; so we'll use an arrayref.

     [ '__bindir'  => $self->{'bindir' } ],
     [ '__pod2man' => $self->{'pod2man'} ],
  );

  DEBUG > 3 and printf "Formatter switches now: [%s]\n",
   join ' ', map "[@$_]", @{ $self->{'formatter_switches'} };

  $self->{'translators'} = [];
  $self->{'extra_search_dirs'} = [];

  return;
}

#..........................................................................

sub init_formatter_class_list {
  my $self = shift;
  $self->{'formatter_classes'} ||= [];

  # Remember, no switches have been read yet, when
  # we've started this routine.

  $self->opt_M_with('Pod::Perldoc::ToPod');   # the always-there fallthru
  $self->opt_o_with('text');

  return;
}

#..........................................................................

sub process {
    # if this ever returns, its retval will be used for exit(RETVAL)

    my $self = shift;
    DEBUG > 1 and print "  Beginning process.\n";
    DEBUG > 1 and print "  Args: @{$self->{'args'}}\n\n";
    if(DEBUG > 3) {
        print "Object contents:\n";
        my @x = %$self;
        while(@x) {
            $x[1] = '<undef>'  unless defined $x[1];
            $x[1] = "@{$x[1]}" if ref( $x[1] ) eq 'ARRAY';
            print "  [$x[0]] => [$x[1]]\n";
            splice @x,0,2;
        }
        print "\n";
    }

    # TODO: make it deal with being invoked as various different things
    #  such as perlfaq".

    return $self->usage_brief  unless  @{ $self->{'args'} };
    $self->options_reading;
    $self->pagers_guessing;
    $self->aside(sprintf "$0 => %s v%s\n", ref($self), $self->VERSION);
    $self->drop_privs_maybe unless ($self->opt_U || $self->opt_F);
    $self->options_processing;

    # Hm, we have @pages and @found, but we only really act on one
    # file per call, with the exception of the opt_q hack, and with
    # -l things

    $self->aside("\n");

    my @pages;
    $self->{'pages'} = \@pages;
    if(    $self->opt_f) { @pages = qw(perlfunc perlop)        }
    elsif( $self->opt_q) { @pages = ("perlfaq1" .. "perlfaq9") }
    elsif( $self->opt_v) { @pages = ("perlvar")                }
    elsif( $self->opt_a) { @pages = ("perlapi")                }
    else                 { @pages = @{$self->{'args'}};
                           # @pages = __FILE__
                           #  if @pages == 1 and $pages[0] eq 'perldoc';
                         }

    return $self->usage_brief  unless  @pages;

    $self->find_good_formatter_class();
    $self->formatter_sanity_check();

    $self->maybe_extend_searchpath();
      # for when we're apparently in a module or extension directory

    my @found = $self->grand_search_init(\@pages);
    exit ($self->is_vms ? 98962 : 1) unless @found;

    if ($self->opt_l and not $self->opt_q ) {
        DEBUG and print "We're in -l mode, so byebye after this:\n";
        print join("\n", @found), "\n";
        return;
    }

    $self->tweak_found_pathnames(\@found);
    $self->assert_closing_stdout;
    return $self->page_module_file(@found)  if  $self->opt_m;
    DEBUG > 2 and print "Found: [@found]\n";

    return $self->render_and_page(\@found);
}

#..........................................................................
{

my( %class_seen, %class_loaded );
sub find_good_formatter_class {
  my $self = $_[0];
  my @class_list = @{ $self->{'formatter_classes'} || [] };
  $self->die( "WHAT?  Nothing in the formatter class list!?" ) unless @class_list;

  local @INC = @INC;
  pop @INC if $INC[-1] eq '.';

  my $good_class_found;
  foreach my $c (@class_list) {
    DEBUG > 4 and print "Trying to load $c...\n";
    if($class_loaded{$c}) {
      DEBUG > 4 and print "OK, the already-loaded $c it is!\n";
      $good_class_found = $c;
      last;
    }

    if($class_seen{$c}) {
      DEBUG > 4 and print
       "I've tried $c before, and it's no good.  Skipping.\n";
      next;
    }

    $class_seen{$c} = 1;

    if( $c->can('parse_from_file') ) {
      DEBUG > 4 and print
       "Interesting, the formatter class $c is already loaded!\n";

    } elsif(
      ( $self->is_os2 or $self->is_mswin32 or $self->is_dos or $self->is_os2)
       # the always case-insensitive filesystems
      and $class_seen{lc("~$c")}++
    ) {
      DEBUG > 4 and print
       "We already used something quite like \"\L$c\E\", so no point using $c\n";
      # This avoids redefining the package.
    } else {
      DEBUG > 4 and print "Trying to eval 'require $c'...\n";

      local $^W = $^W;
      if(DEBUG() or $self->opt_D) {
        # feh, let 'em see it
      } else {
        $^W = 0;
        # The average user just has no reason to be seeing
        #  $^W-suppressible warnings from the require!
      }

      eval "require $c";
      if($@) {
        DEBUG > 4 and print "Couldn't load $c: $!\n";
        next;
      }
    }

    if( $c->can('parse_from_file') ) {
      DEBUG > 4 and print "Settling on $c\n";
      my $v = $c->VERSION;
      $v = ( defined $v and length $v ) ? " version $v" : '';
      $self->aside("Formatter class $c$v successfully loaded!\n");
      $good_class_found = $c;
      last;
    } else {
      DEBUG > 4 and print "Class $c isn't a formatter?!  Skipping.\n";
    }
  }

  $self->die( "Can't find any loadable formatter class in @class_list?!\nAborting" )
    unless $good_class_found;

  $self->{'formatter_class'} = $good_class_found;
  $self->aside("Will format with the class $good_class_found\n");

  return;
}

}
#..........................................................................

sub formatter_sanity_check {
  my $self = shift;
  my $formatter_class = $self->{'formatter_class'}
   || $self->die( "NO FORMATTER CLASS YET!?" );

  if(!$self->opt_T # so -T can FORCE sending to STDOUT
    and $formatter_class->can('is_pageable')
    and !$formatter_class->is_pageable
    and !$formatter_class->can('page_for_perldoc')
  ) {
    my $ext =
     ($formatter_class->can('output_extension')
       && $formatter_class->output_extension
     ) || '';
    $ext = ".$ext" if length $ext;

    my $me = $self->program_name;
    $self->die(
       "When using Perldoc to format with $formatter_class, you have to\n"
     . "specify -T or -dsomefile$ext\n"
     . "See `$me perldoc' for more information on those switches.\n" )
    ;
  }
}

#..........................................................................

sub render_and_page {
    my($self, $found_list) = @_;

    $self->maybe_generate_dynamic_pod($found_list);

    my($out, $formatter) = $self->render_findings($found_list);

    if($self->opt_d) {
      printf "Perldoc (%s) output saved to %s\n",
        $self->{'formatter_class'} || ref($self),
        $out;
      print "But notice that it's 0 bytes long!\n" unless -s $out;


    } elsif(  # Allow the formatter to "page" itself, if it wants.
      $formatter->can('page_for_perldoc')
      and do {
        $self->aside("Going to call $formatter\->page_for_perldoc(\"$out\")\n");
        if( $formatter->page_for_perldoc($out, $self) ) {
          $self->aside("page_for_perldoc returned true, so NOT paging with $self.\n");
          1;
        } else {
          $self->aside("page_for_perldoc returned false, so paging with $self instead.\n");
          '';
        }
      }
    ) {
      # Do nothing, since the formatter has "paged" it for itself.

    } else {
      # Page it normally (internally)

      if( -s $out ) {  # Usual case:
        $self->page($out, $self->{'output_to_stdout'}, $self->pagers);

      } else {
        # Odd case:
        $self->aside("Skipping $out (from $$found_list[0] "
         . "via $$self{'formatter_class'}) as it is 0-length.\n");

        push @{ $self->{'temp_file_list'} }, $out;
        $self->unlink_if_temp_file($out);
      }
    }

    $self->after_rendering();  # any extra cleanup or whatever

    return;
}

#..........................................................................

sub options_reading {
    my $self = shift;

    if( defined $ENV{"PERLDOC"} and length $ENV{"PERLDOC"} ) {
      require Text::ParseWords;
      $self->aside("Noting env PERLDOC setting of $ENV{'PERLDOC'}\n");
      # Yes, appends to the beginning
      unshift @{ $self->{'args'} },
        Text::ParseWords::shellwords( $ENV{"PERLDOC"} )
      ;
      DEBUG > 1 and print "  Args now: @{$self->{'args'}}\n\n";
    } else {
      DEBUG > 1 and print "  Okay, no PERLDOC setting in ENV.\n";
    }

    DEBUG > 1
     and print "  Args right before switch processing: @{$self->{'args'}}\n";

    Pod::Perldoc::GetOptsOO::getopts( $self, $self->{'args'}, 'YES' )
     or return $self->usage;

    DEBUG > 1
     and print "  Args after switch processing: @{$self->{'args'}}\n";

    return $self->usage if $self->opt_h;

    return;
}

#..........................................................................

sub options_processing {
    my $self = shift;

    if ($self->opt_X) {
        my $podidx = "$Config{'archlib'}/pod.idx";
        $podidx = "" unless -f $podidx && -r _ && -M _ <= 7;
        $self->{'podidx'} = $podidx;
    }

    $self->{'output_to_stdout'} = 1  if  $self->opt_T or ! -t STDOUT;

    $self->options_sanity;

    # This used to set a default, but that's now moved into any
    # formatter that cares to have a default.
    if( $self->opt_n ) {
        $self->add_formatter_option( '__nroffer' => $self->opt_n );
    }

    # Get language from PERLDOC_POD2 environment variable
    if ( ! $self->opt_L && $ENV{PERLDOC_POD2} ) {
        if ( $ENV{PERLDOC_POD2} eq '1' ) {
          $self->_elem('opt_L',(split(/\_/, $ENV{LC_ALL} || $ENV{LC_LANG} || $ENV{LANG}))[0] );
        }
        else {
          $self->_elem('opt_L', $ENV{PERLDOC_POD2});
        }
    };

    # Adjust for using translation packages
    $self->add_translator(split(/\s+/,$self->opt_L)) if $self->opt_L;

    return;
}

#..........................................................................

sub options_sanity {
    my $self = shift;

    # The opts-counting stuff interacts quite badly with
    # the $ENV{"PERLDOC"} stuff.  I.e., if I have $ENV{"PERLDOC"}
    # set to -t, and I specify -u on the command line, I don't want
    # to be hectored at that -u and -t don't make sense together.

    #my $opts = grep $_ && 1, # yes, the count of the set ones
    #  $self->opt_t, $self->opt_u, $self->opt_m, $self->opt_l
    #;
    #
    #$self->usage("only one of -t, -u, -m or -l") if $opts > 1;


    # Any sanity-checking need doing here?

    # But does not make sense to set either -f or -q in $ENV{"PERLDOC"}
    if( $self->opt_f or $self->opt_q or $self->opt_a) {
    my $count;
    $count++ if $self->opt_f;
    $count++ if $self->opt_q;
    $count++ if $self->opt_a;
    $self->usage("Only one of -f or -q or -a") if $count > 1;
    $self->warn(
        "Perldoc is meant for reading one file at a time.\n",
        "So these parameters are being ignored: ",
        join(' ', @{$self->{'args'}}),
        "\n" )
        if @{$self->{'args'}}
    }
    return;
}

#..........................................................................

sub grand_search_init {
    my($self, $pages, @found) = @_;

    foreach (@$pages) {
        if (/^http(s)?:\/\//) {
            require HTTP::Tiny;
            require File::Temp;
            my $response = HTTP::Tiny->new->get($_);
            if ($response->{success}) {
                my ($fh, $filename) = File::Temp::tempfile(UNLINK => 1);
                $fh->print($response->{content});
                push @found, $filename;
                ($self->{podnames}{$filename} =
                  m{.*/([^/#?]+)} ? uc $1 : "UNKNOWN")
                   =~ s/\.P(?:[ML]|OD)\z//;
            }
            else {
              print STDERR "No " .
                    ($self->opt_m ? "module" : "documentation") . " found for \"$_\".\n";
              if ( /^https/ ) {
                print STDERR "You may need an SSL library (such as IO::Socket::SSL) for that URL.\n";
              }
            }
            next;
        }
        if ($self->{'podidx'} && open(PODIDX, $self->{'podidx'})) {
            my $searchfor = catfile split '::', $_;
            $self->aside( "Searching for '$searchfor' in $self->{'podidx'}\n" );
            local $_;
            while (<PODIDX>) {
                chomp;
                push(@found, $_) if m,/$searchfor(?:\.(?:pod|pm))?\z,i;
            }
            close(PODIDX)            or $self->die( "Can't close $$self{'podidx'}: $!" );
            next;
        }

        $self->aside( "Searching for $_\n" );

        if ($self->opt_F) {
            next unless -r;
            push @found, $_ if $self->opt_l or $self->opt_m or $self->containspod($_);
            next;
        }

        my @searchdirs;

        # prepend extra search directories (including language specific)
        push @searchdirs, @{ $self->{'extra_search_dirs'} };

        # We must look both in @INC for library modules and in $bindir
        # for executables, like h2xs or perldoc itself.
        push @searchdirs, ($self->{'bindir'}, @{$self->{search_path}}, @INC);
        unless ($self->opt_m) {
            if ($self->is_vms) {
                my($i,$trn);
                for ($i = 0; $trn = $ENV{'DCL$PATH;'.$i}; $i++) {
                    push(@searchdirs,$trn);
                }
                push(@searchdirs,'perl_root:[lib.pods]')  # installed pods
            }
            else {
                push(@searchdirs, grep(-d, split($Config{path_sep},
                                                 $ENV{'PATH'})));
            }
        }
        my @files = $self->searchfor(0,$_,@searchdirs);
        if (@files) {
            $self->aside( "Found as @files\n" );
        }
        # add "perl" prefix, so "perldoc foo" may find perlfoo.pod
    elsif (BE_LENIENT and !/\W/ and  @files = $self->searchfor(0, "perl$_", @searchdirs)) {
            $self->aside( "Loosely found as @files\n" );
        }
        else {
            # no match, try recursive search
            @searchdirs = grep(!/^\.\z/s,@INC);
            @files= $self->searchfor(1,$_,@searchdirs) if $self->opt_r;
            if (@files) {
                $self->aside( "Loosely found as @files\n" );
            }
            else {
                print STDERR "No " .
                    ($self->opt_m ? "module" : "documentation") . " found for \"$_\".\n";
                if ( @{ $self->{'found'} } ) {
                    print STDERR "However, try\n";
                    my $me = $self->program_name;
                    for my $dir (@{ $self->{'found'} }) {
                        opendir(DIR, $dir) or $self->die( "opendir $dir: $!" );
                        while (my $file = readdir(DIR)) {
                            next if ($file =~ /^\./s);
                            $file =~ s/\.(pm|pod)\z//;  # XXX: badfs
                            print STDERR "\t$me $_\::$file\n";
                        }
                        closedir(DIR)    or $self->die( "closedir $dir: $!" );
                    }
                }
            }
        }
        push(@found,@files);
    }
    return @found;
}

#..........................................................................

sub maybe_generate_dynamic_pod {
    my($self, $found_things) = @_;
    my @dynamic_pod;

    $self->search_perlapi($found_things, \@dynamic_pod)   if  $self->opt_a;

    $self->search_perlfunc($found_things, \@dynamic_pod)  if  $self->opt_f;

    $self->search_perlvar($found_things, \@dynamic_pod)   if  $self->opt_v;

    $self->search_perlfaqs($found_things, \@dynamic_pod)  if  $self->opt_q;

    if( ! $self->opt_f and ! $self->opt_q and ! $self->opt_v and ! $self->opt_a) {
        DEBUG > 4 and print "That's a non-dynamic pod search.\n";
    } elsif ( @dynamic_pod ) {
        $self->aside("Hm, I found some Pod from that search!\n");
        my ($buffd, $buffer) = $self->new_tempfile('pod', 'dyn');
        if ( $] >= 5.008 && $self->opt_L ) {
            binmode($buffd, ":encoding(UTF-8)");
            print $buffd "=encoding utf8\n\n";
        }

        push @{ $self->{'temp_file_list'} }, $buffer;
         # I.e., it MIGHT be deleted at the end.

        my $in_list = !$self->not_dynamic && $self->opt_f || $self->opt_v || $self->opt_a;

        print $buffd "=over 8\n\n" if $in_list;
        print $buffd @dynamic_pod  or $self->die( "Can't print $buffer: $!" );
        print $buffd "=back\n"     if $in_list;

        close $buffd        or $self->die( "Can't close $buffer: $!" );

        @$found_things = $buffer;
          # Yes, so found_things never has more than one thing in
          #  it, by time we leave here

        $self->add_formatter_option('__filter_nroff' => 1);

    } else {
        @$found_things = ();
        $self->aside("I found no Pod from that search!\n");
    }

    return;
}

#..........................................................................

sub not_dynamic {
  my ($self,$value) = @_;
  $self->{__not_dynamic} = $value if @_ == 2;
  return $self->{__not_dynamic};
}

#..........................................................................

sub add_formatter_option { # $self->add_formatter_option('key' => 'value');
  my $self = shift;
  push @{ $self->{'formatter_switches'} }, [ @_ ] if @_;

  DEBUG > 3 and printf "Formatter switches now: [%s]\n",
   join ' ', map "[@$_]", @{ $self->{'formatter_switches'} };

  return;
}

#.........................................................................

sub new_translator { # $tr = $self->new_translator($lang);
    my $self = shift;
    my $lang = shift;

    local @INC = @INC;
    pop @INC if $INC[-1] eq '.';
    my $pack = 'POD2::' . uc($lang);
    eval "require $pack";
    if ( !$@ && $pack->can('new') ) {
    return $pack->new();
    }

    eval { require POD2::Base };
    return if $@;

    return POD2::Base->new({ lang => $lang });
}

#.........................................................................

sub add_translator { # $self->add_translator($lang);
    my $self = shift;
    for my $lang (@_) {
        my $tr = $self->new_translator($lang);
        if ( defined $tr ) {
            push @{ $self->{'translators'} }, $tr;
            push @{ $self->{'extra_search_dirs'} }, $tr->pod_dirs;

            $self->aside( "translator for '$lang' loaded\n" );
        } else {
            # non-installed or bad translator package
            $self->warn( "Perldoc cannot load translator package for '$lang': ignored\n" );
        }

    }
    return;
}

#..........................................................................

sub open_fh {
    my ($self, $op, $path) = @_;

    open my $fh, $op, $path or $self->die("Couldn't open $path: $!");
    return $fh;
}

sub set_encoding {
    my ($self, $fh, $encoding) = @_;

    if ( $encoding =~ /utf-?8/i ) {
        $encoding = ":encoding(UTF-8)";
    }
    else {
        $encoding = ":encoding($encoding)";
    }

    if ( $] < 5.008 ) {
        $self->aside("Your old perl doesn't have proper unicode support.");
    }
    else {
        binmode($fh, $encoding);
    }

    return $fh;
}

sub search_perlvar {
    my($self, $found_things, $pod) = @_;

    my $opt = $self->opt_v;

    if ( $opt !~ /^ (?: [\@\%\$]\S+ | [A-Z]\w* ) $/x ) {
        CORE::die( "'$opt' does not look like a Perl variable\n" );
    }

    DEBUG > 2 and print "Search: @$found_things\n";

    my $perlvar = shift @$found_things;
    my $fh = $self->open_fh("<", $perlvar);

    if ( $opt ne '$0' && $opt =~ /^\$\d+$/ ) { # handle $1, $2, ...
      $opt = '$<I<digits>>';
    }
    my $search_re = quotemeta($opt);

    DEBUG > 2 and
     print "Going to perlvar-scan for $search_re in $perlvar\n";

    # Skip introduction
    local $_;
    my $enc;
    while (<$fh>) {
        $enc = $1 if /^=encoding\s+(\S+)/;
        last if /^=over 8/;
    }

    $fh = $self->set_encoding($fh, $enc) if $enc;

    # Look for our variable
    my $found = 0;
    my $inheader = 1;
    my $inlist = 0;
    while (<$fh>) {  
        last if /^=head2 Error Indicators/;
        # \b at the end of $` and friends borks things!
        if ( m/^=item\s+$search_re\s/ )  {
            $found = 1;
        }
        elsif (/^=item/) {
            last if $found && !$inheader && !$inlist;
        }
        elsif (!/^\s+$/) { # not a blank line
            if ( $found ) {
                $inheader = 0; # don't accept more =item (unless inlist)
        }
            else {
                @$pod = (); # reset
                $inheader = 1; # start over
                next;
            }
    }

        if (/^=over/) {
            ++$inlist;
        }
        elsif (/^=back/) {
            last if $found && !$inheader && !$inlist;
            --$inlist;
        }
        push @$pod, $_;
#        ++$found if /^\w/;        # found descriptive text
    }
    @$pod = () unless $found;
    if (!@$pod) {
        CORE::die( "No documentation for perl variable '$opt' found\n" );
    }
    close $fh                or $self->die( "Can't close $perlvar: $!" );

    return;
}

#..........................................................................

sub search_perlop {
  my ($self,$found_things,$pod) = @_;

  $self->not_dynamic( 1 );

  my $perlop = shift @$found_things;
  # XXX FIXME: getting filehandles should probably be done in a single place
  # especially since we need to support UTF8 or other encoding when dealing
  # with perlop, perlfunc, perlapi, perlfaq[1-9]
  my $fh = $self->open_fh('<', $perlop);

  my $thing = $self->opt_f;

  my $previous_line;
  my $push = 0;
  my $seen_item = 0;
  my $skip = 1;

  while( my $line = <$fh> ) {
    $line =~ /^=encoding\s+(\S+)/ && $self->set_encoding($fh, $1);
    # only start search after we hit the operator section
    if ($line =~ m!^X<operator, regexp>!) {
        $skip = 0;
    }

    next if $skip;

    # strategy is to capture the previous line until we get a match on X<$thingy>
    # if the current line contains X<$thingy>, then we push "=over", the previous line, 
    # the current line and keep pushing current line until we see a ^X<some-other-thing>, 
    # then we chop off final line from @$pod and add =back
    #
    # At that point, Bob's your uncle.

    if ( $line =~ m!X<+\s*\Q$thing\E\s*>+!) {
        if ( $previous_line ) {
            push @$pod, "=over 8\n\n", $previous_line;
            $previous_line = "";
        }
        push @$pod, $line;
        $push = 1;

    }
    elsif ( $push and $line =~ m!^=item\s*.*$! ) {
        $seen_item = 1;
    }
    elsif ( $push and $seen_item and $line =~ m!^X<+\s*[ a-z,?-]+\s*>+!) {
        $push = 0;
        $seen_item = 0;
        last;
    }
    elsif ( $push ) {
        push @$pod, $line;
    }

    else {
        $previous_line = $line;
    }

  } #end while

  # we overfilled by 1 line, so pop off final array element if we have any
  if ( scalar @$pod ) {
    pop @$pod;

    # and add the =back
    push @$pod, "\n\n=back\n";
    DEBUG > 8 and print "PERLOP POD --->" . (join "", @$pod) . "<---\n";
  }
  else {
    DEBUG > 4 and print "No pod from perlop\n";
  }

  close $fh;

  return;
}

#..........................................................................

sub search_perlapi {
    my($self, $found_things, $pod) = @_;

    DEBUG > 2 and print "Search: @$found_things\n";

    my $perlapi = shift @$found_things;
    my $fh = $self->open_fh('<', $perlapi);

    my $search_re = quotemeta($self->opt_a);

    DEBUG > 2 and
     print "Going to perlapi-scan for $search_re in $perlapi\n";

    local $_;

    # Look for our function
    my $found = 0;
    my $inlist = 0;

    my @related;
    my $related_re;
    while (<$fh>) {
        /^=encoding\s+(\S+)/ && $self->set_encoding($fh, $1);

        if ( m/^=item\s+$search_re\b/ )  {
            $found = 1;
        }
        elsif (@related > 1 and /^=item/) {
            $related_re ||= join "|", @related;
            if (m/^=item\s+(?:$related_re)\b/) {
                $found = 1;
            }
            else {
                last;
            }
        }
        elsif (/^=item/) {
            last if $found > 1 and not $inlist;
        }
        elsif ($found and /^X<[^>]+>/) {
            push @related, m/X<([^>]+)>/g;
        }
        next unless $found;
        if (/^=over/) {
            ++$inlist;
        }
        elsif (/^=back/) {
            last if $found > 1 and not $inlist;
            --$inlist;
        }
        push @$pod, $_;
        ++$found if /^\w/;        # found descriptive text
    }

    if (!@$pod) {
        CORE::die( sprintf
          "No documentation for perl api function '%s' found\n",
          $self->opt_a )
        ;
    }
    close $fh                or $self->die( "Can't open $perlapi: $!" );

    return;
}

#..........................................................................

sub search_perlfunc {
    my($self, $found_things, $pod) = @_;

    DEBUG > 2 and print "Search: @$found_things\n";

    my $pfunc = shift @$found_things;
    my $fh = $self->open_fh("<", $pfunc); # "Funk is its own reward"

    # Functions like -r, -e, etc. are listed under `-X'.
    my $search_re = ($self->opt_f =~ /^-[rwxoRWXOeszfdlpSbctugkTBMAC]$/)
                        ? '(?:I<)?-X' : quotemeta($self->opt_f) ;

    DEBUG > 2 and
     print "Going to perlfunc-scan for $search_re in $pfunc\n";

    my $re = 'Alphabetical Listing of Perl Functions';

    # Check available translator or backup to default (english)
    if ( $self->opt_L && defined $self->{'translators'}->[0] ) {
        my $tr = $self->{'translators'}->[0];
        $re =  $tr->search_perlfunc_re if $tr->can('search_perlfunc_re');
        if ( $] < 5.008 ) {
            $self->aside("Your old perl doesn't really have proper unicode support.");
        }
    }

    # Skip introduction
    local $_;
    while (<$fh>) {
        /^=encoding\s+(\S+)/ && $self->set_encoding($fh, $1);
        last if /^=head2 (?:$re|Alphabetical Listing of Perl Functions)/;
    }

    # Look for our function
    my $found = 0;
    my $inlist = 0;

    my @perlops = qw(m q qq qr qx qw s tr y);

    my @related;
    my $related_re;
    while (<$fh>) {  # "The Mothership Connection is here!"
        last if( grep{ $self->opt_f eq $_ }@perlops );

        if ( /^=over/ and not $found ) {
            ++$inlist;
        }
        elsif ( /^=back/ and not $found and $inlist ) {
            --$inlist;
        }


        if ( m/^=item\s+$search_re\b/ and $inlist < 2 )  {
            $found = 1;
        }
        elsif (@related > 1 and /^=item/) {
            $related_re ||= join "|", @related;
            if (m/^=item\s+(?:$related_re)\b/) {
                $found = 1;
            }
            else {
                last if $found > 1 and $inlist < 2;
            }
        }
        elsif (/^=item|^=back/) {
            last if $found > 1 and $inlist < 2;
        }
        elsif ($found and /^X<[^>]+>/) {
            push @related, m/X<([^>]+)>/g;
        }
        next unless $found;
        if (/^=over/) {
            ++$inlist;
        }
        elsif (/^=back/) {
            --$inlist;
        }
        push @$pod, $_;
        ++$found if /^\w/;        # found descriptive text
    }

    if( !@$pod ){
        $self->search_perlop( $found_things, $pod );
    }

    if (!@$pod) {
        CORE::die( sprintf
          "No documentation for perl function '%s' found\n",
          $self->opt_f )
        ;
    }
    close $fh                or $self->die( "Can't close $pfunc: $!" );

    return;
}

#..........................................................................

sub search_perlfaqs {
    my( $self, $found_things, $pod) = @_;

    my $found = 0;
    my %found_in;
    my $search_key = $self->opt_q;

    my $rx = eval { qr/$search_key/ }
     or $self->die( <<EOD );
Invalid regular expression '$search_key' given as -q pattern:
$@
Did you mean \\Q$search_key ?

EOD

    local $_;
    foreach my $file (@$found_things) {
        $self->die( "invalid file spec: $!" ) if $file =~ /[<>|]/;
        my $fh = $self->open_fh("<", $file);
        while (<$fh>) {
            /^=encoding\s+(\S+)/ && $self->set_encoding($fh, $1);
            if ( m/^=head2\s+.*(?:$search_key)/i ) {
                $found = 1;
                push @$pod, "=head1 Found in $file\n\n" unless $found_in{$file}++;
            }
            elsif (/^=head[12]/) {
                $found = 0;
            }
            next unless $found;
            push @$pod, $_;
        }
        close($fh);
    }
    CORE::die("No documentation for perl FAQ keyword '$search_key' found\n")
     unless @$pod;

    if ( $self->opt_l ) {
        CORE::die((join "\n", keys %found_in) . "\n");
    }
    return;
}


#..........................................................................

sub render_findings {
  # Return the filename to open

  my($self, $found_things) = @_;

  my $formatter_class = $self->{'formatter_class'}
   || $self->die( "No formatter class set!?" );
  my $formatter = $formatter_class->can('new')
    ? $formatter_class->new
    : $formatter_class
  ;

  if(! @$found_things) {
    $self->die( "Nothing found?!" );
    # should have been caught before here
  } elsif(@$found_things > 1) {
    $self->warn(
     "Perldoc is only really meant for reading one document at a time.\n",
     "So these parameters are being ignored: ",
     join(' ', @$found_things[1 .. $#$found_things] ),
     "\n" );
  }

  my $file = $found_things->[0];

  DEBUG > 3 and printf "Formatter switches now: [%s]\n",
   join ' ', map "[@$_]", @{ $self->{'formatter_switches'} };

  # Set formatter options:
  if( ref $formatter ) {
    foreach my $f (@{ $self->{'formatter_switches'} || [] }) {
      my($switch, $value, $silent_fail) = @$f;
      if( $formatter->can($switch) ) {
        eval { $formatter->$switch( defined($value) ? $value : () ) };
        $self->warn( "Got an error when setting $formatter_class\->$switch:\n$@\n" )
         if $@;
      } else {
        if( $silent_fail or $switch =~ m/^__/s ) {
          DEBUG > 2 and print "Formatter $formatter_class doesn't support $switch\n";
        } else {
          $self->warn( "$formatter_class doesn't recognize the $switch switch.\n" );
        }
      }
    }
  }

  $self->{'output_is_binary'} =
    $formatter->can('write_with_binmode') && $formatter->write_with_binmode;

  if( $self->{podnames} and exists $self->{podnames}{$file} and
      $formatter->can('name') ) {
    $formatter->name($self->{podnames}{$file});
  }

  my ($out_fh, $out) = $self->new_output_file(
    ( $formatter->can('output_extension') && $formatter->output_extension )
     || undef,
    $self->useful_filename_bit,
  );

  # Now, finally, do the formatting!
  {
    local $^W = $^W;
    if(DEBUG() or $self->opt_D) {
      # feh, let 'em see it
    } else {
      $^W = 0;
      # The average user just has no reason to be seeing
      #  $^W-suppressible warnings from the formatting!
    }

    eval {  $formatter->parse_from_file( $file, $out_fh )  };
  }

  $self->warn( "Error while formatting with $formatter_class:\n $@\n" ) if $@;
  DEBUG > 2 and print "Back from formatting with $formatter_class\n";

  close $out_fh
   or $self->warn( "Can't close $out: $!\n(Did $formatter already close it?)" );
  sleep 0; sleep 0; sleep 0;
   # Give the system a few timeslices to meditate on the fact
   # that the output file does in fact exist and is closed.

  $self->unlink_if_temp_file($file);

  unless( -s $out ) {
    if( $formatter->can( 'if_zero_length' ) ) {
      # Basically this is just a hook for Pod::Simple::Checker; since
      # what other class could /happily/ format an input file with Pod
      # as a 0-length output file?
      $formatter->if_zero_length( $file, $out, $out_fh );
    } else {
      $self->warn( "Got a 0-length file from $$found_things[0] via $formatter_class!?\n" );
    }
  }

  DEBUG and print "Finished writing to $out.\n";
  return($out, $formatter) if wantarray;
  return $out;
}

#..........................................................................

sub unlink_if_temp_file {
  # Unlink the specified file IFF it's in the list of temp files.
  # Really only used in the case of -f / -q things when we can
  #  throw away the dynamically generated source pod file once
  #  we've formatted it.
  #
  my($self, $file) = @_;
  return unless defined $file and length $file;

  my $temp_file_list = $self->{'temp_file_list'} || return;
  if(grep $_ eq $file, @$temp_file_list) {
    $self->aside("Unlinking $file\n");
    unlink($file) or $self->warn( "Odd, couldn't unlink $file: $!" );
  } else {
    DEBUG > 1 and print "$file isn't a temp file, so not unlinking.\n";
  }
  return;
}

#..........................................................................


sub after_rendering {
  my $self = $_[0];
  $self->after_rendering_VMS     if $self->is_vms;
  $self->after_rendering_MSWin32 if $self->is_mswin32;
  $self->after_rendering_Dos     if $self->is_dos;
  $self->after_rendering_OS2     if $self->is_os2;
  return;
}

sub after_rendering_VMS      { return }
sub after_rendering_Dos      { return }
sub after_rendering_OS2      { return }
sub after_rendering_MSWin32  { return }

#..........................................................................
#   :   :   :   :   :   :   :   :   :
#..........................................................................

sub minus_f_nocase {   # i.e., do like -f, but without regard to case

     my($self, $dir, $file) = @_;
     my $path = catfile($dir,$file);
     return $path if -f $path and -r _;

     if(!$self->opt_i
        or $self->is_vms or $self->is_mswin32
        or $self->is_dos or $self->is_os2
     ) {
        # On a case-forgiving file system, or if case is important,
    #  that is it, all we can do.
    $self->warn( "Ignored $path: unreadable\n" ) if -f _;
    return '';
     }

     local *DIR;
     my @p = ($dir);
     my($p,$cip);
     foreach $p (splitdir $file){
    my $try = catfile @p, $p;
        $self->aside("Scrutinizing $try...\n");
    stat $try;
    if (-d _) {
        push @p, $p;
        if ( $p eq $self->{'target'} ) {
        my $tmp_path = catfile @p;
        my $path_f = 0;
        for (@{ $self->{'found'} }) {
            $path_f = 1 if $_ eq $tmp_path;
        }
        push (@{ $self->{'found'} }, $tmp_path) unless $path_f;
        $self->aside( "Found as $tmp_path but directory\n" );
        }
    }
    elsif (-f _ && -r _ && lc($try) eq lc($path)) {
        return $try;
    }
    elsif (-f _) {
        $self->warn( "Ignored $try: unreadable or file/dir mismatch\n" );
    }
    elsif (-d catdir(@p)) {  # at least we see the containing directory!
        my $found = 0;
        my $lcp = lc $p;
        my $p_dirspec = catdir(@p);
        opendir DIR, $p_dirspec  or $self->die( "opendir $p_dirspec: $!" );
        while(defined( $cip = readdir(DIR) )) {
        if (lc $cip eq $lcp){
            $found++;
            last; # XXX stop at the first? what if there's others?
        }
        }
        closedir DIR  or $self->die( "closedir $p_dirspec: $!" );
        return "" unless $found;

        push @p, $cip;
        my $p_filespec = catfile(@p);
        return $p_filespec if -f $p_filespec and -r _;
        $self->warn( "Ignored $p_filespec: unreadable\n" ) if -f _;
    }
     }
     return "";
}

#..........................................................................

sub pagers_guessing {
    # TODO: This whole subroutine needs to be rewritten. It's semi-insane
    # right now.

    my $self = shift;

    my @pagers;
    push @pagers, $self->pagers;
    $self->{'pagers'} = \@pagers;

    if ($self->is_mswin32) {
        push @pagers, qw( more< less notepad );
        unshift @pagers, $ENV{PAGER}  if $ENV{PAGER};
    }
    elsif ($self->is_vms) {
        push @pagers, qw( most more less type/page );
    }
    elsif ($self->is_dos) {
        push @pagers, qw( less.exe more.com< );
        unshift @pagers, $ENV{PAGER}  if $ENV{PAGER};
    }
    elsif ( $self->is_amigaos) { 
      push @pagers, qw( /SYS/Utilities/MultiView /SYS/Utilities/More /C/TYPE );
      unshift @pagers, "$ENV{PAGER}" if $ENV{PAGER}; 
    }
    else {
        if ($self->is_os2) {
          unshift @pagers, 'less', 'cmd /c more <';
        }
        push @pagers, qw( more less pg view cat );
        unshift @pagers, "$ENV{PAGER} <"  if $ENV{PAGER};
    }

    if ($self->is_cygwin) {
        if (($pagers[0] eq 'less') || ($pagers[0] eq '/usr/bin/less')) {
            unshift @pagers, '/usr/bin/less -isrR';
            unshift @pagers, $ENV{PAGER}  if $ENV{PAGER};
       }
    }

    if ( $self->opt_m ) {
        unshift @pagers, "$ENV{PERLDOC_SRC_PAGER}" if $ENV{PERLDOC_SRC_PAGER}
    }
    else {
        unshift @pagers, "$ENV{MANPAGER} <" if $ENV{MANPAGER};
        unshift @pagers, "$ENV{PERLDOC_PAGER} <" if $ENV{PERLDOC_PAGER};
    }

    $self->aside("Pagers: ", (join ", ", @pagers));

    return;
}

#..........................................................................

sub page_module_file {
    my($self, @found) = @_;

    # Security note:
    # Don't ever just pass this off to anything like MSWin's "start.exe",
    # since we might be calling on a .pl file, and we wouldn't want that
    # to actually /execute/ the file that we just want to page thru!
    # Also a consideration if one were to use a web browser as a pager;
    # doing so could trigger the browser's MIME mapping for whatever
    # it thinks .pm/.pl/whatever is.  Probably just a (useless and
    # annoying) "Save as..." dialog, but potentially executing the file
    # in question -- particularly in the case of MSIE and it's, ahem,
    # occasionally hazy distinction between OS-local extension
    # associations, and browser-specific MIME mappings.

    if(@found > 1) {
        $self->warn(
            "Perldoc is only really meant for reading one document at a time.\n" .
            "So these files are being ignored: " .
            join(' ', @found[1 .. $#found] ) .
            "\n" )
    }

    return $self->page($found[0], $self->{'output_to_stdout'}, $self->pagers);

}

#..........................................................................

sub check_file {
    my($self, $dir, $file) = @_;

    unless( ref $self ) {
      # Should never get called:
      $Carp::Verbose = 1;
      require Carp;
      Carp::croak( join '',
        "Crazy ", __PACKAGE__, " error:\n",
        "check_file must be an object_method!\n",
        "Aborting"
      );
    }

    if(length $dir and not -d $dir) {
      DEBUG > 3 and print "  No dir $dir -- skipping.\n";
      return "";
    }

    my $path = $self->minus_f_nocase($dir,$file);
    if( length $path and ($self->opt_m ? $self->isprintable($path)
                                      : $self->containspod($path)) ) {
        DEBUG > 3 and print
            "  The file $path indeed looks promising!\n";
        return $path;
    }
    DEBUG > 3 and print "  No good: $file in $dir\n";

    return "";
}

sub isprintable {
	my($self, $file, $readit) = @_;
	my $size= 1024;
	my $maxunprintfrac= 0.2;   # tolerate some unprintables for UTF-8 comments etc.

	return 1 if !$readit && $file =~ /\.(?:pl|pm|pod|cmd|com|bat)\z/i;

	my $data;
	local($_);
	my $fh = $self->open_fh("<", $file);
	read $fh, $data, $size;
	close $fh;
	$size= length($data);
	$data =~ tr/\x09-\x0D\x20-\x7E//d;
	return length($data) <= $size*$maxunprintfrac;
}

#..........................................................................

sub containspod {
    my($self, $file, $readit) = @_;
    return 1 if !$readit && $file =~ /\.pod\z/i;


    #  Under cygwin the /usr/bin/perl is legal executable, but
    #  you cannot open a file with that name. It must be spelled
    #  out as "/usr/bin/perl.exe".
    #
    #  The following if-case under cygwin prevents error
    #
    #     $ perldoc perl
    #     Cannot open /usr/bin/perl: no such file or directory
    #
    #  This would work though
    #
    #     $ perldoc perl.pod

    if ( $self->is_cygwin  and  -x $file  and  -f "$file.exe" )
    {
        $self->warn( "Cygwin $file.exe search skipped\n" ) if DEBUG or $self->opt_D;
        return 0;
    }

    local($_);
    my $fh = $self->open_fh("<", $file);
    while (<$fh>) {
    if (/^=head/) {
        close($fh)     or $self->die( "Can't close $file: $!" );
        return 1;
    }
    }
    close($fh)         or $self->die( "Can't close $file: $!" );
    return 0;
}

#..........................................................................

sub maybe_extend_searchpath {
  my $self = shift;

  # Does this look like a module or extension directory?

  if (-f "Makefile.PL" || -f "Build.PL") {

    push @{$self->{search_path} }, '.','lib';

    # don't add if superuser
    if ($< && $> && -d "blib") {   # don't be looking too hard now!
      push @{ $self->{search_path} }, 'blib';
      $self->warn( $@ ) if $@ && $self->opt_D;
    }
  }

  return;
}

#..........................................................................

sub new_output_file {
  my $self = shift;
  my $outspec = $self->opt_d;  # Yes, -d overrides all else!
                               # So don't call this twice per format-job!

  return $self->new_tempfile(@_) unless defined $outspec and length $outspec;

  # Otherwise open a write-handle on opt_d!f

  DEBUG > 3 and print "About to try writing to specified output file $outspec\n";
  my $fh = $self->open_fh(">", $outspec);

  DEBUG > 3 and print "Successfully opened $outspec\n";
  binmode($fh) if $self->{'output_is_binary'};
  return($fh, $outspec);
}

#..........................................................................

sub useful_filename_bit {
  # This tries to provide a meaningful bit of text to do with the query,
  # such as can be used in naming the file -- since if we're going to be
  # opening windows on temp files (as a "pager" may well do!) then it's
  # better if the temp file's name (which may well be used as the window
  # title) isn't ALL just random garbage!
  # In other words "perldoc_LWPSimple_2371981429" is a better temp file
  # name than "perldoc_2371981429".  So this routine is what tries to
  # provide the "LWPSimple" bit.
  #
  my $self = shift;
  my $pages = $self->{'pages'} || return undef;
  return undef unless @$pages;

  my $chunk = $pages->[0];
  return undef unless defined $chunk;
  $chunk =~ s/:://g;
  $chunk =~ s/\.\w+$//g; # strip any extension
  if( $chunk =~ m/([^\#\\:\/\$]+)$/s ) { # get basename, if it's a file
    $chunk = $1;
  } else {
    return undef;
  }
  $chunk =~ s/[^a-zA-Z0-9]+//g; # leave ONLY a-zA-Z0-9 things!
  $chunk = substr($chunk, -10) if length($chunk) > 10;
  return $chunk;
}

#..........................................................................

sub new_tempfile {    # $self->new_tempfile( [$suffix, [$infix] ] )
  my $self = shift;

  ++$Temp_Files_Created;

  require File::Temp;
  return File::Temp::tempfile(UNLINK => 1);
}

#..........................................................................

sub page {  # apply a pager to the output file
    my ($self, $output, $output_to_stdout, @pagers) = @_;
    if ($output_to_stdout) {
        $self->aside("Sending unpaged output to STDOUT.\n");
        my $fh = $self->open_fh("<", $output);
        local $_;
        while (<$fh>) {
            print or $self->die( "Can't print to stdout: $!" );
        }
        close $fh or $self->die( "Can't close while $output: $!" );
        $self->unlink_if_temp_file($output);
    } else {
        # On VMS, quoting prevents logical expansion, and temp files with no
        # extension get the wrong default extension (such as .LIS for TYPE)

        $output = VMS::Filespec::rmsexpand($output, '.') if $self->is_vms;

        $output =~ s{/}{\\}g if $self->is_mswin32 || $self->is_dos;
        # Altho "/" under MSWin is in theory good as a pathsep,
        #  many many corners of the OS don't like it.  So we
        #  have to force it to be "\" to make everyone happy.

	# if we are on an amiga convert unix path to an amiga one 
	$output =~ s/^\/(.*)\/(.*)/$1:$2/ if $self->is_amigaos;

        foreach my $pager (@pagers) {
            $self->aside("About to try calling $pager $output\n");
            if ($self->is_vms) {
                last if system("$pager $output") == 0;
	    } elsif($self->is_amigaos) { 
                last if system($pager, $output) == 0;
            } else {
                last if system("$pager \"$output\"") == 0;
            }
        }
    }
    return;
}

#..........................................................................

sub searchfor {
    my($self, $recurse,$s,@dirs) = @_;
    $s =~ s!::!/!g;
    $s = VMS::Filespec::unixify($s) if $self->is_vms;
    return $s if -f $s && $self->containspod($s);
    $self->aside( "Looking for $s in @dirs\n" );
    my $ret;
    my $i;
    my $dir;
    $self->{'target'} = (splitdir $s)[-1];  # XXX: why not use File::Basename?
    for ($i=0; $i<@dirs; $i++) {
    $dir = $dirs[$i];
    next unless -d $dir;
    ($dir = VMS::Filespec::unixpath($dir)) =~ s!/\z!! if $self->is_vms;
    if (       (! $self->opt_m && ( $ret = $self->check_file($dir,"$s.pod")))
        or ( $ret = $self->check_file($dir,"$s.pm"))
        or ( $ret = $self->check_file($dir,$s))
        or ( $self->is_vms and
             $ret = $self->check_file($dir,"$s.com"))
        or ( $self->is_os2 and
             $ret = $self->check_file($dir,"$s.cmd"))
        or ( ($self->is_mswin32 or $self->is_dos or $self->is_os2) and
             $ret = $self->check_file($dir,"$s.bat"))
        or ( $ret = $self->check_file("$dir/pod","$s.pod"))
        or ( $ret = $self->check_file("$dir/pod",$s))
        or ( $ret = $self->check_file("$dir/pods","$s.pod"))
        or ( $ret = $self->check_file("$dir/pods",$s))
    ) {
        DEBUG > 1 and print "  Found $ret\n";
        return $ret;
    }

    if ($recurse) {
        opendir(D,$dir) or $self->die( "Can't opendir $dir: $!" );
        my @newdirs = map catfile($dir, $_), grep {
        not /^\.\.?\z/s and
        not /^auto\z/s  and   # save time! don't search auto dirs
        -d  catfile($dir, $_)
        } readdir D;
        closedir(D)     or $self->die( "Can't closedir $dir: $!" );
        next unless @newdirs;
        # what a wicked map!
        @newdirs = map((s/\.dir\z//,$_)[1],@newdirs) if $self->is_vms;
        $self->aside( "Also looking in @newdirs\n" );
        push(@dirs,@newdirs);
    }
    }
    return ();
}

#..........................................................................
{
  my $already_asserted;
  sub assert_closing_stdout {
    my $self = shift;

    return if $already_asserted;

    eval  q~ END { close(STDOUT) || CORE::die "Can't close STDOUT: $!" } ~;
     # What for? to let the pager know that nothing more will come?

    $self->die( $@ ) if $@;
    $already_asserted = 1;
    return;
  }
}

#..........................................................................

sub tweak_found_pathnames {
  my($self, $found) = @_;
  if ($self->is_mswin32) {
    foreach (@$found) { s,/,\\,g }
  }
  foreach (@$found) { s,',\\',g } # RT 37347
  return;
}

#..........................................................................
#   :   :   :   :   :   :   :   :   :
#..........................................................................

sub am_taint_checking {
    my $self = shift;
    $self->die( "NO ENVIRONMENT?!?!" ) unless keys %ENV; # reset iterator along the way
    my($k,$v) = each %ENV;
    return is_tainted($v);
}

#..........................................................................

sub is_tainted { # just a function
    my $arg  = shift;
    my $nada = substr($arg, 0, 0);  # zero-length!
    local $@;  # preserve the caller's version of $@
    eval { eval "# $nada" };
    return length($@) != 0;
}

#..........................................................................

sub drop_privs_maybe {
    my $self = shift;

    DEBUG and print "Attempting to drop privs...\n";

    # Attempt to drop privs if we should be tainting and aren't
    if (!( $self->is_vms || $self->is_mswin32 || $self->is_dos
          || $self->is_os2
         )
        && ($> == 0 || $< == 0)
        && !$self->am_taint_checking()
    ) {
        my $id = eval { getpwnam("nobody") };
        $id = eval { getpwnam("nouser") } unless defined $id;
        $id = -2 unless defined $id;
            #
            # According to Stevens' APUE and various
            # (BSD, Solaris, HP-UX) man pages, setting
            # the real uid first and effective uid second
            # is the way to go if one wants to drop privileges,
            # because if one changes into an effective uid of
            # non-zero, one cannot change the real uid any more.
            #
            # Actually, it gets even messier.  There is
            # a third uid, called the saved uid, and as
            # long as that is zero, one can get back to
            # uid of zero.  Setting the real-effective *twice*
            # helps in *most* systems (FreeBSD and Solaris)
            # but apparently in HP-UX even this doesn't help:
            # the saved uid stays zero (apparently the only way
            # in HP-UX to change saved uid is to call setuid()
            # when the effective uid is zero).
            #
        eval {
            $< = $id; # real uid
            $> = $id; # effective uid
            $< = $id; # real uid
            $> = $id; # effective uid
        };
        if( !$@ && $< && $> ) {
          DEBUG and print "OK, I dropped privileges.\n";
        } elsif( $self->opt_U ) {
          DEBUG and print "Couldn't drop privileges, but in -U mode, so feh."
        } else {
          DEBUG and print "Hm, couldn't drop privileges.  Ah well.\n";
          # We used to die here; but that seemed pointless.
        }
    }
    return;
}

#..........................................................................

1;

__END__

=head1 NAME

Pod::Perldoc - Look up Perl documentation in Pod format.

=head1 SYNOPSIS

    use Pod::Perldoc ();

    Pod::Perldoc->run();

=head1 DESCRIPTION

The guts of L<perldoc> utility.

=head1 SEE ALSO

L<perldoc>

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2002-2007 Sean M. Burke.

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

This program is distributed in the hope that it will be useful, but
without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.

=head1 AUTHOR

Current maintainer: Mark Allen C<< <mallen@cpan.org> >>

Past contributions from:
brian d foy C<< <bdfoy@cpan.org> >>
Adriano R. Ferreira C<< <ferreira@cpan.org> >>,
Sean M. Burke C<< <sburke@cpan.org> >>

=cut
